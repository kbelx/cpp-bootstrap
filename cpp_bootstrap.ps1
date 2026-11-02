<#
.SYNOPSIS
Cria um novo projeto C++23 com estrutura padrao e CMake.

.DESCRIPTION
Gera os diretorios src/, include/ e .vscode/, alem dos arquivos
main.cpp, CMakeLists.txt (C++23 estrito), launch.json, settings.json e .gitignore.
Compativel com PowerShell 7+ (Windows, Linux e macOS) e Windows PowerShell 5.1,
mantendo todos os arquivos estritamente em UTF-8 sem BOM.

.PARAMETER ProjectName
Nome do projeto a ser criado (apenas letras, numeros, '_' e '-', sem iniciar por '-').

.PARAMETER Quiet
Opcional. Suprime o rodape de autoria e links ao final da execucao.

.EXAMPLE
./cpp_bootstrap.ps1 MeuProjeto

.EXAMPLE
pwsh ./cpp_bootstrap.ps1 MeuProjeto
#>

param (
    [Parameter(Position = 0)]
    [AllowEmptyString()]
    [string]$ProjectName,

    [switch]$Quiet,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs
)

$ErrorActionPreference = "Stop"

#========================== [ DETECCAO DE AMBIENTE ] ===========================
# Nao atribui a $IsWindows, $IsLinux ou $IsMacOS (sao read-only no PowerShell 6+).
# Compativel com Set-StrictMode no Windows PowerShell 5.1 e PowerShell 7+.

$isWin = ($PSVersionTable.PSEdition -eq "Desktop") -or
         ($env:OS -eq "Windows_NT") -or
         ((Test-Path Variable:\IsWindows) -and [bool](Get-Variable -Name IsWindows -ValueOnly))
$isMac = (-not $isWin) -and
         ((Test-Path Variable:\IsMacOS) -and [bool](Get-Variable -Name IsMacOS -ValueOnly))

$isRedirected = $false
try {
    $isRedirected = [Console]::IsOutputRedirected
}
catch {
    $isRedirected = $true
}

$hasVtProp = ($null -ne $Host.UI) -and ($null -ne $Host.UI.PSObject.Properties["SupportsVirtualTerminal"])
$useAnsi   = $hasVtProp -and
             [bool]$Host.UI.SupportsVirtualTerminal -and
             (-not $isRedirected) -and
             ([string]::IsNullOrEmpty($env:NO_COLOR))

#============================ [ PALETA / FUNCOES ] =============================

$ESC    = if ($useAnsi) { [string][char]27 } else { "" }
$RESET  = if ($useAnsi) { "$ESC[0m" }  else { "" }
$GREEN  = if ($useAnsi) { "$ESC[32m" } else { "" }
$CYAN   = if ($useAnsi) { "$ESC[36m" } else { "" }
$BLUE   = if ($useAnsi) { "$ESC[34m" } else { "" }
$GRAY   = if ($useAnsi) { "$ESC[90m" } else { "" }
$YELLOW = if ($useAnsi) { "$ESC[33m" } else { "" }
$RED    = if ($useAnsi) { "$ESC[31m" } else { "" }

$separator = "$GRAY----------------------------------------------------$RESET"

function Write-Info {
    param([string]$Message)
    Write-Host "$CYAN$Message$RESET"
}

function Write-Warn {
    param([string]$Message)
    Write-Host "$YELLOW$Message$RESET"
}

function Write-ErrorMsg {
    param([string]$Message)
    Write-Host "$RED$Message$RESET"
}

function Write-Utf8NoBom {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Content
    )
    # Padroniza quebras de linha em LF e garante newline final (regra POSIX/C++)
    $normalized = $Content -replace "`r`n", "`n"
    if (-not $normalized.EndsWith("`n")) {
        $normalized += "`n"
    }
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $normalized, $utf8NoBom)
}

#============================== [ TUTORIAL ] ===================================

if ((-not $PSBoundParameters.ContainsKey("ProjectName")) -and ($null -eq $ExtraArgs -or $ExtraArgs.Count -eq 0)) {
    Write-Host $separator
    Write-Host "Uso:"

    if ($isWin) {
        Write-Host "  ./cpp_bootstrap.ps1 NomeDoProjeto"
    }
    else {
        Write-Host "  pwsh ./cpp_bootstrap.ps1 NomeDoProjeto"
    }

    Write-Host ""
    Write-Host "Dica: use apenas o nome real do projeto, sem < > ou espacos."
    Write-Host $separator
    exit 1
}

# Se o usuario passou multiplos tokens sem aspas (ex: ./cpp_bootstrap.ps1 Meu Projeto),
# recompoe a entrada bruta para que a validacao rejeite espacos internos explicitamente.
if ($null -ne $ExtraArgs -and $ExtraArgs.Count -gt 0) {
    $parts = @()
    if ($PSBoundParameters.ContainsKey("ProjectName")) {
        $parts += $ProjectName
    }
    $parts += $ExtraArgs
    $ProjectName = $parts -join " "
}

# Normaliza espacos externos e aspas envolventes uma unica vez para todo o script
$ProjectName = $ProjectName.Trim().Trim('"').Trim("'").Trim()
$callerDir   = (Get-Location).ProviderPath

#============================ [ VALIDACAO DO NOME ] ============================

function Test-ProjectName {
    param(
        [AllowEmptyString()]
        [string]$Name,
        [string]$BaseDirectory
    )

    # 1 - Nome vazio ou apenas espacos
    if ([string]::IsNullOrWhiteSpace($Name)) {
        Write-ErrorMsg "[ERRO] Nome do projeto vazio."
        Write-Info "[INFO] Exemplo (Windows): ./cpp_bootstrap.ps1 MeuProjeto"
        Write-Info "[INFO] Exemplo (Linux/macOS): pwsh ./cpp_bootstrap.ps1 MeuProjeto"
        return $false
    }

    # 2 - Nome terminando com ponto (invalido no Windows e bloqueado para portabilidade Git)
    if ($Name.EndsWith(".")) {
        Write-Host $separator
        Write-ErrorMsg "[ERRO] Nome invalido: '$Name'"
        Write-Warn "Nomes de pasta nao podem terminar com '.' ou espaco."
        Write-Host $separator
        return $false
    }

    # 3 - Nomes de dispositivos reservados do Windows (bloqueados em todos os SOs para portabilidade)
    $baseName = ($Name.Split(".")[0]).ToUpperInvariant()
    $reserved = @(
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    )
    if ($reserved -contains $baseName) {
        Write-Host $separator
        Write-ErrorMsg "[ERRO] Nome invalido: '$Name'"
        Write-Warn "Esse e um nome reservado do Windows."
        Write-Info "Escolha outro (ex: '${Name}_proj')."
        Write-Host $separator
        return $false
    }

    # 4 - Regra uniforme para CMake e filesystems multiplataforma:
    #     apenas letras ASCII, digitos, '_' e '-', sem iniciar com '-'
    if ($Name -notmatch "^[A-Za-z0-9_][A-Za-z0-9_-]*$") {
        $badChars = $Name.ToCharArray() | Where-Object { $_ -notmatch "[A-Za-z0-9_-]" } | Select-Object -Unique
        $badDisplay = if ($null -ne $badChars -and @($badChars).Count -gt 0) {
            (@($badChars) | ForEach-Object { if ($_ -eq " ") { "<espaco>" } else { "$_" } }) -join " "
        }
        else {
            "inicio com '-'"
        }

        Write-Host $separator
        Write-ErrorMsg "[ERRO] Nome invalido: '$Name'"
        Write-Warn "Caracteres proibidos encontrados: $badDisplay"
        Write-Info "Use apenas letras, numeros, '_' e '-' (sem espacos e sem iniciar com '-')."
        Write-Host $separator
        return $false
    }

    # 5 - Prevencao contra sobrescrita de pasta ou arquivo ja existente
    $candidatePath = Join-Path -Path $BaseDirectory -ChildPath $Name
    if (Test-Path -LiteralPath $candidatePath) {
        Write-Host $separator
        Write-ErrorMsg "[ERRO] Ja existe uma pasta ou arquivo com o nome: '$Name'"
        Write-Info "Sugestoes:"
        Write-Info "  * Use outro nome: ./cpp_bootstrap.ps1 ${Name}_2"
        Write-Info "  * Ou renomeie/apague o item existente."
        Write-Host $separator
        return $false
    }

    return $true
}

if (-not (Test-ProjectName -Name $ProjectName -BaseDirectory $callerDir)) {
    exit 2
}

#============================ [ CRIACAO DO PROJETO ] ===========================

$projectRoot = Join-Path -Path $callerDir -ChildPath $ProjectName
$createdRoot = $false

try {
    # Cria diretorios sem usar Set-Location (nao altera o diretorio do terminal do usuario)
    New-Item -ItemType Directory -Path $projectRoot -ErrorAction Stop | Out-Null
    $createdRoot = $true

    $srcDir     = Join-Path -Path $projectRoot -ChildPath "src"
    $includeDir = Join-Path -Path $projectRoot -ChildPath "include"
    $vscodeDir  = Join-Path -Path $projectRoot -ChildPath ".vscode"

    New-Item -ItemType Directory -Path $srcDir, $includeDir, $vscodeDir -ErrorAction Stop | Out-Null

    # .vscode/launch.json (configuracao multiplataforma Windows / Linux / macOS)
    $launchJsonContent = @'
{
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
'@
    Write-Utf8NoBom -Path (Join-Path -Path $vscodeDir -ChildPath "launch.json") -Content $launchJsonContent

    # .vscode/settings.json (clangd + compile_commands.json multiplataforma)
    $settingsJsonContent = @'
{
    "cmake.exportCompileCommandsFile": true,
    "C_Cpp.intelliSenseEngine": "disabled",
    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build",
        "--query-driver=C:/msys64/*/bin/*g++.exe,/usr/bin/*g++*,/usr/bin/*clang++*,/usr/local/bin/*,/opt/homebrew/bin/*"
    ]
}
'@
    Write-Utf8NoBom -Path (Join-Path -Path $vscodeDir -ChildPath "settings.json") -Content $settingsJsonContent

    # src/main.cpp (here-string literal + locale portavel entre Windows, Linux e macOS)
    $mainCppContent = @'
/******************************************************************************
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
'@
    Write-Utf8NoBom -Path (Join-Path -Path $srcDir -ChildPath "main.cpp") -Content $mainCppContent

    # CMakeLists.txt (substituicao literal segura para o nome do projeto)
    $cmakeTemplate = @'
#===============================================================================
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
'@
    $cmakeContent = $cmakeTemplate.Replace("__PROJECT_NAME__", $ProjectName)
    Write-Utf8NoBom -Path (Join-Path -Path $projectRoot -ChildPath "CMakeLists.txt") -Content $cmakeContent

    # .gitignore (ignora caches do .vscode mas preserva launch.json e settings.json)
    $gitignoreContent = @'
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

# IDEs e caches locais (mantem configuracoes compartilhadas do VS Code)
.vscode/*
!.vscode/launch.json
!.vscode/settings.json
.idea/
'@
    Write-Utf8NoBom -Path (Join-Path -Path $projectRoot -ChildPath ".gitignore") -Content $gitignoreContent
}
catch {
    # Rollback transacional: remove apenas a pasta criada nesta execucao
    if ($createdRoot -and (Test-Path -LiteralPath $projectRoot)) {
        try {
            Remove-Item -LiteralPath $projectRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
        catch {
            # Ignora falha secundaria de limpeza para exibir o erro principal
        }
    }

    Write-Host ""
    Write-Host $separator
    Write-ErrorMsg "[ERRO] Falha ao criar o projeto '$ProjectName'"
    Write-Warn "Detalhe: $($_.Exception.Message)"
    Write-Host $separator
    exit 10
}

#============================= [ MENSAGEM FINAL ] ==============================

Write-Host ""
Write-Host $separator
Write-Host "[OK] Projeto $GREEN'$ProjectName'$RESET criado com sucesso!"
Write-Host ""
Write-Host "${CYAN}Proximos passos:$RESET"
Write-Host "  1 - Entre na pasta:   $BLUE cd $ProjectName$RESET"
Write-Host "  2 - Abra no VS Code:  $BLUE code .$RESET"
Write-Host "  3 - Configure CMake:  $BLUE cmake -B build$RESET"
Write-Host "  4 - Compile:          $BLUE cmake --build build$RESET"
Write-Host ""

$osLabel = if ($isWin) { "Windows" } elseif ($isMac) { "macOS" } else { "Linux" }
Write-Host "${CYAN}Sistema detectado:$RESET $osLabel"

if (-not $Quiet) {
    Write-Host $separator
    Write-Host "${CYAN}Autor:$RESET Bruno Costa ($BLUE aka kbelx $RESET)"

    $githubUrl   = "https://github.com/kbelx"
    $linkedinUrl = "https://www.linkedin.com/in/brunocosta-dev/"

    if ($useAnsi) {
        Write-Host "  GitHub:   $ESC]8;;$githubUrl$ESC\$BLUE$githubUrl$RESET$ESC]8;;$ESC\"
        Write-Host "  LinkedIn: $ESC]8;;$linkedinUrl$ESC\$BLUE$linkedinUrl$RESET$ESC]8;;$ESC\"
    }
    else {
        Write-Host "  GitHub:   $githubUrl"
        Write-Host "  LinkedIn: $linkedinUrl"
    }
}

Write-Host $separator
Write-Host ""
