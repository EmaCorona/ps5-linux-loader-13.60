#ifndef HV_BACKEND_H
#define HV_BACKEND_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hv_profile.h"

typedef enum {
  HV_BACKEND_OK = 0,
  HV_BACKEND_UNAVAILABLE = -1,
  HV_BACKEND_INVALID_PROFILE = -2,
} hv_backend_status;

typedef hv_backend_status (*hv_backend_prepare_fn)(void *shellcode_kernel,
                                                   size_t shellcode_kernel_len);

typedef struct {
  uint32_t firmware;
  const char *name;
  uint32_t required_profile;
  hv_backend_prepare_fn prepare;
  bool available;
} hv_backend_descriptor;

hv_backend_status hv_backend_prepare(void *shellcode_kernel,
                                      size_t shellcode_kernel_len);

const char *hv_backend_name(void);
bool hv_backend_is_available(void);
uint32_t hv_backend_missing_requirements(void);

#endif
