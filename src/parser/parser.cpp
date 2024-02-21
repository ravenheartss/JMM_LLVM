#include "parser.h"
#include "common/ast.h"
#include "common/jmm.h"
#include "lexer/lexer.h"
#include "common/token.h"
#include <limits>
#include <memory>
#include <optional>

using nodePtr = std::shared_ptr<ASTNode>;

// Helper functions
inline nodePtr makeNode(NType ntype)
{
    return std::make_shared<ASTNode>(ntype);
}

inline nodePtr makeNode(NType ntype, std::variant<StmtType,DeclType,ExprType> nkind)
{
    return std::make_shared<ASTNode>(ntype, nkind);
}

Op tokenToOp(Token tok)
{
    switch (tok)
    {
        case Token::MINUS:      return Op::SUB;
        case Token::PLUS:       return Op::ADD;
        case Token::MULT:       return Op::MULT;
        case Token::DIV:        return Op::DIV;
        case Token::MOD:        return Op::MOD;
        case Token::XOR:        return Op::XOR;
        case Token::GT:         return Op::GT;
        case Token::LT:         return Op::LT;
        case Token::GTEQ:       return Op::GE;
        case Token::LTEQ:       return Op::LE;
        case Token::LSHIFT:     return Op::LSHIFT;
        case Token::RSHIFT:     return Op::RSHIFT;
        case Token::EQ:         return Op::EQ;
        case Token::NOT:        return Op::NOT;
        case Token::NEQ:        return Op::NE;
        case Token::LAND:       return Op::LAND;
        case Token::LOR:        return Op::LOR;
        case Token::BAND:       return Op::BAND;
        case Token::BOR:        return Op::BOR;
        default:                return Op::PREINC; // error
    }
}

Parser::~Parser()
{
    m_logger.reset();
    m_lexer.reset();
    m_ast.reset();
    m_current_node.reset();
}

bool Parser::parse()
{
    m_ast = std::make_unique<ASTNode>(NType::Prog);
    start();
    return true;
}

bool Parser::match(Token expected) 
{
    if (m_lexer->peek() != expected)
        return false;

    m_lexer->consume();
    return true;
}

[[noreturn]]
void Parser::error(std::string expected)
{
    m_logger->error(m_lexer->line(), ": Expected ", expected,
            ". Got: '", tokenToStr(m_lexer->peek()), "'");
}

// type = BOOL | INT | STR
std::optional<VType> Parser::type()
{
    if (match(Token::BOOL))
        return VType::Bool;
    else if (match(Token::INT))
        return VType::Int;
    else if (match(Token::STR))
        return VType::Str;

    return {};
}

nodePtr Parser::identifier()
{
    nodePtr id(nullptr);

    if (match(Token::ID))
    {
        id = makeNode(NType::Expr, ExprType::Id);
        id->line = m_lexer->line();
        id->value = m_lexer->lexeme();
    }

    return std::move(id);
}


// literal = NUMBER | STRING | TRUE | FALSE .
nodePtr Parser::literal()
{
    nodePtr node(nullptr);

    if (match(Token::NUM))
    {
        node = makeNode(NType::Expr, ExprType::Lit);
        node->val_type = VType::Int;
        node->line = m_lexer->line();
        node->value = static_cast<int32_t>(std::stoul(m_lexer->lexeme()));
    }
    else if (match(Token::STRLIT))
    {
        node = makeNode(NType::Expr, ExprType::Lit);
        node->val_type = VType::Str;
        node->line = m_lexer->line();
        node->value = m_lexer->lexeme();
    }
    else if (match(Token::TRUE))
    {
        node = makeNode(NType::Expr, ExprType::Lit);
        node->val_type = VType::Bool;
        node->line = m_lexer->line();
        node->value = true;
    }
    else if (match(Token::FALSE))
    {
        node = makeNode(NType::Expr, ExprType::Lit);
        node->val_type = VType::Int;
        node->line = m_lexer->line();
        node->value = false;
    }

    return std::move(node);
}


// variabledeclaration = type identifier SEMCOL
nodePtr Parser::variabledeclaration()
{
    nodePtr node(nullptr);
    std::optional<VType> vtype;

    if (vtype = type(), !vtype.has_value())
    {
        return std::move(node);
    }

    node = makeNode(NType::Decl, DeclType::Var);

    node->val_type = std::move(vtype);
    node->line = m_lexer->line();

    auto id = identifier();
    if (!id)
        error("identifier in a variable declaration after type");

    node->children.push_back(std::move(id));

    if (!match(Token::SEMICOL))
        error("semicolon after variable declaration");

    return std::move(node);
}

// simpleStmt = nullStmt | exprStmt .
// nullStmt = SEMCOL .
nodePtr Parser::simpleStmt()
{
    nodePtr node(nullptr);
    if (match(Token::SEMICOL))
    {
        node = makeNode(NType::Stmt, StmtType::Null);
        node->line = m_lexer->line();
    }
    else
    {
        // can be nullptr
        node = exprStmt();
    }

    return std::move(node);
}

// exprStmt = identifier ( assignment | functioninvocation ) SEMCOL .
nodePtr Parser::exprStmt()
{
    nodePtr node(nullptr);

    auto id = identifier();
    if (!id)
        return std::move(node);

    switch (m_lexer->peek()) 
    {
        case Token::ASSIGN:
            node = assignment();
            break;
        case Token::OPAREN:
            node = functioninvocation();
            break;
        default:
            break;
    }

    if (!node)
        error("assignment or function invocation");

    if (!match(Token::SEMICOL))
        error("semicolon");

    return std::move(node);
}

// returnStmt = RETURN [ expression ] SEMCOL .
nodePtr Parser::returnStmt()
{
    nodePtr node(nullptr);

    if (!match(Token::RETURN))
        return std::move(node);

    node = makeNode(NType::Stmt, StmtType::Return);
    node->line = m_lexer->line();

    auto expr = expression();
    if (expr)
        node->children.push_back(std::move(expr));

    if (!match(Token::SEMICOL))
        error("semicolon after return statement");

    return std::move(node);
}

// ifStmt = IF OPAREN expression CPAREN statement [ ELSE statement ] .
nodePtr Parser::ifStmt()
{
    nodePtr node(nullptr);

    if (!match(Token::IF))
        return std::move(node);

    node = makeNode(NType::Stmt, StmtType::If);
    node->line = m_lexer->line();

    if (!match(Token::OPAREN))
        error("'(' after if");

    auto expr = expression();
    if (!expr)
        error("expression in if statement");

    if (!match(Token::CPAREN))
        error("')' after if expression");

    auto ifblock = statement();
    node->children.push_back(std::move(ifblock));

    if (match(Token::ELSE))
    {
        node->kind = StmtType::IfElse;
        auto elseblock = statement();
        node->children.push_back(std::move(elseblock));
    }

    return std::move(node);

}

// gotoStmt = GOTO expression SEMCOL .
nodePtr Parser::gotoStmt()
{
    nodePtr node(nullptr);

    if (!match(Token::GOTO))
        return std::move(node);

    node = makeNode(NType::Stmt, StmtType::Goto);
    node->line = m_lexer->line();

    auto expr = expression();
    if (!expr) 
        error("expression after goto");

    node->children.push_back(std::move(expr));

    return std::move(node);
}

// whileStmt = WHILE OPAREN expression CPAREN statement .
nodePtr Parser::whileStmt()
{
    nodePtr node(nullptr);

    if (!match(Token::WHILE))
        return std::move(node);

    node = makeNode(NType::Stmt, StmtType::While);
    node->line = m_lexer->line();

    if (!match(Token::OPAREN))
        error("'(' after while statement");

    auto expr = expression();

    if (!expr)
        error("an expression after while");

    node->children.push_back(std::move(expr));

    if (!match(Token::CPAREN))
        error("')' after while expression");

    auto stmt = statement();

    if (!stmt)
        error("a statement after while");

    node->children.push_back(std::move(stmt));

    return std::move(node);
}

// primary = literal | OPAREN expression CPAREN .
nodePtr Parser::primary()
{
    nodePtr node(nullptr);

    if (node = literal(), node)
        return std::move(node);

    if (!match(Token::OPAREN))
        return std::move(node);

    node = expression();

    if (!node)
        error("an expression");

    if (!match(Token::CPAREN))
        error("')' after expression");

    return std::move(node);

}

// argumentlist = expression { COMMA expression } .
nodePtr Parser::argumentlist()
{
    nodePtr actuals =
        makeNode(NType::Expr, ExprType::Actuals);

    while (true)
    {
        if (!match(Token::COMMA))
            break;
        auto actual = expression();
        if (!actual)
            error("an argument after comma");
        actuals->children.push_back(std::move(actual));
    }

    return std::move(actuals);
}

// functioninvocation  = OPAREN [ argumentlist ] CPAREN .
nodePtr Parser::functioninvocation()
{
    nodePtr node(nullptr);

    if (!match(Token::OPAREN))
        return std::move(node);

    node = argumentlist();

    if (!match(Token::CPAREN))
        error("')' after argument(s)");

    return std::move(node);
}

// postfixexpression   = primary postfixexpression1
//                     | identifier [ functioninvocation ] postfixexpression1 .
// postfixexpression1  = [ ( INC | DEC ) [ postfixexpression1 ] ] .
// 
// postfixexpression   = primary { INC | DEC }
//                     | identifier [ functioninvocation ] { INC | DEC } .
nodePtr Parser::postfixexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    if (temp = primary(), temp)
    {
        node = makeNode(NType::Expr);
        while (true)
        {
            if (match(Token::INC) || match(Token::DEC))
            {
                node->kind = ExprType::Unary;
                node->line = m_lexer->line();
                node->op = m_lexer->current() == Token::INC ? Op::POSTINC : Op::POSTDEC;
                node->children.push_back(std::move(temp));
            }
            else
            {
                node.reset();
                node = std::move(temp);
                temp.reset();
                break;
            }
        }
    }
    else if (temp = identifier(), temp)
    {
        // just returns the actuals
        auto args = functioninvocation();
        if (args)
        {
            node = makeNode(NType::Expr, ExprType::FuncCall);
            node->children.push_back(std::move(temp));
            node->children.push_back(std::move(args));

            temp.reset();
            temp = std::move(node);
            node.reset();
        }

        node = makeNode(NType::Expr);
        while (true)
        {
            if (match(Token::INC) || match(Token::DEC))
            {
                node->kind = ExprType::Unary;
                node->line = m_lexer->line();
                node->op = m_lexer->current() == Token::INC ? Op::POSTINC : Op::POSTDEC;
                node->children.push_back(std::move(temp));
            }
            else
            {
                node.reset();
                node = std::move(temp);
                temp.reset();
                break;
            }
        }
    }
    else
    {
        error("a primary or identifier");
    }

    return std::move(node);
}

// unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression | postfixexpression .
nodePtr Parser::unaryexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = makeNode(NType::Expr, ExprType::Unary);
    if (match(Token::MINUS) || match(Token::NOT) || match(Token::INC) || match(Token::DEC))
    {
        node->op = [this->m_lexer](){
            switch(m_lexer->current())
            {
                case Token::MINUS:  return Op::SUB;
                case Token::NOT:    return Op::NOT;
                case Token::INC:    return Op::PREINC;
                case Token::DEC:    return Op::PREDEC;
                default:
                    error("Failed to convert Token to Operator. Oops! Something is seriously wrong!");
            }
        }();
        node->line = m_lexer->line();
        temp = unaryexpression();
        if (!temp)
            error("a unary expression");
        node->children.push_back(std::move(temp));
    }
    else
    {
        node.reset();
        node = postfixexpression();
    }
    return std::move(node);
}

// multiplicativeexpression = unaryexpression { mult_op unaryexpression } .
// mult_op = MULT | DIV | MOD .
nodePtr Parser::multiplicativeexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = unaryexpression();

    if (!node)
        error("an expression");

    if (!match(Token::MULT) || !match(Token::DIV) || !match(Token::MOD))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = tokenToOp(m_lexer->current());

        auto rhs = unaryexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::MULT) || !match(Token::DIV) || !match(Token::MOD))
            break;
    }

    return std::move(node);

}

// additiveexpression = multiplicativeexpression { add_op multiplicativeexpression } .
// add_op = PLUS | MINUS .
nodePtr Parser::additiveexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = multiplicativeexpression();

    if (!node)
        error("an expression");

    if (!match(Token::PLUS) || !match(Token::MINUS))
        return std::move(node);

    while (true)
    {
        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = tokenToOp(m_lexer->current());

        auto rhs = multiplicativeexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::PLUS) || !match(Token::MINUS))
            break;
    }

    return std::move(node);
}

// shiftexpression = additiveexpression { shift_op additiveexpression } .
// shift_op = LSHIFT | RSHIFT .
nodePtr Parser::shiftexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = additiveexpression();

    if (!node)
        error("an expression");

    if (!match(Token::LSHIFT) || !match(Token::RSHIFT))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Bitwise);
        temp->line = m_lexer->line();
        temp->op = tokenToOp(m_lexer->current());

        auto rhs = additiveexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::LSHIFT) || !match(Token::RSHIFT))
            break;
    }

    return std::move(node);
}

// relationalexpression = shiftexpression { rel_op shiftexpression } .
// rel_op = LT | GT | LE | GE .
nodePtr Parser::relationalexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = shiftexpression();

    if (!node)
        error("an expression");

    if (!match(Token::GT) || !match(Token::LT) || !match(Token::LTEQ) || !match(Token::GTEQ))
        return std::move(node);

    while (true)
    {
        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = tokenToOp(m_lexer->current());

        auto rhs = shiftexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::GT) || !match(Token::LT)
                || !match(Token::LTEQ) || !match(Token::GTEQ))
            break;
    }

    return std::move(node);
}

// equalityexpression = relationalexpression { eq_op relationalexpression } .
// eq_op = EQ | NE .
nodePtr Parser::equalityexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = relationalexpression();

    if (!node)
        error("an expression");

    if (!match(Token::EQ) || !match(Token::NEQ))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = tokenToOp(m_lexer->current());

        auto rhs = relationalexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::EQ) || !match(Token::NEQ))
            break;
    }

    return std::move(node);
}

// bitwiseandexpression = equalityexpression { BAND equalityexpression } .
nodePtr Parser::bitwiseandexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = equalityexpression();

    if (!node)
        error("an expression");

    if (!match(Token::BAND))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Bitwise);
        temp->line = m_lexer->line();
        temp->op = Op::BAND;

        auto rhs = equalityexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::BAND))
            break;
    }

    return std::move(node);
}

// exclusiveorexpression = bitwiseandexpression { XOR bitwiseandexpression } .
nodePtr Parser::xorexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = bitwiseandexpression();

    if (!node)
        error("an expression");

    if (!match(Token::XOR))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Bitwise);
        temp->line = m_lexer->line();
        temp->op = Op::XOR;

        auto rhs = bitwiseandexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::XOR))
            break;
    }

    return std::move(node);
}

// bitwiseorexpreession = exclusiveorexpression { BOR exclusiveorexpression } .
nodePtr Parser::bitwiseorexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = xorexpression();

    if (!node)
        error("an expression");

    if (!match(Token::BOR))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Bitwise);
        temp->line = m_lexer->line();
        temp->op = Op::BOR;

        auto rhs = xorexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::BOR))
            break;
    }

    return std::move(node);
}


// conditionalandexpression = bitwiseorexpreession { LAND bitwiseorexpreession } .
nodePtr Parser::conditionalandexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = bitwiseorexpression();

    if (!node)
        error("an expression");

    if (!match(Token::LAND))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = Op::LAND;

        auto rhs = bitwiseorexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::LAND))
            break;
    }

    return std::move(node);
}

// conditionalorexpression = conditionalandexpression { LOR conditionalandexpression } .
nodePtr Parser::conditionalorexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = conditionalandexpression();

    if (!node)
        error("an expression");

    if (!match(Token::LOR))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Binary);
        temp->line = m_lexer->line();
        temp->op = Op::LOR;

        auto rhs = conditionalandexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::LOR))
            break;
    }

    return std::move(node);
}

// assignmentexpression    = conditionalorexpression { ASS conditionalorexpression } .
nodePtr Parser::assignmentexpression()
{
    nodePtr node(nullptr);
    nodePtr temp(nullptr);

    node = conditionalorexpression();

    if (!node)
        error("an expression");

    if (!match(Token::ASSIGN))
        return std::move(node);

    while (true)
    {

        temp = makeNode(NType::Expr, ExprType::Assign);
        temp->line = m_lexer->line();

        auto rhs = conditionalorexpression();

        if (!rhs)
            error("an expression");

        temp->children.push_back(node); // lhs
        temp->children.push_back(rhs);

        node = std::move(temp);
        temp.reset();

        if (!match(Token::ASSIGN))
            break;
    }

    return std::move(node);
}

// assignment  = ASS assignmentexpression .
nodePtr Parser::assignment()
{
    if (!match(Token::ASSIGN))
        error("an assignment (==)");

    return assignmentexpression();
}

// expression  = assignmentexpression .
nodePtr Parser::expression()
{
    return assignmentexpression();
}

// statement = variabledeclaration | simpleStmt | returnStmt |
//              breakStmt | block | ifStmt | gotoStmt | whileStmt .
nodePtr Parser::statement()
{
    nodePtr node(nullptr);

    if (node = variabledeclaration(), node)
    {
    }
    else if (node = simpleStmt(), node)
    {
    }
    else if (node = returnStmt(), node)
    {
    }
    else if (match(Token::BREAK))
    {
        node = makeNode(NType::Stmt, StmtType::Break);
        node->line = m_lexer->line();
    }
    else if (node = block(), node)
    {
    }
    else if (node = ifStmt(), node)
    {
    }
    else if (node = gotoStmt(), node)
    {
    }
    else if (node = whileStmt(), node)
    {
    }

    return std::move(node);
}

// block = OBRCK [ blockstatements ] CBRCK .
// block = OBRCK [ statement { statement } ] CBRCK .
nodePtr Parser::block()
{
    if (!match(Token::OBRCK))
        error("{");

    nodePtr blockNode = 
        makeNode(NType::Stmt, StmtType::Block);

    while (true)
    {
        auto node = statement();
        if (!node)
            break;
        blockNode->children.push_back(std::move(node));
    }

    if (!match(Token::OBRCK))
        error("}");

    return std::move(blockNode);
}

// formalparameterlist = formalparameter { COMMA formalparameter } .
nodePtr Parser::formalparameterlist()
{
    nodePtr params =
        makeNode(NType::Decl, DeclType::Params);

    // formalparameter = type identifier .
    auto formalparameter = [=]()
    {
        nodePtr param(nullptr);
        auto vtype = type();
        if (!vtype.has_value())
            return std::move(param);

        param = makeNode(NType::Decl, DeclType::Param);
        param->val_type = std::move(vtype);
        param->line = m_lexer->line();

        auto id = identifier();
        if (!id)
            error("identifier");

        return std::move(param);
    };

    while (true)
    {
        if (!match(Token::COMMA))
            break;
        auto param = formalparameter();
        if (!param)
            error("a parameter after comma");
        params->children.push_back(std::move(param));
    }

    return std::move(params);
}


// functiondeclarator  = OPAREN [ formalparameterlist ] CPAREN .
nodePtr Parser::functiondeclarator()
{
    if (!match(Token::OPAREN))
        return nodePtr(nullptr);

    nodePtr params = formalparameterlist();

    if (!match(Token::CPAREN))
        error("'('");

    return std::move(params);
}

// functiondeclaration  = functiondeclarator block .
nodePtr Parser::functiondeclaration()
{
    auto node = functiondeclarator();
    if (!node)
        error("function declaration");

    node = block();

    return std::move(node);
}

// mainfunctiondeclaration = id OPAREN CPAREN block .
void Parser::mainfunctiondeclaration()
{
    auto node = identifier();
    if (!node)
        error("identifier");

    m_current_node->kind = DeclType::MFunc;
    m_current_node->line = m_lexer->line();
    m_current_node->val_type = VType::Int;

    // id
    m_current_node->children.push_back(std::move(node));

    if (!match(Token::OPAREN))
        error("'('");

    if (!match(Token::CPAREN))
        m_logger->error(m_lexer->line(),
                ": Main function cannot take any parameters. Expected ')' but found ",
                tokenToStr(m_lexer->peek()));

    node = block();
    m_current_node->children.push_back(std::move(node));
}


// globaldeclaration   = 
//      type identifier ( functiondeclarator | SEMCOL )
//      | VOID identifier functiondeclarator
//      | mainfunctiondeclaration .
nodePtr Parser::globaldeclaration()
{
    m_current_node = makeNode(NType::Decl);

    std::optional<VType> vtype;

    // type id ( functiondeclaration | ; )
    if (vtype = type(), vtype.has_value())
    {
        m_current_node->val_type = std::move(vtype);
        m_current_node->line = m_lexer->line();

        auto node = identifier();
        if (!node)
            error("identifier");

        m_current_node->children.push_back(std::move(node));

        if (node = functiondeclaration(), node)
        {
            m_current_node->kind = DeclType::Func;
            m_current_node->children.push_back(std::move(node));
        }
        else
        {
            m_current_node->kind = DeclType::GVar;
            if (!match(Token::SEMICOL))
                error("semicolon");
        }
    }
    else if (match(Token::VOID)) // void id functiondeclaration
    {
        m_current_node->val_type = VType::Void;
        m_current_node->line = m_lexer->line();

        auto node = identifier();
        if (!node)
            error("identifier");

        m_current_node->children.push_back(std::move(node));

        if (node = functiondeclaration(), !node)
            error("function declaration");

        m_current_node->kind = DeclType::Func;
        m_current_node->children.push_back(std::move(node));
    }
    else if (m_lexer->peek() == Token::ID) // Main function
    {
        mainfunctiondeclaration();
    }
    else 
    {
        m_logger->error(m_lexer->line(), 
                ": Expected either a function or variable declaration.Got: ",
                tokenToStr(m_lexer->peek()));
    }

    return std::move(m_current_node);
}


// start = { globaldeclaration }
void Parser::start()
{
    while (m_lexer->peek() != Token::T_EOF)
        m_ast->children.push_back(std::move(globaldeclaration()));
}
