# Outlines dos Módulos — `cpp-bootstrap`

Módulos da versão `v1.0.0` (C++23).

| Módulo | Arquivos | Namespaces |
| :--- | :--- | :--- |
| [cores](cores.md) | `include/cores.hpp` | `Cores` |
| [terminal](terminal.md) | `include/terminal.hpp` | `Terminal::{Ambiente, Saida}` |
| [cli](cli.md) | `include/cli.hpp`, `src/cli.cpp` | `CLI::{Parser, Telas, Auxiliar}` |
| [validator](validator.md) | `include/validator.hpp`, `src/validator.cpp` | `Validadores::{Caracteres, Regras, Diagnostico}` |
| [templates](templates.md) | `include/templates.hpp` | `Templates::{VSCode, Projeto, Render}` |
| [generator](generator.md) | `include/generator.hpp`, `src/generator.cpp` | `Gerador::{Seguranca, Arquivo, Projeto}` |
| [main](main.md) | `src/main.cpp` | `App::{Banner, Fluxo}` + `main` |
| [tests](tests.md) | `tests/test_cpp_bootstrap.cpp`, `tests/test_*.hpp` | `Testes::{Infra, Arquivos, Processo, Unitarios, IntegracaoCli, Artefatos, BuildReal}` |
