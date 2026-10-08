#ifndef HV_PROVIDER_1360_H
#define HV_PROVIDER_1360_H

#include "hv_backend.h"

/*
 * Safe integration boundary for a future firmware-13.60 Hypervisor provider.
 *
 * This module performs validation and capability reporting only. It does not
 * implement Hypervisor compromise, VM-exit interception, arbitrary HV memory
 * access, or firmware-specific ROP/payload construction.
 */
hv_backend_status hv_provider_1360_prepare(
    const hv_backend_context *context);

bool hv_provider_1360_is_compatible(const hv_backend_context *context);

#endif
