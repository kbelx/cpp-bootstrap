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
    term::print_separator();
    term::print_line("Uso:");

#if defined(_WIN32)
    term::print_line("  cpp-bootstrap NomeDoProjeto");
#else
    term::print_line("  ./cpp-bootstrap NomeDoProjeto");
#endif

    term::print_line("");
    term::print_line("Dica: use apenas o nome real do projeto, sem < > ou espacos.");
    term::print_separator();
}

void print_help() {
    const term::Palette& p = term::palette();

    term::print_separator();
    term::print_line(std::string(p.cyan) + "cpp-bootstrap v" + std::string(term::kVersion) + std::string(p.reset));
    term::print_line("Gerador de projetos C++23 com estrutura padrao, CMake e VS Code.");
    term::print_line("");
    term::print_line("Uso:");
    term::print_line("  cpp-bootstrap <NomeDoProjeto> [opcoes]");
    term::print_line("");
    term::print_line("Opcoes:");
    term::print_line("  -q, --quiet      Suprime o rodape de autoria e links");
    term::print_line("  -h, --help       Exibe esta mensagem de ajuda");
    term::print_line("  -v, --version    Exibe a versao do programa");
    term::print_line("");
    term::print_line("Regras para NomeDoProjeto:");
    term::print_line("  * Apenas letras ASCII, numeros, '_' e '-'");
    term::print_line("  * Sem espacos e sem iniciar com '-'");
    term::print_separator();
}

void print_version() {
    term::print_line("cpp-bootstrap " + std::string(term::kVersion));
}

}  // namespace bootstrap
