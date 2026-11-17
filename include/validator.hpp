/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Declaracao da funcao de validacao multiplataforma de nomes de projeto.
*/

#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <filesystem>
#include <string_view>

//=========================== [ VALIDACAO DE NOMES ] ===========================

namespace Validadores {

    // Valida o nome do projeto segundo regras uniformes multiplataforma e CMake.
    // Emite diagnosticos no terminal (quando Silencioso == false) e retorna false em caso de erro.
    [[nodiscard]] bool validarNomeProjeto(
        std::string_view Nome,
        const std::filesystem::path& DirBase,
        bool Silencioso = false
    );

}  // namespace Validadores

#endif
