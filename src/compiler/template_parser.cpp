#include "template_parser.hpp"
#include <regex>
#include <stack>
#include <sstream>

namespace cppvue::compiler {

namespace {
    // Regex pour le parsing
    const std::regex TAG_REGEX("<([/]?)([^>\\s]+)\\s*([^>]*)>");
    const std::regex ATTR_REGEX("([^\\s=]+)(?:=([\"'])(.*?)\\2)?");
    const std::regex INTERP_REGEX("\\{\\{\\s*(.+?)\\s*\\}\\}");
    const std::regex DIR_REGEX("c-([^:\\s]+)(?::([^.\\s]+))?(?:\\.([^\\s]+))?");
    const std::regex EVENT_REGEX("@([^.\\s]+)(?:\\.([^\\s]+))?");
    const std::regex BIND_REGEX(":([^\\s]+)");
    
    // Helper pour échapper les chaînes C++
    std::string escapeString(const std::string& str) {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '\"': ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default: ss << c;
            }
        }
        return ss.str();
    }
}

std::shared_ptr<TemplateNode> TemplateParser::parse(const std::string& template_content) {
    std::stack<std::shared_ptr<TemplateNode>> nodeStack;
    auto root = TemplateNode::createElement("template");
    nodeStack.push(root);
    
    std::string::const_iterator searchStart(template_content.cbegin());
    std::smatch matches;
    
    while (std::regex_search(searchStart, template_content.cend(), matches, TAG_REGEX)) {
        // Traite le texte avant le tag
        if (matches.prefix().length() > 0) {
            std::string text = matches.prefix().str();
            // Parse les interpolations dans le texte
            std::string::const_iterator textStart(text.cbegin());
            std::smatch interpMatches;
            
            while (std::regex_search(textStart, text.cend(), interpMatches, INTERP_REGEX)) {
                if (interpMatches.prefix().length() > 0) {
                    nodeStack.top()->children.push_back(
                        TemplateNode::createText(interpMatches.prefix().str()));
                }
                
                auto expr = TemplateNode::createExpression(interpMatches[1].str());
                nodeStack.top()->children.push_back(expr);
                
                textStart = interpMatches.suffix().first;
            }
            
            if (textStart != text.cend()) {
                nodeStack.top()->children.push_back(
                    TemplateNode::createText(std::string(textStart, text.cend())));
            }
        }
        
        bool isClosing = matches[1].length() > 0;
        std::string tagName = matches[2];
        std::string attrs = matches[3];
        
        if (isClosing) {
            if (nodeStack.empty() || nodeStack.top()->tag != tagName) {
                throw TemplateParseError("Mismatched closing tag: " + tagName);
            }
            nodeStack.pop();
        } else {
            auto node = TemplateNode::createElement(tagName);
            
            // Parse les attributs
            std::string::const_iterator attrStart(attrs.cbegin());
            std::smatch attrMatches;
            
            while (std::regex_search(attrStart, attrs.cend(), attrMatches, ATTR_REGEX)) {
                std::string attrName = attrMatches[1];
                std::string attrValue = attrMatches[3];
                
                // Vérifie si c'est une directive
                std::smatch dirMatches;
                if (std::regex_match(attrName, dirMatches, DIR_REGEX)) {
                    Expression expr;
                    expr.type = ExpressionType::DIRECTIVE;
                    expr.content = attrValue;
                    expr.arg = dirMatches[2];
                    expr.modifiers = dirMatches[3];
                    node->directives[dirMatches[1]] = expr;
                }
                // Vérifie si c'est un événement
                else if (std::regex_match(attrName, dirMatches, EVENT_REGEX)) {
                    Expression expr;
                    expr.type = ExpressionType::EVENT;
                    expr.content = attrValue;
                    expr.arg = dirMatches[1];
                    expr.modifiers = dirMatches[2];
                    node->directives["on"] = expr;
                }
                // Vérifie si c'est un binding
                else if (std::regex_match(attrName, dirMatches, BIND_REGEX)) {
                    Expression expr;
                    expr.type = ExpressionType::BINDING;
                    expr.content = attrValue;
                    expr.arg = dirMatches[1];
                    node->directives["bind"] = expr;
                }
                // Attribut normal
                else {
                    Expression expr;
                    expr.type = ExpressionType::TEXT;
                    expr.content = attrValue;
                    node->attributes[attrName] = expr;
                }
                
                attrStart = attrMatches.suffix().first;
            }
            
            nodeStack.top()->children.push_back(node);
            
            // Si ce n'est pas un tag auto-fermant, l'ajoute à la pile
            if (tagName != "img" && tagName != "input" && tagName != "br" && 
                tagName != "hr" && tagName != "meta") {
                nodeStack.push(node);
            }
        }
        
        searchStart = matches.suffix().first;
    }
    
    // Traite le texte restant
    if (searchStart != template_content.cend()) {
        std::string remainingText(searchStart, template_content.cend());
        if (!remainingText.empty()) {
            nodeStack.top()->children.push_back(
                TemplateNode::createText(remainingText));
        }
    }
    
    if (nodeStack.size() != 1) {
        throw TemplateParseError("Unclosed tags in template");
    }
    
    return root;
}

std::string TemplateParser::generateCode(std::shared_ptr<TemplateNode> ast) {
    return generateNodeCode(ast);
}

std::string TemplateParser::generateNodeCode(std::shared_ptr<TemplateNode> node) {
    std::stringstream ss;
    
    switch (node->type) {
        case TemplateNode::Type::ELEMENT: {
            ss << "h(\"" << node->tag << "\", ";
            
            // Génère les props (attributs + directives)
            ss << "{\n";
            
            // Attributs normaux
            ss << generateAttributesCode(node->attributes);
            
            // Directives
            if (!node->directives.empty()) {
                if (!node->attributes.empty()) ss << ",\n";
                ss << generateDirectivesCode(node->directives);
            }
            
            ss << "}";
            
            // Génère les enfants
            if (!node->children.empty()) {
                ss << ", {\n";
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (i > 0) ss << ",\n";
                    ss << generateNodeCode(node->children[i]);
                }
                ss << "}";
            }
            
            ss << ")";
            break;
        }
        
        case TemplateNode::Type::TEXT:
            ss << "createTextVNode(\"" << escapeString(node->content) << "\")";
            break;
            
        case TemplateNode::Type::EXPRESSION:
            ss << "createTextVNode(toString(" << node->content << "))";
            break;
    }
    
    return ss.str();
}

std::string TemplateParser::generateAttributesCode(
    const std::unordered_map<std::string, Expression>& attrs) {
    std::stringstream ss;
    bool first = true;
    
    for (const auto& [name, expr] : attrs) {
        if (!first) ss << ",\n";
        first = false;
        
        ss << "\"" << name << "\": ";
        if (expr.type == ExpressionType::TEXT) {
            ss << "\"" << escapeString(expr.content) << "\"";
        } else {
            ss << generateExpressionCode(expr);
        }
    }
    
    return ss.str();
}

std::string TemplateParser::generateDirectivesCode(
    const std::unordered_map<std::string, Expression>& dirs) {
    std::stringstream ss;
    bool first = true;
    
    for (const auto& [name, expr] : dirs) {
        if (!first) ss << ",\n";
        first = false;
        
        if (expr.type == ExpressionType::DIRECTIVE) {
            ss << "\"c-" << name << "\": ";
        } else if (expr.type == ExpressionType::EVENT) {
            ss << "\"@" << expr.arg << "\": ";
        } else if (expr.type == ExpressionType::BINDING) {
            ss << "\":" << expr.arg << "\": ";
        }
        
        ss << generateExpressionCode(expr);
    }
    
    return ss.str();
}

std::string TemplateParser::generateExpressionCode(const Expression& expr) {
    switch (expr.type) {
        case ExpressionType::INTERPOLATION:
        case ExpressionType::BINDING:
            return expr.content;
            
        case ExpressionType::DIRECTIVE:
            return "DirectiveBinding{\"" + expr.content + "\", \"" + 
                   expr.arg + "\", \"" + expr.modifiers + "\"}";
            
        case ExpressionType::EVENT:
            return "EventBinding{\"" + expr.content + "\", \"" + 
                   expr.modifiers + "\"}";
            
        default:
            return "\"" + escapeString(expr.content) + "\"";
    }
}

} // namespace cppvue::compiler
