/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Testes de integracao da CLI, codigos de saida e rollback RAII (Testes 08 a 12).
*/

#ifndef TEST_CLI_INTEGRATION_HPP
#define TEST_CLI_INTEGRATION_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "cli.hpp"
#include "generator.hpp"
#include "test_harness.hpp"
#include "test_process.hpp"

//======================== [ INTEGRACAO CLI / SEGURANCA ] ======================

namespace Testes::IntegracaoCli {

    inline void executar(
        Infra::Estatisticas& Est,
        const std::filesystem::path& ExecAlvo,
        const std::filesystem::path& RaizTemp
    ) {
        Infra::imprimirCabecalhoSecao("2. Integracao CLI, Codigos de Saida e Seguranca");

        Infra::invocarTeste(Est, "Execucao sem argumentos exibe tutorial e encerra com codigo 1", [&]() {
            const Processo::ResultadoProcesso RES{Processo::invocarBootstrap(ExecAlvo, {}, RaizTemp)};
            Infra::exigir(
                RES.CodigoSaida == 1,
                "Esperado exit code 1, obtido " + std::to_string(RES.CodigoSaida)
            );
            Infra::exigir(
                Infra::contem(RES.TextoStdout, "Uso:") && Infra::contem(RES.TextoStdout, "Dica:"),
                "Tutorial ausente"
            );
        });

        Infra::invocarTeste(Est, "Flags --help e --version encerram com codigo 0", [&]() {
            const Processo::ResultadoProcesso RES_AJUDA{
                Processo::invocarBootstrap(ExecAlvo, {"--help"}, RaizTemp)
            };
            Infra::exigir(RES_AJUDA.CodigoSaida == 0, "--help deveria retornar 0");
            Infra::exigir(
                Infra::contem(RES_AJUDA.TextoStdout, "cpp-bootstrap"),
                "--help sem nome do programa"
            );

            const Processo::ResultadoProcesso RES_VERSAO{
                Processo::invocarBootstrap(ExecAlvo, {"--version"}, RaizTemp)
            };
            Infra::exigir(RES_VERSAO.CodigoSaida == 0, "--version deveria retornar 0");
            Infra::exigir(
                Infra::contem(RES_VERSAO.TextoStdout, std::string{CLI::VERSAO}),
                "--version sem versao"
            );
        });

        Infra::invocarTeste(Est, "Entradas invalidas na CLI sao rejeitadas com codigo 2", [&]() {
            const Processo::ResultadoProcesso RES_VAZIO{Processo::invocarBootstrap(ExecAlvo, {""}, RaizTemp)};
            Infra::exigir(
                RES_VAZIO.CodigoSaida == 2 &&
                Infra::contem(RES_VAZIO.TextoStdout, "Nome do projeto vazio"),
                "Entrada vazia"
            );

            const Processo::ResultadoProcesso RES_MAIOR_MENOR{
                Processo::invocarBootstrap(ExecAlvo, {"Projeto<Invalido>"}, RaizTemp)
            };
            Infra::exigir(
                RES_MAIOR_MENOR.CodigoSaida == 2 &&
                Infra::contem(RES_MAIOR_MENOR.TextoStdout, "Caracteres proibidos encontrados"),
                "< >"
            );

            const Processo::ResultadoProcesso RES_ESPACO{
                Processo::invocarBootstrap(ExecAlvo, {"Meu Projeto"}, RaizTemp)
            };
            Infra::exigir(
                RES_ESPACO.CodigoSaida == 2 && Infra::contem(RES_ESPACO.TextoStdout, "<espaco>"),
                "Espaco entre aspas"
            );

            const Processo::ResultadoProcesso RES_MULTI{
                Processo::invocarBootstrap(ExecAlvo, {"Meu", "Projeto"}, RaizTemp)
            };
            Infra::exigir(
                RES_MULTI.CodigoSaida == 2 && Infra::contem(RES_MULTI.TextoStdout, "<espaco>"),
                "Tokens multiplos"
            );

            const Processo::ResultadoProcesso RES_PONTO{
                Processo::invocarBootstrap(ExecAlvo, {"ProjetoComPonto."}, RaizTemp)
            };
            Infra::exigir(
                RES_PONTO.CodigoSaida == 2 &&
                Infra::contem(RES_PONTO.TextoStdout, "nao podem terminar com '.'"),
                "Ponto final"
            );

            const Processo::ResultadoProcesso RES_CON{Processo::invocarBootstrap(ExecAlvo, {"CON"}, RaizTemp)};
            Infra::exigir(
                RES_CON.CodigoSaida == 2 &&
                Infra::contem(RES_CON.TextoStdout, "nome reservado do Windows"),
                "CON"
            );

            const Processo::ResultadoProcesso RES_FLAG{
                Processo::invocarBootstrap(ExecAlvo, {"--opcao-invalida"}, RaizTemp)
            };
            Infra::exigir(
                RES_FLAG.CodigoSaida == 2 && Infra::contem(RES_FLAG.TextoStdout, "Opcao desconhecida"),
                "Flag desconhecida"
            );
        });

        Infra::invocarTeste(Est, "Colisao com pasta ou arquivo existente bloqueia sobrescrita (codigo 2)", [&]() {
            const std::filesystem::path DIR_EXISTENTE{RaizTemp / "ProjetoPreExistente"};
            std::filesystem::create_directories(DIR_EXISTENTE);
            const std::filesystem::path SENTINELA{DIR_EXISTENTE / "sentinela.txt"};
            Gerador::Arquivo::gravarUtf8SemBom(SENTINELA, "intacto");

            const Processo::ResultadoProcesso RES_DIR{
                Processo::invocarBootstrap(ExecAlvo, {"ProjetoPreExistente"}, RaizTemp)
            };
            Infra::exigir(
                RES_DIR.CodigoSaida == 2 &&
                Infra::contem(RES_DIR.TextoStdout, "Ja existe uma pasta ou arquivo"),
                "Pasta"
            );
            Infra::exigir(Arquivos::lerTodoTexto(SENTINELA) == "intacto\n", "Sentinela modificado");

            const std::filesystem::path ARQ_EXISTENTE{RaizTemp / "ArquivoComMesmoNome"};
            Gerador::Arquivo::gravarUtf8SemBom(ARQ_EXISTENTE, "arquivo");
            const Processo::ResultadoProcesso RES_ARQ{
                Processo::invocarBootstrap(ExecAlvo, {"ArquivoComMesmoNome"}, RaizTemp)
            };
            Infra::exigir(RES_ARQ.CodigoSaida == 2, "Arquivo existente deveria retornar 2");
        });

        Infra::invocarTeste(Est, "DirectoryRollbackGuard remove pasta nao comitada e preserva apos confirmar()", [&]() {
            const std::filesystem::path DIR_ROLLBACK{RaizTemp / "RollbackTemp"};
            {
                std::filesystem::create_directories(DIR_ROLLBACK / "src");
                Gerador::Seguranca::DirectoryRollbackGuard Guarda{DIR_ROLLBACK};
            }
            Infra::exigir(
                !std::filesystem::exists(DIR_ROLLBACK),
                "Diretorio deveria ter sido removido pelo rollback RAII"
            );

            const std::filesystem::path DIR_CONFIRMADO{RaizTemp / "CommittedTemp"};
            {
                std::filesystem::create_directories(DIR_CONFIRMADO);
                Gerador::Seguranca::DirectoryRollbackGuard Guarda{DIR_CONFIRMADO};
                Guarda.confirmar();
                Infra::exigir(Guarda.foiConfirmado(), "Diretorio deveria constar como confirmado");
            }
            Infra::exigir(
                std::filesystem::exists(DIR_CONFIRMADO),
                "Diretorio confirmado deveria permanecer intacto"
            );
        });
    }

}  // namespace Testes::IntegracaoCli

#endif
