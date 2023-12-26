# J-- LLVM
A compiler for J-- using LLVM (a subset of Java)

The languae was designed for CPSC 411 - Compiler Construction at the University of Calgary by Dr. John Aycock.
I was a TA for the course and implemented this alongside my students.

While the students didn't use LLVM, I decided to use it since I had already impleneted the compiler for the course when I took it.
It also gave me an oppurtunity to work with LLVM.

-------------

## Specifications:

Data types. int, boolean and strings are supported.
Strings. String variables are constant and cannot be changed.
Functions. Functions may take arguments, and must return either int, boolean, void or a string. Functions may be recursive. You may not have functions nested inside each other.
Expressions. The usual operations: +, -, *, /, %, <, >, <=, >=, =, ==, !=, !, &&, ||, and unary minus. Java precedence and associativity rules apply, but can be modified with the use of parentheses. Logical AND (&&) and OR (/||/) are short-circuiting. Added: Bitwise and (&), or (|), xor (^) and icrementing/decrementing (++/--).
Statements. break, return, if, if-else, and while. Added: goto
Strong type checking. No casting.


No bitwise and, bitwise or, string type, XOR, bitwise shifts, incrementing/decrementing or goto in the original lang but hey why not! They're easy to add.
I've only removed the null character which is supported and preserved in strings that the students had to include.

I have not made any big changes to the language as this was mainly implemented so I can help my students better on issues that they would run into. There hasn't been any big changes to the grammar either. Initially thought of adding arrays but decided not to as I would end up changing the grammar quite a bit.

### Lexer

Single line comments only -- `//`
Identifiers -- start with `_` or alphabet followed by alphanumric or underscores
String literals -- Only escapes `\b, \f, \t, \r, \n, \', \", \\` supported
Integer literals -- base ten only
Reserved words -- `true, false, boolean, int, void, if, else, while, break, return, str, goto`
Operators: `+, -, *, /, %, ^, <, >, <=, >=, =, ==, !=, !, &&, ||, &, |, ++, --`
Parentheses
Braces
Semicolon
Comma
