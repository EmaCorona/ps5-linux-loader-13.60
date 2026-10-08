#ifndef HV_BACKEND_H
#define HV_BACKEND_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  HV_BACKEND_OK = 0,
  HV_BACKEND_UNAVAILABLE = -1,
  HV_BACKEND_INVALID_PROFILE = -2,
} hv_backend_status;

/*
 * Select and execute the firmware-specific HV backend.
 *
 * Existing 3.00-7.61 implementations are delegated to their original
 * backends. Firmware 13.60 is explicitly recognized but remains unavailable
 * until a public, verifiable HV implementation can be integrated.
 */
hv_backend_status hv_backend_prepare(void *shellcode_kernel,
                                      size_t shellcode_kernel_len);

const char *hv_backend_name(void);
bool hv_backend_is_available(void);

#endif
