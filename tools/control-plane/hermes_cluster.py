"""HERMES control plane — cluster state and operations."""

from __future__ import annotations

import json
import random
import sys
import threading
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent

# Legacy 3-node demo cluster — stripped from UI when not connected via bridge/agent.
_DEMO_NODE_NAMES = frozenset({"STM32H7", "ESP32-S3", "nRF52840"})
_DEMO_APP_NAMES = frozenset({"smartfarm", "telemetry", "sensor-net"})

from persist import save_state, load_state, save_checkpoint  # noqa: E402
from blueprints import get_blueprint, list_blueprints, validate_config, display_name  # noqa: E402
from app_config import save_config, load_config, save_runtime, list_configs  # noqa: E402
from blueprint_engine import init_runtime, tick as blueprint_tick, display_label  # noqa: E402


def _verify_binary(binary_path: Path, manifest: dict) -> str | None:
    """Return error string or None if OK. Skips when no binary file."""
    if not binary_path.is_file():
        return None
    import hashlib
    data = binary_path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if manifest.get("binary_sha256") and manifest["binary_sha256"] != digest:
        return "binary hash mismatch"
    sig = manifest.get("signature")
    if sig:
        sys.path.insert(0, str(ROOT / "sdk"))
        from sign import verify_bytes  # noqa: E402
        if not verify_bytes(data, sig):
            return "binary signature invalid"
    return None


@dataclass
class Node:
    id: int
    name: str
    node_type: str
    cpu: float = 0.0
    ram_kb: int = 0
    ram_total_kb: int = 512
    temp_c: float = 25.0
    power_mw: float = 0.0
    online: bool = True
    link_quality: int = 100
    last_heartbeat: float = field(default_factory=time.time)
    agent_connected: bool = False

    def to_dict(self) -> dict:
        return asdict(self)


@dataclass
class App:
    name: str
    version: str
    node_id: int
    state: str
    cpu_percent: float = 0.0
    memory_kb: int = 64
    priority: int = 5
    blueprint: str = ""
    display_name: str = ""
    metrics: dict = field(default_factory=dict)

    def to_dict(self) -> dict:
        return asdict(self)


@dataclass
class NodeCommand:
    cmd_id: int
    action: str
    app_name: str = ""
    payload: dict = field(default_factory=dict)


class ClusterManager:
    """Operates all MCUs as one system — registration, health, recovery."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._cmd_seq = 0
        self._commands: dict[int, list[NodeCommand]] = {}
        self._frame_queues: dict[int, list[dict]] = {}
        self._link_stats: dict[str, int] = {"frames_routed": 0, "pings": 0}
        self.leader_id: int = 0
        self.nodes: dict[int, Node] = {}
        self.apps: dict[str, App] = {}
        self.logs: list[str] = []
        self._quotas: dict[str, dict] = {}
        self._load_persisted()
        self._prune_legacy_demo()
        self._sync_blueprint_configs()
        self._start_telemetry()

    def _prune_legacy_demo(self) -> None:
        """Remove pre-seeded demo nodes/apps from older persisted state."""
        with self._lock:
            self._prune_legacy_demo_unlocked()

    def _prune_legacy_demo_unlocked(self) -> None:
        for nid in list(self.nodes.keys()):
            n = self.nodes[nid]
            if n.name in _DEMO_NODE_NAMES and not n.agent_connected:
                del self.nodes[nid]
                self._commands.pop(nid, None)
                self._frame_queues.pop(nid, None)
        for name in list(self.apps.keys()):
            app = self.apps[name]
            if name in _DEMO_APP_NAMES and not app.blueprint:
                del self.apps[name]
                self._quotas.pop(name, None)

    def _visible_nodes_unlocked(self) -> list[Node]:
        """Nodes connected via serial bridge or node agent."""
        return [n for n in self.nodes.values() if n.agent_connected]

    def _visible_node_ids_unlocked(self) -> set[int]:
        return {n.id for n in self._visible_nodes_unlocked()}

    def _sync_blueprint_configs(self) -> None:
        """Restore blueprint metadata from data/apps/ after restart."""
        for name, data in list_configs().items():
            if name not in self.apps:
                bp_id = data.get("blueprint", "")
                bp = get_blueprint(bp_id) if bp_id else None
                cfg = data.get("config", {})
                nid = int(cfg.get("target_node", 1))
                self.apps[name] = App(
                    name=name,
                    version=bp["version"] if bp else "1.0.0",
                    node_id=nid,
                    state="installed",
                    memory_kb=_parse_memory(bp["memory"]) if bp else 64,
                    blueprint=bp_id,
                    display_name=display_name(bp_id, cfg) if bp_id else name,
                    metrics=data.get("runtime", {}),
                )

    def _serialize(self) -> dict:
        return {
            "leader_id": self.leader_id,
            "nodes": {str(k): v.to_dict() for k, v in self.nodes.items()},
            "apps": {k: v.to_dict() for k, v in self.apps.items()},
            "quotas": self._quotas,
            "logs": self.logs[:50],
        }

    def _persist_unlocked(self) -> None:
        save_state(self._serialize())

    def _persist(self) -> None:
        with self._lock:
            self._persist_unlocked()

    def _load_persisted(self) -> None:
        data = load_state()
        if not data:
            return
        self.leader_id = data.get("leader_id", self.leader_id)
        for k, v in data.get("nodes", {}).items():
            nid = int(k)
            self.nodes[nid] = Node(**v)
        for k, v in data.get("apps", {}).items():
            defaults = {
                "cpu_percent": 0.0,
                "memory_kb": 64,
                "priority": 5,
                "blueprint": "",
                "display_name": "",
                "metrics": {},
            }
            defaults.update(v)
            self.apps[k] = App(**defaults)
        self._quotas.update(data.get("quotas", {}))
        if data.get("logs"):
            self.logs = data["logs"]
        self._log("Restored persisted cluster state")

    def _log(self, msg: str) -> None:
        ts = time.strftime("%H:%M:%S")
        entry = f"[{ts}] {msg}"
        self.logs.insert(0, entry)
        if len(self.logs) > 100:
            self.logs.pop()
        self._persist_unlocked()

    def _start_telemetry(self) -> None:
        def tick() -> None:
            while True:
                time.sleep(2)
                with self._lock:
                    for node in self.nodes.values():
                        if not node.online or node.agent_connected:
                            continue
                        node.cpu = max(5, min(95, node.cpu + random.uniform(-4, 4)))
                        node.temp_c = max(20, min(85, node.temp_c + random.uniform(-1, 1)))
                        node.power_mw = max(50, node.power_mw + random.uniform(-20, 20))
                        node.last_heartbeat = time.time()
                    for app in self.apps.values():
                        if app.state == "running":
                            app.cpu_percent = max(1, app.cpu_percent + random.uniform(-2, 2))
                            if app.blueprint:
                                stored = load_config(app.name)
                                if stored:
                                    cfg = stored.get("config", {})
                                    rt = stored.get("runtime") or init_runtime(app.blueprint, cfg)
                                    app.metrics = blueprint_tick(app.blueprint, cfg, rt)
                                    save_runtime(app.name, rt)
                    self._check_node_health()
                    self._update_leader()
        threading.Thread(target=tick, daemon=True).start()

    def _update_leader(self) -> None:
        online = [n for n in self.nodes.values() if n.online]
        if not online:
            return
        best = min(online, key=lambda n: n.cpu)
        self.leader_id = best.id

    def _check_node_health(self) -> None:
        now = time.time()
        for node in self.nodes.values():
            if node.online and now - node.last_heartbeat > 30:
                node.online = False
                node.agent_connected = False
                self._log(f"Node {node.name} offline - redistributing work")
                self._redistribute(node.id)

    def _redistribute(self, failed_node_id: int) -> None:
        targets = [n.id for n in self.nodes.values() if n.online and n.id != failed_node_id]
        if not targets:
            return
        i = 0
        for app in self.apps.values():
            if app.node_id == failed_node_id and app.state == "running":
                app.node_id = targets[i % len(targets)]
                i += 1
                self._log(f"Redistributed {app.name} -> node {app.node_id}")
                self._enqueue(app.node_id, "run_app", app.name)

    def _enqueue(self, node_id: int, action: str, app_name: str = "", payload: dict | None = None) -> int:
        self._cmd_seq += 1
        cmd = NodeCommand(self._cmd_seq, action, app_name, payload or {})
        self._commands.setdefault(node_id, []).append(cmd)
        return cmd.cmd_id

    def register_node(self, node_id: int, name: str, node_type: str, agent: bool = False) -> dict:
        with self._lock:
            if node_id in self.nodes:
                n = self.nodes[node_id]
                n.name = name
                n.node_type = node_type
                n.online = True
                n.agent_connected = agent
                n.last_heartbeat = time.time()
                self._prune_legacy_demo_unlocked()
                self._log(f"Node {name} re-registered (agent={agent})")
            else:
                self.nodes[node_id] = Node(
                    node_id, name, node_type, online=True, agent_connected=agent,
                )
                self._commands.setdefault(node_id, [])
                self._frame_queues.setdefault(node_id, [])
                self._prune_legacy_demo_unlocked()
                self._log(f"Node {name} registered (id={node_id})")
            return self.nodes[node_id].to_dict()

    def heartbeat(self, node_id: int, telemetry: dict | None = None) -> dict:
        with self._lock:
            if node_id not in self.nodes:
                return {"error": "unknown node"}
            n = self.nodes[node_id]
            n.online = True
            n.last_heartbeat = time.time()
            if telemetry:
                n.agent_connected = True
                n.cpu = float(telemetry.get("cpu", n.cpu))
                n.ram_kb = int(telemetry.get("ram_kb", n.ram_kb))
                n.temp_c = float(telemetry.get("temp_c", n.temp_c))
                n.power_mw = float(telemetry.get("power_mw", n.power_mw))
                n.link_quality = int(telemetry.get("link_quality", n.link_quality))
                try:
                    from mesh import ingest_telemetry  # noqa: E402
                    ingest_telemetry(telemetry)
                except Exception:
                    pass
            cmds = self._commands.get(node_id, [])
            pending = [{"cmd_id": c.cmd_id, "action": c.action, "app_name": c.app_name, "payload": c.payload}
                       for c in cmds]
            frames = self._frame_queues.get(node_id, [])
            delivered = frames[:]
            self._frame_queues[node_id] = []
            return {"ok": True, "leader": self.leader_id, "commands": pending, "frames": delivered}

    def link_send(self, frame: dict) -> dict:
        """Route inter-node frame to destination queue (cluster hub)."""
        with self._lock:
            dst = int(frame.get("dst", -1))
            src = int(frame.get("src", -1))
            if dst not in self._frame_queues:
                return {"error": "invalid dst"}
            self._frame_queues[dst].append(frame)
            self._link_stats["frames_routed"] += 1
            if frame.get("type") == 2:  # PING
                self._link_stats["pings"] += 1
                self._log(f"Link ping: node {src} -> {dst}")
            return {"ok": True, "routed_to": dst}

    def _link_status_unlocked(self) -> dict:
        pending = {str(k): len(v) for k, v in self._frame_queues.items()}
        return {"stats": dict(self._link_stats), "pending": pending}

    def link_status(self) -> dict:
        with self._lock:
            return self._link_status_unlocked()

    def ack_command(self, node_id: int, cmd_id: int) -> dict:
        with self._lock:
            queue = self._commands.get(node_id, [])
            self._commands[node_id] = [c for c in queue if c.cmd_id != cmd_id]
            return {"ok": True}

    def get_commands(self, node_id: int) -> dict:
        with self._lock:
            cmds = self._commands.get(node_id, [])
            return {"commands": [{"cmd_id": c.cmd_id, "action": c.action,
                                  "app_name": c.app_name, "payload": c.payload} for c in cmds]}

    def set_board_gpio(self, node_id: int, template_id: str = "", gpio: dict | None = None) -> dict:
        """Queue board GPIO template or pin map for serial bridge delivery."""
        with self._lock:
            if node_id not in self.nodes:
                return {"error": "unknown node"}
            if template_id and not gpio:
                cmd_id = self._enqueue(node_id, "board_preset", "", {"template_id": template_id})
            else:
                payload = dict(gpio or {})
                if template_id:
                    payload["template_id"] = template_id
                cmd_id = self._enqueue(node_id, "board_gpio", "", payload)
            return {"ok": True, "cmd_id": cmd_id, "node_id": node_id}

    def get_status(self) -> dict:
        with self._lock:
            visible = self._visible_nodes_unlocked()
            visible_ids = {n.id for n in visible}
            online_nodes = [n for n in visible if n.online]
            avg_cpu = sum(n.cpu for n in online_nodes) / max(1, len(online_nodes))
            avg_temp = sum(n.temp_c for n in online_nodes) / max(1, len(online_nodes))
            total_power = sum(n.power_mw for n in online_nodes) / 1000
            agents = len(visible)
            board_mode = len(visible) == 1
            primary = visible[0].to_dict() if board_mode else None
            apps = [
                a.to_dict() for a in self.apps.values()
                if not visible_ids or a.node_id in visible_ids
            ]
            status = {
                "cluster_online": bool(online_nodes),
                "board_mode": board_mode,
                "primary_node": primary,
                "leader_id": self.leader_id,
                "agents_connected": agents,
                "nodes": [n.to_dict() for n in visible],
                "apps": apps,
                "metrics": {
                    "load_percent": round(avg_cpu, 1) if online_nodes else 0,
                    "power_w": round(total_power, 2) if online_nodes else 0,
                    "temp_c": round(avg_temp, 1) if online_nodes else 0,
                    "app_count": len(apps),
                },
                "logs": self.logs[:20],
                "link": self._link_status_unlocked(),
            }
        try:
            from mesh import status_summary  # noqa: E402
            status["mesh"] = status_summary()
        except Exception:
            status["mesh"] = {"enabled": False, "nodes": []}
        try:
            from analytics import record  # noqa: E402
            record(status["metrics"])
        except Exception:
            pass
        try:
            from rules import evaluate_rules  # noqa: E402
            metrics = dict(status["metrics"])
            for app in apps:
                if app.get("metrics"):
                    metrics.update(app["metrics"])
            status["alerts"] = evaluate_rules(metrics)
        except Exception:
            status["alerts"] = []
        return status

    def get_blueprints(self) -> list[dict]:
        return list_blueprints()

    def get_blueprint_detail(self, blueprint_id: str) -> dict:
        bp = get_blueprint(blueprint_id)
        if not bp:
            return {"error": "blueprint not found"}
        return bp

    def create_app_from_blueprint(
        self,
        blueprint_id: str,
        instance_name: str,
        config: dict | None = None,
        node_id: int | None = None,
        auto_start: bool = True,
    ) -> dict:
        bp = get_blueprint(blueprint_id)
        if not bp:
            return {"error": f"unknown blueprint: {blueprint_id}"}
        name = instance_name.strip().replace(" ", "-").lower()
        if not name:
            return {"error": "instance name required"}
        merged, err = validate_config(bp, config or {})
        if err:
            return {"error": err}
        with self._lock:
            if name in self.apps:
                return {"error": f"{name} already exists"}
            nid = node_id if node_id is not None else int(merged.get("target_node", 0))
            visible_ids = self._visible_node_ids_unlocked()
            if visible_ids and nid not in visible_ids:
                nid = next(iter(visible_ids))
                merged["target_node"] = nid
            if nid not in self.nodes:
                return {"error": "no board connected — flash firmware and start the serial bridge"}
            mem = _parse_memory(bp.get("memory", "64KB"))
            runtime = init_runtime(blueprint_id, merged)
            save_config(name, blueprint_id, merged, runtime)
            self.apps[name] = App(
                name=name,
                version=bp.get("version", "1.0.0"),
                node_id=nid,
                state="installed",
                memory_kb=mem,
                priority=bp.get("priority", 5),
                blueprint=blueprint_id,
                display_name=display_name(blueprint_id, merged),
                metrics=runtime,
            )
            self._quotas[name] = {"cpu": 30, "memory_kb": mem}
            payload = {
                "blueprint": blueprint_id,
                "config": merged,
                "version": bp.get("version"),
                "memory_kb": mem,
                "cpu_quota": 30,
            }
            self._log(f"Created {name} from blueprint {blueprint_id} on node {nid}")
            self._enqueue(nid, "install_app", name, payload)
            if auto_start:
                self.apps[name].state = "running"
                self._enqueue(nid, "run_app", name, {"config": merged, "blueprint": blueprint_id})
                self._log(f"Started {name}")
            result = self.apps[name].to_dict()
        self._persist()
        return result

    def get_app_config(self, name: str) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            app = self.apps[name]
            stored = load_config(name)
            if not stored:
                return {"error": "no config for app"}
            bp = get_blueprint(stored.get("blueprint", ""))
            return {
                "app": name,
                "blueprint": stored.get("blueprint"),
                "config": stored.get("config", {}),
                "schema": bp.get("fields", []) if bp else [],
                "metrics": dict(app.metrics),
                "runtime": stored.get("runtime", {}),
            }

    def set_app_config(self, name: str, config: dict) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            app = self.apps[name]
            if not app.blueprint:
                return {"error": "not a blueprint app"}
            bp = get_blueprint(app.blueprint)
            if not bp:
                return {"error": "blueprint missing"}
            merged, err = validate_config(bp, config)
            if err:
                return {"error": err}
            stored = load_config(name) or {}
            runtime = stored.get("runtime") or init_runtime(app.blueprint, merged)
            save_config(name, app.blueprint, merged, runtime)
            new_node = int(merged.get("target_node", app.node_id))
            if new_node != app.node_id and new_node in self.nodes:
                app.node_id = new_node
            app.display_name = display_name(app.blueprint, merged)
            self._log(f"Updated config for {name}")
            self._enqueue(app.node_id, "set_config", name, {
                "blueprint": app.blueprint,
                "config": merged,
            })
            if app.state == "running":
                app.metrics = blueprint_tick(app.blueprint, merged, runtime)
                save_runtime(name, runtime)
            result = {
                "app": name,
                "blueprint": app.blueprint,
                "config": merged,
                "schema": bp.get("fields", []),
                "metrics": dict(app.metrics),
                "runtime": runtime,
            }
        self._persist()
        return result

    def install_app(self, manifest_path: Path) -> dict:
        data = json.loads(manifest_path.read_text())
        name = data["name"]
        binary_path = self._resolve_binary_path(manifest_path, data)
        with self._lock:
            if name in self.apps:
                return {"error": f"{name} already installed"}
            import hashlib
            digest = hashlib.sha256(manifest_path.read_bytes()).hexdigest()
            if data.get("sha256") and data["sha256"] != digest:
                return {"error": "manifest signature mismatch"}
            if binary_path:
                err = _verify_binary(binary_path, data)
                if err:
                    return {"error": err}
            node_id = self._pick_node(data.get("memory", "64KB"))
            mem = _parse_memory(data.get("memory", "64KB"))
            self.apps[name] = App(
                name=name,
                version=data.get("version", "0.1.0"),
                node_id=node_id,
                state="installed",
                memory_kb=mem,
                priority=data.get("priority", 5),
            )
            self._quotas[name] = {"cpu": 30, "memory_kb": mem}
            rel_binary = ""
            if binary_path and binary_path.is_file():
                rel_binary = (
                    str(binary_path.relative_to(ROOT)).replace("\\", "/")
                    if binary_path.is_relative_to(ROOT)
                    else str(binary_path)
                )
            binary_payload = {
                "version": data.get("version"),
                "sha256": digest,
                "binary_sha256": data.get("binary_sha256", ""),
                "binary_path": rel_binary,
                "memory_kb": mem,
                "cpu_quota": 30,
            }
            note = "binary verified" if binary_path else "metadata only"
            self._log(f"Installed {name} v{data.get('version')} on node {node_id} ({note})")
            self._enqueue(node_id, "install_app", name, binary_payload)
            result = self.apps[name].to_dict()
        self._persist()
        return result

    @staticmethod
    def _resolve_binary_path(manifest_path: Path, data: dict) -> Path | None:
        """Locate app binary next to manifest or under examples/…/binaries."""
        name = data["name"]
        manifest_dir = manifest_path.parent
        binary_rel = (data.get("binary_path") or "").strip()
        candidates: list[Path] = []
        if binary_rel:
            rel = Path(binary_rel)
            if rel.is_absolute():
                candidates.append(rel)
            else:
                candidates.append(manifest_dir / rel)
                candidates.append(manifest_dir / rel.name)
                candidates.append(ROOT / rel)
        else:
            candidates.append(manifest_dir / f"{name}.bin")
            candidates.append(manifest_dir / "binaries" / f"{name}.bin")
            candidates.append(ROOT / "examples" / name / "binaries" / f"{name}.bin")
        for path in candidates:
            if path.is_file():
                return path
        return None

    def _pick_node(self, memory: str) -> int:
        mem_kb = _parse_memory(memory)
        best_id = 0
        best_load = float("inf")
        for nid, node in self.nodes.items():
            if not node.online:
                continue
            load = node.cpu + (node.ram_kb / max(1, node.ram_total_kb)) * 50
            if node.ram_total_kb - node.ram_kb >= mem_kb and load < best_load:
                best_load = load
                best_id = nid
        return best_id

    def start_app(self, name: str) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            app = self.apps[name]
            app.state = "running"
            self._log(f"Started {name}")
            payload: dict = {}
            if app.blueprint:
                stored = load_config(name)
                if stored:
                    payload = {"blueprint": app.blueprint, "config": stored.get("config", {})}
            self._enqueue(app.node_id, "run_app", name, payload)
            return app.to_dict()

    def pause_app(self, name: str) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            app = self.apps[name]
            app.state = "paused" if app.state == "running" else "running"
            action = "pause_app" if app.state == "paused" else "run_app"
            self._log(f"{'Paused' if app.state == 'paused' else 'Resumed'} {name}")
            self._enqueue(app.node_id, action, name)
            return app.to_dict()

    def stop_app(self, name: str) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            app = self.apps[name]
            app.state = "stopped"
            app.cpu_percent = 0
            self._log(f"Stopped {name}")
            self._enqueue(app.node_id, "stop_app", name)
            return app.to_dict()

    def migrate_app(self, name: str, dst_node: int) -> dict:
        with self._lock:
            if name not in self.apps:
                return {"error": "not found"}
            if dst_node not in self.nodes:
                return {"error": "invalid node"}
            if not self.nodes[dst_node].online:
                return {"error": "destination offline"}
            app = self.apps[name]
            src = app.node_id
            steps = ["pause", "checkpoint", "transfer", "restore", "resume"]
            prev_state = app.state
            app.state = "paused"
            for step in steps:
                self._log(f"Migrate {name}: [{step}]")
                time.sleep(0.05)
            self._enqueue(src, "checkpoint_app", name)
            cp_path = ROOT / "data" / "checkpoints" / f"{name}.json"
            cp_payload = {}
            if cp_path.exists():
                cp_payload = json.loads(cp_path.read_text())
            self._enqueue(dst_node, "restore_app", name, cp_payload)
            app.node_id = dst_node
            app.state = prev_state if prev_state != "paused" else "running"
            if app.state == "running":
                self._enqueue(dst_node, "run_app", name)
            self._log(f"Migrated {name}: node {src} -> {dst_node}")
            save_checkpoint(name, {"app": app.to_dict(), "steps": steps, "src": src, "dst": dst_node})
            result = {"app": app.to_dict(), "steps": steps}
        self._persist()
        return result


def _parse_memory(s: str) -> int:
    s = s.upper().replace("KB", "").replace("MB", "000").strip()
    try:
        return int(s)
    except ValueError:
        return 64


cluster = ClusterManager()
