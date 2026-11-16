/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Declaracao da funcao de validacao multiplataforma de nomes de projeto.

*/

#pragma once

//============================ [ VALIDACAO DO NOME ] ===========================

#include <filesystem>
#include <string_view>

namespace bootstrap {

// Valida o nome do projeto segundo regras uniformes multiplataforma e CMake.
// Emite diagnosticos no terminal (quando silent == false) e retorna false em caso de erro.
[[nodiscard]] bool test_project_name(
    std::string_view name,
    const std::filesystem::path& base_dir,
    bool silent = false
);

}  // namespace bootstrap
