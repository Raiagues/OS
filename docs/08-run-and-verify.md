# 8. Run and verify

The repository root contains the complete kernel. Six numbered folders under `examples/` are independent projects, each with its own sources, build files and launchers. Start the setup commands at the repository root; enter an example folder when running that example.

Use **CEP** for the graphical window or **`virt`** for a terminal-only run of the complete kernel with standard QEMU. The Docker launcher includes the compiler and custom QEMU build needed by CEP.

## Get the source

On Ubuntu, install Git if needed, then clone:

```sh
sudo apt-get update
sudo apt-get install -y git
git clone https://github.com/Raiagues/OS.git
cd OS
```

For the graphical display on a new Linux setup, follow the Docker steps below. The other options use tools installed directly on the host.

## Graphical run with Docker

This path uses Linux/amd64 and an X11 or XWayland desktop. Install Docker following its [Ubuntu instructions](https://docs.docker.com/engine/install/ubuntu/) and confirm that `docker info` works for your user. Then:

```sh
sudo apt-get install -y xserver-xephyr xauth
./scripts/run-container.sh
```

The script downloads a pinned toolchain image if necessary. It contains Debian 11, GCC 11.1.0, QEMU 6.2.50 and Python 3.9.2. Allow approximately 700 MB for the download and 2 GB for the unpacked image.

Xephyr gives the container its own graphical window and temporary display credential. The container does not receive the main desktop credential. Builds go into `build/container/`; closing the window or pressing Ctrl+C stops the temporary container.

The command above runs the complete kernel. To run a small project, enter its folder and use that folder’s launcher:

```sh
cd examples/02-console
./scripts/run-container.sh
```

Each launcher takes no example argument and runs the project in its own folder. The [example list below](#what-to-run-and-observe) identifies all six folders.

The launcher was tested on Linux/amd64. Windows and macOS were not tested. Xephyr needs free space in `/tmp` for its keyboard cache.

## Standard QEMU, serial output

On Ubuntu:

```sh
sudo apt-get install -y make python3 gcc-riscv64-unknown-elf \
  binutils-riscv64-unknown-elf qemu-system-misc
make PLATFORM=virt check
make PLATFORM=virt run
```

Exit with **Ctrl+A**, then **X**. This recompiles for the `virt` memory map and UART. It does not implement the CEP graphical console. The six numbered graphical examples target CEP.

## Local CEP toolchain

If you have the CEP compiler and QEMU installed directly, install the host tools:

```sh
sudo apt-get install -y make python3
```

Point the launcher at the toolchain directory when it is outside its default location. For example:

```sh
export COURSE_TOOLS=/opt/cep
sh scripts/course-env.sh make check
./run.sh
```

Replace `/opt/cep` with the directory containing `bin/riscv64-unknown-elf-gcc` and `bin/qemu-system-riscv64`. `COURSE_TOOLS` is the environment variable accepted by the launcher. If the launcher already finds both tools, you can omit the export.

The wrapper selects `rv64ima` for GCC 11, keeps temporary files in `build/tmp` unless `TMPDIR` is set, and prepares local `libaio` compatibility when needed on Ubuntu 24.04 x86-64.

Close the QEMU window to stop. In the complete kernel, the three tasks finish after roughly eight kernel seconds, while `idle` and the clock keep running. The same setup also works inside each numbered example folder: its local `run.sh` finds the tools and builds that project.

## What to run and observe

At the repository root, `./run.sh` (local tools) or `./scripts/run-container.sh` (Docker) runs the complete kernel.

Each numbered folder is a complete, separate project. To use local CEP tools, start from the repository root and run:

```sh
cd examples/02-console
./run.sh
```

With Docker, run `./scripts/run-container.sh` from the same folder. Open its `src/main.c` to read the example from `kernel_start` onward. Each example contains its own drivers, runtime, headers, boot assembly and linker script; copying one folder is enough to take its source with you.

| Project folder | Expected result | Recorded output |
| --- | --- | --- |
| [01-pixels](../examples/01-pixels/README.md) | Four static rows of printable ASCII in different colors, then a paragraph wrapping at 160 columns | [Screen](evidence/pixels.png), [log](evidence/pixels.txt) |
| [02-console](../examples/02-console/README.md) | Short control-character examples | [Screen](evidence/console.png), [log](evidence/console.txt) |
| [03-scroll](../examples/03-scroll/README.md) | 100 lines written; oldest lines scroll away | [Screen](evidence/scroll.png), [log](evidence/scroll.txt) |
| [04-uart](../examples/04-uart/README.md) | Direct serial output versus mirrored console output | [Screen](evidence/uart.png), [log](evidence/uart.txt) |
| [05-clock](../examples/05-clock/README.md) | Timer interrupts and a clock without a scheduler | [Screen](evidence/clock.png), [log](evidence/clock.txt) |
| [06-processes](../examples/06-processes/README.md) | Three tasks sleep and terminate; `idle` continues | [Start](evidence/processes-start.png), [end](evidence/processes-end.png), [log](evidence/processes.txt) |

For example, building `examples/02-console` creates `examples/02-console/build/cep/kernel.elf`. Its captured image, UART log and report are in its own `evidence/` directory. Static examples remain visible until QEMU closes.

## Optional debugger

The boot, timer and process chapters use GDB to inspect registers and memory. Install it on the host:

```sh
sudo apt-get install -y gdb-multiarch
```

The debugger commands in those chapters operate on the complete kernel at the repository root. With the local CEP toolchain, `make debug` starts QEMU paused and listens on port 1234.

## Integration tests

With the local CEP toolchain:

```sh
sh scripts/course-env.sh make test
```

With standard packages, use `make test` directly from the repository root. It runs two `virt` kernels: the complete kernel and the independent [kernel test project](../tests/kernel/Makefile). [test_qemu.py](../scripts/test_qemu.py) checks ELF headers, output and timeouts. The complete kernel’s ELF and log are under `build/virt/`; the self-test’s ELF and log are under `tests/kernel/build/virt/`.

The self-test checks formatting bounds, sleep, termination and preemption. For preemption, one task busy-loops without voluntarily yielding; another task must still run. Look for `SELFTEST: ALL PASS`. [Recorded self-test log](evidence/selftest.txt).

The final QEMU `terminating on signal 15` line comes from the test script stopping the emulator after verification.

## Capture the real framebuffer

```sh
sh scripts/course-env.sh python3 scripts/capture_evidence.py
```

Add `--display sdl` to watch the windows. The same capture works in Docker without a desktop:

```sh
docker run --rm --platform linux/amd64 --ulimit core=0 \
  --tmpfs /tmp:rw,size=64m --user "$(id -u):$(id -g)" \
  --mount "type=bind,source=$PWD,target=/workspace" \
  --workdir /workspace --entrypoint sh \
  sebastienviardot/pcsefriscv@sha256:701a53695beefb420e003b39c18c04f3c43de67cddbcf8ab534fa8378855e250 \
  -ec '
    export CROSS_COMPILE=/matieres/4MMPCSEA/xinul/bin/riscv64-unknown-elf-
    export QEMU=/matieres/4MMPCSEA/xinul/bin/qemu-system-riscv64
    export RISCV_ARCH=rv64ima
    python3 scripts/capture_evidence.py
  '
```

The script builds each numbered project in its own folder. It pauses QEMU through QMP, reads the actual 3,686,400 framebuffer bytes at `0x80000000`, converts them to PNG, then resumes. By default, images, UART logs and `report.json` go only into `build/evidence/`. The integration tests run afterward.

The report records versions, source hashes, ELF hashes, capture triggers and output hashes. Images may differ in timing; ELF hashes may differ with compiler versions or debug paths. The tests check behavior rather than requiring a binary-identical build.

To refresh the documentation captures and each example’s own `evidence/` folder after a code change, run from the repository root:

```sh
sh scripts/course-env.sh python3 scripts/capture_evidence.py \
  --output docs/evidence --update-examples
```

ELF and GDB records are produced separately using the commands in the [boot](03-boot-kernel.md), [clock](06-clock-interrupts.md) and [process](07-processes.md) chapters.

Screenshots show the display at particular moments. UART logs show transmitted bytes. The integration test checks preemption, sleep and exit. These are different observations: a screenshot alone cannot prove scheduling correctness or precise timing. Each small project runs its own local implementation, which can be inspected from `src/main.c` through its drivers.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| `unsupported machine type: cep` | Use the custom CEP QEMU build or the Docker launcher |
| Output only in the terminal | `virt` and `run-serial` are serial modes; use CEP for graphics |
| Xephyr cannot open | Check the desktop session, `DISPLAY`, installed dependencies and free space in `/tmp` |
| Docker is inaccessible | Check the service, context and user access with `docker info` |
| CSR assembler errors | Use `rv64ima` for GCC 11, `rv64ima_zicsr` for modern packages |
| Typing does nothing | The kernel does not implement keyboard input or a shell |
| GDB says `optimized out` | Rebuild with `OPT=-O0` and debug that ELF |

GitHub Actions tests the modern `virt` build, compiles all CEP examples and captures them with the pinned toolchain image. Its `cep-framebuffer-evidence` artifact contains the generated output.
