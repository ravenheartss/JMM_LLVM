#ifndef JMM_ANALYZER_H
#define JMM_ANALYZER_H

#include <memory>
#include "common/ast.h"
#include "common/errwarn.h"
#include "common/symtab.h"

class SemanticAnalyzer {
  using nodePtr = std::unique_ptr<ASTNode>;

 public:
  explicit SemanticAnalyzer(std::shared_ptr<Logger>& logger);

  bool analyze(nodePtr& node);

 private:
  void getGlobals(nodePtr& node);

  std::shared_ptr<Logger> m_logger;
  // why shared? well, you need to give it to the visitors!
  // the other way is to pass the symbol table each time. Seems like too much
  // work...
  std::shared_ptr<SymbolTable> m_symtab;
};

#endif  // JMM_ANALYZER_H
