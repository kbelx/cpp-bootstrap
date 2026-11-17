/*
AUTOR: Bruno Costa
DATA: 16/11/2026
PROPOSITO: Declaracao do rollback RAII (DirectoryRollbackGuard), escrita UTF-8 sem BOM e geracao do projeto.
*/

#ifndef GENERATOR_H
#define GENERATOR_H

    #include <filesystem>
    #include <string>
    #include <string_view>

    namespace bootstrap {
        
        // Guarda RAII que remove recursivamente a pasta raiz criada caso ocorra falha antes de commit().
        class DirectoryRollbackGuard {
            public:
            explicit DirectoryRollbackGuard(std::filesystem::path root_path) noexcept;
            ~DirectoryRollbackGuard() noexcept;
            
            DirectoryRollbackGuard(const DirectoryRollbackGuard&) = delete;
            DirectoryRollbackGuard& operator=(const DirectoryRollbackGuard&) = delete;
            DirectoryRollbackGuard(DirectoryRollbackGuard&&) = delete;
            DirectoryRollbackGuard& operator=(DirectoryRollbackGuard&&) = delete;
            
            void commit() noexcept;
            [[nodiscard]] bool is_committed() const noexcept;
            
            private:
            std::filesystem::path root_path_;
            bool committed_ = false;
        };
        
        // Grava arquivo estritamente em UTF-8 sem BOM com quebras de linha LF e newline final.
        void write_utf8_no_bom(const std::filesystem::path& file_path, std::string_view content);
        
        // Gera toda a estrutura do projeto sem alterar o diretorio de trabalho (com rollback automatico).
        [[nodiscard]] bool create_project(
            const std::filesystem::path& base_dir,
            std::string_view project_name,
            std::string* out_error = nullptr
        );
        
    }  // namespace bootstrap

#endif
