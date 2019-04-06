%{
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"

int yyerror(SExpression **expression, yyscan_t scanner, const char *msg) {
}

%}

%code requires {
  typedef void* yyscan_t;
}

%output  "Parser.cpp"
%defines "Parser.h"

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { SExpression **expression }
%parse-param { yyscan_t scanner }

%union {
  int value;
  char name[0x100];
  SExpression *expression;
}

%token TOKEN_SEMICOLON   ";"
%token TOKEN_DEF   "def"
%token TOKEN_DEFUN   "defun"
%token TOKEN_IF  "if"
%token TOKEN_ELSE   "else"
%token TOKEN_COLON   ":"
%token TOKEN_LSPAREN   "["
%token TOKEN_RSPAREN   "]"
%token TOKEN_LPAREN   "("
%token TOKEN_RPAREN   ")"
%token TOKEN_PLUS     "+"
%token TOKEN_MINUS     "-"
%token TOKEN_STAR     "*"
%token TOKEN_DIV     "/"
%token <value> TOKEN_NUMBER "number"
%token <name> TOKEN_NAME "name"
%type <expression> expr

%left ";"
%left "else"
%left ":"
%left "+"
%left "-"   		
%left "*"
%left "/"

%%

input
    : expr { *expression = $1; }
    ;

expr
: expr[L] "+" expr[R] { $$ = createOperation( eADD, $L, $R ); }
| expr[L] ";" expr[R] { $$ = createOperation( eSEM, $L, $R ); }
| expr[L] ";" { $$ = $L; }
| expr[L] "*" expr[R] { $$ = createOperation( eMULTIPLY, $L, $R ); }
| expr[L] "-" expr[R] { $$ = createOperation( eMINUS, $L, $R ); }
| expr[L] "/" expr[R] { $$ = createOperation( eDIV, $L, $R ); }
| "name" "(" expr[E] ")"            { $$ = createCall($1, $E); }
| "def" "name"[N] ":" expr[E] { $$ = createDef($N, $E); }
| "if" "(" expr[C] ")" expr[L] "else" expr[R] { $$ = createIF($C, $L, $R); }
| "defun" "name"[N] ":" expr[E] { $$ = createDefun($N, $E); }
| "(" expr[E] ")"     { $$ = $E; }
| "number"             { $$ = createNumber($1); }
| "name" {$$ = createRef($1);}
| "[" expr[L] ":" expr[R] "]" { $$ = createOperation(eCOLON, $L, $R); }
;

%%
