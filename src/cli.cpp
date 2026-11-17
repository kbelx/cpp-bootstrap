/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Implementacao do parser de linha de comando, limpeza do nome e telas de tutorial/ajuda.
*/

//============================ [ CABECALHOS PADRAO ] ===========================

#include <vector>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "cli.hpp"
#include "terminal.hpp"

//======================== [ AUXILIARES DE TEXTO / FLAGS ] =====================

static constexpr std::string_view CORPO_AJUDA{
R"ajuda(Gerador de projetos C++23 com estrutura padrao, CMake e VS Code.

Uso:
    cpp-bootstrap <NomeDoProjeto> [opcoes]

Opcoes:
    -q, --quiet      Suprime o rodape de autoria e links
    -h, --help       Exibe esta mensagem de ajuda
    -v, --version    Exibe a versao do programa

Regras para NomeDoProjeto:
    * Apenas letras ASCII, numeros, '_' e '-'
    * Sem espacos e sem iniciar com '-')ajuda"
};

namespace CLI::Auxiliar {

    [[nodiscard]] constexpr bool ehEspacoAscii(char Caractere) noexcept {
        return Caractere == ' '  || Caractere == '\t' || Caractere == '\n' ||
               Caractere == '\r' || Caractere == '\v' || Caractere == '\f';
    }

    [[nodiscard]] std::string_view removerEspacosExternos(std::string_view Texto) noexcept {
        while (!Texto.empty() && ehEspacoAscii(Texto.front())) {
            Texto.remove_prefix(1);
        }
        while (!Texto.empty() && ehEspacoAscii(Texto.back())) {
            Texto.remove_suffix(1);
        }
        return Texto;
    }

    [[nodiscard]] std::string_view removerCaractereExterno(std::string_view Texto, char Alvo) noexcept {
        while (!Texto.empty() && Texto.front() == Alvo) {
            Texto.remove_prefix(1);
        }
        while (!Texto.empty() && Texto.back() == Alvo) {
            Texto.remove_suffix(1);
        }
        return Texto;
    }

    [[nodiscard]] std::string juntarPartesNome(const std::vector<std::string_view>& PartesNome) {
        std::string Unido{};
        for (std::size_t Indice{0}; Indice < PartesNome.size(); ++Indice) {
            if (Indice > 0) {
                Unido.push_back(' ');
            }
            Unido.append(PartesNome[Indice]);
        }
        return Unido;
    }

    [[nodiscard]] bool ehFlagAjuda(std::string_view Token) noexcept {
        return Token == "-h" || Token == "--help" || Token == "/?";
    }

    [[nodiscard]] bool ehFlagVersao(std::string_view Token) noexcept {
        return Token == "-v" || Token == "--version";
    }

    [[nodiscard]] bool ehFlagSilenciosa(std::string_view Token) noexcept {
        return Token == "-q" || Token == "--quiet" || Token == "-Quiet" || Token == "-quiet";
    }

    // Flags longas desconhecidas (--foo) ou curtas apos um nome (-x) sao rejeitadas como opcoes invalidas.
    [[nodiscard]] bool ehFlagInvalida(std::string_view Token, bool PossuiPartesNome) noexcept {
        return Token.starts_with("--") ||
               (Token.starts_with('-') && (PossuiPartesNome || Token.size() == 2));
    }

}  // namespace CLI::Auxiliar

//============================= [ PARSER DA CLI ] ==============================

namespace CLI::Parser {

    std::string limparNomeProjeto(std::string_view Bruto) {
        std::string_view Texto{Auxiliar::removerEspacosExternos(Bruto)};
        Texto = Auxiliar::removerCaractereExterno(Texto, '"');
        Texto = Auxiliar::removerCaractereExterno(Texto, '\'');
        Texto = Auxiliar::removerEspacosExternos(Texto);
        return std::string{Texto};
    }

    Opcoes analisarArgs(int QtdArgs, const char* const* VetorArgs) {
        Opcoes OpcoesCli{};
        if (QtdArgs <= 1 || VetorArgs == nullptr) {
            OpcoesCli.AcaoEscolhida = Acao::EXIBIR_ERRO_TUTORIAL;
            return OpcoesCli;
        }

        std::vector<std::string_view> PartesNome{};

        for (int Indice{1}; Indice < QtdArgs; ++Indice) {
            const char* ARG_BRUTO{VetorArgs[Indice]};
            if (ARG_BRUTO == nullptr) {
                continue;
            }
            const std::string_view TOKEN{ARG_BRUTO};

            if (Auxiliar::ehFlagAjuda(TOKEN)) {
                OpcoesCli.AcaoEscolhida = Acao::EXIBIR_AJUDA;
                return OpcoesCli;
            }
            if (Auxiliar::ehFlagVersao(TOKEN)) {
                OpcoesCli.AcaoEscolhida = Acao::EXIBIR_VERSAO;
                return OpcoesCli;
            }
            if (Auxiliar::ehFlagSilenciosa(TOKEN)) {
                OpcoesCli.Silencioso = true;
                continue;
            }
            if (Auxiliar::ehFlagInvalida(TOKEN, !PartesNome.empty())) {
                OpcoesCli.AcaoEscolhida    = Acao::FLAG_INVALIDA;
                OpcoesCli.FlagDesconhecida = std::string{TOKEN};
                return OpcoesCli;
            }

            PartesNome.push_back(TOKEN);
        }

        OpcoesCli.AcaoEscolhida = Acao::CRIAR_PROJETO;
        OpcoesCli.NomeProjeto   = limparNomeProjeto(Auxiliar::juntarPartesNome(PartesNome));
        return OpcoesCli;
    }

}  // namespace CLI::Parser

//=========================== [ TELAS DE EXIBICAO ] ============================

namespace CLI::Telas {

    void exibirTutorial() {
        Terminal::Saida::separador();
        Terminal::Saida::imprimirLinha("Uso:");

    // se o usuario estiver usando Windows...
    #if defined(_WIN32)
        Terminal::Saida::imprimirLinha("  cpp-bootstrap NomeDoProjeto");

    // se o usuario estiver usando linux/macOs...
    #else
        Terminal::Saida::imprimirLinha("  ./cpp-bootstrap NomeDoProjeto");
    #endif

        Terminal::Saida::imprimirLinha("");
        Terminal::Saida::imprimirLinha("Dica: use apenas o nome real do projeto, sem < > ou espacos.");
        Terminal::Saida::separador();
    }

    void exibirAjuda() {
        Terminal::Saida::separador();
        Terminal::Saida::imprimirColorido(
            Terminal::Cores::CIANO,
            "cpp-bootstrap v" + std::string{CLI::VERSAO}
        );
        Terminal::Saida::imprimirLinha(CORPO_AJUDA);
        Terminal::Saida::separador();
    }

    void exibirVersao() {
        Terminal::Saida::imprimirLinha("cpp-bootstrap " + std::string{CLI::VERSAO});
    }

}  // namespace CLI::Telas
