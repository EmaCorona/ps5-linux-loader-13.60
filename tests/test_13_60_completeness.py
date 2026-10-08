#!/usr/bin/env python3
"""Cross-file completeness checks for the 13.60-only implementation.

These tests are intentionally static: they do not execute Hypervisor code and
cannot prove hardware safety. They guard lifecycle ordering, profile isolation,
legacy-backend separation, test wiring and documentation invariants.
"""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


offsets = read("source/offsets.c")
backend = read("source/hv_backend.c")
profile = read("source/hv_profile.c")
provider = read("source/hv_provider_1360.c")
provider_h = read("include/hv_provider_1360.h")
diagnostic = read("source/diagnostic_1360.c")
diagnostic_h = read("include/diagnostic_1360.h")
main = read("source/main.c")
makefile = read("Makefile")
workflow = read(".github/workflows/13-60-validation.yml")
build_workflow = read(".github/workflows/13-60-build.yml")
port_doc = read("docs/PORT_13_60.md")
build_doc = read("docs/BUILD_13_60.md")
contract_doc = read("docs/HV_PROVIDER_13_60.md")


def extract_profile_body(source: str) -> str:
    match = re.search(
        r"offset_list\s+off_1360\s*=\s*\{(.*?)\};",
        source,
        re.DOTALL,
    )
    if not match:
        fail("off_1360 profile is missing")
    return match.group(1)


profile_body = extract_profile_body(offsets)

assignments = re.findall(r"\.([A-Za-z0-9_]+)\s*=", profile_body)
if assignments != ["VMSPACE_VM_PMAP"]:
    fail(f"13.60 profile unexpectedly defines fields: {assignments}")

if not re.search(r"\.VMSPACE_VM_PMAP\s*=\s*0x2E8\b", profile_body, re.IGNORECASE):
    fail("13.60 VMSPACE_VM_PMAP drifted from 0x2e8")

for field in (
    "KERNEL_CODE_CAVE",
    "HV_CODE_CAVE_PA",
    "HV_HANDLE_VMEXIT_PA",
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
    if re.search(rf"\.{field}\s*=", profile_body):
        fail(f"{field} must not be populated in off_1360")

requirements = (
    "HV_PROFILE_REQ_VMSPACE_VM_PMAP",
    "HV_PROFILE_REQ_KERNEL_CODE_CAVE",
    "HV_PROFILE_REQ_HV_CODE_CAVE_PA",
    "HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA",
)
requirement_expr = re.search(
    r"HV_PROFILE_1360_REQUIREMENTS\s*=([\s\S]*?);",
    backend,
)
if not requirement_expr:
    fail("13.60 backend requirement mask is missing")

for token in requirements:
    if token not in requirement_expr.group(1):
        fail(f"13.60 backend does not require {token}")
    if token not in profile:
        fail(f"profile mapping for {token} is missing")

if ".available = false" not in backend:
    fail("13.60 backend must remain disabled")

if "hv_provider_1360_prepare(context)" not in backend:
    fail("13.60 dispatcher must use the dedicated provider")

for legacy in ("hv_defeat_0304", "hv_defeat_0506", "hv_defeat_0607"):
    if legacy in provider or legacy in provider_h:
        fail(f"13.60 provider references legacy backend {legacy}")

for token in (
    "context->firmware != FW_1360",
    "context->kernel_text == 0",
    "context->kernel_data == 0",
    "context->dmap_base == 0",
    "context->shellcode_kernel == NULL",
    "context->shellcode_kernel_len == 0",
    "context->linux_info_va == 0",
):
    if token not in provider:
        fail(f"13.60 provider precondition missing: {token}")

if "return HV_BACKEND_INVALID_ARGUMENT;" not in provider:
    fail("13.60 provider must reject malformed contexts")

if "return HV_BACKEND_UNAVAILABLE;" not in provider:
    fail("13.60 provider must fail closed")

for token in (
    "run_1360_diagnostic",
    "is_kernel_pointer",
    "is_physical_address",
    "is_kernel_pmap_valid",
    "DIAGNOSTIC_MARKER_A",
    "DIAGNOSTIC_MARKER_B",
    "scratch restoration verification failed",
):
    if token not in diagnostic or token not in diagnostic_h:
        fail(f"diagnostic contract token missing: {token}")

for token in (
    "hv_backend_prepare",
    "hv_provider_1360",
    "enter_rest_mode",
    "sceKernelNotifySystemSuspendStart",
):
    if token in diagnostic:
        fail(f"diagnostic must not execute {token}")

gate_start = main.find("if (fw == 0x1360)")
if gate_start < 0:
    fail("13.60 lifecycle gate is missing")

gate_end = main.find("return 0;", gate_start)
if gate_end < 0:
    fail("13.60 lifecycle gate has no terminating return")

gate = main[gate_start:gate_end]
for token in (
    "fetch_linux(&linux_i)",
    "prepare_resume(",
    "hv_backend_prepare(",
    "enter_rest_mode()",
):
    if token in gate:
        fail(f"13.60 path reaches forbidden stage before return: {token}")

if main.count("if (fw == 0x1360)") != 1:
    fail("main must have exactly one 13.60 lifecycle gate")

for required in (
    "tests/test_13_60_profile.py",
    "tests/test_13_60_provider.py",
    "tests/test_13_60_diagnostic.py",
    "tests/test_13_60_completeness.py",
):
    if required not in makefile:
        fail(f"Makefile missing {required}")

if "verify-13-60" not in makefile:
    fail("Makefile must expose a dedicated verify-13-60 target")

if "tests/test_13_60_completeness.py" not in workflow or "make test" not in workflow:
    fail("13.60 validation workflow must execute the complete test suite")

for required in (
    "make test",
    "make -j2",
    "readelf -h bin/ps5-linux-loader.elf",
):
    if required not in build_workflow:
        fail(f"13.60 build workflow missing {required}")

for doc, name in (
    (port_doc, "PORT_13_60.md"),
    (build_doc, "BUILD_13_60.md"),
    (contract_doc, "HV_PROVIDER_13_60.md"),
):
    if not any(
        phrase in doc
        for phrase in (
            "remains pending",
            "remains disabled",
            "integration is pending",
        )
    ):
        fail(f"{name} must state that 13.60 Linux/HV enablement is gated")

for marker in (
    ".HV_CODE_CAVE_PA =",
    ".HV_HANDLE_VMEXIT_PA =",
    ".KERNEL_CODE_CAVE =",
):
    if marker in profile_body:
        fail(f"13.60 profile contains an activation marker: {marker}")

if "backend_1360.available = true" in backend:
    fail("13.60 backend must not be enabled by source text")

print("PASS: 13.60 cross-file invariants, lifecycle ordering, isolation and test wiring are consistent")
