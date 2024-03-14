
#include "driver.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "common/errwarn.h"
#include "common/ast.h"
#include <memory>


Driver::Driver(std::string file) : filename(file)
{
    m_logger = std::make_shared<Logger>();
    m_lexer = std::make_shared<Lexer>(filename, m_logger);
    m_parser = std::make_unique<Parser>(m_lexer, m_logger);
}

Driver::~Driver()
{
    m_parser.reset();
    m_lexer.reset();
    m_logger.reset();
    filename.clear();
    m_ast.reset();
}


bool Driver::compile()
{
    int err = m_parser->parse();
    if (err)
    {
        m_ast = m_parser->getAST();
        m_ast->print(0);
    }

    return err;
}
