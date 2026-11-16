# 04 — Build e Testes

[← Índice](00_indice.md)

## 1. Targets do `CMakeLists.txt`

| Target | Tipo | Conteúdo |
| :--- | :--- | :--- |
| `cpp_bootstrap_core` | Biblioteca `OBJECT` | `src/cli.cpp`, `src/validator.cpp`, `src/generator.cpp`. Exporta `include/`, as flags rigorosas e `stdc++exp` como `PUBLIC`. |
| `cpp-bootstrap` | Executável | `src/main.cpp` + `cpp_bootstrap_core` |
| `test_cpp_bootstrap` | Executável (só com `BUILD_TESTING`) | `tests/test_cpp_bootstrap.cpp` + `cpp_bootstrap_core` |

Como o núcleo é uma biblioteca `OBJECT`, os testes chamam `parse_cli()`, `test_project_name()` e `DirectoryRollbackGuard` diretamente, sem recompilar os fontes duas vezes.

## 2. Opções de configuração

| Opção | Padrão | Efeito |
| :--- | :--- | :--- |
| `CMAKE_BUILD_TYPE` | `Debug` (só em geradores single-config, quando não informado) | `-DCMAKE_BUILD_TYPE=Release` é respeitado |
| `CPP_BOOTSTRAP_STATIC_RUNTIME` | `OFF` | Em MinGW, adiciona `-static -static-libgcc -static-libstdc++` para gerar um `.exe` que roda sem as DLLs do MSYS2 |
| `BUILD_TESTING` | `ON` (via `include(CTest)`) | Compila `test_cpp_bootstrap` e registra o teste `cpp_bootstrap_suite` |

## 3. Comandos

```powershell
# Windows (MSYS2 UCRT64)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Windows (MSVC) / Linux / macOS
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Testes padrao (01 a 19; o 20 aparece como SKIP)
ctest --test-dir build -C Release --output-on-failure

# Suite completa, incluindo build real de um projeto gerado
./build/test_cpp_bootstrap --target-exe ./build/cpp-bootstrap --include-build
```

## 4. Runner de testes

`tests/test_cpp_bootstrap.cpp` é um runner próprio, sem frameworks externos. Opções:

| Opção | Efeito |
| :--- | :--- |
| `--target-exe <caminho>` | Binário `cpp-bootstrap` a ser testado (o CTest passa `$<TARGET_FILE:cpp-bootstrap>`) |
| `--include-build` | Ativa o teste 20 (configura e compila um projeto gerado com CMake) |
| `--verbose` | Mostra stdout/stderr dos subprocessos |

Infraestrutura interna:

- **`TempSandboxGuard`:** cria uma pasta temporária com sufixo aleatório e a remove no destrutor (RAII), mesmo se um teste falhar.
- **`run_process()`:** executa subprocessos com stdout/stderr capturados em threads e stdin ligado a `NUL` / `/dev/null`. Usa `CreateProcessA` no Windows e `fork` + `execvp` no POSIX.
- **`invoke_test()` / `invoke_skip()`:** imprimem `[PASS]`, `[FAIL]` ou `[SKIP]` e alimentam o resumo final.

## 5. Os 20 cenários

| Nº | Grupo | O que verifica |
| :--- | :--- | :--- |
| 01 | Unitário | `trim_project_name` remove espaços e aspas externas |
| 02 | Unitário | Rejeita nome vazio ou só espaços |
| 03 | Unitário | Rejeita `<`, `>`, `/` e início com `-` |
| 04 | Unitário | Rejeita espaço interno (`"Meu Projeto"`) |
| 05 | Unitário | Rejeita nome terminando em `.` |
| 06 | Unitário | Rejeita `CON`, `aux`, `NUL.txt`, `COM1`, `LPT9` |
| 07 | Unitário | `parse_cli`: sem args, `--help`, `--version`, `-q` e múltiplos tokens |
| 08 | Integração CLI | Sem argumentos → código `1` e tutorial |
| 09 | Integração CLI | `--help` e `--version` → código `0` |
| 10 | Integração CLI | Entradas inválidas e flag desconhecida → código `2` |
| 11 | Segurança | Pasta ou arquivo existente → código `2`, conteúdo intacto |
| 12 | Segurança | `DirectoryRollbackGuard` remove sem `commit()` e preserva com `commit()` |
| 13 | Estrutura | `"  ProjetoTrimado  "` cria a pasta exata e o CMake recebe o nome limpo |
| 14 | Estrutura | Projeto válido → código `0`, instruções CMake e `--quiet` sem rodapé |
| 15 | Estrutura | Pastas `src/`, `include/` e `.vscode/` existem |
| 16 | Artefato | `main.cpp` com `int main()`, `<iostream>`, `<clocale>` e `setlocale(LC_ALL, "")` |
| 17 | Artefato | `launch.json` com blocos `windows`/`linux`/`osx` e `settings.json` com `--query-driver` |
| 18 | Artefato | `CMakeLists.txt` com C++23, guarda de build type, flags e Seção 9 (`stdc++exp`) |
| 19 | Encoding | `.gitignore` correto e os 5 arquivos sem BOM e sem CRLF |
| 20 | Integração real | Configura em Release, compila e executa código com `<print>` e `<stacktrace>` (só com `--include-build`) |

## 6. Resultado da verificação na migração

| Toolchain | Build | CTest | Suíte com `--include-build` |
| :--- | :--- | :--- | :--- |
| GCC 15.2.0 (MSYS2 UCRT64) | Sem avisos | 1/1 | 20/20 |
| Clang 21.1.7 (MSYS2 UCRT64) | Sem avisos | 1/1 | 20/20 |
| GCC 15.2.0 + `CPP_BOOTSTRAP_STATIC_RUNTIME=ON` | Sem avisos | 1/1 | — |
