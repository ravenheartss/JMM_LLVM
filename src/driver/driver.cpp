#include "driver.h"
#include <memory>
#include <utility>
#include "common/ast.h"
#include "common/ast_printer.h"
#include "common/errwarn.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semanal/analyzer.h"

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
  ASTPrinter ast_printer = ASTPrinter();
#ifdef PARSER_DEBUG
  m_ast->accept(&ast_printer);
#endif
  SemanticAnalyzer analyzer = SemanticAnalyzer(m_logger);
  analyzer.analyze(m_ast);
#ifdef SEMANAL_DEBUG
  m_ast->accept(&ast_printer);
#endif
  return err;
}
