#include "token.h"

const char* tokenToStr(Token tok)
{
    switch (tok)
    {
        case Token::T_EOF:     return "EOF";
        case Token::ID:        return "ID";
        case Token::NUM:       return "NUM";
        case Token::STRLIT:    return "STR";
        case Token::STR:       return "str";
        case Token::TRUE:      return "true";
        case Token::FALSE:     return "false";
        case Token::BOOL:      return "bool";
        case Token::INT:       return "int";
        case Token::VOID:      return "void";
        case Token::IF:        return "if";
        case Token::ELSE:      return "else";
        case Token::WHILE:     return "while";
        case Token::BREAK:     return "break";
        case Token::RETURN:    return "return";
        case Token::OPAREN:    return "(";
        case Token::CPAREN:    return ")";
        case Token::OBRCK:     return "{";
        case Token::CBRCK:     return "}";
        case Token::MINUS:     return "-";
        case Token::PLUS:      return "+";
        case Token::MULT:      return "*";
        case Token::DIV:       return "/";
        case Token::MOD:       return "%";
        case Token::XOR:       return "^";
        case Token::GT:        return ">";
        case Token::LT:        return "<";
        case Token::GTEQ:      return ">=";
        case Token::LTEQ:      return "<=";
        case Token::EQ:        return "==";
        case Token::NOT:       return "!";
        case Token::NEQ:       return "!=";
        case Token::ASSIGN:    return "=";
        case Token::LAND:      return "&&";
        case Token::LOR:       return "||";
        case Token::BAND:      return "&";
        case Token::BOR:       return "|";
        case Token::COMMA:     return ",";
        case Token::SEMICOL:   return ";";
        case Token::OSQBRCK:   return "[";
        case Token::CSQBRCK:   return "]";
        default:               return "NONE";
    }
}
