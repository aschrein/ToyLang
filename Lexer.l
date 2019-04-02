%{
#include "Expression.h"
#include "Parser.h"

#include <stdio.h>

#define LOG(s) printf("%s\n", s)

%}

%option outfile="Lexer.cpp" header-file="Lexer.h"
%option warn nodefault

%option reentrant noyywrap never-interactive nounistd
%option bison-bridge

%%

[ \r\n\t]*   { continue; }
[0-9]+       { sscanf(yytext, "%d", &yylval->value); return TOKEN_NUMBER; }
"*"          { return TOKEN_STAR; }
":"          { return TOKEN_COLON; }
";"          { return TOKEN_SEMICOLON; }
"["          { return TOKEN_LSPAREN; }
"]"          { return TOKEN_RSPAREN; }
"/"          { return TOKEN_DIV; }
"+"          { return TOKEN_PLUS; }
"("          { return TOKEN_LPAREN; }
")"          { return TOKEN_RPAREN; }
"defun"      { return TOKEN_DEFUN; }
"def"        { return TOKEN_DEF; }
[a-z]+       {
  //yylval->name = (char*)malloc(0x100);
  sscanf(yytext, "%s", yylval->name);
  return TOKEN_NAME;
}
.            { continue; /* Ignore unexpected characters. */}

%%

int yyerror(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    return 0;
}
