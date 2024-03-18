#include "parser.h"
#include <memory>
#include <optional>
#include "common/ast.h"
#include "common/token.h"
#include "lexer/lexer.h"

using nodePtr = std::unique_ptr<ASTNode>;

// Helper functions
template <class T>
inline std::unique_ptr<T> makeNode() {
  return std::make_unique<T>();
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
  m_current_node.reset();
}

bool Parser::parse() {
  m_ast = makeNode<ProgNode>();
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
                  tokenToStr(m_lexer->peek()), "'");
}

// type = BOOL | INT | STR
std::optional<VType> Parser::type() {
  if (match(Token::BOOL)) {
    return VType::Bool;
  } else if (match(Token::INT)) {
    return VType::Int;
  } else if (match(Token::STR)) {
    return VType::Str;
  }

  return {};
}

nodePtr Parser::identifier() {
  nodePtr id(nullptr);

  if (match(Token::ID)) {
    id = makeNode<IdExpr>();
    id->line = m_lexer->line();
    id->value = m_lexer->lexeme();
  }

  return std::move(id);
}

// literal = NUMBER | STRING | TRUE | FALSE .
nodePtr Parser::literal() {
  nodePtr node(nullptr);

  if (match(Token::NUM)) {
    node = makeNode<LitExpr>();
    node->val_type = VType::Int;
    node->line = m_lexer->line();
    node->value = static_cast<int32_t>(std::stoul(m_lexer->lexeme()));
  } else if (match(Token::STRLIT)) {
    node = makeNode<LitExpr>();
    node->val_type = VType::Str;
    node->line = m_lexer->line();
    node->value = m_lexer->lexeme();
  } else if (match(Token::TRUE)) {
    node = makeNode<LitExpr>();
    node->val_type = VType::Bool;
    node->line = m_lexer->line();
    node->value = true;
  } else if (match(Token::FALSE)) {
    node = makeNode<LitExpr>();
    node->val_type = VType::Bool;
    node->line = m_lexer->line();
    node->value = false;
  }

  return std::move(node);
}

// variabledeclaration = type identifier SEMCOL
nodePtr Parser::variabledeclaration() {
  nodePtr node(nullptr);
  std::optional<VType> vtype;

  if (vtype = type(), !vtype.has_value()) {
    return std::move(node);
  }

  node = makeNode<VarDecl>();

  node->val_type = std::move(vtype);
  node->line = m_lexer->line();

  auto id = identifier();
  if (!id) {
    error("identifier in a variable declaration after type");
  }

  node->children.emplace_back(std::move(id));

  if (!match(Token::SEMICOL)) {
    error("semicolon after variable declaration");
  }

  return std::move(node);
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

  return std::move(node);
}

// exprStmt = identifier ( assignment | functioninvocation ) SEMCOL .
// exprStmt = expression SEMCOL .
nodePtr Parser::exprStmt() {
  nodePtr node(nullptr);

  auto temp = expression();

  if (!temp) {
    return nullptr;
  }

  node = makeNode<ExprStmt>();
  node->children.emplace_back(std::move(temp));

  if (!match(Token::SEMICOL)) {
    error("semicolon");
  }

  return std::move(node);
}

// returnStmt = RETURN [ expression ] SEMCOL .
nodePtr Parser::returnStmt() {
  nodePtr node(nullptr);

  if (!match(Token::RETURN)) {
    return std::move(node);
  }

  node = makeNode<ReturnStmt>();
  node->line = m_lexer->line();

  auto expr = expression();
  if (expr) {
    node->children.emplace_back(std::move(expr));
  }

  if (!match(Token::SEMICOL)) {
    error("semicolon after return statement");
  }

  return std::move(node);
}

// ifStmt = IF OPAREN expression CPAREN statement [ ELSE statement ] .
nodePtr Parser::ifStmt() {
  nodePtr node(nullptr);

  if (!match(Token::IF)) {
    return std::move(node);
  }

  node = makeNode<IfStmt>();
  node->line = m_lexer->line();

  if (!match(Token::OPAREN)) {
    error("'(' after if");
  }

  auto expr = expression();
  if (!expr) {
    error("expression in if statement");
  }

  node->children.emplace_back(std::move(expr));

  if (!match(Token::CPAREN)) {
    error("')' after if expression");
  }

  auto ifblock = statement();
  node->children.emplace_back(std::move(ifblock));

  if (match(Token::ELSE)) {
    auto newNode = makeNode<IfElseStmt>();
    newNode->children = std::move(node->children);
    newNode->line = node->line;
    node = std::move(newNode);
    newNode.reset();

    auto elseblock = statement();
    node->children.emplace_back(std::move(elseblock));
  }

  return std::move(node);
}

// gotoStmt = GOTO expression SEMCOL .
nodePtr Parser::gotoStmt() {
  nodePtr node(nullptr);

  if (!match(Token::GOTO)) {
    return std::move(node);
  }

  node = makeNode<GotoStmt>();
  node->line = m_lexer->line();

  auto expr = expression();
  if (!expr) {
    error("expression after goto");
  }

  node->children.emplace_back(std::move(expr));

  return std::move(node);
}

// whileStmt = WHILE OPAREN expression CPAREN statement .
nodePtr Parser::whileStmt() {
  nodePtr node(nullptr);

  if (!match(Token::WHILE)) {
    return std::move(node);
  }

  node = makeNode<WhileStmt>();
  node->line = m_lexer->line();

  if (!match(Token::OPAREN)) {
    error("'(' after while statement");
  }

  auto expr = expression();

  if (!expr) {
    error("an expression after while");
  }

  node->children.emplace_back(std::move(expr));

  if (!match(Token::CPAREN)) {
    error("')' after while expression");
  }

  auto stmt = statement();

  if (!stmt) {
    error("a statement after while");
  }

  node->children.emplace_back(std::move(stmt));

  return std::move(node);
}

// primary = literal | OPAREN expression CPAREN .
nodePtr Parser::primary() {
  nodePtr node(nullptr);

  if (node = literal(), node) {
    return std::move(node);
  }

  if (!match(Token::OPAREN)) {
    return std::move(node);
  }

  node = expression();

  if (!node) {
    error("an expression");
  }

  if (!match(Token::CPAREN)) {
    error("')' after expression");
  }

  return std::move(node);
}

// argumentlist = expression { COMMA expression } .
nodePtr Parser::argumentlist() {
  nodePtr actuals = makeNode<ActualsExpr>();

  auto actual = expression();
  if (!actual) {
    return std::move(actuals);
  }

  while (true) {
    // actual->kind = ExprType::Actual;
    actuals->children.emplace_back(std::move(actual));
    if (!match(Token::COMMA)) {
      break;
    }
    actual = expression();
    if (!actual) {
      error("an argument after comma");
    }
  }

  return std::move(actuals);
}

// functioninvocation  = OPAREN [ argumentlist ] CPAREN .
nodePtr Parser::functioninvocation() {
  nodePtr node(nullptr);

  if (!match(Token::OPAREN)) {
    return std::move(node);
  }

  node = argumentlist();

  if (!match(Token::CPAREN)) {
    error("')' after argument(s)");
  }

  return std::move(node);
}

// postfixexpression   = primary postfixexpression1
//                     | identifier [ functioninvocation ] postfixexpression1 .
// postfixexpression1  = [ ( INC | DEC ) [ postfixexpression1 ] ] .
//
// postfixexpression   = primary { INC | DEC }
//                     | identifier [ functioninvocation ] { INC | DEC } .
nodePtr Parser::postfixexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  if (temp = primary(), temp) {
    while (true) {
      if (match(Token::INC) || match(Token::DEC)) {
        node = makeNode<UnaryExpr>();
        node->line = m_lexer->line();
        node->op = m_lexer->current() == Token::INC ? Op::POSTINC : Op::POSTDEC;
        node->children.emplace_back(std::move(temp));
        temp = std::move(node);
      } else {
        node.reset();
        node = std::move(temp);
        break;
      }
    }
  } else if (temp = identifier(), temp) {
    // just returns the actuals
    auto args = functioninvocation();
    if (args) {
      node = makeNode<FuncCallExpr>();
      node->children.emplace_back(std::move(temp));
      node->children.emplace_back(std::move(args));

      temp.reset();
      temp = std::move(node);
      node.reset();
    }

    while (true) {
      if (match(Token::INC) || match(Token::DEC)) {
        node = makeNode<UnaryExpr>();
        node->line = m_lexer->line();
        node->op = m_lexer->current() == Token::INC ? Op::POSTINC : Op::POSTDEC;
        node->children.emplace_back(std::move(temp));
        temp = std::move(node);
      } else {
        node.reset();
        node = std::move(temp);
        break;
      }
    }
  }

  return std::move(node);
}

// unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression |
// postfixexpression .
nodePtr Parser::unaryexpression() {
  nodePtr node(nullptr);
  nodePtr temp(nullptr);

  node = makeNode<UnaryExpr>();
  if (match(Token::MINUS) || match(Token::NOT) || match(Token::INC) ||
      match(Token::DEC)) {
    // needed for preinc and predec
    node->op = [this]() {
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
              "seriously wrong!");
      }
    }();
    node->line = m_lexer->line();
    temp = unaryexpression();
    if (!temp) {
      error("a unary expression");
    }
    node->children.emplace_back(std::move(temp));
  } else {
    node.reset();
    node = postfixexpression();
  }
  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = tokenToOp(m_lexer->current());

    auto rhs = unaryexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::MULT) && !match(Token::DIV) && !match(Token::MOD)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = tokenToOp(m_lexer->current());

    auto rhs = multiplicativeexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::PLUS) && !match(Token::MINUS)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BitwiseExpr>();
    temp->line = m_lexer->line();
    temp->op = tokenToOp(m_lexer->current());

    auto rhs = additiveexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LSHIFT) && !match(Token::RSHIFT)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = tokenToOp(m_lexer->current());

    auto rhs = shiftexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::GT) && !match(Token::LT) && !match(Token::LTEQ) &&
        !match(Token::GTEQ)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = tokenToOp(m_lexer->current());

    auto rhs = relationalexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::EQ) && !match(Token::NEQ)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BitwiseExpr>();
    temp->line = m_lexer->line();
    temp->op = Op::BAND;

    auto rhs = equalityexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::BAND)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BitwiseExpr>();
    temp->line = m_lexer->line();
    temp->op = Op::XOR;

    auto rhs = bitwiseandexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::XOR)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BitwiseExpr>();
    temp->line = m_lexer->line();
    temp->op = Op::BOR;

    auto rhs = xorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::BOR)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = Op::LAND;

    auto rhs = bitwiseorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LAND)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<BinaryExpr>();
    temp->line = m_lexer->line();
    temp->op = Op::LOR;

    auto rhs = conditionalandexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::LOR)) {
      break;
    }
  }

  return std::move(node);
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
    return std::move(node);
  }

  while (true) {
    temp = makeNode<AssignExpr>();
    temp->line = m_lexer->line();

    auto rhs = conditionalorexpression();

    if (!rhs) {
      error("an expression");
    }

    temp->children.emplace_back(std::move(node));  // lhs
    temp->children.emplace_back(std::move(rhs));   // lhs

    node = std::move(temp);
    temp.reset();

    if (!match(Token::ASSIGN)) {
      break;
    }
  }

  return std::move(node);
}

// assignment  = assignmentexpression .
nodePtr Parser::assignment() { return std::move(assignmentexpression()); }

// expression  = assignmentexpression .
nodePtr Parser::expression() { return std::move(assignmentexpression()); }

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

  return std::move(node);
}

// block = OBRCK [ blockstatements ] CBRCK .
// block = OBRCK [ statement { statement } ] CBRCK .
nodePtr Parser::block() {
  if (!match(Token::OBRCK)) {
    return nullptr;
  }

  nodePtr blockNode = makeNode<BlockStmt>();

  while (true) {
    auto node = statement();
    if (!node) {
      break;
    }
    blockNode->children.emplace_back(std::move(node));
  }

  if (!match(Token::CBRCK)) {
    error("}");
  }

  return std::move(blockNode);
}

// formalparameterlist = formalparameter { COMMA formalparameter } .
nodePtr Parser::formalparameterlist() {
  nodePtr params = makeNode<ParamsDecl>();

  // formalparameter = type identifier .
  auto formalparameter = [&]() {
    nodePtr param(nullptr);
    auto vtype = type();
    if (!vtype.has_value()) {
      return std::move(param);
    }

    param = makeNode<ParamDecl>();
    param->val_type = std::move(vtype);
    param->line = m_lexer->line();

    auto id = identifier();
    if (!id) {
      error("identifier");
    }

    param->value = id->value;

    return std::move(param);
  };

  auto param = formalparameter();
  if (!param) {
    return std::move(params);
  }

  while (true) {
    params->children.emplace_back(std::move(param));

    if (!match(Token::COMMA)) {
      break;
    }

    param = formalparameter();
    if (!param) {
      error("a parameter");
    }
  }

  return std::move(params);
}

// functiondeclarator  = OPAREN [ formalparameterlist ] CPAREN .
nodePtr Parser::functiondeclarator() {
  if (!match(Token::OPAREN)) {
    return nodePtr(nullptr);
  }

  nodePtr params = formalparameterlist();

  if (!match(Token::CPAREN)) {
    error("')'");
  }

  return std::move(params);
}

// mainfunctiondeclaration = id OPAREN CPAREN block .
void Parser::mainfunctiondeclaration() {
  m_current_node = makeNode<MFuncDecl>();
  auto node = identifier();
  if (!node) {
    error("identifier");
  }

  m_current_node->line = m_lexer->line();
  m_current_node->val_type = VType::Int;

  // id
  m_current_node->children.emplace_back(std::move(node));

  if (!match(Token::OPAREN)) {
    error("'('");
  }

  if (!match(Token::CPAREN))
    m_logger->error(
        m_lexer->line(),
        ": Main function cannot take any parameters. Expected ')' but found ",
        tokenToStr(m_lexer->peek()));

  node = block();
  m_current_node->children.emplace_back(std::move(node));
}

// globaldeclaration   =
//      type identifier ( functiondeclarator | SEMCOL )
//      | VOID identifier functiondeclarator
//      | mainfunctiondeclaration .
// functiondeclarator has been expanded here as it's only use is here!
nodePtr Parser::globaldeclaration() {
  // m_current_node = makeNode(NType::Decl);

  std::optional<VType> vtype;

  // type id ( functiondeclaration | ; )
  if (vtype = type(), vtype.has_value()) {
    m_current_node = makeNode<GVarDecl>();
    m_current_node->val_type = std::move(vtype);
    m_current_node->line = m_lexer->line();

    auto node = identifier();
    if (!node) {
      error("identifier");
    }

    m_current_node->children.emplace_back(std::move(node));

    if (node = functiondeclarator(), node) {
      m_current_node->children.emplace_back(std::move(node));

      node = block();
      if (!node) {
        error("function body");
      }

      m_current_node->children.emplace_back(std::move(node));

      auto temp = makeNode<FuncDecl>();
      temp->children = std::move(m_current_node->children);
      temp->val_type = std::move(m_current_node->val_type);
      temp->line = std::move(m_current_node->line);

      m_current_node = std::move(temp);
    } else {
      if (!match(Token::SEMICOL)) {
        error("semicolon");
      }
    }
  } else if (match(Token::VOID))  // void id functiondeclaration
  {
    m_current_node = makeNode<FuncDecl>();
    m_current_node->val_type = VType::Void;
    m_current_node->line = m_lexer->line();

    auto node = identifier();
    if (!node) {
      error("identifier");
    }

    m_current_node->children.emplace_back(std::move(node));

    if (node = functiondeclarator(), !node) {
      error("function declaration");
    }

    m_current_node->children.emplace_back(std::move(node));

    node = block();
    if (!node) {
      error("function body");
    }

    m_current_node->children.emplace_back(std::move(node));
  } else if (m_lexer->peek() == Token::ID)  // Main function
  {
    mainfunctiondeclaration();
  } else {
    m_logger->error(
        m_lexer->line(),
        ": Expected either a function or variable declaration. Got: ",
        tokenToStr(m_lexer->peek()));
  }

  return std::move(m_current_node);
}

// start = { globaldeclaration }
void Parser::start() {
  while (m_lexer->peek() != Token::T_EOF) {
    m_ast->children.emplace_back(std::move(globaldeclaration()));
  }
}
