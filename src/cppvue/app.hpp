#pragma once

#include "../core/component.hpp"
#include "../core/plugin.hpp"
#include "../core/store.hpp"
#include "../core/router.hpp"
#include "../wasm/wasm_bridge.hpp"
#include <memory>
#include <string>

namespace cppvue {

class App {
public:
    static App& instance() {
        static App app;
        return app;
    }
    
    // Configuration de l'application
    void mount(const std::string& selector) {
        rootElement_ = wasm::JsBridge::instance().querySelector(selector);
        if (!rootElement_.isNull()) {
            rootComponent_ = std::make_shared<Component>();
            rootComponent_->mount(rootElement_);
        }
    }
    
    void unmount() {
        if (rootComponent_) {
            rootComponent_->unmount();
            rootComponent_.reset();
        }
    }
    
    // Gestion des plugins
    template<typename P, typename... Args>
    void use(Args&&... args) {
        auto plugin = std::make_shared<P>(std::forward<Args>(args)...);
        plugin->install(*this);
        plugins_.push_back(plugin);
    }
    
    // Gestion des composants globaux
    void component(const std::string& name, std::shared_ptr<Component> component) {
        globalComponents_[name] = component;
    }
    
    std::shared_ptr<Component> createComponent(const std::string& name) {
        auto it = globalComponents_.find(name);
        if (it != globalComponents_.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // Gestion des directives globales
    void directive(const std::string& name, DirectiveHandler handler) {
        globalDirectives_[name] = handler;
    }
    
    // Hot Reload
    void reloadComponent(const std::string& name) {
        auto it = globalComponents_.find(name);
        if (it != globalComponents_.end()) {
            it->second->reload();
        }
    }
    
    // Accès aux ressources partagées
    std::shared_ptr<Router> router() const { return router_; }
    std::shared_ptr<Store> store() const { return store_; }
    
private:
    App() = default;
    
    std::vector<std::shared_ptr<Plugin>> plugins_;
    std::unordered_map<std::string, std::shared_ptr<Component>> globalComponents_;
    std::unordered_map<std::string, DirectiveHandler> globalDirectives_;
    
    std::shared_ptr<Router> router_;
    std::shared_ptr<Store> store_;
    std::shared_ptr<Component> rootComponent_;
    emscripten::val rootElement_;
};

} // namespace cppvue
