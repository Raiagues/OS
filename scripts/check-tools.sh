#!/bin/sh
set -eu
cross=${1:-riscv64-unknown-elf-}
qemu=${2:-qemu-system-riscv64}
platform=${3:-virt}
arch=${4:-rv64ima_zicsr}
python=${5:-python3}
missing=0
for tool in "${cross}gcc" "${cross}readelf" "${cross}size" "$qemu" "$python"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'Missing tool: %s\n' "$tool" >&2
        missing=1
    fi
done
if [ "$missing" -ne 0 ]; then
    printf '%s\n' 'Ubuntu/Debian: sudo apt install make gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf qemu-system-misc python3' >&2
    exit 1
fi
"${cross}gcc" --version | head -n 1
"$qemu" --version | head -n 1
"$python" --version
if ! "$qemu" -machine help | awk -v machine="$platform" '$1 == machine { found = 1 } END { exit !found }'; then
    printf 'QEMU does not provide machine %s. CEP requires the custom course build.\n' "$platform" >&2
    exit 1
fi
probe=$(mktemp -d)
trap 'rm -rf "$probe"' EXIT HUP INT TERM
printf '%s\n' 'void probe(void) { __asm__ volatile("csrr a0, mcause"); }' > "$probe/probe.c"
if ! "${cross}gcc" -march="$arch" -mabi=lp64 -ffreestanding -c "$probe/probe.c" -o "$probe/probe.o"; then
    printf '%s\n' 'The toolchain cannot compile the selected ISA. For the historical GCC 11 course toolchain only, use RISCV_ARCH=rv64ima.' >&2
    exit 1
fi
printf 'Tools ready for PLATFORM=%s, RISCV_ARCH=%s.\n' "$platform" "$arch"
