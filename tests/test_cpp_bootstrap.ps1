<#
.SYNOPSIS
Suite de testes automatizados para cpp_bootstrap.ps1.

.DESCRIPTION
Executa testes estaticos (AST/encoding), unitarios, de validacao, isolamento,
compatibilidade Windows PowerShell 5.1 / PowerShell 7+ e integracao real C++23 com CMake.
Roda em sandbox temporario isolado sem alterar arquivos do repositorio nem o diretorio atual.

.PARAMETER IncludeBuild
Quando especificado, configura e compila o projeto gerado com CMake e um compilador C++23.

.PARAMETER VerboseOutput
Exibe stdout/stderr detalhados dos subprocessos executados durante os testes.

.PARAMETER TargetScript
Caminho opcional para um script alternativo a ser homologado.

.EXAMPLE
./tests/test_cpp_bootstrap.ps1

.EXAMPLE
./tests/test_cpp_bootstrap.ps1 -IncludeBuild

.EXAMPLE
pwsh ./tests/test_cpp_bootstrap.ps1 -IncludeBuild -VerboseOutput
#>

[CmdletBinding()]
param (
    [switch]$IncludeBuild,
    [switch]$VerboseOutput,
    [string]$TargetScript
)

$ErrorActionPreference = "Stop"

#========================== [ DETECCAO DE AMBIENTE ] ===========================

$isWin = ($PSVersionTable.PSEdition -eq "Desktop") -or
         ($env:OS -eq "Windows_NT") -or
         ((Test-Path Variable:\IsWindows) -and [bool](Get-Variable -Name IsWindows -ValueOnly))
$isMac = (-not $isWin) -and
         ((Test-Path Variable:\IsMacOS) -and [bool](Get-Variable -Name IsMacOS -ValueOnly))

$psExe = if ($PSVersionTable.PSEdition -eq "Core") { "pwsh" } else { "powershell" }

if (-not $TargetScript) {
    $TargetScript = Join-Path -Path $PSScriptRoot -ChildPath "..\cpp_bootstrap.ps1"
}
$TargetScript = [System.IO.Path]::GetFullPath($TargetScript)
$SelfScript   = [System.IO.Path]::GetFullPath($PSCommandPath)

if (-not (Test-Path -LiteralPath $TargetScript -PathType Leaf)) {
    Write-Error "Script alvo nao encontrado: $TargetScript"
    exit 1
}

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
$GRAY   = if ($useAnsi) { "$ESC[90m" } else { "" }
$YELLOW = if ($useAnsi) { "$ESC[33m" } else { "" }
$RED    = if ($useAnsi) { "$ESC[31m" } else { "" }
$BOLD   = if ($useAnsi) { "$ESC[1m" }  else { "" }

$divider = "$GRAY------------------------------------------------------------$RESET"

$script:TotalTests   = 0
$script:PassedTests  = 0
$script:FailedTests  = 0
$script:SkippedTests = 0

function Write-SectionHeader {
    param([string]$Title)
    Write-Host ""
    Write-Host $divider
    Write-Host "$BOLD$CYAN$Title$RESET"
    Write-Host $divider
}

function Invoke-Test {
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
            Write-Host "  $GREEN[PASS]$RESET Teste ${testNumber}: $Name"
        }
        else {
            $script:FailedTests++
            Write-Host "  $RED[FAIL]$RESET Teste ${testNumber}: $Name (Assercao retornou falso)"
        }
    }
    catch {
        $script:FailedTests++
        Write-Host "  $RED[FAIL]$RESET Teste ${testNumber}: $Name"
        Write-Host "         $RED-> Erro:$RESET $($_.Exception.Message)"
    }
}

function Invoke-Skip {
    param(
        [string]$Name,
        [string]$Reason
    )
    $script:TotalTests++
    $script:SkippedTests++
    $testNumber = "{0:D2}" -f $script:TotalTests
    Write-Host "  $YELLOW[SKIP]$RESET Teste ${testNumber}: $Name ($Reason)"
}

function Invoke-ExternalProcess {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [AllowEmptyCollection()][AllowEmptyString()][string[]]$ArgumentList = @(),
        [Parameter(Mandatory = $true)][string]$WorkingDirectory
    )

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName               = $FilePath
    $startInfo.WorkingDirectory       = $WorkingDirectory
    $startInfo.RedirectStandardInput  = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError  = $true
    $startInfo.UseShellExecute        = $false
    $startInfo.CreateNoWindow         = $true

    if ($null -ne $startInfo.PSObject.Properties["ArgumentList"]) {
        foreach ($arg in $ArgumentList) {
            [void]$startInfo.ArgumentList.Add($arg)
        }
    }
    else {
        $quotedArgs = foreach ($arg in $ArgumentList) {
            $escaped = $arg -replace '(\\*)"', '$1$1\"' -replace '(\\+)$', '$1$1'
            "`"$escaped`""
        }
        $startInfo.Arguments = $quotedArgs -join " "
    }

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo

    [void]$process.Start()
    $process.StandardInput.Close()
    $stdoutTask = $process.StandardOutput.ReadToEndAsync()
    $stderrTask = $process.StandardError.ReadToEndAsync()
    $process.WaitForExit()

    $stdout = $stdoutTask.GetAwaiter().GetResult()
    $stderr = $stderrTask.GetAwaiter().GetResult()

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

function Invoke-BootstrapIsolated {
    param(
        [AllowEmptyCollection()][AllowEmptyString()][string[]]$ScriptArguments = @(),
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [string]$ShellExecutable = $psExe
    )

    $fullArgs = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $TargetScript)
    if ($null -ne $ScriptArguments -and $ScriptArguments.Count -gt 0) {
        $fullArgs += $ScriptArguments
    }

    return Invoke-ExternalProcess -FilePath $ShellExecutable -ArgumentList $fullArgs -WorkingDirectory $WorkingDirectory
}

function Test-HasUtf8Bom {
    param([Parameter(Mandatory = $true)][string]$Path)
    $bytes = [System.IO.File]::ReadAllBytes($Path)
    return ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
}

#====================== [ EXECUCAO EM SANDBOX ISOLADO ] ========================

$tempRoot = Join-Path -Path ([System.IO.Path]::GetTempPath()) -ChildPath ("cpp_bootstrap_tests_" + [System.Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null

$startTime = [System.Diagnostics.Stopwatch]::StartNew()

try {
    $osName = if ($isWin) { "Windows" } elseif ($isMac) { "macOS" } else { "Linux" }
    Write-SectionHeader "[TEST SUITE] cpp_bootstrap.ps1"
    Write-Host "$GRAY* Script sob teste :$RESET $TargetScript"
    Write-Host "$GRAY* Ambiente sandbox :$RESET $tempRoot"
    Write-Host "$GRAY* Interpretador    :$RESET $psExe ($($PSVersionTable.PSVersion)) em $osName"

    # --------------------------------------------------------------------------
    # 1. Verificacao Estatica de Sintaxe, Encoding e Seguranca de Parsing
    # --------------------------------------------------------------------------
    Write-SectionHeader "1. Verificacao Estatica de Sintaxe e Encoding"

    Invoke-Test "Scripts .ps1 estao em UTF-8 sem BOM, 100% ASCII 7-bit e sem erros de AST" {
        foreach ($scriptPath in @($TargetScript, $SelfScript)) {
            $name = [System.IO.Path]::GetFileName($scriptPath)

            if (Test-HasUtf8Bom -Path $scriptPath) {
                throw "BOM UTF-8 detectado em $name (violacao da regra UTF-8 sem BOM)"
            }

            $bytes = [System.IO.File]::ReadAllBytes($scriptPath)
            for ($i = 0; $i -lt $bytes.Length; $i++) {
                if ($bytes[$i] -gt 0x7F) {
                    throw "Byte nao-ASCII (0x$($bytes[$i].ToString('X2'))) na posicao $i de $name (risco de falha de parsing no PS 5.1)"
                }
            }

            $tokens = $null
            $errors = $null
            [void][System.Management.Automation.Language.Parser]::ParseFile($scriptPath, [ref]$tokens, [ref]$errors)
            if ($null -ne $errors -and $errors.Count -gt 0) {
                $msg = ($errors | ForEach-Object { $_.Message }) -join "; "
                throw "Erros de sintaxe em ${name}: $msg"
            }
        }
        return $true
    }

    # --------------------------------------------------------------------------
    # 2. Validacao de Argumentos, Seguranca no CMake e Prevencao de Erros
    # --------------------------------------------------------------------------
    Write-SectionHeader "2. Validacao de Argumentos e Prevencao de Erros"

    Invoke-Test "Execucao sem argumentos exibe tutorial e encerra com codigo 1" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @() -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 1) { throw "Esperado exit code 1, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Uso:" -or $res.StdOut -notmatch "Dica:") {
            throw "Tutorial de uso nao foi exibido na saida"
        }
        return $true
    }

    Invoke-Test "Nome de projeto vazio ou apenas espacos encerra com codigo 2" {
        foreach ($emptyInput in @("", "   ", '""')) {
            $res = Invoke-BootstrapIsolated -ScriptArguments @($emptyInput) -WorkingDirectory $tempRoot
            if ($res.ExitCode -ne 2) {
                throw "Entrada '$emptyInput': esperado exit code 2, obtido $($res.ExitCode)"
            }
            if ($res.StdOut -notmatch "Nome do projeto vazio") {
                throw "Entrada '$emptyInput': mensagem 'Nome do projeto vazio' ausente"
            }
        }
        return $true
    }

    Invoke-Test "Nome contendo caracteres proibidos e rejeitado com codigo 2" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @("Projeto<Invalido>") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "Caracteres proibidos encontrados") {
            throw "Alerta de caracteres proibidos nao encontrado na saida"
        }
        return $true
    }

    Invoke-Test "Nome com espaco interno ou multiplos tokens sem aspas e rejeitado com codigo 2 (protege CMake)" {
        $resQuoted = Invoke-BootstrapIsolated -ScriptArguments @("Meu Projeto") -WorkingDirectory $tempRoot
        if ($resQuoted.ExitCode -ne 2) {
            throw "Nome com espaco entre aspas: esperado exit code 2, obtido $($resQuoted.ExitCode)"
        }
        if ($resQuoted.StdOut -notmatch "Caracteres proibidos encontrados") {
            throw "Alerta de caracteres proibidos ausente para nome com espaco interno"
        }

        $resUnquoted = Invoke-BootstrapIsolated -ScriptArguments @("Meu", "Projeto") -WorkingDirectory $tempRoot
        if ($resUnquoted.ExitCode -ne 2) {
            throw "Tokens multiplos sem aspas: esperado exit code 2, obtido $($resUnquoted.ExitCode)"
        }
        return $true
    }

    Invoke-Test "Nome terminando com ponto final e rejeitado em todas as plataformas com codigo 2" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @("ProjetoComPonto.") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "nao podem terminar com '\.' ou espaco") {
            throw "Mensagem sobre ponto/espaco final nao encontrada"
        }
        return $true
    }

    Invoke-Test "Nome reservado do Windows (CON, AUX, NUL) e rejeitado em todas as plataformas com codigo 2" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @("CON") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 2) { throw "Esperado exit code 2, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "nome reservado do Windows") {
            throw "Alerta de nome reservado nao encontrado na saida"
        }
        return $true
    }

    Invoke-Test "Pasta ou arquivo ja existente bloqueia sobrescrita e encerra com codigo 2" {
        $existingDir = Join-Path -Path $tempRoot -ChildPath "ProjetoPreExistente"
        New-Item -ItemType Directory -Path $existingDir -Force | Out-Null
        $sentinel = Join-Path -Path $existingDir -ChildPath "sentinela.txt"
        [System.IO.File]::WriteAllText($sentinel, "intacto")

        $resDir = Invoke-BootstrapIsolated -ScriptArguments @("ProjetoPreExistente") -WorkingDirectory $tempRoot
        if ($resDir.ExitCode -ne 2) { throw "Pasta existente: esperado exit code 2, obtido $($resDir.ExitCode)" }
        if ($resDir.StdOut -notmatch "Ja existe uma pasta ou arquivo com o nome") {
            throw "Mensagem de item pre-existente nao encontrada"
        }
        if ([System.IO.File]::ReadAllText($sentinel) -ne "intacto") {
            throw "Conteudo da pasta pre-existente foi modificado indevidamente"
        }

        $existingFile = Join-Path -Path $tempRoot -ChildPath "ArquivoComMesmoNome"
        [System.IO.File]::WriteAllText($existingFile, "arquivo")
        $resFile = Invoke-BootstrapIsolated -ScriptArguments @("ArquivoComMesmoNome") -WorkingDirectory $tempRoot
        if ($resFile.ExitCode -ne 2) { throw "Arquivo existente: esperado exit code 2, obtido $($resFile.ExitCode)" }

        return $true
    }

    # --------------------------------------------------------------------------
    # 3. Normalizacao, Isolamento de Diretorio e Criacao da Estrutura
    # --------------------------------------------------------------------------
    Write-SectionHeader "3. Normalizacao, Isolamento e Estrutura Gerada"

    Invoke-Test "Espacos externos sofrem trim unico e criam a pasta exata sem espacos nas bordas" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @("  ProjetoTrimado  ") -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 0) { throw "Esperado exit code 0, obtido $($res.ExitCode)" }

        $trimmedDir = Join-Path -Path $tempRoot -ChildPath "ProjetoTrimado"
        $untrimmedDir = Join-Path -Path $tempRoot -ChildPath "  ProjetoTrimado  "

        if (-not (Test-Path -LiteralPath $trimmedDir -PathType Container)) {
            throw "Pasta normalizada 'ProjetoTrimado' nao foi criada"
        }
        if ((Get-ChildItem -LiteralPath $tempRoot -Directory | Where-Object { $_.Name -ne $_.Name.Trim() }).Count -gt 0) {
            throw "Foi criada uma pasta com espaco no inicio ou no fim ($untrimmedDir)"
        }

        $cmakeText = [System.IO.File]::ReadAllText((Join-Path -Path $trimmedDir -ChildPath "CMakeLists.txt"))
        if ($cmakeText -notmatch "project\(ProjetoTrimado LANGUAGES CXX\)") {
            throw "CMakeLists.txt nao recebeu o nome trimado 'ProjetoTrimado'"
        }
        return $true
    }

    Invoke-Test "Execucao no mesmo processo nao altera o diretorio atual (sem Set-Location)" {
        $originalDir = (Get-Location).ProviderPath
        Push-Location -LiteralPath $tempRoot
        try {
            $beforeInTemp = (Get-Location).ProviderPath
            & $TargetScript "ProjetoInProcess" -Quiet *> $null
            $afterInTemp = (Get-Location).ProviderPath

            if ($afterInTemp -ne $beforeInTemp) {
                throw "Diretorio atual foi alterado de '$beforeInTemp' para '$afterInTemp'"
            }
            if (-not (Test-Path -LiteralPath (Join-Path -Path $tempRoot -ChildPath "ProjetoInProcess\CMakeLists.txt"))) {
                throw "ProjetoInProcess nao foi criado corretamente na execucao in-process"
            }
        }
        finally {
            Pop-Location
        }

        if ((Get-Location).ProviderPath -ne $originalDir) {
            throw "Diretorio original da sessao nao foi preservado"
        }
        return $true
    }

    $validProject = "ProjetoTesteSucesso"
    $projectDir   = Join-Path -Path $tempRoot -ChildPath $validProject

    Invoke-Test "Criacao de projeto valido encerra com codigo 0 e instrucoes CMake corretas" {
        $res = Invoke-BootstrapIsolated -ScriptArguments @($validProject) -WorkingDirectory $tempRoot
        if ($res.ExitCode -ne 0) { throw "Esperado exit code 0, obtido $($res.ExitCode)" }
        if ($res.StdOut -notmatch "criado com sucesso") {
            throw "Mensagem de sucesso nao encontrada na saida"
        }
        if ($res.StdOut -notmatch "cmake -B build" -or $res.StdOut -notmatch "cmake --build build") {
            throw "Instrucoes finais 'cmake -B build' / 'cmake --build build' ausentes"
        }
        return $true
    }

    Invoke-Test "Diretorios obrigatorios gerados (src, include, .vscode)" {
        $expectedDirs = @(
            $projectDir,
            (Join-Path -Path $projectDir -ChildPath "src"),
            (Join-Path -Path $projectDir -ChildPath "include"),
            (Join-Path -Path $projectDir -ChildPath ".vscode")
        )
        foreach ($dirPath in $expectedDirs) {
            if (-not (Test-Path -LiteralPath $dirPath -PathType Container)) {
                throw "Diretorio obrigatorio ausente: $dirPath"
            }
        }
        return $true
    }

    Invoke-Test "Arquivo src/main.cpp criado com assinatura valida e locale portavel" {
        $mainCpp = Join-Path -Path $projectDir -ChildPath "src/main.cpp"
        if (-not (Test-Path -LiteralPath $mainCpp -PathType Leaf)) {
            throw "Arquivo src/main.cpp nao foi criado"
        }
        $content = [System.IO.File]::ReadAllText($mainCpp)
        if ($content -notmatch "int main\(\)") { throw "Assinatura 'int main()' nao encontrada" }
        if ($content -notmatch "#include <iostream>") { throw "Inclusao <iostream> nao encontrada" }
        if ($content -notmatch "#include <clocale>") { throw "Inclusao <clocale> nao encontrada" }
        if ($content -notmatch 'std::setlocale\(LC_ALL,\s*""\)') {
            throw 'Chamada portavel std::setlocale(LC_ALL, "") nao encontrada'
        }
        return $true
    }

    Invoke-Test "Configuracoes do VS Code possuem sintaxe JSON valida e suporte multiplataforma" {
        $launchJson   = Join-Path -Path $projectDir -ChildPath ".vscode/launch.json"
        $settingsJson = Join-Path -Path $projectDir -ChildPath ".vscode/settings.json"

        if (-not (Test-Path -LiteralPath $launchJson -PathType Leaf)) { throw "launch.json ausente" }
        if (-not (Test-Path -LiteralPath $settingsJson -PathType Leaf)) { throw "settings.json ausente" }

        $launchObj   = [System.IO.File]::ReadAllText($launchJson) | ConvertFrom-Json
        $settingsObj = [System.IO.File]::ReadAllText($settingsJson) | ConvertFrom-Json

        if (-not $launchObj.configurations -or $launchObj.configurations.Count -lt 1) {
            throw "launch.json nao possui configuracoes"
        }
        $cfg = $launchObj.configurations[0]
        if ($null -eq $cfg.windows -or $null -eq $cfg.linux -or $null -eq $cfg.osx) {
            throw "launch.json nao contem blocos multiplataforma (windows, linux, osx)"
        }
        if ($settingsObj."cmake.exportCompileCommandsFile" -ne $true) {
            throw "settings.json nao habilitou cmake.exportCompileCommandsFile"
        }
        $queryDriver = @($settingsObj."clangd.arguments" | Where-Object { $_ -like "--query-driver=*" })
        if ($queryDriver.Count -eq 0 -or $queryDriver[0] -notmatch "/usr/bin/") {
            throw "settings.json nao configurou --query-driver multiplataforma"
        }
        return $true
    }

    Invoke-Test "CMakeLists.txt gerado com C++23 estrito, guarda de CMAKE_BUILD_TYPE e flags rigorosas" {
        $cmakePath = Join-Path -Path $projectDir -ChildPath "CMakeLists.txt"
        if (-not (Test-Path -LiteralPath $cmakePath -PathType Leaf)) {
            throw "CMakeLists.txt nao encontrado"
        }
        $cmakeContent = [System.IO.File]::ReadAllText($cmakePath)

        if ($cmakeContent -match "__PROJECT_NAME__") {
            throw "Placeholder __PROJECT_NAME__ nao foi substituido"
        }
        if ($cmakeContent -notmatch "project\($validProject LANGUAGES CXX\)") {
            throw "Declaracao project($validProject LANGUAGES CXX) ausente"
        }
        if ($cmakeContent -notmatch "set\(CMAKE_CXX_STANDARD 23\)") {
            throw "Padrao C++23 nao configurado"
        }
        if ($cmakeContent -notmatch "set\(CMAKE_CXX_STANDARD_REQUIRED ON\)") {
            throw "Requisito estrito C++23 nao ativado"
        }
        if ($cmakeContent -notmatch "if\(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES\)") {
            throw "Guarda nao-destrutiva de CMAKE_BUILD_TYPE ausente"
        }
        if ($cmakeContent -notmatch "-pedantic-errors" -or $cmakeContent -notmatch "/permissive-") {
            throw "Flags rigorosas de compilacao (GCC/Clang e MSVC) ausentes"
        }
        if ($cmakeContent -notmatch "include\(CheckLinkerFlag\)" -or
            $cmakeContent -notmatch 'check_linker_flag\(CXX\s+"-lstdc\+\+exp"\s+HAVE_STDCXXEXP\)' -or
            $cmakeContent -notmatch 'target_link_libraries\(\$\{PROJECT_NAME\}\s+PRIVATE\s+stdc\+\+exp\)') {
            throw "Secao 9 de suporte a libstdc++exp (CheckLinkerFlag / stdc++exp) ausente no CMakeLists.txt"
        }
        return $true
    }

    Invoke-Test "Arquivo .gitignore ignora artefatos de build e preserva launch.json e settings.json" {
        $gitIgnorePath = Join-Path -Path $projectDir -ChildPath ".gitignore"
        if (-not (Test-Path -LiteralPath $gitIgnorePath -PathType Leaf)) {
            throw ".gitignore nao encontrado"
        }
        $gitIgnoreContent = [System.IO.File]::ReadAllText($gitIgnorePath)
        if ($gitIgnoreContent -notmatch "build/") { throw "Padrao build/ ausente em .gitignore" }
        if ($gitIgnoreContent -notmatch "\*\.exe") { throw "Padrao *.exe ausente em .gitignore" }
        if ($gitIgnoreContent -notmatch "\.vscode/\*") { throw "Padrao .vscode/* ausente em .gitignore" }
        if ($gitIgnoreContent -notmatch "!\.vscode/launch\.json" -or $gitIgnoreContent -notmatch "!\.vscode/settings\.json") {
            throw ".gitignore nao preservou !.vscode/launch.json e !.vscode/settings.json"
        }
        return $true
    }

    Invoke-Test "Todos os arquivos gerados estao estritamente em UTF-8 sem BOM e LF" {
        $generatedFiles = @(
            "CMakeLists.txt",
            ".gitignore",
            "src/main.cpp",
            ".vscode/launch.json",
            ".vscode/settings.json"
        )
        foreach ($rel in $generatedFiles) {
            $fullPath = Join-Path -Path $projectDir -ChildPath $rel
            if (Test-HasUtf8Bom -Path $fullPath) {
                throw "BOM UTF-8 detectado no arquivo gerado: $rel"
            }
            $rawText = [System.IO.File]::ReadAllText($fullPath)
            if ($rawText.Contains("`r`n")) {
                throw "CRLF detectado no arquivo gerado (esperado LF padronizado): $rel"
            }
        }
        return $true
    }

    # --------------------------------------------------------------------------
    # 4. Compatibilidade Cruzada de Interpretadores e Compilacao Real CMake
    # --------------------------------------------------------------------------
    Write-SectionHeader "4. Compatibilidade Windows PowerShell 5.1 e Integracao CMake"

    $winPs51Cmd = if ($isWin) { Get-Command "powershell.exe" -ErrorAction SilentlyContinue } else { $null }
    if ($isWin -and ($null -ne $winPs51Cmd)) {
        Invoke-Test "Execucao real no Windows PowerShell 5.1 funciona sem erro de parsing nem BOM" {
            $ps51Project = "ProjetoWinPS51"
            $res = Invoke-BootstrapIsolated -ScriptArguments @($ps51Project) -WorkingDirectory $tempRoot -ShellExecutable $winPs51Cmd.Source
            if ($res.ExitCode -ne 0) {
                throw "Windows PowerShell 5.1 falhou com exit code $($res.ExitCode). StdErr: $($res.StdErr)"
            }
            if ($res.StdOut -match '(^|[^A-Za-z])e\[\d+m') {
                throw "Vazamento de sequencia ANSI literal 'e[...m' detectado no Windows PowerShell 5.1"
            }

            $ps51Dir = Join-Path -Path $tempRoot -ChildPath $ps51Project
            foreach ($rel in @("CMakeLists.txt", ".gitignore", "src/main.cpp", ".vscode/launch.json", ".vscode/settings.json")) {
                if (Test-HasUtf8Bom -Path (Join-Path -Path $ps51Dir -ChildPath $rel)) {
                    throw "Windows PowerShell 5.1 gravou BOM UTF-8 em $rel"
                }
            }
            return $true
        }
    }
    else {
        Invoke-Skip "Execucao real no Windows PowerShell 5.1" "Disponivel apenas no Windows"
    }

    $hasCmake    = $null -ne (Get-Command "cmake" -ErrorAction SilentlyContinue)
    $hasCompiler = ($null -ne (Get-Command "g++" -ErrorAction SilentlyContinue)) -or
                   ($null -ne (Get-Command "clang++" -ErrorAction SilentlyContinue)) -or
                   ($null -ne (Get-Command "cl" -ErrorAction SilentlyContinue))

    if ($IncludeBuild) {
        if (-not $hasCmake -or -not $hasCompiler) {
            Invoke-Skip "Compilacao real com CMake e C++23" "CMake ou compilador C++ (g++, clang++, cl) ausente no PATH"
            Invoke-Skip "Linkagem e execucao C++23 (<print> / <stacktrace>) via stdc++exp" "CMake ou compilador C++ ausente no PATH"
        }
        else {
            Invoke-Test "Projeto gerado configura (respeitando CMAKE_BUILD_TYPE) e compila em C++23" {
                $buildDir = Join-Path -Path $projectDir -ChildPath "build"
                $configured = $false

                # No Windows com g++ e mingw32-make/make, tenta MinGW Makefiles primeiro
                $hasMake = ($null -ne (Get-Command "mingw32-make" -ErrorAction SilentlyContinue)) -or
                           ($null -ne (Get-Command "make" -ErrorAction SilentlyContinue))

                if ($isWin -and $hasMake -and ($null -ne (Get-Command "g++" -ErrorAction SilentlyContinue))) {
                    $cfgMinGW = Invoke-ExternalProcess -FilePath "cmake" -ArgumentList @("-B", "build", "-G", "MinGW Makefiles", "-DCMAKE_BUILD_TYPE=Release") -WorkingDirectory $projectDir
                    if ($cfgMinGW.ExitCode -eq 0) {
                        $configured = $true
                    }
                    elseif (Test-Path -LiteralPath $buildDir) {
                        # Limpa CMakeCache.txt parcial antes do fallback para evitar generator mismatch
                        Remove-Item -LiteralPath $buildDir -Recurse -Force
                    }
                }

                if (-not $configured) {
                    $cfgDefault = Invoke-ExternalProcess -FilePath "cmake" -ArgumentList @("-B", "build", "-DCMAKE_BUILD_TYPE=Release") -WorkingDirectory $projectDir
                    if ($cfgDefault.ExitCode -ne 0) {
                        throw "cmake -B build falhou (exit $($cfgDefault.ExitCode)): $($cfgDefault.StdErr)"
                    }
                }

                # Verifica se -DCMAKE_BUILD_TYPE=Release nao foi sobrescrito por Debug em geradores single-config
                $cachePath = Join-Path -Path $buildDir -ChildPath "CMakeCache.txt"
                if (Test-Path -LiteralPath $cachePath -PathType Leaf) {
                    $cacheText = [System.IO.File]::ReadAllText($cachePath)
                    if ($cacheText -notmatch "CMAKE_CONFIGURATION_TYPES" -and $cacheText -notmatch "CMAKE_BUILD_TYPE:STRING=Release") {
                        throw "CMakeLists.txt sobrescreveu -DCMAKE_BUILD_TYPE=Release indevidamente"
                    }
                }

                $buildRes = Invoke-ExternalProcess -FilePath "cmake" -ArgumentList @("--build", "build", "--config", "Release") -WorkingDirectory $projectDir
                if ($buildRes.ExitCode -ne 0) {
                    throw "cmake --build build falhou (exit $($buildRes.ExitCode)): $($buildRes.StdErr)"
                }

                $binName = if ($isWin) { "$validProject.exe" } else { $validProject }
                $candidates = @(
                    (Join-Path -Path $buildDir -ChildPath $binName),
                    (Join-Path -Path $buildDir -ChildPath "Release/$binName"),
                    (Join-Path -Path $buildDir -ChildPath "Debug/$binName")
                )
                $foundBin = @($candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf })
                if ($foundBin.Count -eq 0) {
                    throw "Binario compilado nao encontrado em: $($candidates -join ', ')"
                }

                return $true
            }

            Invoke-Test "Linkagem e execucao C++23 (<print> / <stacktrace>) funcionam sem erro de linker (stdc++exp)" {
                $buildDir = Join-Path -Path $projectDir -ChildPath "build"
                $mainCpp  = Join-Path -Path $projectDir -ChildPath "src/main.cpp"

                $cpp23Source = @'
#include <iostream>
#if __has_include(<print>)
#include <print>
#endif
#if __has_include(<stacktrace>)
#include <stacktrace>
#endif

int main() {
#if defined(__cpp_lib_print)
    std::println("C++23 std::println ativo");
#else
    std::cout << "C++23 padrao ativo\n";
#endif
#if defined(__cpp_lib_stacktrace)
    auto st = std::stacktrace::current();
    if (st.empty()) {
        return 1;
    }
#endif
    return 0;
}
'@
                $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
                [System.IO.File]::WriteAllText($mainCpp, ($cpp23Source -replace "`r`n", "`n"), $utf8NoBom)

                $rebuildRes = Invoke-ExternalProcess -FilePath "cmake" -ArgumentList @("--build", "build", "--clean-first", "--config", "Release") -WorkingDirectory $projectDir
                if ($rebuildRes.ExitCode -ne 0) {
                    throw "Falha ao linkar recursos C++23 (<print>/<stacktrace>): $($rebuildRes.StdErr)"
                }

                $binName = if ($isWin) { "$validProject.exe" } else { $validProject }
                $candidates = @(
                    (Join-Path -Path $buildDir -ChildPath $binName),
                    (Join-Path -Path $buildDir -ChildPath "Release/$binName"),
                    (Join-Path -Path $buildDir -ChildPath "Debug/$binName")
                )
                $exePath = @($candidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf })[0]
                if (-not $exePath) {
                    throw "Executavel C++23 nao encontrado apos rebuild"
                }

                $runRes = Invoke-ExternalProcess -FilePath $exePath -ArgumentList @() -WorkingDirectory $projectDir
                if ($runRes.ExitCode -ne 0) {
                    throw "Execucao do binario C++23 falhou com exit code $($runRes.ExitCode)"
                }
                if ($runRes.StdOut -notmatch "C\+\+23") {
                    throw "Saida esperada 'C++23...' nao encontrada: $($runRes.StdOut)"
                }

                return $true
            }
        }
    }
    else {
        Invoke-Skip "Compilacao real com CMake e C++23" "Passe -IncludeBuild para executar este teste"
        Invoke-Skip "Linkagem e execucao C++23 (<print> / <stacktrace>) via stdc++exp" "Passe -IncludeBuild para executar este teste"
    }
}
finally {
    $startTime.Stop()

    if (Test-Path -LiteralPath $tempRoot) {
        try {
            Remove-Item -LiteralPath $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
        }
        catch {
            # Ignora lock transitorio no cleanup do diretorio temporario
        }
    }
}

#=========================== [ RESUMO DA EXECUCAO ] ============================

Write-Host ""
Write-Host $divider
Write-Host "$BOLD[RESUMO DA EXECUCAO DOS TESTES]$RESET"
Write-Host $divider
Write-Host "  Total de testes : $script:TotalTests"
Write-Host "  $GREEN Aprovados       : $script:PassedTests$RESET"
if ($script:FailedTests -gt 0) {
    Write-Host "  $RED Falhos          : $script:FailedTests$RESET"
}
else {
    Write-Host "  $GRAY Falhos          : 0$RESET"
}
if ($script:SkippedTests -gt 0) {
    Write-Host "  $YELLOW Ignorados       : $script:SkippedTests$RESET"
}
Write-Host "  Tempo total     : $($startTime.Elapsed.TotalSeconds.ToString('F2'))s"
Write-Host $divider
Write-Host ""

if ($script:FailedTests -gt 0) {
    Write-Host "$RED[FAIL] A suite de testes falhou!$RESET"
    exit 1
}
else {
    Write-Host "$GREEN[PASS] Todos os testes passaram com sucesso!$RESET"
    exit 0
}
