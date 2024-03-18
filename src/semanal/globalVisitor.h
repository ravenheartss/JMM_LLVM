#ifndef JMM_GLOBALVISITOR_H
#define JMM_GLOBALVISITOR_H

#include "common/ast.h"
#include "common/errwarn.h"
#include "common/symtab.h"

class GlobalsVisitor : public Visitor {
  friend class SemanticAnalyzer;

 public:
  void visit(GVarDecl const* node) override;
  void visit(FuncDecl const* node) override;
  void visit(MFuncDecl const* node) override {}

 private:
  explicit GlobalsVisitor(std::shared_ptr<SymbolTable>& symtab,
                          std::shared_ptr<Logger>& logger)
      : m_symtab(symtab), m_logger(logger) {}

  std::shared_ptr<SymbolTable> m_symtab;
  std::shared_ptr<Logger> m_logger;
};

#endif  // !JMM_GLOBALVISITOR_H
