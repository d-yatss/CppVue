#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace cppvue::compiler {

// Types d'expressions dans le template
enum class ExpressionType {
    TEXT,           // Texte simple
    INTERPOLATION,  // {{ expression }}
    DIRECTIVE,      // c-if, c-for, etc.
    BINDING,        // :prop ou c-bind:prop
    EVENT,          // @event ou c-on:event
};

// Structure pour représenter une expression
struct Expression {
    ExpressionType type;
    std::string content;
    std::string arg;        // Pour les directives/bindings/events
    std::string modifiers;  // Pour les events
};

// Structure pour représenter un nœud dans l'AST
struct TemplateNode {
    enum class Type {
        ELEMENT,
        TEXT,
        EXPRESSION
    };
    
    Type type;
    std::string tag;        // Pour les éléments
    std::string content;    // Pour le texte/expressions
    
    std::vector<std::shared_ptr<TemplateNode>> children;
    std::unordered_map<std::string, Expression> attributes;
    std::unordered_map<std::string, Expression> directives;
    
    // Constructeurs helpers
    static std::shared_ptr<TemplateNode> createElement(const std::string& tag) {
        auto node = std::make_shared<TemplateNode>();
        node->type = Type::ELEMENT;
        node->tag = tag;
        return node;
    }
    
    static std::shared_ptr<TemplateNode> createText(const std::string& content) {
        auto node = std::make_shared<TemplateNode>();
        node->type = Type::TEXT;
        node->content = content;
        return node;
    }
    
    static std::shared_ptr<TemplateNode> createExpression(const std::string& content) {
        auto node = std::make_shared<TemplateNode>();
        node->type = Type::EXPRESSION;
        node->content = content;
        return node;
    }
};

// Parser de template
class TemplateParser {
public:
    // Parse un template en AST
    static std::shared_ptr<TemplateNode> parse(const std::string& template_content);
    
    // Génère le code C++ à partir de l'AST
    static std::string generateCode(std::shared_ptr<TemplateNode> ast);
    
private:
    // Helpers pour le parsing
    static std::vector<std::shared_ptr<TemplateNode>> parseChildren(const std::string& content);
    static std::unordered_map<std::string, Expression> parseAttributes(const std::string& tag);
    static Expression parseExpression(const std::string& content);
    
    // Helpers pour la génération de code
    static std::string generateNodeCode(std::shared_ptr<TemplateNode> node);
    static std::string generateAttributesCode(const std::unordered_map<std::string, Expression>& attrs);
    static std::string generateDirectivesCode(const std::unordered_map<std::string, Expression>& dirs);
    static std::string generateExpressionCode(const Expression& expr);
};

// Classe pour la gestion des erreurs de parsing
class TemplateParseError : public std::runtime_error {
public:
    explicit TemplateParseError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace cppvue::compiler
