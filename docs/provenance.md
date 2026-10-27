---
layout: default
title: Créditos e proveniência
---

# Créditos e proveniência

O projeto acadêmico original é identificado no histórico local como
`ProjetPCSEF_rodriger`, no grupo `pcsef/riscv/seoc2023/students` da Ensimag.
Os commits das implementações estudantis identificam **Emilly Raiane Rodrigues**
como autora. A estrutura inicial também contém código de apoio e bibliotecas de
terceiros, cujos créditos foram preservados nos arquivos.

## Origem da versão recuperada

A árvore de trabalho usada como base estava em
`final/ProjetPCSEF_rodriger`. Seu último commit é
`3a1eac9b9d9fb007e6b332b6825a0ccfcac5a7ce`, de 14/12/2023.
As modificações locais posteriores de `src/console.c` e `src/console.h` também
foram consideradas. A [reconstrução do histórico](history.html) explica a escolha
e as diferenças entre as cópias disponíveis.

O arquivo [original-files.sha256](original-files.sha256) registra os hashes dos
arquivos de texto presentes em `src` da cópia original, antes das adaptações
deste repositório. Ele inclui o Makefile e os scripts de ligação históricos para
permitir identificar a base exata. Os caminhos do manifesto são relativos à
pasta original `final/ProjetPCSEF_rodriger`.

Esse manifesto é um registro de origem, não um checksum esperado dos fontes
atuais. As correções e adaptações da versão distribuída podem alterar os arquivos.

## Avisos existentes nos fontes

| Arquivos | Créditos e termos declarados nos cabeçalhos originais |
| --- | --- |
| `src/printf.c` | Simon Nieuviarts; University of Utah e Flux Group/OSKit; GNU GPL versão 2; aviso adicional de Carnegie Mellon/Mach |
| `src/doprnt.c`, `src/sprintf.c` | University of Utah e Flux Group/OSKit; GNU GPL versão 2; aviso adicional de Carnegie Mellon/Mach |
| `src/doprnt.h`, `src/strtoul.c` | University of Utah e Flux Group/OSKit; GNU GPL versão 2 |
| `src/stdarg.h`, `src/types.h` | Simon Nieuviarts; GNU GPL versão 2 ou posterior |
| `src/font.c` | Daniel Hepper; referência a Marcel Sondaar e fontes VGA da IBM; declaração de domínio público |
| `src/string.c` | O cabeçalho declara geração a partir de vários arquivos de `usr/klibc/`, seguida de pequenas edições; não declara uma licença específica nesse cabeçalho |

Os avisos completos permanecem nos próprios arquivos. Os créditos acima
descrevem o que foi encontrado; não substituem esses avisos nem afirmam autoria
exclusiva sobre todo o projeto.

Não foi encontrado um arquivo `LICENSE` ou `COPYING` global na versão acadêmica
recuperada. Esta organização não atribui uma nova licença genérica ao conjunto
nem remove as condições individuais presentes nos fontes.

O arquivo [COPYING.GPL-2](https://github.com/Raiagues/OS/blob/main/COPYING.GPL-2)
incluído na raiz contém o texto da GNU GPL versão 2, copiado de
`/usr/share/common-licenses/GPL-2` durante a organização do repositório. Ele
acompanha os componentes cujos avisos citam essa licença; sua inclusão não
atribui uma nova licença global ao projeto. Os arquivos que admitem uma versão
posterior continuam com essa opção expressa em seus próprios cabeçalhos.

## O que pertence ao ambiente original

O Makefile escolar apontava para `/matieres/4MMPCSEA/xinul/bin` e executava
`qemu-system-riscv64 -machine cep`. As configurações também referenciavam as
imagens `sebastienviardot/pcsefriscv:20220709` e
`sebastienviardot/pcsefriscv:202212291`.

Esses nomes documentam o ambiente de 2023. Não constituem uma garantia de
disponibilidade, atualização ou reprodução das imagens hoje. O caminho de
execução com QEMU padrão está documentado no
[guia de instalação](getting-started.html).

O projeto original estava associado a um avaliador externo da disciplina.
Seu código, o ponto de entrada `entry_test` e o serviço de avaliação não estão
incluídos aqui. Badges históricos e configurações de integração com esse
serviço não foram tratados como testes locais.

## Escopo da publicação

O repositório contém os fontes organizados, documentação, procedimentos de
compilação e validação. O GitHub Pages apresenta essa documentação; ele não
executa o kernel RISC-V ou um servidor QEMU. Os detalhes estão no
[guia de publicação](publishing.html).
