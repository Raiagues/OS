.DEFAULT_GOAL := all
CROSS_COMPILE ?= riscv64-unknown-elf-
QEMU ?= qemu-system-riscv64
RISCV_ARCH ?= rv64ima_zicsr
BUILD ?= build/cep
OPT ?= -O2
GDB_PORT ?= 1234

CC := $(CROSS_COMPILE)gcc
KERNEL := $(BUILD)/kernel.elf
CPPFLAGS += -Isrc
ARCHFLAGS := -march=$(RISCV_ARCH) -mabi=lp64 -mcmodel=medany -mno-relax
CFLAGS += $(ARCHFLAGS) $(OPT) -g -std=gnu11 -Wall -Wextra -Wno-unused-parameter \
          -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -nostdinc \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS += $(ARCHFLAGS) -nostdlib -nostartfiles -static \
           -Wl,--defsym=KERNEL_BASE=0x1000 -Wl,-T,src/kernel.lds \
           -Wl,--gc-sections -Wl,--build-id=none -Wl,-Map,$(BUILD)/kernel.map
CSOURCES := $(wildcard src/*.c)
ASOURCES := $(wildcard src/*.S)
OBJECTS := $(patsubst src/%.c,$(BUILD)/%.o,$(CSOURCES)) $(patsubst src/%.S,$(BUILD)/%.o,$(ASOURCES))
QEMUFLAGS := -machine cep -bios none -kernel $(KERNEL) -m 32M -smp 1

.PHONY: all check run run-serial debug clean help FORCE
all: $(KERNEL)

$(BUILD):
	mkdir -p "$@"

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
	"$(CROSS_COMPILE)size" "$@"

check:
	@sh scripts/check-tools.sh '$(CROSS_COMPILE)' '$(QEMU)' '$(RISCV_ARCH)'

run: $(KERNEL)
	"$(QEMU)" $(QEMUFLAGS) -name "OS - clock" -serial mon:stdio -display sdl,show-cursor=on

run-serial: $(KERNEL)
	"$(QEMU)" $(QEMUFLAGS) -nographic

debug: $(KERNEL)
	"$(QEMU)" $(QEMUFLAGS) -name "OS - clock" -serial mon:stdio -display sdl,show-cursor=on -gdb tcp:127.0.0.1:$(GDB_PORT) -S

clean:
	rm -rf -- "$(BUILD)"

help:
	@printf '%s\n' \
	  './run.sh          Build and run this example with CEP graphics' \
	  'make              Build build/cep/kernel.elf' \
	  'make check        Check the compiler and CEP QEMU' \
	  'make run-serial   Run without a graphical window' \
	  'make debug        Start paused for GDB on localhost:1234' \
	  'Overrides: CROSS_COMPILE=... QEMU=... RISCV_ARCH=... BUILD=... OPT=-O0'

-include $(OBJECTS:.o=.d)
