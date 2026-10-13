# Agent Guide — Project Bootstrap

## 1. Contexto
Este projeto é um gerador de estrutura de diretórios e arquivos de configuração para projetos C++23 com CMake (`cpp_bootstrap.ps1`).

O agente deve ajudar a:
- Manter consistência estrutural e integridade dos templates gerados.
- Garantir que a documentação única no `README.md` reflita exatamente o que o script produz.
- Automatizar tarefas mantendo compatibilidade cross-platform (PowerShell Core / Windows PowerShell).

---

## 2. Objetivo do Agente
- Gerar e manter estruturas de projeto de forma padronizada e eficiente.
- Documentar o código no próprio script e manter a documentação consolidada no `README.md`.
- Garantir que `cpp_bootstrap.ps1` seja a única fonte de verdade para os templates embutidos.
- Garantir o commit e o push apropriado para cada tarefa.

---

## 3. Estrutura Padrão Gerada

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

### Configuração do CMakeLists.txt Gerado
O script gera um arquivo `CMakeLists.txt` configurado com:
- `cmake_minimum_required(VERSION 3.20)`
- `set(CMAKE_CXX_STANDARD 23)` e `set(CMAKE_CXX_STANDARD_REQUIRED ON)`
- `set(CMAKE_BUILD_TYPE Debug)`
- Coleta de fontes via `file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")`
- Diretório de cabeçalhos privado apontando para `include`
- Flags rigorosas de compilação:
  - MSVC: `/W4 /permissive- /EHsc`
  - GCC / Clang: `-Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors -fdiagnostics-color=always`

---

## 4. Comportamento Esperado

- Sempre criar diretórios antes de criar arquivos.
- Nunca sobrescrever arquivos sem confirmação do usuário.
- Usar nomes em `snake_case` para scripts e arquivos auxiliares.
- Manter código limpo, comentado e interativo, evitando comandos desnecessários ou código morto.
- Manter a codificação estritamente em UTF-8 sem BOM.

---

## 5. Comandos Disponíveis

### Criar projeto (Windows)
```powershell
./cpp_bootstrap.ps1 NomeDoProjeto
```

### Criar projeto (Linux ou macOS)
```powershell
pwsh ./cpp_bootstrap.ps1 NomeDoProjeto
```