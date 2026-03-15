"""Local dev server that serves WASM build and captures console logs to a file.
Start: python serve_with_logging.py
Logs written to: wasm_console.log (in repo root)
Serves files from: build_wasm/dist/
"""
import http.server
import json
import os
import sys
from datetime import datetime, timezone

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SERVE_DIR = os.path.join(SCRIPT_DIR, "build_wasm", "dist")
LOG_FILE = os.path.join(SCRIPT_DIR, "wasm_console.log")
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8090

class LoggingHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=SERVE_DIR, **kwargs)

    def do_POST(self):
        if self.path == "/api/log":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8")
            try:
                data = json.loads(body)
                messages = data.get("messages", [])
            except json.JSONDecodeError:
                messages = [body]
            ts = datetime.now(timezone.utc).strftime("%H:%M:%S.%f")[:-3]
            with open(LOG_FILE, "a", encoding="utf-8") as f:
                for msg in messages:
                    line = f"[{ts}] {msg}\n"
                    f.write(line)
                    sys.stdout.write(line)
                    sys.stdout.flush()
            self.send_response(204)
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
        else:
            self.send_response(404)
            self.end_headers()

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, format, *args):
        # Suppress HTTP access logs to keep stdout clean for app logs
        pass

if __name__ == "__main__":
    with open(LOG_FILE, "w", encoding="utf-8") as f:
        f.write(f"=== Server started at {datetime.now(timezone.utc).isoformat()} ===\n")
    print(f"Serving {SERVE_DIR} on http://localhost:{PORT}")
    print(f"Logs: {LOG_FILE}")
    print("Open http://localhost:8080 in your browser, then click WIZARDS tool.")
    server = http.server.HTTPServer(("", PORT), LoggingHandler)
    server.serve_forever()
