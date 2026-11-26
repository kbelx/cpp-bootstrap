/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Infraestrutura base da suite de testes (estatisticas, assercoes e sandbox RAII).
*/

#ifndef TEST_HARNESS_HPP
#define TEST_HARNESS_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "cores.hpp"
#include "terminal.hpp"

//======================== [ INFRAESTRUTURA DE TESTES ] ========================

namespace Testes::Infra {

    struct Estatisticas {
        int Total{0};
        int Aprovados{0};
        int Falhos{0};
        int Ignorados{0};
    };

    inline bool ModoVerboso{false};

    inline void imprimirDivisor() {
        Terminal::Saida::imprimirColorido(
            Cores::CINZA,
            "------------------------------------------------------------"
        );
    }

    inline void imprimirCabecalhoSecao(std::string_view Titulo) {
        Terminal::Saida::imprimirLinha("");
        imprimirDivisor();
        Terminal::Saida::imprimirColorido(
            std::string{Cores::NEGRITO} + std::string{Cores::CIANO},
            Titulo
        );
        imprimirDivisor();
    }

    [[nodiscard]] inline std::string formatarNumeroTeste(int Numero) {
        std::ostringstream Fluxo{};
        Fluxo << std::setw(2) << std::setfill('0') << Numero;
        return Fluxo.str();
    }

    inline void invocarTeste(
        Estatisticas& Est,
        std::string_view Nome,
        const std::function<void()>& Assercao
    ) {
        ++Est.Total;
        const std::string NUM{formatarNumeroTeste(Est.Total)};

        try {
            Assercao();
            ++Est.Aprovados;
            Terminal::Saida::imprimirLinha(
                "  " + Terminal::Saida::colorirTexto(Cores::VERDE, "[PASS]") +
                " Teste " + NUM + ": " + std::string{Nome}
            );
        }
        catch (const std::exception& EXCECAO) {
            ++Est.Falhos;
            Terminal::Saida::imprimirLinha(
                "  " + Terminal::Saida::colorirTexto(Cores::VERMELHO, "[FAIL]") +
                " Teste " + NUM + ": " + std::string{Nome}
            );
            Terminal::Saida::imprimirLinha(
                "         " + Terminal::Saida::colorirTexto(Cores::VERMELHO, "-> Erro:") +
                " " + EXCECAO.what()
            );
        }
    }

    inline void invocarIgnorado(
        Estatisticas& Est,
        std::string_view Nome,
        std::string_view Motivo
    ) {
        ++Est.Total;
        ++Est.Ignorados;
        const std::string NUM{formatarNumeroTeste(Est.Total)};
        Terminal::Saida::imprimirLinha(
            "  " + Terminal::Saida::colorirTexto(Cores::AMARELO, "[SKIP]") +
            " Teste " + NUM + ": " + std::string{Nome} + " (" + std::string{Motivo} + ")"
        );
    }

    inline void exigir(bool Condicao, std::string_view Mensagem) {
        if (!Condicao) {
            throw std::runtime_error(std::string{Mensagem});
        }
    }

    [[nodiscard]] inline bool contem(std::string_view Texto, std::string_view Trecho) noexcept {
        return Texto.find(Trecho) != std::string_view::npos;
    }

    [[nodiscard]] inline int exibirResumoFinal(const Estatisticas& Est, double SegundosDecorridos) {
        Terminal::Saida::imprimirLinha("");
        imprimirDivisor();
        Terminal::Saida::imprimirColorido(Cores::NEGRITO, "[RESUMO DA EXECUCAO DOS TESTES]");
        imprimirDivisor();
        Terminal::Saida::imprimirLinha("  Total de testes : " + std::to_string(Est.Total));
        Terminal::Saida::imprimirLinha(
            "  " + Terminal::Saida::colorirTexto(
                Cores::VERDE,
                "Aprovados       : " + std::to_string(Est.Aprovados)
            )
        );
        Terminal::Saida::imprimirLinha(
            "  " + Terminal::Saida::colorirTexto(
                Est.Falhos > 0 ? Cores::VERMELHO : Cores::CINZA,
                "Falhos          : " + std::to_string(Est.Falhos)
            )
        );
        if (Est.Ignorados > 0) {
            Terminal::Saida::imprimirLinha(
                "  " + Terminal::Saida::colorirTexto(
                    Cores::AMARELO,
                    "Ignorados       : " + std::to_string(Est.Ignorados)
                )
            );
        }

        std::ostringstream FluxoTempo{};
        FluxoTempo << std::fixed << std::setprecision(2) << SegundosDecorridos << "s";
        Terminal::Saida::imprimirLinha("  Tempo total     : " + FluxoTempo.str());
        imprimirDivisor();
        Terminal::Saida::imprimirLinha("");

        if (Est.Falhos > 0) {
            Terminal::Saida::escreveErro("[FAIL] A suite de testes falhou!");
            return 1;
        }

        Terminal::Saida::imprimirColorido(
            Cores::VERDE,
            "[PASS] Todos os testes passaram com sucesso!"
        );
        return 0;
    }

}  // namespace Testes::Infra

//========================= [ UTILITARIOS DE ARQUIVO ] =========================

namespace Testes::Arquivos {

    [[nodiscard]] inline std::vector<unsigned char> lerTodosBytes(const std::filesystem::path& Caminho) {
        std::ifstream Fluxo{Caminho, std::ios::binary | std::ios::ate};
        Infra::exigir(Fluxo.is_open(), "Nao foi possivel abrir arquivo: " + Caminho.string());

        const std::streamsize TAM{Fluxo.tellg()};
        Infra::exigir(TAM >= 0, "Falha ao obter tamanho do arquivo: " + Caminho.string());
        Fluxo.seekg(0, std::ios::beg);

        std::vector<unsigned char> Bytes(static_cast<std::size_t>(TAM));
        if (TAM > 0) {
            Infra::exigir(
                static_cast<bool>(Fluxo.read(reinterpret_cast<char*>(Bytes.data()), TAM)),
                "Falha ao ler bytes de: " + Caminho.string()
            );
        }
        return Bytes;
    }

    [[nodiscard]] inline std::string lerTodoTexto(const std::filesystem::path& Caminho) {
        const std::vector<unsigned char> BYTES{lerTodosBytes(Caminho)};
        return std::string{reinterpret_cast<const char*>(BYTES.data()), BYTES.size()};
    }

    [[nodiscard]] inline bool possuiBomUtf8(const std::filesystem::path& Caminho) {
        const std::vector<unsigned char> BYTES{lerTodosBytes(Caminho)};
        return BYTES.size() >= 3 && BYTES[0] == 0xEF && BYTES[1] == 0xBB && BYTES[2] == 0xBF;
    }

    // Cria um diretorio temporario isolado e garante limpeza automatica via RAII.
    class GuardaSandboxTemp {
        public:
            GuardaSandboxTemp() {
                std::random_device GeradorAleatorio{};
                std::mt19937_64 Motor{GeradorAleatorio()};
                const auto SUFIXO{Motor()};

                std::ostringstream Fluxo{};
                Fluxo << "cpp_bootstrap_tests_" << std::hex << SUFIXO;
                Raiz = std::filesystem::temp_directory_path() / Fluxo.str();
                std::filesystem::create_directories(Raiz);
            }

            ~GuardaSandboxTemp() noexcept {
                if (!Raiz.empty()) {
                    std::error_code CodigoErro{};
                    std::filesystem::remove_all(Raiz, CodigoErro);
                }
            }

            GuardaSandboxTemp(const GuardaSandboxTemp&) = delete;
            GuardaSandboxTemp& operator=(const GuardaSandboxTemp&) = delete;

            [[nodiscard]] const std::filesystem::path& caminho() const noexcept {
                return Raiz;
            }

        private:
            std::filesystem::path Raiz{};
    };

}  // namespace Testes::Arquivos

#endif
