# Outline — `cli`

- **Header:** [cli.hpp](../../include/cli.hpp)
- **Implementação:** [cli.cpp](../../src/cli.cpp)
- **Namespace:** `bootstrap`
- **Propósito:** Parse dos argumentos da CLI, normalização do nome do projeto e telas de ajuda/versão.

## Tipos Públicos (`cli.hpp`)

- `enum class CliAction`: `CreateProject`, `ShowTutorialError`, `ShowHelp`, `ShowVersion`, `InvalidFlag`.
- `struct CliOptions`: `action`, `project_name`, `unknown_flag`, `quiet`.

## API Pública (`cli.hpp` / `cli.cpp`)

- `trim_project_name(raw)` — remove espaços externos, aspas (`"` / `'`) envolventes e espaços restantes.
- `parse_cli(argc, argv)` — interpreta flags (`-h`/`--help`/`/?`, `-v`/`--version`, `-q`/`--quiet`/`-Quiet`/`-quiet`), rejeita flags desconhecidas (`InvalidFlag`) e recombina tokens posicionais sem aspas com `" "`.
- `print_tutorial()` — exibe uso rápido quando chamado sem argumentos.
- `print_help()` — exibe ajuda completa, opções e regras de nome.
- `print_version()` — imprime `cpp-bootstrap <kVersion>`.

## Helpers Internos (`cli.cpp`, `namespace` anônimo)

- `is_ascii_space`, `strip_outer_spaces`, `strip_outer_char` — apoio ao `trim_project_name`.
- `is_help_flag`, `is_version_flag`, `is_quiet_flag` — reconhecimento de flags.
