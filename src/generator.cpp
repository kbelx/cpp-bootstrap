#include "generator.hpp"

#include "templates.hpp"

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace bootstrap {

DirectoryRollbackGuard::DirectoryRollbackGuard(std::filesystem::path root_path) noexcept
    : root_path_(std::move(root_path)) {}

DirectoryRollbackGuard::~DirectoryRollbackGuard() noexcept {
    if (!committed_ && !root_path_.empty()) {
        std::error_code ec;
        std::filesystem::remove_all(root_path_, ec);
    }
}

void DirectoryRollbackGuard::commit() noexcept {
    committed_ = true;
}

bool DirectoryRollbackGuard::is_committed() const noexcept {
    return committed_;
}

void write_utf8_no_bom(const std::filesystem::path& file_path, std::string_view content) {
    // Padroniza quebras de linha em LF e garante newline final (regra POSIX/C++)
    std::string normalized;
    normalized.reserve(content.size() + 1);

    for (std::size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\r' && (i + 1 < content.size()) && content[i + 1] == '\n') {
            normalized.push_back('\n');
            ++i;
        }
        else {
            normalized.push_back(content[i]);
        }
    }

    if (normalized.empty() || normalized.back() != '\n') {
        normalized.push_back('\n');
    }

    // Modo binario impede que a CRT do Windows converta '\n' em "\r\n"
    std::ofstream ofs(file_path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo para escrita: " + file_path.string());
    }

    ofs.write(normalized.data(), static_cast<std::streamsize>(normalized.size()));
    ofs.flush();
    if (!ofs.good()) {
        throw std::runtime_error("Falha ao gravar dados no arquivo: " + file_path.string());
    }
}

bool create_project(
    const std::filesystem::path& base_dir,
    std::string_view project_name,
    std::string* out_error
) {
    try {
        const std::filesystem::path project_root = base_dir / std::filesystem::path(project_name);

        // Cria a pasta raiz primeiro e arma o rollback RAII imediatamente
        if (!std::filesystem::create_directory(project_root)) {
            if (out_error != nullptr) {
                *out_error = "Nao foi possivel criar o diretorio raiz: " + project_root.string();
            }
            return false;
        }
        DirectoryRollbackGuard rollback_guard(project_root);

        const std::filesystem::path src_dir     = project_root / "src";
        const std::filesystem::path include_dir = project_root / "include";
        const std::filesystem::path vscode_dir  = project_root / ".vscode";

        std::filesystem::create_directory(src_dir);
        std::filesystem::create_directory(include_dir);
        std::filesystem::create_directory(vscode_dir);

        // Gera os 5 arquivos padrao em UTF-8 sem BOM e LF
        write_utf8_no_bom(vscode_dir / "launch.json", tpl::kLaunchJson);
        write_utf8_no_bom(vscode_dir / "settings.json", tpl::kSettingsJson);
        write_utf8_no_bom(src_dir / "main.cpp", tpl::kMainCpp);
        write_utf8_no_bom(project_root / "CMakeLists.txt", tpl::render_cmake_lists(project_name));
        write_utf8_no_bom(project_root / ".gitignore", tpl::kGitignore);

        // Tudo gravado com sucesso: desarma o rollback
        rollback_guard.commit();
        return true;
    }
    catch (const std::exception& ex) {
        if (out_error != nullptr) {
            *out_error = ex.what();
        }
        return false;
    }
}

}  // namespace bootstrap
