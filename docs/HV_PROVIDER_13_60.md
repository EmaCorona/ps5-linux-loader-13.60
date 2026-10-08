# 13.60 HV Provider Contract

## Purpose

This document defines the safe integration boundary between ps5-linux-loader and a future firmware-13.60 Hypervisor provider.

The provider is expected to be supplied independently. This repository does not synthesize or implement Hypervisor exploitation primitives.

## Boundary

The loader owns:

- firmware detection;
- kernel profile selection;
- Linux image loading;
- Linux resume preparation;
- validation of required capabilities;
- error handling and fail-closed behavior.

The firmware-specific provider owns its own Hypervisor interaction and must expose one loader-facing preparation operation.

Conceptually:

    loader -> validate provider -> provider_prepare() -> Linux resume handoff

## Provider requirements

A provider integrated for 13.60 must, before being marked available:

1. Identify the target firmware explicitly as 13.60.
2. Be source-available or otherwise independently auditable.
3. Document every firmware-specific address or structure it consumes.
4. Validate its own assumptions before modifying system state.
5. Report failure without continuing into Linux resume preparation.
6. Be tested on real 13.60 hardware before the backend is enabled.

## Loader contract

The existing backend abstraction is intentionally small:

    hv_backend_prepare(shellcode_kernel, shellcode_kernel_len)

The loader must not require callers to know how the provider implements its firmware-specific work.

On success the provider returns HV_BACKEND_OK.

On an unavailable implementation it returns HV_BACKEND_UNAVAILABLE.

On invalid or inconsistent profile data it returns HV_BACKEND_INVALID_PROFILE.

## Profile rule

Do not populate the following 13.60 fields merely by analogy with another firmware:

- KERNEL_CODE_CAVE
- HV_CODE_CAVE_PA
- HV_HANDLE_VMEXIT_PA

These values become valid only when supported by the actual provider implementation and verified for the target firmware.

## Activation gate

`backend_1360.available` must remain false until:

- provider implementation is integrated;
- required profile validation passes;
- static regression tests pass;
- the complete payload builds with the PS5 Payload SDK;
- real-console validation succeeds.

## Why this boundary matters

Linux boot support depends on more than obtaining kernel read/write access. The existing loader resume path depends on firmware-specific state that must be established safely before entering rest mode.

Keeping the provider isolated prevents firmware-specific Hypervisor logic from leaking into the generic Linux loader and preserves the working 3.00–7.61 implementations.