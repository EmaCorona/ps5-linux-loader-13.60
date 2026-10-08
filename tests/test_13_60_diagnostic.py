#!/usr/bin/env python3
"""Static checks for the executable 13.60 diagnostic route."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "source" / "main.c").read_text(encoding="utf-8")
HEADER = (ROOT / "include" / "diagnostic_1360.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "source" / "diagnostic_1360.c").read_text(encoding="utf-8")
MAKEFILE = (ROOT / "Makefile").read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


for token in (
    "run_1360_diagnostic",
    "FW_1360",
    "VMSPACE_VM_PMAP_1360",
    "DIAGNOSTIC_MARKER_A",
    "DIAGNOSTIC_MARKER_B",
    "is_kernel_pointer",
    "is_physical_address",
    "is_kernel_pmap_valid",
    "kernel_get_proc(0)",
    "getpmap(kernel_proc)",
    "vtophys(ktext)",
    "alloc_page()",
    "kwrite64(scratch_va, DIAGNOSTIC_MARKER_A)",
    "kwrite64(scratch_va + sizeof(uint64_t), DIAGNOSTIC_MARKER_B)",
):
    if token not in SOURCE:
        fail(f"diagnostic token missing: {token}")

for token in (
    "old_a",
    "old_b",
    "modified",
    "round_trip_ok",
    "scratch restoration verification failed",
):
    if token not in SOURCE:
        fail(f"scratch restoration guard missing: {token}")

if "diagnostic_1360.h" not in MAIN:
    fail("main must include the 13.60 diagnostic API")

if "if (fw == 0x1360)" not in MAIN:
    fail("main must contain a 13.60 gate")

gate = MAIN[MAIN.index("if (fw == 0x1360)"):]
return_pos = gate.find("return 0;")
if return_pos == -1:
    fail("13.60 diagnostic branch must return explicitly")

for token in (
    "fetch_linux(&linux_i)",
    "prepare_resume(",
    "hv_backend_prepare(",
    "enter_rest_mode()",
):
    if token in gate[:return_pos]:
        fail(f"13.60 diagnostic path must not call {token} before returning")

for token in (
    "hv_provider_1360",
    "sceKernelNotifySystemSuspendStart",
    "sceKernelSetEventFlag",
):
    if token in SOURCE:
        fail(f"diagnostic must not execute {token}")

if "munmap" in SOURCE:
    fail("diagnostic must not claim ownership of a virtual mapping it does not retain")

if "tests/test_13_60_completeness.py" not in MAKEFILE:
    fail("Makefile must keep the completeness suite wired in")

print("PASS: 13.60 diagnostic route is bounded, reversible and isolated from Linux/HV handoff")
