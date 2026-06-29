<#
.SYNOPSIS
Cria um novo projeto C++ com estrutura padrão.

.DESCRIPTION
Gera diretórios src/, include/, .vscode/
e configura CMake com C++23.

.PARAMETER ProjectName
Nome do projeto a ser criado.

.EXAMPLE
./cpp_bootstrap.ps1 MeuProjeto

.EXAMPLE
cpp-bootstrap MeuProjeto
#>

param (
    [string]$ProjectName
)

$ErrorActionPreference = "Stop";

# Usa as variaveis automaticas do PowerShell para detectar o sistema operacional.
# Nao atribua a $IsWindows, $IsLinux ou $IsMacOS; elas sao read-only.

#============================== [ TUTORIAL ] ===================================

if (-not $ProjectName) {
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    Write-Host "Uso:"

    if ($IsWindows) {
        Write-Host "  ./cpp_bootstrap.ps1 NomeDoProjeto"
    } else {
        Write-Host "  pwsh ./cpp_bootstrap.ps1 NomeDoProjeto"
    }

    Write-Host ""
    Write-Host "Dica: use apenas o nome real do projeto, sem < >."
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 1
}

#============================ [ PALETA / FUNÇÕES ] =============================

$RESET  = "`e[0m"
$GREEN  = "`e[32m"
$CYAN   = "`e[36m"
$BLUE   = "`e[34m"
$GRAY   = "`e[90m"
$YELLOW = "`e[33m"
$RED    = "`e[31m"

function Write-Info($msg)     { Write-Host "$CYAN$msg$RESET" }
function Write-Warn($msg)     { Write-Host "$YELLOW$msg$RESET" }
function Write-ErrorMsg($msg) { Write-Host "$RED$msg$RESET" }

#============================ [ VALIDAÇÃO DO NOME ] ============================

function validar_NomeProjeto {
    param([string]$ProjectName)
    $ProjectName = $ProjectName.Trim().Trim('"').Trim("'")
    $line = "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    
    
    # 1 - Encerra o programa caso NOME ESTEJA VAZIO.
    if ([string]::IsNullOrWhiteSpace($ProjectName)) {
        Write-ErrorMsg "❌ Nome do projeto vazio.";
        Write-Info "✅ Exemplo local: ./cpp_bootstrap.ps1 MeuProjeto";
        Write-Info "✅ Exemplo global: cpp-bootstrap MeuProjeto";

        return $false
    } 
    
    
    # 2 - Encerra o programa caso o nome tenha CARACTERES PROIBIDOS.
    if ($IsWindows){
        $badFound = $ProjectName.ToCharArray() | Where-Object { [System.IO.Path]::GetInvalidFileNameChars() -contains $_ }
        if ($badFound.Count -gt 0) {

            $badList = ($badFound | Select-Object -Unique) -join " "

            Write-Host $line;
            Write-ErrorMsg "❌ Nome inválido: '$ProjectName'";
            Write-Warn "Caracteres proibidos encontrados: $badList";
            Write-Info "Use apenas letras, números, '_' e '-' (recomendado).";
            Write-Host $line;
            
            return $false
        }
        
    } else {
        # Linux e macOS: evitar apenas / e null byte
        if($ProjectName -match '[/\x00]'){
            
            Write-Host $line;
            Write-ErrorMsg "❌ Nome inválido: '$ProjectName'";
            Write-Warn "Não pode conter: / ou caracteres nulos";
            Write-Host $line;
            
            return $false
        }
    }
    

    # [WINDOWS] Encerra o programa caso NOME TERMINAR COM PONTO ou ESPAÇO.
    if ($ProjectName -match '[\.\s]$') {

        Write-Host $line;
        Write-ErrorMsg "❌ Nome inválido: '$ProjectName'";
        Write-Warn "No Windows, nomes de pasta não podem terminar com '.' ou espaço.";
        Write-Host $line;

        return $false
    }
    
    # [WINDOWS] Encerra o programa caso NOME DO PROJETO tenha NOMES RESERVADOS ( Keywords do SO)
    $baseName = ($ProjectName.Split('.')[0]).ToUpperInvariant();
    $reserved = @("CON","PRN","AUX","NUL","COM1","COM2","COM3","COM4",
                  "COM5","COM6","COM7","COM8","COM9","LPT1","LPT2","LPT3",
                  "LPT4","LPT5","LPT6","LPT7","LPT8","LPT9");
    if ($reserved -contains $baseName) {

        Write-Host $line;
        Write-ErrorMsg "❌ Nome inválido: '$ProjectName'";
        Write-Warn "Esse é um nome reservado do Windows.";
        Write-Info "Escolha outro (ex: '${ProjectName}_proj').";
        Write-Host $line;

        return $false
    }
    
    # Encerra o programa caso JÁ EXISTA O MESMO NOME.
    if (Test-Path -LiteralPath $ProjectName -PathType Container) {
        Write-Host $line;
        Write-ErrorMsg "❌ Já existe uma pasta com o nome: '$ProjectName'";
        Write-Info "Sugestões:";
        Write-Info "  • Use outro nome: cpp-bootstrap ${ProjectName}_2";
        Write-Info "  • Ou renomeie/apague a pasta existente.";
        Write-Host $line;

        return $false
    }

    return $true 
}

# Executar validação
if (-not (validar_NomeProjeto $ProjectName)){
    exit 2
}

#============================ [ CRIAÇÃO DO PROJETO ] ============================
try {
    # Cria pasta do projeto e entra nela
    New-Item -ItemType Directory -Force -Path $ProjectName | Out-Null
    Set-Location -LiteralPath $ProjectName

    # Estrutura
    New-Item -ItemType Directory -Force -Path "src","include",".vscode" | Out-Null

    # .vscode/launch.json
    @'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug com g++ (MSYS2)",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": true, 
            "MIMode": "gdb",
            "miDebuggerPath": "C:/msys64/ucrt64/bin/gdb.exe",
            "setupCommands": [
                {
                    "description": "Habilitar impressão bonita para gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
'@ | Out-File ".vscode/launch.json" -Encoding utf8

    # .vscode/settings.json
    @'
{
    "cmake.exportCompileCommandsFile": true,
    "C_Cpp.intelliSenseEngine": "disabled",
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build",
        "--query-driver=C:/msys64/ucrt64/bin/g++.exe"
    ]
}
'@ | Out-File ".vscode/settings.json" -Encoding utf8

# src/main.cpp
    @"
/******************************************************************************
AUTOR:        <SEU_NOME>
DATA:         <DD/MM/AAAA>
NOTAS:

*******************************************************************************/

#include <iostream>
#include <clocale>

int main() {
    std::setlocale(LC_ALL, "pt-BR.UTF-8");

    
    std::cin.ignore();
}
"@ | Out-File src/main.cpp -Encoding utf8

# CMakeLists.txt
$cmake = @'
#═══════════════════════════════════════════════════════════════════════════════
# 1 - CONFIGURAÇÃO MÍNIMA DO CMAKE
# ------------------------------------------------------------------------------
# Define a versão mínima do CMake necessária para gerar o projeto.
# Garante compatibilidade com comandos e recursos usados neste arquivo.
cmake_minimum_required(VERSION 3.20)

#═══════════════════════════════════════════════════════════════════════════════
# 2 - DEFINIÇÃO DO PROJETO
# ------------------------------------------------------------------------------
# Nome do projeto e linguagem utilizada.
# O nome do executável final será o mesmo do projeto.
project(__PROJECT_NAME__ LANGUAGES CXX)

#═══════════════════════════════════════════════════════════════════════════════
# 3 - CONFIGURAÇÕES DA LINGUAGEM C++
# ------------------------------------------------------------------------------
# Define o padrão da linguagem C++ utilizado no projeto.
set(CMAKE_CXX_STANDARD 23)

# Obriga o compilador a respeitar exatamente o padrão definido acima.
# Se o compilador não suportar o padrão definido, a configuração falha.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#═══════════════════════════════════════════════════════════════════════════════
# 4 - TIPO DE BUILD
# ------------------------------------------------------------------------------
# Define o tipo de build padrão.
# "Debug" habilita símbolos de depuração e desativa otimizações.
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

#═══════════════════════════════════════════════════════════════════════════════
# 5 - CRIAÇÃO DO EXECUTÁVEL
# ------------------------------------------------------------------------------
# Cria o target do executável usando o nome do projeto (PROJECT_NAME).
add_executable(${PROJECT_NAME})

#═══════════════════════════════════════════════════════════════════════════════
# 6 - ARQUIVOS-FONTE (.cpp)
# ------------------------------------------------------------------------------
# Associa os arquivos de implementação (.cpp) ao executável.
# O uso de target_sources melhora a organização e facilita a manutenção.
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

#═══════════════════════════════════════════════════════════════════════════════
# 7 - HEADERS (.hpp)
# ------------------------------------------------------------------------------
# Informa ao compilador onde procurar arquivos de cabeçalho (.hpp).
# Boa prática: headers ficam separados na pasta "include/".
target_include_directories(${PROJECT_NAME} PRIVATE include)

#═══════════════════════════════════════════════════════════════════════════════
# 8 - FLAGS DE COMPILAÇÃO (QUALIDADE DE CÓDIGO)
# ------------------------------------------------------------------------------
# Conjunto de warnings (agressivos) para forçar boas práticas em C++.
# Ideal para aprendizado e para evitar comportamentos indefinidos.

if (MSVC)
    # Visual Studio / MSVC
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4            # warnings altos
        /permissive-   # modo mais "padrão"
        /EHsc          # exceções C++ padrão
    )

else()
    # GCC / Clang (MSYS2 g++, clang, Linux, etc.)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall                       # Avisos básicos do compilador
        -Wextra                     # Avisos adicionais (mais rigor)
        -Wconversion                # Alerta sobre conversões implícitas perigosas
        -Wsign-conversion           # Alerta sobre conversões signed/unsigned
        -pedantic-errors            # Segue o padrão C++ à risca (!!! HARDCORE !!!)
        -fdiagnostics-color=always  # Mensagens de erro coloridas no terminal
    )
endif()

'@ 
$cmake = $cmake -replace '__PROJECT_NAME__', $ProjectName
$cmake | Out-File CMakeLists.txt -Encoding utf8

# .gitignore
    @"
# Build e artefatos de compilacao
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

# IDEs e caches locais
.vscode/
.idea/
"@ | Out-File .gitignore -Encoding utf8

}
catch {
    Write-Host ""
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-ErrorMsg "❌ Falha ao criar o projeto '$ProjectName'"
    Write-Warn "Detalhe: $($_.Exception.Message)"
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    exit 10
}

#============================= [ MENSAGEM FINAL ] ==============================

Write-Host ""
Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host "✅ Projeto $GREEN'$ProjectName'$RESET criado com sucesso!"
Write-Host ""
Write-Host "$CYAN📌 Próximo passo:$RESET"
Write-Host "1 - Abra o projeto no VS Code com $BLUE code .$RESET"
Write-Host "2 - Compile com $BLUE cmake --build .$RESET"
Write-Host ""

Write-Host "$CYAN🖥️ Sistema:$RESET"
if ($IsWindows) { 
    Write-Host "   Windows"; 
}
elseif ($IsMacOS) { 
    Write-Host "   macOS"; 
}
else { 
    Write-Host "   Linux"; 
}

Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host ""

Write-Host "$CYAN👤 Autor:$RESET"
Write-Host "   Bruno Costa ($BLUE aka kbelx_$RESET )"
Write-Host ""

$githubUrl   = "https://github.com/kbelx_"
$linkedinUrl = "https://www.linkedin.com/in/brunocosta-dev/"

Write-Host "$CYAN🔗 Links:$RESET"

if ($Host.UI.SupportsVirtualTerminal) {
    Write-Host "   GitHub:   `e]8;;$githubUrl`e\$BLUE$githubUrl$RESET`e]8;;`e\"
    Write-Host "   LinkedIn: `e]8;;$linkedinUrl`e\$BLUE$linkedinUrl$RESET`e]8;;`e\"
} else {
    Write-Host "   GitHub:   $githubUrl"
    Write-Host "   LinkedIn: $linkedinUrl"
}

Write-Host ""
Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host ""
