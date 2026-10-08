#include "diagnostic_1360.h"

#include "offsets.h"
#include "utils.h"
#include <sys/mman.h>

#define FW_1360 0x1360
#define VMSPACE_VM_PMAP_1360 0x2E8
#define DIAGNOSTIC_MARKER 0x1360136013601360ULL

static int fail(const char *message) {
  notify("13.60 diagnostic: FAIL: %s\n", message);
  return -1;
}

int run_1360_diagnostic(void) {
  if (fw != FW_1360)
    return fail("firmware is not 13.60");

  if (env_offset.VMSPACE_VM_PMAP != VMSPACE_VM_PMAP_1360)
    return fail("unexpected VMSPACE_VM_PMAP profile value");

  uint64_t kernel_proc = kernel_get_proc(0);
  if (!INKERNEL(kernel_proc))
    return fail("kernel process pointer is invalid");

  uint64_t kernel_pmap = getpmap(kernel_proc);
  if (!INKERNEL(kernel_pmap))
    return fail("kernel pmap pointer is invalid");

  flat_pmap pmap = {};
  kread(kernel_pmap, &pmap, sizeof(pmap));
  if (pmap.pm_pml4 == 0 || pmap.pm_cr3 == 0)
    return fail("kernel pmap/CR3 could not be read");

  uint64_t text_pa = vtophys(ktext);
  if (text_pa == 0)
    return fail("kernel text virtual-to-physical translation failed");

  /*
   * Exercise the existing kernel R/W path against a freshly allocated page.
   * The test deliberately touches only scratch memory and restores it before
   * returning.
   */
  uint64_t scratch_pa = alloc_page();
  if (scratch_pa == 0)
    return fail("scratch page allocation failed");

  uint64_t scratch_va = pa_to_dmap(scratch_pa);
  uint64_t old_value = kread64(scratch_va);
  kwrite64(scratch_va, DIAGNOSTIC_MARKER);

  if (kread64(scratch_va) != DIAGNOSTIC_MARKER) {
    kwrite64(scratch_va, old_value);
    return fail("kernel R/W round-trip failed");
  }

  kwrite64(scratch_va, old_value);

  notify("13.60 diagnostic: PASS\n");
  notify("  kernel_proc   = 0x%016llx\n",
         (unsigned long long)kernel_proc);
  notify("  kernel_pmap   = 0x%016llx\n",
         (unsigned long long)kernel_pmap);
  notify("  kernel_cr3    = 0x%016llx\n",
         (unsigned long long)pmap.pm_cr3);
  notify("  kernel_text_pa= 0x%016llx\n",
         (unsigned long long)text_pa);
  notify("  R/W scratch   = verified\n");
  notify("Linux boot/HV handoff remains disabled on 13.60.\n");

  return 0;
}
