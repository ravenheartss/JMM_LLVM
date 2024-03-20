#include "ast.h"
#include "ast_printer.h"
#include <iostream>

// ASTPrinter
void ASTPrinter::visit(ASTNode const* node) {
  // This is the root program node
  std::cerr << "Program\n";
  ++m_indent;
  for (auto const& child : node->children) {
    child->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(IfStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "IfStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->if_body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(IfElseStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "IfElseStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->if_body->accept(this);
  node->else_body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(WhileStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "WhileStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(GotoStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "GotoStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(ReturnStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ReturnStmt; Line: " << node->line << '\n';
  ++m_indent;
  if (node->expr) {
    node->expr->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(BreakStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BreakStmt; Line: " << node->line << '\n';
}

void ASTPrinter::visit(BlockStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Block;" << '\n';
  ++m_indent;
  for (auto const& stmt : node->children) {
    stmt->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(ExprStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ExprStmt;" << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(NullStmt const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "NullStmt; Line: " << node->line << '\n';
}

void ASTPrinter::visit(IdExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ID; Value: \"" << node->value << "\"; Line: " << node->line
            << '\n';
}

void ASTPrinter::visit(LitExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Literal; Value: '";
  switch (node->type) {
    case VType::Int:
      std::cerr << node->ival.value();
      break;
    case VType::Str:
      std::cerr << node->sval.value();
      break;
    case VType::Bool:
      std::cerr << node->bval.value();
      break;
    case VType::Void:
      [[fallthrough]];
    default:
      break;
  }
  std::cerr << "'; Type: " << node->type << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(UnaryExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "UnaryExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(BinaryExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BinaryExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(BitwiseExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BitwiseExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(AssignExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "AssignExpr (=); Line: " << node->line << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(FuncCallExpr const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "FuncCall; Value: \"" << node->id << "\"; Line: " << node->line
            << '\n';
  ++m_indent;
  node->args->accept(this);
  --m_indent;
}

void ASTPrinter::visit(FuncDecl const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "FuncDecl; Value: \"" << node->id
            << "\"; Type: " << node->return_type << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->params->accept(this);
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(MFuncDecl const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "MainFuncDecl; Value: \"" << node->id
            << "\"; Line: " << node->line << '\n';
  ++m_indent;
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(VarDecl const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "VarDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(GVarDecl const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "GVarDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(ParamDecl const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ParamDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(Params const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Parameters;\n";
  ++m_indent;
  for (auto const& param : node->params) {
    param->accept(this);
  }
  --m_indent;
}

// This is just a dummy node for now
void ASTPrinter::visit(ActualExpr const* node) { node->expr->accept(this); }

void ASTPrinter::visit(Actuals const* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Actuals;\n";
  ++m_indent;
  for (auto const& actual : node->actuals) {
    actual->accept(this);
  }
  --m_indent;
}
