#pragma once

#include <string>
#include <variant>
#include <functional>
#include <unordered_map>
#include <memory>
#include "reactive.hpp"

namespace cppvue {

// Types de directives
enum class DirectiveType {
    IF,         // c-if
    ELSE,       // c-else
    ELSE_IF,    // c-else-if
    FOR,        // c-for
    MODEL,      // c-model
    ON,         // c-on / @
    BIND,       // c-bind / :
    SHOW,       // c-show
    TEXT,       // c-text
    HTML,       // c-html
    SLOT,       // c-slot
    REF,        // c-ref
    TRANSITION  // c-transition
};

// Structure pour les modificateurs
struct Modifiers {
    bool stop{false};      // .stop
    bool prevent{false};   // .prevent
    bool capture{false};   // .capture
    bool once{false};      // .once
    bool self{false};      // .self
    
    static Modifiers parse(const std::string& modifiersStr);
};

// Structure pour les directives
class Directive {
public:
    DirectiveType type;
    std::string value;
    std::string arg;
    Modifiers modifiers;
    
    static Directive parse(const std::string& directiveStr);
    
    // Helpers pour les types spécifiques
    bool isEvent() const { return type == DirectiveType::ON; }
    bool isBinding() const { return type == DirectiveType::BIND; }
    bool isConditional() const { 
        return type == DirectiveType::IF || 
               type == DirectiveType::ELSE || 
               type == DirectiveType::ELSE_IF; 
    }
};

// Gestionnaire de directives
class DirectiveHandler {
public:
    static void handleDirective(const Directive& directive, 
                              std::shared_ptr<class VNode> node,
                              class Component* component);
    
private:
    static void handleIf(const Directive& directive, 
                        std::shared_ptr<VNode> node,
                        Component* component);
                        
    static void handleFor(const Directive& directive,
                         std::shared_ptr<VNode> node,
                         Component* component);
                         
    static void handleModel(const Directive& directive,
                           std::shared_ptr<VNode> node,
                           Component* component);
                           
    static void handleEvent(const Directive& directive,
                           std::shared_ptr<VNode> node,
                           Component* component);
                           
    static void handleBind(const Directive& directive,
                          std::shared_ptr<VNode> node,
                          Component* component);
                          
    static void handleShow(const Directive& directive,
                          std::shared_ptr<VNode> node,
                          Component* component);
};

// Macro pour simplifier la création de directives
#define DIRECTIVE(name) DirectiveType::name, #name

} // namespace cppvue
