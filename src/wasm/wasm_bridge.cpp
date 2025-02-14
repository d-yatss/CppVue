#include "wasm_bridge.hpp"
#include "../core/component.hpp"
#include "../core/plugin.hpp"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <nlohmann/json.hpp>

namespace cppvue::wasm {

using json = nlohmann::json;

// Implémentation de JsBridge

emscripten::val JsBridge::createElement(const std::string& tag) {
    return emscripten::val::global("document").call<emscripten::val>("createElement", tag);
}

emscripten::val JsBridge::createTextNode(const std::string& text) {
    return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);
}

void JsBridge::appendChild(emscripten::val parent, emscripten::val child) {
    parent.call<void>("appendChild", child);
}

void JsBridge::removeChild(emscripten::val parent, emscripten::val child) {
    parent.call<void>("removeChild", child);
}

void JsBridge::setAttribute(emscripten::val element, 
                          const std::string& name, 
                          const std::string& value) {
    element.call<void>("setAttribute", name, value);
}

void JsBridge::removeAttribute(emscripten::val element, const std::string& name) {
    element.call<void>("removeAttribute", name);
}

void JsBridge::addEventListener(emscripten::val element,
                              const std::string& event,
                              std::function<void(emscripten::val)> callback) {
    // Crée un wrapper JavaScript pour le callback C++
    auto jsCallback = emscripten::val::module_property("createCallback")
        .call<emscripten::val>("bind", nullptr, callback);
    
    element.call<void>("addEventListener", event, jsCallback);
}

void JsBridge::removeEventListener(emscripten::val element,
                                 const std::string& event,
                                 std::function<void(emscripten::val)> callback) {
    // Retrouve le wrapper JavaScript
    auto jsCallback = emscripten::val::module_property("getCallback")
        .call<emscripten::val>("bind", nullptr, callback);
    
    element.call<void>("removeEventListener", event, jsCallback);
}

void JsBridge::setStyle(emscripten::val element,
                       const std::string& property,
                       const std::string& value) {
    element["style"].set(property, value);
}

std::string JsBridge::getStyle(emscripten::val element, const std::string& property) {
    return element["style"][property].as<std::string>();
}

void JsBridge::addClass(emscripten::val element, const std::string& className) {
    element["classList"].call<void>("add", className);
}

void JsBridge::removeClass(emscripten::val element, const std::string& className) {
    element["classList"].call<void>("remove", className);
}

bool JsBridge::hasClass(emscripten::val element, const std::string& className) {
    return element["classList"].call<bool>("contains", className);
}

void JsBridge::animate(emscripten::val element,
                      const std::vector<std::unordered_map<std::string, std::string>>& keyframes,
                      const std::unordered_map<std::string, std::any>& options) {
    // Convertit les keyframes en objet JavaScript
    auto jsKeyframes = emscripten::val::array();
    for (const auto& frame : keyframes) {
        auto jsFrame = emscripten::val::object();
        for (const auto& [prop, value] : frame) {
            jsFrame.set(prop, value);
        }
        jsKeyframes.call<void>("push", jsFrame);
    }
    
    // Convertit les options en objet JavaScript
    auto jsOptions = emscripten::val::object();
    for (const auto& [key, value] : options) {
        if (auto* duration = std::any_cast<int>(&value)) {
            jsOptions.set(key, *duration);
        } else if (auto* easing = std::any_cast<std::string>(&value)) {
            jsOptions.set(key, *easing);
        }
    }
    
    element.call<emscripten::val>("animate", jsKeyframes, jsOptions);
}

emscripten::val JsBridge::fetch(const std::string& url,
                               const std::unordered_map<std::string, std::any>& options) {
    // Convertit les options en objet JavaScript
    auto jsOptions = emscripten::val::object();
    for (const auto& [key, value] : options) {
        if (auto* method = std::any_cast<std::string>(&value)) {
            jsOptions.set(key, *method);
        } else if (auto* headers = std::any_cast<std::unordered_map<std::string, std::string>>(&value)) {
            auto jsHeaders = emscripten::val::object();
            for (const auto& [headerKey, headerValue] : *headers) {
                jsHeaders.set(headerKey, headerValue);
            }
            jsOptions.set(key, jsHeaders);
        } else if (auto* body = std::any_cast<std::string>(&value)) {
            jsOptions.set(key, *body);
        }
    }
    
    return emscripten::val::global("fetch").call<emscripten::val>(url, jsOptions);
}

void JsBridge::setItem(const std::string& key, const std::string& value) {
    emscripten::val::global("localStorage").call<void>("setItem", key, value);
}

std::string JsBridge::getItem(const std::string& key) {
    return emscripten::val::global("localStorage")
        .call<std::string>("getItem", key);
}

void JsBridge::removeItem(const std::string& key) {
    emscripten::val::global("localStorage").call<void>("removeItem", key);
}

void JsBridge::pushState(const std::string& url, const std::string& title) {
    emscripten::val::global("history")
        .call<void>("pushState", emscripten::val::object(), title, url);
}

void JsBridge::replaceState(const std::string& url, const std::string& title) {
    emscripten::val::global("history")
        .call<void>("replaceState", emscripten::val::object(), title, url);
}

std::string JsBridge::getCurrentUrl() {
    return emscripten::val::global("window")["location"]["href"].as<std::string>();
}

// Implémentation de DomElement

DomElement::DomElement(emscripten::val element) : element_(element) {}

std::string DomElement::id() const {
    return element_["id"].as<std::string>();
}

void DomElement::setId(const std::string& id) {
    element_.set("id", id);
}

std::string DomElement::className() const {
    return element_["className"].as<std::string>();
}

void DomElement::setClassName(const std::string& className) {
    element_.set("className", className);
}

std::string DomElement::innerHTML() const {
    return element_["innerHTML"].as<std::string>();
}

void DomElement::setInnerHTML(const std::string& html) {
    element_.set("innerHTML", html);
}

void DomElement::style(const std::string& property, const std::string& value) {
    JsBridge::instance().setStyle(element_, property, value);
}

std::string DomElement::style(const std::string& property) const {
    return JsBridge::instance().getStyle(element_, property);
}

void DomElement::setAttribute(const std::string& name, const std::string& value) {
    JsBridge::instance().setAttribute(element_, name, value);
}

std::string DomElement::getAttribute(const std::string& name) const {
    return element_.call<std::string>("getAttribute", name);
}

void DomElement::removeAttribute(const std::string& name) {
    JsBridge::instance().removeAttribute(element_, name);
}

void DomElement::on(const std::string& event, 
                   std::function<void(emscripten::val)> callback) {
    JsBridge::instance().addEventListener(element_, event, callback);
}

void DomElement::off(const std::string& event,
                    std::function<void(emscripten::val)> callback) {
    JsBridge::instance().removeEventListener(element_, event, callback);
}

void DomElement::append(const DomElement& child) {
    JsBridge::instance().appendChild(element_, child.element_);
}

void DomElement::remove(const DomElement& child) {
    JsBridge::instance().removeChild(element_, child.element_);
}

void DomElement::replaceWith(const DomElement& newElement) {
    element_.call<void>("replaceWith", newElement.element_);
}

void DomElement::animate(const std::vector<std::unordered_map<std::string, std::string>>& keyframes,
                        const std::unordered_map<std::string, std::any>& options) {
    JsBridge::instance().animate(element_, keyframes, options);
}

// Implémentation de WasmEventManager

void WasmEventManager::registerCallback(const std::string& event,
                                      std::function<void(emscripten::val)> callback) {
    callbacks_[event] = std::move(callback);
}

void WasmEventManager::removeCallback(const std::string& event) {
    callbacks_.erase(event);
}

void WasmEventManager::triggerEvent(const std::string& event, emscripten::val data) {
    if (auto it = callbacks_.find(event); it != callbacks_.end()) {
        it->second(data);
    }
}

// Implémentation des fonctions exportées

void initializeWasm() {
    // Initialise l'application
    auto& app = App::instance();
    
    // Configure les plugins de base
    app.use<RouterPlugin>(std::vector<Route>{});
    app.use<StorePlugin>(json::object());
    
    // Configure le bridge WebAssembly
    WasmEventManager::instance().registerCallback("hashchange", [](emscripten::val) {
        auto& router = Router::instance();
        router.updateRoute();
    });
}

void* createComponent(const char* name) {
    try {
        auto component = App::instance().createComponent(name);
        return component.get();
    } catch (const std::exception& e) {
        return nullptr;
    }
}

void updateProps(void* component, const char* props) {
    if (!component) return;
    
    try {
        auto* comp = static_cast<Component*>(component);
        auto propsJson = json::parse(props);
        comp->updateProps(propsJson);
    } catch (const std::exception& e) {
        // Log error
    }
}

void dispatchEvent(void* component, const char* event, const char* data) {
    if (!component) return;
    
    try {
        auto* comp = static_cast<Component*>(component);
        auto eventData = json::parse(data);
        comp->emit(event, eventData);
    } catch (const std::exception& e) {
        // Log error
    }
}

void hotReload(const char* componentName) {
    try {
        App::instance().reloadComponent(componentName);
    } catch (const std::exception& e) {
        // Log error
    }
}

// Binding Emscripten
EMSCRIPTEN_BINDINGS(cppvue) {
    emscripten::function("initializeWasm", &initializeWasm);
    emscripten::function("createComponent", &createComponent);
    emscripten::function("updateProps", &updateProps);
    emscripten::function("dispatchEvent", &dispatchEvent);
    emscripten::function("hotReload", &hotReload);
}

} // namespace cppvue::wasm
