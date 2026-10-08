#!/usr/bin/env python3
"""Static regression checks for the 13.60 loader integration point."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
OFFSETS = (ROOT / "source" / "offsets.c").read_text(encoding="utf-8")
BACKEND = (ROOT / "source" / "hv_backend.c").read_text(encoding="utf-8")
BACKEND_H = (ROOT / "include" / "hv_backend.h").read_text(encoding="utf-8")
MAIN = (ROOT / "source" / "main.c").read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


profile = re.search(
    r"offset_list\s+off_1360\s*=\s*\{(.*?)\};",
    OFFSETS,
    re.DOTALL,
)
if not profile:
    fail("off_1360 profile is missing")

body = profile.group(1)

pmap = re.search(r"\.VMSPACE_VM_PMAP\s*=\s*0x([0-9A-Fa-f]+)", body)
if not pmap or int(pmap.group(1), 16) != 0x2E8:
    fail("13.60 VMSPACE_VM_PMAP must remain 0x2e8")

for field in (
    "KERNEL_CODE_CAVE",
    "HV_CODE_CAVE_PA",
    "HV_HANDLE_VMEXIT_PA",
):
    if re.search(rf"\.{field}\s*=", body):
        fail(f"{field} must not be populated without a verified HV backend")

for token in (
    "HV_PROFILE_REQ_VMSPACE_VM_PMAP",
    "HV_PROFILE_REQ_KERNEL_CODE_CAVE",
    "HV_PROFILE_REQ_HV_CODE_CAVE_PA",
    "HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA",
    "hv_profile_validate",
    "hv_backend_missing_requirements",
):
    if token not in BACKEND:
        fail(f"backend validation token missing: {token}")

for token in (
    "hv_backend_context",
    "hv_backend_prepare_fn",
    "HV_BACKEND_INVALID_ARGUMENT",
    "hv_backend_status_name",
):
    if token not in BACKEND_H:
        fail(f"backend API token missing: {token}")

for token in (
    "const hv_backend_context backend_context",
    "hv_backend_prepare(&backend_context)",
    "hv_backend_is_available",
):
    if token not in MAIN:
        fail(f"main backend lifecycle token missing: {token}")

if "prepare = NULL" not in BACKEND:
    fail("unsupported firmware descriptor must not expose a provider")

if "backend_1360.available = true" in BACKEND:
    fail("13.60 backend must remain disabled until verified")

print("PASS: 13.60 profile, backend contract and lifecycle gate are consistent")