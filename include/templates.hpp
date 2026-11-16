/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Templates embutidos em RAW STRING LITERALS e renderizacao literal do CMakeLists.txt
*/

#ifndef TEMPLATE_H
#define TEMPLATE_H

    #include <string>
    #include <string_view>

    namespace bootstrap::tpl {

    // Base para o arquivo ".vscode/launch.json"
    inline constexpr std::string_view kLaunchJson = R"tpl({
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug C++ (CMake)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "windows": {
                "MIMode": "gdb",
                "miDebuggerPath": "C:/msys64/ucrt64/bin/gdb.exe",
                "externalConsole": true
            },
            "linux": {
                "MIMode": "gdb",
                "miDebuggerPath": "/usr/bin/gdb",
                "externalConsole": false
            },
            "osx": {
                "MIMode": "lldb",
                "externalConsole": false
            }
        }
    ]
}
)tpl";


    // Base para o arquivo ".vscode/settings.json"
    inline constexpr std::string_view kSettingsJson = R"tpl({
    "cmake.exportCompileCommandsFile": true,
    "C_Cpp.intelliSenseEngine": "disabled",
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build",
        "--query-driver=C:/msys64/*/bin/*g++.exe,/usr/bin/*g++*,/usr/bin/*clang++*,/usr/local/bin/*,/opt/homebrew/bin/*"
    ]
}
)tpl";


    // Base para o arquivo "main.cpp"
    inline constexpr std::string_view kMainCpp = R"tpl(/******************************************************************************
AUTOR:        <SEU_NOME>
DATA:         <DD/MM/AAAA>
NOTAS:

*******************************************************************************/

#include <clocale>
#include <iostream>

int main() {
    std::setlocale(LC_ALL, "");

    std::cin.ignore();
    return 0;
}
)tpl";


    // Base para o arquivo "CMakeLists.txt" com essas configuracoes
    inline constexpr std::string_view kCmakeLists = R"tpl(#===============================================================================
# 1 - CONFIGURACAO MINIMA DO CMAKE
# ------------------------------------------------------------------------------
# Define a versao minima do CMake necessaria para gerar o projeto.
# Garante compatibilidade com comandos e recursos usados neste arquivo.
cmake_minimum_required(VERSION 3.20)

#===============================================================================
# 2 - DEFINICAO DO PROJETO
# ------------------------------------------------------------------------------
# Nome do projeto e linguagem utilizada.
# O nome do executavel final sera o mesmo do projeto.
project(__PROJECT_NAME__ LANGUAGES CXX)

#===============================================================================
# 3 - CONFIGURACOES DA LINGUAGEM C++
# ------------------------------------------------------------------------------
# Define o padrao C++23 e exige suporte estrito do compilador.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#===============================================================================
# 4 - TIPO DE BUILD
# ------------------------------------------------------------------------------
# Define "Debug" como padrao apenas em geradores single-config quando o usuario
# nao informou -DCMAKE_BUILD_TYPE=... explicitamente na linha de comando.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Tipo de build padrao" FORCE)
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

#===============================================================================
# 5 - CRIACAO DO EXECUTAVEL
# ------------------------------------------------------------------------------
# Cria o target do executavel usando o nome do projeto (PROJECT_NAME).
add_executable(${PROJECT_NAME})

#===============================================================================
# 6 - FONTES (.cpp)
# ------------------------------------------------------------------------------
# Associa os arquivos de implementacao (.cpp) ao executavel.
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

#===============================================================================
# 7 - HEADERS (.hpp)
# ------------------------------------------------------------------------------
# Informa ao compilador onde procurar arquivos de cabecalho (.hpp / .h).
target_include_directories(${PROJECT_NAME} PRIVATE include)

#===============================================================================
# 8 - FLAGS DE COMPILACAO (QUALIDADE DE CODIGO)
# ------------------------------------------------------------------------------
# Conjunto rigoroso de avisos para incentivar boas praticas em C++23.
if(MSVC)
    # Visual Studio / MSVC
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4            # Nivel alto de avisos
        /permissive-   # Conformidade estrita com o padrao ISO C++
        /EHsc          # Modelo padrao de excecoes C++
    )
else()
    # GCC / Clang (MSYS2, Linux, macOS)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall                       # Avisos basicos do compilador
        -Wextra                     # Avisos adicionais
        -Wconversion                # Alerta sobre conversoes implicitas perigosas
        -Wsign-conversion           # Alerta sobre conversoes signed/unsigned
        -pedantic-errors            # Exige conformidade estrita com o padrao ISO C++
        -fdiagnostics-color=always  # Mensagens coloridas no terminal
    )
endif()

#===============================================================================
# 9 - SUPORTE COMPLETO A BIBLIOTECA PADRAO C++23 (libstdc++exp)
# ------------------------------------------------------------------------------
# Em toolchains baseadas na libstdc++ (GCC e Clang no MSYS2/Linux), recursos
# recentes do C++23 como <print> e <stacktrace> residem em libstdc++exp.a.
if(NOT MSVC AND NOT APPLE)
    include(CheckLinkerFlag)
    check_linker_flag(CXX "-lstdc++exp" HAVE_STDCXXEXP)
    if(HAVE_STDCXXEXP)
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
    endif()
endif()
)tpl";


    // Base para o arquivo ".gitignore"
    inline constexpr std::string_view kGitignore = R"tpl(# Build e artefatos de compilacao
build/
cmake-build-*/
out/
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
compile_commands.json
Makefile

# Binarios e bibliotecas geradas
*.exe
*.out
*.dll
*.so
*.dylib
*.a
*.lib
*.o
*.obj
*.pdb
*.ilk

# IDEs e caches locais (mantem configuracoes compartilhadas do VS Code)
.vscode/*
!.vscode/launch.json
!.vscode/settings.json
.idea/
)tpl";


    // Substitui literalmente todas as ocorrencias de __PROJECT_NAME__ no template CMake.
    [[nodiscard]] inline std::string render_cmake_lists(std::string_view project_name) {
        constexpr std::string_view kPlaceholder = "__PROJECT_NAME__";
        std::string result;
        result.reserve(kCmakeLists.size() + project_name.size());

        std::size_t pos = 0;
        while (pos < kCmakeLists.size()) {
            const std::size_t found = kCmakeLists.find(kPlaceholder, pos);
            if (found == std::string_view::npos) {
                result.append(kCmakeLists.substr(pos));
                break;
            }
            result.append(kCmakeLists.substr(pos, found - pos));
            result.append(project_name);
            pos = found + kPlaceholder.size();
        }

        return result;
    }

    }  // namespace bootstrap::tpl

#endif
