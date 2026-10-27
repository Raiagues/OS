.DEFAULT_GOAL := all

PLATFORM ?= virt
SELFTEST ?= 0
CROSS_COMPILE ?= riscv64-unknown-elf-
QEMU ?= qemu-system-riscv64
PYTHON ?= python3
RISCV_ARCH ?= rv64ima_zicsr
OPT ?= -O2
GDB_PORT ?= 1234

CC := $(CROSS_COMPILE)gcc
SIZE := $(CROSS_COMPILE)size
READELF := $(CROSS_COMPILE)readelf
BUILD := build/$(PLATFORM)$(if $(filter 1,$(SELFTEST)),-selftest)
KERNEL := $(BUILD)/kernel.elf

ifeq ($(PLATFORM),virt)
KERNEL_BASE := 0x80000000
BACKEND := src/console_serial.c
MEMORY := 128M
else ifeq ($(PLATFORM),cep)
KERNEL_BASE := 0x1000
BACKEND := src/console.c src/font.c
MEMORY := 32M
else
$(error PLATFORM must be virt or cep)
endif

CPPFLAGS += -Isrc -DPLATFORM_$(shell printf '%s' '$(PLATFORM)' | tr '[:lower:]' '[:upper:]')
ifeq ($(SELFTEST),1)
CPPFLAGS += -DKERNEL_SELFTEST
endif
ARCHFLAGS := -march=$(RISCV_ARCH) -mabi=lp64 -mcmodel=medany -mno-relax
CFLAGS += $(ARCHFLAGS) $(OPT) -g -std=gnu11 -Wall -Wextra -Wno-unused-parameter \
          -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -nostdinc \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS += $(ARCHFLAGS) -nostdlib -nostartfiles -static \
           -Wl,--defsym=KERNEL_BASE=$(KERNEL_BASE) -Wl,-T,src/kernel.lds \
           -Wl,--gc-sections -Wl,--build-id=none -Wl,-Map,$(BUILD)/kernel.map
CSOURCES := $(filter-out src/console.c src/console_serial.c src/font.c,$(wildcard src/*.c)) $(BACKEND)
ASOURCES := $(wildcard src/*.S)
OBJECTS := $(patsubst src/%.c,$(BUILD)/%.o,$(CSOURCES)) $(patsubst src/%.S,$(BUILD)/%.o,$(ASOURCES))
QEMUFLAGS := -machine $(PLATFORM) -bios none -kernel $(KERNEL) -m $(MEMORY) -smp 1

.PHONY: all check run debug rung test clean help FORCE
all: $(KERNEL)

$(BUILD):
	mkdir -p "$@"

# Record effective flags so a changed compiler/optimization rebuilds objects.
$(BUILD)/.config: FORCE | $(BUILD)
	@printf '%s\n' '$(CC)' '$(CPPFLAGS)' '$(CFLAGS)' '$(LDFLAGS)' > "$@.tmp"
	@cmp -s "$@.tmp" "$@" || cp "$@.tmp" "$@"
	@rm -f "$@.tmp"

$(BUILD)/%.o: src/%.c Makefile $(BUILD)/.config | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/%.o: src/%.S Makefile $(BUILD)/.config | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(KERNEL): $(OBJECTS) src/kernel.lds
	$(CC) $(LDFLAGS) $(OBJECTS) -lgcc -o "$@"
	$(SIZE) "$@"

check:
	@sh scripts/check-tools.sh '$(CROSS_COMPILE)' '$(QEMU)' '$(PLATFORM)' '$(RISCV_ARCH)' '$(PYTHON)'

run: $(KERNEL)
	$(QEMU) $(QEMUFLAGS) -nographic

debug: $(KERNEL)
	$(QEMU) $(QEMUFLAGS) -nographic -gdb tcp:127.0.0.1:$(GDB_PORT) -S

rung: $(KERNEL)
ifeq ($(PLATFORM),cep)
	$(QEMU) $(QEMUFLAGS) -serial mon:stdio -display sdl
else
	@printf '%s\n' 'The virt backend uses a terminal. Run make run, or select PLATFORM=cep with the course QEMU.'
	@exit 1
endif

test:
	$(MAKE) PLATFORM=virt SELFTEST=0 all
	$(MAKE) PLATFORM=virt SELFTEST=1 all
	$(PYTHON) scripts/test_qemu.py --qemu '$(QEMU)' --readelf '$(READELF)' \
	  --demo build/virt/kernel.elf --selftest build/virt-selftest/kernel.elf

clean:
	rm -rf build

help:
	@printf '%s\n' \
	  'make check     Check the compiler, QEMU and Python' \
	  'make           Build build/virt/kernel.elf' \
	  'make run       Run the demonstration (Ctrl+A, then X to quit)' \
	  'make test      Check boot, sleep, termination and timer preemption' \
	  'make debug     Start halted; GDB listens on 127.0.0.1:1234' \
	  'make PLATFORM=cep rung   Graphical mode with the custom course QEMU' \
	  'make clean     Remove generated build files' \
	  'Overrides: CROSS_COMPILE=... QEMU=... RISCV_ARCH=... OPT=-O0'

-include $(OBJECTS:.o=.d)
