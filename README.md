# J-- LLVM
A compiler for J-- using LLVM (a subset of Java)

The languae was designed for CPSC 411 - Compiler Construction at the University of Calgary by Dr. John Aycock.
I was a TA for the course and implemented this alongside my students.

While the students didn't use LLVM, I decided to use it since I had already impleneted the compiler for the course when I took it.
It also gave me an oppurtunity to work with LLVM.

-------------

## Specifications:

### Lexer

Single line comments only -- `//`
Identifiers -- start with `_` or alphabet followed by alphanumric or underscores
String literals -- Only escapes `\b, \f, \t, \r, \n, \', \", \\` supported
Integer literals -- base ten only
Reserved words -- `true, false, boolean, int, void, if, else, while, break, return, str`
Operators: `+, -, *, /, %, ^, <, >, <=, >=, =, ==, !=, !, &&, ||, &, |`
Parentheses
Braces
Semicolon
Comma
Square brackets for arrays

No arrays, bitwise and, bitwise or, string type or XOR in the original lang but hey why not!
I've only removed the null character which is supported and preserved in strings.


Design: peek/match interface.
