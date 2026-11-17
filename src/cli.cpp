/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Implementacao do parser de linha de comando, trim do nome e telas de tutorial/ajuda.
*/

#include "cli.hpp"
#include "terminal.hpp"

#include <vector>

namespace bootstrap {

namespace {

[[nodiscard]] constexpr bool is_ascii_space(char ch) noexcept {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f';
}

[[nodiscard]] std::string_view strip_outer_spaces(std::string_view sv) noexcept {
    while (!sv.empty() && is_ascii_space(sv.front())) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && is_ascii_space(sv.back())) {
        sv.remove_suffix(1);
    }
    return sv;
}

[[nodiscard]] std::string_view strip_outer_char(std::string_view sv, char target) noexcept {
    while (!sv.empty() && sv.front() == target) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && sv.back() == target) {
        sv.remove_suffix(1);
    }
    return sv;
}

[[nodiscard]] bool is_help_flag(std::string_view token) noexcept {
    return token == "-h" || token == "--help" || token == "/?";
}

[[nodiscard]] bool is_version_flag(std::string_view token) noexcept {
    return token == "-v" || token == "--version";
}

[[nodiscard]] bool is_quiet_flag(std::string_view token) noexcept {
    return token == "-q" || token == "--quiet" || token == "-Quiet" || token == "-quiet";
}

}  // namespace

std::string trim_project_name(std::string_view raw) {
    std::string_view sv = strip_outer_spaces(raw);
    sv = strip_outer_char(sv, '"');
    sv = strip_outer_char(sv, '\'');
    sv = strip_outer_spaces(sv);
    return std::string(sv);
}

CliOptions parse_cli(int argc, const char* const* argv) {
    CliOptions options{};
    if (argc <= 1 || argv == nullptr) {
        options.action = CliAction::ShowTutorialError;
        return options;
    }

    std::vector<std::string_view> name_parts;

    for (int i = 1; i < argc; ++i) {
        const char* raw_arg = argv[i];
        if (raw_arg == nullptr) {
            continue;
        }
        const std::string_view token(raw_arg);

        if (is_help_flag(token)) {
            options.action = CliAction::ShowHelp;
            return options;
        }
        if (is_version_flag(token)) {
            options.action = CliAction::ShowVersion;
            return options;
        }
        if (is_quiet_flag(token)) {
            options.quiet = true;
            continue;
        }

        // Flags longas desconhecidas (--foo) ou curtas apos um nome (-x) sao rejeitadas como opcoes invalidas.
        if (token.starts_with("--") ||
            (token.starts_with('-') && (!name_parts.empty() || token.size() == 2))) {
            options.action = CliAction::InvalidFlag;
            options.unknown_flag = std::string(token);
            return options;
        }

        name_parts.push_back(token);
    }

    std::string joined;
    for (std::size_t idx = 0; idx < name_parts.size(); ++idx) {
        if (idx > 0) {
            joined.push_back(' ');
        }
        joined.append(name_parts[idx]);
    }

    options.action = CliAction::CreateProject;
    options.project_name = trim_project_name(joined);
    return options;
}

//============================== [ TUTORIAL ] ==================================

void print_tutorial() {
    Terminal::Saida::separador();
    Terminal::Saida::imprimirLinha("Uso:");

#if defined(_WIN32)
    Terminal::Saida::imprimirLinha("  cpp-bootstrap NomeDoProjeto");
#else
    Terminal::Saida::imprimirLinha("  ./cpp-bootstrap NomeDoProjeto");
#endif

    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirLinha("Dica: use apenas o nome real do projeto, sem < > ou espacos.");
    Terminal::Saida::separador();
}

void print_help() {
    Terminal::Saida::separador();
    Terminal::Saida::imprimirColorido(
        Terminal::Cores::CIANO,
        "cpp-bootstrap v" + std::string(Terminal::Ambiente::VERSAO)
    );
    Terminal::Saida::imprimirLinha("Gerador de projetos C++23 com estrutura padrao, CMake e VS Code.");
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirLinha("Uso:");
    Terminal::Saida::imprimirLinha("  cpp-bootstrap <NomeDoProjeto> [opcoes]");
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirLinha("Opcoes:");
    Terminal::Saida::imprimirLinha("  -q, --quiet      Suprime o rodape de autoria e links");
    Terminal::Saida::imprimirLinha("  -h, --help       Exibe esta mensagem de ajuda");
    Terminal::Saida::imprimirLinha("  -v, --version    Exibe a versao do programa");
    Terminal::Saida::imprimirLinha("");
    Terminal::Saida::imprimirLinha("Regras para NomeDoProjeto:");
    Terminal::Saida::imprimirLinha("  * Apenas letras ASCII, numeros, '_' e '-'");
    Terminal::Saida::imprimirLinha("  * Sem espacos e sem iniciar com '-'");
    Terminal::Saida::separador();
}

void print_version() {
    Terminal::Saida::imprimirLinha("cpp-bootstrap " + std::string(Terminal::Ambiente::VERSAO));
}

}  // namespace bootstrap
