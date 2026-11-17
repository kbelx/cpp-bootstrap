# Outline — `validator`

- **Arquivos:** [validator.hpp](../../include/validator.hpp) | [validator.cpp](../../src/validator.cpp)
- **Propósito:** Validação multiplataforma do nome do projeto em 5 regras.

## `Validadores` (`validator.hpp` / `validator.cpp`)
- `validarNomeProjeto(Nome, DirBase, Silencioso = false)`:
  1. Vazio ou apenas espaços.
  2. Terminando com ponto (`.`).
  3. Reservado no Windows (`CON`, `PRN`, `AUX`, `NUL`, `COM1..9`, `LPT1..9`).
  4. Padrão `^[A-Za-z0-9_][A-Za-z0-9_-]*$`.
  5. Colisão em `DirBase / Nome`.

## Sub-namespaces Internos (`validator.cpp`)
- **`Validadores::Caracteres`:** `ehLetraAscii`, `ehDigitoAscii`, `ehInicioPermitido`, `ehCorpoPermitido`, `paraMaiusculoAscii`, `apenasEspacos`.
- **`Validadores::Regras`:** `ehNomeReservadoWin`, `possuiPadraoValido`, `formatarCharsInvalidos`.
- **`Validadores::Diagnostico`:** `emitirErroVazio`, `emitirErroPontoFinal`, `emitirErroReservado`, `emitirErroPadrao`, `emitirErroColisao`.
