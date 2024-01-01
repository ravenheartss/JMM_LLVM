#ifndef JMM_PARSER_H
#define JMM_PARSER_H

#include "common/ast.h"
#include "common/jmm.h"
#include "common/errwarn.h"
#include <memory>

class Parser 
{
    public:
        Parser(std::shared_ptr<Lexer>& lexer, std::shared_ptr<Logger>& logger) 
            : m_lexer(lexer), m_logger(logger) { }
        ~Parser();

        bool parse();

        std::unique_ptr<ASTNode> getAST() { return std::move(m_ast); }

    private:
        std::shared_ptr<Lexer>      m_lexer;
        std::unique_ptr<ASTNode>    m_ast;
        std::shared_ptr<Logger>     m_logger;

        // void start();

};

#endif // !JMM_PARSER_H
