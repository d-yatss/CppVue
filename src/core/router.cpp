#include "router.hpp"
#include <regex>
#include <sstream>
#include <stack>

namespace cppvue {

namespace {
    // Structure pour représenter un segment de chemin
    struct PathSegment {
        std::string value;
        bool isDynamic;
        bool isOptional;
        bool isCatchAll;
    };
    
    // Parse un pattern de route en segments
    std::vector<PathSegment> parsePattern(const std::string& pattern) {
        std::vector<PathSegment> segments;
        std::istringstream stream(pattern);
        std::string segment;
        
        while (std::getline(stream, segment, '/')) {
            if (segment.empty()) continue;
            
            PathSegment pathSegment;
            if (segment[0] == ':') {
                // Paramètre dynamique
                pathSegment.isDynamic = true;
                pathSegment.value = segment.substr(1);
                pathSegment.isOptional = false;
                pathSegment.isCatchAll = false;
            } else if (segment[0] == '*') {
                // Catch-all
                pathSegment.isDynamic = true;
                pathSegment.value = segment.substr(1);
                pathSegment.isOptional = false;
                pathSegment.isCatchAll = true;
            } else if (segment.back() == '?') {
                // Paramètre optionnel
                pathSegment.isDynamic = true;
                pathSegment.value = segment.substr(0, segment.length() - 1);
                pathSegment.isOptional = true;
                pathSegment.isCatchAll = false;
            } else {
                // Segment statique
                pathSegment.isDynamic = false;
                pathSegment.value = segment;
                pathSegment.isOptional = false;
                pathSegment.isCatchAll = false;
            }
            
            segments.push_back(pathSegment);
        }
        
        return segments;
    }
}

bool Router::matchRoute(const std::string& pattern, const std::string& path) {
    auto patternSegments = parsePattern(pattern);
    auto pathSegments = parsePattern(path);
    
    size_t patternIndex = 0;
    size_t pathIndex = 0;
    
    std::unordered_map<std::string, std::string> params;
    
    while (patternIndex < patternSegments.size() && pathIndex < pathSegments.size()) {
        const auto& patternSegment = patternSegments[patternIndex];
        const auto& pathSegment = pathSegments[pathIndex];
        
        if (patternSegment.isCatchAll) {
            // Catch-all correspond à tous les segments restants
            std::string remainingPath;
            for (size_t i = pathIndex; i < pathSegments.size(); ++i) {
                if (!remainingPath.empty()) remainingPath += "/";
                remainingPath += pathSegments[i].value;
            }
            params[patternSegment.value] = remainingPath;
            return true;
        }
        
        if (patternSegment.isDynamic) {
            // Paramètre dynamique
            params[patternSegment.value] = pathSegment.value;
        } else if (patternSegment.value != pathSegment.value) {
            // Segment statique ne correspond pas
            if (patternSegment.isOptional) {
                patternIndex++;
                continue;
            }
            return false;
        }
        
        patternIndex++;
        pathIndex++;
    }
    
    // Vérifie s'il reste des segments non optionnels
    while (patternIndex < patternSegments.size()) {
        if (!patternSegments[patternIndex].isOptional) {
            return false;
        }
        patternIndex++;
    }
    
    return pathIndex == pathSegments.size();
}

void Router::push(const std::string& path) {
    // Vérifie les guards de navigation
    const auto& oldRoute = *currentRoute_;
    Route newRoute;
    
    // Trouve la nouvelle route
    for (const auto& route : routes_) {
        if (matchRoute(route.path, path)) {
            newRoute = route;
            break;
        }
    }
    
    // Exécute les guards de navigation
    for (const auto& guard : navigationGuards_) {
        if (!guard(oldRoute, newRoute)) {
            return; // Navigation annulée
        }
    }
    
    // Met à jour le chemin et la route
    currentPath_ = path;
    updateCurrentRoute();
    
    // Met à jour l'URL du navigateur
    updateBrowserHistory(path);
}

void Router::updateBrowserHistory(const std::string& path) {
    // TODO: Implémenter l'interaction avec l'historique du navigateur via WebAssembly
}

void Router::initializeFromBrowser() {
    // TODO: Initialiser le routeur avec l'URL actuelle du navigateur
}

// Implémentation de RouterView

std::shared_ptr<VNode> RouterView::render() {
    const auto& currentRoute = Router::instance().currentRoute();
    
    if (!currentRoute->component) {
        return createVNode("div", {{"class", "router-view-error"}},
                         {createTextVNode("404 Not Found")});
    }
    
    auto component = currentRoute->component();
    if (!component) {
        return createVNode("div", {{"class", "router-view-error"}},
                         {createTextVNode("Failed to load component")});
    }
    
    return component->render();
}

// Implémentation de RouterLink

std::shared_ptr<VNode> RouterLink::render() {
    auto router = Router::instance();
    bool isActive = router.currentPath()->find(to_) == 0;
    
    return createVNode("a",
        {
            {"href", to_},
            {"class", isActive ? "router-link-active" : ""},
            {"@click", [this](const std::any&) { navigate(); }}
        },
        slots_["default"] ? slots_["default"]() : std::vector<std::shared_ptr<VNode>>());
}

void RouterLink::navigate() {
    Router::instance().push(to_);
}

} // namespace cppvue
