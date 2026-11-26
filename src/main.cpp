/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Ponto de entrada da CLI cpp-bootstrap, orquestracao do fluxo e banner de sucesso.
*/

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string>
#include <string_view>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "cli.hpp"
#include "cores.hpp"
#include "generator.hpp"
#include "terminal.hpp"
#include "validator.hpp"

//============================ [ BANNER DE SUCESSO ] ===========================

namespace App::Banner {

    inline constexpr std::string_view URL_GITHUB   {"https://github.com/kbelx"};
    inline constexpr std::string_view URL_LINKEDIN {"https://www.linkedin.com/in/brunocosta-dev/"};

    [[nodiscard]] std::string formatarLinkOsc8(std::string_view Url) {
        return std::string{Cores::ESC} + "]8;;" + std::string{Url} +
               std::string{Cores::ESC} + "\\" +
               Terminal::Saida::colorirTexto(Cores::AZUL, Url) +
               std::string{Cores::ESC} + "]8;;" +
               std::string{Cores::ESC} + "\\";
    }

    void exibirProximosPassos(std::string_view NomeProjeto) {
        Terminal::Saida::imprimirColorido(Cores::CIANO, "Proximos passos:");
        Terminal::Saida::imprimirLinha("  1 - Entre na pasta:   " + Terminal::Saida::colorirTexto(Cores::AZUL, "cd " + std::string{NomeProjeto}));
        Terminal::Saida::imprimirLinha("  2 - Abra no VS Code:  " + Terminal::Saida::colorirTexto(Cores::AZUL, "code ."));
        Terminal::Saida::imprimirLinha("  3 - Configure CMake:  " + Terminal::Saida::colorirTexto(Cores::AZUL, "cmake -B build"));
        Terminal::Saida::imprimirLinha("  4 - Compile:          " + Terminal::Saida::colorirTexto(Cores::AZUL, "cmake --build build"));
    }

    void exibirRodapeAutor() {
        Terminal::Saida::separador();

        Terminal::Saida::imprimirLinha(Terminal::Saida::colorirTexto(Cores::CIANO, "Autor:") +
            " Bruno Costa (" +
            Terminal::Saida::colorirTexto(Cores::AZUL, "aka kbelx") +
            ")"
        );

        if (Terminal::Ambiente::deveUsarAnsi()) {
            Terminal::Saida::imprimirLinha("  GitHub:   " + formatarLinkOsc8(URL_GITHUB));
            Terminal::Saida::imprimirLinha("  LinkedIn: " + formatarLinkOsc8(URL_LINKEDIN));
        }
        else {
            Terminal::Saida::imprimirLinha("  GitHub:   " + std::string{URL_GITHUB});
            Terminal::Saida::imprimirLinha("  LinkedIn: " + std::string{URL_LINKEDIN});
        }
    }

    void exibirBannerSucesso(std::string_view NomeProjeto, bool Silencioso) {
        Terminal::Saida::imprimirLinha("");
        Terminal::Saida::separador();
        Terminal::Saida::imprimirLinha(
            "[OK] Projeto " +
            Terminal::Saida::colorirTexto(Cores::VERDE, "'" + std::string{NomeProjeto} + "'") +
            " criado com sucesso!"
        );
        Terminal::Saida::imprimirLinha("");

        exibirProximosPassos(NomeProjeto);

        Terminal::Saida::imprimirLinha("");
        Terminal::Saida::imprimirLinha(
            Terminal::Saida::colorirTexto(Cores::CIANO, "Sistema detectado:") +
            " " +
            std::string{Terminal::Ambiente::detectarSO()}
        );

        if (!Silencioso) {
            exibirRodapeAutor();
        }

        Terminal::Saida::separador();
        Terminal::Saida::imprimirLinha("");
    }

}  // namespace App::Banner

//========================== [ DIAGNOSTICOS DE FLUXO ] =========================

namespace App::Fluxo {

    void exibirErroFlagInvalida(std::string_view FlagDesconhecida) {
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro(
            "[ERRO] Opcao desconhecida: '" + std::string{FlagDesconhecida} + "'"
        );
        Terminal::Saida::escreveInfo("Use --help para ver as opcoes disponiveis.");
        Terminal::Saida::separador();
    }

    void exibirErroCriacao(std::string_view NomeProjeto, std::string_view DetalheErro) {
        Terminal::Saida::imprimirLinha("");
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro(
            "[ERRO] Falha ao criar o projeto '" + std::string{NomeProjeto} + "'"
        );
        Terminal::Saida::escreveAviso("Detalhe: " + std::string{DetalheErro});
        Terminal::Saida::separador();
    }

}  // namespace App::Fluxo

//============================ [ PONTO DE ENTRADA ] ============================

int main(int QtdArgs, char* VetorArgs[]) {

    const CLI::Opcoes OPCOES_CLI { CLI::Parser::analisarArgs(QtdArgs, VetorArgs) };

    switch (OPCOES_CLI.AcaoEscolhida) {
        case CLI::Acao::EXIBIR_ERRO_TUTORIAL:
            CLI::Telas::exibirTutorial();
            return 1;

        case CLI::Acao::EXIBIR_AJUDA:
            CLI::Telas::exibirAjuda();
            return 0;

        case CLI::Acao::EXIBIR_VERSAO:
            CLI::Telas::exibirVersao();
            return 0;

        case CLI::Acao::FLAG_INVALIDA:
            App::Fluxo::exibirErroFlagInvalida(OPCOES_CLI.FlagDesconhecida);
            return 2;

        case CLI::Acao::CRIAR_PROJETO:
            break;
    }

    const std::filesystem::path DIR_CHAMADOR{std::filesystem::current_path()};

    if (!Validadores::validarNomeProjeto(OPCOES_CLI.NomeProjeto, DIR_CHAMADOR)) {
        return 2;
    }

    std::string DetalheErro{};
    if (!Gerador::Projeto::criarProjeto(DIR_CHAMADOR, OPCOES_CLI.NomeProjeto, &DetalheErro)) {
        App::Fluxo::exibirErroCriacao(OPCOES_CLI.NomeProjeto, DetalheErro);
        return 10;
    }

    App::Banner::exibirBannerSucesso(OPCOES_CLI.NomeProjeto, OPCOES_CLI.Silencioso);
    return 0;
}
