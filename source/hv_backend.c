#include "hv_backend.h"
#include "hv_defeat_0304.h"
#include "hv_defeat_0506.h"
#include "hv_defeat_0607.h"
#include "utils.h"

static const uint32_t HV_PROFILE_1360_REQUIREMENTS =
    HV_PROFILE_REQ_VMSPACE_VM_PMAP | HV_PROFILE_REQ_KERNEL_CODE_CAVE |
    HV_PROFILE_REQ_HV_CODE_CAVE_PA | HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA;

/*
 * Firmware 13.60 currently has no public, verifiable HV payload in this tree.
 * Keep the backend as a real descriptor so a future public implementation can
 * be added without changing the loader's dispatch logic again.
 */
static hv_backend_status hv_backend_prepare_1360(void *shellcode_kernel,
                                                 size_t shellcode_kernel_len) {
  (void)shellcode_kernel;
  (void)shellcode_kernel_len;

  notify("HV backend for firmware 13.60 is not available.\n");
  notify("No firmware-specific VM-exit/resume payload will be attempted.\n");
  return HV_BACKEND_UNAVAILABLE;
}

static const hv_backend_descriptor backend_1360 = {
    .firmware = 0x1360,
    .name = "13.60 (integration pending)",
    .required_profile = HV_PROFILE_1360_REQUIREMENTS,
    .prepare = hv_backend_prepare_1360,
    .available = false,
};

static hv_backend_descriptor backend_for_firmware(void) {
  if (fw == backend_1360.firmware)
    return backend_1360;

  if ((0x0300 <= fw) && (fw < 0x0500)) {
    return (hv_backend_descriptor){
        .firmware = fw,
        .name = "HV 3.00-4.xx",
        .required_profile = 0,
        .prepare = NULL,
        .available = hv_profile_is_complete(),
    };
  }

  if ((0x0500 <= fw) && (fw < 0x0650)) {
    return (hv_backend_descriptor){
        .firmware = fw,
        .name = "HV 5.00-6.02",
        .required_profile = 0,
        .prepare = NULL,
        .available = hv_profile_is_complete(),
    };
  }

  if ((0x0650 <= fw) && (fw < 0x0800)) {
    return (hv_backend_descriptor){
        .firmware = fw,
        .name = "HV 6.50-7.61",
        .required_profile = 0,
        .prepare = NULL,
        .available = hv_profile_is_complete(),
    };
  }

  return (hv_backend_descriptor){
      .firmware = fw,
      .name = "unsupported",
      .required_profile = 0,
      .prepare = NULL,
      .available = false,
  };
}

uint32_t hv_backend_missing_requirements(void) {
  hv_backend_descriptor backend = backend_for_firmware();
  return hv_profile_missing(backend.required_profile);
}

hv_backend_status hv_backend_prepare(void *shellcode_kernel,
                                      size_t shellcode_kernel_len) {
  hv_backend_descriptor backend = backend_for_firmware();

  if (!backend.available && backend.required_profile != 0 &&
      hv_profile_validate(backend.required_profile) != 0) {
    return HV_BACKEND_INVALID_PROFILE;
  }

  if (backend.firmware == 0x1360)
    return backend.prepare(shellcode_kernel, shellcode_kernel_len);

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
  return backend_for_firmware().name;
}

bool hv_backend_is_available(void) {
  hv_backend_descriptor backend = backend_for_firmware();

  if (!backend.available)
    return false;

  return backend.required_profile == 0 ||
         hv_profile_validate(backend.required_profile) == 0;
}
