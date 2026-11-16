# Outline — `validator`

- **Header:** [validator.hpp](../../include/validator.hpp)
- **Implementação:** [validator.cpp](../../src/validator.cpp)
- **Namespace:** `bootstrap`
- **Propósito:** Validação uniforme (multiplataforma e CMake) do nome do projeto em 5 regras.

## API Pública (`validator.hpp` / `validator.cpp`)

- `test_project_name(name, base_dir, silent = false)` — valida o nome e imprime diagnósticos via `bootstrap::term` (quando `!silent`):
  1. Nome vazio ou composto apenas por espaços.
  2. Nome terminando com ponto (`.`).
  3. Nomes reservados de dispositivos do Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1..9`, `LPT1..9`).
  4. Padrão `^[A-Za-z0-9_][A-Za-z0-9_-]*$` (sem espaços e sem iniciar com `-`).
  5. Colisão com pasta ou arquivo existente em `base_dir / name`.

## Helpers Internos (`validator.cpp`, `namespace` anônimo)

- `is_ascii_alpha`, `is_ascii_digit`, `is_allowed_start`, `is_allowed_body`, `to_ascii_upper`, `is_all_whitespace`.
- `is_windows_reserved_name(name)` — extrai prefixo antes de `.` em maiúsculas e compara com os 22 nomes reservados.
- `format_invalid_chars(name)` — lista caracteres proibidos únicos (`<espaco>`, `<ctrl>`) ou `"inicio com '-'"`.
