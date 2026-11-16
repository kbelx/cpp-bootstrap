# Documentação da Transição — `cpp-bootstrap`

Esta pasta reúne toda a documentação da migração do `cpp-bootstrap`: do protótipo em **PowerShell (`v0.9.0-beta`)** para a primeira versão oficial em **C++23 (`v1.0.0`)**.

> [!NOTE]
> O [README.md](../README.md) da raiz é o manual de **uso** do binário. Os documentos daqui explicam **como e por que** o projeto chegou ao estado atual e **como mantê-lo**.

## Índice

| Nº | Documento | Conteúdo |
| :--- | :--- | :--- |
| 01 | [Visão geral da transição](01_visao_geral_transicao.md) | Motivação, linha do tempo, versionamento e estado final |
| 02 | [Mapa PowerShell → C++23](02_mapa_powershell_para_cpp.md) | Onde cada seção e função do script foi parar no C++ |
| 03 | [Arquitetura C++23](03_arquitetura_cpp23.md) | Módulos, fluxo de execução, códigos de saída e regras de validação |
| 04 | [Build e testes](04_build_e_testes.md) | Targets CMake, opções, CTest e os 20 cenários da suíte |
| 05 | [CI/CD no GitHub Actions](05_ci_cd_github_actions.md) | Workflows `ci.yml` e `release.yml` e como publicar uma versão |
| 06 | [Guia de manutenção](06_guia_de_manutencao.md) | Receitas para adicionar regras, templates, flags e lançar versões |
| 07 | [Registro de decisões](07_registro_de_decisoes.md) | Decisões tomadas durante a transição e suas justificativas |
| 08 | [Histórico da fase beta](08_historico_fase_beta.md) | Bugs encontrados e corrigidos na versão PowerShell |

## Referências rápidas

- **Versão legada (PowerShell):** tag `v0.9.0-beta` e branch `legacy/powershell-beta`.
- **Commit da migração:** `36834ba` — `feat: migrar cpp-bootstrap para C++23 (v1.0.0) com CTest e GitHub Actions`.
- **Checklist original do feedback:** [docs_TRANSICAO/feedback_projeto_18102026.md](docs_TRANSICAO/feedback_projeto_18102026.md).
