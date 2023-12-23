#include <memory>

#include <common/token.h>
#include <lexer/lexer.h>

int warnings = 0;
int main (int argc, char *argv[])
{
    std::unique_ptr<Lexer> lexer;

    if (argc == 2)
        lexer = std::make_unique<Lexer>(argv[1]);
    else
        lexer = std::make_unique<Lexer>("/dev/stdin");

    Token tok;
    while ((tok = lexer->peek()) != Token::T_EOF)
    {
        fprintf(stdout, "Line: %d \t Token: %s \t Lexeme: %s\n", lexer->line(), tokenToStr(tok),
                lexer->lexeme().c_str());
        lexer->consume();
    }

    return 0;
}
