# Outline — `tests`

- **Orquestrador:** [test_cpp_bootstrap.cpp](../../tests/test_cpp_bootstrap.cpp) *(executável `test_cpp_bootstrap` no CTest)*
- **Propósito:** Suíte modular em C++23 com 20 testes (`--target-exe`, `--include-build`, `--verbose`).

## Módulos (`tests/`)

| Arquivo | Namespace | Escopo |
| :--- | :--- | :--- |
| [test_harness.hpp](../../tests/test_harness.hpp) | `Testes::Infra`, `Testes::Arquivos` | Estatísticas, asserções, resumo final, leitura de arquivos e `GuardaSandboxTemp` (RAII). |
| [test_process.hpp](../../tests/test_process.hpp) | `Testes::Processo` | Subprocessos Win32/POSIX, `comandoExisteNoPath` e `invocarBootstrap`. |
| [test_unit.hpp](../../tests/test_unit.hpp) | `Testes::Unitarios` | Testes **01–07**: `limparNomeProjeto`, `validarNomeProjeto` e `analisarArgs`. |
| [test_cli_integration.hpp](../../tests/test_cli_integration.hpp) | `Testes::IntegracaoCli` | Testes **08–12**: códigos de saída (`0`/`1`/`2`), colisões e `DirectoryRollbackGuard`. |
| [test_artifacts.hpp](../../tests/test_artifacts.hpp) | `Testes::Artefatos` | Testes **13–19**: trim ponta a ponta, `--quiet`, estrutura, templates, UTF-8 sem BOM e LF. |
| [test_real_build.hpp](../../tests/test_real_build.hpp) | `Testes::BuildReal` | Teste **20**: CMake, preservação de `Release` e compilação C++23 (`stdc++exp`) com `--include-build`. |
