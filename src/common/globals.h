#ifndef JMM_GLOBALS_H
#define JMM_GLOBALS_H

// Enums
#include <cstdint>

enum class Token {
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
  INC,
  DEC,
  MULT,
  DIV,
  MOD,
  XOR,
  GT,
  LT,
  GTEQ,
  LTEQ,
  LSHIFT,
  RSHIFT,
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
};

enum class VType : uint8_t { Int, Str, Bool, Void };

enum class Op : uint8_t {
  BAND,
  BOR,
  LAND,
  LOR,
  XOR,
  ADD,
  POSTINC,
  POSTDEC,
  PREINC,
  PREDEC,
  SUB,
  MULT,
  DIV,
  MOD,
  NOT,
  EQ,
  NE,
  GT,
  LT,
  GE,
  LE,
  LSHIFT,
  RSHIFT
};

#endif  //! JMM_GLOBALS_H
