# 13.60 build

The CI workflow uses PS5 Payload SDK v0.43, which includes firmware 13.60 support. It runs the static regression checks and builds `bin/ps5-linux-loader.elf`.

The 13.60 HV backend remains disabled until a separately verified firmware-specific implementation is available. The resulting ELF is therefore a safe, fail-closed integration build rather than a claim of Linux boot support.
