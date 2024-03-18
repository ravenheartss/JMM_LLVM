#include "globalVisitor.h"
#include "common/ast.h"
#include "common/symtab.h"

void GlobalsVisitor::visit(GVarDecl const* node) {
  auto const& val = std::get<std::string>(node->value);
  auto res = m_symtab->query(Scope::GLOBAL, val);
  if (res.has_value()) {
    m_logger->warning("Ignoring redefinition of global variable ", val,
                      " at line ", node->line);
    return;
  }

  // If there isn't any bug in building the AST, this should not throw an error.
  m_symtab->insert(Scope::GLOBAL, val,
                   VarSig(node->val_type.value(), node->line));
}

void GlobalsVisitor::visit(FuncDecl const* node) {
  auto const& val = std::get<std::string>(node->value);
  auto res = m_symtab->query(Scope::GLOBAL, val);
  if (res.has_value()) {
    m_logger->warning("Redefinition of function ", val, " at line ",
                      node->line);
    return;
  }

  std::vector<VType> param_types;

  FunctionSig func_sig =
      FunctionSig(node->val_type.value(), {}, false, node->line);
  // If there isn't any bug in building the AST, this should not throw an error.
  // m_symtab->insert(Scope::GLOBAL, val, );
}
