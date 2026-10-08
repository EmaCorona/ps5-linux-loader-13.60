#include "diagnostic_1360.h"

#include "offsets.h"
#include "utils.h"

#include <stdbool.h>
#include <stdint.h>

#define FW_1360 0x1360u
#define VMSPACE_VM_PMAP_1360 0x2E8u
#define DIAGNOSTIC_MARKER_A 0x1360136013601360ULL
#define DIAGNOSTIC_MARKER_B 0xA5A55A5A13601360ULL
#define PHYSICAL_ADDRESS_MASK 0x000FFFFFFFFFF000ULL

static int fail(const char *message) {
  notify("13.60 diagnostic: FAIL: %s\n", message);
  return -1;
}

static bool is_kernel_pointer(uint64_t value) {
  return (value & 0xFFFF000000000000ULL) == 0xFFFF000000000000ULL;
}

static bool is_physical_address(uint64_t value) {
  return value != 0 && (value & ~PHYSICAL_ADDRESS_MASK) == 0;
}

static bool is_kernel_pmap_valid(const flat_pmap *pmap) {
  if (pmap == NULL)
    return false;

  if (!is_kernel_pointer(pmap->pm_pml4))
    return false;

  if (!is_physical_address(pmap->pm_cr3))
    return false;

  return true;
}

int run_1360_diagnostic(void) {
  if (fw != FW_1360)
    return fail("firmware is not 13.60");

  if (env_offset.VMSPACE_VM_PMAP != VMSPACE_VM_PMAP_1360)
    return fail("unexpected VMSPACE_VM_PMAP profile value");

  /*
   * The diagnostic must remain completely below the Linux/HV handoff. It
   * checks only the state that the loader already has through its kernel R/W
   * primitive and exercises a scratch page owned by this process.
   */
  notify("13.60 diagnostic: checking kernel process and pmap...\n");

  uint64_t kernel_proc = kernel_get_proc(0);
  if (!is_kernel_pointer(kernel_proc))
    return fail("kernel process pointer is invalid");

  uint64_t kernel_pmap = getpmap(kernel_proc);
  if (!is_kernel_pointer(kernel_pmap))
    return fail("kernel pmap pointer is invalid");

  flat_pmap pmap = {};
  kread(kernel_pmap, &pmap, sizeof(pmap));
  if (!is_kernel_pmap_valid(&pmap))
    return fail("kernel pmap/CR3 is invalid");

  notify("13.60 diagnostic: checking kernel VA->PA translation...\n");

  uint64_t text_pa = vtophys(ktext);
  if (!is_physical_address(text_pa))
    return fail("kernel text virtual-to-physical translation failed");

  if (!is_kernel_pointer(pa_to_dmap(text_pa)))
    return fail("kernel direct-map translation is invalid");

  /*
   * Exercise the existing kernel R/W path against a freshly allocated page.
   * Two adjacent words are used so a bad alias/offset cannot accidentally
   * satisfy a single-value smoke test.
   */
  notify("13.60 diagnostic: checking scratch kernel R/W...\n");

  uint64_t scratch_pa = alloc_page();
  if (!is_physical_address(scratch_pa))
    return fail("scratch page allocation failed");

  uint64_t scratch_va = pa_to_dmap(scratch_pa);
  if (!is_kernel_pointer(scratch_va))
    return fail("scratch direct-map translation is invalid");

  const uint64_t old_a = kread64(scratch_va);
  const uint64_t old_b = kread64(scratch_va + sizeof(uint64_t));

  bool modified = false;
  kwrite64(scratch_va, DIAGNOSTIC_MARKER_A);
  modified = true;
  kwrite64(scratch_va + sizeof(uint64_t), DIAGNOSTIC_MARKER_B);

  const bool round_trip_ok =
      kread64(scratch_va) == DIAGNOSTIC_MARKER_A &&
      kread64(scratch_va + sizeof(uint64_t)) == DIAGNOSTIC_MARKER_B;

  if (modified) {
    kwrite64(scratch_va, old_a);
    kwrite64(scratch_va + sizeof(uint64_t), old_b);
  }

  if (!round_trip_ok)
    return fail("kernel R/W round-trip failed");

  /*
   * Restoration is part of the test contract. If this check fails, stop
   * rather than continuing toward any later loader stage.
   */
  if (kread64(scratch_va) != old_a ||
      kread64(scratch_va + sizeof(uint64_t)) != old_b)
    return fail("scratch restoration verification failed");

  notify("13.60 diagnostic: PASS\n");
  notify("  kernel_proc    = 0x%016llx\n",
         (unsigned long long)kernel_proc);
  notify("  kernel_pmap    = 0x%016llx\n",
         (unsigned long long)kernel_pmap);
  notify("  kernel_cr3     = 0x%016llx\n",
         (unsigned long long)pmap.pm_cr3);
  notify("  kernel_text_pa = 0x%016llx\n",
         (unsigned long long)text_pa);
  notify("  scratch R/W    = verified + restored\n");
  notify("Linux boot/HV handoff remains disabled on 13.60.\n");

  return 0;
}
