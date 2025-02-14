#pragma once

#include "reactive.hpp"
#include "component.hpp"
#include <string>
#include <variant>
#include <functional>

namespace cppvue {

// Système de directives similaire à Vue3
enum class DirectiveType {
    IF,         // v-if
    FOR,        // v-for
    MODEL,      // v-model
    ON,         // v-on / @
    BIND,       // v-bind / :
    SHOW        // v-show
};

struct Directive {
    DirectiveType type;
    std::string value;
    std::string arg;
    std::string modifiers;
};

// Support des expressions
class Expression {
public:
    template<typename T>
    static Expression fromValue(T value) {
        return Expression([value]() -> std::any { return value; });
    }

    template<typename F>
    static Expression fromFunction(F&& fn) {
        return Expression(std::forward<F>(fn));
    }

    std::any evaluate() const { return fn_(); }

private:
    explicit Expression(std::function<std::any()> fn) : fn_(std::move(fn)) {}
    std::function<std::any()> fn_;
};

// Support des templates comme dans Vue3
class Template {
public:
    Template(std::string html) : html_(std::move(html)) {}
    
    // Compilation du template en VNode
    std::shared_ptr<VNode> compile();
    
    // Support des slots
    void setSlot(const std::string& name, std::function<std::shared_ptr<VNode>()> slot) {
        slots_[name] = std::move(slot);
    }

private:
    std::string html_;
    std::unordered_map<std::string, std::function<std::shared_ptr<VNode>()>> slots_;
    
    // Méthodes internes de parsing
    std::shared_ptr<VNode> parseElement(const std::string& element);
    std::vector<Directive> parseDirectives(const std::string& attrs);
};

// Macro pour simplifier la création de templates
#define TEMPLATE(html) Template(#html)

// Support des styles scoped
class Style {
public:
    explicit Style(std::string css) : css_(std::move(css)) {}
    
    std::string compile(const std::string& componentId) const;

private:
    std::string css_;
};

} // namespace cppvue
