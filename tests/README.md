# Suíte de Testes Automatizados — C++ Bootstrap

Documentação detalhada da suíte de testes do gerador [`cpp_bootstrap.ps1`](../cpp_bootstrap.ps1), implementada em [`tests/test_cpp_bootstrap.ps1`](./test_cpp_bootstrap.ps1).

---

## 1. Visão Geral e Arquitetura

A suíte de testes tem como objetivo validar a estabilidade, a segurança, as regras de nomenclatura e a integridade de todos os modelos de código gerados pelo script principal.

### Princípios de Concepção

1. **Isolamento Total (Sandbox Temporário):**
   - Os testes **nunca** criam pastas ou arquivos dentro da árvore do repositório.
   - Cada execução cria uma pasta temporária exclusiva em `$env:TEMP` (com um GUID aleatório).
   - O `Set-Location` executado internamente pelo [`cpp_bootstrap.ps1`](../cpp_bootstrap.ps1) não afeta o shell do usuário, pois cada teste roda em um subprocesso filho isolado (`System.Diagnostics.Process`).

2. **Teardown Automático:**
   - Ao final da execução (mesmo se ocorrer falha ou cancelamento abrupto), o bloco `finally` remove a pasta temporária do disco de forma limpa.

3. **Compatibilidade Multiplataforma:**
   - Funciona em **PowerShell 7+ (`pwsh`)** e **Windows PowerShell 5.1**.
   - Se invocado a partir do Windows PowerShell 5.1 e o `pwsh` estiver presente no sistema, o script delega a execução automaticamente para o PowerShell 7 para garantir renderização correta de sequências ANSI e compatibilidade com UTF-8 sem BOM.

---

## 2. Cenários de Teste Cobertos

A suíte abrange 14 testes divididos em 4 etapas lógicas:

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

## 3. Como Executar

### 3.1. Execução Padrão (Rápida)
Executa a análise de sintaxe, todos os testes de segurança/validação e a checagem da estrutura gerada:

```powershell
./tests/test_cpp_bootstrap.ps1
```

*(No Linux ou macOS, utilize `pwsh ./tests/test_cpp_bootstrap.ps1`)*

### 3.2. Execução com Compilação Real C++23
Gera um projeto real na pasta temporária e invoca `cmake -B build` e `cmake --build build` com o GCC/G++ do MSYS2:

```powershell
./tests/test_cpp_bootstrap.ps1 -IncludeBuild
```

### 3.3. Modo Detalhado (Verbose)
Exibe a saída bruta de console (`stdout` e `stderr`) gerada em cada caso de teste:

```powershell
./tests/test_cpp_bootstrap.ps1 -VerboseOutput
```

### 3.4. Testar Script Específico
Permite apontar para outro script de destino para homologação:

```powershell
./tests/test_cpp_bootstrap.ps1 -TargetScript "caminho/para/outro_script.ps1"
```

---

## 4. Códigos de Saída do Runner (Exit Codes)

O script [`test_cpp_bootstrap.ps1`](./test_cpp_bootstrap.ps1) segue convenções padrão para integração contínua (CI/CD):

- **`0`**: Todos os testes passaram com sucesso.
- **`1`**: Um ou mais testes falharam ou erro crítico impediu a execução.
