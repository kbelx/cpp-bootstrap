/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Ponto de entrada da CLI cpp-bootstrap, orquestracao do fluxo e banner de sucesso.
*/

#include <filesystem>
#include <string>
#include <string_view>

#include "cli.hpp"
#include "generator.hpp"
#include "terminal.hpp"
#include "validator.hpp"


namespace {

// Exibe a mensagem final de sucesso
void print_success_banner(std::string_view project_name, bool quiet) {
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::separador();
    Terminal::Saida::imprimirLinha(
        "[OK] Projeto " +
        Terminal::Saida::colorirTexto(Terminal::Cores::VERDE, "'" + std::string(project_name) + "'") +
        " criado com sucesso!"
    );
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirColorido(Terminal::Cores::CIANO, "Proximos passos:");
    Terminal::Saida::imprimirLinha(
        "  1 - Entre na pasta:   " +
        Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, "cd " + std::string(project_name))
    );
    Terminal::Saida::imprimirLinha(
        "  2 - Abra no VS Code:  " +
        Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, "code .")
    );
    Terminal::Saida::imprimirLinha(
        "  3 - Configure CMake:  " +
        Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, "cmake -B build")
    );
    Terminal::Saida::imprimirLinha(
        "  4 - Compile:          " +
        Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, "cmake --build build")
    );
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirLinha(
        Terminal::Saida::colorirTexto(Terminal::Cores::CIANO, "Sistema detectado:") +
        " " +
        std::string(Terminal::Ambiente::detectarSO())
    );

    if (!quiet) {
        Terminal::Saida::separador();
        Terminal::Saida::imprimirLinha(
            Terminal::Saida::colorirTexto(Terminal::Cores::CIANO, "Autor:") +
            " Bruno Costa (" +
            Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, "aka kbelx") +
            ")"
        );

        constexpr std::string_view kGithubUrl   = "https://github.com/kbelx";
        constexpr std::string_view kLinkedinUrl = "https://www.linkedin.com/in/brunocosta-dev/";

        if (Terminal::Ambiente::deveUsarAnsi()) {
            auto osc8_link = [](std::string_view url) {
                return std::string(Terminal::Cores::ESC) + "]8;;" + std::string(url) +
                       std::string(Terminal::Cores::ESC) + "\\" +
                       Terminal::Saida::colorirTexto(Terminal::Cores::AZUL, url) +
                       std::string(Terminal::Cores::ESC) + "]8;;" +
                       std::string(Terminal::Cores::ESC) + "\\";
            };
            Terminal::Saida::imprimirLinha("  GitHub:   " + osc8_link(kGithubUrl));
            Terminal::Saida::imprimirLinha("  LinkedIn: " + osc8_link(kLinkedinUrl));
        }
        else {
            Terminal::Saida::imprimirLinha("  GitHub:   " + std::string(kGithubUrl));
            Terminal::Saida::imprimirLinha("  LinkedIn: " + std::string(kLinkedinUrl));
        }
    }

    Terminal::Saida::separador();
    Terminal::Saida::imprimirLinha("");
}

}  // namespace

int main(int argc, char* argv[]) {
    const bootstrap::CliOptions options = bootstrap::parse_cli(argc, argv);

    switch (options.action) {
        case bootstrap::CliAction::ShowTutorialError:
            bootstrap::print_tutorial();
            return 1;

        case bootstrap::CliAction::ShowHelp:
            bootstrap::print_help();
            return 0;

        case bootstrap::CliAction::ShowVersion:
            bootstrap::print_version();
            return 0;

        case bootstrap::CliAction::InvalidFlag:
            Terminal::Saida::separador();
            Terminal::Saida::escreveErro("[ERRO] Opcao desconhecida: '" + options.unknown_flag + "'");
            Terminal::Saida::escreveInfo("Use --help para ver as opcoes disponiveis.");
            Terminal::Saida::separador();
            return 2;

        case bootstrap::CliAction::CreateProject:
            break;
    }

    const std::filesystem::path caller_dir = std::filesystem::current_path();

    if (!Validadores::test_project_name(options.project_name, caller_dir)) {
        return 2;
    }

    std::string error_detail;
    if (!bootstrap::create_project(caller_dir, options.project_name, &error_detail)) {
        Terminal::Saida::imprimirLinha("");
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro("[ERRO] Falha ao criar o projeto '" + options.project_name + "'");
        Terminal::Saida::escreveAviso("Detalhe: " + error_detail);
        Terminal::Saida::separador();
        return 10;
    }

    print_success_banner(options.project_name, options.quiet);
    return 0;
}
