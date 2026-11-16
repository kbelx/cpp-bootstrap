# Outline — `terminal`

- **Header:** [terminal.hpp](../../include/terminal.hpp) *(header-only)*
- **Namespace:** `bootstrap::term`
- **Propósito:** Detecção de SO, suporte a cores ANSI/VT e funções de saída no terminal.

## Constantes e Estruturas

- `kVersion`: `"1.0.0"` (`constexpr std::string_view`).
- `struct Palette`: guarda `enabled` (`bool`) e os códigos ANSI (`esc`, `reset`, `green`, `cyan`, `blue`, `gray`, `yellow`, `red`, `bold`).

## Funções

- `detect_os_label()` — retorna `"Windows"`, `"macOS"` ou `"Linux"` em tempo de compilação.
- `has_no_color_env()` — verifica se `NO_COLOR` está definida e não vazia.
- `should_use_ansi()` — checa `NO_COLOR`, `_isatty`/`isatty` em `stdout` e ativa `ENABLE_VIRTUAL_TERMINAL_PROCESSING` no Windows.
- `palette()` — cache estático da `Palette` ativa (vazia quando ANSI está desabilitado).
- `print_line(text)` — imprime linha com `std::println` (se `__cpp_lib_print`) ou fallback `std::cout`.
- `print_separator()` — imprime linha divisória cinza (`52` hífens).
- `write_colored(color, message)` — aplica cor + `reset` e chama `print_line`.
- `write_info(message)` / `write_warn(message)` / `write_error(message)` — saídas em ciano, amarelo e vermelho.
