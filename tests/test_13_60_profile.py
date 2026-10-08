#!/usr/bin/env python3
"""Static regression checks for the 13.60 firmware profile and backend gate."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
OFFSETS = (ROOT / "source" / "offsets.c").read_text(encoding="utf-8")
BACKEND = (ROOT / "source" / "hv_backend.c").read_text(encoding="utf-8")
PROFILE = (ROOT / "source" / "hv_profile.c").read_text(encoding="utf-8")
BACKEND_H = (ROOT / "include" / "hv_backend.h").read_text(encoding="utf-8")
MAIN = (ROOT / "source" / "main.c").read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


def profile_body() -> str:
    match = re.search(
        r"offset_list\s+off_1360\s*=\s*\{(.*?)\};",
        OFFSETS,
        re.DOTALL,
    )
    if not match:
        fail("off_1360 profile is missing")
    return match.group(1)


body = profile_body()

pmap = re.search(r"\.VMSPACE_VM_PMAP\s*=\s*0x([0-9A-Fa-f]+)", body)
if not pmap or int(pmap.group(1), 16) != 0x2E8:
    fail("13.60 VMSPACE_VM_PMAP must remain 0x2e8")

known_loader_fields = (
    "VMSPACE_VM_PMAP",
    "KERNEL_CODE_CAVE",
    "HV_CODE_CAVE_PA",
    "HV_HANDLE_VMEXIT_PA",
)

for field in known_loader_fields:
    assignments = re.findall(rf"\.{field}\s*=", body)
    if field != "VMSPACE_VM_PMAP" and assignments:
        fail(f"{field} must remain unset in the 13.60 profile")

for field in (
    "ACPIGBL_FACS",
    "IDT",
    "COMMON_TSS",
    "FUN_MEMCPY",
    "GAD_POP_RAX_RET",
    "GAD_POP_RDI_RET",
    "GAD_POP_RSI_RET",
    "GAD_POP_RDX_RET",
    "GAD_POP_RCX_RET",
    "GAD_WRMSR_RET",
):
    if re.search(rf"\.{field}\s*=", body):
        fail(f"{field} must not be populated in off_1360")

required_tokens = (
    "HV_PROFILE_REQ_VMSPACE_VM_PMAP",
    "HV_PROFILE_REQ_KERNEL_CODE_CAVE",
    "HV_PROFILE_REQ_HV_CODE_CAVE_PA",
    "HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA",
    "hv_profile_validate",
    "hv_backend_missing_requirements",
)

for token in required_tokens:
    if token not in BACKEND:
        fail(f"backend validation token missing: {token}")

for token in (
    "HV_PROFILE_REQ_VMSPACE_VM_PMAP",
    "HV_PROFILE_REQ_KERNEL_CODE_CAVE",
    "HV_PROFILE_REQ_HV_CODE_CAVE_PA",
    "HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA",
):
    if token not in PROFILE:
        fail(f"profile requirement mapping missing: {token}")

for token in (
    "hv_backend_context",
    "hv_backend_prepare_fn",
    "HV_BACKEND_INVALID_ARGUMENT",
    "hv_backend_status_name",
):
    if token not in BACKEND_H:
        fail(f"backend API token missing: {token}")

if "backend_1360.available = true" in BACKEND:
    fail("13.60 backend must remain disabled until verified")

if "prepare = NULL" not in BACKEND:
    fail("unsupported firmware descriptor must not expose a provider")

if "if (fw == 0x1360)" not in MAIN:
    fail("main must have an explicit 13.60 gate")

diagnostic_pos = MAIN.index("if (fw == 0x1360)")
return_pos = MAIN.index("return 0;", diagnostic_pos)
if return_pos < 0:
    fail("13.60 gate must have an explicit terminating return")

prefix = MAIN[diagnostic_pos:return_pos]
for forbidden in (
    "fetch_linux(&linux_i)",
    "prepare_resume(",
    "hv_backend_prepare(",
    "enter_rest_mode()",
):
    if forbidden in prefix:
        fail(f"13.60 path reaches forbidden stage before return: {forbidden}")

print("PASS: 13.60 profile, capability mapping and lifecycle gate are consistent")
