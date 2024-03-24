#include "global_visitor.h"
#include <memory>
#include "common/ast.h"
#include "common/symtab.h"

// Adds functions and symbols to the global scope in the symbol table

void GlobalsVisitor::visit(GVarDecl * node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res) {
    if (res->symbol_type == SymType::VAR) {
      m_logger->error("Redefinition of global variable '", name, "' at line ",
                      node->line, ". Previously declared at line ", res->line);
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res->line);
  }

  // If there isn't any bug in building the AST, this should not throw an error.
  node->symbol = std::make_shared<Symbol>(node->type, node->line);
  m_symtab->insert(Scope::GLOBAL, name, node->symbol);
}

void GlobalsVisitor::visit(FuncDecl * node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res) {
    if (res->symbol_type == SymType::FUNC) {
      m_logger->error("Redefinition of global variable '", name, "' at line ",
                      node->line, ". Previously declared at line ", res->line);
      return;
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res->line);
  }

  // Get formal types
  std::vector<VType> param_types;
  param_types.reserve(node->params->params.size());
  for (auto const& param : node->params->params) {
    param_types.emplace_back(param->type);
  }

  node->symbol = std::make_shared<Symbol>(
      node->return_type, std::move(param_types), node->line);
  m_symtab->insert(Scope::GLOBAL, name, node->symbol);
}

void GlobalsVisitor::visit(MFuncDecl * node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res) {
    if (res->symbol_type == SymType::FUNC) {
      m_logger->error("Redefinition of function'", name, "' at line ",
                      node->line, ". Previously declared at line ",
                      res->line);
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res->line);
  }

  node->symbol = std::make_shared<Symbol>(node->line);
  m_symtab->insert(Scope::GLOBAL, name, node->symbol);
}

void GlobalsVisitor::visit(ASTNode * node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}
