# 08 — Histórico da Fase Beta (PowerShell)

[← Índice](00_indice.md)

Resumo dos problemas encontrados na revisão de 18/10/2026 e de como foram resolvidos antes de a versão PowerShell ser congelada como `v0.9.0-beta`. Esses aprendizados viraram requisitos da versão C++23.

## 1. Bugs confirmados

| # | Problema | Correção na beta | Como ficou no C++23 |
| :--- | :--- | :--- | :--- |
| 1.1 | Script não abria no Windows PowerShell 5.1: UTF-8 sem BOM lido como ANSI quebrava o parsing; `` `e `` não existia; `$IsWindows` era `$null` | Código 100% ASCII 7-bit, `[char]27` para ANSI e detecção de SO compatível | Não se aplica (binário nativo). A regra ASCII 7-bit foi mantida para `.hpp`/`.cpp` |
| 1.2 | O nome validado não era o usado para criar a pasta (`" Proj"` criava `" Proj"`) | Trim único no início do script | `trim_project_name()` aplicado uma vez em `parse_cli()` (teste 13) |
| 1.3 | Nomes com espaço passavam e quebravam `project(...)` no CMake | Regra única `^[A-Za-z0-9_][A-Za-z0-9_-]*$` | Mesma regra em `test_project_name()` (testes 04 e 10) |

## 2. Problemas de comportamento

| # | Problema | Correção |
| :--- | :--- | :--- |
| 2.1 | `Set-Location` mudava a pasta do terminal e não havia limpeza em falhas | Caminhos com `Join-Path` + rollback → no C++23, `DirectoryRollbackGuard` |
| 2.2 | Mensagem final sugeria `cmake --build .` sem configurar antes | Passos `cmake -B build` e `cmake --build build` |
| 2.3 | `set(CMAKE_BUILD_TYPE Debug)` ignorava `-DCMAKE_BUILD_TYPE=Release` | Guarda `if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)` |
| 2.4 | `Out-File -Encoding utf8` gravava BOM no PS 5.1 | Escrita explícita sem BOM → no C++23, `write_utf8_no_bom()` em modo binário |
| 2.5 | `.gitignore` gerado ignorava `.vscode/` inteiro | `.vscode/*` + `!.vscode/launch.json` + `!.vscode/settings.json` |
| 2.6 | Caminhos do MSYS2 fixos nos JSON do VS Code | Blocos `windows`/`linux`/`osx` e `--query-driver` multiplataforma |
| 2.7 | `setlocale(LC_ALL, "pt-BR.UTF-8")` só funcionava no Windows | `std::setlocale(LC_ALL, "")` |
| 2.8 | Here-strings expansíveis podiam trocar `$` sem aviso | Here-strings literais → no C++23, raw string literals |

## 3. Adição posterior: Seção 9 (`stdc++exp`)

No Windows com MinGW, `std::println` falhava no linker com `undefined reference to std::__open_terminal`, tanto no GCC quanto no Clang, porque esses símbolos ficam em `libstdc++exp.a`. A primeira proposta (`if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")`) deixava de fora o Clang no MSYS2 e o `<stacktrace>` no Linux. A solução adotada testa o linker:

```cmake
if(NOT MSVC AND NOT APPLE)
    include(CheckLinkerFlag)
    check_linker_flag(CXX "-lstdc++exp" HAVE_STDCXXEXP)
    if(HAVE_STDCXXEXP)
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
    endif()
endif()
```

Ela funciona com GCC e Clang, não quebra com `libc++`/MSVC/macOS e foi aplicada tanto no template gerado quanto no `CMakeLists.txt` do próprio `cpp-bootstrap`.

## 4. Evolução da suíte de testes

| Fase | Testes | Interpretadores / compiladores |
| :--- | :--- | :--- |
| Revisão inicial | 14 (13 aprovados, 1 ignorado) | `pwsh` |
| Correções beta | 19 | `pwsh` 7.6 e Windows PowerShell 5.1 |
| Seção 9 | 20 | `pwsh` 7.6, PS 5.1, GCC 15.2 e Clang 21.1 |
| `v1.0.0` (C++23) | 20 | GCC 15.2 e Clang 21.1 local; CI em GCC 14, MSVC, MSYS2 e AppleClang |

## 5. Itens que ficaram em aberto na beta

- Migração dos testes para Pester → superado pela suíte em C++23.
- Workflow de CI no GitHub Actions → implementado na `v1.0.0`.
