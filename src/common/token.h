#ifndef JMM_TOKEN_H
#define JMM_TOKEN_H

enum class Token
{
    T_EOF,
    ID,
    NUM,
    STRLIT,
    STR,
    TRUE,
    FALSE,
    BOOL,
    INT,
    VOID,
    IF,
    ELSE,
    WHILE,
    BREAK,
    RETURN,
    OPAREN,
    CPAREN,
    OBRCK,
    CBRCK,
    MINUS,
    PLUS,
    MULT,
    DIV,
    MOD,
    XOR,
    GT,
    LT,
    GTEQ,
    LTEQ,
    EQ,
    NOT,
    NEQ,
    ASSIGN,
    LAND,
    LOR,
    BAND,
    BOR,
    COMMA,
    SEMICOL,
    OSQBRCK,
    CSQBRCK
};

const char* tokenToStr(Token tok);

#endif // !JMM_TOKEN_H
