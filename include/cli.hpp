#pragma once

//============================ [ CLI / PARSE ] =================================

#include <string>
#include <string_view>

namespace bootstrap {

enum class CliAction {
    CreateProject,
    ShowTutorialError,
    ShowHelp,
    ShowVersion,
    InvalidFlag
};

struct CliOptions {
    CliAction action = CliAction::ShowTutorialError;
    std::string project_name;
    std::string unknown_flag;
    bool quiet = false;
};

// Remove espacos e aspas envolventes externas uma unica vez antes da validacao.
[[nodiscard]] std::string trim_project_name(std::string_view raw);

// Faz o parse dos argumentos de linha de comando preservando multi-tokens sem aspas.
[[nodiscard]] CliOptions parse_cli(int argc, const char* const* argv);

// Exibe o tutorial rapido quando executado sem argumentos (exit 1).
void print_tutorial();

// Exibe a ajuda completa de uso e opcoes (-h / --help).
void print_help();

// Exibe a versao do utilitario (-v / --version).
void print_version();

}  // namespace bootstrap
