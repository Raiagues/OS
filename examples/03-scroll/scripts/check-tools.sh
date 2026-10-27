#!/bin/sh
set -eu
cross=${1:-riscv64-unknown-elf-}
qemu=${2:-qemu-system-riscv64}
arch=${3:-rv64ima_zicsr}
for tool in make "${cross}gcc" "${cross}size" "$qemu"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'Missing tool: %s. See README.md.\n' "$tool" >&2
        exit 1
    fi
done
"${cross}gcc" --version | head -n 1
"$qemu" --version | head -n 1
if ! "$qemu" -machine help | awk '$1 == "cep" { found=1 } END { exit !found }'; then
    printf 'This example needs a QEMU build with the CEP machine.\n' >&2
    exit 1
fi
probe=$(mktemp -d)
trap 'rm -rf "$probe"' EXIT HUP INT TERM
printf '%s\n' 'void probe(void) { __asm__ volatile("csrr a0, mcause"); }' > "$probe/probe.c"
"${cross}gcc" -march="$arch" -mabi=lp64 -ffreestanding -c "$probe/probe.c" -o "$probe/probe.o"
printf 'Tools ready for CEP, RISCV_ARCH=%s.\n' "$arch"
