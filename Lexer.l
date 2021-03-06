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
"//".*   { continue; }
[0-9]+       { sscanf(yytext, "%d", &yylval->value); return TOKEN_NUMBER; }
"*"          { return TOKEN_STAR; }
","          { return TOKEN_COMMA; }
":"          { return TOKEN_COLON; }
"<"          { return TOKEN_LESS; }
"=="         { return TOKEN_EQEQ; }
";"          { return TOKEN_SEMICOLON; }
"["          { return TOKEN_LSPAREN; }
"]"          { return TOKEN_RSPAREN; }
"/"          { return TOKEN_DIV; }
"+"          { return TOKEN_PLUS; }
"-"          { return TOKEN_MINUS; }
"("          { return TOKEN_LPAREN; }
")"          { return TOKEN_RPAREN; }
"}"          { return TOKEN_RBRACE; }
"{"          { return TOKEN_LBRACE; }
"defun"      { return TOKEN_DEFUN; }
"def"        { return TOKEN_DEF; }
"if"        { return TOKEN_IF; }
"else"        { return TOKEN_ELSE; }
[a-zA-Z]+[0-9a-zA-Z]*       {
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
