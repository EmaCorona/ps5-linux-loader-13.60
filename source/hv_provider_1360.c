#include "hv_provider_1360.h"

#define FW_1360 0x1360u

static bool context_is_valid(const hv_backend_context *context) {
  if (context == NULL)
    return false;

  if (context->firmware != FW_1360)
    return false;

  /*
   * These values are loader-produced state. Requiring all of them here makes
   * the provider boundary fail closed if a future caller wires the backend
   * too early in the lifecycle.
   */
  if (context->kernel_text == 0 || context->kernel_data == 0 ||
      context->dmap_base == 0)
    return false;

  if (context->shellcode_kernel == NULL ||
      context->shellcode_kernel_len == 0)
    return false;

  if (context->linux_info_va == 0)
    return false;

  return true;
}

bool hv_provider_1360_is_compatible(const hv_backend_context *context) {
  /*
   * Compatibility means only that the complete loader context is internally
   * well formed for firmware 13.60. It is not proof that a Hypervisor
   * transition is available.
   */
  return context_is_valid(context);
}

hv_backend_status hv_provider_1360_prepare(
    const hv_backend_context *context) {
  if (!context_is_valid(context))
    return HV_BACKEND_INVALID_ARGUMENT;

  /*
   * Deliberately fail closed until a separately verified provider is
   * available. Keeping this boundary explicit prevents accidental reuse of
   * a legacy HV implementation on 13.60.
   */
  return HV_BACKEND_UNAVAILABLE;
}
