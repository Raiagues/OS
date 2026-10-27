# Independent examples

Start with one directory and read its `src/main.c`. Each directory contains the startup assembly, drivers, headers, runtime support, linker script and launchers needed by that example. No example includes or builds source from another directory.

| Order | Project | Start here |
| --- | --- | --- |
| 1 | [Pixels and characters](01-pixels/README.md) | [main.c](01-pixels/src/main.c) |
| 2 | [Console controls](02-console/README.md) | [main.c](02-console/src/main.c) |
| 3 | [Scrolling](03-scroll/README.md) | [main.c](03-scroll/src/main.c) |
| 4 | [UART output](04-uart/README.md) | [main.c](04-uart/src/main.c) |
| 5 | [Clock and interrupts](05-clock/README.md) | [main.c](05-clock/src/main.c) |
| 6 | [Processes](06-processes/README.md) | [main.c](06-processes/src/main.c) |

## Run one example

After [installing the dependencies](../docs/08-run-and-verify.md), choose a directory:

```sh
cd examples/02-console
./scripts/run-container.sh
```

The Docker launcher mounts only that example directory and opens its graphical window. With local CEP tools, use `./run.sh` instead. Close QEMU before running another example.

Each project builds `build/cep/kernel.elf`. Its `evidence/` directory contains real QEMU captures, the UART log and a report identifying the sources and tools used. Modify a short string in `src/main.c`, run it again and compare the result.

## What each directory contains

- `README.md`: commands, reading order and expected output.
- `src/main.c`: the small program to follow first.
- `src/`: only the implementation and support files needed by the example.
- `Makefile`, `run.sh`, `scripts/`: local compilation and launch commands.
- `evidence/`: screenshots, serial output and capture metadata.
- `COPYING.GPL-2`: license text used by the included support components.

The first four examples do not include a scheduler. The clock adds timer handling; the process example combines the console, timer and saved task contexts. The complete kernel remains in the repository's [src directory](../src/).

Use the [project guide](../docs/README.md) for diagrams and detailed explanations of each mechanism.
