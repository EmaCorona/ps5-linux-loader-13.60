#!/usr/bin/env python3
"""Static checks for the 13.60 provider contract and fail-closed behavior."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "include" / "hv_provider_1360.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "source" / "hv_provider_1360.c").read_text(encoding="utf-8")
BACKEND = (ROOT / "source" / "hv_backend.c").read_text(encoding="utf-8")
MAIN = (ROOT / "source" / "main.c").read_text(encoding="utf-8")


def fail(message: str) -> None:
    print(f"FAIL: {message}")
    raise SystemExit(1)


for token in (
    "hv_provider_1360_prepare",
    "hv_provider_1360_is_compatible",
):
    if token not in HEADER or token not in SOURCE:
        fail(f"provider API token missing: {token}")

for token in (
    "context->firmware != FW_1360",
    "context->kernel_text == 0",
    "context->kernel_data == 0",
    "context->dmap_base == 0",
    "context->shellcode_kernel == NULL",
    "context->shellcode_kernel_len == 0",
    "context->linux_info_va == 0",
):
    if token not in SOURCE:
        fail(f"provider precondition missing: {token}")

if "HV_BACKEND_INVALID_ARGUMENT" not in SOURCE:
    fail("provider must reject malformed context")

if "HV_BACKEND_UNAVAILABLE" not in SOURCE:
    fail("provider must fail closed while no verified implementation exists")

for legacy in (
    "hv_defeat_0304.h",
    "hv_defeat_0506.h",
    "hv_defeat_0607.h",
):
    if legacy in SOURCE:
        fail(f"13.60 provider must not depend on {legacy}")

if "hv_provider_1360_prepare(context)" not in BACKEND:
    fail("13.60 backend must delegate through the provider boundary")

prepare_match = re.search(
    r"hv_provider_1360_prepare\([\s\S]*?\)\s*\{([\s\S]*?)\n\}",
    SOURCE,
)
if not prepare_match:
    fail("could not locate provider prepare implementation")

prepare_body = prepare_match.group(1)
if "context_is_valid(context)" not in prepare_body:
    fail("prepare must validate context before returning")

if "return HV_BACKEND_UNAVAILABLE;" not in prepare_body:
    fail("prepare must remain fail-closed")

if "if (fw == 0x1360)" not in MAIN:
    fail("main must contain the 13.60 gate")

print("PASS: 13.60 provider rejects malformed state and never falls through to a legacy HV backend")
