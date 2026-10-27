#!/bin/sh
# Open the graphical console developed for the original CEP platform.
set -eu
repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$repo_dir"
course_tools=${COURSE_TOOLS:-/matieres/4MMPCSEA/xinul}
if [ -x "$course_tools/bin/qemu-system-riscv64" ] && \
   [ -x "$course_tools/bin/riscv64-unknown-elf-gcc" ]; then
    exec sh scripts/course-env.sh make PLATFORM=cep run "$@"
fi
# An independently installed custom QEMU can also provide the CEP machine.
qemu=${QEMU:-qemu-system-riscv64}
if command -v "$qemu" >/dev/null 2>&1 && \
   "$qemu" -machine help 2>/dev/null | awk '$1 == "cep" { found=1 } END { exit !found }'; then
    exec make PLATFORM=cep run "$@"
fi
printf '%s\n' \
  'A interface grafica precisa do QEMU da disciplina, com a maquina cep.' \
  'O QEMU comum dos pacotes Ubuntu nao inclui essa maquina.' \
  'Informe sua instalacao: COURSE_TOOLS=/caminho/xinul ./run.sh' >&2
exit 1
