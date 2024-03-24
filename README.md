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

Statements. break, return, if, if-else, and while.

Strong type checking. No casting.


No bitwise and, bitwise or, string type, XOR, bitwise shifts, incrementing/decrementing in the original lang but hey why not! They're easy to add.
I've only removed the null character which is supported and preserved in strings that the students had to include.

I have not made any big changes to the language as this was mainly implemented so I can help my students better on issues that they would run into. There hasn't been any big changes to the grammar either.


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

#### LL(1) J-- Grammar:

Okay, before everyone starts yelling: No, the grammar is not really LL(1). And it cannot be converted to one!!! The dangling else problem! However, it is easy to parse the grammar using a predictive parser (recursive descent without backtracking).

Neither the original grammar above nor the one below is LL(1). It cannot be converted to LL(1) with all of the rules intact.
The assignment expression will have to take a hit here, which is fine. Since there are two rules that lead to the same non-terminal (`ID`) from `assignmentexpression`, there is a clash in the parse table. So, it is not possible to build a predictive parser for a grammar that is not LL(1). Making it LL(1) would allow incorrect assignment expressions. This is perfectly fine and will be caught in semantic analysis.


<details>
<summary<b>Grammar in EBNF (Predictive Parsing)</b></summary>

```EBNF
start   = { globaldeclaration } .

literal = NUMBER | STRING | TRUE | FALSE .

type    = BOOLEAN | INT | STR .

mult_op = MULT | DIV | MOD .

add_op = PLUS | MINUS .

shift_op = LSHIFT | RSHIFT .

rel_op = LT | GT | LE | GE .

eq_op = EQ | NE .

globaldeclaration   = type identifier ( functiondeclaration | SEMCOL ) .
                    | VOID identifier functiondeclaration
                    | mainfunctiondeclaration .

variabledeclaration = type identifier SEMCOL

identifier  = ID .

functiondeclaration = functiondeclarator block .

functiondeclarator  = OPAREN [ formalparameterlist ] CPAREN .

formalparameterlist = formalparameter { COMMA formalparameter } .

formalparameter = type identifier .

mainfunctiondeclaration = mainfunctiondeclarator block .

mainfunctiondeclarator  = identifier OPAREN CPAREN .

block   = OBRCK [ blockstatements ] CBRCK .

blockstatements = statement { statement } .

statement   = variabledeclaration | simpleStmt | returnStmt |
                breakStmt | block | ifStmt | whileStmt .

simpleStmt  = nullStmt | exprStmt .

nullStmt    = SEMCOL .

exprStmt    = identifier ( assignment | functioninvocation ) SEMCOL .

returnStmt  = RETURN [ expression ] SEMCOL .

breakStmt   = BREAK SEMCOL .

ifStmt      = IF OPAREN expression CPAREN statement [ ELSE statement ] .

whileStmt   = WHILE OPAREN expression CPAREN statement .

primary     = literal | OPAREN expression CPAREN .

argumentlist    = expression { COMMA expression } .

functioninvocation  = OPAREN [ argumentlist ] CPAREN .

postfixexpression   = primary postfixexpression1
                    | identifier [ functioninvocation ] postfixexpression1 .

postfixexpression1  = [ ( INC | DEC ) [ postfixexpression1 ] ] .

unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression | postfixexpression .

multiplicativeexpression    = unaryexpression { mult_op unaryexpression } .

additiveexpression  = multiplicativeexpression { add_op multiplicativeexpression } .

shiftexpression = additiveexpression { shift_op additiveexpression } .

relationalexpression    = shiftexpression { rel_op shiftexpression } .

equalityexpression  = relationalexpression { eq_op relationalexpression } .

bitwiseandexpression    = equalityexpression { BAND equalityexpression } .

exclusiveorexpression   = bitwiseandexpression { XOR bitwiseandexpression } .

bitwiseorexpreession    = exclusiveorexpression { BOR exclusiveorexpression } .

conditionalandexpression = bitwiseorexpreession { LAND bitwiseorexpreession } .

conditionalorexpression = conditionalandexpression { LOR conditionalandexpression } .

assignmentexpression    = conditionalorexpression { ASS conditionalorexpression } .

assignment  = ASS assignmentexpression .

expression  = assignmentexpression .

```
 
</details>
