.DEFAULT_GOAL := all

CROSS_COMPILE ?= riscv64-unknown-elf-
QEMU ?= qemu-system-riscv64
RISCV_ARCH ?= rv64ima_zicsr
BUILD ?= build/cep

CC := $(CROSS_COMPILE)gcc
KERNEL := $(BUILD)/kernel.elf
CPPFLAGS += -Isrc
ARCHFLAGS := -march=$(RISCV_ARCH) -mabi=lp64 -mcmodel=medany -mno-relax
CFLAGS += $(ARCHFLAGS) -O2 -g -std=gnu11 -Wall -Wextra -Wno-unused-parameter \
          -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -nostdinc \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS += $(ARCHFLAGS) -nostdlib -nostartfiles -static \
           -Wl,-T,src/kernel.lds -Wl,--gc-sections -Wl,--build-id=none \
           -Wl,-Map,$(BUILD)/kernel.map
CSOURCES := src/main.c src/console.c src/font.c src/uart.c src/string.c src/printf.c src/doprnt.c
ASOURCES := src/crt0.S
OBJECTS := $(patsubst src/%.c,$(BUILD)/%.o,$(CSOURCES)) $(patsubst src/%.S,$(BUILD)/%.o,$(ASOURCES))
QEMUFLAGS := -machine cep -bios none -kernel $(KERNEL) -m 32M -smp 1

.PHONY: all check run run-serial clean FORCE
all: $(KERNEL)

$(BUILD):
	mkdir -p "$@"

# A compiler or flag change rebuilds this folder's objects.
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
	"$(QEMU)" $(QEMUFLAGS) -name "02 - Console" -serial mon:stdio -display sdl,show-cursor=on

run-serial: $(KERNEL)
	"$(QEMU)" $(QEMUFLAGS) -nographic

clean:
	rm -rf "$(BUILD)"

-include $(OBJECTS:.o=.d)
