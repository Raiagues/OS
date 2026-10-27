# OS — um pequeno kernel RISC-V

Projeto de sistema operacional desenvolvido em C e assembly RISC-V na disciplina PCSEF / SEOC da Ensimag, em 2023. O kernel implementa console, interrupções de timer, troca de contexto, escalonamento round-robin, suspensão e término de processos.

Este repositório organiza a versão final da disciplina e inclui correções para executá-la hoje. O caminho principal usa **QEMU padrão, máquina `virt`, com saída no terminal**. O console gráfico original continua disponível para quem tem o QEMU modificado com a máquina `cep`.

## Rodar no Ubuntu 24.04 / Debian

No Windows, execute os mesmos comandos dentro de uma distribuição Ubuntu no WSL2.

```bash
sudo apt-get update
sudo apt-get install -y make git python3 gcc-riscv64-unknown-elf \
  binutils-riscv64-unknown-elf qemu-system-misc gdb-multiarch

git clone https://github.com/Raiagues/OS.git
cd OS
make check
make
make run
```

Se você já está na pasta deste repositório, comece em `make check`.
Para sair do QEMU, pressione **Ctrl+A**, solte, depois **X**. `Ctrl+C` é enviado à máquina emulada.

A demonstração cria `idle`, `proc1`, `proc2` e `proc3`. Os três processos imprimem seu nome/PID, dormem por 2, 3 e 4 segundos, respectivamente, e terminam após duas iterações. Ao final, `idle` continua aguardando interrupções: o kernel não encerra sozinho.

```text
OS / RISC-V RV64 / QEMU virt
Timer: 20 Hz | processos: idle + 3 | modo: M
```

As mensagens seguintes mostram criação, execução, sono, despertar e término. A ordem exata depende do escalonamento. Veja o [guia completo](docs/getting-started.md) para depuração, outras opções e solução de problemas.

## Neste computador, com as ferramentas da disciplina

Se a instalação antiga ainda estiver em `/matieres/4MMPCSEA/xinul`, este comando configura os caminhos e executa o kernel sem reinstalar ferramentas:

```bash
sh scripts/course-env.sh make run
```

Para validar com esse mesmo ambiente, use `sh scripts/course-env.sh make test`. O wrapper só altera o ambiente do comando e arquivos gerados dentro de `build/`.

## O que está implementado

| Parte | Implementação |
| --- | --- |
| Inicialização | Boot bare metal em modo máquina, inicialização da pilha e limpeza de BSS |
| Console | UART no QEMU `virt`; framebuffer 1280 × 720 e caracteres 8 × 8 no CEP |
| Temporizador | CLINT de 10 MHz, interrupções a 20 Hz |
| Processos | Quatro posições: `idle` e até três tarefas, cada uma com sua pilha |
| Escalonamento | Round-robin preemptivo, troca de contexto em assembly |
| Ciclo de vida | Estados `RUN`, `RUNNABLE`, `WAIT`, `DIE`; sono e término explícito/implícito |
| Validação | Testes em QEMU e workflow do GitHub Actions |

É um kernel didático: usa um único hart e todas as tarefas compartilham memória e privilégios. Não há shell, sistema de arquivos, chamadas de sistema, modo usuário ou isolamento entre processos. O termo “processo” segue a disciplina; aqui são tarefas internas do kernel.

## Comandos principais

```bash
make              # gera build/virt/kernel.elf
make run          # executa no terminal
make test         # valida demonstração e autoteste no QEMU
make debug        # inicia pausado, GDB em localhost:1234
make clean        # remove somente os artefatos de build
```

Em outro terminal, para depurar:

```bash
gdb-multiarch build/virt/kernel.elf
```

```gdb
target remote localhost:1234
break kernel_start
continue
```

O ELF contém símbolos de depuração. Não execute `kernel.elf` diretamente no Linux: ele é um kernel RISC-V para a máquina emulada.

## Organização

```text
OS/
├── src/                 # kernel, drivers, assembly e biblioteca mínima
├── scripts/             # diagnóstico de ferramentas e testes de execução
├── docs/                # documentação e conteúdo do GitHub Pages
├── .github/workflows/   # compilação e testes automáticos
├── Makefile
└── README.md
```

O código mantém os nomes franceses da disciplina para facilitar a relação com o trabalho original. As pastas intermediárias, o PDF de captura do GitLab e os binários antigos ficaram fora deste repositório.

## Console original

![Console CEP executando a demonstração, com relógio e estados dos processos](docs/assets/cep-demo.png)

Captura do framebuffer real da plataforma CEP durante a validação. A execução padrão em `virt` apresenta os eventos no terminal.

## Documentação

- [Instalação, execução e depuração](docs/getting-started.md)
- [Arquitetura e leitura guiada do código](docs/architecture.md)
- [Etapas do projeto e histórico recuperado](docs/history.md)
- [Testes, resultados e limites da validação](docs/validation.md)
- [Publicar no GitHub e configurar o Pages](docs/publishing.md)
- [Créditos e proveniência dos fontes](docs/provenance.md)

O GitHub Pages pode apresentar estes guias em `https://raiagues.github.io/OS/` depois de habilitado nas configurações do repositório. O kernel é executado localmente no QEMU; o site não inclui um emulador no navegador.

## Origem e créditos

A base é a árvore final de `ProjetPCSEF_rodriger`, commit `3a1eac9` de dezembro de 2023, incluindo as alterações locais de console. A implementação estudantil é de **Emilly Raiane Rodrigues**, sobre a estrutura e o código de apoio da disciplina.

Os avisos de terceiros foram preservados. Há componentes com GPL v2, GPL v2 ou posterior e fonte de caracteres declarada como domínio público. Não se atribui uma licença única nova ao conjunto; veja [proveniência](docs/provenance.md) e os cabeçalhos dos arquivos.
