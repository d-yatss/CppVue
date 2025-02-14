#pragma once

#include "reactive.hpp"
#include "lifecycle.hpp"
#include "directives.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <any>

namespace cppvue {

// Forward declarations
class Component;
class Slot;

// Structure représentant un nœud du DOM virtuel
class VNode {
public:
    std::string tag;
    std::unordered_map<std::string, std::string> props;
    std::vector<std::shared_ptr<VNode>> children;
    std::string textContent;
    std::vector<Directive> directives;
    std::weak_ptr<Component> component;
    
    static std::shared_ptr<VNode> create(
        const std::string& tag,
        const std::unordered_map<std::string, std::string>& props = {},
        const std::vector<std::shared_ptr<VNode>>& children = {},
        const std::string& text = ""
    );
    
    // Helpers pour les directives
    void addDirective(const Directive& directive);
    bool hasDirective(DirectiveType type) const;
    const Directive& getDirective(DirectiveType type) const;
};

// Gestionnaire de slots
class Slot {
public:
    virtual ~Slot() = default;
    virtual std::shared_ptr<VNode> render(const std::unordered_map<std::string, std::any>& props = {}) = 0;
};

// Classe de base pour les composants
class Component : public std::enable_shared_from_this<Component> {
public:
    Component();
    virtual ~Component();
    
    // Méthode principale pour le rendu du composant
    virtual std::shared_ptr<VNode> render() = 0;
    
    // Cycle de vie
    LifecycleManager& lifecycle() { return lifecycle_; }
    
    // Gestion des props
    template<typename T>
    void setProp(const std::string& name, T&& value) {
        props_[name] = std::forward<T>(value);
    }
    
    template<typename T>
    T getProp(const std::string& name, const T& defaultValue = T{}) const {
        auto it = props_.find(name);
        if (it != props_.end()) {
            return std::any_cast<T>(it->second);
        }
        return defaultValue;
    }
    
    // Gestion des slots
    void setSlot(const std::string& name, std::shared_ptr<Slot> slot);
    std::shared_ptr<Slot> getSlot(const std::string& name) const;
    
    // Gestion des refs
    template<typename T>
    void setRef(const std::string& name, T&& value) {
        refs_[name] = std::forward<T>(value);
    }
    
    template<typename T>
    T getRef(const std::string& name) const {
        auto it = refs_.find(name);
        if (it != refs_.end()) {
            return std::any_cast<T>(it->second);
        }
        throw std::runtime_error("Ref not found: " + name);
    }
    
    // Émission d'événements
    template<typename... Args>
    void emit(const std::string& event, Args&&... args) {
        auto handler = eventHandlers_.find(event);
        if (handler != eventHandlers_.end()) {
            handler->second(std::forward<Args>(args)...);
        }
    }
    
    // Utilitaire pour créer des nœuds virtuels
    static std::shared_ptr<VNode> h(
        const std::string& tag,
        const std::unordered_map<std::string, std::string>& props = {},
        const std::vector<std::shared_ptr<VNode>>& children = {}
    );
    
    static std::shared_ptr<VNode> h(
        const std::string& tag,
        const std::vector<std::shared_ptr<VNode>>& children
    );
    
    static std::shared_ptr<VNode> h(
        const std::string& tag,
        const std::string& text
    );

protected:
    // Méthodes utilitaires pour la réactivité
    template<typename F>
    void watchEffect(F&& fn) {
        effects_.push_back(createEffect(std::forward<F>(fn)));
    }
    
    template<typename T, typename F>
    void watch(const Reactive<T>& source, F&& callback) {
        effects_.push_back(createEffect([&source, callback = std::forward<F>(callback)]() {
            callback(*source);
        }));
    }

private:
    LifecycleManager lifecycle_;
    std::vector<std::shared_ptr<Effect>> effects_;
    std::unordered_map<std::string, std::any> props_;
    std::unordered_map<std::string, std::any> refs_;
    std::unordered_map<std::string, std::shared_ptr<Slot>> slots_;
    std::unordered_map<std::string, std::function<void(std::any)>> eventHandlers_;
    
    friend class LifecycleWatchdog;
};

// Fonction utilitaire pour créer une instance de composant
template<typename T, typename... Args>
std::shared_ptr<Component> createComponent(Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
    auto component = std::make_shared<T>(std::forward<Args>(args)...);
    component->lifecycle().callHook(LifecycleHook::CREATED);
    return component;
}

// Fonction pour obtenir l'instance de composant courante
Component* getCurrentInstance();

} // namespace cppvue
