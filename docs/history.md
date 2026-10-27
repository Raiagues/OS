---
layout: default
title: Histórico e etapas do projeto
---

# Histórico e etapas do projeto

Este repositório organiza a versão final de um projeto de sistema operacional
didático desenvolvido em 2023, no contexto PCSEF/RISC-V/SEOC da Ensimag. O projeto
original se chamava `ProjetPCSEF_rodriger`.

As etapas abaixo foram reconstruídas a partir dos arquivos, dos commits locais e
de uma captura do GitLab. Elas descrevem a evolução observável do código. Não são
uma transcrição dos enunciados da disciplina, que não estavam presentes no
material recuperado.

## Qual versão foi usada

A base escolhida foi a árvore de trabalho da pasta
`final/ProjetPCSEF_rodriger`, com o último commit
`3a1eac9b9d9fb007e6b332b6825a0ccfcac5a7ce`, de 14 de dezembro de 2023,
intitulado **Ajuste timer**. Havia alterações locais posteriores em `README.md`,
`src/console.c` e `src/console.h`; os arquivos de console foram considerados na
recuperação, inclusive seus comentários adicionais.

As outras pastas eram cópias de estágios anteriores:

| Cópia local | Último commit | Data |
| --- | --- | --- |
| `ProjetPCSEF_rodriger` | `79474f6` | 30/11/2023 |
| `TP5/ProjetPCSEF_rodriger` | `15d388f` | 02/12/2023 |
| `TP5/PIPELINE CORRETA/ProjetPCSEF_rodriger` | `cdfd2c6` | 03/12/2023 |
| `TP5/OUTRA PIPELINE DE CERTO PFVR EU TE IMPLORO/ProjetPCSEF_rodriger` | `e6d6689` | 05/12/2023 |
| `final/ProjetPCSEF_rodriger` | `3a1eac9` | 14/12/2023 |

Os hashes identificam o histórico local original. Não pressupõem que esses
commits estejam disponíveis no histórico deste novo repositório GitHub.

## 1. Estrutura inicial e inicialização

O commit inicial, `352e9f4`, de 24/09/2023, recebeu o nome **Point de départ**.
A base fornecia o código de entrada em assembly, o script de ligação, uma
biblioteca C reduzida, definições de hardware e o Makefile do ambiente escolar.

O aprendizado parte do executável carregado pelo QEMU: preparar a pilha, entrar
em C e acessar periféricos por endereços de memória. Não há Linux executando
dentro da máquina emulada.

## 2. Console, caracteres e rolagem

Os commits de outubro desenvolvem `console.c`. O texto é desenhado em um
framebuffer com fonte de 8 × 8 pixels, incluindo cursor, quebra de linha,
tabulação, retorno de carro, backspace, limpeza da tela e rolagem.

A captura preservada em `TP1` foi salva em 25/10/2023 e mostra uma avaliação de
19/10/2023 com os testes de console marcados como **ok**. Na mesma captura, timer
e processos ainda estavam marcados como **TODO**. Ela registra esse momento
específico; não demonstra o resultado da entrega final.

## 3. Temporizador e interrupções

Os commits de 29/10/2023, identificados como **Seance 2**, acrescentam a etapa de
temporização. O código configura `mtvec`, habilita a interrupção de timer e usa
os registradores `mtime` e `mtimecmp` do CLINT.

O tratador em assembly preserva o contexto necessário para chamar C e retornar
com `mret`. O contador de interrupções permite apresentar o tempo decorrido e,
mais tarde, acionar o escalonador.

## 4. Processos e troca de contexto

Em 26/11/2023 aparecem os commits **Seance processus 1** (`0df81eb`) e
**Add assembly** (`1b2c495`). A tabela de processos passa a manter PID, nome,
estado, pilha e registradores salvos.

O escalonador escolhe um processo apto a executar; `ctx_sw`, em assembly,
preserva o contexto do processo atual e restaura o próximo. As tarefas têm
pilhas próprias, mas compartilham memória e privilégios do kernel.

## 5. Espera e despertar

O commit **Include dors** (`663e39f`), de 02/12/2023, registra a evolução da
função `dors`. Um processo pode solicitar uma espera em segundos e liberar a
CPU. O escalonador acompanha o prazo de despertar e o torna executável de novo.

Os vários commits de 2 a 5 de dezembro ajustam inicialização, estados,
escalonamento e integração com a avaliação escolar. Seus nomes não são, por si
só, comprovação de que uma avaliação tenha sido aprovada.

## 6. Encerramento de processos

O commit **Terminasion implicite** (`2a88ec3`), de 14/12/2023, trata do
encerramento quando a função da tarefa retorna. A função intermediária
`proc_launcher` executa a tarefa e chama `fin_processus` ao final. O
encerramento explícito também pode ser solicitado pela própria tarefa.

Embora existam commits com nomes relacionados a teclado e outras interrupções,
o material final não oferece uma interface de teclado funcional nem um shell.
Esses títulos não devem ser usados como lista de funcionalidades implementadas.

## Organização para execução fora da disciplina

A recuperação mantém os módulos e nomes das funções do projeto, documenta as
dependências e acrescenta um caminho de execução com a máquina `virt` do QEMU
padrão. O console gráfico original pertence à máquina `cep`, uma extensão usada
no ambiente da disciplina.

O novo repositório reúne uma única versão dos fontes. As cópias intermediárias,
os objetos compilados, o executável antigo, o swap do editor, a captura de tela
em PDF e os diretórios `.git` anteriores permanecem fora dele.

As adaptações também corrigem problemas encontrados na versão recuperada:

- A inicialização passa a preparar `gp`, zerar `.bss` e configurar uma pilha
  alinhada antes de chamar C.
- A tabela de processos deixa de escrever além de sua última entrada; a
  demonstração cria os três processos uma única vez, sem apagá-los com uma
  segunda chamada de inicialização.
- A troca de contexto preserva `mstatus`, e as regiões que alteram estados de
  tarefas protegem-se contra interrupções durante a atualização.
- A leitura de `mcause` usa 64 bits e distingue a interrupção de timer de traps
  inesperadas, que produzem uma mensagem de diagnóstico.
- O timer é programado antes de habilitar interrupções; a criação de tarefas
  valida argumentos e limita a cópia do nome.

Essas mudanças fazem parte da recuperação para distribuição. Elas não são
atribuídas retroativamente à entrega acadêmica de 2023.

O antigo README dependia de badges hospedados pela disciplina. A CI do GitLab
disparava um avaliador externo com variáveis `TRIGGER_TOKEN` e
`EVAL_PROJECT_ID`. O alvo original `test.bin` exigia o ponto de entrada
`entry_test`, ausente nos fontes recuperados. Portanto, o avaliador escolar não
faz parte deste repositório; os testes atuais e seus resultados estão descritos
na [validação](validation.html).

Consulte também a [arquitetura](architecture.html), os
[créditos e a proveniência](provenance.html) e o
[guia de instalação e execução](getting-started.html).
