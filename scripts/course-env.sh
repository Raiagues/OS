#!/bin/sh
# Environment for the original Ensimag QEMU CEP installation on Linux x86_64.
set -eu
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$repo_dir"
# Keep compiler and capture temporary files with the other build artifacts.
if [ -z "${TMPDIR-}" ]; then
    mkdir -p build/tmp
    export TMPDIR="$repo_dir/build/tmp"
fi
course_tools=${COURSE_TOOLS:-/matieres/4MMPCSEA/xinul}
export CROSS_COMPILE="$course_tools/bin/riscv64-unknown-elf-"
export QEMU="$course_tools/bin/qemu-system-riscv64"
export RISCV_ARCH=rv64ima
for tool in "${CROSS_COMPILE}gcc" "$QEMU"; do
    if [ ! -x "$tool" ]; then
        printf 'Course tool not found: %s\nSet COURSE_TOOLS or use the normal installation from README.md.\n' "$tool" >&2
        exit 1
    fi
done
# Ubuntu 24.04 renamed libaio even on x86_64, where time_t was already 64-bit.
# Confine the compatibility name to this repository; never change /usr/lib.
dependencies=$(ldd "$QEMU" 2>&1 || true)
if printf '%s\n' "$dependencies" | awk '$1 == "libaio.so.1" && $2 == "=>" && $3 == "not" { found=1 } END { exit !found }'; then
    if [ "$(uname -m)" = x86_64 ]; then
        aio=$(ldconfig -p 2>/dev/null | awk '$1 == "libaio.so.1t64" && /x86-64/ { print $NF; exit }')
        if [ -n "$aio" ] && [ -f "$aio" ]; then
            mkdir -p build/host-libs
            ln -sfn "$aio" build/host-libs/libaio.so.1
            export LD_LIBRARY_PATH="$repo_dir/build/host-libs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
        fi
    fi
fi
if [ "$#" -eq 0 ]; then
    set -- make run
fi
exec "$@"
