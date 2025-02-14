#pragma once

#include "component.hpp"
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>

namespace cppvue {

// Interface pour les plugins
class Plugin {
public:
    virtual ~Plugin() = default;
    virtual void install(class App& app) = 0;
};

// Application principale
class App {
public:
    static App& instance() {
        static App app;
        return app;
    }
    
    // Configuration de l'application
    void mount(const std::string& selector);
    void unmount();
    
    // Gestion des plugins
    template<typename P, typename... Args>
    void use(Args&&... args) {
        auto plugin = std::make_shared<P>(std::forward<Args>(args)...);
        plugin->install(*this);
        plugins_.push_back(plugin);
    }
    
    // Gestion des composants globaux
    void component(const std::string& name, std::shared_ptr<Component> component);
    
    // Gestion des directives globales
    void directive(const std::string& name, DirectiveHandler handler);
    
    // Gestion des mixins globaux
    void mixin(const Mixin& mixin);
    
    // Configuration globale
    template<typename T>
    void config(const std::string& key, T value) {
        config_[key] = std::any(value);
    }
    
    template<typename T>
    T getConfig(const std::string& key) const {
        auto it = config_.find(key);
        if (it != config_.end()) {
            return std::any_cast<T>(it->second);
        }
        throw std::runtime_error("Config key not found: " + key);
    }
    
    // Accès aux ressources partagées
    std::shared_ptr<Router> router() const { return router_; }
    std::shared_ptr<Store> store() const { return store_; }
    
private:
    App() = default;
    
    std::vector<std::shared_ptr<Plugin>> plugins_;
    std::unordered_map<std::string, std::shared_ptr<Component>> globalComponents_;
    std::unordered_map<std::string, DirectiveHandler> globalDirectives_;
    std::vector<Mixin> globalMixins_;
    std::unordered_map<std::string, std::any> config_;
    
    std::shared_ptr<Router> router_;
    std::shared_ptr<Store> store_;
    std::shared_ptr<Component> rootComponent_;
};

// Plugin pour le routage
class RouterPlugin : public Plugin {
public:
    explicit RouterPlugin(const std::vector<Route>& routes);
    void install(App& app) override;
    
private:
    std::vector<Route> routes_;
};

// Plugin pour la gestion d'état
class StorePlugin : public Plugin {
public:
    template<typename State>
    explicit StorePlugin(const State& initialState) {
        store_ = defineStore("main", initialState);
    }
    
    void install(App& app) override;
    
private:
    std::shared_ptr<Store> store_;
};

// Plugin pour l'internationalisation
class I18nPlugin : public Plugin {
public:
    explicit I18nPlugin(const std::unordered_map<std::string, 
                       std::unordered_map<std::string, std::string>>& messages);
    void install(App& app) override;
    
private:
    std::unordered_map<std::string, 
                      std::unordered_map<std::string, std::string>> messages_;
};

// Plugin pour les validations de formulaire
class ValidationPlugin : public Plugin {
public:
    void install(App& app) override;
    void addRule(const std::string& name, std::function<bool(const std::string&)> validator);
    
private:
    std::unordered_map<std::string, std::function<bool(const std::string&)>> rules_;
};

// Plugin pour les animations
class AnimationPlugin : public Plugin {
public:
    void install(App& app) override;
    void addTransition(const std::string& name, const Transition& transition);
    
private:
    std::unordered_map<std::string, Transition> transitions_;
};

// Plugin pour les requêtes HTTP
class HttpPlugin : public Plugin {
public:
    void install(App& app) override;
    
    template<typename T>
    std::future<T> get(const std::string& url);
    
    template<typename T, typename U>
    std::future<T> post(const std::string& url, const U& data);
    
private:
    std::string baseUrl_;
    std::unordered_map<std::string, std::string> headers_;
};

} // namespace cppvue
