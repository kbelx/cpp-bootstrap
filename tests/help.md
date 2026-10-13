  ### 1. Como Funciona o Ambiente de Testes

O ambiente valida o gerador cpp_bootstrap.ps1 de forma não-destrutiva e isolada:

- Isolamento em Sandbox ($env:TEMP): Cada execução gera uma pasta temporária com GUID exclusivo onde os testes são realizados. Nenhum arquivo de teste é criado dentro do repositório ou no diretório onde você invocou o comando.

- Execução em Subprocesso: O script alvo é executado em processos filhos isolados (System.Diagnostics.Process),
  garantindo que o Set-Location interno do cpp_bootstrap.ps1 nunca altere o diretório de trabalho do seu terminal.

- Teardown Automático (finally): Ao concluir ou em caso de erro, a pasta temporária é limpa automaticamente sem
  deixar resíduos.

- Compatibilidade Multiplataforma: Suporta nativamente PowerShell 7+ (pwsh) e Windows PowerShell 5.1 (com
  delegação automática transparente para pwsh quando disponível).
  ──────
  ### 2. O Que é Testado (14 Cenários)

  1. Sintaxe do Script: Validação estática da AST do PowerShell via [System.Management.Automation.Language.Parser].
  2. Casos de Validação e Segurança (Erros Esperados):
      • Execução sem argumentos (exibe mensagem de ajuda/tutorial e retorna código 1).
      • Nome vazio ou somente com espaços (retorna código 2).
      • Caracteres proibidos como < > : " / \ | ? * (retorna código 2).
      • Nome terminando com ponto final . ou espaço (regra Windows, código 2).
      • Nomes reservados do sistema como CON, NUL, AUX (código 2).
      • Pasta já existente (bloqueia sobrescrita acidental e retorna código 2).
  3. Criação e Integridade da Estrutura (Sucesso):
      • Retorno com código 0 e mensagem de sucesso.
      • Criação das pastas src/, include/ e .vscode/.
      • src/main.cpp: valida presença de int main(), #include <iostream> e std::setlocale.
      • .vscode/launch.json e .vscode/settings.json: validação da sintaxe JSON e chaves obrigatórias.
      • CMakeLists.txt: valida substituição do nome do projeto, C++23 estrito e flags de compilação.
      • .gitignore: valida inclusão dos filtros essenciais (build/, *.exe).
  4. Integração Real (CMake + Compilador C++23):
      • Valida opcionalmente se o projeto gerado configura (cmake -B build) e compila com sucesso (cmake --build
      build) gerando o executável final com o GCC/G++ do MSYS2.

  ──────
  ### 3. Como Usar

  #### Execução Padrão

  Executa a validação de sintaxe, regras de segurança e geração estrutural dos artefatos:

    ./tests/test_cpp_bootstrap.ps1

  (ou pwsh ./tests/test_cpp_bootstrap.ps1 no Linux/macOS)

  #### Execução com Compilação Real C++23 (CMake + g++)

  Adicione a flag -IncludeBuild para que o teste gere um projeto real e realize a compilação completa de ponta a
  ponta:

    ./tests/test_cpp_bootstrap.ps1 -IncludeBuild

  #### Modo Detalhado (Verbose)

  Para inspecionar todo o stdout e stderr emitido em cada subprocesso:

    ./tests/test_cpp_bootstrap.ps1 -VerboseOutput
  ──────
  ### 4. Atualizações Realizadas no Repositório

  • Criada a pasta  e o script test_cpp_bootstrap.ps1 em UTF-8 sem BOM.
  • Adicionada a seção 6. Ambiente de Testes Automatizados no README.md.
  • Commit e push realizados na branch main no GitHub.