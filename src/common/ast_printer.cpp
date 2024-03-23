#include "ast.h"
#include "ast_printer.h"
#include <iostream>

// ASTPrinter
void ASTPrinter::visit(ASTNode* node) {
  // This is the root program node
  std::cerr << "Program\n";
  ++m_indent;
  for (auto const& child : node->children) {
    child->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(IfStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "IfStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->if_body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(IfElseStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "IfElseStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->if_body->accept(this);
  node->else_body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(WhileStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "WhileStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->condition->accept(this);
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(GotoStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "GotoStmt; Line: " << node->line << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(ReturnStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ReturnStmt; Line: " << node->line << '\n';
  ++m_indent;
  if (node->expr) {
    node->expr->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(BreakStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BreakStmt; Line: " << node->line << '\n';
}

void ASTPrinter::visit(BlockStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Block;" << '\n';
  ++m_indent;
  for (auto const& stmt : node->children) {
    stmt->accept(this);
  }
  --m_indent;
}

void ASTPrinter::visit(ExprStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ExprStmt;" << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(NullStmt* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "NullStmt; Line: " << node->line << '\n';
}

void ASTPrinter::visit(IdExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ID; Value: \"" << node->value << "\"; Line: " << node->line
            << '\n';
}

void ASTPrinter::visit(LitExpr* node) {
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

void ASTPrinter::visit(UnaryExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "UnaryExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->expr->accept(this);
  --m_indent;
}

void ASTPrinter::visit(BinaryExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BinaryExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(BitwiseExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "BitwiseExpr; Op: " << node->op << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(AssignExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "AssignExpr (=); Line: " << node->line << '\n';
  ++m_indent;
  node->lhs->accept(this);
  node->rhs->accept(this);
  --m_indent;
}

void ASTPrinter::visit(FuncCallExpr* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "FuncCall; Value: \"" << node->id << "\"; Line: " << node->line
            << '\n';
  ++m_indent;
  node->args->accept(this);
  --m_indent;
}

void ASTPrinter::visit(FuncDecl* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "FuncDecl; Value: \"" << node->id
            << "\"; Type: " << node->return_type << "; Line: " << node->line
            << '\n';
  ++m_indent;
  node->params->accept(this);
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(MFuncDecl* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "MainFuncDecl; Value: \"" << node->id
            << "\"; Line: " << node->line << '\n';
  ++m_indent;
  node->body->accept(this);
  --m_indent;
}

void ASTPrinter::visit(VarDecl* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "VarDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(GVarDecl* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "GVarDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(ParamDecl* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "ParamDecl; Value: \"" << node->id << "\" Type: " << node->type
            << "; Line: " << node->line << '\n';
}

void ASTPrinter::visit(Params* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Parameters;\n";
  ++m_indent;
  for (auto const& param : node->params) {
    param->accept(this);
  }
  --m_indent;
}

// This is just a dummy node for now
void ASTPrinter::visit(ActualExpr* node) { node->expr->accept(this); }

void ASTPrinter::visit(Actuals* node) {
  std::cerr << std::string(m_indent, ' ');
  std::cerr << "Actuals;\n";
  ++m_indent;
  for (auto const& actual : node->actuals) {
    actual->accept(this);
  }
  --m_indent;
}
