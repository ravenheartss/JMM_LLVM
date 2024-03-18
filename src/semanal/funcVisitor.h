#ifndef JMM_FUNCVISITOR_H
#define JMM_FUNCVISITOR_H

#include "common/ast.h"
#include "common/symtab.h"
#include "semanal/analyzer.h"

class FunctionVisitor : public Visitor {
 private:
  explicit FunctionVisitor(std::shared_ptr<SymbolTable>& symtab,
                           std::shared_ptr<Logger>& logger)
      : m_symtab(symtab), m_logger(logger) {}

  std::shared_ptr<SymbolTable> m_symtab;
  std::shared_ptr<Logger> m_logger;
  friend class SemanticAnalyzer;
};

#endif  // !JMM_FUNCVISITOR_H
