# 07 — Registro de Decisões

[← Índice](00_indice.md)

Decisões tomadas durante o planejamento da transição (16/11/2026), com a alternativa descartada e o motivo.

| # | Tema | Decisão | Alternativa descartada | Motivo |
| :--- | :--- | :--- | :--- | :--- |
| D1 | Linguagem da v1.0.0 | C++23 | Python (stdlib), Rust/Go | Coerência com o propósito do projeto e valor como portfólio |
| D2 | Versionamento | PowerShell como `v0.9.0-beta`; C++23 como `v1.0.0` | Manter as duas implementações na `main` | História clara no Git, sem duplicar manutenção |
| D3 | Destino dos `.ps1` | Tag `v0.9.0-beta` + branch `legacy/powershell-beta`; `.ps1` removidos da `main` | Pasta `legacy/` dentro da `main` | A `main` fica 100% C++23 e o histórico continua acessível |
| D4 | Escopo da CLI | Paridade 1:1 + `-q`/`--quiet`, `-h`/`--help`, `-v`/`--version` | `--dry-run`, `--git` | Lançar com o que já estava validado; novidades ficam para depois |
| D5 | Armazenamento dos templates | Raw string literals em `include/templates.hpp` | Pasta `templates/` convertida pelo CMake | Build simples e binário autônomo |
| D6 | Testes | Runner próprio em C++23 + CTest, sem dependências | doctest/Catch2 via `FetchContent` | Zero dependências externas e build offline |
| D7 | CI/CD | `ci.yml` + `release.yml` já na transição | Só CI, ou nada por enquanto | Binários prontos para quem não quer compilar |
| D8 | Nome do executável | `cpp-bootstrap` | `cpp_bootstrap` | Padrão de CLIs modernas e igual ao nome do repositório |
| D9 | Organização dos arquivos | Pares `.hpp` + `.cpp` para `cli`, `validator` e `generator`; headers puros para `terminal` e `templates` | Tudo em headers; ou CLI dentro do `main.cpp` | Padrão clássico de projetos CMake |
| D10 | Mensagens da validação | `test_project_name()` imprime dentro de cada regra e retorna `bool`, com parâmetro `silent` | Retornar uma struct/`std::expected` e imprimir no `main` | Mais próximo do script original e mais fácil de adicionar regras |
| D11 | Estilo de código | Banners `[ SECAO ]`, comentários numerados em português, `snake_case`, `namespace bootstrap` | — | Continuidade com o estilo do script |

## Decisões técnicas complementares

| Tema | Decisão | Motivo |
| :--- | :--- | :--- |
| Rollback | Classe RAII `DirectoryRollbackGuard` | Substitui a flag `$createdRoot` e garante limpeza em qualquer saída por exceção |
| Núcleo compartilhado | Biblioteca `OBJECT` `cpp_bootstrap_core` | Testes unitários usam as mesmas funções do binário sem recompilar |
| Windows standalone | Opção `CPP_BOOTSTRAP_STATIC_RUNTIME` | Gera `.exe` MinGW sem depender das DLLs do MSYS2 |
| Release no Windows | Compilado com MSVC | Binário sem dependências de runtime do MinGW |
| Flags desconhecidas | Retornam código `2` | Evita que um erro de digitação vire nome de pasta |
| `stdc++exp` | Mantido no gerador e no próprio projeto | `<print>` e `<stacktrace>` ficam em `libstdc++exp.a` no GCC/Clang com libstdc++ |

## Ideias registradas para o futuro

- `--dry-run`: mostrar a árvore que seria criada sem tocar no disco.
- `--git`: executar `git init` no projeto criado.
- `--with-tests`: gerar a estrutura com CTest já configurado.
- Escolha de compilador/preset (`--compiler gcc|clang|msvc`).
- Mover templates para uma pasta `templates/` embutida no build, caso o número de templates cresça.
