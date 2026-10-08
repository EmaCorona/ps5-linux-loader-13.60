#include "hv_backend.h"
#include "loader.h"
#include "prepare_resume.h"
#include "utils.h"
#include <unistd.h>

int main(void) {
  if (setup_env()) {
    notify("Something went wrong while initiating.\nPlease make sure your fw "
           "is supported.");
    return -1;
  }

  if (!hv_backend_is_available()) {
    notify("Firmware %04x has no complete Linux HV backend.\n", fw);
    if (fw == 0x1360) {
      notify("The 13.60 kernel profile is present, but HV integration is pending.\n");
      uint32_t missing = hv_backend_missing_requirements();
      if (missing != 0) {
        notify("Missing backend profile capabilities:\n");
        for (uint32_t bit = 1; bit != 0; bit <<= 1) {
          if (missing & bit)
            notify("  - %s\n", hv_profile_requirement_name(bit));
        }
      }
    }
    notify("Aborting before Linux file mapping and resume preparation.\n");
    return -1;
  }

  if (fetch_linux(&linux_i)) {
    notify("Something went wrong while installing linux files.\n");
    return -1;
  }

  void *shellcode_kernel;
  size_t shellcode_kernel_len;
  if (prepare_resume(&shellcode_kernel, &shellcode_kernel_len)) {
    notify("Something went wrong while preparing resume.\n");
    return -1;
  }

  const hv_backend_context backend_context = {
      .firmware = fw,
      .kernel_text = ktext,
      .kernel_data = kdata,
      .dmap_base = dmap,
      .shellcode_kernel = shellcode_kernel,
      .shellcode_kernel_len = shellcode_kernel_len,
      .linux_info_va = linux_i.linux_info,
  };

  notify("Selected HV backend: %s\n", hv_backend_name());
  hv_backend_status backend_status = hv_backend_prepare(&backend_context);
  if (backend_status != HV_BACKEND_OK) {
    notify("HV backend preparation failed: %s.\n",
           hv_backend_status_name(backend_status));
    goto err;
  }

  notify("Finished preparation. Going to rest mode in 5 seconds.\nPlease wait "
         "for the orange light to stop "
         "blinking and then wakeup to Linux :)\n");

  sleep(5);
  enter_rest_mode();

  while (1) {
    sleep(30);
  }

  return 0;

err:
  notify("Please make sure your fw is supported.");
  return -1;
}