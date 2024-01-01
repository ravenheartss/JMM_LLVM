#include "parser.h"
#include "common/jmm.h"
#include "lexer/lexer.h"
#include "common/token.h"

Parser::~Parser()
{
    m_logger.reset();
    m_lexer.reset();
    m_ast.reset();
}

bool Parser::parse()
{
    // start();
    Token tok;
    while (true)
    {
        tok = m_lexer->peek();
        m_lexer->consume();
        if (tok == Token::T_EOF) break;
    }

    return true;
}
