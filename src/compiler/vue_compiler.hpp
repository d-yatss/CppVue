#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace cppvue::compiler {

struct VueFileContent {
    std::string template_content;
    std::string script_content;
    std::string style_content;
    bool style_scoped{false};
};

class VueCompiler {
public:
    static VueFileContent parseVueFile(const std::string& content);
    static std::string generateCppCode(const VueFileContent& content, const std::string& className);
    
private:
    static std::string parseTemplate(const std::string& template_content);
    static std::string generateTemplateCode(const std::string& parsed_template);
};

// Classe utilitaire pour la compilation des fichiers .vue
class VueFileCompiler {
public:
    static bool compileFile(const std::filesystem::path& vue_file, 
                          const std::filesystem::path& output_dir);
    
private:
    static std::string generateClassName(const std::filesystem::path& vue_file);
    static std::string readFile(const std::filesystem::path& path);
    static void writeFile(const std::filesystem::path& path, const std::string& content);
};

} // namespace cppvue::compiler
