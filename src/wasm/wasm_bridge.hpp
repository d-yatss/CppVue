#pragma once

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <functional>
#include <memory>

namespace cppvue::wasm {

// Bridge pour les appels JavaScript
class JsBridge {
public:
    static JsBridge& instance() {
        static JsBridge bridge;
        return bridge;
    }
    
    // DOM Manipulation
    emscripten::val createElement(const std::string& tag);
    emscripten::val createTextNode(const std::string& text);
    void appendChild(emscripten::val parent, emscripten::val child);
    void removeChild(emscripten::val parent, emscripten::val child);
    void setAttribute(emscripten::val element, const std::string& name, const std::string& value);
    void removeAttribute(emscripten::val element, const std::string& name);
    
    // Event Handling
    void addEventListener(emscripten::val element, 
                        const std::string& event,
                        std::function<void(emscripten::val)> callback);
    void removeEventListener(emscripten::val element,
                           const std::string& event,
                           std::function<void(emscripten::val)> callback);
    
    // Style Management
    void setStyle(emscripten::val element, const std::string& property, const std::string& value);
    std::string getStyle(emscripten::val element, const std::string& property);
    
    // Class Management
    void addClass(emscripten::val element, const std::string& className);
    void removeClass(emscripten::val element, const std::string& className);
    bool hasClass(emscripten::val element, const std::string& className);
    
    // Animation
    void animate(emscripten::val element,
                const std::vector<std::unordered_map<std::string, std::string>>& keyframes,
                const std::unordered_map<std::string, std::any>& options);
    
    // HTTP Requests
    emscripten::val fetch(const std::string& url,
                         const std::unordered_map<std::string, std::any>& options);
    
    // Local Storage
    void setItem(const std::string& key, const std::string& value);
    std::string getItem(const std::string& key);
    void removeItem(const std::string& key);
    
    // History API
    void pushState(const std::string& url, const std::string& title);
    void replaceState(const std::string& url, const std::string& title);
    std::string getCurrentUrl();
    
private:
    JsBridge() = default;
};

// Wrapper pour les éléments DOM
class DomElement {
public:
    explicit DomElement(emscripten::val element);
    
    // Propriétés
    std::string id() const;
    void setId(const std::string& id);
    
    std::string className() const;
    void setClassName(const std::string& className);
    
    std::string innerHTML() const;
    void setInnerHTML(const std::string& html);
    
    // Style
    void style(const std::string& property, const std::string& value);
    std::string style(const std::string& property) const;
    
    // Attributs
    void setAttribute(const std::string& name, const std::string& value);
    std::string getAttribute(const std::string& name) const;
    void removeAttribute(const std::string& name);
    
    // Événements
    void on(const std::string& event, std::function<void(emscripten::val)> callback);
    void off(const std::string& event, std::function<void(emscripten::val)> callback);
    
    // Manipulation
    void append(const DomElement& child);
    void remove(const DomElement& child);
    void replaceWith(const DomElement& newElement);
    
    // Animation
    void animate(const std::vector<std::unordered_map<std::string, std::string>>& keyframes,
                const std::unordered_map<std::string, std::any>& options);
    
private:
    emscripten::val element_;
};

// Gestionnaire d'événements WebAssembly
class WasmEventManager {
public:
    static WasmEventManager& instance() {
        static WasmEventManager manager;
        return manager;
    }
    
    // Enregistre un callback pour un événement
    void registerCallback(const std::string& event,
                        std::function<void(emscripten::val)> callback);
    
    // Supprime un callback
    void removeCallback(const std::string& event);
    
    // Déclenche un événement
    void triggerEvent(const std::string& event, emscripten::val data);
    
private:
    WasmEventManager() = default;
    std::unordered_map<std::string, std::function<void(emscripten::val)>> callbacks_;
};

// Fonctions exportées vers JavaScript
extern "C" {
    // Initialisation
    void EMSCRIPTEN_KEEPALIVE initializeWasm();
    
    // Création de composants
    void* EMSCRIPTEN_KEEPALIVE createComponent(const char* name);
    
    // Mise à jour des props
    void EMSCRIPTEN_KEEPALIVE updateProps(void* component, const char* props);
    
    // Gestion des événements
    void EMSCRIPTEN_KEEPALIVE dispatchEvent(void* component, const char* event, const char* data);
    
    // Hot Reload
    void EMSCRIPTEN_KEEPALIVE hotReload(const char* componentName);
}

} // namespace cppvue::wasm
