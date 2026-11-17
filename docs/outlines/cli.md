# Outline — `cli`

- **Arquivos:** [cli.hpp](../../include/cli.hpp) | [cli.cpp](../../src/cli.cpp)
- **Propósito:** Parse da CLI, normalização do nome e telas de ajuda/versão.

## `CLI` (`cli.hpp`)
- `VERSAO`: `"1.0.0"` (`inline constexpr std::string_view`).
- `enum class Acao`: `CRIAR_PROJETO`, `EXIBIR_ERRO_TUTORIAL`, `EXIBIR_AJUDA`, `EXIBIR_VERSAO`, `FLAG_INVALIDA`.
- `struct Opcoes`: `AcaoEscolhida`, `NomeProjeto`, `FlagDesconhecida`, `Silencioso`.

## `CLI::Parser` (`cli.hpp` / `cli.cpp`)
- `limparNomeProjeto(Bruto)` — remove espaços e aspas envolventes (`"`, `'`).
- `analisarArgs(QtdArgs, VetorArgs)` — interpreta `-h`/`--help`/`/?`, `-v`/`--version`, `-q`/`--quiet`/`-Quiet`/`-quiet`, rejeita flags desconhecidas (`FLAG_INVALIDA`) e une tokens posicionais.

## `CLI::Telas` (`cli.hpp` / `cli.cpp`)
- `exibirTutorial()` — uso rápido (sem argumentos).
- `exibirAjuda()` — imprime cabeçalho colorido + `CORPO_AJUDA`.
- `exibirVersao()` — imprime `cpp-bootstrap <VERSAO>`.

## `CLI::Auxiliar` (`cli.cpp`)
- `ehEspacoAscii`, `removerEspacosExternos`, `removerCaractereExterno`, `juntarPartesNome`, `ehFlagAjuda`, `ehFlagVersao`, `ehFlagSilenciosa`, `ehFlagInvalida`.
