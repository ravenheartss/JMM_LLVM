#include "func_visitor.h"
#include "common/symtab.h"

void FunctionVisitor::visit(ASTNode* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void FunctionVisitor::visit(IfStmt* node) {
  node->condition->accept(this);
  node->if_body->accept(this);
}

void FunctionVisitor::visit(IfElseStmt* node) {
  node->condition->accept(this);
  node->if_body->accept(this);
  node->else_body->accept(this);
}

void FunctionVisitor::visit(WhileStmt* node) {
  m_inside_while = true;
  node->condition->accept(this);
  node->body->accept(this);
  m_inside_while = false;
}

void FunctionVisitor::visit(GotoStmt* node) { node->expr->accept(this); }

void FunctionVisitor::visit(ReturnStmt* node) {
  if (node->expr) {
    node->expr->accept(this);
  }
}

void FunctionVisitor::visit(BreakStmt* node) {
  if (!m_inside_while) {
    m_logger->error("Break statement at line ", node->line,
                      " outside while body");
  }
}

void FunctionVisitor::visit(BlockStmt* node) {
  m_symtab->enter_block();
  for (auto const& child : node->children) {
    child->accept(this);
  }
  m_symtab->exit_block();
}

void FunctionVisitor::visit(ExprStmt* node) { node->expr->accept(this); }

// void FunctionVisitor::visit(NullStmt* node) {}

void FunctionVisitor::visit(IdExpr* node) {
  // Check if defined
  auto const& name = node->value;
  auto res = m_symtab->query(name);
  if (!res.has_value()) {
    m_logger->error("Use of undefined variable '", name, "' at line ",
                      node->line);
  } else {
    node->symbol = res.value().second;
  }
}

// void FunctionVisitor::visit(LitExpr* node) {}

void FunctionVisitor::visit(UnaryExpr* node) { node->expr->accept(this); }

void FunctionVisitor::visit(BinaryExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);
}

void FunctionVisitor::visit(BitwiseExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);
}

void FunctionVisitor::visit(AssignExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);
}

void FunctionVisitor::visit(FuncCallExpr* node) {
  auto const& name = node->id;
  auto sym = m_symtab->query(Scope::GLOBAL, name);
  if (!sym) {
    m_logger->error("Call to undefined function '", name, "' at line ",
                    node->line);
  }
  node->symbol = sym;
}

void FunctionVisitor::visit(FuncDecl* node) {
  // enter function
  node->params->accept(this);
  node->body->accept(this);

  m_symtab->exit_func();
}

void FunctionVisitor::visit(MFuncDecl* node) {
  if (m_visited_main) {
    m_logger->error("Redfinition of main function at line ", node->line);
  }

  m_visited_main = true;
}

void FunctionVisitor::visit(VarDecl* node) {  // can only be in block
  auto const& name = node->id;
  node->symbol = std::make_shared<Symbol>(node->type, node->line);
  auto const res = m_symtab->insert(Scope::BLOCK, name, node->symbol);
  auto const& prev = m_symtab->query(Scope::BLOCK, name);
  if (!res) {
    m_logger->error("Redefinition of variable '", name, "' at line ",
                    node->line, ". Previously declared at line ", prev->line);
  }
}

// One can only reach here using params's visit which in turn can only be
// reached by FuncDecl's visit. So it's sure that we are currently processing
// function
void FunctionVisitor::visit(ParamDecl* node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::FUNC, name);
  if (res) {
    m_logger->error("Redefinition of parameter'", name, "' of type ",
                    node->type, " at line ", node->line,
                    ". Previously declared as type ", res->v_type);
  }

  node->symbol = std::make_shared<Symbol>(node->type, node->line);
  m_symtab->insert(Scope::FUNC, name, node->symbol);
}

// One can only reach here using FuncDecl's visit
void FunctionVisitor::visit(Params* node) {
  for (auto const& param : node->params) {
    param->accept(this);
  }
}

void FunctionVisitor::visit(ActualExpr* node) { node->expr->accept(this); }

void FunctionVisitor::visit(Actuals* node) {
  for (auto const& actual : node->actuals) {
    actual->accept(this);
  }
}
