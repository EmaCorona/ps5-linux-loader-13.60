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
    'FW_1360',
    'fw != FW_1360',
    "VMSPACE_VM_PMAP",
    "kernel_get_proc(0)",
    "getpmap(kernel_proc)",
    "vtophys(ktext)",
    "DIAGNOSTIC_MARKER",
):
    if token not in SOURCE:
        fail(f"diagnostic token missing: {token}")

if "diagnostic_1360.h" not in MAIN:
    fail("main must include the 13.60 diagnostic API")

diagnostic_gate = """if (fw == 0x1360) {
    notify("Firmware 13.60 detected: executable diagnostic path enabled.\\n");
    if (run_1360_diagnostic())
      return -1;
    return 0;
  }"""

if diagnostic_gate not in MAIN:
    fail("13.60 must route to the executable diagnostic path before the HV gate")

if "munmap" in SOURCE:
    fail("diagnostic must not add unnecessary resource teardown after exercising the R/W path")

if "hv_backend_prepare" in SOURCE:
    fail("diagnostic must not invoke the HV backend")

if "tests/test_13_60_diagnostic.py" not in MAKEFILE:
    fail("Makefile must run the diagnostic regression test")

print("PASS: executable 13.60 diagnostic route is wired into the loader")
