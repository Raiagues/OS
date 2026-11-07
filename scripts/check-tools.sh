#!/bin/sh
set -eu
cross=${1:-riscv64-unknown-elf-}
qemu=${2:-qemu-system-riscv64}
arch=${3:-rv64ima_zicsr}
for tool in "${cross}gcc" "${cross}readelf" "${cross}size" "$qemu"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'Missing tool: %s. See README.md for local or Docker setup.\n' "$tool" >&2
        exit 1
    fi
done
"${cross}gcc" --version | head -n 1
"$qemu" --version | head -n 1
if ! "$qemu" -machine help | awk '$1 == "cep" { found=1 } END { exit !found }'; then
    printf '%s\n' 'This example needs QEMU with the cep machine; standard QEMU does not include it.' >&2
    exit 1
fi
mkdir -p build/tmp
probe=$(mktemp -d build/tmp/check.XXXXXX)
trap 'rm -rf "$probe"' EXIT HUP INT TERM
printf '%s\n' 'void probe(void) { __asm__ volatile("csrr a0, mcause"); }' > "$probe/probe.c"
if ! "${cross}gcc" -march="$arch" -mabi=lp64 -ffreestanding -c "$probe/probe.c" -o "$probe/probe.o"; then
    printf '%s\n' 'The selected ISA is unsupported. Use RISCV_ARCH=rv64ima with the bundled GCC 11 compiler.' >&2
    exit 1
fi
printf 'Tools ready for CEP, RISCV_ARCH=%s.\n' "$arch"
