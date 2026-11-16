# Outline — `tests`

- **Implementação:** [test_cpp_bootstrap.cpp](../../tests/test_cpp_bootstrap.cpp) *(executável `test_cpp_bootstrap` registrado no CTest)*
- **Propósito:** Suíte autônoma em C++23 com 20 cenários (unitários, integração CLI, encoding e compilação CMake real).

## Infraestrutura (`namespace` anônimo)

- `struct ProcessResult` / `struct TestStats` — resultado de subprocessos e contadores (`total`, `passed`, `failed`, `skipped`).
- `class TempSandboxGuard` — cria pasta temporária aleatória e remove no destrutor (RAII).
- `run_process(args, working_dir)` / `invoke_bootstrap(...)` / `command_exists_in_path(cmd)` — execução isolada multiplataforma (`CreateProcessA` no Windows, `fork`+`execvp` no POSIX).
- `invoke_test(...)`, `invoke_skip(...)`, `require(...)`, `read_all_bytes(...)`, `read_all_text(...)`, `has_utf8_bom(...)`.

## `main(argc, argv)`

- Flags aceitas: `--target-exe <caminho>`, `--include-build`, `--verbose`.
- Executa 4 blocos:
  1. Testes **01–07**: unitários (`trim_project_name`, `test_project_name`, `parse_cli`).
  2. Testes **08–12**: integração CLI (`exit 0/1/2`), colisão de caminho e `DirectoryRollbackGuard`.
  3. Testes **13–19**: trim ponta a ponta, `--quiet`, estrutura, conteúdo dos templates, ausência de BOM e LF.
  4. Teste **20**: configuração, compilação e execução C++23 (`<print>`/`<stacktrace>` via `stdc++exp`) com `--include-build`.
