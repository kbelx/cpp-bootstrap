<#
.SYNOPSIS
Suíte de testes automatizados para cpp_bootstrap.ps1.

.DESCRIPTION
Executa testes unitários, de validação e de integração para o gerador
de projetos C++ (cpp_bootstrap.ps1). Executa em ambiente isolado (pasta temporária)
sem modificar os arquivos do repositório ou o diretório de trabalho do usuário.

.PARAMETER IncludeBuild
Quando especificado, também compila o projeto gerado usando CMake e o compilador C++23.

.PARAMETER VerboseOutput
Exibe os logs detalhados (stdout/stderr) dos comandos executados durante os testes.

.EXAMPLE
./tests/test_cpp_bootstrap.ps1

.EXAMPLE
./tests/test_cpp_bootstrap.ps1 -IncludeBuild

.EXAMPLE
pwsh ./tests/test_cpp_bootstrap.ps1 -VerboseOutput
#>

[CmdletBinding()]
param (
    [switch]$IncludeBuild,
    [switch]$VerboseOutput,
    [string]$TargetScript
)

# Se executado no Windows PowerShell 5.1 e o pwsh (PowerShell 7+) estiver disponivel,
# delega para pwsh para garantir compatibilidade completa com UTF-8 sem BOM e sequencias ANSI.
if ($PSVersionTable.PSEdition -ne "Core") {
    $pwshCmd = Get-Command "pwsh" -ErrorAction SilentlyContinue
    if ($pwshCmd) {
        $forwardArgs = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $PSCommandPath)
        if ($PSBoundParameters.Count -gt 0) {
            foreach ($key in $PSBoundParameters.Keys) {
                $val = $PSBoundParameters[$key]
                if ($val -is [switch] -and $val.IsPresent) {
                    $forwardArgs += "-$key"
                } elseif ($val -isnot [switch]) {
                    $forwardArgs += "-$key"
                    $forwardArgs += "$val"
                }
            }
        }
        & $pwshCmd.Source $forwardArgs
        exit $LASTEXITCODE
    }
}

# Determina o interpretador do PowerShell atual para invocar os subprocessos isolados
$psExe = if ($PSVersionTable.PSEdition -eq "Core") { "pwsh" } else { "powershell" }

# Localiza o script principal a ser testado
if (-not $TargetScript) {
    $TargetScript = Join-Path -Path $PSScriptRoot -ChildPath "..\cpp_bootstrap.ps1"
}
$TargetScript = [System.IO.Path]::GetFullPath($TargetScript)

if (-not (Test-Path -LiteralPath $TargetScript -PathType Leaf)) {
    Write-Error "Script alvo não encontrado: $TargetScript"
    exit 1
}

# Paleta de cores para saída formatada
$RESET  = "`e[0m"
$GREEN  = "`e[32m"
$CYAN   = "`e[36m"
$BLUE   = "`e[34m"
$GRAY   = "`e[90m"
$YELLOW = "`e[33m"
$RED    = "`e[31m"
$BOLD   = "`e[1m"

$script:TotalTests  = 0
$script:PassedTests = 0
$script:FailedTests = 0
$script:SkippedTests = 0

function Write-Header {
    param([string]$Title)
    Write-Host ""
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
    Write-Host "$BOLD$CYAN$Title$RESET"
    Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
}

function Run-Test {
    param(
        [string]$Name,
        [scriptblock]$Assertion
    )
    $script:TotalTests++
    $testNumber = "{0:D2}" -f $script:TotalTests

    try {
        $result = & $Assertion
        if ($result -or ($null -eq $result)) {
            $script:PassedTests++
            Write-Host "  $GREEN[PASS]$RESET Teste $($testNumber): $Name"
        } else {
            $script:FailedTests++
            Write-Host "  $RED[FAIL]$RESET Teste $($testNumber): $Name (Asserção retornou falso)"
        }
    }
    catch {
        $script:FailedTests++
        Write-Host "  $RED[FAIL]$RESET Teste $($testNumber): $Name"
        Write-Host "         $RED-> Erro:$RESET $($_.Exception.Message)"
    }
}

function Run-Skip {
    param(
        [string]$Name,
        [string]$Reason
    )
    $script:TotalTests++
    $script:SkippedTests++
    $testNumber = "{0:D2}" -f $script:TotalTests
    Write-Host "  $YELLOW[SKIP]$RESET Teste $($testNumber): $Name ($Reason)"
}

function Invoke-BootstrapIsolated {
    param(
        [string[]]$ScriptArguments,
        [string]$WorkingDirectory
    )

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $psExe
    
    # Monta a linha de comando para o processo filho
    $argList = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "`"$TargetScript`"")
    if ($ScriptArguments) {
        foreach ($arg in $ScriptArguments) {
            $argList += "`"$arg`""
        }
    }
    
    $startInfo.Arguments = $argList -join " "
    $startInfo.WorkingDirectory = $WorkingDirectory
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo

    $null = $process.Start()
    $stdout = $process.StandardOutput.ReadToEnd()
    $stderr = $process.StandardError.ReadToEnd()
    $process.WaitForExit()

    if ($VerboseOutput) {
        if ($stdout) { Write-Host "$GRAY[STDOUT] $stdout$RESET" }
        if ($stderr) { Write-Host "$RED[STDERR] $stderr$RESET" }
    }

    return [PSCustomObject]@{
        ExitCode = $process.ExitCode
        StdOut   = $stdout
        StdErr   = $stderr
    }
}

# Criação do diretório temporário de testes (sandbox)
$tempRoot = Join-Path -Path ([System.IO.Path]::GetTempPath()) -ChildPath ("cpp_bootstrap_tests_" + [System.Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null

$startTime = [System.Diagnostics.Stopwatch]::StartNew()

try {
    Write-Header "[TEST SUITE] cpp_bootstrap.ps1"
    Write-Host "$GRAY* Script sob teste :$RESET $TargetScript"
    Write-Host "$GRAY* Ambiente de teste :$RESET $tempRoot"
    Write-Host "$GRAY* Interpretador     :$RESET $psExe ($($PSVersionTable.PSVersion))"

    # --------------------------------------------------------------------------
    # 1. Testes de Sintaxe e Análise Estática
    # --------------------------------------------------------------------------
    Write-Header "1. Verificação Estática de Sintaxe"

    Run-Test "Sintaxe do script é válida (sem erros de parsing no PowerShell)" {
        $tokens = $null
        $errors = $null
        $null = [System.Management.Automation.Language.Parser]::ParseFile($TargetScript, [ref]$tokens, [ref]$errors)
        if ($errors -and $errors.Count -gt 0) {
            $msg = ($errors | ForEach-Object { $_.Message }) -join "; "
            throw "Erros de sintaxe encontrados: $msg"
        }
        return $true
    }

    # --------------------------------------------------------------------------
    # 2. Testes de Validação de Parâmetros e Segurança (Casos de Falha)
    # --------------------------------------------------------------------------
    Write-Header "2. Validação de Argumentos e Prevenção de Erros"

    Run-Test "Execução sem argumentos exibe tutorial e encerra com código 1" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @() -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 1) { throw "Esperado exit code 1, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Uso:" -or $res.StdOut -notmatch "Dica:") {
            throw "Tutorial de uso não foi exibido na saída"
        }
        return $true
    }

    Run-Test "Nome de projeto vazio deve encerrar com código 2 e mensagem descritiva" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @('""') -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Nome do projeto vazio") {
            throw "Mensagem de 'Nome do projeto vazio' não encontrada"
        }
        return $true
    }

    Run-Test "Nome contendo caracteres proibidos deve ser rejeitado com código 2" {
        $badName = "Projeto<Invalido>"
        $res = Invoke-BootstrapIsolated -ScriptArguments @($badName) -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Caracteres proibidos encontrados") {
            throw "Alerta de caracteres proibidos não encontrado na saída"
        }
        return $true
    }

    Run-Test "Nome terminando com ponto deve ser rejeitado no Windows com código 2" {
        if (-not $IsWindows -and $PSVersionTable.Platform -ne "Win32NT") {
            return $true # Validação específica de regras do Windows
        }
        $res = Invoke-BootstrapIsolated -ScriptArguments @("ProjetoComPonto.") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "não podem terminar com '\.' ou espaço") {
            throw "Mensagem de regra do Windows sobre ponto/espaço não encontrada"
        }
        return $true
    }

    Run-Test "Nome reservado do Windows (ex: CON, AUX) deve ser rejeitado com código 2" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @("CON") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "nome reservado do Windows") {
            throw "Alerta de nome reservado não encontrado na saída"
        }
        return $true
    }

    Run-Test "Pasta já existente bloqueia sobrescrita e encerra com código 2" {
        $existingName = "ProjetoPreExistente"
        $existingPath = Join-Path -Path $tempRoot -ChildPath $existingName
        New-Item -ItemType Directory -Path $existingPath -Force | Out-Null

        $res = Invoke-BootstrapIsolated -ScriptArguments @($existingName) -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Já existe uma pasta com o nome") {
            throw "Mensagem de pasta pré-existente não encontrada"
        }
        return $true
    }

    # --------------------------------------------------------------------------
    # 3. Testes de Criação e Integridade da Estrutura (Caso de Sucesso)
    # --------------------------------------------------------------------------
    Write-Header "3. Criação de Projeto Válido e Estrutura Gerada"

    $validProject = "ProjetoTesteSucesso"
    $projectDir = Join-Path -Path $tempRoot -ChildPath $validProject

    Run-Test "Criação de projeto válido encerra com código 0 e mensagem de sucesso" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @($validProject) -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 0) { throw "Esperado exit code 0, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "criado com sucesso") {
            throw "Mensagem de sucesso não encontrada na saída"
        }
        return $true
    }

    Run-Test "Diretórios obrigatórios gerados (src, include, .vscode)" {
        $expectedDirs = @(
            $projectDir,
            (Join-Path $projectDir "src"),
            (Join-Path $projectDir "include"),
            (Join-Path $projectDir ".vscode")
        )
        foreach ($d in $expectedDirs) {
            if (-not (Test-Path -LiteralPath $d -PathType Container)) {
                throw "Diretório obrigatório ausente: $d"
            }
        }
        return $true
    }

    Run-Test "Arquivo src/main.cpp criado com assinatura válida em C++" {
        $mainCpp = Join-Path $projectDir "src/main.cpp"
        if (-not (Test-Path -LiteralPath $mainCpp -PathType Leaf)) {
            throw "Arquivo src/main.cpp não foi criado"
        }
        $content = Get-Content -LiteralPath $mainCpp -Raw
        if ($content -notmatch "int main\(\)") { throw "Assinatura 'int main()' não encontrada" }
        if ($content -notmatch "#include <iostream>") { throw "Inclusão <iostream> não encontrada" }
        if ($content -notmatch "std::setlocale") { throw "Chamada std::setlocale não encontrada" }
        return $true
    }

    Run-Test "Configurações do VS Code criadas com sintaxe JSON válida" {
        $launchJson = Join-Path $projectDir ".vscode/launch.json"
        $settingsJson = Join-Path $projectDir ".vscode/settings.json"

        if (-not (Test-Path -LiteralPath $launchJson)) { throw "launch.json ausente" }
        if (-not (Test-Path -LiteralPath $settingsJson)) { throw "settings.json ausente" }

        # Testa parsing JSON
        $launchContent = Get-Content -LiteralPath $launchJson -Raw | ConvertFrom-Json
        $settingsContent = Get-Content -LiteralPath $settingsJson -Raw | ConvertFrom-Json

        if (-not $launchContent.configurations) { throw "launch.json não possui chave 'configurations'" }
        if ($settingsContent.'cmake.exportCompileCommandsFile' -ne $true) {
            throw "settings.json não habilitou 'cmake.exportCompileCommandsFile'"
        }
        return $true
    }

    Run-Test "CMakeLists.txt gerado com C++23 e substituição correta do nome do projeto" {
        $cmakePath = Join-Path $projectDir "CMakeLists.txt"
        if (-not (Test-Path -LiteralPath $cmakePath -PathType Leaf)) {
            throw "CMakeLists.txt não encontrado"
        }
        $cmakeContent = Get-Content -LiteralPath $cmakePath -Raw

        # Valida substituição de template
        if ($cmakeContent -match "__PROJECT_NAME__") {
            throw "Placeholder __PROJECT_NAME__ não foi substituído no CMakeLists.txt"
        }
        if ($cmakeContent -notmatch "project\($validProject LANGUAGES CXX\)") {
            throw "Declaração project($validProject LANGUAGES CXX) não encontrada"
        }
        if ($cmakeContent -notmatch "set\(CMAKE_CXX_STANDARD 23\)") {
            throw "Padrão C++23 não configurado"
        }
        if ($cmakeContent -notmatch "set\(CMAKE_CXX_STANDARD_REQUIRED ON\)") {
            throw "Requisito estrito C++23 não ativado"
        }
        if ($cmakeContent -notmatch "-pedantic-errors") {
            throw "Flags de compilação esperadas ausentes"
        }
        return $true
    }

    Run-Test "Arquivo .gitignore criado com padrões essenciais de build" {
        $gitIgnorePath = Join-Path $projectDir ".gitignore"
        if (-not (Test-Path -LiteralPath $gitIgnorePath -PathType Leaf)) {
            throw ".gitignore não encontrado"
        }
        $gitIgnoreContent = Get-Content -LiteralPath $gitIgnorePath -Raw
        if ($gitIgnoreContent -notmatch "build/") { throw "Padrão build/ não encontrado em .gitignore" }
        if ($gitIgnoreContent -notmatch "\*\.exe") { throw "Padrão *.exe não encontrado em .gitignore" }
        return $true
    }

    # --------------------------------------------------------------------------
    # 4. Teste de Integração Real: Compilação com CMake e GCC (Opcional ou via Switch)
    # --------------------------------------------------------------------------
    Write-Header "4. Integração e Compilação Real com CMake"

    $hasCmake = (Get-Command "cmake" -ErrorAction SilentlyContinue) -ne $null
    $hasGcc = (Get-Command "g++" -ErrorAction SilentlyContinue) -ne $null

    if ($IncludeBuild) {
        if (-not $hasCmake -or -not $hasGcc) {
            Run-Skip "Compilação real com CMake e C++23" "CMake ou g++ não estão no PATH"
        } else {
            Run-Test "Projeto gerado configura e compila com sucesso via CMake (C++23)" {
                # Configura build
                $cmakeConfig = Start-Process -FilePath "cmake" -ArgumentList "-B", "build", "-G", "`"MinGW Makefiles`"" -WorkingDirectory $projectDir -NoNewWindow -Wait -PassThru
                if ($cmakeConfig.ExitCode -ne 0) {
                    # Tenta com gerador padrão caso MinGW Makefiles não seja suportado diretamente
                    $cmakeConfig = Start-Process -FilePath "cmake" -ArgumentList "-B", "build" -WorkingDirectory $projectDir -NoNewWindow -Wait -PassThru
                }
                if ($cmakeConfig.ExitCode -ne 0) {
                    throw "cmake -B build falhou com exit code $($cmakeConfig.ExitCode)"
                }

                # Compila
                $cmakeBuild = Start-Process -FilePath "cmake" -ArgumentList "--build", "build" -WorkingDirectory $projectDir -NoNewWindow -Wait -PassThru
                if ($cmakeBuild.ExitCode -ne 0) {
                    throw "cmake --build build falhou com exit code $($cmakeBuild.ExitCode)"
                }

                # Verifica binário gerado
                $binName = if ($IsWindows -or $PSVersionTable.Platform -eq "Win32NT") { "$validProject.exe" } else { $validProject }
                $binPath = Join-Path $projectDir "build/$binName"
                if (-not (Test-Path -LiteralPath $binPath)) {
                    # Em geradores multi-config (ex: Visual Studio / Ninja Multi-Config)
                    $binPathDebug = Join-Path $projectDir "build/Debug/$binName"
                    if (-not (Test-Path -LiteralPath $binPathDebug)) {
                        throw "Binário final não foi gerado em $binPath nem em $binPathDebug"
                    }
                }
                return $true
            }
        }
    } else {
        Run-Skip "Compilação real com CMake e C++23" "Passe o parâmetro -IncludeBuild para rodar este teste"
    }

}
finally {
    $startTime.Stop()

    # Limpeza segura da pasta temporária
    if (Test-Path -LiteralPath $tempRoot) {
        try {
            Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
        } catch {
            # Se algum processo manteve lock temporário, ignora erro no cleanup
        }
    }
}

# ------------------------------------------------------------------------------
# Resumo da Execucao
# ------------------------------------------------------------------------------
Write-Host ""
Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host "$BOLD[RESUMO DA EXECUCAO DOS TESTES]$RESET"
Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host "  Total de testes : $script:TotalTests"
Write-Host "  $GREEN Aprovados        : $script:PassedTests$RESET"
if ($script:FailedTests -gt 0) {
    Write-Host "  $RED Falhos           : $script:FailedTests$RESET"
} else {
    Write-Host "  $GRAY Falhos           : 0$RESET"
}
if ($script:SkippedTests -gt 0) {
    Write-Host "  $YELLOW Ignorados        : $script:SkippedTests$RESET"
}
Write-Host "  Tempo total     : $($startTime.Elapsed.TotalSeconds.ToString('F2'))s"
Write-Host "$GRAY━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━$RESET"
Write-Host ""

if ($script:FailedTests -gt 0) {
    Write-Host "$RED[FAIL] A suite de testes falhou!$RESET"
    exit 1
} else {
    Write-Host "$GREEN[PASS] Todos os testes passaram com sucesso!$RESET"
    exit 0
}
