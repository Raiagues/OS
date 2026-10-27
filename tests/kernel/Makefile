.DEFAULT_GOAL := all

PLATFORM ?= virt
CROSS_COMPILE ?= riscv64-unknown-elf-
QEMU ?= qemu-system-riscv64
RISCV_ARCH ?= rv64ima_zicsr
BUILD ?= build/$(PLATFORM)
OPT ?= -O2
GDB_PORT ?= 1234

CC := $(CROSS_COMPILE)gcc
SIZE := $(CROSS_COMPILE)size
KERNEL := $(BUILD)/kernel.elf

ifeq ($(PLATFORM),cep)
KERNEL_BASE := 0x1000
BACKEND := src/console.c src/font.c
MEMORY := 32M
else ifeq ($(PLATFORM),virt)
KERNEL_BASE := 0x80000000
BACKEND := src/console_serial.c
MEMORY := 128M
else
$(error PLATFORM must be cep or virt)
endif

CPPFLAGS += -Isrc -DPLATFORM_$(shell printf '%s' '$(PLATFORM)' | tr '[:lower:]' '[:upper:]')
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

.PHONY: all check run run-serial debug clean help FORCE
all: $(KERNEL)

$(BUILD):
	mkdir -p "$@"

# Rebuild if compiler or effective flags change.
$(BUILD)/.config: FORCE | $(BUILD)
	@printf '%s\n' '$(CC)' '$(CPPFLAGS)' '$(CFLAGS)' '$(LDFLAGS)' > "$@.tmp"
	@cmp -s "$@.tmp" "$@" || cp "$@.tmp" "$@"
	@rm -f "$@.tmp"

$(BUILD)/%.o: src/%.c Makefile $(BUILD)/.config | $(BUILD)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(BUILD)/%.o: src/%.S Makefile $(BUILD)/.config | $(BUILD)
	"$(CC)" $(CPPFLAGS) $(CFLAGS) -c "$<" -o "$@"

$(KERNEL): $(OBJECTS) src/kernel.lds
	"$(CC)" $(LDFLAGS) $(OBJECTS) -lgcc -o "$@"
	"$(SIZE)" "$@"

check:
	@sh scripts/check-tools.sh '$(CROSS_COMPILE)' '$(QEMU)' '$(PLATFORM)' '$(RISCV_ARCH)'

run: $(KERNEL)
ifeq ($(PLATFORM),cep)
	"$(QEMU)" $(QEMUFLAGS) -name "Kernel checks - CEP" -serial mon:stdio -display sdl,show-cursor=on
else
	"$(QEMU)" $(QEMUFLAGS) -nographic
endif

run-serial: $(KERNEL)
	"$(QEMU)" $(QEMUFLAGS) -nographic

debug: $(KERNEL)
ifeq ($(PLATFORM),cep)
	"$(QEMU)" $(QEMUFLAGS) -name "Kernel checks - CEP" -serial mon:stdio -display sdl,show-cursor=on -gdb tcp:127.0.0.1:$(GDB_PORT) -S
else
	"$(QEMU)" $(QEMUFLAGS) -nographic -gdb tcp:127.0.0.1:$(GDB_PORT) -S
endif

clean:
	rm -rf build

help:
	@printf '%s\n' \
	  './run.sh                  Run the kernel checks on CEP' \
	  './scripts/run-container.sh  Run this example with the pinned tools image' \
	  'make                      Build build/virt/kernel.elf' \
	  'make PLATFORM=virt run    Run kernel checks in a serial terminal' \
	  'make check                Check the compiler and selected QEMU machine' \
	  'make run-serial           Run without a graphical window' \
	  'make debug                Start halted; GDB at 127.0.0.1:1234' \
	  'make clean                Remove the build directory' \
	  'Overrides: CROSS_COMPILE=... QEMU=... RISCV_ARCH=... BUILD=... OPT=-O0'

-include $(OBJECTS:.o=.d)
