# 01 — Visão Geral da Transição

[← Índice](00_indice.md)

## 1. Por que migrar de PowerShell para C++23

O `cpp-bootstrap` nasceu como um script PowerShell (`cpp_bootstrap.ps1`). Conforme ganhou validação rigorosa, rollback, flags e uma suíte de 20 testes, o PowerShell passou a gerar mais atrito do que ajuda:

| Problema no PowerShell | Impacto |
| :--- | :--- |
| **Fratura PS 5.1 × PS 7+** | O código precisou ficar em ASCII 7-bit puro, sem `` `e ``, e com detecção manual de SO, porque o PS 5.1 lê UTF-8 sem BOM como ANSI e quebra o parsing. |
| **Público-alvo** | Quem programa C++ no Linux/macOS raramente tem `pwsh` instalado. Exigir um runtime só para gerar 5 arquivos era uma barreira. |
| **Execution Policy** | No Windows, rodar `.ps1` frequentemente exige `-ExecutionPolicy Bypass`. |
| **Escala** | Templates embutidos em here-strings e lógica de CLI ficaram difíceis de evoluir. |

Escrever o gerador em C++23 resolve esses pontos e ainda transforma o projeto em **portfólio**: um gerador de projetos C++23 escrito em C++23, com as mesmas flags rigorosas que ele recomenda.

## 2. Estratégia de versionamento

- **`v0.x-beta` — PowerShell:** protótipo onde as regras, templates e testes foram validados. Ficou congelado.
- **`v1.0.0` — C++23:** primeira versão oficial, distribuída como binário nativo.

## 3. Linha do tempo

| Data | Marco |
| :--- | :--- |
| 18/10/2026 | Revisão completa do script PowerShell e criação do checklist de feedback. |
| 01/11/2026 | Correção de parsing no PS 5.1, UTF-8 sem BOM, validação de nomes e templates multiplataforma (`227a82c`). |
| 02/11/2026 | Seção 9 do `CMakeLists.txt` gerado: `stdc++exp` via `CheckLinkerFlag` (`f06b8db`). |
| 16/11/2026 | Decisão de migrar para C++23, entrevista de design e definição da arquitetura. |
| 16/11/2026 | Tag `v0.9.0-beta` + branch `legacy/powershell-beta` criados sobre `f06b8db`. |
| 16/11/2026 | Migração completa para C++23 com CTest e GitHub Actions (`36834ba`). |

## 4. O que mudou para o usuário final

| Antes (`v0.9.0-beta`) | Depois (`v1.0.0`) |
| :--- | :--- |
| `./cpp_bootstrap.ps1 MeuProjeto` | `cpp-bootstrap MeuProjeto` |
| `pwsh ./cpp_bootstrap.ps1 MeuProjeto` (Linux/macOS) | `./cpp-bootstrap MeuProjeto` |
| `-Quiet` | `-q` / `--quiet` (`-Quiet` continua aceito) |
| — | `-h` / `--help` e `-v` / `--version` |
| Exige PowerShell instalado | Binário nativo, sem runtime externo |

A **estrutura gerada** (`src/main.cpp`, `include/`, `.vscode/launch.json`, `.vscode/settings.json`, `CMakeLists.txt`, `.gitignore`) e o **conteúdo dos templates** são idênticos aos da `v0.9.0-beta`.

## 5. Estado final

- Código principal da `main` 100% em C++23, organizado em `include/` + `src/`.
- 20 testes em `tests/test_cpp_bootstrap.cpp`, integrados ao CTest, sem dependências externas.
- CI em Ubuntu (GCC 14), Windows (MSVC e MSYS2) e macOS (Clang).
- Release automático de binários para Windows x64, Linux x64 e macOS arm64 ao criar tags `v*`.
- Verificado localmente com **GCC 15.2.0** e **Clang 21.1.7** (MSYS2 UCRT64): 20/20 testes aprovados, incluindo build real com `--include-build`.

> [!IMPORTANT]
> A tag `v1.0.0` ainda **não** foi criada. Ela deve ser publicada depois que o workflow de CI ficar verde no GitHub (veja [05 — CI/CD](05_ci_cd_github_actions.md)).
