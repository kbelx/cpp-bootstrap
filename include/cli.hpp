/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Declaracoes de opcoes da CLI, normalizacao de nome e exibicao de ajuda/versao.
*/

#ifndef CLI_HPP
#define CLI_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <string>
#include <string_view>

//=========================== [ TIPOS E OPCOES CLI ] ===========================

namespace CLI {

    inline constexpr std::string_view VERSAO {"1.0.0"};

    enum class Acao {
        CRIAR_PROJETO,
        EXIBIR_ERRO_TUTORIAL,
        EXIBIR_AJUDA,
        EXIBIR_VERSAO,
        FLAG_INVALIDA
    };

    struct Opcoes {
        Acao AcaoEscolhida { Acao::EXIBIR_ERRO_TUTORIAL };
        std::string NomeProjeto{};
        std::string FlagDesconhecida{};
        bool Silencioso{false};
    };

}  // namespace CLI

//============================= [ PARSER DA CLI ] ==============================

namespace CLI::Parser {

    // Remove espacos e aspas envolventes externas uma unica vez antes da validacao.
    [[nodiscard]] std::string limparNomeProjeto(std::string_view Bruto);

    // Analisa os argumentos de linha de comando preservando multi-tokens sem aspas.
    [[nodiscard]] Opcoes analisarArgs(int QtdArgs, const char* const* VetorArgs);

}  // namespace CLI::Parser

//=========================== [ TELAS DE EXIBICAO ] ============================

namespace CLI::Telas {

    // Exibe o tutorial rapido quando executado sem argumentos (exit 1).
    void exibirTutorial();

    // Exibe a ajuda completa de uso e opcoes (-h / --help).
    void exibirAjuda();

    // Exibe a versao do utilitario (-v / --version).
    void exibirVersao();

}  // namespace CLI::Telas

#endif
