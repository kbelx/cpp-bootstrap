# 02 — Mapa PowerShell → C++23

[← Índice](00_indice.md)

Este mapa mostra onde cada seção do `cpp_bootstrap.ps1` (versão `v0.9.0-beta`) foi parar no código C++23. A ordem lógica foi preservada para facilitar a manutenção por quem conhece o script original.

## 1. Tabela de correspondência

| Seção no `cpp_bootstrap.ps1` | Arquivo C++23 | Equivalente |
| :--- | :--- | :--- |
| `[ DETECCAO DE AMBIENTE ]` — `$isWin`, `$isMac` | `include/terminal.hpp` | `term::detect_os_label()` (decidido em tempo de compilação via `_WIN32` / `__APPLE__`) |
| `[ DETECCAO DE AMBIENTE ]` — `$isRedirected`, `$useAnsi`, `NO_COLOR` | `include/terminal.hpp` | `term::has_no_color_env()` e `term::should_use_ansi()` (usa `isatty` e ativa VT no console do Windows) |
| `[ PALETA / FUNCOES ]` — `$RESET`, `$GREEN`, `$CYAN`... | `include/terminal.hpp` | `struct term::Palette` + `term::palette()` (cores vazias quando não há ANSI) |
| `$separator` | `include/terminal.hpp` | `term::print_separator()` |
| `Write-Info`, `Write-Warn`, `Write-ErrorMsg` | `include/terminal.hpp` | `term::write_info()`, `term::write_warn()`, `term::write_error()` |
| `Write-Host` | `include/terminal.hpp` | `term::print_line()` (usa `std::println` quando disponível, senão `std::cout`) |
| `param(...)` — `$ProjectName`, `$Quiet`, `$ExtraArgs` | `include/cli.hpp` + `src/cli.cpp` | `bootstrap::parse_cli()` → `struct CliOptions` |
| Recomposição de `$ExtraArgs` com `-join " "` | `src/cli.cpp` | `parse_cli()` junta tokens posicionais com espaço |
| `.Trim().Trim('"').Trim("'").Trim()` | `src/cli.cpp` | `bootstrap::trim_project_name()` |
| `[ TUTORIAL ]` | `src/cli.cpp` | `bootstrap::print_tutorial()` (+ novos `print_help()` e `print_version()`) |
| `[ VALIDACAO DO NOME ]` — `Test-ProjectName` | `include/validator.hpp` + `src/validator.cpp` | `bootstrap::test_project_name(name, base_dir, silent)` com os mesmos 5 blocos numerados |
| `Write-Utf8NoBom` | `include/generator.hpp` + `src/generator.cpp` | `bootstrap::write_utf8_no_bom()` (`std::ofstream` em modo binário) |
| `$createdRoot` + `try/catch` com `Remove-Item` | `include/generator.hpp` + `src/generator.cpp` | `class DirectoryRollbackGuard` (RAII: o destrutor apaga a pasta se `commit()` não foi chamado) |
| Here-strings `$launchJsonContent`, `$settingsJsonContent`, `$mainCppContent`, `$cmakeTemplate`, `$gitignoreContent` | `include/templates.hpp` | `tpl::kLaunchJson`, `tpl::kSettingsJson`, `tpl::kMainCpp`, `tpl::kCmakeLists`, `tpl::kGitignore` (raw string literals `R"tpl(...)tpl"`) |
| `$cmakeTemplate.Replace("__PROJECT_NAME__", ...)` | `include/templates.hpp` | `tpl::render_cmake_lists()` |
| `[ CRIACAO DO PROJETO ]` — `New-Item` + gravações | `src/generator.cpp` | `bootstrap::create_project()` |
| `[ MENSAGEM FINAL ]` | `src/main.cpp` | `print_success_banner(project_name, quiet)` |
| `exit 1`, `exit 2`, `exit 10` | `src/main.cpp` | `return 1;`, `return 2;`, `return 10;` em `main()` |
| `tests/test_cpp_bootstrap.ps1` | `tests/test_cpp_bootstrap.cpp` | Runner nativo C++23 integrado ao CTest |

## 2. Diferenças de comportamento intencionais

| Ponto | PowerShell | C++23 |
| :--- | :--- | :--- |
| Flags de ajuda/versão | Não existiam | `-h`/`--help`/`/?` e `-v`/`--version` retornam `0` |
| Flag desconhecida | Tratada como parte do nome | `--qualquer-coisa` (ou `-x` curta) retorna `2` com `[ERRO] Opcao desconhecida` |
| Exemplos nas mensagens | `./cpp_bootstrap.ps1 MeuProjeto` | `cpp-bootstrap MeuProjeto` |
| Detecção de SO | Em tempo de execução | Em tempo de compilação (um binário por plataforma) |
| Rollback | Flag `$createdRoot` no `catch` | RAII — impossível esquecer a limpeza |

## 3. Regra de validação preservada (lado a lado)

```text
PowerShell (Test-ProjectName)          C++23 (test_project_name)
---------------------------------      ----------------------------------------
1 - IsNullOrWhiteSpace($Name)     ->   name.empty() || is_all_whitespace(name)
2 - $Name.EndsWith(".")           ->   name.back() == '.'
3 - $reserved -contains $baseName ->   is_windows_reserved_name(name)
4 - -notmatch "^[A-Za-z0-9_]..."  ->   is_allowed_start() + is_allowed_body()
5 - Test-Path -LiteralPath        ->   std::filesystem::exists(base_dir / name)
```

As mensagens de erro são as mesmas, exceto pelos exemplos de comando, que agora citam `cpp-bootstrap`.
