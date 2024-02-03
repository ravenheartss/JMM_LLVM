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

    return node;

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
            if (match(Token::INC))
            {
                auto toAdd = makeNode(NType::Expr, ExprType::Assign);
                node->kind = ExprType::Unary;
                node->line = m_lexer->line();
                node->op = Token::INC;
                node->children.push_back(temp);

                break;
            }
            else if (match(Token::DEC))
            {
            }
            else
            {
                break;
            }
        }
    }
    else if (temp = identifier(), temp)
    {
        auto invoc = functioninvocation();
        if (invoc)
        {
        }
        else
        {
        }
    }
    else
    {
    }

}

// unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression | postfixexpression .
nodePtr Parser::unaryexpression()
{
}

// multiplicativeexpression = unaryexpression { mult_op unaryexpression } .
nodePtr Parser::multiplicativeexpression()
{
}

// additiveexpression = multiplicativeexpression { add_op multiplicativeexpression } .
nodePtr Parser::additiveexpression()
{
}

// shiftexpression = additiveexpression { shift_op additiveexpression } .
nodePtr Parser::shiftexpression()
{
}

// relationalexpression = shiftexpression { rel_op shiftexpression } .
nodePtr Parser::relationalexpression()
{
}

// equalityexpression = relationalexpression { eq_op relationalexpression } .
nodePtr Parser::equalityexpression()
{
}

// bitwiseandexpression = equalityexpression { BAND equalityexpression } .
nodePtr Parser::bitwiseandexpression()
{
}

// exclusiveorexpression = bitwiseandexpression { XOR bitwiseandexpression } .
nodePtr Parser::xorexpression()
{
}

// bitwiseorexpreession = exclusiveorexpression { BOR exclusiveorexpression } .
nodePtr Parser::bitwiseorexpression()
{
}


// conditionalandexpression = bitwiseorexpreession { LAND bitwiseorexpreession } .
nodePtr Parser::conditionalandexpression()
{
}

// conditionalorexpression = conditionalandexpression { LOR conditionalandexpression } .
nodePtr Parser::conditionalorexpression()
{
}

// assignmentexpression    = conditionalorexpression { ASS conditionalorexpression } .
nodePtr Parser::assignmentexpression()
{
}

// assignment  = ASS assignmentexpression .
nodePtr Parser::assignment()
{
}

// expression  = assignmentexpression .
nodePtr Parser::expression()
{
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
