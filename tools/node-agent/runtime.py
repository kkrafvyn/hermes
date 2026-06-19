"""Local app runtime on node agents — binary deploy, quotas, checkpoint I/O."""

from __future__ import annotations

import hashlib
import json
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
FLASH_DIR = ROOT / "data" / "flash"
CHECKPOINT_DIR = ROOT / "data" / "checkpoints"

sys.path.insert(0, str(ROOT / "tools" / "control-plane"))
from blueprint_engine import init_runtime, tick as blueprint_tick  # noqa: E402


@dataclass
class LocalApp:
    name: str
    state: str = "stopped"
    cpu_percent: float = 0.0
    memory_kb: int = 64
    binary_path: str = ""
    blueprint: str = ""
    config: dict = field(default_factory=dict)
    app_state: dict = field(default_factory=dict)


@dataclass
class NodeRuntime:
    node_id: int
    apps: dict[str, LocalApp] = field(default_factory=dict)
    base_cpu: float = 10.0
    quotas: dict[str, dict] = field(default_factory=dict)

    def __post_init__(self) -> None:
        FLASH_DIR.mkdir(parents=True, exist_ok=True)
        CHECKPOINT_DIR.mkdir(parents=True, exist_ok=True)

    def _enforce_quota(self, app_name: str) -> None:
        app = self.apps.get(app_name)
        if not app or app.state != "running":
            return
        q = self.quotas.get(app_name, {"cpu": 30, "memory_kb": 64})
        if app.cpu_percent > q["cpu"]:
            app.state = "paused"
            app.cpu_percent = 0

    def handle(self, action: str, app_name: str, payload: dict | None = None) -> str:
        payload = payload or {}
        if action == "install_app":
            blueprint = payload.get("blueprint", "")
            config = payload.get("config", {})
            app = LocalApp(
                name=app_name,
                state="installed",
                memory_kb=payload.get("memory_kb", 64),
                blueprint=blueprint,
                config=config,
            )
            if blueprint:
                app.app_state = init_runtime(blueprint, config)
            binary_rel = payload.get("binary_path", "")
            binary_path = ROOT / binary_rel if binary_rel else None
            if binary_path and binary_path.exists():
                dest = FLASH_DIR / f"node{self.node_id}_{app_name}.bin"
                dest.write_bytes(binary_path.read_bytes())
                app.binary_path = str(dest)
                expected = payload.get("binary_sha256", "")
                if expected:
                    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
                    if digest != expected:
                        return f"binary verify failed for {app_name}"
            self.apps[app_name] = app
            self.quotas[app_name] = {
                "cpu": payload.get("cpu_quota", 30),
                "memory_kb": payload.get("memory_kb", app.memory_kb),
            }
            if blueprint:
                return f"installed {app_name} (blueprint {blueprint})"
            return f"installed {app_name} (binary flash OK)"
        if action == "set_config":
            app = self.apps.get(app_name)
            if not app:
                return f"set_config failed: {app_name} not found"
            config = payload.get("config", {})
            app.config = config
            if payload.get("blueprint"):
                app.blueprint = payload["blueprint"]
            if app.blueprint and app.state == "running":
                app.app_state = blueprint_tick(app.blueprint, app.config, app.app_state)
            return f"config updated for {app_name}"
        if action == "run_app":
            app = self.apps.setdefault(app_name, LocalApp(name=app_name))
            if payload.get("config"):
                app.config = payload["config"]
            if payload.get("blueprint"):
                app.blueprint = payload["blueprint"]
            if app.blueprint and not app.app_state:
                app.app_state = init_runtime(app.blueprint, app.config)
            app.state = "running"
            app.cpu_percent = min(8.0, self.quotas.get(app_name, {}).get("cpu", 30))
            self._enforce_quota(app_name)
            return f"running {app_name}"
        if action == "pause_app":
            if app_name in self.apps:
                self.apps[app_name].state = "paused"
                self.apps[app_name].cpu_percent = 0
            return f"paused {app_name}"
        if action == "stop_app":
            if app_name in self.apps:
                self.apps[app_name].state = "stopped"
                self.apps[app_name].cpu_percent = 0
            return f"stopped {app_name}"
        if action == "checkpoint_app":
            app = self.apps.get(app_name)
            if not app:
                return f"checkpoint failed: {app_name} not found"
            cp = {
                "app_id": app_name,
                "state": app.state,
                "cpu_percent": app.cpu_percent,
                "app_state": app.app_state,
                "node_id": self.node_id,
            }
            path = CHECKPOINT_DIR / f"{app_name}.json"
            path.write_text(json.dumps(cp, indent=2) + "\n")
            return f"checkpoint {app_name} -> {path.name}"
        if action == "restore_app":
            if payload and payload.get("app_state") is not None:
                app = self.apps.setdefault(app_name, LocalApp(name=app_name))
                app.app_state = payload.get("app_state", {})
                app.state = payload.get("state", "running")
                app.cpu_percent = payload.get("cpu_percent", 5.0)
                return f"restore {app_name} from migration payload"
            path = CHECKPOINT_DIR / f"{app_name}.json"
            if not path.exists():
                return f"restore failed: no checkpoint for {app_name}"
            cp = json.loads(path.read_text())
            app = self.apps.setdefault(app_name, LocalApp(name=app_name))
            app.state = cp.get("state", "running")
            app.cpu_percent = cp.get("cpu_percent", 5.0)
            app.app_state = cp.get("app_state", {})
            return f"restore {app_name} from checkpoint"
        return f"unknown {action}"

    def total_cpu(self) -> float:
        total = self.base_cpu
        for app in self.apps.values():
            if app.state == "running":
                self._enforce_quota(app.name)
                total += app.cpu_percent
        return total

    def running_apps(self) -> list[str]:
        return [a.name for a in self.apps.values() if a.state == "running"]
