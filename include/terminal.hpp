/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Deteccao de SO, suporte ANSI/VT e utilitarios de saida formatada no terminal.
*/

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

// Inclui <print> (C++23) quando disponivel na biblioteca padrao do compilador
#if __has_include(<print>)
    #include <print>
#endif

//======================== [ CABECALHOS DE PLATAFORMA ] ========================

#if defined(_WIN32)

    // Desativa macros min/max legadas da Win32 que conflitam com <algorithm>
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    // Exclui APIs raramente usadas de <windows.h> para acelerar a compilacao
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>  // GetStdHandle, GetConsoleMode, SetConsoleMode

    // Fallback para SDKs ou toolchains MinGW antigos sem suporte nativo a flag VT
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <unistd.h>   // isatty, STDOUT_FILENO (POSIX: Linux / macOS)
#endif

//============================ [ PALETA DE CORES ] =============================

namespace Terminal::Cores {

    inline constexpr std::string_view ESC      = "\x1b";
    inline constexpr std::string_view RESET    = "\x1b[0m";
    inline constexpr std::string_view VERDE    = "\x1b[32m";
    inline constexpr std::string_view CIANO    = "\x1b[36m";
    inline constexpr std::string_view AZUL     = "\x1b[34m";
    inline constexpr std::string_view CINZA    = "\x1b[90m";
    inline constexpr std::string_view AMARELO  = "\x1b[33m";
    inline constexpr std::string_view VERMELHO = "\x1b[31m";
    inline constexpr std::string_view NEGRITO  = "\x1b[1m";

}  // namespace Terminal::Cores

//======================== [ DETECCAO DE AMBIENTE / SO ] =======================

namespace Terminal::Ambiente {

    inline constexpr std::string_view VERSAO = "1.0.0";

    // Retorna o nome do SO detectado em tempo de compilacao.
    [[nodiscard]] constexpr std::string_view detectarSO() noexcept {

    // Se for Windows...
    #if defined(_WIN32)
        return "Windows";

    // Se for Apple ( macOs )...
    #elif defined(__APPLE__)
        return "macOS";
    
    // Se for Linux...
    #else
        return "Linux";
    #endif
    }

    // Verifica se a variavel de ambiente NO_COLOR esta definida com valor nao vazio.
    [[nodiscard]] inline bool possuiVarSemCor() noexcept {
    #if defined(_WIN32)
        char Buffer[4]{};
        const DWORD TAM = GetEnvironmentVariableA("NO_COLOR", Buffer, static_cast<DWORD>(sizeof(Buffer)));
        return TAM > 0;
    #else
        const char* VALOR = std::getenv("NO_COLOR");
        return VALOR != nullptr && VALOR[0] != '\0';
    #endif
    }

    // Verifica se STDOUT é um terminal interativo e habilita o processamento VT quando necessario.
    [[nodiscard]] inline bool configTerminalAnsi() noexcept {
    #if defined(_WIN32)
        HANDLE ManipSaida {GetStdHandle(STD_OUTPUT_HANDLE)};
        if (ManipSaida == INVALID_HANDLE_VALUE || ManipSaida == nullptr) {
            return false;
        }

        DWORD Modo = 0;
        if (GetConsoleMode(ManipSaida, &Modo) == 0) {
            return false;
        }

        if ((Modo & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
            if (SetConsoleMode(ManipSaida, Modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
                return false;
            }
        }
        return true;
    #else
        return isatty(STDOUT_FILENO) != 0;
    #endif
    }

    // Detecta se stdout e um terminal interativo com suporte a sequencias ANSI (com cache estatico).
    [[nodiscard]] inline bool deveUsarAnsi() noexcept {
        static const bool SUPORTA_ANSI = !possuiVarSemCor() && configTerminalAnsi();
        return SUPORTA_ANSI;
    }

}  // namespace Terminal::Ambiente

//=========================== [ FUNCOES DE SAIDA ] =============================

namespace Terminal::Saida {

    // Imprime uma linha em stdout usando std::println quando disponivel no toolchain.
    inline void imprimirLinha(std::string_view Texto = "") {

    // Caso o usuário user versões acima ou igual ao C++23...
    #if defined(__cpp_lib_print) && (__cpp_lib_print >= 202207L) 
        std::println("{}", Texto);

    // Caso o usuário user versões abaixo do C++23...
    #else
        std::cout << Texto << '\n'; 

    #endif
    }

    // Envolve a mensagem com a cor informada e RESET quando o terminal suportar ANSI.
    [[nodiscard]] inline std::string colorirTexto(std::string_view Cor, std::string_view Mensagem) {
        if (!Ambiente::deveUsarAnsi()) {
            return std::string(Mensagem);
        }

        std::string Linha;
        Linha.reserve(Cor.size() + Mensagem.size() + Cores::RESET.size());
        Linha.append(Cor);
        Linha.append(Mensagem);
        Linha.append(Cores::RESET);
        return Linha;
    }

    // Imprime uma mensagem colorida seguida de quebra de linha.
    inline void imprimirColorido(std::string_view Cor, std::string_view Mensagem) {
        imprimirLinha(colorirTexto(Cor, Mensagem));
    }

    inline void separador() {
        imprimirColorido(Cores::CINZA, "----------------------------------------------------");
    }

    inline void escreveInfo(std::string_view Mensagem) {
        imprimirColorido(Cores::CIANO, Mensagem);
    }

    inline void escreveAviso(std::string_view Mensagem) {
        imprimirColorido(Cores::AMARELO, Mensagem);
    }

    inline void escreveErro(std::string_view Mensagem) {
        imprimirColorido(Cores::VERMELHO, Mensagem);
    }

}  // namespace Terminal::Saida

#endif
