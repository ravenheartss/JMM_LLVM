#include "func_visitor.h"
#include "common/symtab.h"

void FunctionVisitor::visit(ASTNode* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }

  if (!m_visited_main) {
    m_logger->error("No main function found");
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
  m_inside_while.emplace();
  node->condition->accept(this);
  node->body->accept(this);
  m_inside_while.pop();
}

void FunctionVisitor::visit(ReturnStmt* node) {
  if (node->expr) {
    node->expr->accept(this);
  }

  if (!m_current_function) {
    m_logger->error("Return statement not inside function");
  }
}

void FunctionVisitor::visit(BreakStmt* node) {
  if (m_inside_while.empty()) {
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
  }
  if (res.value().second->symbol_type == SymType::FUNC) {
    m_logger->error("Cannot use a function as a variable at line ", node->line);
  }
  node->symbol = res.value().second;
}

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
  node->args->accept(this);
  auto const& name = node->id;
  auto sym = m_symtab->query(name);
  if (!sym) {
    m_logger->error("Call to undefined function '", name, "' at line ",
                    node->line);
  }
  if (sym.value().second->symbol_type == SymType::VAR) {
    m_logger->error("Cannot invoke a variable as a function at line ",
                    node->line);
  }
  if (sym.value().second->isMain) {
    m_logger->error("Cannot invoke main function at line ", node->line);
  }
  node->symbol = sym.value().second;
}

void FunctionVisitor::visit(FuncDecl* node) {
  // enter function
  // This helps in examining stuff like return statements
  m_current_function = node->symbol;

  node->params->accept(this);
  node->body->accept(this);

  m_symtab->exit_func();
  m_current_function.reset();
}

void FunctionVisitor::visit(MFuncDecl* node) {
  m_current_function = node->symbol;
  if (m_visited_main) {
    m_logger->error("Redfinition of main function at line ", node->line);
  }

  node->body->accept(this);
  m_visited_main = true;
  m_current_function.reset();
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
