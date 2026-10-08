#!/usr/bin/env python3
"""Static checks for the safe 13.60 provider integration boundary."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "include" / "hv_provider_1360.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "source" / "hv_provider_1360.c").read_text(encoding="utf-8")
BACKEND = (ROOT / "source" / "hv_backend.c").read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


for token in (
    "hv_provider_1360_prepare",
    "hv_provider_1360_is_compatible",
):
    if token not in HEADER or token not in SOURCE:
        fail(f"provider API token missing: {token}")

if "context->firmware == 0x1360" not in SOURCE:
    fail("provider must validate the firmware")

if "HV_BACKEND_INVALID_ARGUMENT" not in SOURCE:
    fail("provider must reject malformed context")

if "HV_BACKEND_UNAVAILABLE" not in SOURCE:
    fail("provider must fail closed while no verified implementation exists")

if '#include "hv_defeat_0607.h"' in SOURCE:
    fail("13.60 provider must not depend on the legacy 6.50-7.61 backend")

if "hv_provider_1360_prepare(context)" not in BACKEND:
    fail("13.60 backend must delegate through the provider boundary")

print("PASS: 13.60 provider boundary is explicit and fail-closed")
