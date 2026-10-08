.PHONY: all clean test verify-13-60

ifndef PS5_PAYLOAD_SDK
    PS5_PAYLOAD_SDK = /opt/ps5-payload-sdk/
endif

# Host-side regression tests must not require the PS5 SDK.
# The toolchain is only included when a build-oriented target is requested.
ifneq ($(filter test verify-13-60,$(MAKECMDGOALS)),test verify-13-60)
include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
endif

BIN := bin/ps5-linux-loader.elf
SRC := $(wildcard source/*.c)
OBJS := $(SRC:.c=.o)

CFLAGS  := -std=c23 -Wall -Iinclude -Ishellcode_hv -Ishellcode_kernel
LDFLAGS :=

SC_0607_H := shellcode_0607/shellcode_0607.h
SC_HV_H := shellcode_hv/shellcode_hv.h
SC_K_H  := shellcode_kernel/shellcode_kernel.h

all: $(SC_0607_H) $(SC_HV_H) $(SC_K_H) $(BIN)

$(SC_0607_H):
	$(MAKE) -C shellcode_0607

$(SC_HV_H):
	$(MAKE) -C shellcode_hv

$(SC_K_H): $(SC_HV_H)
	$(MAKE) -C shellcode_kernel

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

source/prepare_resume.o: $(SC_K_H)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

test:
	python3 -m compileall -q tests
	python3 tests/test_13_60_profile.py
	python3 tests/test_13_60_provider.py
	python3 tests/test_13_60_diagnostic.py
	python3 tests/test_13_60_completeness.py

verify-13-60: test

clean:
	rm -f $(BIN) $(OBJS)
	$(MAKE) -C shellcode_0607 clean
	$(MAKE) -C shellcode_hv clean
	$(MAKE) -C shellcode_kernel clean
