#include "component.hpp"

namespace cppvue {

std::shared_ptr<VNode> VNode::create(
    const std::string& tag,
    const std::unordered_map<std::string, std::string>& props,
    const std::vector<std::shared_ptr<VNode>>& children,
    const std::string& text
) {
    auto node = std::make_shared<VNode>();
    node->tag = tag;
    node->props = props;
    node->children = children;
    node->textContent = text;
    return node;
}

std::shared_ptr<VNode> Component::h(
    const std::string& tag,
    const std::unordered_map<std::string, std::string>& props,
    const std::vector<std::shared_ptr<VNode>>& children
) {
    return VNode::create(tag, props, children);
}

std::shared_ptr<VNode> Component::h(
    const std::string& tag,
    const std::vector<std::shared_ptr<VNode>>& children
) {
    return VNode::create(tag, {}, children);
}

std::shared_ptr<VNode> Component::h(
    const std::string& tag,
    const std::string& text
) {
    return VNode::create(tag, {}, {}, text);
}

} // namespace cppvue
