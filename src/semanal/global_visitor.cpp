#include "global_visitor.h"
#include "common/ast.h"
#include "common/symtab.h"

// Adds functions and symbols to the global scope in the symbol table

void GlobalsVisitor::visit(GVarDecl const* node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res.has_value()) {
    if (res.value().symbol_type == SymType::VAR) {
      m_logger->error("Redefinition of global variable '", name, "' at line ",
                      node->line, ". Previously declared at line ",
                      res.value().line);
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res.value().line);
  }

  // If there isn't any bug in building the AST, this should not throw an error.
  m_symtab->insert(Scope::GLOBAL, name, Symbol(node->type, node->line));
}

void GlobalsVisitor::visit(FuncDecl const* node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res.has_value()) {
    if (res.value().symbol_type == SymType::FUNC) {
      m_logger->error("Redefinition of global variable '", name, "' at line ",
                      node->line, ". Previously declared at line ",
                      res.value().line);
      return;
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res.value().line);
  }

  // Get formal types
  std::vector<VType> param_types;
  param_types.reserve(node->params->params.size());
  for (auto const& param : node->params->params) {
    param_types.emplace_back(param->type);
  }

  Symbol func_sig =
      Symbol(node->return_type, std::move(param_types), false, node->line);
  m_symtab->insert(Scope::GLOBAL, name, func_sig);
}

void GlobalsVisitor::visit(MFuncDecl const* node) {
  auto const& name = node->id;
  auto res = m_symtab->query(Scope::GLOBAL, name);
  if (res.has_value()) {
    if (res.value().symbol_type == SymType::FUNC) {
      m_logger->error("Redefinition of function'", name, "' at line ",
                      node->line, ". Previously declared at line ",
                      res.value().line);
    }
    m_logger->error("Redefinition of global variable '", name,
                    "' of different kind at line ", node->line,
                    ". Previously declared at line ", res.value().line);
  }

  Symbol func_sig = Symbol(VType::Void, {}, true, node->line);
  m_symtab->insert(Scope::GLOBAL, name, func_sig);
}

void GlobalsVisitor::visit(ASTNode const* node) {
  for (auto const& child : node->children) {
    child->accept(this);
  }
}
