/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Validacao de nomes em 5 regras (vazio, ponto final, reservados, CMake e colisao).
*/

//============================ [ CABECALHOS PADRAO ] ===========================

#include <algorithm>
#include <array>
#include <system_error>
#include <vector>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "terminal.hpp"
#include "validator.hpp"

//======================== [ AUXILIARES DE CARACTERES ] ========================

namespace Validadores::Caracteres {

    [[nodiscard]] constexpr bool ehLetraAscii(char Caractere) noexcept {
        return (Caractere >= 'A' && Caractere <= 'Z') ||
               (Caractere >= 'a' && Caractere <= 'z');
    }

    [[nodiscard]] constexpr bool ehDigitoAscii(char Caractere) noexcept {
        return Caractere >= '0' && Caractere <= '9';
    }

    [[nodiscard]] constexpr bool ehInicioPermitido(char Caractere) noexcept {
        return ehLetraAscii(Caractere) || ehDigitoAscii(Caractere) || Caractere == '_';
    }

    [[nodiscard]] constexpr bool ehCorpoPermitido(char Caractere) noexcept {
        return ehInicioPermitido(Caractere) || Caractere == '-';
    }

    [[nodiscard]] constexpr char paraMaiusculoAscii(char Caractere) noexcept {
        if (Caractere >= 'a' && Caractere <= 'z') {
            return static_cast<char>(Caractere - ('a' - 'A'));
        }
        return Caractere;
    }

    [[nodiscard]] bool apenasEspacos(std::string_view Texto) noexcept {
        for (const char CARACTERE : Texto) {
            if (CARACTERE != ' '  && CARACTERE != '\t' && CARACTERE != '\n' &&
                CARACTERE != '\r' && CARACTERE != '\v' && CARACTERE != '\f') {
                return false;
            }
        }
        return true;
    }

}  // namespace Validadores::Caracteres

//========================== [ REGRAS DE VALIDACAO ] ===========================

namespace Validadores::Regras {

    [[nodiscard]] bool ehNomeReservadoWin(std::string_view Nome) {
        static constexpr std::array<std::string_view, 22> NOMES_RESERVADOS{
            "CON", "PRN", "AUX", "NUL",
            "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
            "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
        };

        const std::size_t POS_PONTO{Nome.find('.')};
        const std::string_view PREFIXO{
            (POS_PONTO == std::string_view::npos) ? Nome : Nome.substr(0, POS_PONTO)
        };

        std::string Maiusculo{};
        Maiusculo.reserve(PREFIXO.size());
        for (const char CARACTERE : PREFIXO) {
            Maiusculo.push_back(Caracteres::paraMaiusculoAscii(CARACTERE));
        }

        return std::find(NOMES_RESERVADOS.begin(), NOMES_RESERVADOS.end(), Maiusculo) !=
               NOMES_RESERVADOS.end();
    }

    [[nodiscard]] bool possuiPadraoValido(std::string_view Nome) noexcept {
        if (Nome.empty() || !Caracteres::ehInicioPermitido(Nome.front())) {
            return false;
        }
        for (const char CARACTERE : Nome) {
            if (!Caracteres::ehCorpoPermitido(CARACTERE)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::string formatarCharsInvalidos(std::string_view Nome) {
        std::vector<std::string> Encontrados{};

        for (const char CARACTERE : Nome) {
            if (Caracteres::ehCorpoPermitido(CARACTERE)) {
                continue;
            }

            std::string Token{};
            if (CARACTERE == ' ') {
                Token = "<espaco>";
            }
            else if (static_cast<unsigned char>(CARACTERE) < 0x20 ||
                     static_cast<unsigned char>(CARACTERE) == 0x7F) {
                Token = "<ctrl>";
            }
            else {
                Token.push_back(CARACTERE);
            }

            if (std::find(Encontrados.begin(), Encontrados.end(), Token) == Encontrados.end()) {
                Encontrados.push_back(std::move(Token));
            }
        }

        if (Encontrados.empty()) {
            return "inicio com '-'";
        }

        std::string Unido{};
        for (std::size_t Indice{0}; Indice < Encontrados.size(); ++Indice) {
            if (Indice > 0) {
                Unido.push_back(' ');
            }
            Unido.append(Encontrados[Indice]);
        }
        return Unido;
    }

}  // namespace Validadores::Regras

//======================== [ DIAGNOSTICOS NO TERMINAL ] ========================

namespace Validadores::Diagnostico {

    void emitirErroVazio() {
        Terminal::Saida::escreveErro("[ERRO] Nome do projeto vazio.");
        Terminal::Saida::escreveInfo("[INFO] Exemplo (Windows): cpp-bootstrap MeuProjeto");
        Terminal::Saida::escreveInfo("[INFO] Exemplo (Linux/macOS): ./cpp-bootstrap MeuProjeto");
    }

    void emitirErroPontoFinal(std::string_view Nome) {
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string{Nome} + "'");
        Terminal::Saida::escreveAviso("Nomes de pasta nao podem terminar com '.' ou espaco.");
        Terminal::Saida::separador();
    }

    void emitirErroReservado(std::string_view Nome) {
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string{Nome} + "'");
        Terminal::Saida::escreveAviso("Esse e um nome reservado do Windows.");
        Terminal::Saida::escreveInfo("Escolha outro (ex: '" + std::string{Nome} + "_proj').");
        Terminal::Saida::separador();
    }

    void emitirErroPadrao(std::string_view Nome, std::string_view CharsInvalidos) {
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro("[ERRO] Nome invalido: '" + std::string{Nome} + "'");
        Terminal::Saida::escreveAviso("Caracteres proibidos encontrados: " + std::string{CharsInvalidos});
        Terminal::Saida::escreveInfo(
            "Use apenas letras, numeros, '_' e '-' (sem espacos e sem iniciar com '-')."
        );
        Terminal::Saida::separador();
    }

    void emitirErroColisao(std::string_view Nome) {
        Terminal::Saida::separador();
        Terminal::Saida::escreveErro(
            "[ERRO] Ja existe uma pasta ou arquivo com o nome: '" + std::string{Nome} + "'"
        );
        Terminal::Saida::escreveInfo("Sugestoes:");
        Terminal::Saida::escreveInfo("  * Use outro nome: cpp-bootstrap " + std::string{Nome} + "_2");
        Terminal::Saida::escreveInfo("  * Ou renomeie/apague o item existente.");
        Terminal::Saida::separador();
    }

}  // namespace Validadores::Diagnostico

//=========================== [ VALIDACAO PUBLICA ] ============================

namespace Validadores {

    bool validarNomeProjeto(
        std::string_view Nome,
        const std::filesystem::path& DirBase,
        bool Silencioso
    ) {
        // 1 - Nome vazio ou apenas espacos
        if (Nome.empty() || Caracteres::apenasEspacos(Nome)) {
            if (!Silencioso) {
                Diagnostico::emitirErroVazio();
            }
            return false;
        }

        // 2 - Nome terminando com ponto (invalido no Windows e bloqueado para portabilidade Git)
        if (Nome.back() == '.') {
            if (!Silencioso) {
                Diagnostico::emitirErroPontoFinal(Nome);
            }
            return false;
        }

        // 3 - Nomes de dispositivos reservados do Windows (bloqueados em todos os SOs)
        if (Regras::ehNomeReservadoWin(Nome)) {
            if (!Silencioso) {
                Diagnostico::emitirErroReservado(Nome);
            }
            return false;
        }

        // 4 - Regra uniforme para CMake e filesystems multiplataforma
        if (!Regras::possuiPadraoValido(Nome)) {
            if (!Silencioso) {
                const std::string CHARS_INVALIDOS{Regras::formatarCharsInvalidos(Nome)};
                Diagnostico::emitirErroPadrao(Nome, CHARS_INVALIDOS);
            }
            return false;
        }

        // 5 - Prevencao contra sobrescrita de pasta ou arquivo ja existente
        std::error_code CodigoErro{};
        const std::filesystem::path CAMINHO_CANDIDATO{DirBase / std::filesystem::path{Nome}};
        if (std::filesystem::exists(CAMINHO_CANDIDATO, CodigoErro)) {
            if (!Silencioso) {
                Diagnostico::emitirErroColisao(Nome);
            }
            return false;
        }

        return true;
    }

}  // namespace Validadores
