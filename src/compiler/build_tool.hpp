#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

namespace cppvue::compiler {

struct BuildConfig {
    std::filesystem::path sourceDir;      // Répertoire source contenant les fichiers .cvue
    std::filesystem::path outputDir;      // Répertoire de sortie pour les fichiers générés
    std::filesystem::path buildDir;       // Répertoire de build pour les fichiers temporaires
    std::string projectName;              // Nom du projet
    bool enableHotReload = false;         // Activer le rechargement à chaud
    bool minifyCss = false;              // Minifier le CSS généré
    std::vector<std::string> includes;    // Chemins d'inclusion supplémentaires
};

class BuildTool {
public:
    explicit BuildTool(BuildConfig config);
    
    // Génère les fichiers de build
    bool generateBuildFiles();
    
    // Compile tous les fichiers .cvue
    bool buildComponents();
    
    // Surveille les changements et recompile automatiquement
    void watchAndRebuild();
    
private:
    // Génération des fichiers CMake
    void generateCMakeLists();
    void generateComponentRegistry();
    
    // Compilation des composants
    bool compileSingleComponent(const std::filesystem::path& cvueFile);
    void generateComponentLoader(const std::vector<std::filesystem::path>& components);
    
    // Gestion des dépendances
    struct ComponentDependency {
        std::filesystem::path path;
        std::time_t lastModified;
        std::vector<std::string> dependencies;
    };
    
    void analyzeDependencies();
    bool needsRecompilation(const std::filesystem::path& cvueFile);
    void updateDependencyInfo(const std::filesystem::path& cvueFile);
    
    // Hot Reload
    void setupHotReload();
    void notifyHotReload(const std::filesystem::path& component);
    
    // Helpers
    std::string generateComponentId(const std::filesystem::path& cvueFile);
    std::vector<std::filesystem::path> findCvueFiles();
    void ensureDirectories();
    
    BuildConfig config_;
    std::unordered_map<std::string, ComponentDependency> dependencies_;
};

// Classe pour la gestion des erreurs de build
class BuildError : public std::runtime_error {
public:
    explicit BuildError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace cppvue::compiler
