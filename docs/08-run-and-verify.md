# 8. Run and verify

Run commands from the repository root. The graphical interface needs the course QEMU with its **CEP** machine. Standard distribution QEMU provides the separate `virt` serial backend.

## Get the source

On Ubuntu, install Git if needed, then clone:

```sh
sudo apt-get update
sudo apt-get install -y git
git clone https://github.com/Raiagues/OS.git
cd OS
```

Choose one of the following environments.

## Course tools already installed

The default location is `/matieres/4MMPCSEA/xinul`. With `make` and Python installed:

```sh
sudo apt-get install -y make python3
sh scripts/course-env.sh make check
./run.sh
```

For another location:

```sh
export COURSE_TOOLS=/path/to/xinul
./run.sh
```

That directory must contain `bin/riscv64-unknown-elf-gcc` and `bin/qemu-system-riscv64`. The wrapper selects the historical `rv64ima` toolchain. It keeps temporary files in `build/tmp` unless `TMPDIR` is already set, and prepares local `libaio` compatibility when needed on Ubuntu 24.04 x86-64.

Close the QEMU window to stop. The process example finishes after roughly eight kernel seconds, but `idle` and the clock keep running.

## Course environment in Docker

This path uses Linux/amd64 and an X11 or XWayland desktop. Install Docker following its [Ubuntu instructions](https://docs.docker.com/engine/install/ubuntu/) and confirm that `docker info` works for your user. Then:

```sh
sudo apt-get install -y xserver-xephyr xauth
./scripts/run-container.sh
```

The script downloads a pinned copy of the course image if necessary. It contains Debian 11, GCC 11.1.0, QEMU 6.2.50 and Python 3.9.2. Allow approximately 700 MB for the download and 2 GB for the unpacked image.

Xephyr gives the container its own graphical window and temporary display credential. The container does not receive the main desktop credential. Builds go into `build/container/`; closing the window or pressing Ctrl+C stops the temporary container.

Select an example by name:

```sh
./scripts/run-container.sh pixels
./scripts/run-container.sh console
./scripts/run-container.sh scroll
./scripts/run-container.sh uart
./scripts/run-container.sh clock
```

The launcher was tested on Linux/amd64. Windows and macOS were not tested. Keep free space in `/tmp` for Xephyr's keyboard cache; on the recovery computer, its cache had to be redirected for the GUI check because that partition was full.

## Standard QEMU, serial output

On Ubuntu:

```sh
sudo apt-get install -y make python3 gcc-riscv64-unknown-elf \
  binutils-riscv64-unknown-elf qemu-system-misc
make PLATFORM=virt check
make PLATFORM=virt run
```

Exit with **Ctrl+A**, then **X**. This recompiles for the `virt` memory map and UART. It does not implement the CEP graphical console. The pixel, console and scrolling examples require CEP.

## What to run and observe

| Local command | Expected result | Recorded output |
| --- | --- | --- |
| `./run.sh` | Three tasks sleep and terminate; `idle` continues | [Start](evidence/processes-start.png), [end](evidence/processes-end.png), [log](evidence/processes.txt) |
| `./run.sh DEMO=pixels` | Four colored rows of printable ASCII, then a long paragraph wrapping at 160 columns; static screen | [Screen](evidence/pixels.png), [log](evidence/pixels.txt) |
| `./run.sh DEMO=console` | Short control-character examples | [Screen](evidence/console.png), [log](evidence/console.txt) |
| `./run.sh DEMO=scroll` | 100 lines written; oldest lines scroll away | [Screen](evidence/scroll.png), [log](evidence/scroll.txt) |
| `./run.sh DEMO=uart` | Direct serial output versus mirrored console output | [Screen](evidence/uart.png), [log](evidence/uart.txt) |
| `./run.sh DEMO=clock` | Clock advances with `idle` only | [Screen](evidence/clock.png), [log](evidence/clock.txt) |

`DEMO` is a build option, not a command typed inside the kernel. Executables have separate directories, such as `build/cep-pixels/kernel.elf`. Static examples remain open until you close QEMU.

## Integration tests

With course tools:

```sh
sh scripts/course-env.sh make test
```

With standard packages, use `make test` directly. It runs two `virt` kernels: the normal process example and the self-test. [test_qemu.py](../scripts/test_qemu.py) checks ELF headers, output and timeouts. Logs are saved under `build/virt/` and `build/virt-selftest/`.

The self-test checks formatting bounds, sleep, termination and preemption. For preemption, one task busy-loops without voluntarily yielding; another task must still run. Look for `SELFTEST: ALL PASS`. [Recorded self-test log](evidence/selftest.txt).

The final QEMU `terminating on signal 15` line comes from the test script stopping the emulator after verification.

## Capture the real framebuffer

```sh
sh scripts/course-env.sh python3 scripts/capture_evidence.py
```

Add `--display sdl` to watch the windows. Without local course tools, the same capture works in Docker without a desktop:

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

The script pauses QEMU through QMP, reads the actual 3,686,400 framebuffer bytes at `0x80000000`, converts them to PNG, then resumes. It saves images, UART logs and `report.json` in `build/evidence/`, and runs the integration tests afterward.

The report records versions, source hashes, ELF hashes, capture triggers and output hashes. Images may differ in timing; ELF hashes may differ with compiler versions or debug paths. The tests check behavior rather than requiring a binary-identical build.

To update the committed samples after a code change, run the capture with `--output docs/evidence`. ELF and GDB records are produced separately using the commands in the [boot](03-boot-kernel.md), [clock](06-clock-interrupts.md) and [process](07-processes.md) chapters.

Screenshots show the display at particular moments. UART logs show transmitted bytes. The integration test checks preemption, sleep and exit. These are different observations: a screenshot alone cannot prove scheduling correctness or precise timing. The small guest examples intentionally stay close to the original code.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| `unsupported machine type: cep` | Use the course QEMU or the Docker launcher |
| Output only in the terminal | `virt` and `run-serial` are serial modes; use CEP for graphics |
| Xephyr cannot open | Check the desktop session, `DISPLAY`, installed dependencies and free space in `/tmp` |
| Docker is inaccessible | Check the service, context and user access with `docker info` |
| CSR assembler errors | Use `rv64ima` for the historical compiler, `rv64ima_zicsr` for modern packages |
| Typing does nothing | There is no working keyboard driver or shell in the recovered project |
| GDB says `optimized out` | Rebuild with `OPT=-O0` and debug that ELF |

GitHub Actions tests the modern `virt` build, compiles all CEP examples and captures them with the pinned course image. Its `cep-framebuffer-evidence` artifact contains the generated output.
