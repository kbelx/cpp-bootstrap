#include "cli.hpp"
#include "generator.hpp"
#include "terminal.hpp"
#include "validator.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

//=========================== [ INFRAESTRUTURA DE TESTES ] =====================

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

struct TestStats {
    int total   = 0;
    int passed  = 0;
    int failed  = 0;
    int skipped = 0;
};

bool g_verbose = false;

void print_divider() {
    const auto& p = bootstrap::term::palette();
    bootstrap::term::print_line(
        std::string(p.gray) + "------------------------------------------------------------" + std::string(p.reset)
    );
}

void print_section_header(std::string_view title) {
    const auto& p = bootstrap::term::palette();
    bootstrap::term::print_line("");
    print_divider();
    bootstrap::term::print_line(std::string(p.bold) + std::string(p.cyan) + std::string(title) + std::string(p.reset));
    print_divider();
}

std::string format_test_number(int n) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << n;
    return oss.str();
}

void invoke_test(TestStats& stats, std::string_view name, const std::function<void()>& assertion) {
    ++stats.total;
    const std::string num = format_test_number(stats.total);
    const auto& p = bootstrap::term::palette();

    try {
        assertion();
        ++stats.passed;
        bootstrap::term::print_line(
            "  " + std::string(p.green) + "[PASS]" + std::string(p.reset) + " Teste " + num + ": " + std::string(name)
        );
    }
    catch (const std::exception& ex) {
        ++stats.failed;
        bootstrap::term::print_line(
            "  " + std::string(p.red) + "[FAIL]" + std::string(p.reset) + " Teste " + num + ": " + std::string(name)
        );
        bootstrap::term::print_line(
            "         " + std::string(p.red) + "-> Erro:" + std::string(p.reset) + " " + ex.what()
        );
    }
}

void invoke_skip(TestStats& stats, std::string_view name, std::string_view reason) {
    ++stats.total;
    ++stats.skipped;
    const std::string num = format_test_number(stats.total);
    const auto& p = bootstrap::term::palette();
    bootstrap::term::print_line(
        "  " + std::string(p.yellow) + "[SKIP]" + std::string(p.reset) +
        " Teste " + num + ": " + std::string(name) + " (" + std::string(reason) + ")"
    );
}

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(std::string(message));
    }
}

bool contains(std::string_view haystack, std::string_view needle) {
    return haystack.find(needle) != std::string_view::npos;
}

std::vector<unsigned char> read_all_bytes(const fs::path& path) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    require(ifs.is_open(), "Nao foi possivel abrir arquivo: " + path.string());
    const std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    if (size > 0) {
        require(
            static_cast<bool>(ifs.read(reinterpret_cast<char*>(bytes.data()), size)),
            "Falha ao ler bytes de: " + path.string()
        );
    }
    return bytes;
}

std::string read_all_text(const fs::path& path) {
    const auto bytes = read_all_bytes(path);
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

bool has_utf8_bom(const fs::path& path) {
    const auto bytes = read_all_bytes(path);
    return bytes.size() >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF;
}

class TempSandboxGuard {
public:
    TempSandboxGuard() {
        std::random_device rd;
        std::mt19937_64 rng(rd());
        const auto suffix = rng();
        std::ostringstream oss;
        oss << "cpp_bootstrap_tests_" << std::hex << suffix;
        root_ = fs::temp_directory_path() / oss.str();
        fs::create_directories(root_);
    }

    ~TempSandboxGuard() noexcept {
        if (!root_.empty()) {
            std::error_code ec;
            fs::remove_all(root_, ec);
        }
    }

    TempSandboxGuard(const TempSandboxGuard&) = delete;
    TempSandboxGuard& operator=(const TempSandboxGuard&) = delete;

    [[nodiscard]] const fs::path& path() const noexcept {
        return root_;
    }

private:
    fs::path root_;
};

#if defined(_WIN32)

std::string quote_win32_arg(std::string_view arg) {
    if (!arg.empty() && arg.find_first_of(" \t\n\v\"") == std::string_view::npos) {
        return std::string(arg);
    }

    std::string quoted;
    quoted.push_back('"');
    for (std::size_t i = 0; i < arg.size(); ++i) {
        std::size_t backslashes = 0;
        while (i < arg.size() && arg[i] == '\\') {
            ++backslashes;
            ++i;
        }
        if (i == arg.size()) {
            quoted.append(backslashes * 2, '\\');
            break;
        }
        if (arg[i] == '"') {
            quoted.append(backslashes * 2 + 1, '\\');
            quoted.push_back('"');
        }
        else {
            quoted.append(backslashes, '\\');
            quoted.push_back(arg[i]);
        }
    }
    quoted.push_back('"');
    return quoted;
}

ProcessResult run_process(const std::vector<std::string>& args, const fs::path& working_dir) {
    require(!args.empty(), "Lista de argumentos vazia para run_process");

    std::string cmdline;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            cmdline.push_back(' ');
        }
        cmdline.append(quote_win32_arg(args[i]));
    }

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    HANDLE stdout_read = nullptr;
    HANDLE stdout_write = nullptr;
    HANDLE stderr_read = nullptr;
    HANDLE stderr_write = nullptr;

    require(CreatePipe(&stdout_read, &stdout_write, &sa, 0) != 0, "Falha em CreatePipe stdout");
    require(SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0) != 0, "Falha em SetHandleInformation stdout");
    require(CreatePipe(&stderr_read, &stderr_write, &sa, 0) != 0, "Falha em CreatePipe stderr");
    require(SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0) != 0, "Falha em SetHandleInformation stderr");

    HANDLE nul_in = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    STARTUPINFOA si{};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = (nul_in != INVALID_HANDLE_VALUE) ? nul_in : GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = stdout_write;
    si.hStdError = stderr_write;

    PROCESS_INFORMATION pi{};
    const std::string cwd_str = working_dir.string();
    std::string mutable_cmd = cmdline;

    const BOOL ok = CreateProcessA(
        nullptr,
        mutable_cmd.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        cwd_str.c_str(),
        &si,
        &pi
    );

    CloseHandle(stdout_write);
    CloseHandle(stderr_write);
    if (nul_in != INVALID_HANDLE_VALUE) {
        CloseHandle(nul_in);
    }

    if (ok == 0) {
        CloseHandle(stdout_read);
        CloseHandle(stderr_read);
        throw std::runtime_error("CreateProcessA falhou para: " + cmdline);
    }

    ProcessResult result{};
    auto drain_pipe = [](HANDLE h, std::string& target) {
        char buf[4096];
        DWORD read_bytes = 0;
        while (ReadFile(h, buf, static_cast<DWORD>(sizeof(buf)), &read_bytes, nullptr) != 0 && read_bytes > 0) {
            target.append(buf, static_cast<std::size_t>(read_bytes));
        }
    };

    std::thread out_thread(drain_pipe, stdout_read, std::ref(result.stdout_text));
    std::thread err_thread(drain_pipe, stderr_read, std::ref(result.stderr_text));

    WaitForSingleObject(pi.hProcess, INFINITE);
    out_thread.join();
    err_thread.join();

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    result.exit_code = static_cast<int>(exit_code);

    CloseHandle(stdout_read);
    CloseHandle(stderr_read);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (g_verbose) {
        if (!result.stdout_text.empty()) {
            bootstrap::term::print_line("[STDOUT] " + result.stdout_text);
        }
        if (!result.stderr_text.empty()) {
            bootstrap::term::print_line("[STDERR] " + result.stderr_text);
        }
    }
    return result;
}

bool command_exists_in_path(std::string_view cmd) {
    return SearchPathA(nullptr, std::string(cmd).c_str(), ".exe", 0, nullptr, nullptr) != 0;
}

#else

ProcessResult run_process(const std::vector<std::string>& args, const fs::path& working_dir) {
    require(!args.empty(), "Lista de argumentos vazia para run_process");

    int stdout_pipe[2]{-1, -1};
    int stderr_pipe[2]{-1, -1};
    require(pipe(stdout_pipe) == 0, "Falha ao criar pipe stdout");
    require(pipe(stderr_pipe) == 0, "Falha ao criar pipe stderr");

    const pid_t pid = fork();
    require(pid >= 0, "Falha ao executar fork()");

    if (pid == 0) {
        if (chdir(working_dir.c_str()) != 0) {
            _exit(126);
        }

        const int devnull = open("/dev/null", O_RDONLY);
        if (devnull >= 0) {
            dup2(devnull, STDIN_FILENO);
            close(devnull);
        }

        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);

        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        std::vector<char*> c_args;
        c_args.reserve(args.size() + 1);
        for (const auto& a : args) {
            c_args.push_back(const_cast<char*>(a.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());
        _exit(127);
    }

    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    ProcessResult result{};
    auto drain_fd = [](int fd, std::string& target) {
        char buf[4096];
        for (;;) {
            const ssize_t n = read(fd, buf, sizeof(buf));
            if (n <= 0) {
                break;
            }
            target.append(buf, static_cast<std::size_t>(n));
        }
    };

    std::thread out_thread(drain_fd, stdout_pipe[0], std::ref(result.stdout_text));
    std::thread err_thread(drain_fd, stderr_pipe[0], std::ref(result.stderr_text));

    int status = 0;
    waitpid(pid, &status, 0);
    out_thread.join();
    err_thread.join();

    close(stdout_pipe[0]);
    close(stderr_pipe[0]);

    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    }
    else {
        result.exit_code = -1;
    }

    if (g_verbose) {
        if (!result.stdout_text.empty()) {
            bootstrap::term::print_line("[STDOUT] " + result.stdout_text);
        }
        if (!result.stderr_text.empty()) {
            bootstrap::term::print_line("[STDERR] " + result.stderr_text);
        }
    }
    return result;
}

bool command_exists_in_path(std::string_view cmd) {
    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        return false;
    }
    std::stringstream ss(path_env);
    std::string dir;
    while (std::getline(ss, dir, ':')) {
        if (dir.empty()) {
            continue;
        }
        const fs::path candidate = fs::path(dir) / std::string(cmd);
        std::error_code ec;
        if (fs::exists(candidate, ec) && access(candidate.c_str(), X_OK) == 0) {
            return true;
        }
    }
    return false;
}

#endif

ProcessResult invoke_bootstrap(
    const fs::path& exe_path,
    const std::vector<std::string>& extra_args,
    const fs::path& working_dir
) {
    std::vector<std::string> full_args;
    full_args.reserve(1 + extra_args.size());
    full_args.push_back(exe_path.string());
    full_args.insert(full_args.end(), extra_args.begin(), extra_args.end());
    return run_process(full_args, working_dir);
}

}  // namespace

//=========================== [ PONTO DE ENTRADA ] =============================

int main(int argc, char* argv[]) {
    fs::path target_exe;
    bool include_build = false;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--target-exe" && (i + 1) < argc) {
            target_exe = fs::weakly_canonical(fs::path(argv[++i]));
        }
        else if (arg == "--include-build") {
            include_build = true;
        }
        else if (arg == "--verbose") {
            g_verbose = true;
        }
    }

    if (target_exe.empty()) {
        const fs::path default_dir = fs::path(argv[0]).parent_path();
#if defined(_WIN32)
        target_exe = fs::weakly_canonical(default_dir / "cpp-bootstrap.exe");
#else
        target_exe = fs::weakly_canonical(default_dir / "cpp-bootstrap");
#endif
    }

    if (!fs::exists(target_exe)) {
        bootstrap::term::write_error("[ERRO] Executavel alvo nao encontrado: " + target_exe.string());
        return 1;
    }

    TempSandboxGuard sandbox;
    const fs::path& temp_root = sandbox.path();

    const auto start_tp = std::chrono::steady_clock::now();
    TestStats stats{};

    print_section_header("[TEST SUITE] cpp-bootstrap (C++23)");
    bootstrap::term::print_line("* Executavel alvo  : " + target_exe.string());
    bootstrap::term::print_line("* Ambiente sandbox : " + temp_root.string());
    bootstrap::term::print_line("* Sistema          : " + std::string(bootstrap::term::detect_os_label()));

    // -------------------------------------------------------------------------
    // 1. Testes Unitarios (Logica Pura em Memoria)
    // -------------------------------------------------------------------------
    print_section_header("1. Testes Unitarios de CLI, Normalizacao e Validacao");

    invoke_test(stats, "trim_project_name remove espacos e aspas envolventes", []() {
        require(bootstrap::trim_project_name("  Projeto  ") == "Projeto", "Falha no trim de espacos");
        require(bootstrap::trim_project_name(" \"'Projeto'\" ") == "Projeto", "Falha no trim de aspas");
        require(bootstrap::trim_project_name("   \"\"   ").empty(), "Aspas vazias devem virar string vazia");
    });

    invoke_test(stats, "test_project_name rejeita nomes vazios ou apenas espacos", [&]() {
        require(!bootstrap::test_project_name("", temp_root, true), "String vazia deveria falhar");
        require(!bootstrap::test_project_name("   ", temp_root, true), "Apenas espacos deveria falhar");
    });

    invoke_test(stats, "test_project_name rejeita caracteres proibidos e inicio com '-'", [&]() {
        require(!bootstrap::test_project_name("Projeto<Invalido>", temp_root, true), "< > deveria falhar");
        require(!bootstrap::test_project_name("dir/sub", temp_root, true), "/ deveria falhar");
        require(!bootstrap::test_project_name("-iniciadoComTraco", temp_root, true), "Inicio com '-' deveria falhar");
    });

    invoke_test(stats, "test_project_name rejeita espaco interno (protege CMake)", [&]() {
        require(!bootstrap::test_project_name("Meu Projeto", temp_root, true), "Espaco interno deveria falhar");
    });

    invoke_test(stats, "test_project_name rejeita nome terminando com ponto final", [&]() {
        require(!bootstrap::test_project_name("ProjetoComPonto.", temp_root, true), "Ponto final deveria falhar");
    });

    invoke_test(stats, "test_project_name rejeita nomes reservados do Windows em qualquer SO", [&]() {
        require(!bootstrap::test_project_name("CON", temp_root, true), "CON deveria falhar");
        require(!bootstrap::test_project_name("aux", temp_root, true), "aux deveria falhar");
        require(!bootstrap::test_project_name("NUL.txt", temp_root, true), "NUL.txt deveria falhar");
        require(!bootstrap::test_project_name("COM1", temp_root, true), "COM1 deveria falhar");
        require(!bootstrap::test_project_name("LPT9", temp_root, true), "LPT9 deveria falhar");
    });

    invoke_test(stats, "parse_cli trata sem args, --help, --version, -q e multiplos tokens", []() {
        const char* no_args[] = {"cpp-bootstrap"};
        require(bootstrap::parse_cli(1, no_args).action == bootstrap::CliAction::ShowTutorialError, "Sem args");

        const char* help_args[] = {"cpp-bootstrap", "--help"};
        require(bootstrap::parse_cli(2, help_args).action == bootstrap::CliAction::ShowHelp, "--help");

        const char* ver_args[] = {"cpp-bootstrap", "--version"};
        require(bootstrap::parse_cli(2, ver_args).action == bootstrap::CliAction::ShowVersion, "--version");

        const char* quiet_args[] = {"cpp-bootstrap", "-q", "MeuProj"};
        const auto q_opt = bootstrap::parse_cli(3, quiet_args);
        require(q_opt.action == bootstrap::CliAction::CreateProject && q_opt.quiet && q_opt.project_name == "MeuProj", "-q");

        const char* multi_args[] = {"cpp-bootstrap", "Meu", "Projeto"};
        const auto m_opt = bootstrap::parse_cli(3, multi_args);
        require(m_opt.project_name == "Meu Projeto", "Multiplos tokens devem ser recombinados com espaco");
    });

    // -------------------------------------------------------------------------
    // 2. Integracao CLI e Seguranca (Subprocesso Isolado)
    // -------------------------------------------------------------------------
    print_section_header("2. Integracao CLI, Codigos de Saida e Seguranca");

    invoke_test(stats, "Execucao sem argumentos exibe tutorial e encerra com codigo 1", [&]() {
        const auto res = invoke_bootstrap(target_exe, {}, temp_root);
        require(res.exit_code == 1, "Esperado exit code 1, obtido " + std::to_string(res.exit_code));
        require(contains(res.stdout_text, "Uso:") && contains(res.stdout_text, "Dica:"), "Tutorial ausente");
    });

    invoke_test(stats, "Flags --help e --version encerram com codigo 0", [&]() {
        const auto help_res = invoke_bootstrap(target_exe, {"--help"}, temp_root);
        require(help_res.exit_code == 0, "--help deveria retornar 0");
        require(contains(help_res.stdout_text, "cpp-bootstrap"), "--help sem nome do programa");

        const auto ver_res = invoke_bootstrap(target_exe, {"--version"}, temp_root);
        require(ver_res.exit_code == 0, "--version deveria retornar 0");
        require(contains(ver_res.stdout_text, std::string(bootstrap::term::kVersion)), "--version sem versao");
    });

    invoke_test(stats, "Entradas invalidas na CLI sao rejeitadas com codigo 2", [&]() {
        const auto empty_res = invoke_bootstrap(target_exe, {""}, temp_root);
        require(empty_res.exit_code == 2 && contains(empty_res.stdout_text, "Nome do projeto vazio"), "Entrada vazia");

        const auto bad_res = invoke_bootstrap(target_exe, {"Projeto<Invalido>"}, temp_root);
        require(bad_res.exit_code == 2 && contains(bad_res.stdout_text, "Caracteres proibidos encontrados"), "< >");

        const auto space_res = invoke_bootstrap(target_exe, {"Meu Projeto"}, temp_root);
        require(space_res.exit_code == 2 && contains(space_res.stdout_text, "<espaco>"), "Espaco entre aspas");

        const auto multi_res = invoke_bootstrap(target_exe, {"Meu", "Projeto"}, temp_root);
        require(multi_res.exit_code == 2 && contains(multi_res.stdout_text, "<espaco>"), "Tokens multiplos");

        const auto dot_res = invoke_bootstrap(target_exe, {"ProjetoComPonto."}, temp_root);
        require(dot_res.exit_code == 2 && contains(dot_res.stdout_text, "nao podem terminar com '.'"), "Ponto final");

        const auto con_res = invoke_bootstrap(target_exe, {"CON"}, temp_root);
        require(con_res.exit_code == 2 && contains(con_res.stdout_text, "nome reservado do Windows"), "CON");

        const auto flag_res = invoke_bootstrap(target_exe, {"--opcao-invalida"}, temp_root);
        require(flag_res.exit_code == 2 && contains(flag_res.stdout_text, "Opcao desconhecida"), "Flag desconhecida");
    });

    invoke_test(stats, "Colisao com pasta ou arquivo existente bloqueia sobrescrita (codigo 2)", [&]() {
        const fs::path existing_dir = temp_root / "ProjetoPreExistente";
        fs::create_directories(existing_dir);
        const fs::path sentinel = existing_dir / "sentinela.txt";
        bootstrap::write_utf8_no_bom(sentinel, "intacto");

        const auto dir_res = invoke_bootstrap(target_exe, {"ProjetoPreExistente"}, temp_root);
        require(dir_res.exit_code == 2 && contains(dir_res.stdout_text, "Ja existe uma pasta ou arquivo"), "Pasta");
        require(read_all_text(sentinel) == "intacto\n", "Sentinela modificado");

        const fs::path existing_file = temp_root / "ArquivoComMesmoNome";
        bootstrap::write_utf8_no_bom(existing_file, "arquivo");
        const auto file_res = invoke_bootstrap(target_exe, {"ArquivoComMesmoNome"}, temp_root);
        require(file_res.exit_code == 2, "Arquivo existente deveria retornar 2");
    });

    invoke_test(stats, "DirectoryRollbackGuard remove pasta nao comitada e preserva apos commit()", [&]() {
        const fs::path rollback_dir = temp_root / "RollbackTemp";
        {
            fs::create_directories(rollback_dir / "src");
            bootstrap::DirectoryRollbackGuard guard(rollback_dir);
        }
        require(!fs::exists(rollback_dir), "Diretorio deveria ter sido removido pelo rollback RAII");

        const fs::path committed_dir = temp_root / "CommittedTemp";
        {
            fs::create_directories(committed_dir);
            bootstrap::DirectoryRollbackGuard guard(committed_dir);
            guard.commit();
        }
        require(fs::exists(committed_dir), "Diretorio comitado deveria permanecer intacto");
    });

    // -------------------------------------------------------------------------
    // 3. Normalizacao, Estrutura e Encoding dos Artefatos Gerados
    // -------------------------------------------------------------------------
    print_section_header("3. Normalizacao, Estrutura e Encoding dos Artefatos");

    invoke_test(stats, "Espacos externos sofrem trim unico e criam a pasta exata", [&]() {
        const auto res = invoke_bootstrap(target_exe, {"  ProjetoTrimado  "}, temp_root);
        require(res.exit_code == 0, "Esperado exit code 0");

        const fs::path trimmed_dir = temp_root / "ProjetoTrimado";
        require(fs::is_directory(trimmed_dir), "Pasta 'ProjetoTrimado' nao criada");

        for (const auto& entry : fs::directory_iterator(temp_root)) {
            const std::string fname = entry.path().filename().string();
            require(fname.front() != ' ' && fname.back() != ' ', "Pasta criada com espaco nas bordas: " + fname);
        }

        const std::string cmake_text = read_all_text(trimmed_dir / "CMakeLists.txt");
        require(contains(cmake_text, "project(ProjetoTrimado LANGUAGES CXX)"), "CMakeLists nao recebeu nome trimado");
    });

    const std::string valid_project = "ProjetoTesteSucesso";
    const fs::path project_dir = temp_root / valid_project;

    invoke_test(stats, "Criacao de projeto valido retorna 0, instrucoes CMake e suporta --quiet", [&]() {
        const auto res = invoke_bootstrap(target_exe, {valid_project}, temp_root);
        require(res.exit_code == 0, "Esperado exit code 0");
        require(contains(res.stdout_text, "criado com sucesso"), "Mensagem de sucesso ausente");
        require(contains(res.stdout_text, "cmake -B build") && contains(res.stdout_text, "cmake --build build"), "Instrucoes CMake ausentes");
        require(contains(res.stdout_text, "Autor:"), "Rodape de autor deveria aparecer sem --quiet");

        const auto quiet_res = invoke_bootstrap(target_exe, {"ProjetoQuiet", "--quiet"}, temp_root);
        require(quiet_res.exit_code == 0, "--quiet deveria retornar 0");
        require(!contains(quiet_res.stdout_text, "Autor:"), "--quiet deveria suprimir rodape de autor");
    });

    invoke_test(stats, "Diretorios obrigatorios gerados (src, include, .vscode)", [&]() {
        require(fs::is_directory(project_dir), "Raiz do projeto ausente");
        require(fs::is_directory(project_dir / "src"), "Pasta src ausente");
        require(fs::is_directory(project_dir / "include"), "Pasta include ausente");
        require(fs::is_directory(project_dir / ".vscode"), "Pasta .vscode ausente");
    });

    invoke_test(stats, "Arquivo src/main.cpp gerado com assinatura valida e locale portavel", [&]() {
        const std::string main_cpp = read_all_text(project_dir / "src" / "main.cpp");
        require(contains(main_cpp, "int main()"), "Assinatura int main() ausente");
        require(contains(main_cpp, "#include <iostream>"), "<iostream> ausente");
        require(contains(main_cpp, "#include <clocale>"), "<clocale> ausente");
        require(contains(main_cpp, "std::setlocale(LC_ALL, \"\")"), "std::setlocale portavel ausente");
    });

    invoke_test(stats, "Configuracoes do VS Code possuem blocos multiplataforma validos", [&]() {
        const std::string launch_json = read_all_text(project_dir / ".vscode" / "launch.json");
        require(contains(launch_json, "\"windows\"") && contains(launch_json, "\"linux\"") && contains(launch_json, "\"osx\""),
                "launch.json sem blocos multiplataforma");

        const std::string settings_json = read_all_text(project_dir / ".vscode" / "settings.json");
        require(contains(settings_json, "\"cmake.exportCompileCommandsFile\": true"), "exportCompileCommandsFile ausente");
        require(contains(settings_json, "--query-driver=") && contains(settings_json, "/usr/bin/"),
                "--query-driver multiplataforma ausente");
    });

    invoke_test(stats, "CMakeLists.txt gerado com C++23 estrito, guarda de build e Secao 9 (stdc++exp)", [&]() {
        const std::string cmake = read_all_text(project_dir / "CMakeLists.txt");
        require(!contains(cmake, "__PROJECT_NAME__"), "Placeholder __PROJECT_NAME__ nao substituido");
        require(contains(cmake, "project(" + valid_project + " LANGUAGES CXX)"), "Declaracao project() ausente");
        require(contains(cmake, "set(CMAKE_CXX_STANDARD 23)"), "C++23 ausente");
        require(contains(cmake, "set(CMAKE_CXX_STANDARD_REQUIRED ON)"), "CXX_STANDARD_REQUIRED ausente");
        require(contains(cmake, "if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)"), "Guarda CMAKE_BUILD_TYPE ausente");
        require(contains(cmake, "-pedantic-errors") && contains(cmake, "/permissive-"), "Flags rigorosas ausentes");
        require(contains(cmake, "include(CheckLinkerFlag)") &&
                contains(cmake, "check_linker_flag(CXX \"-lstdc++exp\" HAVE_STDCXXEXP)") &&
                contains(cmake, "target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)"),
                "Secao 9 stdc++exp ausente");
    });

    invoke_test(stats, ".gitignore preserva VS Code e todos os 5 arquivos estao em UTF-8 sem BOM + LF", [&]() {
        const std::string gitignore = read_all_text(project_dir / ".gitignore");
        require(contains(gitignore, "build/") && contains(gitignore, "*.exe"), "Padroes de build ausentes");
        require(contains(gitignore, ".vscode/*") &&
                contains(gitignore, "!.vscode/launch.json") &&
                contains(gitignore, "!.vscode/settings.json"),
                "Regras de preservacao .vscode ausentes");

        const std::vector<fs::path> generated = {
            project_dir / "CMakeLists.txt",
            project_dir / ".gitignore",
            project_dir / "src" / "main.cpp",
            project_dir / ".vscode" / "launch.json",
            project_dir / ".vscode" / "settings.json"
        };
        for (const auto& file : generated) {
            require(fs::is_regular_file(file), "Arquivo gerado ausente: " + file.string());
            require(!has_utf8_bom(file), "BOM UTF-8 detectado em: " + file.string());
            const std::string text = read_all_text(file);
            require(!contains(text, "\r\n"), "CRLF detectado em: " + file.string());
        }
    });

    // -------------------------------------------------------------------------
    // 4. Integracao Real: Compilacao CMake e C++23 (<print> / <stacktrace>)
    // -------------------------------------------------------------------------
    print_section_header("4. Integracao Real com CMake e C++23");

    const bool has_cmake = command_exists_in_path("cmake");
    const bool has_compiler = command_exists_in_path("g++") ||
                              command_exists_in_path("clang++") ||
                              command_exists_in_path("cl");

    if (include_build) {
        if (!has_cmake || !has_compiler) {
            invoke_skip(stats, "Compilacao real e linkagem C++23 (<print>/<stacktrace>)", "CMake ou compilador C++ ausente no PATH");
        }
        else {
            invoke_test(stats, "Projeto gerado configura, respeita Release e executa C++23 com stdc++exp", [&]() {
                const fs::path build_dir = project_dir / "build";
                bool configured = false;

#if defined(_WIN32)
                const bool has_make = command_exists_in_path("mingw32-make") || command_exists_in_path("make");
                if (has_make && command_exists_in_path("g++")) {
                    const auto mingw_cfg = run_process(
                        {"cmake", "-B", "build", "-G", "MinGW Makefiles", "-DCMAKE_BUILD_TYPE=Release"},
                        project_dir
                    );
                    if (mingw_cfg.exit_code == 0) {
                        configured = true;
                    }
                    else if (fs::exists(build_dir)) {
                        std::error_code ec;
                        fs::remove_all(build_dir, ec);
                    }
                }
#endif

                if (!configured) {
                    const auto default_cfg = run_process(
                        {"cmake", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"},
                        project_dir
                    );
                    require(default_cfg.exit_code == 0, "cmake -B build falhou: " + default_cfg.stderr_text);
                }

                const fs::path cache_file = build_dir / "CMakeCache.txt";
                if (fs::exists(cache_file)) {
                    const std::string cache_text = read_all_text(cache_file);
                    if (!contains(cache_text, "CMAKE_CONFIGURATION_TYPES")) {
                        require(contains(cache_text, "CMAKE_BUILD_TYPE:STRING=Release"),
                                "CMakeLists.txt sobrescreveu CMAKE_BUILD_TYPE=Release");
                    }
                }

                const auto build_res = run_process(
                    {"cmake", "--build", "build", "--config", "Release"},
                    project_dir
                );
                require(build_res.exit_code == 0, "cmake --build build falhou: " + build_res.stderr_text);

                // Substitui src/main.cpp para exercitar <print> e <stacktrace> via stdc++exp
                constexpr std::string_view kCpp23Check = R"cpp(#include <iostream>
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
)cpp";
                bootstrap::write_utf8_no_bom(project_dir / "src" / "main.cpp", kCpp23Check);

                const auto rebuild_res = run_process(
                    {"cmake", "--build", "build", "--clean-first", "--config", "Release"},
                    project_dir
                );
                require(rebuild_res.exit_code == 0, "Falha ao compilar/linkar C++23: " + rebuild_res.stderr_text);

#if defined(_WIN32)
                const std::string bin_name = valid_project + ".exe";
#else
                const std::string bin_name = valid_project;
#endif
                const std::vector<fs::path> candidates = {
                    build_dir / bin_name,
                    build_dir / "Release" / bin_name,
                    build_dir / "Debug" / bin_name
                };

                fs::path found_bin;
                for (const auto& c : candidates) {
                    if (fs::is_regular_file(c)) {
                        found_bin = c;
                        break;
                    }
                }
                require(!found_bin.empty(), "Binario C++23 compilado nao encontrado");

                const auto run_bin = run_process({found_bin.string()}, project_dir);
                require(run_bin.exit_code == 0, "Execucao do binario gerado falhou");
                require(contains(run_bin.stdout_text, "C++23"), "Saida esperada C++23 ausente: " + run_bin.stdout_text);
            });
        }
    }
    else {
        invoke_skip(stats, "Compilacao real e linkagem C++23 (<print>/<stacktrace>)", "Passe --include-build para executar");
    }

    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start_tp).count();
    const auto& p = bootstrap::term::palette();

    bootstrap::term::print_line("");
    print_divider();
    bootstrap::term::print_line(std::string(p.bold) + "[RESUMO DA EXECUCAO DOS TESTES]" + std::string(p.reset));
    print_divider();
    bootstrap::term::print_line("  Total de testes : " + std::to_string(stats.total));
    bootstrap::term::print_line("  " + std::string(p.green) + "Aprovados       : " + std::to_string(stats.passed) + std::string(p.reset));
    bootstrap::term::print_line(
        "  " + std::string(stats.failed > 0 ? p.red : p.gray) + "Falhos          : " + std::to_string(stats.failed) + std::string(p.reset)
    );
    if (stats.skipped > 0) {
        bootstrap::term::print_line(
            "  " + std::string(p.yellow) + "Ignorados       : " + std::to_string(stats.skipped) + std::string(p.reset)
        );
    }

    std::ostringstream time_oss;
    time_oss << std::fixed << std::setprecision(2) << elapsed << "s";
    bootstrap::term::print_line("  Tempo total     : " + time_oss.str());
    print_divider();
    bootstrap::term::print_line("");

    if (stats.failed > 0) {
        bootstrap::term::write_error("[FAIL] A suite de testes falhou!");
        return 1;
    }

    bootstrap::term::print_line(
        std::string(p.green) + "[PASS] Todos os testes passaram com sucesso!" + std::string(p.reset)
    );
    return 0;
}
