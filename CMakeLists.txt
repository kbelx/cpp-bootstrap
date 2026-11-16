#===============================================================================
# 1 - CONFIGURACAO MINIMA DO CMAKE
# ------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)

#===============================================================================
# 2 - DEFINICAO DO PROJETO
# ------------------------------------------------------------------------------
project(cpp-bootstrap VERSION 1.0.0 LANGUAGES CXX)

#===============================================================================
# 3 - CONFIGURACOES DA LINGUAGEM C++
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#===============================================================================
# 4 - TIPO DE BUILD E OPCOES
# ------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Tipo de build padrao" FORCE)
endif()

option(
    CPP_BOOTSTRAP_STATIC_RUNTIME
    "Vincula o runtime C++ estaticamente em builds MinGW para distribuicao standalone"
    OFF
)

#===============================================================================
# 5 - NUCLEO COMPARTILHADO (CLI, VALIDADOR, GERADOR)
# ------------------------------------------------------------------------------
add_library(cpp_bootstrap_core OBJECT
    src/cli.cpp
    src/validator.cpp
    src/generator.cpp
)

#===============================================================================
# 6 - EXECUTAVEL PRINCIPAL
# ------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE cpp_bootstrap_core)

if(MINGW AND CPP_BOOTSTRAP_STATIC_RUNTIME)
    target_link_options(${PROJECT_NAME} PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()

#===============================================================================
# 7 - HEADERS (.hpp)
# ------------------------------------------------------------------------------
target_include_directories(cpp_bootstrap_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

#===============================================================================
# 8 - FLAGS DE COMPILACAO (QUALIDADE DE CODIGO)
# ------------------------------------------------------------------------------
if(MSVC)
    target_compile_options(cpp_bootstrap_core PUBLIC
        /W4
        /permissive-
        /EHsc
    )
else()
    target_compile_options(cpp_bootstrap_core PUBLIC
        -Wall
        -Wextra
        -Wconversion
        -Wsign-conversion
        -pedantic-errors
        -fdiagnostics-color=always
    )
endif()

#===============================================================================
# 9 - SUPORTE COMPLETO A BIBLIOTECA PADRAO C++23 (libstdc++exp) E TESTES
# ------------------------------------------------------------------------------
if(NOT MSVC AND NOT APPLE)
    include(CheckLinkerFlag)
    check_linker_flag(CXX "-lstdc++exp" HAVE_STDCXXEXP)
    if(HAVE_STDCXXEXP)
        target_link_libraries(cpp_bootstrap_core PUBLIC stdc++exp)
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(test_cpp_bootstrap tests/test_cpp_bootstrap.cpp)
    target_link_libraries(test_cpp_bootstrap PRIVATE cpp_bootstrap_core)

    add_test(
        NAME cpp_bootstrap_suite
        COMMAND test_cpp_bootstrap --target-exe "$<TARGET_FILE:cpp-bootstrap>"
    )
endif()
