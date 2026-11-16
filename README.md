---
title: C++ Bootstrap
author: Bruno Costa
tags:
  - cpp
  - cpp23
  - cmake
  - cli
  - obsidian
---

# C++ Bootstrap

Ferramenta CLI nativa e multiplataforma escrita em **C++23** para inicialização rápida de projetos **C++23** com **CMake**, estrutura de diretórios padronizada, rollback transacional (RAII) e ambiente pronto para desenvolvimento e depuração no **Visual Studio Code** (Windows, Linux e macOS).

> **Histórico de Versões:** O protótipo original em PowerShell encontra-se congelado na tag [`v0.9.0-beta`](../../tree/v0.9.0-beta) e no branch `legacy/powershell-beta`. A partir da **`v1.0.0`**, o projeto é distribuído como executável nativo em C++23 sem dependências de runtime externas.

---

## 1. Apresentação

- **Título:** C++ Bootstrap (`cpp-bootstrap`)
- **Versão Atual:** `1.0.0`
- **Descrição:** Gerador CLI de estrutura e arquivos de configuração para novos projetos C++23.
- **Autoria:** Bruno Costa ([@kbelx](https://github.com/kbelx))
- **Padrão C++:** C++23 estrito (`set(CMAKE_CXX_STANDARD_REQUIRED ON)`)
- **Build System:** CMake (mínimo 3.20)
- **Compilador / Debugger Alvo dos Templates:**
  - **Windows:** GCC (`g++`) e GDB via MSYS2 (UCRT64) ou MSVC
  - **Linux:** GCC / Clang e GDB (`/usr/bin/gdb`)
  - **macOS:** Clang / GCC e LLDB (`lldb`)

---

## 2. Instalação e Compilação

### Opção A — Baixar Binário Pronto (GitHub Releases)
Acesse a aba **Releases** do repositório e baixe o pacote pré-compilado para o seu sistema (`windows-x64.zip`, `linux-x64.tar.gz` ou `macos-arm64.tar.gz`). Basta extrair o executável `cpp-bootstrap` para uma pasta no seu `PATH`.

### Opção B — Compilar a partir do Código-Fonte

#### Pré-requisitos
1. **CMake** 3.20 ou superior.
2. **Compilador C++23**:
   - **Windows:** MSYS2 UCRT64 (`g++` 14+ / `clang++` 18+) ou Visual Studio 2022+ (MSVC).
   - **Linux:** GCC 14+ ou Clang 18+.
   - **macOS:** Xcode Command Line Tools (Clang) ou Homebrew GCC/LLVM.

#### Build Local
```powershell
# Windows (MSYS2 UCRT64 / MinGW)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Windows (MSVC) / Linux / macOS
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

*(Para gerar um executável standalone no MinGW sem dependência das DLLs do MSYS2, adicione `-DCPP_BOOTSTRAP_STATIC_RUNTIME=ON` na etapa de configuração.)*

---

## 3. Como Usar

```powershell
# Criar um novo projeto no diretorio atual
cpp-bootstrap MeuProjeto

# Modo enxuto (suprime o rodape de autoria e links)
cpp-bootstrap MeuProjeto -q
cpp-bootstrap MeuProjeto --quiet

# Ajuda e versao
cpp-bootstrap --help
cpp-bootstrap --version
```

### Validações de Nome e Segurança Embutidas
Antes de criar qualquer arquivo no disco, o `cpp-bootstrap` normaliza espaços e aspas externas uma única vez e aplica regras uniformes entre Windows, Linux e macOS:
1. Rejeita nomes vazios ou compostos apenas por espaços (`exit 2`).
2. Rejeita nomes terminados com ponto final `.` para manter compatibilidade entre sistemas de arquivos via Git (`exit 2`).
3. Impede o uso de nomes de dispositivos reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`) (`exit 2`).
4. Aceita exclusivamente caracteres seguros para CMake e filesystems multiplataforma (`^[A-Za-z0-9_][A-Za-z0-9_-]*$`), bloqueando espaços internos (`"Meu Projeto"`), múltiplos tokens sem aspas, caracteres especiais (`\ / : * ? " < > |`) e início com `-` (`exit 2`).
5. Impede a sobrescrita se uma pasta ou arquivo com o mesmo nome já existir (`exit 2`).
6. Garante integridade transacional via **RAII (`DirectoryRollbackGuard`)**: em caso de falha inesperada de I/O, a pasta criada pela metade é removida automaticamente (`exit 10`).

---

## 4. Estrutura Gerada e Arquitetura do Projeto

### 4.1. Estrutura Gerada para Novos Projetos
Todos os arquivos gerados saem estritamente em **UTF-8 sem BOM** com quebras de linha **LF**:

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

| Arquivo | Descrição |
| :--- | :--- |
| `src/main.cpp` | Código de partida com locale portável do ambiente (`std::setlocale(LC_ALL, "")`), cabeçalho de autoria e pausa de terminal. |
| `include/` | Diretório dedicado para arquivos de cabeçalho (`.hpp` / `.h`). |
| `.vscode/launch.json` | Configuração de depuração multiplataforma (`cppdbg`) com blocos dedicados para `windows` (MSYS2 GDB), `linux` (`/usr/bin/gdb`) e `osx` (`lldb`). |
| `.vscode/settings.json` | Habilita `compile_commands.json` e configura o `--query-driver` do `clangd` para compiladores no Windows (MSYS2), Linux e macOS. |
| `CMakeLists.txt` | Configuração CMake com C++23 estrito, `CMAKE_BUILD_TYPE` padrão `Debug` não-destrutivo (respeita `-DCMAKE_BUILD_TYPE=Release` e geradores multi-config), inclusão de `include/`, coleta em `src/*.cpp`, flags rigorosas (`/W4 /permissive- /EHsc` no MSVC; `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors` no GCC/Clang) e linkagem automática de `stdc++exp` via `CheckLinkerFlag` para `<print>` e `<stacktrace>`. |
| `.gitignore` | Ignora pastas de build (`build/`, `out/`), binários e caches locais do `.vscode/*`, preservando `!.vscode/launch.json` e `!.vscode/settings.json` no controle de versão. |

### 4.2. Arquitetura Interna do Gerador (`cpp-bootstrap`)

```text
cpp-bootstrap/
├── .github/workflows/
│   ├── ci.yml                  # CI multiplataforma (Ubuntu, Windows MSVC/MSYS2, macOS)
│   └── release.yml             # Publicacao automatica de binarios ao criar tags v*
├── include/
│   ├── terminal.hpp            # Deteccao de SO, ANSI/VT e helpers de saida
│   ├── cli.hpp                 # Parser de argumentos e mensagens de ajuda/tutorial
│   ├── validator.hpp           # Validacao uniforme de nomes (5 regras)
│   ├── templates.hpp           # Templates embutidos (R"tpl(...)tpl") e renderizacao
│   └── generator.hpp           # Escrita UTF-8 sem BOM + DirectoryRollbackGuard (RAII)
├── src/
│   ├── cli.cpp
│   ├── validator.cpp
│   ├── generator.cpp
│   └── main.cpp
├── tests/
│   └── test_cpp_bootstrap.cpp  # Suite nativa em C++23 integrada ao CTest
└── CMakeLists.txt
```

---

## 5. Compilação e Execução de Projetos Gerados

Após criar seu projeto:

```powershell
cd MeuProjeto
code .
```

- **No VS Code:** pressione **F5** para compilar automaticamente e iniciar o depurador (`gdb` no Windows/Linux, `lldb` no macOS).
- **Via Terminal:**
  ```powershell
  cmake -B build
  cmake --build build
  ./build/MeuProjeto.exe   # Windows
  ./build/MeuProjeto       # Linux / macOS
  ```

---

## 6. Testes Automatizados (CTest) e CI/CD

A suíte de testes em [`tests/test_cpp_bootstrap.cpp`](tests/test_cpp_bootstrap.cpp) não utiliza dependências externas, executa em sandbox temporário isolado com limpeza automática via RAII (`TempSandboxGuard`) e cobre **20 cenários**:

| Nº | Categoria | Descrição do Teste |
| :--- | :--- | :--- |
| **01–07** | **Unitários** | Normalização (`trim_project_name`), rejeição de vazios/espaços/caracteres proibidos/ponto final/nomes reservados (`CON`, `AUX`, etc.) e `parse_cli` |
| **08–12** | **Integração CLI & Segurança** | Códigos de saída (`1` sem args, `0` em `--help`/`--version`, `2` em entradas inválidas e flags desconhecidas), bloqueio de sobrescrita e rollback RAII (`DirectoryRollbackGuard`) |
| **13–19** | **Estrutura & Encoding** | Trim ponta a ponta, `--quiet`, diretórios gerados, `main.cpp`, `.vscode/*.json`, `CMakeLists.txt` (9 seções), `.gitignore` e auditoria binária UTF-8 sem BOM + LF |
| **20** | **Integração Real C++23** | Configuração e compilação real via CMake (`-DCMAKE_BUILD_TYPE=Release`) + linkagem e execução de `<print>` / `<stacktrace>` via `stdc++exp` (`--include-build`) |

### Como Rodar os Testes Localmente

```powershell
# Padrao via CTest (testes 01 a 19; teste 20 marcado como SKIP)
ctest --test-dir build -C Release --output-on-failure

# Suite completa incluindo compilacao real C++23 (Windows MSYS2 / Linux / macOS)
./build/test_cpp_bootstrap --target-exe ./build/cpp-bootstrap --include-build

# Suite completa (Windows MSVC)
./build/Release/test_cpp_bootstrap.exe --target-exe ./build/Release/cpp-bootstrap.exe --include-build
```

### Automação no GitHub Actions
- **CI (`.github/workflows/ci.yml`):** valida build e executa todos os 20 testes a cada `push` ou `pull_request` na branch `main` em **Ubuntu 24.04 (GCC 14)**, **Windows (MSVC)**, **Windows (MSYS2 UCRT64 GCC)** e **macOS 15 (Clang)**.
- **Releases (`.github/workflows/release.yml`):** ao enviar uma tag no formato `v*` (ex.: `git tag -a v1.0.0 -m "release: v1.0.0" && git push origin v1.0.0`), compila os binários em `Release`, roda o `CTest` e publica automaticamente os pacotes `.zip` e `.tar.gz` na página de **GitHub Releases**.
