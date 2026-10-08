#include "hv_backend.h"
#include "hv_defeat_0304.h"
#include "hv_defeat_0506.h"
#include "hv_defeat_0607.h"
#include "utils.h"

static hv_backend_status hv_backend_prepare_1360(void) {
  /*
   * 13.60 has a verified kernel profile, but no public HV backend in this
   * tree. Do not reuse the 6.50-7.61 backend: its suspend/VM-exit machinery
   * depends on firmware-specific hypervisor state.
   */
  notify("HV backend for firmware 13.60 is not available.\n");
  notify("No firmware-specific VM-exit/resume payload will be attempted.\n");
  return HV_BACKEND_UNAVAILABLE;
}

hv_backend_status hv_backend_prepare(void *shellcode_kernel,
                                      size_t shellcode_kernel_len) {
  if (fw == 0x1360)
    return hv_backend_prepare_1360();

  if ((0x0300 <= fw) && (fw < 0x0500)) {
    return hv_defeat_0304(shellcode_kernel, shellcode_kernel_len)
               ? HV_BACKEND_INVALID_PROFILE
               : HV_BACKEND_OK;
  }

  if ((0x0500 <= fw) && (fw < 0x0650)) {
    return hv_defeat_0506(shellcode_kernel, shellcode_kernel_len)
               ? HV_BACKEND_INVALID_PROFILE
               : HV_BACKEND_OK;
  }

  if ((0x0650 <= fw) && (fw < 0x0800)) {
    return hv_defeat_0607(shellcode_kernel, shellcode_kernel_len)
               ? HV_BACKEND_INVALID_PROFILE
               : HV_BACKEND_OK;
  }

  return HV_BACKEND_UNAVAILABLE;
}

const char *hv_backend_name(void) {
  if (fw == 0x1360)
    return "13.60 (stub)";
  if ((0x0300 <= fw) && (fw < 0x0500))
    return "HV 3.00-4.xx";
  if ((0x0500 <= fw) && (fw < 0x0650))
    return "HV 5.00-6.02";
  if ((0x0650 <= fw) && (fw < 0x0800))
    return "HV 6.50-7.61";
  return "unsupported";
}

bool hv_backend_is_available(void) {
  if (fw == 0x1360)
    return false;
  return hv_profile_is_complete();
}
