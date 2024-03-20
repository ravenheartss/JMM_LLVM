#ifndef JMM_PARSER_H
#define JMM_PARSER_H

#include <memory>
#include <optional>
#include "common/ast.h"
#include "common/errwarn.h"
#include "common/token.h"
#include "lexer/lexer.h"

class Parser {
  using nodePtr = std::unique_ptr<ASTNode>;
  template <class T>
  using nodeT = std::unique_ptr<T>;

 public:
  Parser(std::shared_ptr<Lexer>& lexer, std::shared_ptr<Logger>& logger)
      : m_lexer(lexer), m_logger(logger) {}

  ~Parser();

  bool parse();

  std::unique_ptr<ASTNode> getAST() { return std::move(m_ast); }

 private:
  std::shared_ptr<Lexer> m_lexer;
  std::unique_ptr<ASTNode> m_ast;
  std::shared_ptr<Logger> m_logger;

  bool match(Token expected);

  [[noreturn]] void error(std::string expected);

  void start();
  std::optional<VType> type();
  auto identifier();
  nodePtr block();
  nodePtr blockstatements();
  nodePtr statement();
  nodePtr expression();
  nodePtr variabledeclaration();
  nodePtr simpleStmt();
  nodePtr returnStmt();
  nodePtr ifStmt();
  nodePtr gotoStmt();
  nodePtr exprStmt();
  nodePtr whileStmt();
  nodePtr literal();
  nodePtr primary();
  nodePtr postfixexpression();
  std::vector<nodeT<ActualExpr>> argumentlist();
  nodeT<Actuals> functioninvocation();
  nodePtr unaryexpression();
  nodePtr multiplicativeexpression();
  nodePtr conditionalandexpression();
  nodePtr conditionalorexpression();
  nodePtr additiveexpression();
  nodePtr shiftexpression();
  nodePtr relationalexpression();
  nodePtr equalityexpression();
  nodePtr bitwiseandexpression();
  nodePtr bitwiseorexpression();
  nodePtr assignmentexpression();
  nodePtr xorexpression();
  nodePtr assignment();
  std::vector<nodeT<ParamDecl>> formalparameterlist();
  nodeT<Params> functiondeclarator();
  nodePtr mainfunctiondeclaration();
  nodePtr globaldeclaration();
};

#endif  // !JMM_PARSER_H
