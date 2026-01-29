# C++ Project Generator (PowerShell)

Script em PowerShell para gerar automaticamente projetos C++ com:

- Estrutura padrão (`src/`, `include/`, `.vscode/`)
- CMake configurado com C++23
- Integração com clangd e compile_commands.json
- Ambiente MSYS2 (GCC) + VS Code

---

## Uso

```powershell
./criar_projeto_cpp.ps1 MeuProjeto
```

---
## Requisitos

- Windows
- PowerShell 7+ (ou Windows PowerShell)
- CMake
- MSYS2 (UCRT64)
- VS Code + extensões CMake Tools e clangd

