#include "build_tool.hpp"
#include "cvue_compiler.hpp"
#include "template_parser.hpp"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <regex>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace cppvue::compiler {

BuildTool::BuildTool(BuildConfig config)
    : config_(std::move(config)) {
    ensureDirectories();
}

bool BuildTool::generateBuildFiles() {
    try {
        generateCMakeLists();
        generateComponentRegistry();
        return true;
    } catch (const std::exception& e) {
        throw BuildError("Failed to generate build files: " + std::string(e.what()));
    }
}

void BuildTool::generateCMakeLists() {
    std::filesystem::path cmakeFile = config_.buildDir / "CMakeLists.txt";
    std::ofstream file(cmakeFile);
    
    if (!file) {
        throw BuildError("Failed to create CMakeLists.txt");
    }
    
    file << "cmake_minimum_required(VERSION 3.15)\n\n";
    file << "project(" << config_.projectName << ")\n\n";
    
    // Configuration C++
    file << "set(CMAKE_CXX_STANDARD 20)\n";
    file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";
    
    // Inclusions
    file << "include_directories(${CMAKE_SOURCE_DIR}/include)\n";
    for (const auto& include : config_.includes) {
        file << "include_directories(" << include << ")\n";
    }
    file << "\n";
    
    // Sources
    file << "set(COMPONENT_SOURCES\n";
    for (const auto& cvueFile : findCvueFiles()) {
        auto relativePath = std::filesystem::relative(cvueFile, config_.sourceDir);
        auto cppFile = config_.buildDir / relativePath.replace_extension(".cpp");
        file << "    " << cppFile.string() << "\n";
    }
    file << ")\n\n";
    
    // Cible de la bibliothèque
    file << "add_library(" << config_.projectName << " STATIC\n";
    file << "    ${COMPONENT_SOURCES}\n";
    file << "    ${CMAKE_SOURCE_DIR}/src/component_registry.cpp\n";
    file << ")\n\n";
    
    // Configuration de la cible
    file << "target_compile_definitions(" << config_.projectName << " PRIVATE\n";
    if (config_.enableHotReload) {
        file << "    ENABLE_HOT_RELOAD\n";
    }
    file << ")\n";
}

void BuildTool::generateComponentRegistry() {
    // Génère le header
    {
        std::ofstream file(config_.buildDir / "include/component_registry.hpp");
        file << "#pragma once\n\n";
        file << "#include <cppvue/component.hpp>\n";
        file << "#include <string>\n";
        file << "#include <memory>\n\n";
        file << "namespace " << config_.projectName << " {\n\n";
        file << "class ComponentRegistry {\n";
        file << "public:\n";
        file << "    static ComponentRegistry& instance();\n";
        file << "    std::shared_ptr<cppvue::Component> createComponent(const std::string& name);\n";
        file << "    void registerComponent(const std::string& name, std::function<std::shared_ptr<cppvue::Component>()> factory);\n";
        file << "private:\n";
        file << "    ComponentRegistry() = default;\n";
        file << "    std::unordered_map<std::string, std::function<std::shared_ptr<cppvue::Component>()>> factories_;\n";
        file << "};\n\n";
        file << "} // namespace " << config_.projectName << "\n";
    }
    
    // Génère l'implémentation
    {
        std::ofstream file(config_.buildDir / "src/component_registry.cpp");
        file << "#include \"component_registry.hpp\"\n\n";
        file << "namespace " << config_.projectName << " {\n\n";
        file << "ComponentRegistry& ComponentRegistry::instance() {\n";
        file << "    static ComponentRegistry registry;\n";
        file << "    return registry;\n";
        file << "}\n\n";
        file << "std::shared_ptr<cppvue::Component> ComponentRegistry::createComponent(const std::string& name) {\n";
        file << "    auto it = factories_.find(name);\n";
        file << "    if (it != factories_.end()) {\n";
        file << "        return it->second();\n";
        file << "    }\n";
        file << "    return nullptr;\n";
        file << "}\n\n";
        file << "void ComponentRegistry::registerComponent(const std::string& name, std::function<std::shared_ptr<cppvue::Component>()> factory) {\n";
        file << "    factories_[name] = std::move(factory);\n";
        file << "}\n\n";
        file << "} // namespace " << config_.projectName << "\n";
    }
}

bool BuildTool::buildComponents() {
    bool success = true;
    auto components = findCvueFiles();
    
    // Analyse les dépendances
    analyzeDependencies();
    
    // Compile chaque composant
    for (const auto& cvueFile : components) {
        if (needsRecompilation(cvueFile)) {
            if (!compileSingleComponent(cvueFile)) {
                success = false;
            }
        }
    }
    
    if (success) {
        generateComponentLoader(components);
    }
    
    return success;
}

bool BuildTool::compileSingleComponent(const std::filesystem::path& cvueFile) {
    try {
        // Lit le fichier .cvue
        std::ifstream input(cvueFile);
        if (!input) {
            throw BuildError("Failed to open file: " + cvueFile.string());
        }
        
        std::string content((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
        
        // Parse le fichier
        auto cvueContent = CvueCompiler::parseCvueFile(content);
        
        // Génère le code C++
        std::string className = cvueFile.stem().string();
        std::string cppCode = CvueCompiler::generateCppCode(cvueContent, className);
        
        // Génère le CSS
        std::string componentId = generateComponentId(cvueFile);
        std::string cssCode = CvueCompiler::generateCssCode(cvueContent, componentId);
        
        // Écrit les fichiers générés
        auto outputPath = config_.buildDir / cvueFile.filename().replace_extension(".cpp");
        auto cssPath = config_.buildDir / cvueFile.filename().replace_extension(".css");
        
        {
            std::ofstream cpp(outputPath);
            if (!cpp) {
                throw BuildError("Failed to create output file: " + outputPath.string());
            }
            cpp << cppCode;
        }
        
        {
            std::ofstream css(cssPath);
            if (!css) {
                throw BuildError("Failed to create CSS file: " + cssPath.string());
            }
            css << cssCode;
        }
        
        // Met à jour les informations de dépendance
        updateDependencyInfo(cvueFile);
        
        // Notifie le hot reload si activé
        if (config_.enableHotReload) {
            notifyHotReload(cvueFile);
        }
        
        return true;
    } catch (const std::exception& e) {
        throw BuildError("Failed to compile " + cvueFile.string() + ": " + e.what());
    }
}

void BuildTool::watchAndRebuild() {
    if (!config_.enableHotReload) {
        return;
    }
    
    std::thread([this]() {
        std::queue<std::filesystem::path> changedFiles;
        std::mutex mutex;
        std::condition_variable cv;
        bool running = true;
        
        // Thread de surveillance
        std::thread watchThread([&]() {
            while (running) {
                for (const auto& cvueFile : findCvueFiles()) {
                    if (needsRecompilation(cvueFile)) {
                        std::lock_guard<std::mutex> lock(mutex);
                        changedFiles.push(cvueFile);
                        cv.notify_one();
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        });
        
        // Thread de compilation
        while (running) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&]() { return !changedFiles.empty(); });
            
            while (!changedFiles.empty()) {
                auto file = changedFiles.front();
                changedFiles.pop();
                lock.unlock();
                
                try {
                    compileSingleComponent(file);
                } catch (const std::exception& e) {
                    // Log error
                }
                
                lock.lock();
            }
        }
        
        watchThread.join();
    }).detach();
}

std::string BuildTool::generateComponentId(const std::filesystem::path& cvueFile) {
    return std::to_string(std::hash<std::string>{}(cvueFile.string()));
}

std::vector<std::filesystem::path> BuildTool::findCvueFiles() {
    std::vector<std::filesystem::path> result;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(config_.sourceDir)) {
        if (entry.path().extension() == ".cvue") {
            result.push_back(entry.path());
        }
    }
    return result;
}

void BuildTool::ensureDirectories() {
    std::filesystem::create_directories(config_.buildDir);
    std::filesystem::create_directories(config_.buildDir / "include");
    std::filesystem::create_directories(config_.buildDir / "src");
    std::filesystem::create_directories(config_.outputDir);
}

void BuildTool::analyzeDependencies() {
    for (const auto& cvueFile : findCvueFiles()) {
        std::ifstream file(cvueFile);
        std::string content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
        
        // Parse les imports et les composants utilisés
        std::regex importRegex("#include\\s+[\"<]([^>\"]+)[\">]");
        std::regex componentRegex("<([A-Z][\\w-]*)");
        
        std::smatch matches;
        std::string::const_iterator searchStart(content.cbegin());
        
        ComponentDependency& dep = dependencies_[cvueFile.string()];
        dep.path = cvueFile;
        dep.lastModified = std::filesystem::last_write_time(cvueFile).time_since_epoch().count();
        
        while (std::regex_search(searchStart, content.cend(), matches, importRegex)) {
            dep.dependencies.push_back(matches[1].str());
            searchStart = matches.suffix().first;
        }
        
        searchStart = content.cbegin();
        while (std::regex_search(searchStart, content.cend(), matches, componentRegex)) {
            dep.dependencies.push_back(matches[1].str());
            searchStart = matches.suffix().first;
        }
    }
}

bool BuildTool::needsRecompilation(const std::filesystem::path& cvueFile) {
    auto it = dependencies_.find(cvueFile.string());
    if (it == dependencies_.end()) {
        return true;
    }
    
    auto currentTime = std::filesystem::last_write_time(cvueFile).time_since_epoch().count();
    if (currentTime > it->second.lastModified) {
        return true;
    }
    
    // Vérifie les dépendances
    for (const auto& dep : it->second.dependencies) {
        auto depIt = dependencies_.find(dep);
        if (depIt != dependencies_.end() && 
            depIt->second.lastModified > it->second.lastModified) {
            return true;
        }
    }
    
    return false;
}

void BuildTool::updateDependencyInfo(const std::filesystem::path& cvueFile) {
    auto& dep = dependencies_[cvueFile.string()];
    dep.lastModified = std::filesystem::last_write_time(cvueFile).time_since_epoch().count();
}

void BuildTool::notifyHotReload(const std::filesystem::path& component) {
    // TODO: Implémenter la notification WebSocket pour le hot reload
}

} // namespace cppvue::compiler
