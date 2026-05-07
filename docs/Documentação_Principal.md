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

CLI em PowerShell para gerar rapidamente projetos C++ com estrutura padronizada, CMake e arquivos de apoio para desenvolvimento local.

## Apresentação

- **Título:** C++ Bootstrap
- **Descrição:** utilitário de linha de comando para criar projetos C++ prontos para uso com `src/`, `include/`, `.vscode/` e `CMakeLists.txt`.
- **Autoria:** Bruno Costa

## Estrutura de Diretórios

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

## Uso

### Instalação global

Para criar projetos de qualquer pasta do PC, execute uma vez o instalador na raiz do repositório:

```powershell
./install.ps1
```

Depois disso, abra um novo terminal e use:

```powershell
cpp-bootstrap MeuProjeto
```

### Execução local

Se preferir executar direto do repositório, use:

### Windows

```powershell
./cpp_bootstrap.ps1 MeuProjeto
```

### Linux e macOS

```powershell
pwsh ./cpp_bootstrap.ps1 MeuProjeto
```

## Build

Depois de gerar o projeto, entre na pasta criada e execute:

```powershell
mkdir build
cd build
cmake ..
cmake --build .
```

Para executar o alvo auxiliar:

```powershell
cmake --build . --target run
```

## Observações

- A documentação foi mantida em uma pasta dedicada para funcionar bem como vault no Obsidian.
- O arquivo raiz [`README.md`](../README.md) fica apenas como ponto de entrada para esta documentação.
- O comando global `cpp-bootstrap` aponta para o mesmo gerador do arquivo local `cpp_bootstrap.ps1`.
