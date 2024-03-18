#include "driver.h"
#include <memory>
#include <utility>
#include "common/ast.h"
#include "common/errwarn.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

Driver::Driver(std::string file) : m_filename(std::move(file)) {
  m_logger = std::make_shared<Logger>();
  m_lexer = std::make_shared<Lexer>(m_filename, m_logger);
  m_parser = std::make_unique<Parser>(m_lexer, m_logger);
}

Driver::~Driver() {
  m_parser.reset();
  m_lexer.reset();
  m_logger.reset();
  m_filename.clear();
  m_ast.reset();
}

bool Driver::compile() {
  bool err = m_parser->parse();
  m_ast = m_parser->getAST();
#ifdef PARSER_DEBUG
  m_ast->print(0);
#endif

  return err;
}
