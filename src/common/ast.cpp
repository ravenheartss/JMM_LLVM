#include "ast.h"

ASTNode::~ASTNode() { children.clear(); }

ASTNode& ASTNode::operator=(ASTNode&& node) noexcept {
  line = node.line;
  children = std::move(node.children);
  return *this;
}

ASTNode::ASTNode(ASTNode && node) noexcept {
  line = node.line;
  children = std::move(node.children);
}
