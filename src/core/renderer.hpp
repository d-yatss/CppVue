#pragma once

#include "component.hpp"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace cppvue {

// Forward declarations
class VNode;
class Component;

// Interface pour le rendu platform-specific
class PlatformRenderer {
public:
    virtual ~PlatformRenderer() = default;
    
    // Création d'éléments
    virtual void* createElement(const std::string& tag) = 0;
    virtual void* createTextNode(const std::string& text) = 0;
    
    // Manipulation des éléments
    virtual void setAttribute(void* element, const std::string& name, const std::string& value) = 0;
    virtual void removeAttribute(void* element, const std::string& name) = 0;
    virtual void setProperty(void* element, const std::string& name, const std::any& value) = 0;
    
    // Manipulation du DOM
    virtual void insertBefore(void* parent, void* newNode, void* referenceNode) = 0;
    virtual void removeChild(void* parent, void* child) = 0;
    virtual void appendChild(void* parent, void* child) = 0;
    
    // Événements
    virtual void addEventListener(void* element, 
                               const std::string& event, 
                               std::function<void(void*)> callback) = 0;
    virtual void removeEventListener(void* element, 
                                   const std::string& event, 
                                   std::function<void(void*)> callback) = 0;
};

// Classe principale du renderer
class Renderer {
public:
    explicit Renderer(std::unique_ptr<PlatformRenderer> platformRenderer);
    
    // Montage initial d'un composant
    void mount(std::shared_ptr<Component> component, void* container);
    
    // Mise à jour d'un composant
    void update(std::shared_ptr<Component> component);
    
    // Démontage d'un composant
    void unmount(std::shared_ptr<Component> component);
    
private:
    // Algorithme de diff et patch
    void patch(std::shared_ptr<VNode> oldNode, 
              std::shared_ptr<VNode> newNode, 
              void* container);
    
    // Création d'éléments DOM
    void* createDOMElement(std::shared_ptr<VNode> vnode);
    void updateDOMElement(void* element, 
                         const std::unordered_map<std::string, std::string>& oldProps,
                         const std::unordered_map<std::string, std::string>& newProps);
    
    // Gestion des composants
    void mountComponent(std::shared_ptr<Component> component, void* container);
    void updateComponent(std::shared_ptr<Component> component);
    void unmountComponent(std::shared_ptr<Component> component);
    
    // Helpers
    bool isSameVNode(std::shared_ptr<VNode> n1, std::shared_ptr<VNode> n2);
    void patchChildren(std::shared_ptr<VNode> oldNode, 
                      std::shared_ptr<VNode> newNode,
                      void* container);
    
    // Gestion des événements
    void patchEvents(void* element,
                    const std::unordered_map<std::string, std::any>& oldEvents,
                    const std::unordered_map<std::string, std::any>& newEvents);
    
    // Cache pour les éléments DOM
    std::unordered_map<std::shared_ptr<VNode>, void*> nodeToElement_;
    std::unordered_map<void*, std::shared_ptr<VNode>> elementToNode_;
    
    // Renderer spécifique à la plateforme
    std::unique_ptr<PlatformRenderer> platformRenderer_;
};

// Classe pour le rendu Web (WebAssembly)
class WebRenderer : public PlatformRenderer {
public:
    void* createElement(const std::string& tag) override;
    void* createTextNode(const std::string& text) override;
    void setAttribute(void* element, const std::string& name, const std::string& value) override;
    void removeAttribute(void* element, const std::string& name) override;
    void setProperty(void* element, const std::string& name, const std::any& value) override;
    void insertBefore(void* parent, void* newNode, void* referenceNode) override;
    void removeChild(void* parent, void* child) override;
    void appendChild(void* parent, void* child) override;
    void addEventListener(void* element, 
                        const std::string& event,
                        std::function<void(void*)> callback) override;
    void removeEventListener(void* element,
                           const std::string& event,
                           std::function<void(void*)> callback) override;
    
private:
    // Helpers pour la conversion entre C++ et JavaScript
    static std::string serializeValue(const std::any& value);
    static std::any deserializeValue(const std::string& value);
};

} // namespace cppvue
