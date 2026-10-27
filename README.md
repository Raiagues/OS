# OS — RISC-V

Projeto de sistema operacional em C e assembly RISC-V, desenvolvido na disciplina PCSEF / SEOC da Ensimag em 2023.

A interface original abre em uma **janela do QEMU**: console gráfico de 1280 × 720, relógio e estados dos processos. O kernel implementa interrupções de timer, escalonamento round-robin, sono e término de processos.

## Abrir a interface gráfica

Dentro desta pasta:

```bash
./run.sh
```

O script compila e abre o QEMU com a máquina **CEP**, usada na disciplina. Neste computador, as ferramentas estão em `/matieres/4MMPCSEA/xinul`; não é necessário reinstalá-las. É preciso executar em uma sessão gráfica do Linux. Para sair, feche a janela do QEMU.

A demonstração executa três processos que dormem por 2, 3 e 4 segundos. Depois que eles terminam, o kernel permanece ativo em `idle` e o relógio continua avançando.

## Usar em outro computador

```bash
git clone https://github.com/Raiagues/OS.git
cd OS
```

A interface precisa do **QEMU modificado da disciplina, com suporte à máquina `cep`**, além de `make` e da toolchain RISC-V. O QEMU dos pacotes comuns do Ubuntu não inclui essa máquina.

Se você tem a instalação da disciplina em outro diretório:

```bash
COURSE_TOOLS=/caminho/para/xinul ./run.sh
```

Esse diretório deve conter `bin/riscv64-unknown-elf-gcc` e `bin/qemu-system-riscv64`. O script configura os caminhos e, quando necessário no Ubuntu 24.04 x86-64, prepara a compatibilidade de `libaio` dentro de `build/`, sem alterar bibliotecas do sistema.

## Alternativa pelo terminal

Para estudar o kernel usando o QEMU padrão, há um backend serial separado:

```bash
sudo apt-get update
sudo apt-get install -y make python3 gcc-riscv64-unknown-elf \
  binutils-riscv64-unknown-elf qemu-system-misc
make PLATFORM=virt run
```

Esse modo usa o terminal. Para sair: **Ctrl+A**, depois **X**.

## Testes

Com as ferramentas da disciplina:

```bash
sh scripts/course-env.sh make test
```

Com as ferramentas instaladas pelos pacotes acima:

```bash
make test
```

Os testes verificam boot, criação de tarefas, preempção, sono, término e limites de formatação. O GitHub Actions também executa esses testes. Os logs ficam em `build/virt/` e `build/virt-selftest/`.

## Código

- `src/console.c` e `src/font.c`: interface gráfica e caracteres.
- `src/time.c` e `src/traitant.S`: temporizador e interrupções.
- `src/processus.c` e `src/ctxt.S`: processos e troca de contexto.
- `src/test_student.c`: demonstração e testes.
- `src/crt0.S` e `src/kernel.lds`: boot e disposição da memória.

O projeto usa um hart, com tarefas em modo máquina compartilhando memória. Os nomes franceses das funções foram mantidos.

## Créditos

Implementação estudantil de **Emilly Raiane Rodrigues**, sobre a estrutura da disciplina. A base é a versão final de dezembro de 2023, com correções de execução e compatibilidade.

O código de apoio inclui componentes Flux/OSKit, CMU/Mach, klibc e uma fonte bitmap com créditos a Daniel Hepper, Marcel Sondaar e IBM. Os avisos originais permanecem nos fontes; `COPYING.GPL-2` acompanha os componentes que citam essa licença.
