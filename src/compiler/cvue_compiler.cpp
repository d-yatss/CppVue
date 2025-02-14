#include "cvue_compiler.hpp"
#include <fstream>
#include <sstream>
#include <regex>

namespace cppvue::compiler {

CvueFileContent CvueCompiler::parseCvueFile(const std::string& content) {
    CvueFileContent result;
    std::istringstream stream(content);
    std::string line;
    
    enum class Section {
        None,
        Template,
        Cpp,
        Style
    } currentSection = Section::None;
    
    std::stringstream currentContent;
    
    while (std::getline(stream, line)) {
        if (isTemplateStart(line)) {
            currentSection = Section::Template;
            continue;
        } else if (isCppStart(line)) {
            currentSection = Section::Cpp;
            continue;
        } else if (isStyleStart(line)) {
            currentSection = Section::Style;
            result.style_scoped = (line.find("scoped") != std::string::npos);
            continue;
        } else if (isSectionEnd(line)) {
            switch (currentSection) {
                case Section::Template:
                    result.template_content = currentContent.str();
                    break;
                case Section::Cpp:
                    result.cpp_content = currentContent.str();
                    break;
                case Section::Style:
                    result.style_content = currentContent.str();
                    break;
                default:
                    break;
            }
            currentContent.str("");
            currentContent.clear();
            currentSection = Section::None;
            continue;
        }
        
        if (currentSection != Section::None) {
            currentContent << line << "\n";
        }
    }
    
    return result;
}

std::string CvueCompiler::generateCppCode(const CvueFileContent& content, const std::string& className) {
    std::stringstream result;
    
    // Génère le code pour le template
    std::string parsedTemplate = parseTemplate(content.template_content);
    std::string templateCode = generateTemplateCode(parsedTemplate);
    
    // Combine le code C++ et le template
    result << "#include <cppvue/component.hpp>\n";
    result << "#include <cppvue/template.hpp>\n\n";
    result << content.cpp_content << "\n\n";
    
    // Ajoute la méthode de rendu générée
    result << "std::shared_ptr<VNode> " << className << "::render() override {\n";
    result << "    return " << templateCode << ";\n";
    result << "}\n";
    
    return result.str();
}

std::string CvueCompiler::generateCssCode(const CvueFileContent& content, const std::string& componentId) {
    if (!content.style_scoped) {
        return content.style_content;
    }
    
    // Ajoute l'attribut data-v-{componentId} à chaque sélecteur
    std::regex selectorRegex("([^{]+)({[^}]*})");
    std::string result = content.style_content;
    std::string replacement = "$1[data-v-" + componentId + "]$2";
    
    return std::regex_replace(result, selectorRegex, replacement);
}

bool CvueCompiler::isTemplateStart(const std::string& line) {
    return line.find("@template") != std::string::npos;
}

bool CvueCompiler::isCppStart(const std::string& line) {
    return line.find("@cpp") != std::string::npos;
}

bool CvueCompiler::isStyleStart(const std::string& line) {
    return line.find("@style") != std::string::npos;
}

bool CvueCompiler::isSectionEnd(const std::string& line) {
    return line.find("@end") != std::string::npos;
}

std::string CvueCompiler::parseTemplate(const std::string& template_content) {
    // Implémentation du parsing du template
    // Convertit le HTML en appels de la méthode h()
    // TODO: Implémenter un parser HTML complet
    return template_content;
}

std::string CvueCompiler::generateTemplateCode(const std::string& parsed_template) {
    // Génère le code C++ pour le template
    // TODO: Implémenter la génération de code complète
    return "h(\"div\", \"Template not implemented yet\")";
}

bool CvueFileCompiler::compileFile(const std::filesystem::path& cvue_file, 
                                 const std::filesystem::path& output_dir) {
    try {
        // Lit le fichier .cvue
        std::string content = readFile(cvue_file);
        
        // Parse le contenu
        auto cvueContent = CvueCompiler::parseCvueFile(content);
        
        // Génère le nom de la classe
        std::string className = generateClassName(cvue_file);
        
        // Génère le code C++
        std::string cppCode = CvueCompiler::generateCppCode(cvueContent, className);
        
        // Génère le code CSS
        std::string componentId = className + "_" + std::to_string(std::hash<std::string>{}(className));
        std::string cssCode = CvueCompiler::generateCssCode(cvueContent, componentId);
        
        // Écrit les fichiers générés
        auto cppPath = output_dir / (cvue_file.stem().string() + ".cpp");
        auto cssPath = output_dir / (cvue_file.stem().string() + ".css");
        
        writeFile(cppPath, cppCode);
        writeFile(cssPath, cssCode);
        
        return true;
    } catch (const std::exception& e) {
        // Log error
        return false;
    }
}

std::string CvueFileCompiler::generateClassName(const std::filesystem::path& cvue_file) {
    return cvue_file.stem().string();
}

std::string CvueFileCompiler::readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path.string());
    }
    return std::string(std::istreambuf_iterator<char>(file),
                      std::istreambuf_iterator<char>());
}

void CvueFileCompiler::writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write file: " + path.string());
    }
    file << content;
}

} // namespace cppvue::compiler
