# Agent Guide — Project Bootstrap

## 1. Contexto
Este projeto é um gerador CLI escrito em **C++23** (`cpp-bootstrap`) para inicialização rápida de estruturas de diretórios e arquivos de configuração para novos projetos C++23 com CMake.

A versão inicial em PowerShell permanece congelada na tag `v0.9.0-beta` e no branch `legacy/powershell-beta`. A partir da `v1.0.0`, o código principal da branch `main` é inteiramente em C++23.

O agente deve ajudar a:
- Manter consistência estrutural, modularidade (`include/` + `src/`) e integridade dos templates embutidos em `include/templates.hpp`.
- Garantir que a documentação única no `README.md` reflita exatamente o que o binário produz.
- Manter compilação limpa com zero avisos sob flags rigorosas em Windows (MSVC e MSYS2 GCC/Clang), Linux (GCC/Clang) e macOS (Clang).

---

## 2. Objetivo do Agente
- Evoluir e manter a arquitetura modular em C++23 (`cli`, `validator`, `generator`, `terminal`, `templates`) com comentários didáticos em português e estilo limpo.
- Garantir que `include/templates.hpp` seja a única fonte de verdade para os templates embutidos.
- Garantir que toda alteração passe pela suíte de testes no `CTest` (`tests/test_cpp_bootstrap.cpp`) com `--include-build`.
- Garantir o commit e o push apropriado para cada tarefa.

---

## 3. Estrutura do Repositório vs. Estrutura Gerada

### Repositório (`cpp-bootstrap` v1.0.0)
- `include/terminal.hpp`: detecção de SO (`detect_os_label`), suporte ANSI (`should_use_ansi`) e helpers de saída (`print_line`, `write_info`, `write_warn`, `write_error`).
- `include/cli.hpp` + `src/cli.cpp`: parse da CLI (`parse_cli`, `trim_project_name`, `print_tutorial`, `print_help`, `print_version`).
- `include/validator.hpp` + `src/validator.cpp`: validação de nome em 5 regras (`test_project_name`).
- `include/templates.hpp`: templates embutidos em *raw string literals* e `render_cmake_lists`.
- `include/generator.hpp` + `src/generator.cpp`: criação transacional sem mudança de diretório (`DirectoryRollbackGuard`, `write_utf8_no_bom`, `create_project`).
- `src/main.cpp`: ponto de entrada e mensagem final (`print_success_banner`).
- `tests/test_cpp_bootstrap.cpp`: suíte de testes unitários e de integração integrada ao `CTest`.
- `.github/workflows/ci.yml` e `.github/workflows/release.yml`: automação de CI multiplataforma e publicação de Releases em tags `v*`.

### Estrutura Padrão Gerada para Novos Projetos

```text
MeuProjeto/
    ├── src/
    │   └── main.cpp
    │
    ├── include/
    ├── .vscode/
    │   ├── launch.json
    │   └── settings.json
    │
    ├── CMakeLists.txt
    └── .gitignore
```

### Configuração do `CMakeLists.txt` Gerado
O gerador cria um arquivo `CMakeLists.txt` configurado com:
- `cmake_minimum_required(VERSION 3.20)`
- `set(CMAKE_CXX_STANDARD 23)` e `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
- `CMAKE_BUILD_TYPE` padrão `Debug` não-destrutivo em geradores *single-config* (`if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)`)
- Coleta de fontes via `file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")`
- Diretório de cabeçalhos privado apontando para `include`
- Flags rigorosas de compilação:
  - MSVC: `/W4 /permissive- /EHsc`
  - GCC / Clang: `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fdiagnostics-color=always`
- Vinculação condicional automática de `stdc++exp` via `CheckLinkerFlag` (`check_linker_flag(CXX "-lstdc++exp" HAVE_STDCXXEXP)`) fora de MSVC/Apple para suportar recursos C++23 da `libstdc++` (como `<print>` e `<stacktrace>`) tanto no GCC quanto no Clang.

---

## 4. Comportamento Esperado

- Sempre criar diretórios antes de criar arquivos, sem alterar o diretório de trabalho do processo chamador e com rollback RAII automático (`DirectoryRollbackGuard`) em caso de falha.
- Nunca sobrescrever pastas ou arquivos existentes.
- Usar `snake_case` para funções e variáveis em C++ dentro do `namespace bootstrap`.
- Manter código limpo, comentado e didático, sem avisos de compilação e sem código morto.
- Manter a codificação de todos os arquivos estritamente em **UTF-8 sem BOM** com fim de linha **LF**; nos arquivos `.hpp`, `.cpp` e `CMakeLists.txt`, manter apenas caracteres **ASCII 7-bit (`<= 0x7F`)**.

---

## 5. Comandos Disponíveis

### Compilar e Testar o `cpp-bootstrap`
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Executar Suíte Completa com Compilação Real C++23
```powershell
./build/test_cpp_bootstrap --target-exe ./build/cpp-bootstrap --include-build
```

### Uso do Binário
```powershell
cpp-bootstrap NomeDoProjeto
cpp-bootstrap NomeDoProjeto -q
cpp-bootstrap --help
cpp-bootstrap --version
```
