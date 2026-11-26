/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Criacao transacional da estrutura do projeto com gravacao em UTF-8 sem BOM e LF.
*/

//============================ [ CABECALHOS PADRAO ] ===========================

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "generator.hpp"
#include "templates.hpp"

//=========================== [ SEGURANCA / RAII ] =============================

namespace Gerador::Seguranca {

    DirectoryRollbackGuard::DirectoryRollbackGuard(std::filesystem::path CaminhoRaizParam) noexcept
        : CaminhoRaiz{std::move(CaminhoRaizParam)} {}

    DirectoryRollbackGuard::~DirectoryRollbackGuard() noexcept {
        if (!Confirmado && !CaminhoRaiz.empty()) {
            std::error_code CodigoErro{};
            std::filesystem::remove_all(CaminhoRaiz, CodigoErro);
        }
    }

    void DirectoryRollbackGuard::confirmar() noexcept {
        Confirmado = true;
    }

    bool DirectoryRollbackGuard::foiConfirmado() const noexcept {
        return Confirmado;
    }

}  // namespace Gerador::Seguranca

//=========================== [ ESCRITA DE ARQUIVOS ] ==========================

namespace Gerador::Arquivo {

    [[nodiscard]] std::string normalizarQuebrasLinha(std::string_view Conteudo) {
        std::string Normalizado{};
        Normalizado.reserve(Conteudo.size() + 1);

        for (std::size_t Indice{0}; Indice < Conteudo.size(); ++Indice) {
            if (Conteudo[Indice] == '\r' &&
                (Indice + 1 < Conteudo.size()) &&
                Conteudo[Indice + 1] == '\n') {
                Normalizado.push_back('\n');
                ++Indice;
            }
            else {
                Normalizado.push_back(Conteudo[Indice]);
            }
        }

        if (Normalizado.empty() || Normalizado.back() != '\n') {
            Normalizado.push_back('\n');
        }
        return Normalizado;
    }

    void gravarUtf8SemBom(const std::filesystem::path& CaminhoArq, std::string_view Conteudo) {
        // Padroniza quebras de linha em LF e garante newline final (regra POSIX/C++)
        const std::string CONTEUDO_LF{normalizarQuebrasLinha(Conteudo)};

        // Modo binario impede que a CRT do Windows converta '\n' em "\r\n"
        std::ofstream FluxoSaida{CaminhoArq, std::ios::binary | std::ios::trunc};
        if (!FluxoSaida.is_open()) {
            throw std::runtime_error(
                "Nao foi possivel abrir o arquivo para escrita: " + CaminhoArq.string()
            );
        }

        FluxoSaida.write(CONTEUDO_LF.data(), static_cast<std::streamsize>(CONTEUDO_LF.size()));
        FluxoSaida.flush();
        if (!FluxoSaida.good()) {
            throw std::runtime_error(
                "Falha ao gravar dados no arquivo: " + CaminhoArq.string()
            );
        }
    }

}  // namespace Gerador::Arquivo

//=========================== [ GERACAO DO PROJETO ] ===========================

namespace Gerador::Projeto {

    void criarSubdiretorios(const std::filesystem::path& RaizProjeto) {
        std::filesystem::create_directory(RaizProjeto / "src");
        std::filesystem::create_directory(RaizProjeto / "include");
        std::filesystem::create_directory(RaizProjeto / ".vscode");
    }

    void gerarArquivosPadrao(const std::filesystem::path& RaizProjeto, std::string_view NomeProjeto) {
        const std::filesystem::path DIR_SRC{RaizProjeto / "src"};
        const std::filesystem::path DIR_VSCODE{RaizProjeto / ".vscode"};

        Arquivo::gravarUtf8SemBom(DIR_VSCODE / "launch.json",    Templates::VSCode::LAUNCH_JSON);
        Arquivo::gravarUtf8SemBom(DIR_VSCODE / "settings.json",  Templates::VSCode::SETTINGS_JSON);
        Arquivo::gravarUtf8SemBom(DIR_SRC /    "main.cpp",       Templates::Projeto::MAIN_CPP);
        Arquivo::gravarUtf8SemBom(
            RaizProjeto / "CMakeLists.txt",
            Templates::Render::renderizarCMake(NomeProjeto)
        );
        Arquivo::gravarUtf8SemBom(RaizProjeto / ".gitignore",    Templates::Projeto::GITIGNORE);
    }

    bool criarProjeto(const std::filesystem::path& DirBase, std::string_view NomeProjeto, std::string* ErroSaida) {
        try {
            const std::filesystem::path RAIZ_PROJETO{DirBase / std::filesystem::path{NomeProjeto}};

            // Cria a pasta raiz primeiro e arma o rollback RAII imediatamente
            if (!std::filesystem::create_directory(RAIZ_PROJETO)) {
                if (ErroSaida != nullptr) {
                    *ErroSaida = "Nao foi possivel criar o diretorio raiz: " + RAIZ_PROJETO.string();
                }
                return false;
            }
            Seguranca::DirectoryRollbackGuard GuardaRollback{RAIZ_PROJETO};

            criarSubdiretorios(RAIZ_PROJETO);
            gerarArquivosPadrao(RAIZ_PROJETO, NomeProjeto);

            // Tudo gravado com sucesso: desarma o rollback
            GuardaRollback.confirmar();
            return true;
        }
        catch (const std::exception& Excecao) {
            if (ErroSaida != nullptr) {
                *ErroSaida = Excecao.what();
            }
            return false;
        }
    }

}  // namespace Gerador::Projeto
