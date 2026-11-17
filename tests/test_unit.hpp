/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Testes unitarios de CLI, normalizacao e validacao de nomes (Testes 01 a 07).
*/

#ifndef TEST_UNIT_HPP
#define TEST_UNIT_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "cli.hpp"
#include "test_harness.hpp"
#include "validator.hpp"

//=========================== [ TESTES UNITARIOS ] =============================

namespace Testes::Unitarios {

    inline void executar(Infra::Estatisticas& Est, const std::filesystem::path& RaizTemp) {
        Infra::imprimirCabecalhoSecao("1. Testes Unitarios de CLI, Normalizacao e Validacao");

        Infra::invocarTeste(Est, "limparNomeProjeto remove espacos e aspas envolventes", []() {
            Infra::exigir(
                CLI::Parser::limparNomeProjeto("  Projeto  ") == "Projeto",
                "Falha no trim de espacos"
            );
            Infra::exigir(
                CLI::Parser::limparNomeProjeto(" \"'Projeto'\" ") == "Projeto",
                "Falha no trim de aspas"
            );
            Infra::exigir(
                CLI::Parser::limparNomeProjeto("   \"\"   ").empty(),
                "Aspas vazias devem virar string vazia"
            );
        });

        Infra::invocarTeste(Est, "validarNomeProjeto rejeita nomes vazios ou apenas espacos", [&]() {
            Infra::exigir(
                !Validadores::validarNomeProjeto("", RaizTemp, true),
                "String vazia deveria falhar"
            );
            Infra::exigir(
                !Validadores::validarNomeProjeto("   ", RaizTemp, true),
                "Apenas espacos deveria falhar"
            );
        });

        Infra::invocarTeste(Est, "validarNomeProjeto rejeita caracteres proibidos e inicio com '-'", [&]() {
            Infra::exigir(
                !Validadores::validarNomeProjeto("Projeto<Invalido>", RaizTemp, true),
                "< > deveria falhar"
            );
            Infra::exigir(
                !Validadores::validarNomeProjeto("dir/sub", RaizTemp, true),
                "/ deveria falhar"
            );
            Infra::exigir(
                !Validadores::validarNomeProjeto("-iniciadoComTraco", RaizTemp, true),
                "Inicio com '-' deveria falhar"
            );
        });

        Infra::invocarTeste(Est, "validarNomeProjeto rejeita espaco interno (protege CMake)", [&]() {
            Infra::exigir(
                !Validadores::validarNomeProjeto("Meu Projeto", RaizTemp, true),
                "Espaco interno deveria falhar"
            );
        });

        Infra::invocarTeste(Est, "validarNomeProjeto rejeita nome terminando com ponto final", [&]() {
            Infra::exigir(
                !Validadores::validarNomeProjeto("ProjetoComPonto.", RaizTemp, true),
                "Ponto final deveria falhar"
            );
        });

        Infra::invocarTeste(Est, "validarNomeProjeto rejeita nomes reservados do Windows em qualquer SO", [&]() {
            Infra::exigir(!Validadores::validarNomeProjeto("CON", RaizTemp, true), "CON deveria falhar");
            Infra::exigir(!Validadores::validarNomeProjeto("aux", RaizTemp, true), "aux deveria falhar");
            Infra::exigir(!Validadores::validarNomeProjeto("NUL.txt", RaizTemp, true), "NUL.txt deveria falhar");
            Infra::exigir(!Validadores::validarNomeProjeto("COM1", RaizTemp, true), "COM1 deveria falhar");
            Infra::exigir(!Validadores::validarNomeProjeto("LPT9", RaizTemp, true), "LPT9 deveria falhar");
        });

        Infra::invocarTeste(Est, "analisarArgs trata sem args, --help, --version, -q e multiplos tokens", []() {
            const char* ARGS_VAZIO[]{"cpp-bootstrap"};
            Infra::exigir(
                CLI::Parser::analisarArgs(1, ARGS_VAZIO).AcaoEscolhida == CLI::Acao::EXIBIR_ERRO_TUTORIAL,
                "Sem args"
            );

            const char* ARGS_AJUDA[]{"cpp-bootstrap", "--help"};
            Infra::exigir(
                CLI::Parser::analisarArgs(2, ARGS_AJUDA).AcaoEscolhida == CLI::Acao::EXIBIR_AJUDA,
                "--help"
            );

            const char* ARGS_VERSAO[]{"cpp-bootstrap", "--version"};
            Infra::exigir(
                CLI::Parser::analisarArgs(2, ARGS_VERSAO).AcaoEscolhida == CLI::Acao::EXIBIR_VERSAO,
                "--version"
            );

            const char* ARGS_QUIET[]{"cpp-bootstrap", "-q", "MeuProj"};
            const CLI::Opcoes OP_QUIET{CLI::Parser::analisarArgs(3, ARGS_QUIET)};
            Infra::exigir(
                OP_QUIET.AcaoEscolhida == CLI::Acao::CRIAR_PROJETO &&
                OP_QUIET.Silencioso &&
                OP_QUIET.NomeProjeto == "MeuProj",
                "-q"
            );

            const char* ARGS_MULTI[]{"cpp-bootstrap", "Meu", "Projeto"};
            const CLI::Opcoes OP_MULTI{CLI::Parser::analisarArgs(3, ARGS_MULTI)};
            Infra::exigir(
                OP_MULTI.NomeProjeto == "Meu Projeto",
                "Multiplos tokens devem ser recombinados com espaco"
            );
        });
    }

}  // namespace Testes::Unitarios

#endif
