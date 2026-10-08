#ifndef DIAGNOSTIC_1360_H
#define DIAGNOSTIC_1360_H

/*
 * Executable 13.60 preflight path.
 *
 * This verifies the loader-visible kernel environment and the already
 * available kernel read/write primitive without attempting Hypervisor
 * compromise, VM-exit interception, suspend/resume or Linux boot.
 */
int run_1360_diagnostic(void);

#endif
