.DEFAULT_GOAL := all

PLATFORM ?= cep
SELFTEST ?= 0
DEMO ?= processes
CROSS_COMPILE ?= riscv64-unknown-elf-
QEMU ?= qemu-system-riscv64
PYTHON ?= python3
RISCV_ARCH ?= rv64ima_zicsr
OPT ?= -O2
GDB_PORT ?= 1234

CC := $(CROSS_COMPILE)gcc
SIZE := $(CROSS_COMPILE)size
READELF := $(CROSS_COMPILE)readelf
VALID_DEMOS := processes pixels console scroll uart clock
ifeq ($(filter $(DEMO),$(VALID_DEMOS)),)
$(error DEMO must be one of: $(VALID_DEMOS))
endif
ifneq ($(SELFTEST),0)
ifneq ($(DEMO),processes)
$(error SELFTEST requires DEMO=processes)
endif
endif
ifneq ($(PLATFORM),cep)
ifneq ($(filter $(DEMO),pixels console scroll),)
$(error DEMO=$(DEMO) inspects the CEP framebuffer; use PLATFORM=cep)
endif
endif
BUILD := build/$(PLATFORM)$(if $(filter 1,$(SELFTEST)),-selftest,$(if $(filter-out processes,$(DEMO)),-$(DEMO)))
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

CPPFLAGS += -DDEMO_$(shell printf '%s' '$(DEMO)' | tr '[:lower:]' '[:upper:]')
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

.PHONY: all check run run-serial debug rung test clean help FORCE
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
ifeq ($(PLATFORM),cep)
	$(QEMU) $(QEMUFLAGS) -name "OS - graphical console" -serial mon:stdio -display sdl,show-cursor=on
else
	$(QEMU) $(QEMUFLAGS) -nographic
endif

run-serial: $(KERNEL)
	$(QEMU) $(QEMUFLAGS) -nographic

debug: $(KERNEL)
ifeq ($(PLATFORM),cep)
	$(QEMU) $(QEMUFLAGS) -name "OS - graphical console" -serial mon:stdio -display sdl,show-cursor=on -gdb tcp:127.0.0.1:$(GDB_PORT) -S
else
	$(QEMU) $(QEMUFLAGS) -nographic -gdb tcp:127.0.0.1:$(GDB_PORT) -S
endif

rung: $(KERNEL)
ifeq ($(PLATFORM),cep)
	$(QEMU) $(QEMUFLAGS) -name "OS - graphical console" -serial mon:stdio -display sdl,show-cursor=on
else
	@printf '%s\n' 'The virt backend uses a terminal. Run make run, or select PLATFORM=cep with CEP QEMU.'
	@exit 1
endif

test:
	$(MAKE) PLATFORM=virt DEMO=processes SELFTEST=0 all
	$(MAKE) PLATFORM=virt DEMO=processes SELFTEST=1 all
	$(PYTHON) scripts/test_qemu.py --qemu '$(QEMU)' --readelf '$(READELF)' \
	  --demo build/virt/kernel.elf --selftest build/virt-selftest/kernel.elf

clean:
	rm -rf build

help:
	@printf '%s\n' \
	  'make check     Check the compiler, QEMU and Python' \
	  './run.sh       Open the CEP graphical interface' \
	  './run.sh DEMO=pixels    Run a small demo (pixels/console/scroll/uart/clock)' \
	  'make           Build build/cep/kernel.elf (CEP toolchain required)' \
	  'make run       Open the CEP graphical window' \
	  'make PLATFORM=virt run   Optional serial mode with upstream QEMU' \
	  'make run-serial          CEP serial output without a window' \
	  'make test      Check boot, sleep, termination and timer preemption' \
	  'make debug     Start halted; GDB listens on 127.0.0.1:1234' \
	  'make PLATFORM=cep rung   Graphical mode with the custom CEP QEMU' \
	  'make clean     Remove generated build files' \
	  'Overrides: CROSS_COMPILE=... QEMU=... RISCV_ARCH=... OPT=-O0'

-include $(OBJECTS:.o=.d)
