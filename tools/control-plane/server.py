#!/usr/bin/env python3

"""HERMES Control Plane API — REST + WebSocket + auth."""



from __future__ import annotations



import json

import sys

import threading

import time

from http.server import HTTPServer, BaseHTTPRequestHandler

from pathlib import Path

from urllib.parse import urlparse



ROOT = Path(__file__).resolve().parent.parent.parent

sys.path.insert(0, str(Path(__file__).resolve().parent))



from hermes_cluster import cluster  # noqa: E402

from auth import validate_token, extract_token, issue_token  # noqa: E402

from analytics import query as analytics_query  # noqa: E402

import websocket  # noqa: E402



PORT = 8787

DASHBOARD_DIR = ROOT / "dashboard"

PUBLIC_PATHS = {"/api/auth/login", "/api/status"}





class Handler(BaseHTTPRequestHandler):

    def _cors(self) -> None:

        self.send_header("Access-Control-Allow-Origin", "*")

        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")

        self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Hermes-Token")



    def _authorized(self) -> bool:

        path = urlparse(self.path).path

        if path in PUBLIC_PATHS or not path.startswith("/api/"):

            return True

        return validate_token(extract_token(self.headers))



    def _json(self, code: int, data: dict) -> None:

        body = json.dumps(data).encode()

        self.send_response(code)

        self.send_header("Content-Type", "application/json")

        self._cors()

        self.send_header("Content-Length", str(len(body)))

        self.end_headers()

        self.wfile.write(body)



    def _read_json(self) -> dict:

        length = int(self.headers.get("Content-Length", 0))

        if length == 0:

            return {}

        return json.loads(self.rfile.read(length))



    def do_OPTIONS(self) -> None:

        self.send_response(204)

        self._cors()

        self.end_headers()



    def do_GET(self) -> None:

        path = urlparse(self.path).path



        if path in ("/", "/index.html"):

            return self._serve_file(DASHBOARD_DIR / "index.html", "text/html")

        if path == "/style.css":

            return self._serve_file(DASHBOARD_DIR / "style.css", "text/css")

        if path == "/app.js":

            return self._serve_file(DASHBOARD_DIR / "app.js", "application/javascript")



        if not self._authorized():

            return self._json(401, {"error": "unauthorized"})



        if path == "/api/status":

            return self._json(200, cluster.get_status())

        if path == "/api/nodes":

            return self._json(200, {"nodes": cluster.get_status()["nodes"]})

        if path == "/api/apps":

            return self._json(200, {"apps": cluster.get_status()["apps"]})

        if path == "/api/logs":

            return self._json(200, {"logs": cluster.logs[:50]})

        if path.startswith("/api/nodes/") and path.endswith("/commands"):

            node_id = int(path.split("/")[3])

            return self._json(200, cluster.get_commands(node_id))

        if path == "/api/link/status":

            return self._json(200, cluster.link_status())

        if path == "/api/analytics":

            return self._json(200, analytics_query())

        if path == "/api/mcus":

            sys.path.insert(0, str(ROOT / "scripts"))
            from mcu_registry import mcu_summary  # noqa: E402
            return self._json(200, mcu_summary())

        if path == "/api/deploy/approaches":

            from mcu_deploy import deploy_approaches_summary  # noqa: E402
            return self._json(200, deploy_approaches_summary())

        if path.startswith("/api/mcus/") and path.endswith("/deploy"):

            sys.path.insert(0, str(ROOT / "scripts"))
            from mcu_registry import all_targets  # noqa: E402
            from mcu_deploy import deploy_plan_detail  # noqa: E402
            tid = path.split("/")[3]
            if tid not in all_targets():
                return self._json(404, {"error": f"unknown mcu: {tid}"})
            result = deploy_plan_detail(tid)
            return self._json(200, result)

        if path == "/api/sensors":

            from sensors import catalog_summary  # noqa: E402

            return self._json(200, catalog_summary())

        if path == "/api/blueprints":

            return self._json(200, {"blueprints": cluster.get_blueprints()})

        if path.startswith("/api/blueprints/"):

            bp_id = path.split("/")[-1]

            result = cluster.get_blueprint_detail(bp_id)

            code = 404 if "error" in result else 200

            return self._json(code, result)

        if path.startswith("/api/apps/") and path.endswith("/config"):

            name = path.split("/")[3]

            result = cluster.get_app_config(name)

            code = 400 if "error" in result else 200

            return self._json(code, result)

        if path == "/api/events":

            return self._sse_stream()

        if path == "/ws":

            return self._ws_upgrade()



        self._json(404, {"error": "not found"})



    def do_POST(self) -> None:

        path = urlparse(self.path).path

        body = self._read_json()



        if path == "/api/auth/login":

            token = issue_token(body.get("username", "admin"))

            return self._json(200, {"token": token})



        if not self._authorized():

            return self._json(401, {"error": "unauthorized"})



        if path == "/api/nodes/register":

            return self._json(200, cluster.register_node(

                int(body.get("node_id", 0)),

                body.get("name", "unknown"),

                body.get("node_type", "unknown"),

                bool(body.get("agent", False)),

            ))



        if path == "/api/nodes/heartbeat":

            telemetry = body.get("telemetry")

            return self._json(200, cluster.heartbeat(int(body.get("node_id", 0)), telemetry))



        if path == "/api/nodes/ack":

            return self._json(200, cluster.ack_command(

                int(body.get("node_id", 0)),

                int(body.get("cmd_id", 0)),

            ))



        if path == "/api/link/send":

            return self._json(200, cluster.link_send(body.get("frame", body)))



        if path == "/api/apps/create":

            result = cluster.create_app_from_blueprint(

                body.get("blueprint", ""),

                body.get("name", ""),

                body.get("config"),

                int(body["node_id"]) if body.get("node_id") is not None else None,

                bool(body.get("auto_start", True)),

            )

            code = 400 if "error" in result else 200

            return self._json(code, result)



        if path == "/api/apps/install":

            manifest = body.get("manifest_path")

            if not manifest:

                return self._json(400, {"error": "manifest_path required"})

            p = Path(manifest)

            if not p.is_absolute():

                p = ROOT / p

            if not p.exists():

                return self._json(404, {"error": f"manifest not found: {p}"})

            result = cluster.install_app(p)

            code = 400 if "error" in result else 200

            return self._json(code, result)



        if path.startswith("/api/apps/"):

            parts = path.strip("/").split("/")

            if len(parts) == 4:

                name, action = parts[2], parts[3]

                handlers = {

                    "start": cluster.start_app,

                    "pause": cluster.pause_app,

                    "stop": cluster.stop_app,

                }

                if action == "migrate":

                    dst = int(body.get("dst_node", (cluster.apps[name].node_id + 1) % 3))

                    return self._json(200, cluster.migrate_app(name, dst))

                if action == "config":

                    result = cluster.set_app_config(name, body.get("config", body))

                    code = 400 if "error" in result else 200

                    return self._json(code, result)

                if action in handlers:

                    result = handlers[action](name)

                    code = 400 if "error" in result else 200

                    return self._json(code, result)



        self._json(404, {"error": "not found"})



    def _ws_upgrade(self) -> None:

        key = self.headers.get("Sec-WebSocket-Key")

        if not key:

            return self._json(400, {"error": "missing Sec-WebSocket-Key"})

        accept = websocket.accept_key(key)

        self.send_response(101)

        self.send_header("Upgrade", "websocket")

        self.send_header("Connection", "Upgrade")

        self.send_header("Sec-WebSocket-Accept", accept)

        self.end_headers()

        websocket.register(self)

        try:

            while True:

                time.sleep(30)

        except Exception:

            pass

        finally:

            websocket.unregister(self)



    def _sse_stream(self) -> None:

        self.send_response(200)

        self.send_header("Content-Type", "text/event-stream")

        self.send_header("Cache-Control", "no-cache")

        self._cors()

        self.end_headers()

        try:

            for _ in range(30):

                data = json.dumps(cluster.get_status())

                self.wfile.write(f"data: {data}\n\n".encode())

                self.wfile.flush()

                time.sleep(2)

        except (BrokenPipeError, ConnectionResetError):

            pass



    def _serve_file(self, path: Path, content_type: str) -> None:

        if not path.exists():

            return self._json(404, {"error": "not found"})

        data = path.read_bytes()

        self.send_response(200)

        self.send_header("Content-Type", content_type)

        self.end_headers()

        self.wfile.write(data)



    def log_message(self, fmt: str, *args) -> None:

        if args and str(args[0]).startswith("GET /api"):

            sys.stderr.write(f"{self.address_string()} - {fmt % args}\n")





def main() -> None:

    threading.Thread(

        target=websocket.pump_status,

        args=(cluster.get_status,),

        daemon=True,

    ).start()

    server = HTTPServer(("0.0.0.0", PORT), Handler)

    print(f"HERMES Control Plane running at http://localhost:{PORT}")

    print(f"  Dashboard: http://localhost:{PORT}/")

    print(f"  WebSocket: ws://localhost:{PORT}/ws")

    print(f"  Auth: POST /api/auth/login or HERMES_TOKEN env")

    try:

        server.serve_forever()

    except KeyboardInterrupt:

        print("\nShutting down.")

        server.server_close()





if __name__ == "__main__":

    main()


