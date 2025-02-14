#pragma once

#include "component.hpp"
#include "reactive.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace cppvue {

struct Route {
    std::string path;
    std::string name;
    std::function<std::shared_ptr<Component>()> component;
    std::unordered_map<std::string, std::string> meta;
};

class Router {
public:
    static Router& instance() {
        static Router router;
        return router;
    }

    // Configuration des routes
    void addRoute(Route route) {
        routes_.push_back(std::move(route));
    }

    // Navigation programmatique
    void push(const std::string& path) {
        currentPath_ = path;
        updateCurrentRoute();
    }

    void replace(const std::string& path) {
        push(path);
        // Mise à jour de l'historique du navigateur
    }

    // Hooks de navigation
    void beforeEach(std::function<bool(const Route&, const Route&)> guard) {
        navigationGuards_.push_back(std::move(guard));
    }

    // État réactif du routeur
    const Reactive<std::string>& currentPath() const { return currentPath_; }
    const Reactive<Route>& currentRoute() const { return currentRoute_; }

private:
    Router() = default;
    
    void updateCurrentRoute() {
        for (const auto& route : routes_) {
            if (matchRoute(route.path, *currentPath_)) {
                currentRoute_ = route;
                break;
            }
        }
    }

    bool matchRoute(const std::string& pattern, const std::string& path);

    std::vector<Route> routes_;
    Reactive<std::string> currentPath_;
    Reactive<Route> currentRoute_;
    std::vector<std::function<bool(const Route&, const Route&)>> navigationGuards_;
};

// Helper pour créer un routeur
inline auto createRouter(const std::vector<Route>& routes) {
    auto& router = Router::instance();
    for (const auto& route : routes) {
        router.addRoute(route);
    }
    return router;
}

// Composant RouterView
class RouterView : public Component {
public:
    std::shared_ptr<VNode> render() override {
        const auto& currentRoute = Router::instance().currentRoute();
        if (auto component = currentRoute->component()) {
            return component->render();
        }
        return h("div", "404 Not Found");
    }
};

// Composant RouterLink
class RouterLink : public Component {
public:
    explicit RouterLink(std::string to) : to_(std::move(to)) {}

    std::shared_ptr<VNode> render() override {
        return h("a", 
            {{"href", to_}, {"@click", "navigate"}},
            {h("slot")}
        );
    }

private:
    void navigate() {
        Router::instance().push(to_);
    }

    std::string to_;
};

} // namespace cppvue
