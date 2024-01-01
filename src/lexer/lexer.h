// license: MIT
// copyright: Shankar Ganesh

#include "common/jmm.h"
#include "common/errwarn.h"
#include "common/token.h"
#include <string>
#include <fstream>

class Lexer 
{
    public:
        Lexer(std::string_view filename, std::shared_ptr<Logger> &logger);
        ~Lexer();

        Token peek();
        Token consume();

        const std::string& lexeme() { return {m_curr_lexeme}; };
        uint32_t line() { return m_lineno; }

    private:
        void lex();
        void isIdentifier();
        void isReserved();
        void isNumeric();
        void isStr();
        bool isOperator();
        bool isSpecial();

    private:
        std::string             m_curr_lexeme;
        Token                   m_curr_token;
        bool                    m_consumed;
        uint32_t                m_lineno;

        std::string_view        m_filename;
        std::ifstream           m_input;

        std::shared_ptr<Logger> m_logger;
};
