/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Deteccao de SO, suporte ANSI/VT e utilitarios de saida formatada no terminal.

*/

#pragma once

//============================ [ DETECCAO DE AMBIENTE ] ========================

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#if defined(__has_include)
#  if __has_include(<print>)
#    include <print>
#  endif
#endif

#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <cstdio>
#  include <io.h>
#  include <windows.h>
#  ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#    define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#  endif
#else
#  include <unistd.h>
#endif

namespace bootstrap::term {

inline constexpr std::string_view kVersion = "1.0.0";

// Retorna o nome do sistema operacional detectado em tempo de compilacao.
[[nodiscard]] constexpr std::string_view detect_os_label() noexcept {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Linux";
#endif
}

// Verifica se a variavel de ambiente NO_COLOR esta definida com valor nao vazio.
[[nodiscard]] inline bool has_no_color_env() noexcept {
#if defined(_WIN32)
    char buffer[4]{};
    const DWORD len = GetEnvironmentVariableA("NO_COLOR", buffer, static_cast<DWORD>(sizeof(buffer)));
    return len > 0;
#else
    const char* val = std::getenv("NO_COLOR");
    return val != nullptr && val[0] != '\0';
#endif
}

// Detecta se stdout e um terminal interativo com suporte a sequencias ANSI.
[[nodiscard]] inline bool should_use_ansi() noexcept {
    if (has_no_color_env()) {
        return false;
    }

#if defined(_WIN32)
    if (_isatty(_fileno(stdout)) == 0) {
        return false;
    }
    HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (stdout_handle == INVALID_HANDLE_VALUE || stdout_handle == nullptr) {
        return false;
    }
    DWORD mode = 0;
    if (GetConsoleMode(stdout_handle, &mode) == 0) {
        return false;
    }
    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        if (SetConsoleMode(stdout_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
            return false;
        }
    }
    return true;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

//============================ [ PALETA / FUNCOES ] ============================

struct Palette {
    bool enabled = false;
    std::string_view esc;
    std::string_view reset;
    std::string_view green;
    std::string_view cyan;
    std::string_view blue;
    std::string_view gray;
    std::string_view yellow;
    std::string_view red;
    std::string_view bold;
};

// Retorna a paleta ANSI ativa quando o terminal suporta VT e nao esta redirecionado.
[[nodiscard]] inline const Palette& palette() noexcept {
    static const Palette cached = []() {
        Palette p{};
        p.enabled = should_use_ansi();
        if (p.enabled) {
            p.esc    = "\x1b";
            p.reset  = "\x1b[0m";
            p.green  = "\x1b[32m";
            p.cyan   = "\x1b[36m";
            p.blue   = "\x1b[34m";
            p.gray   = "\x1b[90m";
            p.yellow = "\x1b[33m";
            p.red    = "\x1b[31m";
            p.bold   = "\x1b[1m";
        }
        return p;
    }();
    return cached;
}

// Imprime uma linha em stdout usando std::println quando disponivel no toolchain.
inline void print_line(std::string_view text = "") {
#if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L)
    std::println("{}", text);
#else
    std::cout << text << '\n';
#endif
}

inline void print_separator() {
    const Palette& p = palette();
    std::string line;
    line.reserve(p.gray.size() + 52 + p.reset.size());
    line.append(p.gray);
    line.append("----------------------------------------------------");
    line.append(p.reset);
    print_line(line);
}

inline void write_colored(std::string_view color, std::string_view message) {
    const Palette& p = palette();
    std::string line;
    line.reserve(color.size() + message.size() + p.reset.size());
    line.append(color);
    line.append(message);
    line.append(p.reset);
    print_line(line);
}

inline void write_info(std::string_view message) {
    write_colored(palette().cyan, message);
}

inline void write_warn(std::string_view message) {
    write_colored(palette().yellow, message);
}

inline void write_error(std::string_view message) {
    write_colored(palette().red, message);
}

}  // namespace bootstrap::term
