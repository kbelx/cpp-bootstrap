# Outline — `templates`

- **Header:** [templates.hpp](../../include/templates.hpp) *(header-only)*
- **Propósito:** Fonte única de verdade dos 5 arquivos gerados pelo `cpp-bootstrap`.

## `Templates::VSCode`
- `LAUNCH_JSON` — `.vscode/launch.json` (`cppdbg` para Windows, Linux e macOS).
- `SETTINGS_JSON` — `.vscode/settings.json` (`compile_commands.json` + `--query-driver`).

## `Templates::Projeto`
- `MAIN_CPP` — `src/main.cpp` com `std::setlocale(LC_ALL, "")`.
- `CMAKE_LISTS` — `CMakeLists.txt` em 9 seções (C++23 estrito, guarda de build, flags e `stdc++exp`).
- `GITIGNORE` — `.gitignore` preservando `.vscode/launch.json` e `.vscode/settings.json`.

## `Templates::Render`
- `MARCADOR_PROJETO` (`"__PROJECT_NAME__"`) e `renderizarCMake(NomeProjeto)` — substitui o marcador em `CMAKE_LISTS`.
