# Outline — `generator`

- **Arquivos:** [generator.hpp](../../include/generator.hpp) | [generator.cpp](../../src/generator.cpp)
- **Propósito:** Criação transacional dos diretórios e escrita em UTF-8 sem BOM + LF com rollback RAII.

## `Gerador::Seguranca`
- `class DirectoryRollbackGuard` *(reexportada em `Gerador`)* — armazena `CaminhoRaiz`, remove recursivamente no destrutor se `!Confirmado`, com `confirmar()` e `foiConfirmado()`.

## `Gerador::Arquivo`
- `normalizarQuebrasLinha(Conteudo)` — converte `\r\n` em `\n` e garante newline final.
- `gravarUtf8SemBom(CaminhoArq, Conteudo)` — grava em `std::ofstream` binário.

## `Gerador::Projeto`
- `criarSubdiretorios(RaizProjeto)` — cria `src/`, `include/` e `.vscode/`.
- `gerarArquivosPadrao(RaizProjeto, NomeProjeto)` — grava os 5 arquivos via `Templates`.
- `criarProjeto(DirBase, NomeProjeto, ErroSaida = nullptr)` — orquestra criação, rollback RAII e retorna `bool`.
