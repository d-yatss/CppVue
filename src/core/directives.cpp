#include "directives.hpp"
#include "component.hpp"
#include <regex>
#include <sstream>

namespace cppvue {

Modifiers Modifiers::parse(const std::string& modifiersStr) {
    Modifiers mods;
    std::istringstream stream(modifiersStr);
    std::string modifier;
    
    while (std::getline(stream, modifier, '.')) {
        if (modifier == "stop") mods.stop = true;
        else if (modifier == "prevent") mods.prevent = true;
        else if (modifier == "capture") mods.capture = true;
        else if (modifier == "once") mods.once = true;
        else if (modifier == "self") mods.self = true;
    }
    
    return mods;
}

Directive Directive::parse(const std::string& directiveStr) {
    // Format: c-if, c-for, c-model, etc.
    // ou: c-on:click.stop, c-bind:class, etc.
    std::regex directiveRegex("c-(\\w+)(?::(\\w+))?(?:\\.(\\w+(?:\\.\\w+)*))?");
    std::smatch matches;
    
    Directive directive;
    if (std::regex_match(directiveStr, matches, directiveRegex)) {
        std::string name = matches[1].str();
        
        // Détermine le type de directive
        if (name == "if") directive.type = DirectiveType::IF;
        else if (name == "else") directive.type = DirectiveType::ELSE;
        else if (name == "else-if") directive.type = DirectiveType::ELSE_IF;
        else if (name == "for") directive.type = DirectiveType::FOR;
        else if (name == "model") directive.type = DirectiveType::MODEL;
        else if (name == "on") directive.type = DirectiveType::ON;
        else if (name == "bind") directive.type = DirectiveType::BIND;
        else if (name == "show") directive.type = DirectiveType::SHOW;
        
        // Argument de la directive (ex: click dans @click)
        directive.arg = matches[2].str();
        
        // Parse les modificateurs
        if (matches[3].matched) {
            directive.modifiers = Modifiers::parse(matches[3].str());
        }
    }
    
    return directive;
}

void DirectiveHandler::handleDirective(const Directive& directive, 
                                     std::shared_ptr<VNode> node,
                                     Component* component) {
    switch (directive.type) {
        case DirectiveType::IF:
            handleIf(directive, node, component);
            break;
        case DirectiveType::FOR:
            handleFor(directive, node, component);
            break;
        case DirectiveType::MODEL:
            handleModel(directive, node, component);
            break;
        case DirectiveType::ON:
            handleEvent(directive, node, component);
            break;
        case DirectiveType::BIND:
            handleBind(directive, node, component);
            break;
        case DirectiveType::SHOW:
            handleShow(directive, node, component);
            break;
        default:
            break;
    }
}

void DirectiveHandler::handleIf(const Directive& directive,
                              std::shared_ptr<VNode> node,
                              Component* component) {
    // Évalue l'expression conditionnelle
    bool condition = component->evaluateExpression<bool>(directive.value);
    if (!condition) {
        node->tag = ""; // Node ne sera pas rendu
        node->children.clear();
    }
}

void DirectiveHandler::handleFor(const Directive& directive,
                               std::shared_ptr<VNode> node,
                               Component* component) {
    // Parse l'expression "item in items"
    std::regex forRegex("(\\w+)\\s+in\\s+(\\w+)");
    std::smatch matches;
    if (std::regex_match(directive.value, matches, forRegex)) {
        std::string itemName = matches[1].str();
        std::string itemsName = matches[2].str();
        
        // Obtient la collection à itérer
        auto items = component->evaluateExpression<std::vector<std::any>>(itemsName);
        
        // Crée un nœud pour chaque élément
        std::vector<std::shared_ptr<VNode>> newNodes;
        for (const auto& item : items) {
            // Crée une copie du template
            auto clone = std::make_shared<VNode>(*node);
            
            // Ajoute l'élément au scope local
            component->pushScope();
            component->setScopeVariable(itemName, item);
            
            // Rend le nœud avec le nouvel élément
            newNodes.push_back(clone);
            
            component->popScope();
        }
        
        // Remplace le nœud original par les nouveaux nœuds
        node->children = newNodes;
    }
}

void DirectiveHandler::handleModel(const Directive& directive,
                                 std::shared_ptr<VNode> node,
                                 Component* component) {
    // Ajoute la liaison de valeur
    node->props["value"] = component->evaluateExpression<std::string>(directive.value);
    
    // Ajoute l'événement input pour la mise à jour
    Directive inputEvent;
    inputEvent.type = DirectiveType::ON;
    inputEvent.arg = "input";
    inputEvent.value = directive.value + " = $event.target.value";
    handleEvent(inputEvent, node, component);
}

void DirectiveHandler::handleEvent(const Directive& directive,
                                 std::shared_ptr<VNode> node,
                                 Component* component) {
    std::string eventName = directive.arg;
    
    // Crée le gestionnaire d'événements
    auto handler = [component, directive](const std::any& event) {
        if (directive.modifiers.stop) {
            // Implémentation de stopPropagation
        }
        if (directive.modifiers.prevent) {
            // Implémentation de preventDefault
        }
        
        component->evaluateExpression<void>(directive.value, {{"$event", event}});
    };
    
    // Ajoute le gestionnaire d'événements au nœud
    node->props["on" + eventName] = handler;
}

void DirectiveHandler::handleBind(const Directive& directive,
                                std::shared_ptr<VNode> node,
                                Component* component) {
    // Évalue l'expression et lie la valeur à l'attribut
    auto value = component->evaluateExpression<std::string>(directive.value);
    node->props[directive.arg] = value;
}

void DirectiveHandler::handleShow(const Directive& directive,
                                std::shared_ptr<VNode> node,
                                Component* component) {
    bool show = component->evaluateExpression<bool>(directive.value);
    if (!show) {
        node->props["style"] = "display: none;";
    }
}

} // namespace cppvue
