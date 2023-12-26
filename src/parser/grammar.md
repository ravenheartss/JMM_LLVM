# J-- Grammar

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
                | STRING
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
                        | conditionalandexpression AND bitwiseorexpreession
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


