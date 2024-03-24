#include "type_visitor.h"
#include "common/ast.h"
#include "common/globals.h"
#include "common/symtab.h"

void TypeVisitor::visit(ASTNode* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void TypeVisitor::visit(IfStmt* node) {
  node->condition->accept(this);
  node->if_body->accept(this);

  if (node->condition->a_type.value() != VType::Bool) {
    m_logger->error("If condition at line ", node->line,
                    " needs to evaluate to a boolean value");
  }
}

void TypeVisitor::visit(IfElseStmt* node) {
  node->condition->accept(this);
  node->if_body->accept(this);
  node->else_body->accept(this);

  if (node->condition->a_type.value() != VType::Bool) {
    m_logger->error("If condition at line ", node->line,
                    " needs to evaluate to a boolean value");
  }
}

void TypeVisitor::visit(WhileStmt* node) {
  node->condition->accept(this);
  node->body->accept(this);

  if (node->condition->a_type.value() != VType::Bool) {
    m_logger->error("While condition at line ", node->line,
                    " needs to evaluate to a boolean value");
  }
}

void TypeVisitor::visit(ReturnStmt* node) {
  m_return_visited = true;
  if (node->expr) {
    node->expr->accept(this);
  }

  if (!m_current_function) {
    m_logger->error("Return statement not inside function");
  }

  if (m_current_function->v_type == VType::Void && node->expr) {
    m_logger->error("Cannot return a value from a void function at line ",
                    node->line);
  }

  if (node->expr && m_current_function->v_type != node->expr->a_type) {
    m_logger->error(
        "Function return type and returned value type differ at line ",
        node->line);
  }

  if (node->expr) {
    node->a_type = node->expr->a_type;
  } else {
    node->a_type = VType::Void;
  }
}

void TypeVisitor::visit(BlockStmt* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}

void TypeVisitor::visit(ExprStmt* node) {
  node->expr->accept(this);

  node->a_type = node->expr->a_type;
}

// Undeclared vars are caught in Func visitor
void TypeVisitor::visit(IdExpr* node) { node->a_type = node->symbol->v_type; }

void TypeVisitor::visit(UnaryExpr* node) {
  node->expr->accept(this);

  switch (node->op) {
    case Op::POSTINC:
    case Op::POSTDEC:
    case Op::PREINC:
    case Op::PREDEC: {
      if (!node->expr->symbol) {
        m_logger->error("Invalid use of increment operator at line ",
                        node->line,
                        ". Can only increment l-values "
                        "(variables) once.");
      }
      if (node->expr->symbol->symbol_type != SymType::VAR) {
        m_logger->error("Cannot increment a function call at line ",
                        node->line);
      }
      if (node->expr->symbol->v_type != VType::Int) {
        m_logger->error("Can only increment an int at line ", node->line);
      }
      break;
    }
    case Op::NOT:
      if (node->expr->a_type != VType::Bool) {
        m_logger->error("Cannot negate an integer. Expected a boolean at line ",
                        node->line);
      }
      break;
    default:
      if (node->expr->a_type != VType::Int) {
        m_logger->error("Can only perform operation ", node->op,
                        " on an int at line ", node->line);
      }
      break;
  }
  node->a_type = node->expr->a_type;
}

void TypeVisitor::visit(BinaryExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);

  auto check_same_type = [&]() {
    if (node->lhs->a_type != node->rhs->a_type) {
      m_logger->error("Types for operation ", node->op, " differ at line ",
                      node->line);
    }
  };

  switch (node->op) {
    case Op::LAND:
    case Op::LOR:
      check_same_type();
      if (node->lhs->a_type != VType::Bool) {
        m_logger->error("Can only perform operation ", node->op,
                        " on boolean at line ", node->line);
      }
      node->a_type = VType::Bool;
      break;
    case Op::BAND:
    case Op::BOR:
    case Op::XOR:
    case Op::ADD:
    case Op::SUB:
    case Op::MULT:
    case Op::DIV:
    case Op::MOD:
    case Op::LSHIFT:
    case Op::RSHIFT:
      check_same_type();
      if (node->lhs->a_type != VType::Int) {
        m_logger->error("Can only perform operation ", node->op,
                        " on integers at line ", node->line);
      }
      node->a_type = node->lhs->a_type;
      break;
    case Op::NE:
    case Op::EQ:
      check_same_type();
      if (node->lhs->a_type == VType::Str || node->lhs->a_type == VType::Void) {
        m_logger->error("Cannot perform operation ", node->op, " on ",
                        node->lhs->a_type.value(), "at line ", node->line);
      }
      node->a_type = VType::Bool;
      break;
    case Op::GT:
    case Op::LT:
    case Op::GE:
    case Op::LE:
      check_same_type();
      if (node->lhs->a_type != VType::Int) {
        m_logger->error("Can only perform operation ", node->op,
                        " on integers at line ", node->line);
      }
      node->a_type = VType::Bool;
      break;
    default:
      m_logger->error("Unsupported operation - Not a binary op!!!");
      break;
  }
}

void TypeVisitor::visit(BitwiseExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);

  node->a_type = node->lhs->a_type;
}

void TypeVisitor::visit(AssignExpr* node) {
  node->lhs->accept(this);
  node->rhs->accept(this);

  if (!node->lhs->symbol) {
    m_logger->error(
        "Left hand side of assignment needs to be an identifier (variable) at "
        "line ",
        node->line);
  }

  if (node->lhs->symbol->symbol_type != SymType::VAR) {
    m_logger->error(
        "Left hand side of assignment needs to be an variable at "
        "line ",
        node->line);
  }

  if (node->lhs->a_type != node->rhs->a_type) {
    m_logger->error("Cannot assign a value of type ", node->rhs->a_type.value(),
                    " to variable of type ", node->lhs->a_type.value(),
                    " at line ", node->line);
  }

  node->a_type = node->lhs->a_type;
}

void TypeVisitor::visit(FuncCallExpr* node) {
  node->args->accept(this);

  auto sym = node->symbol;
  // Check arg types
  if (sym->param_types.has_value()) {
    if (sym->param_types.value().size() != node->args->actuals.size()) {
      m_logger->error("Expected ", sym->param_types.value().size(),
                      " arguments to function '", node->id, "' at line ",
                      node->line, " but received ", node->args->actuals.size());
    }

    auto it1 = sym->param_types.value().begin();
    auto it2 = node->args->actuals.begin();

    // Guaranteed to be of the same length thanks to the check above
    while (it1 != sym->param_types.value().end() &&
           it2 != node->args->actuals.end()) {
      if (*it1 != (*it2)->a_type.value()) {
        break;
      }
      ++it1;
      ++it2;
    }

    if (it2 != node->args->actuals.end()) {
      m_logger->error("Incorrect argument types while invoking function '",
                      node->id, "' at line ", node->line);
    }
  }
  node->a_type = sym->v_type;
}

void TypeVisitor::visit(FuncDecl* node) {
  // enter function
  // This helps in examining stuff like return statements
  m_current_function = node->symbol;

  node->params->accept(this);
  node->body->accept(this);

  node->a_type = node->symbol->v_type;
  m_current_function.reset();
}

void TypeVisitor::visit(MFuncDecl* node) {
  m_current_function = node->symbol;
  node->body->accept(this);
  m_current_function.reset();
}

void TypeVisitor::visit(ActualExpr* node) {
  node->expr->accept(this);
  node->a_type = node->expr->a_type;
}

void TypeVisitor::visit(Actuals* node) {
  for (auto const& actual : node->actuals) {
    actual->accept(this);
  }
}

void TypeVisitor::visit(LitExpr* node) { node->a_type = node->type; }

void TypeVisitor::visit(VarDecl* node) { node->a_type = node->type; }

void TypeVisitor::visit(GVarDecl* node) { node->a_type = node->type; }

void TypeVisitor::visit(ParamDecl* node) { node->a_type = node->type; }

void TypeVisitor::visit(Params* node) {
  for (auto const& param : node->params) {
    param->accept(this);
  }
}
