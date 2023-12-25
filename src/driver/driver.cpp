
#include "driver.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include "common/errwarn.h"
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
}


bool Driver::compile()
{
    return m_parser->parse();
}
