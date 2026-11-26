# Outline — `terminal`

- **Header:** [terminal.hpp](../../include/terminal.hpp) *(header-only, utiliza `Cores` de [cores.hpp](../../include/cores.hpp))*
- **Propósito:** Detecção de SO, suporte ANSI/VT e saída colorida no terminal.

## `Terminal::Ambiente`
- `detectarSO()` — retorna `"Windows"`, `"macOS"` ou `"Linux"` em tempo de compilação.
- `possuiVarSemCor()` — verifica `NO_COLOR`.
- `configTerminalAnsi()` — checa `stdout` e ativa `ENABLE_VIRTUAL_TERMINAL_PROCESSING` no Windows.
- `deveUsarAnsi()` — resultado em cache estático (`SUPORTA_ANSI`).

## `Terminal::Saida`
- `imprimirLinha(Texto)` — imprime via `std::println` (C++23) ou `std::cout`.
- `colorirTexto(Cor, Mensagem)` — aplica cor + `RESET` quando `deveUsarAnsi()` for `true`.
- `imprimirColorido(Cor, Mensagem)` — imprime linha já colorida.
- `separador()` / `escreveInfo()` / `escreveAviso()` / `escreveErro()` — saídas padronizadas.
