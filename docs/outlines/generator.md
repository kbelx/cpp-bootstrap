# Outline — `generator`

- **Header:** [generator.hpp](../../include/generator.hpp)
- **Implementação:** [generator.cpp](../../src/generator.cpp)
- **Namespace:** `bootstrap`
- **Propósito:** Criação transacional dos diretórios e escrita dos arquivos em UTF-8 sem BOM + LF, com rollback RAII.

## Classes e Funções Públicas

- `class DirectoryRollbackGuard`:
  - Construtor armazena `root_path_`; não copiável nem movível.
  - `~DirectoryRollbackGuard() noexcept` — remove `root_path_` recursivamente se `!committed_`.
  - `commit()` / `is_committed()` — desarma o rollback após geração completa.
- `write_utf8_no_bom(file_path, content)` — normaliza `\r\n` para `\n`, garante newline final e grava em `std::ofstream` binário (`std::ios::binary | std::ios::trunc`).
- `create_project(base_dir, project_name, out_error = nullptr)` — cria `<project_name>`, arma `DirectoryRollbackGuard`, cria `src/`, `include/`, `.vscode/`, grava os 5 arquivos via `bootstrap::tpl` e chama `commit()`. Retorna `false` e preenche `out_error` em falhas de I/O.
