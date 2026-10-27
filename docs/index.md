---
layout: default
title: OS · Kernel RISC-V
---

# Um kernel, do boot aos processos

Projeto de sistema operacional em **C e assembly RISC-V**, desenvolvido por **Emilly Raiane Rodrigues** na disciplina PCSEF / SEOC da Ensimag, em 2023.

A versão final reúne console, temporizador, interrupções, escalonamento e ciclo de vida de processos. Esta publicação recupera o projeto e documenta como compilá-lo e executá-lo no QEMU atual.

[Começar pela instalação](getting-started.html) · [Ver código no GitHub](https://github.com/Raiagues/OS)

## Executar no seu computador

```bash
sudo apt-get update
sudo apt-get install -y make git python3 gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf qemu-system-misc
git clone https://github.com/Raiagues/OS.git
cd OS
make
make run
```

O terminal mostra três tarefas alternando execução, dormindo e terminando. Use **Ctrl+A, depois X** para sair. O guia explica também depuração com GDB e o console gráfico da plataforma original.

## Explorar o projeto

| Guia | Conteúdo |
| --- | --- |
| [Instalar e executar](getting-started.html) | Dependências, comandos, demonstração e depuração |
| [Arquitetura](architecture.html) | Boot, memória, timer, contexto e escalonador |
| [Histórico](history.html) | Etapas reconstruídas a partir dos fontes e commits |
| [Validação](validation.html) | O que é testado e quais são as limitações |
| [Publicação](publishing.html) | Repositório e configuração do GitHub Pages |
| [Créditos](provenance.html) | Origem acadêmica e componentes de terceiros |

## Escopo

É um kernel didático de um único hart, executado em modo máquina. As tarefas compartilham memória; não há shell, sistema de arquivos ou modo usuário. Este site contém a documentação. Para executar o kernel, use o QEMU localmente.
