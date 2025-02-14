#include "renderer.hpp"
#include <algorithm>
#include <queue>

namespace cppvue {

Renderer::Renderer(std::unique_ptr<PlatformRenderer> platformRenderer)
    : platformRenderer_(std::move(platformRenderer)) {}

void Renderer::mount(std::shared_ptr<Component> component, void* container) {
    // Appelle le hook beforeMount
    component->lifecycle().callHook(LifecycleHook::BEFORE_MOUNT);
    
    // Rend le composant
    auto vnode = component->render();
    
    // Crée l'élément DOM
    void* element = createDOMElement(vnode);
    
    // Ajoute l'élément au container
    platformRenderer_->appendChild(container, element);
    
    // Met à jour les caches
    nodeToElement_[vnode] = element;
    elementToNode_[element] = vnode;
    
    // Appelle le hook mounted
    component->lifecycle().callHook(LifecycleHook::MOUNTED);
}

void Renderer::update(std::shared_ptr<Component> component) {
    // Appelle le hook beforeUpdate
    component->lifecycle().callHook(LifecycleHook::BEFORE_UPDATE);
    
    // Obtient l'ancien et le nouveau VNode
    auto oldVNode = elementToNode_[nodeToElement_[component->render()]];
    auto newVNode = component->render();
    
    // Applique les différences
    patch(oldVNode, newVNode, nullptr);
    
    // Appelle le hook updated
    component->lifecycle().callHook(LifecycleHook::UPDATED);
}

void Renderer::unmount(std::shared_ptr<Component> component) {
    // Appelle le hook beforeUnmount
    component->lifecycle().callHook(LifecycleHook::BEFORE_UNMOUNT);
    
    // Trouve l'élément DOM
    auto vnode = component->render();
    auto element = nodeToElement_[vnode];
    
    // Supprime l'élément du DOM
    auto parent = platformRenderer_->getParentNode(element);
    platformRenderer_->removeChild(parent, element);
    
    // Nettoie les caches
    nodeToElement_.erase(vnode);
    elementToNode_.erase(element);
    
    // Appelle le hook unmounted
    component->lifecycle().callHook(LifecycleHook::UNMOUNTED);
}

void* Renderer::createDOMElement(std::shared_ptr<VNode> vnode) {
    void* element;
    
    if (vnode->tag.empty()) {
        // Nœud texte
        element = platformRenderer_->createTextNode(vnode->textContent);
    } else {
        // Élément normal
        element = platformRenderer_->createElement(vnode->tag);
        
        // Applique les attributs
        for (const auto& [name, value] : vnode->props) {
            platformRenderer_->setAttribute(element, name, value);
        }
        
        // Crée les enfants
        for (const auto& child : vnode->children) {
            void* childElement = createDOMElement(child);
            platformRenderer_->appendChild(element, childElement);
        }
    }
    
    return element;
}

void Renderer::patch(std::shared_ptr<VNode> oldNode, 
                    std::shared_ptr<VNode> newNode,
                    void* container) {
    if (!isSameVNode(oldNode, newNode)) {
        // Les nœuds sont différents, remplace complètement
        auto oldElement = nodeToElement_[oldNode];
        auto newElement = createDOMElement(newNode);
        
        platformRenderer_->insertBefore(container, newElement, oldElement);
        platformRenderer_->removeChild(container, oldElement);
        
        // Met à jour les caches
        nodeToElement_.erase(oldNode);
        elementToNode_.erase(oldElement);
        nodeToElement_[newNode] = newElement;
        elementToNode_[newElement] = newNode;
    } else {
        // Les nœuds sont similaires, met à jour
        auto element = nodeToElement_[oldNode];
        
        // Met à jour les props
        updateDOMElement(element, oldNode->props, newNode->props);
        
        // Met à jour les enfants
        patchChildren(oldNode, newNode, element);
        
        // Met à jour les caches
        nodeToElement_[newNode] = element;
        elementToNode_[element] = newNode;
    }
}

void Renderer::updateDOMElement(void* element,
                              const std::unordered_map<std::string, std::string>& oldProps,
                              const std::unordered_map<std::string, std::string>& newProps) {
    // Supprime les anciennes props qui n'existent plus
    for (const auto& [name, value] : oldProps) {
        if (newProps.find(name) == newProps.end()) {
            platformRenderer_->removeAttribute(element, name);
        }
    }
    
    // Ajoute ou met à jour les nouvelles props
    for (const auto& [name, value] : newProps) {
        auto oldValue = oldProps.find(name);
        if (oldValue == oldProps.end() || oldValue->second != value) {
            platformRenderer_->setAttribute(element, name, value);
        }
    }
}

void Renderer::patchChildren(std::shared_ptr<VNode> oldNode,
                           std::shared_ptr<VNode> newNode,
                           void* container) {
    const auto& oldChildren = oldNode->children;
    const auto& newChildren = newNode->children;
    
    // Algorithme de diff pour les enfants
    int oldStartIdx = 0;
    int oldEndIdx = oldChildren.size() - 1;
    int newStartIdx = 0;
    int newEndIdx = newChildren.size() - 1;
    
    while (oldStartIdx <= oldEndIdx && newStartIdx <= newEndIdx) {
        auto oldStartChild = oldChildren[oldStartIdx];
        auto oldEndChild = oldChildren[oldEndIdx];
        auto newStartChild = newChildren[newStartIdx];
        auto newEndChild = newChildren[newEndIdx];
        
        if (isSameVNode(oldStartChild, newStartChild)) {
            // Les premiers enfants sont identiques
            patch(oldStartChild, newStartChild, container);
            oldStartIdx++;
            newStartIdx++;
        } else if (isSameVNode(oldEndChild, newEndChild)) {
            // Les derniers enfants sont identiques
            patch(oldEndChild, newEndChild, container);
            oldEndIdx--;
            newEndIdx--;
        } else {
            // Cas plus complexes nécessitant des déplacements
            // TODO: Implémenter la réorganisation des nœuds
        }
    }
    
    // Ajoute les nouveaux nœuds restants
    while (newStartIdx <= newEndIdx) {
        auto newChild = newChildren[newStartIdx];
        auto element = createDOMElement(newChild);
        platformRenderer_->appendChild(container, element);
        newStartIdx++;
    }
    
    // Supprime les anciens nœuds restants
    while (oldStartIdx <= oldEndIdx) {
        auto oldChild = oldChildren[oldStartIdx];
        auto element = nodeToElement_[oldChild];
        platformRenderer_->removeChild(container, element);
        oldStartIdx++;
    }
}

bool Renderer::isSameVNode(std::shared_ptr<VNode> n1, std::shared_ptr<VNode> n2) {
    return n1->tag == n2->tag && n1->key == n2->key;
}

// Implémentation de WebRenderer

void* WebRenderer::createElement(const std::string& tag) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
    return nullptr;
}

void* WebRenderer::createTextNode(const std::string& text) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
    return nullptr;
}

void WebRenderer::setAttribute(void* element, const std::string& name, const std::string& value) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::removeAttribute(void* element, const std::string& name) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::setProperty(void* element, const std::string& name, const std::any& value) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::insertBefore(void* parent, void* newNode, void* referenceNode) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::removeChild(void* parent, void* child) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::appendChild(void* parent, void* child) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::addEventListener(void* element,
                                 const std::string& event,
                                 std::function<void(void*)> callback) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

void WebRenderer::removeEventListener(void* element,
                                    const std::string& event,
                                    std::function<void(void*)> callback) {
    // TODO: Implémenter l'appel à JavaScript via WebAssembly
}

} // namespace cppvue
