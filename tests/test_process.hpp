/*
AUTOR: Bruno Costa
DATA: 17/11/2026
PROPOSITO: Execucao multiplataforma de subprocessos e verificacao de comandos no PATH para testes.
*/

#ifndef TEST_PROCESS_HPP
#define TEST_PROCESS_HPP

//============================ [ CABECALHOS PADRAO ] ===========================

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

//======================== [ CABECALHOS DE PLATAFORMA ] ========================

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <unistd.h>
#endif

//=========================== [ CABECALHOS DO PROJETO ] ========================

#include "terminal.hpp"
#include "test_harness.hpp"

//========================= [ EXECUCAO DE PROCESSOS ] ==========================

namespace Testes::Processo {

    struct ResultadoProcesso {
        int CodigoSaida{-1};
        std::string TextoStdout{};
        std::string TextoStderr{};
    };

#if defined(_WIN32)

    [[nodiscard]] inline std::string citarArgWin32(std::string_view Arg) {
        if (!Arg.empty() && Arg.find_first_of(" \t\n\v\"") == std::string_view::npos) {
            return std::string{Arg};
        }

        std::string Citado{};
        Citado.push_back('"');
        for (std::size_t Indice{0}; Indice < Arg.size(); ++Indice) {
            std::size_t Barras{0};
            while (Indice < Arg.size() && Arg[Indice] == '\\') {
                ++Barras;
                ++Indice;
            }
            if (Indice == Arg.size()) {
                Citado.append(Barras * 2, '\\');
                break;
            }
            if (Arg[Indice] == '"') {
                Citado.append(Barras * 2 + 1, '\\');
                Citado.push_back('"');
            }
            else {
                Citado.append(Barras, '\\');
                Citado.push_back(Arg[Indice]);
            }
        }
        Citado.push_back('"');
        return Citado;
    }

    [[nodiscard]] inline ResultadoProcesso executarProcesso(
        const std::vector<std::string>& Args,
        const std::filesystem::path& DirTrabalho
    ) {
        Infra::exigir(!Args.empty(), "Lista de argumentos vazia para executarProcesso");

        std::string LinhaComando{};
        for (std::size_t Indice{0}; Indice < Args.size(); ++Indice) {
            if (Indice > 0) {
                LinhaComando.push_back(' ');
            }
            LinhaComando.append(citarArgWin32(Args[Indice]));
        }

        SECURITY_ATTRIBUTES AtribSeg{};
        AtribSeg.nLength              = sizeof(SECURITY_ATTRIBUTES);
        AtribSeg.bInheritHandle       = TRUE;
        AtribSeg.lpSecurityDescriptor = nullptr;

        HANDLE StdoutLeitura{nullptr};
        HANDLE StdoutEscrita{nullptr};
        HANDLE StderrLeitura{nullptr};
        HANDLE StderrEscrita{nullptr};

        Infra::exigir(
            CreatePipe(&StdoutLeitura, &StdoutEscrita, &AtribSeg, 0) != 0,
            "Falha em CreatePipe stdout"
        );
        Infra::exigir(
            SetHandleInformation(StdoutLeitura, HANDLE_FLAG_INHERIT, 0) != 0,
            "Falha em SetHandleInformation stdout"
        );
        Infra::exigir(
            CreatePipe(&StderrLeitura, &StderrEscrita, &AtribSeg, 0) != 0,
            "Falha em CreatePipe stderr"
        );
        Infra::exigir(
            SetHandleInformation(StderrLeitura, HANDLE_FLAG_INHERIT, 0) != 0,
            "Falha em SetHandleInformation stderr"
        );

        HANDLE NulEntrada{CreateFileA(
            "NUL",
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            &AtribSeg,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        )};

        STARTUPINFOA InfoInicial{};
        InfoInicial.cb         = sizeof(STARTUPINFOA);
        InfoInicial.dwFlags    = STARTF_USESTDHANDLES;
        InfoInicial.hStdInput  = (NulEntrada != INVALID_HANDLE_VALUE) ? NulEntrada : GetStdHandle(STD_INPUT_HANDLE);
        InfoInicial.hStdOutput = StdoutEscrita;
        InfoInicial.hStdError  = StderrEscrita;

        PROCESS_INFORMATION InfoProc{};
        const std::string DIR_STR{DirTrabalho.string()};
        std::string CmdMutavel{LinhaComando};

        const BOOL INICIOU{CreateProcessA(
            nullptr,
            CmdMutavel.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            DIR_STR.c_str(),
            &InfoInicial,
            &InfoProc
        )};

        CloseHandle(StdoutEscrita);
        CloseHandle(StderrEscrita);
        if (NulEntrada != INVALID_HANDLE_VALUE) {
            CloseHandle(NulEntrada);
        }

        if (INICIOU == 0) {
            CloseHandle(StdoutLeitura);
            CloseHandle(StderrLeitura);
            throw std::runtime_error("CreateProcessA falhou para: " + LinhaComando);
        }

        ResultadoProcesso Resultado{};
        auto drenarPipe = [](HANDLE Manip, std::string& Destino) {
            char Buffer[4096]{};
            DWORD BytesLidos{0};
            while (ReadFile(Manip, Buffer, static_cast<DWORD>(sizeof(Buffer)), &BytesLidos, nullptr) != 0 &&
                   BytesLidos > 0) {
                Destino.append(Buffer, static_cast<std::size_t>(BytesLidos));
            }
        };

        std::thread ThreadOut{drenarPipe, StdoutLeitura, std::ref(Resultado.TextoStdout)};
        std::thread ThreadErr{drenarPipe, StderrLeitura, std::ref(Resultado.TextoStderr)};

        WaitForSingleObject(InfoProc.hProcess, INFINITE);
        ThreadOut.join();
        ThreadErr.join();

        DWORD CodigoSaidaWin{0};
        GetExitCodeProcess(InfoProc.hProcess, &CodigoSaidaWin);
        Resultado.CodigoSaida = static_cast<int>(CodigoSaidaWin);

        CloseHandle(StdoutLeitura);
        CloseHandle(StderrLeitura);
        CloseHandle(InfoProc.hProcess);
        CloseHandle(InfoProc.hThread);

        if (Infra::ModoVerboso) {
            if (!Resultado.TextoStdout.empty()) {
                Terminal::Saida::imprimirLinha("[STDOUT] " + Resultado.TextoStdout);
            }
            if (!Resultado.TextoStderr.empty()) {
                Terminal::Saida::imprimirLinha("[STDERR] " + Resultado.TextoStderr);
            }
        }
        return Resultado;
    }

    [[nodiscard]] inline bool comandoExisteNoPath(std::string_view Comando) {
        return SearchPathA(nullptr, std::string{Comando}.c_str(), ".exe", 0, nullptr, nullptr) != 0;
    }

#else

    [[nodiscard]] inline ResultadoProcesso executarProcesso(
        const std::vector<std::string>& Args,
        const std::filesystem::path& DirTrabalho
    ) {
        Infra::exigir(!Args.empty(), "Lista de argumentos vazia para executarProcesso");

        int PipeStdout[2]{-1, -1};
        int PipeStderr[2]{-1, -1};
        Infra::exigir(pipe(PipeStdout) == 0, "Falha ao criar pipe stdout");
        Infra::exigir(pipe(PipeStderr) == 0, "Falha ao criar pipe stderr");

        const pid_t PID{fork()};
        Infra::exigir(PID >= 0, "Falha ao executar fork()");

        if (PID == 0) {
            if (chdir(DirTrabalho.c_str()) != 0) {
                _exit(126);
            }

            const int DEV_NULL{open("/dev/null", O_RDONLY)};
            if (DEV_NULL >= 0) {
                dup2(DEV_NULL, STDIN_FILENO);
                close(DEV_NULL);
            }

            dup2(PipeStdout[1], STDOUT_FILENO);
            dup2(PipeStderr[1], STDERR_FILENO);

            close(PipeStdout[0]);
            close(PipeStdout[1]);
            close(PipeStderr[0]);
            close(PipeStderr[1]);

            std::vector<char*> ArgsC{};
            ArgsC.reserve(Args.size() + 1);
            for (const std::string& ARG : Args) {
                ArgsC.push_back(const_cast<char*>(ARG.c_str()));
            }
            ArgsC.push_back(nullptr);

            execvp(ArgsC[0], ArgsC.data());
            _exit(127);
        }

        close(PipeStdout[1]);
        close(PipeStderr[1]);

        ResultadoProcesso Resultado{};
        auto drenarFd = [](int Descritor, std::string& Destino) {
            char Buffer[4096]{};
            for (;;) {
                const ssize_t LIDOS{read(Descritor, Buffer, sizeof(Buffer))};
                if (LIDOS <= 0) {
                    break;
                }
                Destino.append(Buffer, static_cast<std::size_t>(LIDOS));
            }
        };

        std::thread ThreadOut{drenarFd, PipeStdout[0], std::ref(Resultado.TextoStdout)};
        std::thread ThreadErr{drenarFd, PipeStderr[0], std::ref(Resultado.TextoStderr)};

        int Estado{0};
        waitpid(PID, &Estado, 0);
        ThreadOut.join();
        ThreadErr.join();

        close(PipeStdout[0]);
        close(PipeStderr[0]);

        if (WIFEXITED(Estado)) {
            Resultado.CodigoSaida = WEXITSTATUS(Estado);
        }
        else {
            Resultado.CodigoSaida = -1;
        }

        if (Infra::ModoVerboso) {
            if (!Resultado.TextoStdout.empty()) {
                Terminal::Saida::imprimirLinha("[STDOUT] " + Resultado.TextoStdout);
            }
            if (!Resultado.TextoStderr.empty()) {
                Terminal::Saida::imprimirLinha("[STDERR] " + Resultado.TextoStderr);
            }
        }
        return Resultado;
    }

    [[nodiscard]] inline bool comandoExisteNoPath(std::string_view Comando) {
        const char* ENV_PATH{std::getenv("PATH")};
        if (ENV_PATH == nullptr) {
            return false;
        }
        std::stringstream Fluxo{ENV_PATH};
        std::string Diretorio{};
        while (std::getline(Fluxo, Diretorio, ':')) {
            if (Diretorio.empty()) {
                continue;
            }
            const std::filesystem::path CANDIDATO{std::filesystem::path{Diretorio} / std::string{Comando}};
            std::error_code CodigoErro{};
            if (std::filesystem::exists(CANDIDATO, CodigoErro) && access(CANDIDATO.c_str(), X_OK) == 0) {
                return true;
            }
        }
        return false;
    }

#endif

    [[nodiscard]] inline ResultadoProcesso invocarBootstrap(
        const std::filesystem::path& CaminhoExe,
        const std::vector<std::string>& ArgsExtras,
        const std::filesystem::path& DirTrabalho
    ) {
        std::vector<std::string> ArgsCompletos{};
        ArgsCompletos.reserve(1 + ArgsExtras.size());
        ArgsCompletos.push_back(CaminhoExe.string());
        ArgsCompletos.insert(ArgsCompletos.end(), ArgsExtras.begin(), ArgsExtras.end());
        return executarProcesso(ArgsCompletos, DirTrabalho);
    }

}  // namespace Testes::Processo

#endif
