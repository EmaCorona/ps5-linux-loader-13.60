#include "hv_profile.h"
#include "offsets.h"
#include "utils.h"

static uint64_t hv_profile_requirement_value(uint32_t requirement) {
  switch (requirement) {
  case HV_PROFILE_REQ_VMSPACE_VM_PMAP:
    return env_offset.VMSPACE_VM_PMAP;
  case HV_PROFILE_REQ_KERNEL_CODE_CAVE:
    return env_offset.KERNEL_CODE_CAVE;
  case HV_PROFILE_REQ_HV_CODE_CAVE_PA:
    return env_offset.HV_CODE_CAVE_PA;
  case HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA:
    return env_offset.HV_HANDLE_VMEXIT_PA;
  default:
    return 0;
  }
}

uint32_t hv_profile_missing(uint32_t requirements) {
  uint32_t missing = 0;

  for (uint32_t bit = 1; bit != 0; bit <<= 1) {
    if (!(requirements & bit))
      continue;
    if (hv_profile_requirement_value(bit) == 0)
      missing |= bit;
  }

  return missing;
}

const char *hv_profile_requirement_name(uint32_t requirement) {
  switch (requirement) {
  case HV_PROFILE_REQ_VMSPACE_VM_PMAP:
    return "VMSPACE_VM_PMAP";
  case HV_PROFILE_REQ_KERNEL_CODE_CAVE:
    return "KERNEL_CODE_CAVE";
  case HV_PROFILE_REQ_HV_CODE_CAVE_PA:
    return "HV_CODE_CAVE_PA";
  case HV_PROFILE_REQ_HV_HANDLE_VMEXIT_PA:
    return "HV_HANDLE_VMEXIT_PA";
  default:
    return "unknown";
  }
}

int hv_profile_validate(uint32_t requirements) {
  return hv_profile_missing(requirements) == 0 ? 0 : -1;
}
