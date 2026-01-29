param (
    [string]$ProjectName
)

$ErrorActionPreference = "Stop"

if (-not $ProjectName) {
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    Write-Host "Siga esses passos para gerar o projeto:"
    Write-Host "1. Coloque o 'criar_projeto_cpp.ps1' em alguma pasta;"
    Write-Host "2. Abra o terminal na pasta;"
    Write-Host "3. Rode: ./criar_projeto_cpp.ps1 NomeDoProjeto"
    Write-Host "4. NÃO INCLUA '<>'"
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 1
}

#============================ [ PALETA / FUNÇÕES ] ============================

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

$ProjectName = $ProjectName.Trim().Trim('"').Trim("'")

if ([string]::IsNullOrWhiteSpace($ProjectName)) {
    Write-ErrorMsg "❌ Nome do projeto vazio."
    Write-Info "✅ Exemplo: ./criar_projeto_cpp.ps1 MeuProjeto"
    exit 2
}

$invalidChars = [System.IO.Path]::GetInvalidFileNameChars()
$badFound = $ProjectName.ToCharArray() | Where-Object { $invalidChars -contains $_ }

if ($badFound.Count -gt 0) {
    $badList = ($badFound | Select-Object -Unique) -join " "
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-ErrorMsg "❌ Nome inválido: '$ProjectName'"
    Write-Warn "Caracteres proibidos encontrados: $badList"
    Write-Info "Use apenas letras, números, '_' e '-' (recomendado)."
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    exit 3
}

if ($ProjectName -match '[\.\s]$') {
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-ErrorMsg "❌ Nome inválido: '$ProjectName'"
    Write-Warn "No Windows, nomes de pasta não podem terminar com '.' ou espaço."
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    exit 4
}

$baseName = ($ProjectName.Split('.')[0]).ToUpperInvariant()
$reserved = @("CON","PRN","AUX","NUL","COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9","LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9")
if ($reserved -contains $baseName) {
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-ErrorMsg "❌ Nome inválido: '$ProjectName'"
    Write-Warn "Esse é um nome reservado do Windows."
    Write-Info "Escolha outro (ex: '${ProjectName}_proj')."
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    exit 5
}

if (Test-Path -LiteralPath $ProjectName -PathType Container) {
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-ErrorMsg "❌ Já existe uma pasta com o nome: '$ProjectName'"
    Write-Info "Sugestões:"
    Write-Info "  • Use outro nome: ./criar_projeto_cpp.ps1 ${ProjectName}_2"
    Write-Info "  • Ou renomeie/apague a pasta existente."
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    exit 6
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
            "name": "Rodar no Terminal Externo",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "externalTerminal"
        }
    ]
}
'@ | Out-File ".vscode/launch.json" -Encoding utf8

    # .vscode/settings.json
    @'
{
    "cmake.exportCompileCommandsFile": true,

    "clangd.arguments": [
        "--compile-commands-dir=${workspaceFolder}/build",
        "--query-driver=C:/msys64/ucrt64/bin/g++.exe"
    ],

    "C_Cpp.intelliSenseEngine": "disabled"
}
'@ | Out-File ".vscode/settings.json" -Encoding utf8

# src/main.cpp
    @"
/******************************************************************************
COMPONENTE:   <NOME_DO_ARQUIVO>.cpp
AUTOR:      <SEU_NOME>
DATA:        <DD/MM/AAAA>
PROPOSITO:     <Função do componente dentro do sistema>

RESPONSABILIDADES:
- <Responsabilidade 1>
- <Responsabilidade 2>
- <Responsabilidade n>

NOTAS:
- <Observações adicionais>
*******************************************************************************/

#include <iostream>
#include <clocale>

int main(){
    setlocale(LC_ALL, "pt-BR.UTF-8");

    
    std::cin.get();
}
"@ | Out-File src/main.cpp -Encoding utf8

# CMakeLists.txt
    @"
#═══════════════════════════════════════════════════════════════════════════════
# 🧱 CONFIGURAÇÃO MÍNIMA DO CMAKE
# ------------------------------------------------------------------------------
# Define a versão mínima do CMake necessária para gerar o projeto.
# Garante compatibilidade com comandos e recursos usados neste arquivo.
cmake_minimum_required(VERSION 3.20)

#═══════════════════════════════════════════════════════════════════════════════
# 📦 DEFINIÇÃO DO PROJETO
# ------------------------------------------------------------------------------
# Nome do projeto e linguagem utilizada.
# O nome do executável final será o mesmo do projeto.
project($ProjectName LANGUAGES CXX)

#═══════════════════════════════════════════════════════════════════════════════
# 🔢 CONFIGURAÇÕES DA LINGUAGEM C++
# ------------------------------------------------------------------------------
# Define o padrão da linguagem C++ utilizado no projeto.
# Aqui usamos C++23.
set(CMAKE_CXX_STANDARD 23)

# Obriga o compilador a respeitar exatamente o padrão definido acima.
# Se o compilador não suportar C++23, a configuração falha.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#═══════════════════════════════════════════════════════════════════════════════
# 🐞 TIPO DE BUILD
# ------------------------------------------------------------------------------
# Define o tipo de build padrão.
# "Debug" habilita símbolos de depuração e desativa otimizações.
# Obs: funciona bem com Ninja/Make (geradores single-config).
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

#═══════════════════════════════════════════════════════════════════════════════
# 🚀 CRIAÇÃO DO EXECUTÁVEL
# ------------------------------------------------------------------------------
# Cria o target do executável usando o nome do projeto (PROJECT_NAME).
add_executable(${PROJECT_NAME})

#═══════════════════════════════════════════════════════════════════════════════
# 📄 ARQUIVOS FONTE (.cpp)
# ------------------------------------------------------------------------------
# Associa os arquivos de implementação (.cpp) ao executável.
# O uso de target_sources melhora a organização e facilita a manutenção.
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

#═══════════════════════════════════════════════════════════════════════════════
# 📁 DIRETÓRIOS DE HEADERS (.hpp)
# ------------------------------------------------------------------------------
# Informa ao compilador onde procurar arquivos de cabeçalho (.hpp).
# Boa prática: headers ficam separados na pasta "include/".
target_include_directories(${PROJECT_NAME} PRIVATE include)

#═══════════════════════════════════════════════════════════════════════════════
# 🚨 FLAGS DE COMPILAÇÃO (QUALIDADE DE CÓDIGO)
# ------------------------------------------------------------------------------
# Conjunto de warnings agressivos para forçar boas práticas em C++.
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
        -pedantic-errors            # Segue o padrão C++ à risca (modo hardcore 😈)
        -fdiagnostics-color=always  # Mensagens de erro coloridas no terminal 🌈
    )
endif()

#═══════════════════════════════════════════════════════════════════════════════
# ▶️ TARGET "run" (WINDOWS) com fallback
# ------------------------------------------------------------------------------
# Target auxiliar para executar o programa após a compilação.
#
# Comportamento:
# - Abre um CMD externo (fora do VS Code)
# - Executa o binário gerado (independente do nome/pasta/configuração)
# - Mantém a janela aberta após a execução (/k)
#
# Execução:
#   cmake --build . --target run
#
# Implementação:
# - Usa o PowerShell para iniciar um novo processo (Start-Process)
# - O CMD é aberto com o parâmetro /k para não fechar automaticamente
#
if (WIN32)
    add_custom_target(run
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command
        " $exe = Join-Path '${CMAKE_CURRENT_BINARY_DIR}' '${PROJECT_NAME}.exe'; \
          if (Get-Command wt.exe -ErrorAction SilentlyContinue) { \
              wt.exe nt -d '${CMAKE_CURRENT_BINARY_DIR}' cmd.exe /k \"\"$exe\"\"; \
          } else { \
              cmd.exe /k \"\"$exe\"\"; \
          } "
        DEPENDS ${PROJECT_NAME}
        VERBATIM
    )
endif()
"@ | Out-File CMakeLists.txt -Encoding utf8

# .gitignore
    @"
build/
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
Write-Host "   $BLUE•$RESET Abra o projeto no VS Code com:"
Write-Host "     $BLUE code .$RESET"
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
