# 06 — Guia de Manutenção

[← Índice](00_indice.md)

Receitas para as mudanças mais comuns. Depois de **qualquer** alteração, rode a verificação da seção 6.

## 1. Adicionar uma nova regra de validação

Arquivo: `src/validator.cpp`, função `test_project_name()`.

1. Adicione um novo bloco numerado na ordem desejada, seguindo o padrão dos existentes:

   ```cpp
   // 6 - Descricao curta da nova regra
   if (condicao_invalida) {
       if (!silent) {
           term::print_separator();
           term::write_error("[ERRO] Nome invalido: '" + std::string(name) + "'");
           term::write_warn("Explicacao do problema.");
           term::print_separator();
       }
       return false;
   }
   ```

2. Mantenha a regra de colisão com pasta existente por último (ela acessa o disco).
3. Adicione um teste unitário em `tests/test_cpp_bootstrap.cpp` usando `test_project_name(..., temp_root, true)`.
4. Atualize a seção de validações do `README.md` e de [03 — Arquitetura](03_arquitetura_cpp23.md).

## 2. Alterar um template gerado

Arquivo: `include/templates.hpp` (única fonte de verdade).

- Edite a constante correspondente (`kMainCpp`, `kCmakeLists` etc.) dentro de `R"tpl( ... )tpl"`.
- Não é preciso escapar aspas nem barras dentro do raw string. Só evite a sequência `)tpl"` no conteúdo.
- O nome do projeto entra no CMake pelo marcador `__PROJECT_NAME__`, substituído por `render_cmake_lists()`.
- Ajuste os testes 16 a 19 se mudar algo que eles verificam.
- Atualize a tabela de arquivos gerados no `README.md` e no `AGENTS.md`.

## 3. Adicionar um novo arquivo gerado

1. Crie a constante em `include/templates.hpp`.
2. Em `src/generator.cpp` (`create_project()`), crie a pasta se necessário e chame `write_utf8_no_bom(caminho, tpl::kNovoArquivo)` **antes** de `rollback_guard.commit()`.
3. Inclua o arquivo na lista auditada pelo teste 19 (BOM/CRLF).
4. Atualize a árvore de estrutura gerada no `README.md` e no `AGENTS.md`.

## 4. Adicionar uma nova flag de CLI

1. Em `include/cli.hpp`, adicione o campo em `CliOptions` (ou um novo valor em `CliAction`).
2. Em `src/cli.cpp`, crie um `is_<flag>_flag()` e trate-o no laço de `parse_cli()` **antes** da checagem de flags desconhecidas.
3. Documente a flag em `print_help()`.
4. Use o novo campo em `src/main.cpp`.
5. Adicione casos ao teste 07 (unitário) e, se fizer sentido, um teste de integração.

## 5. Lançar uma nova versão

1. Atualize `kVersion` em `include/terminal.hpp` e `VERSION` em `project(...)` no `CMakeLists.txt` (devem ser iguais).
2. Atualize o `README.md` se houver mudanças visíveis.
3. Faça commit, push e espere o CI ficar verde.
4. Crie a tag: `git tag -a vX.Y.Z -m "release: cpp-bootstrap vX.Y.Z"` e `git push origin vX.Y.Z`.

Sugestão de versionamento semântico: `PATCH` para correções, `MINOR` para flags ou templates novos compatíveis, `MAJOR` para mudanças que alteram o que já era gerado.

## 6. Verificação obrigatória antes de cada commit

```powershell
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/test_cpp_bootstrap.exe --target-exe ./build/cpp-bootstrap.exe --include-build
```

Checklist:

- [ ] Compila **sem nenhum aviso** (as flags incluem `-pedantic-errors`).
- [ ] 20/20 testes aprovados com `--include-build`.
- [ ] Arquivos `.hpp`, `.cpp` e `CMakeLists.txt` só com ASCII 7-bit.
- [ ] Todos os arquivos em UTF-8 sem BOM e LF.
- [ ] `README.md` e `AGENTS.md` refletem o que o binário produz.
- [ ] Pasta `build/` não foi commitada.

Para auditar BOM, CRLF e caracteres não-ASCII no PowerShell:

```powershell
Get-ChildItem include, src, tests -Recurse -File | ForEach-Object {
    $b = [IO.File]::ReadAllBytes($_.FullName)
    $bom = $b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF
    "{0,-40} BOM={1} CR={2} NonASCII={3}" -f $_.Name, $bom, ($b -contains 13), ($b | Where-Object { $_ -gt 127 }).Count
}
```

## 7. Armadilhas conhecidas

| Armadilha | Como evitar |
| :--- | :--- |
| Gravar arquivos sem `std::ios::binary` no Windows | Sempre usar `write_utf8_no_bom()` |
| Trocar o diretório do processo | Montar caminhos com `base_dir / nome`; nunca chamar `current_path(novo)` |
| Esquecer `commit()` no guard | O projeto inteiro é apagado ao sair do escopo; `commit()` deve ser a última linha antes do `return true` |
| Usar `std::println` sem guarda | Usar `term::print_line()`, que tem fallback para `std::cout` |
| Erro de linker com `<print>` / `<stacktrace>` | A Seção 9 do `CMakeLists.txt` já liga `stdc++exp` quando disponível |
| Nome de executável em geradores multi-config | No MSVC o binário fica em `build/Release/`; nos demais, em `build/` |
