#include "parser.h"
#include <memory>
#include <optional>
#include "common/ast.h"
#include "common/globals.h"
#include "lexer/lexer.h"

template <class T>
using nodeT = std::unique_ptr<T>;

// Helper functions
template <class T, typename... Args>
inline std::unique_ptr<T> makeNode(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

Op tokenToOp(Token tok) {
  switch (tok) {
    case Token::MINUS:
      return Op::SUB;
    case Token::PLUS:
      return Op::ADD;
    case Token::MULT:
      return Op::MULT;
    case Token::DIV:
      return Op::DIV;
    case Token::MOD:
      return Op::MOD;
    case Token::XOR:
      return Op::XOR;
    case Token::GT:
      return Op::GT;
    case Token::LT:
      return Op::LT;
    case Token::GTEQ:
      return Op::GE;
    case Token::LTEQ:
      return Op::LE;
    case Token::LSHIFT:
      return Op::LSHIFT;
    case Token::RSHIFT:
      return Op::RSHIFT;
    case Token::EQ:
      return Op::EQ;
    case Token::NOT:
      return Op::NOT;
    case Token::NEQ:
      return Op::NE;
    case Token::LAND:
      return Op::LAND;
    case Token::LOR:
      return Op::LOR;
    case Token::BAND:
      return Op::BAND;
    case Token::BOR:
      return Op::BOR;
    default:
      return Op::PREINC;  // error
  }
}

Parser::~Parser() {
  m_logger.reset();
  m_lexer.reset();
  m_ast.reset();
}

bool Parser::parse() {
  m_ast = makeNode<ASTNode>();
  start();
  return true;
}

bool Parser::match(Token expected) {
  if (m_lexer->peek() != expected) {
    return false;
  }

  m_lexer->consume();
  return true;
}

[[noreturn]] void Parser::error(std::string expected) {
  m_logger->error(m_lexer->line(), ": Expected ", expected, " Got: '",
                  m_lexer->peek(), "'");
}

// type = BOOL | INT | STR
std::optional<VType> Parser::type() {
  if (match(Token::BOOL)) {
    return VType::Bool;
  }
  if (match(Token::INT)) {
    return VType::Int;
  }
  if (match(Token::STR)) {
    return VType::Str;
  }

  return {};
}

// This needs to return Id so we can retreive the value
auto Parser::identifier() {
  nodeT<IdExpr> id(nullptr);

  if (match(Token::ID)) {
    id = makeNode<IdExpr>(m_lexer->lexeme());
    id->line = m_lexer->line();
  }

  return id;
}

// literal = NUMBER | STRING | TRUE | FALSE .
nodePtr Parser::literal() {
  nodePtr node(nullptr);

  if (match(Token::NUM)) {
    node =
        makeNode<LitExpr>(static_cast<int32_t>(std::stoul(m_lexer->lexeme())));
    node->line = m_lexer->line();
  } else if (match(Token::STRLIT)) {
    node = makeNode<LitExpr>(m_lexer->lexeme());
    node->line = m_lexer->line();
  } else if (match(Token::TRUE)) {
    node = makeNode<LitExpr>(true);
    node->line = m_lexer->line();
  } else if (match(Token::FALSE)) {
    node = makeNode<LitExpr>(false);
    node->line = m_lexer->line();
  }

  return node;
}

// variabledeclaration = type identifier SEMCOL
nodePtr Parser::variabledeclaration() {
  nodePtr node(nullptr);
  std::optional<VType> vtype = type();

  if (!vtype.has_value()) {
    return node;
  }

  auto id = identifier();
  if (!id) {
    error("identifier in a variable declaration after type");
  }

  node = makeNode<VarDecl>(id->value, vtype.value());
  node->line = id->line;

  if (!match(Token::SEMICOL)) {
    error("semicolon after variable declaration");
  }

  return node;
}

// simpleStmt = nullStmt | exprStmt .
// nullStmt = SEMCOL .
nodePtr Parser::simpleStmt() {
  nodePtr node(nullptr);
  if (match(Token::SEMICOL)) {
    node = makeNode<NullStmt>();
    node->line = m_lexer->line();
  } else {
    // can be nullptr
    node = exprStmt();
  }

  return node;
}

// exprStmt = identifier ( assignment | functioninvocation ) SEMCOL .
// exprStmt = expression SEMCOL .
nodePtr Parser::exprStmt() {
  auto expr = expression();

  if (!expr) {
    return nullptr;
  }

  if (!match(Token::SEMICOL)) {
    error("semicolon");
  }

  return makeNode<ExprStmt>(std::move(expr));
}

// returnStmt = RETURN [ expression ] SEMCOL .
nodePtr Parser::returnStmt() {
  if (!match(Token::RETURN)) {
    return nullptr;
  }

  auto ret_line = m_lexer->line();
  auto expr = expression();

  if (!match(Token::SEMICOL)) {
    error("semicolon after return statement");
  }

  auto node = makeNode<ReturnStmt>();
  if (expr) {
    node->expr = std::move(expr);
  }
  node->line = ret_line;

  return node;
}

// ifStmt = IF OPAREN expression CPAREN statement [ ELSE statement ] .
nodePtr Parser::ifStmt() {
  if (!match(Token::IF)) {
    return nullptr;
  }

  auto if_line = m_lexer->line();

  if (!match(Token::OPAREN)) {
    error("'(' after if");
  }

  auto cond = expression();
  if (!cond) {
    error("expression in if statement condition");
  }

  if (!match(Token::CPAREN)) {
    error("')' after if expression");
  }

  auto ifblock = statement();

  if (!ifblock) {
    error("body after if");
  }

  nodePtr node(nullptr);

  if (match(Token::ELSE)) {
    auto elseblock = statement();
    if (!elseblock) {
      error("body after else");
    }
    node = makeNode<IfElseStmt>(std::move(cond), std::move(ifblock),
                                std::move(elseblock));
  } else {
    node = makeNode<IfStmt>(std::move(cond), std::move(ifblock));
  }

  node->line = if_line;
  return node;
}

// gotoStmt = GOTO expression SEMCOL .
nodePtr Parser::gotoStmt() {
  if (!match(Token::GOTO)) {
    return nullptr;
  }

  auto goto_line = m_lexer->line();

  auto expr = expression();
  if (!expr) {
    error("expression after goto");
  }

  auto node = makeNode<GotoStmt>(std::move(expr));
  node->line = goto_line;

  return node;
}

// whileStmt = WHILE OPAREN expression CPAREN statement .
nodePtr Parser::whileStmt() {
  if (!match(Token::WHILE)) {
    return nullptr;
  }

  auto while_line = m_lexer->line();

  if (!match(Token::OPAREN)) {
    error("'(' after while statement");
  }

  auto expr = expression();
  if (!expr) {
    error("an expression after while");
  }

  if (!match(Token::CPAREN)) {
    error("')' after while expression");
  }

  auto body = statement();
  if (!body) {
    error("a statement after while");
  }

  auto node = makeNode<WhileStmt>(std::move(expr), std::move(body));
  node->line = while_line;

  return node;
}

// primary = literal | OPAREN expression CPAREN .
nodePtr Parser::primary() {
  nodePtr node = literal();

  if (node) {
    return node;
  }

  if (!match(Token::OPAREN)) {
    return node;
  }

  node = expression();

  if (!node) {
    error("an expression");
  }

  if (!match(Token::CPAREN)) {
    error("')' after expression");
  }

  return node;
}

// argumentlist = expression { COMMA expression } .
std::vector<nodeT<ActualExpr>> Parser::argumentlist() {
  std::vector<nodeT<ActualExpr>> actuals;

  auto actual = expression();
  if (!actual) {
    return actuals;
  }

  while (true) {
    auto node = makeNode<ActualExpr>(std::move(actual));
    actuals.emplace_back(std::move(node));
    if (!match(Token::COMMA)) {
      break;
    }
    actual = expression();
    if (!actual) {
      error("an argument after comma");
    }
  }

  return actuals;
}

// functioninvocation  = OPAREN [ argumentlist ] CPAREN .
nodeT<Actuals> Parser::functioninvocation() {
  if (!match(Token::OPAREN)) {
    return nullptr;
  }

  auto args = argumentlist();
  if (!match(Token::CPAREN)) {
    error("')' after argument(s)");
  }

  auto node = makeNode<Actuals>(std::move(args));
  return node;
}

// postfixexpression   = primary postfixexpression1
//                     | identifier [ functioninvocation ] postfixexpression1 .
// postfixexpression1  = [ ( INC | DEC ) [ postfixexpression1 ] ] .
//
// postfixexpression   = primary { INC | DEC }
//                     | identifier [ functioninvocation ] { INC | DEC } .
nodePtr Parser::postfixexpression() {
  nodePtr node(nullptr);
  nodePtr temp = primary();

  auto parse_postfixexpression1 = [&]() {
    while (true) {
      if (match(Token::INC) || match(Token::DEC)) {
        node = makeNode<UnaryExpr>(
            m_lexer->current() == Token::INC ? Op::POSTINC : Op::POSTDEC,
            std::move(temp));
        node->line = m_lexer->line();
        temp = std::move(node);
      } else {
        node.reset();
        node = std::move(temp);
        break;
      }
    }
  };

  if (temp) {
    parse_postfixexpression1();
    return node;
  }

  auto id = identifier();
  if (!id) {
    return node;
  }

  // just returns the actuals
  auto args = functioninvocation();
  if (args){
    node = std::make_unique<FuncCallExpr>(id->value, std::move(args));
    node->line = id->line;

    // need to do this to parse the inc/dec operators
    temp = std::move(node);
    node.reset();
  } else {
    temp = std::move(id);
    id.reset();
  }
  parse_postfixexpression1();

  return node;
}

// unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression |
// postfixexpression .
nodePtr Parser::unaryexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  if (match(Token::MINUS) || match(Token::NOT) || match(Token::INC) ||
      match(Token::DEC)) {
    // needed for preinc and predec
    Op op = [this]() {
      switch (m_lexer->current()) {
        case Token::MINUS:
          return Op::SUB;
        case Token::NOT:
          return Op::NOT;
        case Token::INC:
          return Op::PREINC;
        case Token::DEC:
          return Op::PREDEC;
        default:
          error(
              "Failed to convert Token to Operator. Oops! Something is "
              "seriously wrong with the compiler!");
      }
    }();

    auto op_line = m_lexer->line();

    temp = unaryexpression();
    if (!temp) {
      error("a unary expression");
    }

    node = makeNode<UnaryExpr>(op, std::move(temp));
    node->line = op_line;
    return node;
  }

  return postfixexpression();
}

// multiplicativeexpression = unaryexpression { mult_op unaryexpression } .
// mult_op = MULT | DIV | MOD .
nodePtr Parser::multiplicativeexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = unaryexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::MULT) && !match(Token::DIV) && !match(Token::MOD)) {
    return node;
  }

  while (true) {
    auto op = tokenToOp(m_lexer->current());
    auto op_line = m_lexer->line();

    auto rhs = unaryexpression();
    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(op, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::MULT) && !match(Token::DIV) && !match(Token::MOD)) {
      break;
    }
  }

  return node;
}

// additiveexpression = multiplicativeexpression { add_op
// multiplicativeexpression } . add_op = PLUS | MINUS .
nodePtr Parser::additiveexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = multiplicativeexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::PLUS) && !match(Token::MINUS)) {
    return node;
  }

  while (true) {
    auto op = tokenToOp(m_lexer->current());
    auto op_line = m_lexer->line();

    auto rhs = multiplicativeexpression();
    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(op, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::PLUS) && !match(Token::MINUS)) {
      break;
    }
  }

  return node;
}

// shiftexpression = additiveexpression { shift_op additiveexpression } .
// shift_op = LSHIFT | RSHIFT .
nodePtr Parser::shiftexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = additiveexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::LSHIFT) && !match(Token::RSHIFT)) {
    return node;
  }

  while (true) {
    auto op = tokenToOp(m_lexer->current());
    auto op_line = m_lexer->line();

    auto rhs = additiveexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BitwiseExpr>(op, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LSHIFT) && !match(Token::RSHIFT)) {
      break;
    }
  }

  return node;
}

// relationalexpression = shiftexpression { rel_op shiftexpression } .
// rel_op = LT | GT | LE | GE .
nodePtr Parser::relationalexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = shiftexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::GT) && !match(Token::LT) && !match(Token::LTEQ) &&
      !match(Token::GTEQ)) {
    return node;
  }

  while (true) {
    auto op = tokenToOp(m_lexer->current());
    auto op_line = m_lexer->line();

    auto rhs = shiftexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(op, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::GT) && !match(Token::LT) && !match(Token::LTEQ) &&
        !match(Token::GTEQ)) {
      break;
    }
  }

  return node;
}

// equalityexpression = relationalexpression { eq_op relationalexpression } .
// eq_op = EQ | NE .
nodePtr Parser::equalityexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = relationalexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::EQ) && !match(Token::NEQ)) {
    return node;
  }

  while (true) {
    auto op = tokenToOp(m_lexer->current());
    auto op_line = m_lexer->line();

    auto rhs = relationalexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(op, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::EQ) && !match(Token::NEQ)) {
      break;
    }
  }

  return node;
}

// bitwiseandexpression = equalityexpression { BAND equalityexpression } .
nodePtr Parser::bitwiseandexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = equalityexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::BAND)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();
    auto rhs = equalityexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BitwiseExpr>(Op::BAND, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::BAND)) {
      break;
    }
  }

  return node;
}

// exclusiveorexpression = bitwiseandexpression { XOR bitwiseandexpression } .
nodePtr Parser::xorexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = bitwiseandexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::XOR)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();
    auto rhs = bitwiseandexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BitwiseExpr>(Op::XOR, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::XOR)) {
      break;
    }
  }

  return node;
}

// bitwiseorexpreession = exclusiveorexpression { BOR exclusiveorexpression } .
nodePtr Parser::bitwiseorexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = xorexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::BOR)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();
    auto rhs = xorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BitwiseExpr>(Op::BOR, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::BOR)) {
      break;
    }
  }

  return node;
}

// conditionalandexpression = bitwiseorexpreession { LAND bitwiseorexpreession }
// .
nodePtr Parser::conditionalandexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = bitwiseorexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::LAND)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();
    auto rhs = bitwiseorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(Op::LAND, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LAND)) {
      break;
    }
  }

  return node;
}

// conditionalorexpression = conditionalandexpression { LOR
// conditionalandexpression } .
nodePtr Parser::conditionalorexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = conditionalandexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::LOR)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();
    auto rhs = conditionalandexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<BinaryExpr>(Op::LOR, std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LOR)) {
      break;
    }
  }

  return node;
}

// assignmentexpression    = conditionalorexpression { ASS
// conditionalorexpression } .
nodePtr Parser::assignmentexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = conditionalorexpression();

  if (!node) {
    return nullptr;
  }

  if (!match(Token::ASSIGN)) {
    return node;
  }

  while (true) {
    auto op_line = m_lexer->line();

    auto rhs = conditionalorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp = makeNode<AssignExpr>(std::move(node), std::move(rhs));
    temp->line = op_line;

    node = std::move(temp);
    temp.reset();

    if (!match(Token::ASSIGN)) {
      break;
    }
  }

  return node;
}

// assignment  = assignmentexpression .
nodePtr Parser::assignment() { return assignmentexpression(); }

// expression  = assignmentexpression .
nodePtr Parser::expression() { return assignmentexpression(); }

// statement = variabledeclaration | simpleStmt | returnStmt |
//              breakStmt | block | ifStmt | gotoStmt | whileStmt .
nodePtr Parser::statement() {
  nodePtr node(nullptr);

  node = variabledeclaration();

  if (!node) {
    node = simpleStmt();
  }
  if (!node) {
    node = returnStmt();
  }
  if (!node && match(Token::BREAK)) {
    node = makeNode<BreakStmt>();
    node->line = m_lexer->line();
  }
  if (!node) {
    node = block();
  }
  if (!node) {
    node = ifStmt();
  }
  if (!node) {
    node = gotoStmt();
  }
  if (!node) {
    node = whileStmt();
  }

  return node;
}

// block = OBRCK [ blockstatements ] CBRCK .
// block = OBRCK [ statement { statement } ] CBRCK .
nodePtr Parser::block() {
  if (!match(Token::OBRCK)) {
    return nullptr;
  }

  nodePtr block_node = makeNode<BlockStmt>();

  while (true) {
    auto node = statement();
    if (!node) {
      break;
    }
    block_node->children.emplace_back(std::move(node));
  }

  if (!match(Token::CBRCK)) {
    error("}");
  }

  return block_node;
}

// formalparameterlist = formalparameter { COMMA formalparameter } .
std::vector<nodeT<ParamDecl>> Parser::formalparameterlist() {
  std::vector<nodeT<ParamDecl>> params;

  // formalparameter = type identifier .
  auto formalparameter = [&]() {
    auto vtype = type();
    if (!vtype.has_value()) {
      return nodeT<ParamDecl>(nullptr);
    }

    auto id = identifier();
    if (!id) {
      error("identifier");
    }

    auto param = makeNode<ParamDecl>(id->value, vtype.value());
    param->line = m_lexer->line();

    return param;
  };

  auto param = formalparameter();
  if (!param) {
    return params;
  }

  while (true) {
    params.emplace_back(std::move(param));

    if (!match(Token::COMMA)) {
      break;
    }

    param = formalparameter();
    if (!param) {
      error("a parameter after comma");
    }
  }

  return params;
}

// functiondeclarator  = OPAREN [ formalparameterlist ] CPAREN .
nodeT<Params> Parser::functiondeclarator() {
  if (!match(Token::OPAREN)) {
    return nullptr;
  }

  auto params = makeNode<Params>(formalparameterlist());

  if (!match(Token::CPAREN)) {
    error("')'");
  }

  return params;
}

// mainfunctiondeclaration = id OPAREN CPAREN block .
nodePtr Parser::mainfunctiondeclaration() {
  auto id = identifier();
  if (!id) {
    error("identifier");
  }

  if (!match(Token::OPAREN)) {
    error("'('");
  }

  if (!match(Token::CPAREN)) {
    m_logger->error(
        m_lexer->line(),
        ": Main function cannot take any parameters. Expected ')' but found ",
        m_lexer->peek());
  }

  auto body = block();
  if (!body) {
    error("block after main function declaration");
  }
  auto node = makeNode<MFuncDecl>(id->value, std::move(body));
  node->line = id->line;
  return node;
}

// globaldeclaration   =
//      type identifier ( functiondeclarator | SEMCOL )
//      | VOID identifier functiondeclarator
//      | mainfunctiondeclaration .
// functiondeclarator has been expanded here as it's only use is here!
nodePtr Parser::globaldeclaration() {
  std::optional<VType> vtype = type();

  // type id ( functiondeclaration | ; )
  if (vtype.has_value()) {
    auto type_line = m_lexer->line();

    auto id = identifier();
    if (!id) {
      error("identifier");
    }

    auto params = functiondeclarator();
    if (params) {
      auto body = block();
      if (!body) {
        error("function body");
      }

      auto node = makeNode<FuncDecl>(
          id->value, vtype.value(), std::move(params), std::move(body));
      node->line = type_line;
      return node;
    }
    // var decl
    if (!match(Token::SEMICOL)) {
      error("semicolon after variable declaration");
    }

    auto node = makeNode<GVarDecl>(id->value, vtype.value());
    node->line = type_line;

    return node;
  }

  if (match(Token::VOID)) {  // void id functiondeclaration
    auto type_line = m_lexer->line();

    auto id = identifier();
    if (!id) {
      error("identifier");
    }

    auto params = functiondeclarator();
    if (!params) {
      error("function declaration");
    }

    auto body = block();
    if (!body) {
      error("function body");
    }
    auto node = makeNode<FuncDecl>(id->value, VType::Void,
                                   std::move(params), std::move(body));
    node->line = type_line;

    return node;
  }

  if (m_lexer->peek() == Token::ID)  // Main function
  {
    return mainfunctiondeclaration();
  }

  m_logger->error(m_lexer->line(),
                  ": Expected either a function or variable declaration. Got: ",
                  m_lexer->peek());
}

// start = { globaldeclaration }
void Parser::start() {
  while (m_lexer->peek() != Token::T_EOF) {
    m_ast->children.emplace_back(globaldeclaration());
  }
}
