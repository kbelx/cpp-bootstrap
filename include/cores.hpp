/*
AUTOR: Bruno Costa
DATA: 25/11/2026
PROPOSITO: Constantes de sequencias ANSI para paleta de cores e estilos no terminal.
*/

#ifndef CORES_HPP
#define CORES_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <string_view>

//============================ [ PALETA DE CORES ] =============================

namespace Cores {

    inline constexpr std::string_view ESC      {"\x1b"};
    inline constexpr std::string_view RESET    {"\x1b[0m"};
    inline constexpr std::string_view VERDE    {"\x1b[32m"};
    inline constexpr std::string_view CIANO    {"\x1b[36m"};
    inline constexpr std::string_view AZUL     {"\x1b[34m"};
    inline constexpr std::string_view CINZA    {"\x1b[90m"};
    inline constexpr std::string_view AMARELO  {"\x1b[33m"};
    inline constexpr std::string_view VERMELHO {"\x1b[31m"};
    inline constexpr std::string_view NEGRITO  {"\x1b[1m"};

}  // namespace Cores

#endif
