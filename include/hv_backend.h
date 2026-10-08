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
  HV_BACKEND_INVALID_ARGUMENT = -3,
} hv_backend_status;

/*
 * Read-only context passed to a firmware-specific backend.
 *
 * The context deliberately contains loader state only. A backend that needs
 * firmware-specific Hypervisor primitives must implement those independently
 * and expose only its preparation result through this contract.
 */
typedef struct {
  uint32_t firmware;
  uint64_t kernel_text;
  uint64_t kernel_data;
  uint64_t dmap_base;
  void *shellcode_kernel;
  size_t shellcode_kernel_len;
  uint64_t linux_info_va;
} hv_backend_context;

typedef hv_backend_status (*hv_backend_prepare_fn)(
    const hv_backend_context *context);

typedef struct {
  uint32_t firmware;
  const char *name;
  uint32_t required_profile;
  hv_backend_prepare_fn prepare;
  bool available;
} hv_backend_descriptor;

hv_backend_status hv_backend_prepare(const hv_backend_context *context);

const char *hv_backend_name(void);
const char *hv_backend_status_name(hv_backend_status status);
bool hv_backend_is_available(void);
uint32_t hv_backend_missing_requirements(void);

#endif