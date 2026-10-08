#ifndef HV_PROFILE_H
#define HV_PROFILE_H

#include <stdint.h>

enum hv_profile_requirement {
  HV_PROFILE_REQ_VMSPACE_VM_PMAP = 1u << 0,
  HV_PROFILE_REQ_KERNEL_CODE_CAVE = 1u << 1,
  HV_PROFILE_REQ_HV_CODE_CAVE_PA = 1u << 2,
  HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA = 1u << 3,
};

uint32_t hv_profile_missing(uint32_t requirements);
const char *hv_profile_requirement_name(uint32_t requirement);
int hv_profile_validate(uint32_t requirements);

#endif
