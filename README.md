# J-- LLVM
All compilers for J-- using LLVM (a subset of Java)

The language was designed for CPSC 411 - Compiler Construction at the University of Calgary by Dr. John Aycock.
I was a TA for the course and implemented this alongside my students.

While the students didn't use LLVM, I decided to use it since I had already implemeted the compiler for the course when I took it but to a different target langauge (WebAssembly Text).
It also gave me an opportunity to learn and work with LLVM.

-------------

## Specifications:

Data types. int, boolean and strings are supported.

Strings. String variables are constant and cannot be changed.

Functions. Functions may take arguments, and must return either int, boolean, void or a string. Functions may be recursive. You may not have functions nested inside each other.

Expressions. The usual operations: +, -, *, /, %, <, >, <=, >=, =, ==, !=, !, &&, ||, and unary minus. Java precedence and associativity rules apply, but can be modified with the use of parentheses. Logical AND (&&) and OR (/||/) are short-circuiting. Added: Bitwise and (&), or (|), xor (^) and icrementing/decrementing (++/--).

Statements. break, return, if, if-else, and while.

Strong type checking. No casting.


No bitwise and, bitwise or, string type, XOR, bitwise shifts, incrementing/decrementing in the original lang but hey why not! They're easy to add.
I've only removed the null character which is supported and preserved in strings that the students had to include.

I have not made any big changes to the language as this was mainly implemented so I can help my students better on issues that they would run into and for me to learn LLVM. There hasn't been any big changes to the grammar either.


--------------

### Lexer

* Single line comments only -- `//`
* Identifiers -- start with `_` or alphabet followed by alphanumric or underscores
* String literals -- Only escapes `\b, \f, \t, \r, \n, \', \", \\` supported
* Integer literals -- base ten only
* Reserved words -- `true, false, boolean, int, void, if, else, while, break, return, str`
* Operators: `+, -, *, /, %, ^, <, >, <=, >=, =, ==, !=, !, &&, ||, &, |, ++, --`
* Parentheses
* Braces
* Semicolon
* Comma

--------------

### Parser 

<details>
<summary><b>Original J-- Grammar</b></summary>

```yacc
start           : /* empty */
                | globaldeclarations
                ;

literal         : NUMBER
                | STRING
                | TRUE
                | FALSE
                ;

type            : BOOLEAN
                | INT
                ;

globaldeclarations      : globaldeclaration
                        | globaldeclarations globaldeclaration
                        ;

globaldeclaration       : variabledeclaration
                        | functiondeclaration
                        | mainfunctiondeclaration
                        ;

variabledeclaration     : type identifier ';'
                        ;

identifier              : ID
                        ;

functiondeclaration     : functionheader block
                        ;

functionheader          : type functiondeclarator
                        | VOID functiondeclarator
                        ;

functiondeclarator      : identifier '(' formalparameterlist ')'
                        | identifier '(' ')'
                        ;

formalparameterlist     : formalparameter
                        | formalparameterlist ',' formalparameter
                        ;

formalparameter         : type identifier
                        ;

mainfunctiondeclaration : mainfunctiondeclarator block
                        ;

mainfunctiondeclarator  : identifier '(' ')'
                        ;

block                   : '{' blockstatements '}'
                        | '{' '}'
                        ;

blockstatements         : blockstatement
                        | blockstatements blockstatement
                        ;

blockstatement          : variabledeclaration
                        | statement
                        ;

statement               : block
                        | ';'
                        | statementexpression ';'
                        | BREAK ';'
                        | RETURN expression ';'
                        | RETURN ';'
                        | IF '(' expression ')' statement
                        | IF '(' expression ')' statement ELSE statement
                        | WHILE '(' expression ')' statement
                        ;

statementexpression     : assignment
                        | functioninvocation
                        ;

primary                 : literal
                        | '(' expression ')'
                        | functioninvocation
                        ;

argumentlist            : expression
                        | argumentlist ',' expression
                        ;

functioninvocation      : identifier '(' argumentlist ')'
                        | identifier '(' ')'
                        ;

postfixexpression       : primary
                        | identifier
                        ;

unaryexpression         : '-' unaryexpression
                        | '!' unaryexpression
                        | postfixexpression
                        ;

multiplicativeexpression: unaryexpression
                        | multiplicativeexpression '*' unaryexpression
                        | multiplicativeexpression '/' unaryexpression
                        | multiplicativeexpression '% ' unaryexpression
                        ;

additiveexpression      : multiplicativeexpression
                        | additiveexpression '+' multiplicativeexpression
                        | additiveexpression '-' multiplicativeexpression
                        ;

relationalexpression    : additiveexpression
                        | relationalexpression '<' additiveexpression
                        | relationalexpression '>' additiveexpression
                        | relationalexpression LE additiveexpression
                        | relationalexpression GE additiveexpression
                        ;

equalityexpression      : relationalexpression
                        | equalityexpression EQ relationalexpression
                        | equalityexpression NE relationalexpression
                        ;

conditionalandexpression: equalityexpression
                        | conditionalandexpression AND equalityexpression
                        ;

conditionalorexpression : conditionalandexpression
                        | conditionalorexpression OR conditionalandexpression
                        ;

assignmentexpression    : conditionalorexpression
                        | assignment
                        ;

assignment              : identifier '=' assignmentexpression
                        ;

expression              : assignmentexpression
                        ;
```

</details>

<details>
<summary><b>J-- Grammar w/ added features</b></summary>

```yacc
start           : /* empty */
                | globaldeclarations
                ;

literal         : NUMBER
                | STRING
                | TRUE
                | FALSE
                ;

type            : BOOLEAN
                | INT
                | STR
                ;

globaldeclarations      : globaldeclaration
                        | globaldeclarations globaldeclaration
                        ;

globaldeclaration       : variabledeclaration
                        | functiondeclaration
                        | mainfunctiondeclaration
                        ;

variabledeclaration     : type identifier ';'
                        ;

identifier              : ID
                        ;

functiondeclaration     : functionheader block
                        ;

functionheader          : type functiondeclarator
                        | VOID functiondeclarator
                        ;

functiondeclarator      : identifier '(' formalparameterlist ')'
                        | identifier '(' ')'
                        ;

formalparameterlist     : formalparameter
                        | formalparameterlist ',' formalparameter
                        ;

formalparameter         : type identifier
                        ;

mainfunctiondeclaration : mainfunctiondeclarator block
                        ;

mainfunctiondeclarator  : identifier '(' ')'
                        ;

block                   : '{' blockstatements '}'
                        | '{' '}'
                        ;

blockstatements         : blockstatement
                        | blockstatements blockstatement
                        ;

blockstatement          : variabledeclaration
                        | statement
                        ;

statement               : block
                        | ';'
                        | statementexpression ';'
                        | BREAK ';'
                        | RETURN expression ';'
                        | RETURN ';'
                        | IF '(' expression ')' statement
                        | IF '(' expression ')' statement ELSE statement
                        | WHILE '(' expression ')' statement
                        ;

statementexpression     : assignment
                        | functioninvocation
                        ;

primary                 : literal
                        | '(' expression ')'
                        | functioninvocation
                        ;

argumentlist            : expression
                        | argumentlist ',' expression
                        ;

functioninvocation      : identifier '(' argumentlist ')'
                        | identifier '(' ')'
                        ;

postfixexpression       : primary
                        | identifier
                        | postfixexpression INC
                        | postfixexpression DEC
                        ;

unaryexpression         : '-' unaryexpression
                        | '!' unaryexpression
                        | INC unaryexpression
                        | DEC unaryexpression
                        | postfixexpression
                        ;

multiplicativeexpression: unaryexpression
                        | multiplicativeexpression '*' unaryexpression
                        | multiplicativeexpression '/' unaryexpression
                        | multiplicativeexpression '% ' unaryexpression
                        ;

additiveexpression      : multiplicativeexpression
                        | additiveexpression '+' multiplicativeexpression
                        | additiveexpression '-' multiplicativeexpression
                        ;

shiftexpression         : additiveexpression
                        | shiftexpression LSHIFT additiveexpression
                        | shiftexpression RSHIFT additiveexpression
                        ;

relationalexpression    : shiftexpression
                        | relationalexpression '<' shiftexpression
                        | relationalexpression '>' shiftexpression
                        | relationalexpression LE shiftexpression
                        | relationalexpression GE shiftexpression
                        ;

equalityexpression      : relationalexpression
                        | equalityexpression EQ relationalexpression
                        | equalityexpression NE relationalexpression
                        ;

bitwiseandexpression    : equalityexpression
                        | bitwiseandexpression '&' equalityexpression
                        ;

exclusiveorexpression   : bitwiseandexpression
                        | exclusiveorexpression '^' bitwiseandexpression
                        ;

bitwiseorexpreession    : exclusiveorexpression
                        | bitwiseorexpreession '|' exclusiveorexpression
                        ;

conditionalandexpression: bitwiseorexpreession
                        | conditionalandexpression LAND bitwiseorexpreession
                        ;

conditionalorexpression : conditionalandexpression
                        | conditionalorexpression LOR conditionalandexpression
                        ;

assignmentexpression    : conditionalorexpression
                        | assignment
                        ;

assignment              : identifier '=' assignmentexpression
                        ;

expression              : assignmentexpression
                        ;
```
</details>
