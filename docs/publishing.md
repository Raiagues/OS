---
layout: default
title: Publicar no GitHub
---

# Publicar no GitHub e no Pages

O repositório de destino é [Raiagues/OS](https://github.com/Raiagues/OS). A pasta `OS` é a raiz do projeto; as pastas da disciplina ao lado dela não precisam ser enviadas.

## Código e atualizações

Dentro da pasta `OS`, verifique o estado antes de enviar uma alteração:

```bash
git status
git remote -v
make test
git add README.md docs src scripts Makefile .github .gitignore
git commit -m "Descreve a alteração"
git push origin main
```

Adicione somente os caminhos que você realmente alterou. O `.gitignore` exclui build, objetos, logs e arquivos temporários. Se o Git pedir autenticação, use seu método habitual de acesso ao GitHub; não grave tokens no repositório.

O workflow de CI compila e executa os testes no QEMU padrão. O resultado aparece na aba **Actions**. Ele não depende dos tokens, runners ou avaliadores da universidade.

## Habilitar GitHub Pages

Este projeto já contém Markdown com front matter e configuração Jekyll em `docs/`. Para habilitar a publicação:

1. Abra [Settings → Pages](https://github.com/Raiagues/OS/settings/pages).
2. Em **Build and deployment → Source**, escolha **GitHub Actions**.
3. Abra **Actions → Kernel CI** e execute **Run workflow** no branch `main`, ou envie uma alteração para esse branch.
4. O workflow compila o kernel, executa os testes e gera o site a partir de `docs/`. O job `deploy-pages` publica somente depois que as verificações passam.
5. Depois que o deploy terminar, abra [raiagues.github.io/OS](https://raiagues.github.io/OS/).

A publicação exige a configuração inicial do Pages por alguém com permissão de administração/manutenção. O workflow usa o token temporário do GitHub Actions; não precisa de um token pessoal nos secrets. A presença dos arquivos no GitHub, sozinha, não comprova que o site já está publicado. O endereço pode retornar 404 enquanto o Pages estiver desabilitado ou o primeiro build estiver pendente.

O Jekyll transforma os arquivos `.md` em páginas `.html`. Os links internos da documentação usam essa extensão e o layout usa `relative_url` para respeitar o prefixo `/OS`. Para ler os fontes antes da publicação, use os links `.md` do README.

## O que o site oferece

O Pages apresenta instalação, arquitetura, histórico, créditos e validação. A execução do kernel continua no QEMU do computador de quem clonou o projeto. GitHub Pages hospeda arquivos estáticos e não executa um processo QEMU no servidor. Uma versão executável no navegador exigiria portar ou integrar um emulador RISC-V/WebAssembly; isso não faz parte desta versão.

Referências: [o que é GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/what-is-github-pages) e [configuração da origem de publicação](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site).
