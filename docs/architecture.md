---
layout: default
title: Arquitetura do kernel
---

# Arquitetura do kernel

O projeto é um kernel didático escrito em C e assembly RISC-V de 64 bits.
Executa diretamente na máquina emulada pelo QEMU, em modo máquina (**M-mode**),
com um único hart, isto é, um único contexto de execução de CPU.

As tarefas chamadas de processos no código possuem pilhas e contextos próprios,
mas compartilham o mesmo espaço de memória e os privilégios do kernel. Isso
permite estudar escalonamento e interrupções sem implementar processos isolados
como os de um sistema operacional de uso geral.

## Duas plataformas

| Aspecto | `virt`: execução padrão | `cep`: plataforma da disciplina |
| --- | --- | --- |
| Emulador | QEMU padrão com suporte a RISC-V | QEMU modificado com a máquina `cep` |
| Endereço do kernel | `0x80000000` | `0x1000` |
| Saída principal | Terminal serial | Console gráfico e serial |
| UART | NS16550A em `0x10000000` | SiFive em `0x10013000` |
| Framebuffer | Não utilizado | `0x80000000`, 1280 × 720 pixels |
| Timer | CLINT | CLINT |

A escolha de plataforma altera o script de ligação por meio de `KERNEL_BASE`,
seleciona a implementação do console e define `PLATFORM_CEP` quando necessário.
O endereço `0x80000000` tem usos diferentes nas duas máquinas: RAM do kernel em
`virt` e framebuffer em `cep`. Por isso, trocar apenas o argumento `-machine`
do QEMU não torna os executáveis intercambiáveis.

No caminho `virt`, as funções gráficas sem equivalente serial não acessam o
framebuffer. A atividade das tarefas e seu tempo de execução aparecem nas
mensagens do terminal. O relógio e a tabela de estados desenhados na primeira
linha da tela pertencem ao console CEP.

## Inicialização

O fluxo principal é:

```text
QEMU carrega o ELF
    → entry, em crt0.S
    → kernel_start, em start.c
    → inicialização de UART e console
    → criação das tarefas e configuração do timer
    → idle / tarefas / interrupções do timer
```

O ponto de entrada desabilita interrupções, mantém harts adicionais em espera,
prepara o registrador global `gp` e a pilha inicial e zera a seção `.bss` antes
de chamar C. O linker reúne código, dados constantes, dados inicializados e
variáveis zeradas nos endereços da plataforma escolhida.

O kernel é *freestanding*: utiliza as rotinas de suporte incluídas no próprio
projeto, como `printf`, `sprintf` e operações de strings. Não depende de uma
libc hospedada em Linux dentro da máquina emulada.

## Temporizador e tratamento de interrupções

O timer usa os registradores de memória `mtime` e `mtimecmp`, respectivamente
em `0x0200bff8` e `0x02004000`, para o hart 0. O código considera uma base de
tempo de 10 MHz e configura **20 interrupções por segundo**, com intervalo
nominal de 50 ms.

`mtvec` aponta para `mon_traitant`, em `traitant.S`. Ao receber uma interrupção,
o tratador salva registradores na pilha, lê a causa em `mcause` e chama a parte
em C. Uma interrupção de timer atualiza a contagem de tempo, programa a próxima
comparação e oferece ao escalonador uma oportunidade de trocar de tarefa.

O retorno ao código interrompido usa `mret`. A camada de contexto preserva o
estado necessário para que uma tarefa interrompida possa continuar do ponto em
que parou. `nbr_secondes()` fornece o tempo em segundos inteiros; ele não é um
relógio de calendário.

## Tarefas e escalonamento

A tabela tem quatro entradas: `idle`, com PID 0, e até três tarefas de
demonstração. Cada registro mantém PID, nome, estado, contexto salvo, pilha e
informações de despertar. A pilha reservada por tarefa tem 4096 elementos de
64 bits, ou **32 KiB**, alinhada a 16 bytes. `idle` inicia na pilha de boot,
de 16 KiB; seu ponteiro de pilha é salvo na primeira troca de contexto.

| Estado no código | Significado |
| --- | --- |
| `RUN` | A tarefa está executando |
| `RUNNABLE` | A tarefa está pronta para executar |
| `WAIT` | A tarefa aguarda seu prazo de despertar |
| `DIE` | A tarefa terminou ou a entrada está livre |

O escalonamento é circular, sem prioridades. O escalonador procura tarefas
prontas, incluindo `idle` no ciclo. `idle` permanece disponível mesmo quando
todas as outras tarefas estão esperando ou terminaram. Sua espera utiliza
`wfi`, que permite aguardar uma interrupção.

Há duas situações de troca de tarefa: o timer interrompe a execução, ou uma
tarefa chama uma função como `dors` ou `fin_processus`. A troca de contexto em
`ctxt.S` salva os registradores necessários e troca a pilha ativa. As regiões
que alteram estados compartilhados precisam impedir que uma interrupção
observe uma atualização incompleta.

O contexto da tarefa contém 18 valores: `ra`, `sp`, `gp`, `tp`, `s0` a `s11`,
`mepc` e `mstatus`. O tratador de interrupção preserva adicionalmente os
registradores temporários e de argumentos na pilha da tarefa. As interrupções
permanecem desabilitadas durante a troca de pilha; o código que retoma a execução
restaura seu estado anterior. Uma tarefa iniciada por `proc_launcher` habilita
interrupções antes de chamar sua função, permitindo preempção desde a primeira
execução.

## API usada pelas tarefas

| Função | Uso |
| --- | --- |
| `init_proc()` | Inicializar a tabela e estabelecer `idle` |
| `cree_processus(code, nom)` | Criar uma tarefa a partir de uma função C e retornar seu PID; retorna `-1` se a criação falhar |
| `mon_pid()` / `mon_nom()` | Consultar a identidade da tarefa ativa |
| `ordonnance()` | Executar a seleção da próxima tarefa |
| `dors(segundos)` | Suspender a tarefa até o prazo em segundos |
| `fin_processus()` | Encerrar explicitamente a tarefa ativa |
| `nbr_secondes()` | Consultar os segundos inteiros decorridos |

Uma tarefa também pode terminar retornando de sua função C. O ponto de partida
intermediário `proc_launcher` chama essa função e encaminha seu retorno para
`fin_processus`. O kernel continua executando `idle` após o encerramento das
tarefas; a ausência de novas mensagens não significa que o QEMU tenha encerrado.

As esperas usam o relógio de segundos inteiros do kernel. O pedido de espera não
constitui uma garantia de tempo real nem um temporizador de alta resolução.
`dors(0)` apenas oferece uma oportunidade de escalonamento. As entradas de
tarefas encerradas não são recicladas: após criar as três tarefas disponíveis,
uma nova criação retorna `-1`, mesmo que alguma delas já tenha terminado.

Na demonstração padrão, `proc1`, `proc2` e `proc3` executam duas iterações,
intercaladas com esperas de 2, 3 e 4 segundos, respectivamente. `proc2` chama
`fin_processus` explicitamente; as outras duas retornam de suas funções. A
mensagem `DEMO COMPLETE` aparece depois que as três atingem `DIE`, normalmente
em torno de oito segundos do relógio do kernel.

## Console

`printf` encaminha texto para `console_putbytes`. No QEMU `virt`,
`console_serial.c` envia os bytes à UART; a escrita usa consulta do estado do
transmissor, sem interrupções de UART. A saída converte quebra de linha para o
formato esperado pelo terminal serial.

No CEP, `console.c` desenha caracteres a partir de `font.c` e também transmite
texto pela UART. As rotinas gráficas tratam cursor, tabulação, limpeza de tela e
rolagem. Essa implementação é parte do trabalho acadêmico preservado; sua
validação deve ser distinguida da execução em `virt`.

## Mapa dos fontes

| Arquivo ou grupo | Responsabilidade |
| --- | --- |
| `src/crt0.S`, `src/kernel.lds` | Entrada, pilha inicial, limpeza de `.bss` e disposição do executável |
| `src/start.c` | Inicialização e entrada na demonstração |
| `src/test_student.c` | Tarefas de demonstração e cenários executados no kernel |
| `src/processus.c`, `src/processus.h` | Tabela de tarefas, estados, espera e escalonamento |
| `src/ctxt.S` | Troca de contexto |
| `src/traitant.S`, `src/time.c`, `src/time.h` | Entrada de traps, temporizador e contagem de tempo |
| `src/cpu.h`, `src/cep_platform.h` | Operações de CPU e definições de hardware |
| `src/console_serial.c` | Console serial para `virt` |
| `src/console.c`, `src/font.c` | Console gráfico CEP e fonte bitmap |
| `src/uart.c`, `src/uart.h` | Transmissão serial específica de cada plataforma |
| `src/printf.c`, `src/doprnt.c`, `src/sprintf.c`, `src/string.c` e cabeçalhos de suporte | Biblioteca C reduzida incluída no kernel |

Os identificadores em francês, como `ordonnance`, `dors` e `cree_processus`,
foram mantidos para facilitar a relação com o material original.

## Limites do projeto

O kernel não implementa modo usuário, isolamento de memória, chamadas de
sistema, sistema de arquivos, rede, shell ou entrada de teclado funcional. Não
há suporte a múltiplos harts executando o escalonador em paralelo. A memória das
tarefas é reservada estaticamente.

O registro de [validação](validation.html) informa o que foi efetivamente
testado. Para executar e depurar, siga o
[guia de instalação](getting-started.html); para entender a origem das etapas,
consulte o [histórico](history.html).
