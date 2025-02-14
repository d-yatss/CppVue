#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace cppvue::compiler {

struct CvueFileContent {
    std::string template_content;
    std::string cpp_content;
    std::string style_content;
    bool style_scoped{false};
};

class CvueCompiler {
public:
    // Parse un fichier .cvue
    static CvueFileContent parseCvueFile(const std::string& content);
    
    // Génère le code C++ final
    static std::string generateCppCode(const CvueFileContent& content, const std::string& className);
    
    // Génère le code CSS avec support du scoping
    static std::string generateCssCode(const CvueFileContent& content, const std::string& componentId);

private:
    static std::string parseTemplate(const std::string& template_content);
    static std::string generateTemplateCode(const std::string& parsed_template);
    
    // Helpers pour parser les sections
    static bool isTemplateStart(const std::string& line);
    static bool isCppStart(const std::string& line);
    static bool isStyleStart(const std::string& line);
    static bool isSectionEnd(const std::string& line);
};

// Utilitaire pour compiler les fichiers .cvue
class CvueFileCompiler {
public:
    static bool compileFile(const std::filesystem::path& cvue_file, 
                          const std::filesystem::path& output_dir);
    
private:
    static std::string generateClassName(const std::filesystem::path& cvue_file);
    static std::string readFile(const std::filesystem::path& path);
    static void writeFile(const std::filesystem::path& path, const std::string& content);
};

} // namespace cppvue::compiler
