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
    if (fw == 0x1360)
      notify("The 13.60 kernel profile is present, but HV integration is pending.\n");
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

  notify("Selected HV backend: %s\n", hv_backend_name());
  if (hv_backend_prepare(shellcode_kernel, shellcode_kernel_len) != HV_BACKEND_OK)
    goto err;

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
  notify("HV backend preparation failed.\nPlease make sure "
         "your fw is supported.");
  return -1;
}
