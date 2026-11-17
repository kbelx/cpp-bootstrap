/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Teste de integracao real com CMake e compilacao C++23 (<print>/<stacktrace>) (Teste 20).
*/

#ifndef TEST_REAL_BUILD_HPP
#define TEST_REAL_BUILD_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "generator.hpp"
#include "test_harness.hpp"
#include "test_process.hpp"

//======================== [ INTEGRACAO REAL / CMAKE ] =========================

namespace Testes::BuildReal {

    inline void configurarProjetoCMake(
        const std::filesystem::path& DirProjeto,
        [[maybe_unused]] const std::filesystem::path& DirBuild
    ) {
        bool Configurado{false};

    #if defined(_WIN32)
        const bool TEM_MAKE{
            Processo::comandoExisteNoPath("mingw32-make") || Processo::comandoExisteNoPath("make")
        };
        if (TEM_MAKE && Processo::comandoExisteNoPath("g++")) {
            const Processo::ResultadoProcesso CFG_MINGW{Processo::executarProcesso(
                {"cmake", "-B", "build", "-G", "MinGW Makefiles", "-DCMAKE_BUILD_TYPE=Release"},
                DirProjeto
            )};
            if (CFG_MINGW.CodigoSaida == 0) {
                Configurado = true;
            }
            else if (std::filesystem::exists(DirBuild)) {
                std::error_code CodigoErro{};
                std::filesystem::remove_all(DirBuild, CodigoErro);
            }
        }
    #endif

        if (!Configurado) {
            const Processo::ResultadoProcesso CFG_PADRAO{Processo::executarProcesso(
                {"cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"},
                DirProjeto
            )};
            Infra::exigir(CFG_PADRAO.CodigoSaida == 0, "cmake -B build falhou: " + CFG_PADRAO.TextoStderr);
        }
    }

    inline void verificarPreservacaoRelease(const std::filesystem::path& DirBuild) {
        const std::filesystem::path ARQUIVO_CACHE{DirBuild / "CMakeCache.txt"};
        if (std::filesystem::exists(ARQUIVO_CACHE)) {
            const std::string TEXTO_CACHE{Arquivos::lerTodoTexto(ARQUIVO_CACHE)};
            if (!Infra::contem(TEXTO_CACHE, "CMAKE_CONFIGURATION_TYPES")) {
                Infra::exigir(
                    Infra::contem(TEXTO_CACHE, "CMAKE_BUILD_TYPE:STRING=Release"),
                    "CMakeLists.txt sobrescreveu CMAKE_BUILD_TYPE=Release"
                );
            }
        }
    }

    [[nodiscard]] inline std::filesystem::path localizarBinarioCompilado(
        const std::filesystem::path& DirBuild,
        std::string_view NomeProjetoValido
    ) {
    #if defined(_WIN32)
        const std::string NOME_BIN{std::string{NomeProjetoValido} + ".exe"};
    #else
        const std::string NOME_BIN{std::string{NomeProjetoValido}};
    #endif
        const std::vector<std::filesystem::path> CANDIDATOS{
            DirBuild / NOME_BIN,
            DirBuild / "Release" / NOME_BIN,
            DirBuild / "Debug" / NOME_BIN
        };

        for (const std::filesystem::path& CANDIDATO : CANDIDATOS) {
            if (std::filesystem::is_regular_file(CANDIDATO)) {
                return CANDIDATO;
            }
        }
        return {};
    }

    inline void executar(
        Infra::Estatisticas& Est,
        const std::filesystem::path& DirProjeto,
        std::string_view NomeProjetoValido,
        bool IncluirBuild
    ) {
        Infra::imprimirCabecalhoSecao("4. Integracao Real com CMake e C++23");

        const bool TEM_CMAKE{Processo::comandoExisteNoPath("cmake")};
        const bool TEM_COMPILADOR{
            Processo::comandoExisteNoPath("g++") ||
            Processo::comandoExisteNoPath("clang++") ||
            Processo::comandoExisteNoPath("cl")
        };

        if (!IncluirBuild) {
            Infra::invocarIgnorado(
                Est,
                "Compilacao real e linkagem C++23 (<print>/<stacktrace>)",
                "Passe --include-build para executar"
            );
            return;
        }

        if (!TEM_CMAKE || !TEM_COMPILADOR) {
            Infra::invocarIgnorado(
                Est,
                "Compilacao real e linkagem C++23 (<print>/<stacktrace>)",
                "CMake ou compilador C++ ausente no PATH"
            );
            return;
        }

        Infra::invocarTeste(Est, "Projeto gerado configura, respeita Release e executa C++23 com stdc++exp", [&]() {
            const std::filesystem::path DIR_BUILD{DirProjeto / "build"};
            configurarProjetoCMake(DirProjeto, DIR_BUILD);
            verificarPreservacaoRelease(DIR_BUILD);

            const Processo::ResultadoProcesso RES_BUILD{Processo::executarProcesso(
                {"cmake", "--build", "build", "--config", "Release"},
                DirProjeto
            )};
            Infra::exigir(RES_BUILD.CodigoSaida == 0, "cmake --build build falhou: " + RES_BUILD.TextoStderr);

            // Substitui src/main.cpp para exercitar <print> e <stacktrace> via stdc++exp
            static constexpr std::string_view CHECAGEM_CPP23{R"cpp(#include <iostream>
#if __has_include(<print>)
#include <print>
#endif
#if __has_include(<stacktrace>)
#include <stacktrace>
#endif

int main() {
#if defined(__cpp_lib_print)
    std::println("C++23 std::println ativo");
#else
    std::cout << "C++23 padrao ativo\n";
#endif
#if defined(__cpp_lib_stacktrace)
    auto st = std::stacktrace::current();
    if (st.empty()) {
        return 1;
    }
#endif
    return 0;
}
)cpp"};
            Gerador::Arquivo::gravarUtf8SemBom(DirProjeto / "src" / "main.cpp", CHECAGEM_CPP23);

            const Processo::ResultadoProcesso RES_REBUILD{Processo::executarProcesso(
                {"cmake", "--build", "build", "--clean-first", "--config", "Release"},
                DirProjeto
            )};
            Infra::exigir(
                RES_REBUILD.CodigoSaida == 0,
                "Falha ao compilar/linkar C++23: " + RES_REBUILD.TextoStderr
            );

            const std::filesystem::path BIN_ENCONTRADO{
                localizarBinarioCompilado(DIR_BUILD, NomeProjetoValido)
            };
            Infra::exigir(!BIN_ENCONTRADO.empty(), "Binario C++23 compilado nao encontrado");

            const Processo::ResultadoProcesso RES_EXEC{
                Processo::executarProcesso({BIN_ENCONTRADO.string()}, DirProjeto)
            };
            Infra::exigir(RES_EXEC.CodigoSaida == 0, "Execucao do binario gerado falhou");
            Infra::exigir(
                Infra::contem(RES_EXEC.TextoStdout, "C++23"),
                "Saida esperada C++23 ausente: " + RES_EXEC.TextoStdout
            );
        });
    }

}  // namespace Testes::BuildReal

#endif
