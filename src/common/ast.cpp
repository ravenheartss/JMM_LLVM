#include "ast.h"
#include <iostream>

std::ostream& operator<<(std::ostream& out, VType value_type) {
  switch (value_type) {
    case VType::Int:
      return out << "Int";
    case VType::Str:
      return out << "Str";
    case VType::Bool:
      return out << "Bool";
    case VType::Void:
      return out << "Void";
    default:
      return out;
  }
}

std::ostream& operator<<(std::ostream& out, Op oper) {
  switch (oper) {
    case Op::BAND:
      return out << "&";
    case Op::BOR:
      return out << "|";
    case Op::LAND:
      return out << "&&";
    case Op::LOR:
      return out << "||";
    case Op::XOR:
      return out << "^";
    case Op::ADD:
      return out << "+";
    case Op::POSTINC:
      return out << "Post ++";
    case Op::POSTDEC:
      return out << "Post --";
    case Op::PREINC:
      return out << "Pre ++";
    case Op::PREDEC:
      return out << "Pre --";
    case Op::SUB:
      return out << "-";
    case Op::MULT:
      return out << "*";
    case Op::DIV:
      return out << "/";
    case Op::MOD:
      return out << "%";
    case Op::NOT:
      return out << "!";
    case Op::EQ:
      return out << "==";
    case Op::NE:
      return out << "!=";
    case Op::GT:
      return out << ">";
    case Op::LT:
      return out << "<";
    case Op::GE:
      return out << ">=";
    case Op::LE:
      return out << "<=";
    case Op::LSHIFT:
      return out << "<<";
    case Op::RSHIFT:
      return out << ">>";
    default:
      return out;
  }
}

ASTNode::~ASTNode() { children.clear(); }

ASTNode& ASTNode::operator=(ASTNode&& node) noexcept {
  line = node.line;
  children = std::move(children);
  return *this;
}

ASTNode::ASTNode(ASTNode const&& node) noexcept {
  line = node.line;
  children = std::move(children);
}

