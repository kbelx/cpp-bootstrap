/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Declaracao do rollback RAII (DirectoryRollbackGuard), escrita UTF-8 sem BOM e geracao do projeto.
*/

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string>
#include <string_view>

//=========================== [ SEGURANCA / RAII ] =============================

namespace Gerador::Seguranca {

    // Guarda RAII que remove recursivamente a pasta raiz criada caso ocorra falha antes de confirmar().
    class DirectoryRollbackGuard {
        public:
            explicit DirectoryRollbackGuard(std::filesystem::path CaminhoRaizParam) noexcept;
            ~DirectoryRollbackGuard() noexcept;

            DirectoryRollbackGuard(const DirectoryRollbackGuard&) = delete;
            DirectoryRollbackGuard& operator=(const DirectoryRollbackGuard&) = delete;
            DirectoryRollbackGuard(DirectoryRollbackGuard&&) = delete;
            DirectoryRollbackGuard& operator=(DirectoryRollbackGuard&&) = delete;

            void confirmar() noexcept;
            [[nodiscard]] bool foiConfirmado() const noexcept;

        private:
            std::filesystem::path CaminhoRaiz{};
            bool Confirmado{false};
    };

}  // namespace Gerador::Seguranca

//=========================== [ ESCRITA DE ARQUIVOS ] ==========================

namespace Gerador::Arquivo {

    // Grava arquivo estritamente em UTF-8 sem BOM com quebras de linha LF e newline final.
    void gravarUtf8SemBom(const std::filesystem::path& CaminhoArq, std::string_view Conteudo);

}  // namespace Gerador::Arquivo

//=========================== [ GERACAO DO PROJETO ] ===========================

namespace Gerador::Projeto {

    // Gera toda a estrutura do projeto sem alterar o diretorio de trabalho (com rollback automatico).
    [[nodiscard]] bool criarProjeto(
        const std::filesystem::path& DirBase,
        std::string_view NomeProjeto,
        std::string* ErroSaida = nullptr
    );

}  // namespace Gerador::Projeto

namespace Gerador {

    using Seguranca::DirectoryRollbackGuard;

}  // namespace Gerador

#endif
