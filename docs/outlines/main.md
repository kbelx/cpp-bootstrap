# Outline — `main`

- **Implementação:** [main.cpp](../../src/main.cpp)
- **Propósito:** Ponto de entrada do executável `cpp-bootstrap` e exibição do banner final.

## `App::Banner`
- `URL_GITHUB`, `URL_LINKEDIN` e `formatarLinkOsc8(Url)`.
- `exibirProximosPassos(NomeProjeto)` e `exibirRodapeAutor()`.
- `exibirBannerSucesso(NomeProjeto, Silencioso)` — confirma criação, mostra SO e omite rodapé se `Silencioso`.

## `App::Fluxo`
- `exibirErroFlagInvalida(FlagDesconhecida)` e `exibirErroCriacao(NomeProjeto, DetalheErro)`.

## `main(QtdArgs, VetorArgs)`
- Analisa argumentos via `CLI::Parser::analisarArgs()` e despacha:
  - `EXIBIR_ERRO_TUTORIAL` → `CLI::Telas::exibirTutorial()` (`1`).
  - `EXIBIR_AJUDA` / `EXIBIR_VERSAO` → `exibirAjuda()` / `exibirVersao()` (`0`).
  - `FLAG_INVALIDA` → `App::Fluxo::exibirErroFlagInvalida()` (`2`).
  - `CRIAR_PROJETO` → `Validadores::validarNomeProjeto()` (`2` se inválido) → `Gerador::Projeto::criarProjeto()` (`10` em erro de I/O) → `exibirBannerSucesso()` (`0`).
