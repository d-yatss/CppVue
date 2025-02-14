#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <functional>
#include <memory>
#include <stack>
#include <vector>

namespace cppvue {

// Forward declarations
class Component;

// Classe pour représenter une expression
class Expression {
public:
    // Constructeur pour une expression littérale
    template<typename T>
    static Expression literal(T value) {
        return Expression([value]() -> std::any { return value; });
    }
    
    // Constructeur pour une expression référençant une variable
    static Expression identifier(const std::string& name) {
        return Expression([name](const EvaluationContext& ctx) -> std::any {
            return ctx.getVariable(name);
        });
    }
    
    // Évalue l'expression dans un contexte donné
    template<typename T>
    T evaluate(const EvaluationContext& context = EvaluationContext()) const {
        return std::any_cast<T>(evaluator_(context));
    }
    
private:
    using Evaluator = std::function<std::any(const EvaluationContext&)>;
    
    explicit Expression(Evaluator evaluator)
        : evaluator_(std::move(evaluator)) {}
    
    Evaluator evaluator_;
};

// Contexte d'évaluation des expressions
class EvaluationContext {
public:
    explicit EvaluationContext(Component* component = nullptr)
        : component_(component) {}
    
    // Accès aux variables
    std::any getVariable(const std::string& name) const;
    void setVariable(const std::string& name, std::any value);
    
    // Gestion du scope
    void pushScope();
    void popScope();
    
    // Accès au composant
    Component* component() const { return component_; }
    
private:
    Component* component_;
    std::stack<std::unordered_map<std::string, std::any>> scopes_;
    
    friend class Expression;
};

// Parser d'expressions
class ExpressionParser {
public:
    static Expression parse(const std::string& expressionStr);
    
private:
    struct Token {
        enum class Type {
            IDENTIFIER,
            NUMBER,
            STRING,
            OPERATOR,
            PUNCTUATION
        };
        
        Type type;
        std::string value;
    };
    
    static std::vector<Token> tokenize(const std::string& input);
    static Expression parseTokens(const std::vector<Token>& tokens);
    
    // Helpers pour le parsing
    static bool isOperator(char c);
    static bool isPunctuation(char c);
    static bool isIdentifierStart(char c);
    static bool isIdentifierPart(char c);
};

// Macro pour simplifier la création d'expressions
#define EXPR(str) ExpressionParser::parse(str)

} // namespace cppvue
