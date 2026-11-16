#include "cli.hpp"
#include "generator.hpp"
#include "terminal.hpp"
#include "validator.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace {

//============================= [ MENSAGEM FINAL ] =============================

void print_success_banner(std::string_view project_name, bool quiet) {
    const bootstrap::term::Palette& p = bootstrap::term::palette();

    auto wrap = [](std::string_view color, std::string_view text, std::string_view reset) {
        return std::string(color) + std::string(text) + std::string(reset);
    };

    bootstrap::term::print_line("");
    bootstrap::term::print_separator();
    bootstrap::term::print_line(
        "[OK] Projeto " + wrap(p.green, "'" + std::string(project_name) + "'", p.reset) + " criado com sucesso!"
    );
    bootstrap::term::print_line("");
    bootstrap::term::print_line(wrap(p.cyan, "Proximos passos:", p.reset));
    bootstrap::term::print_line("  1 - Entre na pasta:   " + wrap(p.blue, "cd " + std::string(project_name), p.reset));
    bootstrap::term::print_line("  2 - Abra no VS Code:  " + wrap(p.blue, "code .", p.reset));
    bootstrap::term::print_line("  3 - Configure CMake:  " + wrap(p.blue, "cmake -B build", p.reset));
    bootstrap::term::print_line("  4 - Compile:          " + wrap(p.blue, "cmake --build build", p.reset));
    bootstrap::term::print_line("");
    bootstrap::term::print_line(
        wrap(p.cyan, "Sistema detectado:", p.reset) + " " + std::string(bootstrap::term::detect_os_label())
    );

    if (!quiet) {
        bootstrap::term::print_separator();
        bootstrap::term::print_line(
            wrap(p.cyan, "Autor:", p.reset) + " Bruno Costa (" + wrap(p.blue, "aka kbelx", p.reset) + ")"
        );

        constexpr std::string_view kGithubUrl   = "https://github.com/kbelx";
        constexpr std::string_view kLinkedinUrl = "https://www.linkedin.com/in/brunocosta-dev/";

        if (p.enabled) {
            auto osc8_link = [&](std::string_view url) {
                return std::string(p.esc) + "]8;;" + std::string(url) + std::string(p.esc) + "\\" +
                       wrap(p.blue, url, p.reset) +
                       std::string(p.esc) + "]8;;" + std::string(p.esc) + "\\";
            };
            bootstrap::term::print_line("  GitHub:   " + osc8_link(kGithubUrl));
            bootstrap::term::print_line("  LinkedIn: " + osc8_link(kLinkedinUrl));
        }
        else {
            bootstrap::term::print_line("  GitHub:   " + std::string(kGithubUrl));
            bootstrap::term::print_line("  LinkedIn: " + std::string(kLinkedinUrl));
        }
    }

    bootstrap::term::print_separator();
    bootstrap::term::print_line("");
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
            bootstrap::term::print_separator();
            bootstrap::term::write_error("[ERRO] Opcao desconhecida: '" + options.unknown_flag + "'");
            bootstrap::term::write_info("Use --help para ver as opcoes disponiveis.");
            bootstrap::term::print_separator();
            return 2;

        case bootstrap::CliAction::CreateProject:
            break;
    }

    const std::filesystem::path caller_dir = std::filesystem::current_path();

    if (!bootstrap::test_project_name(options.project_name, caller_dir)) {
        return 2;
    }

    std::string error_detail;
    if (!bootstrap::create_project(caller_dir, options.project_name, &error_detail)) {
        bootstrap::term::print_line("");
        bootstrap::term::print_separator();
        bootstrap::term::write_error("[ERRO] Falha ao criar o projeto '" + options.project_name + "'");
        bootstrap::term::write_warn("Detalhe: " + error_detail);
        bootstrap::term::print_separator();
        return 10;
    }

    print_success_banner(options.project_name, options.quiet);
    return 0;
}
