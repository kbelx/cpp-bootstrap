# Outline — `templates`

- **Header:** [templates.hpp](../../include/templates.hpp) *(header-only)*
- **Namespace:** `bootstrap::tpl`
- **Propósito:** Única fonte de verdade dos 5 arquivos gerados pelo `cpp-bootstrap` (embutidos em *raw string literals*).

## Constantes (`constexpr std::string_view`)

- `kLaunchJson` — template de `.vscode/launch.json` (`cppdbg` com blocos `windows`, `linux` e `osx`).
- `kSettingsJson` — template de `.vscode/settings.json` (`compile_commands.json` + `--query-driver` multiplataforma).
- `kMainCpp` — template de `src/main.cpp` (cabeçalho de autoria, `std::setlocale(LC_ALL, "")`, `std::cin.ignore()`).
- `kCmakeLists` — template de `CMakeLists.txt` em 9 seções (C++23 estrito, guarda de `CMAKE_BUILD_TYPE`, `GLOB_RECURSE`, flags rigorosas e `CheckLinkerFlag` para `stdc++exp`).
- `kGitignore` — template de `.gitignore` (ignora build/binários e `.vscode/*`, preservando `launch.json` e `settings.json`).

## Funções

- `render_cmake_lists(project_name)` — substitui literalmente todas as ocorrências de `"__PROJECT_NAME__"` em `kCmakeLists`.
