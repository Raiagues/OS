---
layout: default
title: Instalar e executar
---

# Instalar e executar

O caminho recomendado é Ubuntu 24.04 com as ferramentas dos repositórios da distribuição. Debian usa os mesmos nomes de pacotes, mas a validação de CI está configurada para Ubuntu 24.04. Windows pode usar Ubuntu no WSL2. Não é necessário instalar Linux dentro do QEMU nem baixar uma imagem de disco: o próprio projeto é o kernel.

## 1. Instalar as ferramentas

```bash
sudo apt-get update
sudo apt-get install -y make git python3 gcc-riscv64-unknown-elf \
  binutils-riscv64-unknown-elf qemu-system-misc gdb-multiarch
```

| Pacote | Para que serve |
| --- | --- |
| `make` | Executa as regras de compilação |
| `git` | Baixa e versiona os fontes |
| `gcc-riscv64-unknown-elf` | Compila C e assembly para RV64 bare metal |
| `binutils-riscv64-unknown-elf` | Linker, inspeção de ELF e ferramentas de objetos |
| `qemu-system-misc` | Fornece `qemu-system-riscv64` no Ubuntu/Debian |
| `python3` | Executa os testes com timeout e análise da saída |
| `gdb-multiarch` | Depuração remota; opcional para apenas executar |

O GCC nativo (`gcc`) compila para o computador hospedeiro e não substitui o compilador RISC-V. O projeto contém seus próprios headers e funções de biblioteca; não depende de uma libc de Linux RISC-V, OpenSBI ou firmware externo.

## 2. Baixar e conferir o ambiente

```bash
git clone https://github.com/Raiagues/OS.git
cd OS
make check
```

Execute os comandos a partir da pasta que contém este `Makefile`. Se já recebeu a pasta `OS`, não precisa clonar novamente.

## 3. Compilar

```bash
make
```

O resultado principal é `build/virt/kernel.elf`. O build usa RV64IMA com extensão Zicsr, ABI LP64 e modelo de código `medany`. O arquivo é um ELF com símbolos; o nome antigo `kernel.bin` também designava um ELF, não uma imagem binária crua.

A memória do kernel começa em `0x80000000` na plataforma `virt`. São geradas dependências de headers para recompilar arquivos afetados. As flags efetivas são registradas para recompilar quando a configuração muda; `make clean` permite refazer tudo do zero.

## 4. Executar

```bash
make run
```

A execução é equivalente a carregar o ELF no QEMU com máquina `virt`, um hart, `-bios none` e console serial. O timer funciona a 20 interrupções por segundo. A demonstração cria três tarefas, que dormem por 2, 3 e 4 segundos e terminam depois de duas passagens. O processo `idle` continua ativo depois do término das três.

**Para sair:** Ctrl+A, solte as teclas, depois X. O projeto não tem prompt de comandos nem leitura de teclado; digitar no terminal não inicia programas.

## 5. Validar

```bash
make test
```

Os testes usam QEMU sem janela e encerram o emulador ao encontrar o marcador de conclusão. Um timeout limita a espera em caso de travamento. Conferem as mensagens de execução e o autoteste de preempção, sono e término. Um processo em busy-loop precisa ser interrompido pelo timer para que outro execute: isso distingue preempção real de uma demonstração apenas cooperativa. Consulte [validação](validation.html) para os critérios e resultados.

## 6. Depurar com GDB

No primeiro terminal:

```bash
make debug
```

O QEMU inicia pausado. No segundo terminal, dentro do repositório:

```bash
gdb-multiarch build/virt/kernel.elf
```

```gdb
set architecture riscv:rv64
target remote localhost:1234
break kernel_start
continue
break ordonnance
continue
info registers
print table_des_processus[1]
```

Use `step`, `next` e `continue` para avançar. O temporizador pode causar paradas em pontos diferentes do fluxo C. Ao terminar, encerre também o QEMU. A porta de depuração fica vinculada ao loopback; não é necessário abrir portas na rede.

## Ferramentas em outro diretório

É possível informar os executáveis sem alterar o sistema:

```bash
make CROSS_COMPILE=/caminho/toolchain/bin/riscv64-unknown-elf-
make run CROSS_COMPILE=/caminho/toolchain/bin/riscv64-unknown-elf- QEMU=/caminho/qemu-system-riscv64
```

Se usar a toolchain antiga da disciplina, que antecede a separação explícita da extensão Zicsr, compile com `RISCV_ARCH=rv64ima`. Use a mesma configuração em todos os comandos.

```bash
make clean
make CROSS_COMPILE=/matieres/4MMPCSEA/xinul/bin/riscv64-unknown-elf- RISCV_ARCH=rv64ima
make run CROSS_COMPILE=/matieres/4MMPCSEA/xinul/bin/riscv64-unknown-elf- RISCV_ARCH=rv64ima QEMU=/matieres/4MMPCSEA/xinul/bin/qemu-system-riscv64
```

Esses caminhos só existem em máquinas que ainda conservam a instalação da disciplina. A instalação por pacotes continua sendo o procedimento para um computador novo.

## Ambiente da disciplina recuperado neste computador

O wrapper abaixo é útil quando as ferramentas ainda estão instaladas em `/matieres/4MMPCSEA/xinul`:

```bash
sh scripts/course-env.sh make check
sh scripts/course-env.sh make run
sh scripts/course-env.sh make test
```

Ele define o prefixo do compilador, o executável QEMU e as flags antigas apenas para o comando executado. No host x86-64 com `libaio.so.1t64`, prepara um alias local se o QEMU histórico precisar de `libaio.so.1`. Os arquivos auxiliares ficam em `build/host-libs`; não altera bibliotecas do sistema nem exige `sudo`. `COURSE_TOOLS=/outro/caminho sh scripts/course-env.sh make run` seleciona outra instalação.

## Console gráfico original: plataforma CEP

A máquina `cep` é uma modificação usada na disciplina e **não vem no QEMU padrão**. Verifique a instalação original com:

```bash
/caminho/qemu-system-riscv64 -machine help
```

Se a lista contiver `cep`:

```bash
make PLATFORM=cep
make rung PLATFORM=cep QEMU=/caminho/qemu-system-riscv64
```

Para a toolchain antiga, acrescente `CROSS_COMPILE=...` e `RISCV_ARCH=rv64ima` também. O ELF de CEP fica em `build/cep/kernel.elf`; a plataforma tem outro mapa de memória, UART e framebuffer. O comando `make run PLATFORM=cep QEMU=...` permite observar a saída serial sem janela.

As imagens Docker históricas estão registradas na [proveniência](provenance.html). Elas não são uma dependência do caminho recomendado e sua disponibilidade não é presumida.

## Problemas comuns

| Sintoma | O que verificar |
| --- | --- |
| `riscv64-unknown-elf-gcc: not found` | Instale o compilador ou informe `CROSS_COMPILE` |
| `qemu-system-riscv64: not found` | Instale `qemu-system-misc` ou informe `QEMU` |
| `Unsupported machine type: cep` | Use `make run` para `virt`, ou o QEMU customizado para CEP |
| Assembler rejeita `zicsr` | Toolchain antiga: use `RISCV_ARCH=rv64ima` ou instale a versão da distribuição |
| Não aparece nenhuma mensagem em `make debug` | O emulador está pausado: conecte o GDB e execute `continue` |
| Não aparece janela em `make run` | A execução padrão usa o terminal serial |
| Fim das mensagens após alguns segundos | As tarefas terminam; `idle` segue aguardando interrupções |
| `Address already in use` no debug | Outro QEMU pode estar usando a porta 1234 |
| Bibliotecas `.so` ausentes no QEMU antigo | Prefira o QEMU dos pacotes atuais para `virt`; a instalação antiga depende de bibliotecas do seu ambiente original |

O teste de execução precisa de tempo para o relógio emulado avançar. Máquinas muito lentas podem precisar de um timeout maior, conforme a ajuda de `python3 scripts/test_qemu.py --help`.

## Referências da plataforma

- [Emulação RISC-V no QEMU](https://www.qemu.org/docs/master/system/target-riscv.html)
- [Máquina virt do QEMU](https://www.qemu.org/docs/master/system/riscv/virt.html)
