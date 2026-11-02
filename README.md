---
title: C++ Bootstrap
author: Bruno Costa
tags:
  - cpp
  - powershell
  - cmake
  - obsidian
---

# C++ Bootstrap

Utilitário multiplataforma em PowerShell para inicialização rápida de projetos em **C++23** com **CMake**, estrutura de diretórios padronizada e ambiente pronto para desenvolvimento e depuração no **Visual Studio Code** (Windows, Linux e macOS).

---

## 1. Apresentação

- **Título:** C++ Bootstrap
- **Descrição:** Gerador automatizado de estrutura e arquivos de configuração para novos projetos C++.
- **Autoria:** Bruno Costa ([@kbelx](https://github.com/kbelx))
- **Padrão C++:** C++23 estrito (`set(CMAKE_CXX_STANDARD_REQUIRED ON)`)
- **Build System:** CMake (mínimo 3.20)
- **Compilador / Debugger:**
  - **Windows:** GCC (`g++`) e GDB via MSYS2 (UCRT64) ou MSVC
  - **Linux:** GCC / Clang e GDB (`/usr/bin/gdb`)
  - **macOS:** Clang / GCC e LLDB (`lldb`)

---

## 2. Pré-requisitos do Ambiente

Para compilar e depurar os projetos gerados sem configurações manuais adicionais:

1. **PowerShell**: PowerShell 7+ (`pwsh` no Windows, Linux ou macOS) ou Windows PowerShell 5.1.
2. **CMake**: Versão 3.20 ou superior adicionada ao `PATH`.
3. **Toolchain C++23**:
   - **Windows (MSYS2 UCRT64 em `C:\msys64\ucrt64\`)**: pacotes `mingw-w64-ucrt-x86_64-gcc` e `mingw-w64-ucrt-x86_64-gdb`.
   - **Linux**: `g++` (GCC 13+) ou `clang++` (Clang 17+) e `gdb`.
   - **macOS**: Xcode Command Line Tools (`clang++` e `lldb`) ou Homebrew LLVM/GCC.
4. **Visual Studio Code**:
   - Extensão **CMake Tools**
   - Extensão **C/C++** (`ms-vscode.cpptools`, para o motor de depuração `cppdbg`)
   - Extensão **clangd** (com análise de código apontando para `compile_commands.json`)

---

## 3. Como Usar

Execute o script [`cpp_bootstrap.ps1`](cpp_bootstrap.ps1) informando o nome do novo projeto:

### No Windows (PowerShell 7+ ou Windows PowerShell 5.1):
```powershell
./cpp_bootstrap.ps1 MeuProjeto
```

### No Linux ou macOS:
```powershell
pwsh ./cpp_bootstrap.ps1 MeuProjeto
```

### Modo Enxuto (`-Quiet`):
Opcionalmente, suprima o rodapé de autoria e links ao usar o script em automações:
```powershell
./cpp_bootstrap.ps1 MeuProjeto -Quiet
```

### Validações de Nome e Segurança Embutidas
O script normaliza espaços/aspas externas uma única vez e aplica regras uniformes em todos os sistemas operacionais (garantindo portabilidade via Git e integridade do `CMakeLists.txt`):
- Rejeita nomes vazios ou compostos apenas por espaços.
- Rejeita nomes terminados com ponto final `.` (restrição de filesystem do Windows preservada para portabilidade).
- Impede o uso de dispositivos reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`).
- Aceita exclusivamente caracteres seguros para CMake e filesystems multiplataforma (`^[A-Za-z0-9_][A-Za-z0-9_-]*$`), bloqueando espaços internos (`"Meu Projeto"`), caracteres especiais (`\ / : * ? " < > |`) e início com `-`.
- Impede a sobrescrita acidental se uma pasta ou arquivo com o mesmo nome já existir no diretório atual.
- Não altera o diretório atual do terminal do usuário e remove a pasta criada automaticamente em caso de falha inesperada de I/O (*rollback* transacional).

---

## 4. Estrutura de Diretórios Gerada

Ao criar um projeto, a seguinte árvore é gerada em **UTF-8 sem BOM** e quebras de linha **LF**:

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

### Anatomia dos Arquivos Gerados

| Arquivo | Descrição |
| :--- | :--- |
| `src/main.cpp` | Código de partida com locale portável do ambiente (`std::setlocale(LC_ALL, "")`), cabeçalho de autoria e pausa de terminal. |
| `include/` | Diretório dedicado para arquivos de cabeçalho (`.hpp` / `.h`). |
| `.vscode/launch.json` | Configuração de depuração multiplataforma (`cppdbg`) com blocos dedicados para `windows` (MSYS2 GDB), `linux` (`/usr/bin/gdb`) e `osx` (`lldb`). |
| `.vscode/settings.json` | Habilita `compile_commands.json` e configura o `--query-driver` do `clangd` para compiladores no Windows (MSYS2), Linux e macOS. |
| `CMakeLists.txt` | Configuração CMake com C++23 estrito, `CMAKE_BUILD_TYPE` padrão `Debug` não-destrutivo (respeita `-DCMAKE_BUILD_TYPE=Release` e geradores multi-config), inclusão de `include/`, coleta em `src/*.cpp`, flags rigorosas (`/W4 /permissive- /EHsc` no MSVC; `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors` no GCC/Clang) e linkagem automática de `stdc++exp` via `CheckLinkerFlag` para recursos C++23 da `libstdc++` (`<print>`, `<stacktrace>`). |
| `.gitignore` | Ignora pastas de build (`build/`, `out/`), binários e caches locais do `.vscode/*`, preservando `!.vscode/launch.json` e `!.vscode/settings.json` no controle de versão. |

---

## 5. Compilação e Execução

Após gerar o projeto, entre na pasta criada:

```powershell
cd MeuProjeto
code .
```

### Depuração no Visual Studio Code:
- Pressione **F5** para compilar automaticamente e iniciar o depurador da plataforma atual (`gdb` ou `lldb`).

### Compilação Manual via Terminal:
```powershell
# 1. Configurar os arquivos de build (padrão Debug; opcional: -DCMAKE_BUILD_TYPE=Release)
cmake -B build

# 2. Compilar o projeto
cmake --build build

# 3. Executar o binário gerado
# Windows:
./build/MeuProjeto.exe
# Linux / macOS:
./build/MeuProjeto
```

---

## 6. Ambiente de Testes Automatizados

O repositório inclui uma suíte completa de testes automatizados em [`tests/test_cpp_bootstrap.ps1`](tests/test_cpp_bootstrap.ps1), projetada para validar a segurança de parsing/encoding, as regras de validação de nomes, o isolamento de diretório, a geração de todos os templates, a compatibilidade direta com Windows PowerShell 5.1 e a compilação C++23 real de ponta a ponta.

### 6.1. Arquitetura e Princípios

1. **Blindagem de Parsing e Encoding (ASCII 7-bit / UTF-8 sem BOM):**
   - Os scripts `.ps1` contêm exclusivamente bytes ASCII 7-bit (`<= 0x7F`) sem BOM e utilizam `[char]27` para sequências ANSI quando o terminal suporta VT, eliminando erros de lexer e *mojibake* em qualquer codepage do Windows PowerShell 5.1 e do PowerShell 7+.

2. **Isolamento Total (Sandbox Temporário) e Teardown Automático:**
   - Nenhum teste cria artefatos dentro da árvore do repositório; cada execução utiliza uma pasta temporária exclusiva com GUID em `[System.IO.Path]::GetTempPath()` removida automaticamente no bloco `finally`.

3. **Execução Nativa Multi-Interpretador:**
   - Roda nativamente tanto no **PowerShell 7+ (`pwsh`)** (Windows, Linux e macOS) quanto diretamente no **Windows PowerShell 5.1 (`powershell.exe`)**, sem mascarar falhas de nenhum interpretador.

---

### 6.2. Matriz dos 20 Cenários de Teste

| Nº | Categoria | Descrição do Teste | Critério de Sucesso |
| :--- | :--- | :--- | :--- |
| **01** | **Sintaxe & Encoding** | Auditoria estática de bytes e AST (`Parser::ParseFile`) nos scripts `.ps1` | Sem BOM, 100% bytes ASCII `<= 0x7F` e zero erros de parsing |
| **02** | **Validação** | Chamada sem argumentos (`./cpp_bootstrap.ps1`) | Exit code `1` e exibição do tutorial de uso |
| **03** | **Validação** | Nome vazio ou apenas espaços (`""`, `"   "`, `'""'`) | Exit code `2` e mensagem `"Nome do projeto vazio"` |
| **04** | **Validação** | Caracteres proibidos no sistema de arquivos (`Projeto<Invalido>`) | Exit code `2` e alerta de caracteres proibidos |
| **05** | **Validação** | Nome com espaço interno (`"Meu Projeto"`) ou múltiplos tokens sem aspas | Exit code `2` protegendo a sintaxe do `CMakeLists.txt` |
| **06** | **Validação** | Nome terminando com ponto final `.` | Exit code `2` em todas as plataformas |
| **07** | **Validação** | Nomes de dispositivos reservados (`CON`, `PRN`, `AUX`, `NUL`, etc.) | Exit code `2` em todas as plataformas |
| **08** | **Segurança** | Prevenção de sobrescrita em pasta ou arquivo preexistente | Exit code `2` sem modificar o item existente |
| **09** | **Normalização** | Nome com espaços externos (`"  ProjetoTrimado  "`) | Exit code `0`, pasta exata `ProjetoTrimado` e CMake sincronizado |
| **10** | **Isolamento** | Execução *in-process* (`& ./cpp_bootstrap.ps1 ... -Quiet`) | Diretório atual (`Get-Location`) permanece inalterado |
| **11** | **Criação** | Execução com nome válido (`ProjetoTesteSucesso`) | Exit code `0` e instruções `cmake -B build` / `cmake --build build` |
| **12** | **Estrutura** | Verificação das pastas obrigatórias | Pastas raiz, `src/`, `include/` e `.vscode/` presentes |
| **13** | **Artefatos** | Integridade de `src/main.cpp` | Presença de `int main()`, `<iostream>`, `<clocale>` e `std::setlocale(LC_ALL, "")` |
| **14** | **Artefatos** | Configurações multiplataforma do VS Code (`launch.json` e `settings.json`) | JSON válido com blocos `windows`/`linux`/`osx` e `--query-driver` multi-SO |
| **15** | **Artefatos** | Configuração do `CMakeLists.txt` | Template substituído, `C++23` estrito, guarda de `CMAKE_BUILD_TYPE`, flags rigorosas e Seção 9 (`stdc++exp`) |
| **16** | **Artefatos** | Regras do `.gitignore` gerado | Ignora `build/`, binários e `.vscode/*`, preservando `!.vscode/launch.json` e `!.vscode/settings.json` |
| **17** | **Encoding** | Auditoria binária dos 5 arquivos gerados | Todos estritamente em UTF-8 sem BOM e fim de linha LF |
| **18** | **Compatibilidade** | Execução direta no Windows PowerShell 5.1 (`powershell.exe`) | Exit code `0`, sem vazamento ANSI `e[` e sem gravação de BOM (`SKIP` fora do Windows) |
| **19** | **Integração** | Compilação C++23 real com CMake (`-DCMAKE_BUILD_TYPE=Release`) | Executável compilado com sucesso e `CMAKE_BUILD_TYPE` respeitado |
| **20** | **Integração** | Linkagem e execução real C++23 (`<print>` / `<stacktrace>`) via `stdc++exp` | Binário C++23 com `std::println`/`std::stacktrace` linka e executa com exit code `0` |

---

### 6.3. Como Executar os Testes

#### 1. Execução Padrão (Sintaxe, Validações, Isolamento e Estrutura)
```powershell
./tests/test_cpp_bootstrap.ps1
```
*(No Linux ou macOS: `pwsh ./tests/test_cpp_bootstrap.ps1`)*

#### 2. Execução Completa com Compilação Real C++23 (CMake + Compilador C++)
```powershell
./tests/test_cpp_bootstrap.ps1 -IncludeBuild
```

#### 3. Modo Detalhado (Verbose)
```powershell
./tests/test_cpp_bootstrap.ps1 -VerboseOutput
```

#### 4. Testar Script Específico
```powershell
./tests/test_cpp_bootstrap.ps1 -TargetScript "caminho/para/outro_script.ps1"
```

---

### 6.4. Códigos de Saída do Runner (Exit Codes)

- **`0`**: Todos os testes executados passaram com sucesso.
- **`1`**: Um ou mais testes falharam ou erro crítico impediu a execução.
