#ifndef JMM_TYPEVISITOR_H
#define JMM_TYPEVISITOR_H

#include "common/ast.h"
#include "common/errwarn.h"
#include "common/symtab.h"

class TypeAnalVisitor : public Visitor {  // Last thing to run
  friend class SemanticAnalyzer;

 private:
  explicit TypeAnalVisitor(std::shared_ptr<SymbolTable>& symtab,
                           std::shared_ptr<Logger>& logger)
      : m_symtab(symtab), m_logger(logger) {}

  std::shared_ptr<SymbolTable> m_symtab;
  std::shared_ptr<Logger> m_logger;
};

#endif  // !JMM_TYPEVISITOR_H
