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

Utilitário em PowerShell para inicialização rápida de projetos em **C++23** com **CMake**, estrutura de diretórios padronizada e ambiente pronto para desenvolvimento e depuração no **Visual Studio Code**.

---

## 1. Apresentação

- **Título:** C++ Bootstrap
- **Descrição:** Gerador automatizado de estrutura e arquivos de configuração para novos projetos C++.
- **Autoria:** Bruno Costa ([@kbelx_](https://github.com/kbelx_))
- **Padrão C++:** C++23 estrito (`set(CMAKE_CXX_STANDARD_REQUIRED ON)`)
- **Build System:** CMake (mínimo 3.20)
- **Compilador / Debugger:** GCC (`g++`) e GDB via MSYS2 (UCRT64)

---

## 2. Pré-requisitos do Ambiente

Para compilar e depurar os projetos gerados sem configurações manuais adicionais:

1. **PowerShell**: PowerShell 7+ (recomendado) ou Windows PowerShell.
2. **CMake**: Versão 3.20 ou superior adicionada ao `PATH`.
3. **MSYS2 (UCRT64)**: Instalado no caminho padrão (`C:\msys64\ucrt64\`), com os seguintes pacotes instalados:
   - `mingw-w64-ucrt-x86_64-gcc`
   - `mingw-w64-ucrt-x86_64-gdb`
4. **Visual Studio Code**:
   - Extensão **CMake Tools**
   - Extensão **clangd** (com a análise de código apontando para `compile_commands.json`)

---

## 3. Como Usar

Execute o script [`cpp_bootstrap.ps1`](cpp_bootstrap.ps1) na raiz do repositório informando o nome do novo projeto:

### No Windows:
```powershell
./cpp_bootstrap.ps1 MeuProjeto
```

### No Linux ou macOS:
```powershell
pwsh ./cpp_bootstrap.ps1 MeuProjeto
```

### Validações de Nome Embutidas
O script realiza checagens automáticas de segurança antes de criar qualquer arquivo:
- Rejeita nomes vazios ou que terminem com ponto final `.` ou espaço (regras de sistema de arquivos do Windows).
- Bloqueia caracteres proibidos (`\ / : * ? " < > |` ou nulos).
- Impede o uso de palavras reservadas do sistema (ex.: `CON`, `PRN`, `AUX`, `NUL`, `COM1-9`, `LPT1-9`).
- Impede a sobrescrita acidental se uma pasta com o mesmo nome já existir no diretório atual.

---

## 4. Estrutura de Diretórios Gerada

Ao criar um projeto, a seguinte árvore é gerada:

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
| `src/main.cpp` | Código de partida com suporte a acentuação em português (`std::setlocale`), cabeçalho de autoria e pausa de terminal. |
| `include/` | Diretório dedicado para arquivos de cabeçalho (`.hpp` / `.h`). |
| `.vscode/launch.json` | Configuração de depuração pronta (`cppdbg`) apontando para `C:/msys64/ucrt64/bin/gdb.exe` e ativando pretty-printing. |
| `.vscode/settings.json` | Configuração que habilita `compile_commands.json` e direciona o `clangd` para o compilador do MSYS2. |
| `CMakeLists.txt` | Configuração CMake completa com C++23, inclusão de `include/`, fontes em `src/*.cpp` e flags de aviso rigorosas (`/W4` no MSVC, `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors` no GCC/Clang). |
| `.gitignore` | Regras prontas para ignorar pastas de build (`build/`, `out/`), binários (`*.exe`, bibliotecas, objetos) e caches locais de IDE. |

---

## 5. Compilação e Execução

Após gerar o projeto, navegue até a pasta criada:

```powershell
cd MeuProjeto
code .
```

### Depuração no Visual Studio Code:
- Pressione **F5** para iniciar a compilação automática e depurar com o GDB integrado.

### Compilação Manual via Terminal:
```powershell
# 1. Gerar os arquivos de build
cmake -B build

# 2. Compilar o projeto
cmake --build build

# 3. Executar o binário gerado (Windows)
./build/MeuProjeto.exe
```

---

## 6. Ambiente de Testes Automatizados

O repositório inclui uma suíte completa de testes automatizados em [`tests/test_cpp_bootstrap.ps1`](tests/test_cpp_bootstrap.ps1), projetada para validar a integridade sintática, as regras de segurança/validação de nomes, a correta geração de todos os arquivos e modelos, além de permitir o teste de compilação real de ponta a ponta.

### 6.1. Arquitetura e Princípios

1. **Isolamento Total (Sandbox Temporário):**
   - Os testes nunca criam arquivos dentro da árvore do repositório.
   - Cada execução cria uma pasta temporária exclusiva em `$env:TEMP` com um GUID aleatório.
   - O `Set-Location` interno do [`cpp_bootstrap.ps1`](cpp_bootstrap.ps1) não afeta o shell do usuário, pois cada teste roda em um subprocesso filho isolado (`System.Diagnostics.Process`).

2. **Teardown Automático:**
   - Ao final da execução (mesmo se ocorrer falha ou cancelamento abrupto), o bloco `finally` remove a pasta temporária do disco de forma limpa.

3. **Compatibilidade Multiplataforma:**
   - Funciona em **PowerShell 7+ (`pwsh`)** e **Windows PowerShell 5.1**.
   - Se invocado a partir do Windows PowerShell 5.1 e o `pwsh` estiver presente no sistema, o script delega a execução automaticamente para o PowerShell 7 para garantir suporte completo a sequências ANSI e UTF-8 sem BOM.

---

### 6.2. Matriz dos 14 Cenários de Teste

| Nº | Categoria | Descrição do Teste | Critério de Sucesso |
| :--- | :--- | :--- | :--- |
| **01** | **Sintaxe** | Análise estática da AST do script via `[System.Management.Automation.Language.Parser]` | Sem erros de parsing no PowerShell |
| **02** | **Validação** | Chamada sem argumentos (`./cpp_bootstrap.ps1`) | Exit code `1` e exibição do tutorial de uso |
| **03** | **Validação** | Nome vazio ou apenas espaços (`""`, `"   "`) | Exit code `2` e mensagem `"Nome do projeto vazio"` |
| **04** | **Validação** | Caracteres proibidos no sistema de arquivos (`< > : " / \ \| ? *`) | Exit code `2` e alerta de caracteres proibidos |
| **05** | **Validação** | Nome terminando com ponto final `.` ou espaço (regra Windows) | Exit code `2` e alerta da restrição de sistema |
| **06** | **Validação** | Nomes de dispositivos reservados do Windows (`CON`, `PRN`, `AUX`, `NUL`, etc.) | Exit code `2` e alerta de nome reservado |
| **07** | **Segurança** | Prevenção de sobrescrita acidental em pasta preexistente | Exit code `2` e aviso de pasta existente sem alterar nada |
| **08** | **Criação** | Execução com nome de projeto válido (`ProjetoTesteSucesso`) | Exit code `0` e mensagem de confirmação |
| **09** | **Estrutura** | Verificação das pastas geradas | Pastas raiz, `src/`, `include/` e `.vscode/` presentes |
| **10** | **Artefatos** | Integridade do código C++ (`src/main.cpp`) | Presença de `int main()`, `#include <iostream>` e `std::setlocale` |
| **11** | **Artefatos** | Configurações do VS Code (`launch.json` e `settings.json`) | Arquivos JSON válidos com configurações corretas de debug e clangd |
| **12** | **Artefatos** | Configuração de compilação (`CMakeLists.txt`) | Template substituído, `C++23` estrito e flags de compilação presentes |
| **13** | **Artefatos** | Regras de versão (`.gitignore`) | Presença de exclusões de `build/`, `out/`, binários e caches de IDE |
| **14** | **Integração** | Compilação real de ponta a ponta com CMake e `g++` | Executável C++23 gerado com sucesso sem erros |

---

### 6.3. Como Executar os Testes

#### 1. Execução Padrão (Sintaxe, Validações e Estrutura)
```powershell
./tests/test_cpp_bootstrap.ps1
```
*(No Linux ou macOS: `pwsh ./tests/test_cpp_bootstrap.ps1`)*

#### 2. Execução Completa com Compilação Real C++23 (CMake + g++)
Testa a geração e compila o código do projeto gerado para validar que o `CMakeLists.txt` e o `src/main.cpp` compilam sem warnings/erros:
```powershell
./tests/test_cpp_bootstrap.ps1 -IncludeBuild
```

#### 3. Modo Detalhado (Verbose)
Exibe a saída bruta (`stdout`/`stderr`) de cada comando e processo invocado internamente:
```powershell
./tests/test_cpp_bootstrap.ps1 -VerboseOutput
```

#### 4. Testar Script Específico
Permite apontar para outro script de destino para homologação:
```powershell
./tests/test_cpp_bootstrap.ps1 -TargetScript "caminho/para/outro_script.ps1"
```

---

### 6.4. Códigos de Saída do Runner (Exit Codes)

O script [`test_cpp_bootstrap.ps1`](tests/test_cpp_bootstrap.ps1) segue convenções padrão para integração contínua (CI/CD):

- **`0`**: Todos os testes passaram com sucesso.
- **`1`**: Um ou mais testes falharam ou erro crítico impediu a execução.


