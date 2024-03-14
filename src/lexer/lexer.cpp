
#include "lexer.h"
#include "common/token.h"
#include "common/errwarn.h"

#include <cctype>
#include <fstream>
#include <memory>
#include <cstring>
#include <unordered_map>

Lexer::Lexer(std::string_view filename, std::shared_ptr<Logger> &logger)
    : m_curr_lexeme()
    , m_consumed(true)
    , m_lineno(1)
    , m_filename(filename)
    , m_logger(logger)
{
    m_input = std::ifstream(std::string(filename));
    if (!m_input.is_open())
        m_logger->error("Failed to open file.\n", std::strerror(errno));
}

Lexer::~Lexer()
{
    m_curr_lexeme.clear();
    m_input.close();
    m_logger.reset();
}

Token Lexer::peek()
{
    if (!m_consumed)
        return m_curr_token;

    lex();
    m_consumed = false;

#ifdef LEXER_DEBUG
    fprintf(stderr, "Line: %d \t Token: %s \t Lexeme: %s\n", m_lineno, tokenToStr(m_curr_token),
            m_curr_lexeme.c_str());
#endif
    return m_curr_token;
}

Token Lexer::consume()
{
    m_consumed = true;
    return m_curr_token;
}

void Lexer::lex()
{
    while (!m_input.eof())
    {
        m_curr_lexeme.clear();
        char c = m_input.peek();
        if (c == '\n')
        {
            m_input.get();
            m_lineno++;
            continue;
        }
        else if (isspace(c))
        {
            m_input.get();
            continue;
        }
        else if (isalpha(c) || c == '_')
        {
            isIdentifier();
            return;
        }
        else if (isdigit(c))
        {
            isNumeric();
            return;
        }
        else if (c == '"')
        {
            isStr();
            return;
        }
        else if (c == '/') // Check if it's a comment first 
        {
            m_input.get();
            if (m_input.peek() == '/')
            {
                while (m_input.peek() != '\n') m_input.get();
            }
            else
            {
                // m_curr_token = Token::DIV;      break;
                m_input.unget();
                isOperator(); // If not it's an operator
                return;
            }
        }
        else if (isSpecial())
        {
            return;
        }
        else if (isOperator())
        {
            return;
        }
        else if (c == EOF)
        {
            m_curr_token = Token::T_EOF;
            return;
        }
        else
        {
            m_logger->warning("Ignoring unknown character ", c, "at line ", m_lineno);
            m_input.get();
        }
    }

    m_curr_token = Token::T_EOF;
    return;
}

void Lexer::isIdentifier()
{
    m_curr_lexeme.push_back(m_input.get());
    char c = m_input.peek();
    while (isalnum(c) || c == '_')
    {
        m_curr_lexeme.push_back(m_input.get());
        c = m_input.peek();
    }
    m_curr_token = Token::ID;
    isReserved();
}

void Lexer::isReserved()
{
    static const std::unordered_map<std::string, Token> reserved_words(
            {
            {"true",    Token::TRUE},
            {"false",   Token::FALSE},
            {"boolean", Token::BOOL},
            {"int",     Token::INT},
            {"void",    Token::VOID},
            {"if",      Token::IF},
            {"else",    Token::ELSE},
            {"while",   Token::WHILE},
            {"break",   Token::BREAK},
            {"return",  Token::RETURN},
            {"str",     Token::STR},
            {"goto",    Token::GOTO}
            });

    if (auto it = reserved_words.find(m_curr_lexeme) ; it != reserved_words.end())
    {
        m_curr_token = it->second;
        m_curr_lexeme.clear();
    }
}

void Lexer::isNumeric()
{
    char c = m_input.peek();
    while (isdigit(c))
    {
        m_curr_lexeme.push_back(m_input.get());
        c = m_input.peek();
    }
    m_curr_token = Token::NUM;
}

void Lexer::isStr()
{
    m_curr_token = Token::STRLIT;
    m_input.get(); // Opening quote
    char c = m_input.get();
    while (c != '"')
    {
        if (c == '\\')
        {
            m_curr_lexeme.push_back('\\');
            c = m_input.get();
            switch (c) 
            {
                case 'b':
                    m_curr_lexeme.push_back('b');
                    break;
                case 'f':
                    m_curr_lexeme.push_back('f');
                    break;
                case 't':
                    m_curr_lexeme.push_back('t');
                    break;
                case 'r':
                    m_curr_lexeme.push_back('r');
                    break;
                case 'n':
                    m_curr_lexeme.push_back('n');
                    break;
                case '\'':
                    m_curr_lexeme.push_back('\'');
                    break;
                case '"':
                    m_curr_lexeme.push_back('"');
                    break;
                case '\\':
                    m_curr_lexeme.push_back('\\');
                    break;
                default:
                    m_logger->warning("Ignoring bad escape char at ", m_lineno);
                    m_curr_lexeme.pop_back();
                    break;
            }
        }
        else if (c == '\n')
        {
            m_logger->warning("Strings cannot contain newlines at ", m_lineno);
        }
        else 
        {
            m_curr_lexeme.push_back(c);
        }

        c = m_input.get();
    }
}

bool Lexer::isOperator()
{
    auto c = m_input.get();
    switch (c) 
    {
        case '+':
            m_curr_token = Token::PLUS;
            if (m_input.peek() == '+')
            {
                m_curr_token = Token::INC;
                m_input.get();
            }
            break;
        case '-':   
            m_curr_token = Token::MINUS;
            if (m_input.peek() == '-')
            {
                m_curr_token = Token::DEC;
                m_input.get();
            }
            break;
        case '/':   m_curr_token = Token::DIV;      break;
        case '*':   m_curr_token = Token::MULT;     break;
        case '%':   m_curr_token = Token::MOD;      break;
        case '^':   m_curr_token = Token::XOR;      break;
        case '<':
            m_curr_token = Token::LT;
            if (m_input.peek() == '=')
            {
                m_curr_token = Token::LTEQ;
                m_input.get();
            }
            else if (m_input.peek() == '<')
            {
                m_curr_token = Token::LSHIFT;
                m_input.get();
            }
            break;
        case '>':
            m_curr_token = Token::GT;
            if (m_input.peek() == '=')
            {
                m_curr_token = Token::GTEQ;
                m_input.get();
            }
            else if (m_input.peek() == '>')
            {
                m_curr_token = Token::RSHIFT;
                m_input.get();
            }
            break;
        case '=':
            m_curr_token = Token::ASSIGN;
            if (m_input.peek() == '=')
            {
                m_curr_token = Token::EQ;
                m_input.get();
            }
            break;
        case '!':
            m_curr_token = Token::NOT;
            if (m_input.peek() == '=')
            {
                m_curr_token = Token::NEQ;
                m_input.get();
            }
            break;
        case '&':
            m_curr_token = Token::BAND;
            if (m_input.peek() == '&')
            {
                m_curr_token = Token::LAND;
                m_input.get();
            }
            break;
        case '|':
            m_curr_token = Token::BOR;
            if (m_input.peek() == '|')
            {
                m_curr_token = Token::LOR;
                m_input.get();
            }
            break;
        default:
            m_input.unget();
            return false;
    }

    return true;
}

bool Lexer::isSpecial()
{
    switch (m_input.get()) 
    {
        case '(':   m_curr_token = Token::OPAREN;   break;
        case ')':   m_curr_token = Token::CPAREN;   break;
        case '{':   m_curr_token = Token::OBRCK;    break;
        case '}':   m_curr_token = Token::CBRCK;    break;
        case ';':   m_curr_token = Token::SEMICOL;  break;
        case ',':   m_curr_token = Token::COMMA;    break;
        default:    m_input.unget();         return false;
    }

    return true;
}

