/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Validacao de nomes em 5 regras (vazio, ponto final, reservados, CMake e colisao).
*/

#include <algorithm>
#include <array>
#include <system_error>
#include <vector>

#include "validator.hpp"
#include "terminal.hpp"


namespace Validadores {

namespace {

[[nodiscard]] constexpr bool is_ascii_alpha(char ch) noexcept {
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

[[nodiscard]] constexpr bool is_ascii_digit(char ch) noexcept {
    return ch >= '0' && ch <= '9';
}

[[nodiscard]] constexpr bool is_allowed_start(char ch) noexcept {
    return is_ascii_alpha(ch) || is_ascii_digit(ch) || ch == '_';
}

[[nodiscard]] constexpr bool is_allowed_body(char ch) noexcept {
    return is_allowed_start(ch) || ch == '-';
}

[[nodiscard]] constexpr char to_ascii_upper(char ch) noexcept {
    if (ch >= 'a' && ch <= 'z') {
        return static_cast<char>(ch - ('a' - 'A'));
    }
    return ch;
}

[[nodiscard]] bool is_all_whitespace(std::string_view sv) noexcept {
    for (const char ch : sv) {
        if (ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' && ch != '\v' && ch != '\f') {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool is_windows_reserved_name(std::string_view name) {
    static constexpr std::array<std::string_view, 22> kReserved = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    const std::size_t dot_pos = name.find('.');
    const std::string_view prefix = (dot_pos == std::string_view::npos) ? name : name.substr(0, dot_pos);

    std::string upper;
    upper.reserve(prefix.size());
    for (const char ch : prefix) {
        upper.push_back(to_ascii_upper(ch));
    }

    return std::find(kReserved.begin(), kReserved.end(), upper) != kReserved.end();
}

[[nodiscard]] std::string format_invalid_chars(std::string_view name) {
    std::vector<std::string> seen;

    for (const char ch : name) {
        if (is_allowed_body(ch)) {
            continue;
        }

        std::string token;
        if (ch == ' ') {
            token = "<espaco>";
        }
        else if (static_cast<unsigned char>(ch) < 0x20 || static_cast<unsigned char>(ch) == 0x7F) {
            token = "<ctrl>";
        }
        else {
            token.push_back(ch);
        }

        if (std::find(seen.begin(), seen.end(), token) == seen.end()) {
            seen.push_back(std::move(token));
        }
    }

    if (seen.empty()) {
        return "inicio com '-'";
    }

    std::string joined;
    for (std::size_t i = 0; i < seen.size(); ++i) {
        if (i > 0) {
            joined.push_back(' ');
        }
        joined.append(seen[i]);
    }
    return joined;
}

}  // namespace

bool test_project_name(
    std::string_view name,
    const std::filesystem::path& base_dir,
    bool silent
) {
    // 1 - Nome vazio ou apenas espacos
    if (name.empty() || is_all_whitespace(name)) {
        if (!silent) {
            Terminal::Saida::escreveErro("[ERRO] Nome do projeto vazio.");
            Terminal::Saida::escreveInfo("[INFO] Exemplo (Windows): cpp-bootstrap MeuProjeto");
            Terminal::Saida::escreveInfo("[INFO] Exemplo (Linux/macOS): ./cpp-bootstrap MeuProjeto");
        }
        return false;
    }

    // 2 - Nome terminando com ponto (invalido no Windows e bloqueado para portabilidade Git)
    if (name.back() == '.') {
        if (!silent) {
            Terminal::Saida::separador();
            Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string(name) + "'");
            Terminal::Saida::escreveAviso("Nomes de pasta nao podem terminar com '.' ou espaco.");
            Terminal::Saida::separador();
        }
        return false;
    }

    // 3 - Nomes de dispositivos reservados do Windows (bloqueados em todos os SOs para portabilidade)
    if (is_windows_reserved_name(name)) {
        if (!silent) {
            Terminal::Saida::separador();
            Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string(name) + "'");
            Terminal::Saida::escreveAviso("Esse e um nome reservado do Windows.");
            Terminal::Saida::escreveInfo("Escolha outro (ex: '" + std::string(name) + "_proj').");
            Terminal::Saida::separador();
        }
        return false;
    }

    // 4 - Regra uniforme para CMake e filesystems multiplataforma:
    //     apenas letras ASCII, digitos, '_' e '-', sem iniciar com '-'
    bool valid_pattern = is_allowed_start(name.front());
    if (valid_pattern) {
        for (const char ch : name) {
            if (!is_allowed_body(ch)) {
                valid_pattern = false;
                break;
            }
        }
    }

    if (!valid_pattern) {
        if (!silent) {
            const std::string bad_display = format_invalid_chars(name);
            Terminal::Saida::separador();
            Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string(name) + "'");
            Terminal::Saida::escreveAviso("Caracteres proibidos encontrados: " + bad_display);
            Terminal::Saida::escreveInfo("Use apenas letras, numeros, '_' e '-' (sem espacos e sem iniciar com '-').");
            Terminal::Saida::separador();
        }
        return false;
    }

    // 5 - Prevencao contra sobrescrita de pasta ou arquivo ja existente
    std::error_code ec;
    const std::filesystem::path candidate_path = base_dir / std::filesystem::path(name);
    if (std::filesystem::exists(candidate_path, ec)) {
        if (!silent) {
            Terminal::Saida::separador();
            Terminal::Saida::escreveErro("[ERRO] Ja existe uma pasta ou arquivo com o nome: '" + std::string(name) + "'");
            Terminal::Saida::escreveInfo("Sugestoes:");
            Terminal::Saida::escreveInfo("  * Use outro nome: cpp-bootstrap " + std::string(name) + "_2");
            Terminal::Saida::escreveInfo("  * Ou renomeie/apague o item existente.");
            Terminal::Saida::separador();
        }
        return false;
    }

    return true;
}

}  // namespace bootstrap
