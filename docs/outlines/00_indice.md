# Outlines dos Módulos — `cpp-bootstrap`

Resumo estrutural de cada módulo e seu respectivo headerfile em `v1.0.0` (C++23).

| Módulo | Arquivos | Tipo |
| :--- | :--- | :--- |
| [terminal](terminal.md) | `include/terminal.hpp` | Header-only (`bootstrap::term`) |
| [cli](cli.md) | `include/cli.hpp`, `src/cli.cpp` | Header + implementação (`bootstrap`) |
| [validator](validator.md) | `include/validator.hpp`, `src/validator.cpp` | Header + implementação (`bootstrap`) |
| [templates](templates.md) | `include/templates.hpp` | Header-only (`bootstrap::tpl`) |
| [generator](generator.md) | `include/generator.hpp`, `src/generator.cpp` | Header + implementação (`bootstrap`) |
| [main](main.md) | `src/main.cpp` | Ponto de entrada (executável `cpp-bootstrap`) |
| [tests](tests.md) | `tests/test_cpp_bootstrap.cpp` | Suíte CTest (executável `test_cpp_bootstrap`) |
