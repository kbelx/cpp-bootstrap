# C++ Bootstrap

Ferramenta CLI nativa em **C++23** para criar projetos **C++23 + CMake** com estrutura padronizada, rollback transacional (RAII) e suporte pronto ao **VS Code** (Windows, Linux e macOS).

> - Protótipo legado em PowerShell congelado na tag [`v0.9.0-beta`](../../tree/v0.9.0-beta) (`legacy/powershell-beta`).
> - A partir da **`v1.0.0`**, o binário é nativo em C++23 sem dependências de runtime externas.

| Info | Descrição |
| :--- | :--- |
| Autoria | Bruno Costa ([@kbelx](https://github.com/kbelx)) |
| Padrão | C++23 estrito (`CMAKE_CXX_STANDARD_REQUIRED ON`) |
| Build System | CMake (`>= 3.20`) |
| Alvos dos Templates | **Windows:** GCC/GDB (MSYS2 UCRT64) ou MSVC • **Linux:** GCC/Clang + GDB • **macOS:** Clang/GCC + LLDB |

---

## 1. Instalação e Compilação

### Opção A — Binário Pronto (GitHub Releases)
Baixe o pacote da sua plataforma na aba **Releases** (`windows-x64.zip`, `linux-x64.tar.gz` ou `macos-arm64.tar.gz`) e extraia o executável `cpp-bootstrap` para uma pasta no seu `PATH`.

### Opção B — Compilar do Código-Fonte
**Pré-requisitos:** CMake `>= 3.20` e compilador C++23 (GCC 14+, Clang 18+ ou Visual Studio 2022+).

```powershell
# Windows (MSYS2 UCRT64 / MinGW)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Windows (MSVC) / Linux / macOS
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

*(Adicione `-DCPP_BOOTSTRAP_STATIC_RUNTIME=ON` no MinGW para gerar binário standalone sem DLLs do MSYS2.)*

---

## 2. Uso e Validações

```powershell
cpp-bootstrap MeuProjeto         # Cria o projeto no diretorio atual
cpp-bootstrap MeuProjeto -q      # Suprime o rodape de autoria e links (--quiet)
cpp-bootstrap --help             # Exibe ajuda
cpp-bootstrap --version          # Exibe versao
```

Antes de gravar no disco, o nome sofre trim único de espaços/aspas externas e passa por 5 regras (`exit 2` se inválido):
1. Não vazio e não composto apenas por espaços.
2. Não termina com `.`.
3. Não usa nomes reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1..9`, `LPT1..9`).
4. Aceita apenas `^[A-Za-z0-9_][A-Za-z0-9_-]*$` (sem espaços internos ou `-` inicial).
5. Não colide com pasta ou arquivo existente.

Falhas de I/O acionam rollback automático via **RAII (`DirectoryRollbackGuard`)** removendo a pasta parcial (`exit 10`).

---

## 3. Estrutura Gerada e Arquitetura

### 3.1. Estrutura Gerada para Novos Projetos
Todos os arquivos saem em **UTF-8 sem BOM** e **LF**:

```text
MeuProjeto/
├── src/
│   └── main.cpp
├── include/
├── .vscode/
│   ├── launch.json
│   └── settings.json
├── CMakeLists.txt
└── .gitignore
```

| Arquivo | Função |
| :--- | :--- |
| `src/main.cpp` | Ponto de entrada com `std::setlocale(LC_ALL, "")` e pausa de terminal. |
| `include/` | Cabeçalhos do projeto (`.hpp` / `.h`). |
| `.vscode/launch.json` | Depuração `cppdbg` para `windows`, `linux` e `osx`. |
| `.vscode/settings.json` | `compile_commands.json` + `--query-driver` multiplataforma para `clangd`. |
| `CMakeLists.txt` | C++23 estrito, `Debug` padrão não-destrutivo, flags (`/W4 /permissive-` ou `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors`) e linkagem opcional de `stdc++exp`. |
| `.gitignore` | Ignora artefatos de build/binários e `.vscode/*`, preservando `launch.json` e `settings.json`. |

### 3.2. Arquitetura Interna (`cpp-bootstrap`)

```text
cpp-bootstrap/
├── .github/workflows/
│   ├── ci.yml                     # CI multiplataforma (Ubuntu, Windows MSVC/MSYS2, macOS)
│   └── release.yml                # Publicacao automatica de Releases em tags v*
├── include/
│   ├── cores.hpp                  # Cores (paleta de sequencias ANSI header-only)
│   ├── terminal.hpp               # Terminal::{Ambiente, Saida}
│   ├── cli.hpp                    # CLI::{VERSAO, Acao, Opcoes, Parser, Telas}
│   ├── validator.hpp              # Validadores::validarNomeProjeto
│   ├── templates.hpp              # Templates::{VSCode, Projeto, Render}
│   └── generator.hpp              # Gerador::{Seguranca, Arquivo, Projeto}
├── src/
│   ├── cli.cpp
│   ├── validator.cpp
│   ├── generator.cpp
│   └── main.cpp                   # App::{Banner, Fluxo} e main()
├── tests/
│   ├── test_harness.hpp           # Testes::{Infra, Arquivos} (GuardaSandboxTemp)
│   ├── test_process.hpp           # Testes::Processo (subprocessos Win32/POSIX)
│   ├── test_unit.hpp              # Testes::Unitarios (01-07)
│   ├── test_cli_integration.hpp   # Testes::IntegracaoCli (08-12)
│   ├── test_artifacts.hpp         # Testes::Artefatos (13-19)
│   ├── test_real_build.hpp        # Testes::BuildReal (20)
│   └── test_cpp_bootstrap.cpp     # Orquestrador da suite (CTest)
└── CMakeLists.txt
```

---

## 4. Compilação e Execução de Projetos Gerados

```powershell
cd MeuProjeto
code .           # Pressione F5 no VS Code para compilar e depurar
```

Ou via terminal:

```powershell
cmake -B build
cmake --build build
./build/MeuProjeto.exe   # Windows
./build/MeuProjeto       # Linux / macOS
```

---

## 5. Testes Automatizados (CTest) e CI/CD

A suíte em [`tests/`](tests/) executa em sandbox temporário RAII (`GuardaSandboxTemp`) cobrindo **20 cenários**:

| Nº | Categoria | Escopo |
| :--- | :--- | :--- |
| **01–07** | **Unitários** | `limparNomeProjeto`, `validarNomeProjeto` (5 regras) e `analisarArgs` |
| **08–12** | **Integração CLI & Segurança** | Códigos `0`/`1`/`2`, colisões e `DirectoryRollbackGuard` (`confirmar()`/`foiConfirmado()`) |
| **13–19** | **Estrutura & Encoding** | Trim ponta a ponta, `--quiet`, templates gerados, UTF-8 sem BOM e LF |
| **20** | **Build Real C++23** | CMake `Release` + compilação e execução com `<print>` / `<stacktrace>` via `stdc++exp` (`--include-build`) |

```powershell
# Padrao via CTest (testes 01-19; teste 20 marcado como SKIP)
ctest --test-dir build -C Release --output-on-failure

# Suite completa (Windows MSYS2 / Linux / macOS)
./build/test_cpp_bootstrap --target-exe ./build/cpp-bootstrap --include-build

# Suite completa (Windows MSVC)
./build/Release/test_cpp_bootstrap.exe --target-exe ./build/Release/cpp-bootstrap.exe --include-build
```

### GitHub Actions
- **CI (`ci.yml`):** roda build e os 20 testes em **Ubuntu 24.04 (GCC 14)**, **Windows (MSVC)**, **Windows (MSYS2 UCRT64 GCC)** e **macOS 15 (Clang)**.
- **Releases (`release.yml`):** ao enviar uma tag `v*`, gera e publica os pacotes `.zip` e `.tar.gz` em **GitHub Releases**.
