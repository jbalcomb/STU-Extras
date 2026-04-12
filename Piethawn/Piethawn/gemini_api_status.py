#!/usr/bin/env python3
"""Check the status of the backend API used by the local Gemini CLI setup."""

from __future__ import annotations

import argparse
import http.client
import json
import os
import shutil
import socket
import ssl
import subprocess
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional


USER_GEMINI_DIR = Path.home() / ".gemini"
USER_SETTINGS_PATH = USER_GEMINI_DIR / "settings.json"


@dataclass(frozen=True)
class EndpointProbe:
    auth_type: str
    transport_name: str
    host: str
    port: int
    method: str
    path: str
    request_body: Optional[str]
    headers: Dict[str, str]
    notes: List[str]


@dataclass(frozen=True)
class CheckResult:
    ok: bool
    label: str
    detail: str
    extra: Optional[Dict[str, Any]] = None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check the backend API host and request path used by the current Gemini CLI authentication mode.",
    )
    parser.add_argument("--timeout", type=float, default=10.0, help="Socket and HTTP timeout in seconds. Default: 10")
    parser.add_argument("--json", action="store_true", help="Print machine-readable JSON output.")
    parser.add_argument(
        "--auth-type",
        default=None,
        help="Override the detected Gemini auth type for the probe, for example oauth-personal or gemini-api-key.",
    )
    return parser.parse_args()


def load_json_file(path: Path) -> Dict[str, Any]:
    if not path.exists():
        return {}
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}


def normalize_auth_type(value: Optional[str]) -> Optional[str]:
    if not value:
        return None

    normalized = value.strip().lower().replace("_", "-")
    aliases = {
        "google": "oauth-personal",
        "login-with-google": "oauth-personal",
        "loginwithgoogle": "oauth-personal",
        "oauth": "oauth-personal",
        "oauth-personal": "oauth-personal",
        "gemini-api-key": "gemini-api-key",
        "use-gemini": "gemini-api-key",
        "api-key": "gemini-api-key",
        "vertex-ai": "vertex-ai",
        "use-vertex-ai": "vertex-ai",
        "gateway": "gateway",
    }
    return aliases.get(normalized, normalized)


def env_flag(name: str) -> bool:
    value = os.environ.get(name, "")
    return value.strip().lower() in {"1", "true", "yes", "on"}


def detect_effective_auth_type(user_settings: Dict[str, Any], override: Optional[str]) -> tuple[Optional[str], str, Optional[str]]:
    if override:
        return normalize_auth_type(override), "--auth-type", override

    if env_flag("GOOGLE_GENAI_USE_GCA"):
        return "oauth-personal", "environment", os.environ.get("GOOGLE_GENAI_USE_GCA")

    if env_flag("GOOGLE_GENAI_USE_VERTEXAI"):
        return "vertex-ai", "environment", os.environ.get("GOOGLE_GENAI_USE_VERTEXAI")

    if os.environ.get("GEMINI_API_KEY"):
        return "gemini-api-key", "environment", "GEMINI_API_KEY"

    if os.environ.get("GOOGLE_API_KEY") and os.environ.get("GOOGLE_CLOUD_PROJECT") and os.environ.get("GOOGLE_CLOUD_LOCATION"):
        return "vertex-ai", "environment", "GOOGLE_API_KEY"

    selected = (
        user_settings.get("security", {})
        .get("auth", {})
        .get("selectedType")
    )
    return normalize_auth_type(selected), "user-settings", selected


def build_endpoint_probe(auth_type: Optional[str]) -> EndpointProbe:
    if auth_type == "oauth-personal":
        return EndpointProbe(
            auth_type=auth_type,
            transport_name="Google Code Assist",
            host="cloudcode-pa.googleapis.com",
            port=443,
            method="POST",
            path="/v1internal:streamGenerateContent?alt=sse",
            request_body="{}",
            headers={"Content-Type": "application/json", "User-Agent": "Piethawn-GeminiApiStatus/1.0"},
            notes=[
                "This is the Code Assist endpoint used by Gemini CLI when auth type is oauth-personal.",
                "An unauthenticated probe should still return an HTTP error if the service is reachable.",
            ],
        )

    if auth_type == "gemini-api-key":
        return EndpointProbe(
            auth_type=auth_type,
            transport_name="Gemini Developer API",
            host="generativelanguage.googleapis.com",
            port=443,
            method="GET",
            path="/v1beta/models",
            request_body=None,
            headers={"User-Agent": "Piethawn-GeminiApiStatus/1.0"},
            notes=[
                "This is the public Gemini Developer API host used for API key auth.",
                "A missing or invalid API key is acceptable for this status probe; any HTTP response means the host is reachable.",
            ],
        )

    if auth_type == "vertex-ai":
        project = os.environ.get("GOOGLE_CLOUD_PROJECT") or os.environ.get("GOOGLE_CLOUD_PROJECT_ID") or "PROJECT"
        location = os.environ.get("GOOGLE_CLOUD_LOCATION") or "us-central1"
        return EndpointProbe(
            auth_type=auth_type,
            transport_name="Vertex AI Gemini API",
            host="aiplatform.googleapis.com",
            port=443,
            method="GET",
            path=f"/v1/projects/{project}/locations/{location}/publishers/google/models",
            request_body=None,
            headers={"User-Agent": "Piethawn-GeminiApiStatus/1.0"},
            notes=[
                "This is the Vertex AI host used when Gemini CLI is configured for Vertex auth.",
                "If project or location is not configured, the path is still usable as a reachability probe.",
            ],
        )

    return EndpointProbe(
        auth_type=auth_type or "unknown",
        transport_name="Unknown backend",
        host="cloudcode-pa.googleapis.com",
        port=443,
        method="GET",
        path="/",
        request_body=None,
        headers={"User-Agent": "Piethawn-GeminiApiStatus/1.0"},
        notes=[
            "Auth type could not be mapped cleanly, so this probe falls back to the Code Assist host.",
        ],
    )


def get_gemini_cli_version() -> Optional[str]:
    executable = shutil.which("gemini")
    if not executable:
        return None
    try:
        completed = subprocess.run(
            [executable, "--version"],
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
        )
    except (OSError, subprocess.SubprocessError):
        return None

    output = (completed.stdout or completed.stderr or "").strip()
    return output or None


def probe_dns(host: str) -> CheckResult:
    started = time.perf_counter()
    try:
        results = socket.getaddrinfo(host, 443, type=socket.SOCK_STREAM)
    except OSError as exc:
        return CheckResult(False, "DNS", str(exc))

    unique_addresses = sorted({item[4][0] for item in results})
    elapsed_ms = int((time.perf_counter() - started) * 1000)
    return CheckResult(
        True,
        "DNS",
        f"Resolved {host} in {elapsed_ms} ms.",
        {"addresses": unique_addresses},
    )


def probe_tcp(host: str, port: int, timeout: float) -> CheckResult:
    started = time.perf_counter()
    try:
        with socket.create_connection((host, port), timeout=timeout) as sock:
            peer = sock.getpeername()
    except OSError as exc:
        return CheckResult(False, "TCP", str(exc))

    elapsed_ms = int((time.perf_counter() - started) * 1000)
    return CheckResult(
        True,
        "TCP",
        f"Connected to {host}:{port} in {elapsed_ms} ms.",
        {"peer": [peer[0], peer[1]]},
    )


def probe_tls(host: str, port: int, timeout: float) -> CheckResult:
    context = ssl.create_default_context()
    started = time.perf_counter()
    try:
        with socket.create_connection((host, port), timeout=timeout) as raw_sock:
            with context.wrap_socket(raw_sock, server_hostname=host) as tls_sock:
                certificate = tls_sock.getpeercert()
                cipher = tls_sock.cipher()
                version = tls_sock.version()
    except OSError as exc:
        return CheckResult(False, "TLS", str(exc))

    elapsed_ms = int((time.perf_counter() - started) * 1000)
    subject_parts = certificate.get("subject", ())
    subject = ", ".join("=".join(part) for group in subject_parts for part in group)
    return CheckResult(
        True,
        "TLS",
        f"Negotiated {version} with {host} in {elapsed_ms} ms.",
        {
            "cipher": cipher[0] if cipher else None,
            "subject": subject,
        },
    )


def probe_http(endpoint: EndpointProbe, timeout: float) -> CheckResult:
    context = ssl.create_default_context()
    connection = http.client.HTTPSConnection(endpoint.host, endpoint.port, timeout=timeout, context=context)
    started = time.perf_counter()
    try:
        connection.request(
            endpoint.method,
            endpoint.path,
            body=endpoint.request_body,
            headers=endpoint.headers,
        )
        response = connection.getresponse()
        body = response.read(512)
    except OSError as exc:
        return CheckResult(False, "HTTP", str(exc))
    finally:
        connection.close()

    elapsed_ms = int((time.perf_counter() - started) * 1000)
    body_preview = body.decode("utf-8", errors="replace").strip()
    body_preview = body_preview[:240]
    status_ok = response.status < 500
    return CheckResult(
        status_ok,
        "HTTP",
        f"{endpoint.method} {endpoint.path} returned {response.status} {response.reason} in {elapsed_ms} ms.",
        {
            "status": response.status,
            "reason": response.reason,
            "body_preview": body_preview,
            "headers": dict(response.getheaders()),
        },
    )


def results_to_exit_code(results: List[CheckResult]) -> int:
    for result in results:
        if not result.ok:
            return 1
    return 0


def build_output_document(
    effective_auth_type: Optional[str],
    auth_source: str,
    auth_source_value: Optional[str],
    endpoint: EndpointProbe,
    gemini_version: Optional[str],
    results: List[CheckResult],
) -> Dict[str, Any]:
    return {
        "effective_auth_type": effective_auth_type,
        "auth_source": auth_source,
        "auth_source_value": auth_source_value,
        "gemini_cli_version": gemini_version,
        "endpoint": asdict(endpoint),
        "results": [asdict(result) for result in results],
        "ok": all(result.ok for result in results),
    }


def print_text_report(document: Dict[str, Any]) -> None:
    print("Gemini CLI API status")
    print(f"Auth type:      {document['effective_auth_type'] or 'unknown'}")
    print(f"Auth source:    {document['auth_source']}")
    if document["auth_source_value"]:
        print(f"Source value:   {document['auth_source_value']}")
    if document["gemini_cli_version"]:
        print(f"Gemini CLI:     {document['gemini_cli_version']}")

    endpoint = document["endpoint"]
    print(f"Transport:      {endpoint['transport_name']}")
    print(f"Host:           {endpoint['host']}:{endpoint['port']}")
    print(f"Probe request:  {endpoint['method']} {endpoint['path']}")
    for note in endpoint["notes"]:
        print(f"Note:           {note}")

    print("")
    for result in document["results"]:
        status = "OK" if result["ok"] else "FAIL"
        print(f"[{status}] {result['label']}: {result['detail']}")
        extra = result.get("extra") or {}
        if result["label"] == "DNS" and extra.get("addresses"):
            print(f"      Addresses: {', '.join(extra['addresses'])}")
        elif result["label"] == "TCP" and extra.get("peer"):
            print(f"      Peer:      {extra['peer'][0]}:{extra['peer'][1]}")
        elif result["label"] == "TLS":
            if extra.get("cipher"):
                print(f"      Cipher:    {extra['cipher']}")
            if extra.get("subject"):
                print(f"      Subject:   {extra['subject']}")
        elif result["label"] == "HTTP":
            if extra.get("body_preview"):
                print(f"      Body:      {extra['body_preview']}")


def main() -> int:
    args = parse_args()
    user_settings = load_json_file(USER_SETTINGS_PATH)
    effective_auth_type, auth_source, auth_source_value = detect_effective_auth_type(user_settings, args.auth_type)
    endpoint = build_endpoint_probe(effective_auth_type)
    gemini_version = get_gemini_cli_version()

    results = [
        probe_dns(endpoint.host),
        probe_tcp(endpoint.host, endpoint.port, args.timeout),
        probe_tls(endpoint.host, endpoint.port, args.timeout),
        probe_http(endpoint, args.timeout),
    ]

    document = build_output_document(
        effective_auth_type,
        auth_source,
        auth_source_value,
        endpoint,
        gemini_version,
        results,
    )

    if args.json:
        print(json.dumps(document, indent=2, sort_keys=True))
    else:
        print_text_report(document)

    return results_to_exit_code(results)


if __name__ == "__main__":
    raise SystemExit(main())