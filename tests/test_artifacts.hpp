/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Testes de normalizacao, estrutura e encoding dos artefatos gerados (Testes 13 a 19).
*/

#ifndef TEST_ARTIFACTS_HPP
#define TEST_ARTIFACTS_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "test_harness.hpp"
#include "test_process.hpp"

//========================= [ TESTES DE ARTEFATOS ] ============================

namespace Testes::Artefatos {

    inline void executar(
        Infra::Estatisticas& Est,
        const std::filesystem::path& ExecAlvo,
        const std::filesystem::path& RaizTemp,
        const std::filesystem::path& DirProjeto,
        std::string_view NomeProjetoValido
    ) {
        Infra::imprimirCabecalhoSecao("3. Normalizacao, Estrutura e Encoding dos Artefatos");

        Infra::invocarTeste(Est, "Espacos externos sofrem trim unico e criam a pasta exata", [&]() {
            const Processo::ResultadoProcesso RES{
                Processo::invocarBootstrap(ExecAlvo, {"  ProjetoTrimado  "}, RaizTemp)
            };
            Infra::exigir(RES.CodigoSaida == 0, "Esperado exit code 0");

            const std::filesystem::path DIR_TRIMADO{RaizTemp / "ProjetoTrimado"};
            Infra::exigir(std::filesystem::is_directory(DIR_TRIMADO), "Pasta 'ProjetoTrimado' nao criada");

            for (const auto& ENTRADA : std::filesystem::directory_iterator(RaizTemp)) {
                const std::string NOME_PASTA{ENTRADA.path().filename().string()};
                Infra::exigir(
                    NOME_PASTA.front() != ' ' && NOME_PASTA.back() != ' ',
                    "Pasta criada com espaco nas bordas: " + NOME_PASTA
                );
            }

            const std::string TEXTO_CMAKE{Arquivos::lerTodoTexto(DIR_TRIMADO / "CMakeLists.txt")};
            Infra::exigir(
                Infra::contem(TEXTO_CMAKE, "project(ProjetoTrimado LANGUAGES CXX)"),
                "CMakeLists nao recebeu nome trimado"
            );
        });

        Infra::invocarTeste(Est, "Criacao de projeto valido retorna 0, instrucoes CMake e suporta --quiet", [&]() {
            const Processo::ResultadoProcesso RES{
                Processo::invocarBootstrap(ExecAlvo, {std::string{NomeProjetoValido}}, RaizTemp)
            };
            Infra::exigir(RES.CodigoSaida == 0, "Esperado exit code 0");
            Infra::exigir(Infra::contem(RES.TextoStdout, "criado com sucesso"), "Mensagem de sucesso ausente");
            Infra::exigir(
                Infra::contem(RES.TextoStdout, "cmake -B build") &&
                Infra::contem(RES.TextoStdout, "cmake --build build"),
                "Instrucoes CMake ausentes"
            );
            Infra::exigir(
                Infra::contem(RES.TextoStdout, "Autor:"),
                "Rodape de autor deveria aparecer sem --quiet"
            );

            const Processo::ResultadoProcesso RES_QUIET{
                Processo::invocarBootstrap(ExecAlvo, {"ProjetoQuiet", "--quiet"}, RaizTemp)
            };
            Infra::exigir(RES_QUIET.CodigoSaida == 0, "--quiet deveria retornar 0");
            Infra::exigir(
                !Infra::contem(RES_QUIET.TextoStdout, "Autor:"),
                "--quiet deveria suprimir rodape de autor"
            );
        });

        Infra::invocarTeste(Est, "Diretorios obrigatorios gerados (src, include, .vscode)", [&]() {
            Infra::exigir(std::filesystem::is_directory(DirProjeto), "Raiz do projeto ausente");
            Infra::exigir(std::filesystem::is_directory(DirProjeto / "src"), "Pasta src ausente");
            Infra::exigir(std::filesystem::is_directory(DirProjeto / "include"), "Pasta include ausente");
            Infra::exigir(std::filesystem::is_directory(DirProjeto / ".vscode"), "Pasta .vscode ausente");
        });

        Infra::invocarTeste(Est, "Arquivo src/main.cpp gerado com assinatura valida e locale portavel", [&]() {
            const std::string MAIN_CPP{Arquivos::lerTodoTexto(DirProjeto / "src" / "main.cpp")};
            Infra::exigir(Infra::contem(MAIN_CPP, "int main()"), "Assinatura int main() ausente");
            Infra::exigir(Infra::contem(MAIN_CPP, "#include <iostream>"), "<iostream> ausente");
            Infra::exigir(Infra::contem(MAIN_CPP, "#include <clocale>"), "<clocale> ausente");
            Infra::exigir(Infra::contem(MAIN_CPP, "std::setlocale(LC_ALL, \"\")"), "std::setlocale portavel ausente");
        });

        Infra::invocarTeste(Est, "Configuracoes do VS Code possuem blocos multiplataforma validos", [&]() {
            const std::string LAUNCH_JSON{Arquivos::lerTodoTexto(DirProjeto / ".vscode" / "launch.json")};
            Infra::exigir(
                Infra::contem(LAUNCH_JSON, "\"windows\"") &&
                Infra::contem(LAUNCH_JSON, "\"linux\"") &&
                Infra::contem(LAUNCH_JSON, "\"osx\""),
                "launch.json sem blocos multiplataforma"
            );

            const std::string SETTINGS_JSON{Arquivos::lerTodoTexto(DirProjeto / ".vscode" / "settings.json")};
            Infra::exigir(
                Infra::contem(SETTINGS_JSON, "\"cmake.exportCompileCommandsFile\": true"),
                "exportCompileCommandsFile ausente"
            );
            Infra::exigir(
                Infra::contem(SETTINGS_JSON, "--query-driver=") &&
                Infra::contem(SETTINGS_JSON, "/usr/bin/"),
                "--query-driver multiplataforma ausente"
            );
        });

        Infra::invocarTeste(Est, "CMakeLists.txt gerado com C++23 estrito, guarda de build e Secao 9 (stdc++exp)", [&]() {
            const std::string TEXTO_CMAKE{Arquivos::lerTodoTexto(DirProjeto / "CMakeLists.txt")};
            Infra::exigir(!Infra::contem(TEXTO_CMAKE, "__PROJECT_NAME__"), "Placeholder __PROJECT_NAME__ nao substituido");
            Infra::exigir(
                Infra::contem(TEXTO_CMAKE, "project(" + std::string{NomeProjetoValido} + " LANGUAGES CXX)"),
                "Declaracao project() ausente"
            );
            Infra::exigir(Infra::contem(TEXTO_CMAKE, "set(CMAKE_CXX_STANDARD 23)"), "C++23 ausente");
            Infra::exigir(Infra::contem(TEXTO_CMAKE, "set(CMAKE_CXX_STANDARD_REQUIRED ON)"), "CXX_STANDARD_REQUIRED ausente");
            Infra::exigir(
                Infra::contem(TEXTO_CMAKE, "if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)"),
                "Guarda CMAKE_BUILD_TYPE ausente"
            );
            Infra::exigir(
                Infra::contem(TEXTO_CMAKE, "-pedantic-errors") && Infra::contem(TEXTO_CMAKE, "/permissive-"),
                "Flags rigorosas ausentes"
            );
            Infra::exigir(
                Infra::contem(TEXTO_CMAKE, "include(CheckLinkerFlag)") &&
                Infra::contem(TEXTO_CMAKE, "check_linker_flag(CXX \"-lstdc++exp\" HAVE_STDCXXEXP)") &&
                Infra::contem(TEXTO_CMAKE, "target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)"),
                "Secao 9 stdc++exp ausente"
            );
        });

        Infra::invocarTeste(Est, ".gitignore preserva VS Code e todos os 5 arquivos estao em UTF-8 sem BOM + LF", [&]() {
            const std::string GITIGNORE{Arquivos::lerTodoTexto(DirProjeto / ".gitignore")};
            Infra::exigir(
                Infra::contem(GITIGNORE, "build/") && Infra::contem(GITIGNORE, "*.exe"),
                "Padroes de build ausentes"
            );
            Infra::exigir(
                Infra::contem(GITIGNORE, ".vscode/*") &&
                Infra::contem(GITIGNORE, "!.vscode/launch.json") &&
                Infra::contem(GITIGNORE, "!.vscode/settings.json"),
                "Regras de preservacao .vscode ausentes"
            );

            const std::vector<std::filesystem::path> ARQUIVOS_GERADOS{
                DirProjeto / "CMakeLists.txt",
                DirProjeto / ".gitignore",
                DirProjeto / "src" / "main.cpp",
                DirProjeto / ".vscode" / "launch.json",
                DirProjeto / ".vscode" / "settings.json"
            };
            for (const std::filesystem::path& ARQUIVO : ARQUIVOS_GERADOS) {
                Infra::exigir(std::filesystem::is_regular_file(ARQUIVO), "Arquivo gerado ausente: " + ARQUIVO.string());
                Infra::exigir(!Arquivos::possuiBomUtf8(ARQUIVO), "BOM UTF-8 detectado em: " + ARQUIVO.string());
                const std::string TEXTO{Arquivos::lerTodoTexto(ARQUIVO)};
                Infra::exigir(!Infra::contem(TEXTO, "\r\n"), "CRLF detectado em: " + ARQUIVO.string());
            }
        });
    }

}  // namespace Testes::Artefatos

#endif
