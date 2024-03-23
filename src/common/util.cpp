#include "util.h"

char const* tokenToStr(Token tok) {
  switch (tok) {
    case Token::T_EOF:
      return "EOF";
    case Token::ID:
      return "ID";
    case Token::NUM:
      return "NUM";
    case Token::STRLIT:
      return "STR";
    case Token::STR:
      return "str";
    case Token::TRUE:
      return "true";
    case Token::FALSE:
      return "false";
    case Token::BOOL:
      return "bool";
    case Token::INT:
      return "int";
    case Token::VOID:
      return "void";
    case Token::IF:
      return "if";
    case Token::ELSE:
      return "else";
    case Token::WHILE:
      return "while";
    case Token::BREAK:
      return "break";
    case Token::RETURN:
      return "return";
    case Token::GOTO:
      return "goto";
    case Token::OPAREN:
      return "(";
    case Token::CPAREN:
      return ")";
    case Token::OBRCK:
      return "{";
    case Token::CBRCK:
      return "}";
    case Token::MINUS:
      return "-";
    case Token::PLUS:
      return "+";
    case Token::INC:
      return "++";
    case Token::DEC:
      return "--";
    case Token::MULT:
      return "*";
    case Token::DIV:
      return "/";
    case Token::MOD:
      return "%";
    case Token::XOR:
      return "^";
    case Token::GT:
      return ">";
    case Token::LT:
      return "<";
    case Token::GTEQ:
      return ">=";
    case Token::LTEQ:
      return "<=";
    case Token::EQ:
      return "==";
    case Token::NOT:
      return "!";
    case Token::NEQ:
      return "!=";
    case Token::ASSIGN:
      return "=";
    case Token::LAND:
      return "&&";
    case Token::LOR:
      return "||";
    case Token::BAND:
      return "&";
    case Token::BOR:
      return "|";
    case Token::COMMA:
      return ",";
    case Token::SEMICOL:
      return ";";
    default:
      return "NONE";
  }
}

std::ostream& operator<<(std::ostream& out, VType value_type) {
  switch (value_type) {
    case VType::Int:
      return out << "Int";
    case VType::Str:
      return out << "Str";
    case VType::Bool:
      return out << "Bool";
    case VType::Void:
      return out << "Void";
    default:
      return out;
  }
}

std::ostream& operator<<(std::ostream& out, Op oper) {
  switch (oper) {
    case Op::BAND:
      return out << "&";
    case Op::BOR:
      return out << "|";
    case Op::LAND:
      return out << "&&";
    case Op::LOR:
      return out << "||";
    case Op::XOR:
      return out << "^";
    case Op::ADD:
      return out << "+";
    case Op::POSTINC:
      return out << "Post ++";
    case Op::POSTDEC:
      return out << "Post --";
    case Op::PREINC:
      return out << "Pre ++";
    case Op::PREDEC:
      return out << "Pre --";
    case Op::SUB:
      return out << "-";
    case Op::MULT:
      return out << "*";
    case Op::DIV:
      return out << "/";
    case Op::MOD:
      return out << "%";
    case Op::NOT:
      return out << "!";
    case Op::EQ:
      return out << "==";
    case Op::NE:
      return out << "!=";
    case Op::GT:
      return out << ">";
    case Op::LT:
      return out << "<";
    case Op::GE:
      return out << ">=";
    case Op::LE:
      return out << "<=";
    case Op::LSHIFT:
      return out << "<<";
    case Op::RSHIFT:
      return out << ">>";
    default:
      return out;
  }
}

std::ostream& operator<<(std::ostream& out, Token tok) {
  switch (tok) {
    case Token::T_EOF:
      return out << "EOF";
    case Token::ID:
      return out << "ID";
    case Token::NUM:
      return out << "NUM";
    case Token::STRLIT:
      return out << "STR";
    case Token::STR:
      return out << "str";
    case Token::TRUE:
      return out << "true";
    case Token::FALSE:
      return out << "false";
    case Token::BOOL:
      return out << "bool";
    case Token::INT:
      return out << "int";
    case Token::VOID:
      return out << "void";
    case Token::IF:
      return out << "if";
    case Token::ELSE:
      return out << "else";
    case Token::WHILE:
      return out << "while";
    case Token::BREAK:
      return out << "break";
    case Token::RETURN:
      return out << "return";
    case Token::GOTO:
      return out << "goto";
    case Token::OPAREN:
      return out << "(";
    case Token::CPAREN:
      return out << ")";
    case Token::OBRCK:
      return out << "{";
    case Token::CBRCK:
      return out << "}";
    case Token::MINUS:
      return out << "-";
    case Token::PLUS:
      return out << "+";
    case Token::INC:
      return out << "++";
    case Token::DEC:
      return out << "--";
    case Token::MULT:
      return out << "*";
    case Token::DIV:
      return out << "/";
    case Token::MOD:
      return out << "%";
    case Token::XOR:
      return out << "^";
    case Token::GT:
      return out << ">";
    case Token::LT:
      return out << "<";
    case Token::GTEQ:
      return out << ">=";
    case Token::LTEQ:
      return out << "<=";
    case Token::EQ:
      return out << "==";
    case Token::NOT:
      return out << "!";
    case Token::NEQ:
      return out << "!=";
    case Token::ASSIGN:
      return out << "=";
    case Token::LAND:
      return out << "&&";
    case Token::LOR:
      return out << "||";
    case Token::BAND:
      return out << "&";
    case Token::BOR:
      return out << "|";
    case Token::COMMA:
      return out << ",";
    case Token::SEMICOL:
      return out << ";";
    default:
      return out << "NONE";
  }
}
