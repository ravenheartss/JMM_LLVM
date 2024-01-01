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

I have not made any big changes to the language as this was mainly implemented so I can help my students better on issues that they would run into. There hasn't been any big changes to the grammar either.


--------------

### Lexer

* Single line comments only -- `//`
* Identifiers -- start with `_` or alphabet followed by alphanumric or underscores
* String literals -- Only escapes `\b, \f, \t, \r, \n, \', \", \\` supported
* Integer literals -- base ten only
* Reserved words -- `true, false, boolean, int, void, if, else, while, break, return, str, goto`
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
                        | GOTO expression ';'
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
<summary><b>Step 1: Remove immediate left recursion</b></summary>

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

globaldeclarations      : globaldeclaration globaldeclarations1
                        ;

globaldeclarations1     :
                        | globaldeclaration1 globaldeclaration
                        ;

globaldeclaration       : variabledeclaration
                        | functiondeclaration
                        | mainfunctiondeclaration
                        ;

variabledeclaration     : type identifier SEMCOL
                        ;

identifier              : ID
                        ;

functiondeclaration     : functionheader block
                        ;

functionheader          : type functiondeclarator
                        | VOID functiondeclarator
                        ;

functiondeclarator      : identifier OPAREN formalparameterlist CPAREN
                        | identifier OPAREN CPAREN
                        ;

formalparameterlist     : formalparameter formalparameterlist1
                        ;

formalparameterlist1    :
                        | COMMA formalparameter formalparameterlist1
                        ;

formalparameter         : type identifier
                        ;

mainfunctiondeclaration : mainfunctiondeclarator block
                        ;

mainfunctiondeclarator  : identifier OPAREN CPAREN
                        ;

block                   : OBRCK blockstatements CBRCK
                        | OBRCK CBRCK
                        ;

blockstatements         : blockstatement blockstatements1
                        ;

blockstatements1        :
                        | blockstatement blockstatements1
                        ;

blockstatement          : variabledeclaration
                        | statement
                        ;

statement               : block
                        | SEMCOL
                        | statementexpression SEMCOL
                        | BREAK SEMCOL
                        | RETURN expression SEMCOL
                        | RETURN SEMCOL
                        | IF OPAREN expression CPAREN statement
                        | IF OPAREN expression CPAREN statement ELSE statement
                        | WHILE OPAREN expression CPAREN statement
                        | GOTO expression SEMCOL
                        ;

statementexpression     : assignment
                        | functioninvocation
                        ;

primary                 : literal
                        | OPAREN expression CPAREN
                        | functioninvocation
                        ;

argumentlist            : expression argumentlist1
                        ;

argumentlist1           :
                        | COMMA expression argumentlist1
                        ;

functioninvocation      : identifier OPAREN argumentlist CPAREN
                        | identifier OPAREN CPAREN
                        ;

postfixexpression       : primary postfixexpression1
                        | identifier postfixexpression1
                        ;

postfixexpression1      :
                        | INC postfixexpression1
                        | DEC postfixexpression1
                        ;

unaryexpression         : MINUS unaryexpression
                        | NOT unaryexpression
                        | INC unaryexpression
                        | DEC unaryexpression
                        | postfixexpression
                        ;

multiplicativeexpression: unaryexpression multiplicativeexpression1
                        ;

multiplicativeexpression1   : 
                            | MULT unaryexpression multiplicativeexpression1
                            | DIV unaryexpression multiplicativeexpression1
                            | MOD unaryexpression multiplicativeexpression1
                            ;


additiveexpression      : multiplicativeexpression additiveexpression1
                        ;

additiveexpression1     : 
                        | PLUS multiplicativeexpression additiveexpression1
                        | MINUS multiplicativeexpression additiveexpression1
                        ;

shiftexpression         : additiveexpression shiftexpression1
                        ;

shiftexpression1        :
                        | LSHIFT additiveexpression shiftexpression1
                        | RSHIFT additiveexpression shiftexpression1
                        ;

relationalexpression    : shiftexpression relationalexpression1
                        ;


relationalexpression1   :
                        | LT shiftexpression relationalexpression1
                        | GT shiftexpression relationalexpression1
                        | LE shiftexpression relationalexpression1
                        | GE shiftexpression relationalexpression1
                        ;

equalityexpression      : relationalexpression equalityexpression1
                        ;

equalityexpression1     :
                        | EQ relationalexpression equalityexpression1
                        | NE relationalexpression equalityexpression1
                        ;

bitwiseandexpression    : equalityexpression bitwiseandexpression1
                        ;

bitwiseandexpression1   :
                        | BAND equalityexpression bitwiseandexpression1
                        ;

exclusiveorexpression   : bitwiseandexpression exclusiveorexpression1
                        ;

exclusiveorexpression1  :
                        | XOR bitwiseandexpression exclusiveorexpression1
                        ;

bitwiseorexpreession    : exclusiveorexpression bitwiseorexpreession1
                        ;

bitwiseorexpreession1   :
                        | BOR exclusiveorexpression bitwiseorexpreession1
                        ;


conditionalandexpression    : bitwiseorexpreession conditionalandexpression1
                            ;

conditionalandexpression1   :
                            | LAND bitwiseorexpreession conditionalandexpression1
                            ;


conditionalorexpression : conditionalandexpression conditionalorexpression1
                        ;

conditionalorexpression1:
                        | LOR conditionalandexpression conditionalorexpression1
                        ;

assignmentexpression    : conditionalorexpression
                        | assignment
                        ;

assignment              : identifier ASS assignmentexpression
                        ;

expression              : assignmentexpression
                        ;
```

</details>

<details>
<summary><b>Step 2: Left factor</b></summary>


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

globaldeclarations      : globaldeclaration globaldeclarations1
                        ;

globaldeclarations1     :
                        | globaldeclaration1 globaldeclaration
                        ;

globaldeclaration       : variabledeclaration
                        | functiondeclaration
                        | mainfunctiondeclaration
                        ;

variabledeclaration     : type identifier SEMCOL
                        ;

identifier              : ID
                        ;

functiondeclaration     : functionheader block
                        ;

functionheader          : type functiondeclarator
                        | VOID functiondeclarator
                        ;

functiondeclarator      : identifier OPAREN functiondeclaratorf
                        ;

functiondeclaratorf     : formalparameterlist CPAREN
                        | CPAREN
                        ;

formalparameterlist     : formalparameter formalparameterlist1
                        ;

formalparameterlist1    :
                        | COMMA formalparameter formalparameterlist1
                        ;

formalparameter         : type identifier
                        ;

mainfunctiondeclaration : mainfunctiondeclarator block
                        ;

mainfunctiondeclarator  : identifier OPAREN CPAREN
                        ;

block                   : OBRCK blockstatements CBRCK
                        | OBRCK CBRCK
                        ;

blockf                  : blockstatements CBRCK
                        | CBRCK
                        ;

blockstatements         : blockstatement blockstatements1
                        ;

blockstatements1        :
                        | blockstatement blockstatements1
                        ;

blockstatement          : variabledeclaration
                        | statement
                        ;

statement               : block
                        | SEMCOL
                        | statementexpression SEMCOL
                        | BREAK SEMCOL
                        | RETURN retstmtf
                        | IF OPAREN expression CPAREN ifstmtf
                        | WHILE OPAREN expression CPAREN statement
                        | GOTO expression SEMCOL
                        ;

retstmtf                : expression SEMCOL
                        | SEMCOL
                        ;

ifstmtf                 : statement
                        | statement ELSE statement
                        ;

statementexpression     : assignment
                        | functioninvocation
                        ;

primary                 : literal
                        | OPAREN expression CPAREN
                        | functioninvocation
                        ;

argumentlist            : expression argumentlist1
                        ;

argumentlist1           :
                        | COMMA expression argumentlist1
                        ;

functioninvocation      : identifier OPAREN argumentlist CPAREN
                        | identifier OPAREN CPAREN
                        ;

functioninvocationf     : argumentlist CPAREN
                        | CPAREN
                        ;

postfixexpression       : primary postfixexpression1
                        | identifier postfixexpression1
                        ;

postfixexpression1      :
                        | INC postfixexpression1
                        | DEC postfixexpression1
                        ;

unaryexpression         : MINUS unaryexpression
                        | NOT unaryexpression
                        | INC unaryexpression
                        | DEC unaryexpression
                        | postfixexpression
                        ;

multiplicativeexpression: unaryexpression multiplicativeexpression1
                        ;

multiplicativeexpression1   : 
                            | MULT unaryexpression multiplicativeexpression1
                            | DIV unaryexpression multiplicativeexpression1
                            | MOD unaryexpression multiplicativeexpression1
                            ;


additiveexpression      : multiplicativeexpression additiveexpression1
                        ;

additiveexpression1     : 
                        | PLUS multiplicativeexpression additiveexpression1
                        | MINUS multiplicativeexpression additiveexpression1
                        ;

shiftexpression         : additiveexpression shiftexpression1
                        ;

shiftexpression1        :
                        | LSHIFT additiveexpression shiftexpression1
                        | RSHIFT additiveexpression shiftexpression1
                        ;

relationalexpression    : shiftexpression relationalexpression1
                        ;


relationalexpression1   :
                        | LT shiftexpression relationalexpression1
                        | GT shiftexpression relationalexpression1
                        | LE shiftexpression relationalexpression1
                        | GE shiftexpression relationalexpression1
                        ;

equalityexpression      : relationalexpression equalityexpression1
                        ;

equalityexpression1     :
                        | EQ relationalexpression equalityexpression1
                        | NE relationalexpression equalityexpression1
                        ;

bitwiseandexpression    : equalityexpression bitwiseandexpression1
                        ;

bitwiseandexpression1   :
                        | BAND equalityexpression bitwiseandexpression1
                        ;

exclusiveorexpression   : bitwiseandexpression exclusiveorexpression1
                        ;

exclusiveorexpression1  :
                        | XOR bitwiseandexpression exclusiveorexpression1
                        ;

bitwiseorexpreession    : exclusiveorexpression bitwiseorexpreession1
                        ;

bitwiseorexpreession1   :
                        | BOR exclusiveorexpression bitwiseorexpreession1
                        ;


conditionalandexpression    : bitwiseorexpreession conditionalandexpression1
                            ;

conditionalandexpression1   :
                            | LAND bitwiseorexpreession conditionalandexpression1
                            ;


conditionalorexpression : conditionalandexpression conditionalorexpression1
                        ;

conditionalorexpression1:
                        | LOR conditionalandexpression conditionalorexpression1
                        ;

assignmentexpression    : conditionalorexpression
                        | assignment
                        ;

assignment              : identifier ASS assignmentexpression
                        ;

expression              : assignmentexpression
                        ;
```

</details>


<details>
<summary><b>LL(1) Grammar</b></summary>

```yacc
start   : 
        | globaldeclarations
        ;


literal : NUMBER
        | STRING
        | TRUE
        | FALSE
        ;


type    : BOOLEAN
        | INT
        | STR
        ;

globaldeclarations  : globaldeclaration globaldeclarations1
                    ;

globaldeclarations1 : 
                    | globaldeclaration globaldeclarations1
                    ;

globaldeclaration   : type identifier globaldeclarationf
                    | VOID identifier OPAREN functiondeclarator
                    | mainfunctiondeclaration
                    ;

globaldeclarationf  : SEMCOL
                    | OPAREN functiondeclarator
                    ;

variabledeclaration : type identifier SEMCOL
                    ;

identifier  : ID
            ;

functiondeclarator  : formalparameterlist CPAREN
                    | CPAREN
                    ;

formalparameterlist : formalparameter formalparameterlist1
                    ;

formalparameterlist1    : 
                        | COMMA formalparameter formalparameterlist1
                        ;

formalparameter : type identifier
                ;

mainfunctiondeclaration : mainfunctiondeclarator block
                        ;

mainfunctiondeclarator  : identifier OPAREN CPAREN
                        ;

block   : OBRCK blockf
        ;

blockf  : CBRCK
        | blockstatements CBRCK
        ;

blockstatements : blockstatement blockstatements1
                ;

blockstatements1    : 
                    | blockstatement blockstatements1
                    ;

blockstatement  : variabledeclaration
                | statement
                ;

statement   : block
            | SEMCOL
            | statementexpression SEMCOL
            | BREAK SEMCOL
            | RETURN retstmtf
            | IF OPAREN expression CPAREN ifstmtf
            | WHILE OPAREN expression CPAREN statement
            | GOTO expression SEMCOL
            ;

retstmtf    : expression SEMCOL
            | SEMCOL
            ;

ifstmtf     : statement
            | statement ELSE statement
            ;

statementexpression : identifier statementexpressionf

statementexpressionf    : assignment
                        | functioninvocation
                        ;

primary : literal
        | OPAREN expression CPAREN
        ;

argumentlist    : expression argumentlist1
                ;

argumentlist1   : 
                | COMMA expression argumentlist1
                ;

functioninvocation  : OPAREN functioninvocationf
                    ;

functioninvocationf : argumentlist CPAREN
                    | CPAREN
                    ;

postfixexpression   : primary postfixexpression1
                    | identifier postfixexpressionf
                    ;

postfixexpressionf  : postfixexpression1
                    | functioninvocation postfixexpression1
                    ;

postfixexpression1  : 
                    | INC postfixexpression1
                    | DEC postfixexpression1
                    ;

unaryexpression : MINUS unaryexpression
                | NOT unaryexpression
                | INC unaryexpression
                | DEC unaryexpression
                | postfixexpression
                ;

multiplicativeexpression    : unaryexpression multiplicativeexpression1
                            ;

multiplicativeexpression1   : 
                            | MULT unaryexpression multiplicativeexpression1
                            | DIV unaryexpression multiplicativeexpression1
                            | MOD unaryexpression multiplicativeexpression1
                            ;

additiveexpression  : multiplicativeexpression additiveexpression1
                    ;

additiveexpression1 : 
                    | PLUS multiplicativeexpression additiveexpression1
                    | MINUS multiplicativeexpression additiveexpression1
                    ;

shiftexpression : additiveexpression shiftexpression1
                ;

shiftexpression1    : 
                    | LSHIFT additiveexpression shiftexpression1
                    | RSHIFT additiveexpression shiftexpression1
                    ;

relationalexpression    : shiftexpression relationalexpression1
                        ;

relationalexpression1   : 
                        | LT shiftexpression relationalexpression1
                        | GT shiftexpression relationalexpression1
                        | LE shiftexpression relationalexpression1
                        | GE shiftexpression relationalexpression1
                        ;

equalityexpression  : relationalexpression equalityexpression1
                    ;

equalityexpression1 : 
                    | EQ relationalexpression equalityexpression1
                    | NE relationalexpression equalityexpression1
                    ;

bitwiseandexpression    : equalityexpression bitwiseandexpression1
                        ;

bitwiseandexpression1   : 
                        | BAND equalityexpression bitwiseandexpression1
                        ;

exclusiveorexpression   : bitwiseandexpression exclusiveorexpression1
                        ;

exclusiveorexpression1  : 
                        | XOR bitwiseandexpression exclusiveorexpression1
                        ;

bitwiseorexpreession    : exclusiveorexpression bitwiseorexpreession1
                        ;

bitwiseorexpreession1   : 
                        | BOR exclusiveorexpression bitwiseorexpreession1
                        ;

conditionalandexpression    : bitwiseorexpreession conditionalandexpression1
                            ;

conditionalandexpression1   : 
                            | LAND bitwiseorexpreession conditionalandexpression1
                            ;

conditionalorexpression : conditionalandexpression conditionalorexpression1
                        ;

conditionalorexpression1    : 
                            | LOR conditionalandexpression conditionalorexpression1
                            ;

assignmentexpression    : conditionalorexpression assignmentexpression1
                        ;

assignmentexpression1   : ASS conditionalorexpression assignmentexpression1
                        | 
                        ;

assignment  : ASS assignmentexpression
            ;

expression  : assignmentexpression
            ;
```


</details>

<details>
<summary<b>LL(1) Grammar in EBNF</b></summary>

```EBNF
start   = { globaldeclaration } .

literal = NUMBER | STRING | TRUE | FALSE .

type    = BOOLEAN | INT | STR .

globaldeclaration   = type identifier [ OPAREN functiondeclarator ] SEMCOL
                    | VOID identifier OPAREN functiondeclarator
                    | mainfunctiondeclaration .

variabledeclaration = type identifier SEMCOL

identifier  = ID .

functiondeclarator  = [ [formalparameterlist ] CPAREN ] .

formalparameterlist = formalparameter { COMMA formalparameter } .

formalparameter = type identifier .

mainfunctiondeclaration = mainfunctiondeclarator block .

mainfunctiondeclarator  = identifier OPAREN CPAREN .

block   = OBRCK [ blockstatements ] CBRCK .

blockstatements = statement { statement } .

statement   = variabledeclaration | simpleStmt | returnStmt |
                breakStmt | block | ifStmt | gotoStmt | whileStmt .

simpleStmt  = nullStmt | exprStmt .

nullStmt    = SEMCOL .

exprStmt    = identifier ( assignment | functioninvocation ) SEMCOL .

returnStmt  = RETURN [ expression ] SEMCOL .

breakStmt   = BREAK SEMCOL .

ifStmt      = IF OPAREN expression CPAREN statement [ ELSE statement ] .

gotoStmt    = GOTO expression SEMCOL .

whileStmt   = WHILE OPAREN expression CPAREN statement .

primary     = literal | OPAREN expression CPAREN .

argumentlist    = expression { COMMA expression } .

functioninvocation  = OPAREN [ argumentlist ] CPAREN .

postfixexpression   = primary postfixexpression1
                    | identifier [ functioninvocation ] postfixexpression1 .

postfixexpression1  = [ ( INC | DEC ) [ postfixexpression1 ] ] .

unaryexpression = ( MINUS | NOT | INC | DEC ) unaryexpression | postfixexpression .

multiplicativeexpression    = unaryexpression { mult_op unaryexpression } .

mult_op = MULT | DIV | MOD .

additiveexpression  = multiplicativeexpression { add_op multiplicativeexpression } .

add_op = PLUS | MINUS .

shift_op = LSHIFT | RSHIFT .

shiftexpression = additiveexpression { shift_op additiveexpression } .

rel_op = LT | GT | LE | GE .

relationalexpression    = shiftexpression { rel_op shiftexpression } .

eq_op = EQ | NE .

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
