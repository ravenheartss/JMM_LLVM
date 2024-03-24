#ifndef JMM_GLOBAL_VISITOR_H
#define JMM_GLOBAL_VISITOR_H

#include "common/ast.h"
#include "common/errwarn.h"
#include "common/symtab.h"

class GlobalsVisitor : public Visitor {
friend class SemanticAnalyzer;

 public:
  void visit(GVarDecl * node) override;
  void visit(FuncDecl * node) override;
  void visit(MFuncDecl * node) override;
  void visit(ASTNode * node) override;

 private:
  explicit GlobalsVisitor(std::shared_ptr<SymbolTable>& symtab,
                          std::shared_ptr<Logger>& logger)
      : m_symtab(symtab), m_logger(logger) {}

  std::shared_ptr<SymbolTable> m_symtab;
  std::shared_ptr<Logger> m_logger;
};

#endif  // !JMM_GLOBALVISITOR_H
