# Agent Guide — Project Bootstrap

## 1. Contexto

Este projeto se chama **C++ Bootstrap**. É um gerador CLI escrito em **C++23** para inicialização rápida de estruturas de diretórios e arquivos de configuração para **novos projetos C++23 com CMake**.

As versões em PowerShell permanecem **EM LEGADO** (_ou seja, estão CONGELADAS_) na branch `legacy`. A partir da `v1.0.0`, o código principal da branch `main` **é inteiramente em C++23**.

---

## 2. Objetivos do Agente

### 2.1 Formatação
- **Inicialização de Variável**: Inicialização Uniforme para inicializar valores, Inicialização por Cópia para atribuir
- **Variáveis e Parâmetros NÃO CONSTANTES:** usar apenas *PascalCase*, em língua portuguesa (PT-BR). Inici
- **Variáveis e Parâmetros CONSTANTES:** usar apenas *UPPERCASE*, em língua portuguesa (PT-BR). Inici
- **Funções:** usar apenas *camelCase*, em língua portuguesa (PT-BR).
- **Comentários**: usar em Lingua Portuguesa (PT-BR).

### 2.2 Organização de Código
- Preferir a estratégia "dividir para conquistar".
- Ao dividir funções grandes em pequenas funções, considerar agrupá-las em namespaces; ao ter múltiplos namespaces, considerar agrupá-los em um namespace-pai.
- como a Lingua Portuguesa possui nomes/palavras grandes comparados aos Inglês, considere abreviar palavras/nomes compostos ( ex.: Sistema Operacional -> SO) afim de manter legibilidade

### 2.3 Arquitetura
- Manter consistência estrutural, modularidade (`include/` + `src/`) e integridade entre os arquivos.
- Garantir que a documentação única no `README.md` reflita exatamente o que o binário produz.
- Garantir que `include/templates.hpp` seja a única fonte de verdade para os templates embutidos.
- Garantir que toda alteração passe pela suíte de testes no `CTest` (`tests/test_cpp_bootstrap.cpp`) com `--include-build`.
- Evoluir e manter a arquitetura modular em C++23 com comentários didáticos em português e estilo limpo.

### 2.4 Compilação
- Manter compilação limpa com zero avisos sob flags rigorosas em Windows (MSVC e MSYS2 GCC/Clang), Linux (GCC/Clang) e macOS (Clang).

### 2.5 Commits e Pushes
- **Commits:** apenas se o usuário pedir.
- **Pushes:** apenas se o usuário pedir.

---

## 3. Comportamento Esperado

### 3.1 Sistema de Arquivos e Segurança
- Sempre criar diretórios antes de criar arquivos, sem alterar o diretório de trabalho do processo chamador e com rollback RAII automático (`DirectoryRollbackGuard`) em caso de falha.
- Nunca sobrescrever pastas ou arquivos existentes.

### 3.2 Qualidade e Codificação
- Manter código limpo, comentado e didático, sem avisos de compilação
- Manter a codificação de todos os arquivos estritamente em **UTF-8 sem BOM** com fim de linha **LF**; nos arquivos `.hpp`, `.cpp` e `CMakeLists.txt`, manter apenas caracteres **ASCII 7-bit (`<= 0x7F`)**.

---

## 4. Estrutura Padrão Gerada para Novos Projetos

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

---

## 5. Comandos Disponíveis

### 5.1 Compilar e Testar o `cpp-bootstrap`
```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### 5.2 Executar Suíte Completa com Compilação Real C++23
```powershell
./build/test_cpp_bootstrap --target-exe ./build/cpp-bootstrap --include-build
```

### 5.3 Uso do Binário
```powershell
cpp-bootstrap NomeDoProjeto
cpp-bootstrap NomeDoProjeto -q
cpp-bootstrap --help
cpp-bootstrap --version
```

---

## 6. Versionamento e Git

Esta seção estabelece as diretrizes operacionais para controle de versão, padronização de commits, envio remoto e higienização do repositório **C++ Bootstrap** (`cpp-bootstrap`).

### 6.1 Versionamento Semântico (SemVer)
O projeto segue a especificação **Semantic Versioning (SemVer)** no formato `MAJOR.MINOR.PATCH` (com tags Git no padrão `vMAJOR.MINOR.PATCH`, ex.: `v1.0.0`):

| Incremento | Formato | Critério no `cpp-bootstrap` | Correlação com _Conventional Commits_ |
| :--- | :--- | :--- | :--- |
| **MAJOR** | `X.0.0` | Mudanças incompatíveis na interface de linha de comando (CLI), códigos de saída ou estrutura padrão gerada. | `BREAKING CHANGE` / `<tipo>!:` |
| **MINOR** | `0.Y.0` | Novas funcionalidades, flags ou melhorias nos templates gerados mantendo retrocompatibilidade. | `feat` |
| **PATCH** | `0.0.Z` | Correções de bugs, ajustes internos, validação ou portabilidade sem alterar o contrato da CLI. | `fix` / `refactor` |

- **Sincronia de Versão:** ao atualizar a versão do projeto, manter o valor sincronizado entre `CMakeLists.txt` (`project(cpp-bootstrap VERSION X.Y.Z)`), a constante de versão no código (`VERSAO`) e a documentação (`README.md`).

### 6.2 Padrão de Mensagens de Commit
Todos os commits deste repositório **DEVEM** seguir a especificação _Conventional Commits_ no formato:

```text
<tipo>(<escopo>): <descrição concisa no infinitivo>
```

| Tipo       | Escopos Comuns                                           | Propósito / Quando Utilizar                                                                                                   | Exemplo Real do Projeto                                                               |
| :--------- | :------------------------------------------------------- | :---------------------------------------------------------------------------------------------------------------------------- | :------------------------------------------------------------------------------------ |
| `feat`     | `cli`, `generator`, `templates`, `validator`, `terminal` | Adição de novas funcionalidades, opções de linha de comando ou novos modelos embutidos.                                       | `feat(cli): adicionar suporte a flag silenciosa --quiet`                              |
| `fix`      | `validator`, `generator`, `terminal`, `cli`              | Correção de bugs na validação de nomes, criação de arquivos, rollback RAII ou detecção ANSI/VT.                               | `fix(generator): garantir escrita estrita em UTF-8 sem BOM`                           |
| `refactor` | `terminal`, `validator`, `cli`, `generator`              | Reestruturação de namespaces, limpeza de cabeçalhos ou padronização para *PascalCase*/*camelCase* sem alterar comportamento.  | `refactor(terminal): fragmentar codigo em namespaces Cores, Ambiente e Saida`         |
| `test`     | `ctest`, `suite`                                         | Adição ou ajuste de testes unitários e de integração em `tests/test_cpp_bootstrap.cpp`.                                       | `test(suite): validar criacao de projeto com compilacao real C++23`                   |
| `docs`     | `readme`, `agents`, `comments`                           | Atualização da documentação em `README.md`, regras em `AGENTS.md` ou comentários didáticos nos cabeçalhos.                    | `docs(agents): adaptar diretrizes de versionamento para o cpp-bootstrap`              |
| `chore`    | `cmake`, `ci`, `gitignore`                               | Ajustes em `CMakeLists.txt`, pipelines do GitHub Actions (`.github/workflows/`) e regras de arquivos ignorados.               | `chore(cmake): configurar flags rigorosas de compilacao para MSVC e GCC`              |

> [!TIP]
> **Melhores Práticas:** Mantenha os commits atômicos e coesos — separe alterações de código-fonte e testes (`feat` / `fix` / `refactor` / `test`) de atualizações de documentação ou configuração de build (`docs` / `chore`).

### 6.3 Fluxo Operacional no Terminal (_PowerShell_)
Como o ambiente de desenvolvimento principal roda sob Windows, todas as rotinas Git (quando solicitadas pelo usuário) devem ser executadas via **PowerShell**:

1. **Inspeção Prévia:** Executar sempre `git status` e `git diff --stat` antes de preparar qualquer commit para auditar os arquivos modificados em `include/`, `src/`, `tests/` e na raiz do projeto.
2. **Preparação Seletiva (_Staging_):** Preferir `git add <caminho>` (ex.: `git add include/terminal.hpp`) em vez de `git add .` indiscriminado quando houver pastas de compilação (`build/`), binários de teste ou alterações não relacionadas na árvore de trabalho.
3. **Verificação Pós-Operação:** Executar `git status` após o commit ou push para confirmar que a branch principal (`main`) encontra-se limpa e sincronizada com a remota (`origin/main`).

Exemplo de rotina padrão no PowerShell:
```powershell
git status
git diff --stat
git add include/terminal.hpp
git commit -m "refactor(terminal): organizar cabecalhos e separar codigo em namespaces"
git status
```

### 6.4 Higienização e Controle de Arquivos Transitórios
- **Artefatos de Compilação (`build/`, `out/`, `Testing/`):** Os diretórios e binários gerados pelo CMake, compiladores C++ e CTest possuem caráter estritamente local e transitório, nunca devendo ser versionados.
- **Projetos Gerados em Testes:** Qualquer diretório de projeto criado manualmente para testar o binário `cpp-bootstrap` fora da pasta temporária de testes deve ser removido antes de preparar commits.
- **Proibição de Poluição do Histórico:** Assegurar que executáveis (`.exe`), objetos de compilação (`.obj`, `.o`, `.pdb`) ou arquivos temporários de rascunho não sejam commitados junto ao código-fonte em `include/`, `src/` e `tests/`.

### 6.5 Privacidade e Identidade do Autor
- Respeitar estritamente as configurações locais e globais de `user.name` e `user.email` já definidas no ambiente Git do usuário.
- Quando o envio remoto (`git push`) estiver sujeito a restrições de privacidade de e-mail do GitHub, assegurar que o autor do commit utilize o endereço _no-reply_ configurado na conta, nunca sobrescrevendo credenciais sem autorização explícita.

---

## 7. Edição do Arquivo `AGENTS.md`
- Este é o arquivo mais crítico de todo projeto
- Qualquer edição REQUER APROVAÇÃO do usuário. 
- Se aprovado, o agente deve agir apenas sob o escopo que o usuário permitir.
