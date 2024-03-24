#ifndef JMM_DRIVER_H
#define JMM_DRIVER_H

#include <memory>
#include <string>

#include "common/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

class Driver {
 public:
  explicit Driver(std::string file);
  ~Driver();

  bool compile();

 private:
  std::shared_ptr<Lexer> m_lexer;
  std::unique_ptr<Parser> m_parser;
  std::shared_ptr<Logger> m_logger;

  std::unique_ptr<ASTNode> m_ast;

  std::string m_filename;
};
#endif  // !JMM_DRIVER_H
