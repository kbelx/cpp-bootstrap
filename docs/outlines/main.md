# Outline — `main`

- **Implementação:** [main.cpp](../../src/main.cpp) *(sem header próprio; ponto de entrada do executável `cpp-bootstrap`)*
- **Propósito:** Orquestrar CLI, validação, geração transacional e exibição da mensagem final.

## Funções

- `print_success_banner(project_name, quiet)` *(namespace anônimo)* — imprime confirmação, próximos passos (`cd`, `code .`, `cmake -B build`, `cmake --build build`), SO detectado e, se `!quiet`, rodapé do autor com links (OSC 8 quando `Palette::enabled`).

- `main(argc, argv)` — executa `parse_cli()` e despacha:
  - `ShowTutorialError` → `print_tutorial()`, retorna `1`.
  - `ShowHelp` / `ShowVersion` → `print_help()` / `print_version()`, retorna `0`.
  - `InvalidFlag` → erro + dica `--help`, retorna `2`.
  - `CreateProject` → `test_project_name()` (retorna `2` se inválido) → `create_project()` (retorna `10` em falha de I/O) → `print_success_banner()`, retorna `0`.
