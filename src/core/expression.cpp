#include "expression.hpp"
#include "component.hpp"
#include <cctype>
#include <sstream>
#include <stack>

namespace cppvue {

std::any EvaluationContext::getVariable(const std::string& name) const {
    // Cherche dans les scopes du plus récent au plus ancien
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto var = it->find(name);
        if (var != it->end()) {
            return var->second;
        }
    }
    
    // Si non trouvé dans les scopes, cherche dans le composant
    if (component_) {
        return component_->getVariable(name);
    }
    
    throw std::runtime_error("Variable not found: " + name);
}

void EvaluationContext::setVariable(const std::string& name, std::any value) {
    if (scopes_.empty()) {
        pushScope();
    }
    scopes_.top()[name] = std::move(value);
}

void EvaluationContext::pushScope() {
    scopes_.push({});
}

void EvaluationContext::popScope() {
    if (!scopes_.empty()) {
        scopes_.pop();
    }
}

std::vector<ExpressionParser::Token> ExpressionParser::tokenize(const std::string& input) {
    std::vector<Token> tokens;
    std::string::const_iterator it = input.begin();
    
    while (it != input.end()) {
        // Ignore les espaces
        if (std::isspace(*it)) {
            ++it;
            continue;
        }
        
        // Nombres
        if (std::isdigit(*it)) {
            std::string number;
            bool hasDecimal = false;
            
            while (it != input.end() && 
                   (std::isdigit(*it) || (!hasDecimal && *it == '.'))) {
                if (*it == '.') hasDecimal = true;
                number += *it++;
            }
            
            tokens.push_back({Token::Type::NUMBER, number});
            continue;
        }
        
        // Identifiants
        if (isIdentifierStart(*it)) {
            std::string identifier;
            
            while (it != input.end() && isIdentifierPart(*it)) {
                identifier += *it++;
            }
            
            tokens.push_back({Token::Type::IDENTIFIER, identifier});
            continue;
        }
        
        // Chaînes de caractères
        if (*it == '"' || *it == '\'') {
            char quote = *it++;
            std::string str;
            
            while (it != input.end() && *it != quote) {
                if (*it == '\\' && (it + 1) != input.end()) {
                    ++it;
                    switch (*it) {
                        case 'n': str += '\n'; break;
                        case 't': str += '\t'; break;
                        case 'r': str += '\r'; break;
                        default: str += *it;
                    }
                } else {
                    str += *it;
                }
                ++it;
            }
            
            if (it != input.end()) ++it; // Skip closing quote
            tokens.push_back({Token::Type::STRING, str});
            continue;
        }
        
        // Opérateurs
        if (isOperator(*it)) {
            std::string op;
            while (it != input.end() && isOperator(*it)) {
                op += *it++;
            }
            tokens.push_back({Token::Type::OPERATOR, op});
            continue;
        }
        
        // Ponctuation
        if (isPunctuation(*it)) {
            tokens.push_back({Token::Type::PUNCTUATION, std::string(1, *it)});
            ++it;
            continue;
        }
        
        // Caractère invalide
        throw std::runtime_error("Invalid character in expression: " + 
                               std::string(1, *it));
    }
    
    return tokens;
}

Expression ExpressionParser::parse(const std::string& expressionStr) {
    auto tokens = tokenize(expressionStr);
    return parseTokens(tokens);
}

Expression ExpressionParser::parseTokens(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        return Expression::literal(std::string());
    }
    
    // Pour l'instant, on gère uniquement les expressions simples
    if (tokens.size() == 1) {
        const auto& token = tokens[0];
        switch (token.type) {
            case Token::Type::IDENTIFIER:
                return Expression::identifier(token.value);
                
            case Token::Type::NUMBER:
                if (token.value.find('.') != std::string::npos) {
                    return Expression::literal(std::stod(token.value));
                } else {
                    return Expression::literal(std::stoi(token.value));
                }
                
            case Token::Type::STRING:
                return Expression::literal(token.value);
                
            default:
                throw std::runtime_error("Unexpected token type");
        }
    }
    
    // TODO: Implémenter le parsing d'expressions plus complexes
    // (opérateurs, appels de méthodes, etc.)
    
    throw std::runtime_error("Complex expressions not yet implemented");
}

bool ExpressionParser::isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || 
           c == '=' || c == '!' || c == '<' || c == '>' || 
           c == '&' || c == '|' || c == '^';
}

bool ExpressionParser::isPunctuation(char c) {
    return c == '(' || c == ')' || c == '[' || c == ']' || 
           c == '{' || c == '}' || c == '.' || c == ',' || 
           c == ';';
}

bool ExpressionParser::isIdentifierStart(char c) {
    return std::isalpha(c) || c == '_' || c == '$';
}

bool ExpressionParser::isIdentifierPart(char c) {
    return isIdentifierStart(c) || std::isdigit(c);
}

} // namespace cppvue
