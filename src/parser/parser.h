#ifndef JMM_PARSER_H
#define JMM_PARSER_H

#include "common/ast.h"
#include "common/jmm.h"
#include "common/errwarn.h"
#include <memory>

class Parser 
{
    using nodePtr = std::shared_ptr<ASTNode>;
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

        // Current global declaration
        std::shared_ptr<ASTNode>    m_current_node; 

        bool match(Token expected);

        [[noreturn]] void error(std::string expected);

        void start();
        std::optional<VType> type();
        nodePtr identifier();
        nodePtr block();
        nodePtr blockstatements();
        nodePtr functiondeclaration();
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
        nodePtr argumentlist();
        nodePtr functioninvocation();
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
        nodePtr formalparameterlist();
        nodePtr functiondeclarator();
        void mainfunctiondeclaration();
        nodePtr globaldeclaration(); 

};

#endif // !JMM_PARSER_H
