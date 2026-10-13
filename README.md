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
