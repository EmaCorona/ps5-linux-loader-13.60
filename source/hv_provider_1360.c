#include "hv_provider_1360.h"

#include <stdbool.h>

static bool context_is_valid(const hv_backend_context *context) {
  return context != NULL && context->firmware == 0x1360 &&
         context->shellcode_kernel != NULL &&
         context->shellcode_kernel_len != 0;
}

bool hv_provider_1360_is_compatible(const hv_backend_context *context) {
  /*
   * Compatibility here intentionally means only that the loader supplied a
   * well-formed 13.60 context. It must not be interpreted as proof that the
   * required HV transition is available.
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
