/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Orquestrador da suite automatizada de testes unitarios e de integracao C++23 no CTest.
*/

//============================ [ CABECALHOS PADRAO ] ===========================

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "terminal.hpp"
#include "test_artifacts.hpp"
#include "test_cli_integration.hpp"
#include "test_harness.hpp"
#include "test_real_build.hpp"
#include "test_unit.hpp"

//============================ [ PONTO DE ENTRADA ] ============================

int main(int QtdArgs, char* VetorArgs[]) {
    std::filesystem::path ExecAlvo{};
    bool IncluirBuild{false};

    for (int Indice{1}; Indice < QtdArgs; ++Indice) {
        const std::string_view ARG{VetorArgs[Indice]};
        if (ARG == "--target-exe" && (Indice + 1) < QtdArgs) {
            ExecAlvo = std::filesystem::weakly_canonical(std::filesystem::path{VetorArgs[++Indice]});
        }
        else if (ARG == "--include-build") {
            IncluirBuild = true;
        }
        else if (ARG == "--verbose") {
            Testes::Infra::ModoVerboso = true;
        }
    }

    if (ExecAlvo.empty()) {
        const std::filesystem::path DIR_PADRAO{std::filesystem::path{VetorArgs[0]}.parent_path()};
    #if defined(_WIN32)
        ExecAlvo = std::filesystem::weakly_canonical(DIR_PADRAO / "cpp-bootstrap.exe");
    #else
        ExecAlvo = std::filesystem::weakly_canonical(DIR_PADRAO / "cpp-bootstrap");
    #endif
    }

    if (!std::filesystem::exists(ExecAlvo)) {
        Terminal::Saida::escreveErro("[ERRO] Executavel alvo nao encontrado: " + ExecAlvo.string());
        return 1;
    }

    Testes::Arquivos::GuardaSandboxTemp Sandbox{};
    const std::filesystem::path& RAIZ_TEMP{Sandbox.caminho()};

    const auto INICIO{std::chrono::steady_clock::now()};
    Testes::Infra::Estatisticas Est{};

    Testes::Infra::imprimirCabecalhoSecao("[TEST SUITE] cpp-bootstrap (C++23)");
    Terminal::Saida::imprimirLinha("* Executavel alvo  : " + ExecAlvo.string());
    Terminal::Saida::imprimirLinha("* Ambiente sandbox : " + RAIZ_TEMP.string());
    Terminal::Saida::imprimirLinha("* Sistema          : " + std::string{Terminal::Ambiente::detectarSO()});

    static constexpr std::string_view NOME_PROJETO_VALIDO{"ProjetoTesteSucesso"};
    const std::filesystem::path DIR_PROJETO{RAIZ_TEMP / NOME_PROJETO_VALIDO};

    Testes::Unitarios::executar(Est, RAIZ_TEMP);
    Testes::IntegracaoCli::executar(Est, ExecAlvo, RAIZ_TEMP);
    Testes::Artefatos::executar(Est, ExecAlvo, RAIZ_TEMP, DIR_PROJETO, NOME_PROJETO_VALIDO);
    Testes::BuildReal::executar(Est, DIR_PROJETO, NOME_PROJETO_VALIDO, IncluirBuild);

    const double SEGUNDOS_DECORRIDOS{
        std::chrono::duration<double>(std::chrono::steady_clock::now() - INICIO).count()
    };
    return Testes::Infra::exibirResumoFinal(Est, SEGUNDOS_DECORRIDOS);
}
