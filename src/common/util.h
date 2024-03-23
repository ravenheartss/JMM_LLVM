#ifndef JMM_UTIL_H
#define JMM_UTIL_H

#include "globals.h"
#include <ostream>

char const* tokenToStr(Token tok);

std::ostream& operator<<(std::ostream& out, VType value_type);
std::ostream& operator<<(std::ostream& out, Op oper);
std::ostream& operator<<(std::ostream& out, Token tok);

#endif  // !JMM_UTIL_H
