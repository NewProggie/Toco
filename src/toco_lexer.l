%{
#include <string>
#include <iostream>
#include "nodes.h"
#include "toco_parser.h"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
%}

FLOAT_NUMBER   [0-9][0-9]*(.[0-9]+)?
NAT_NUMBER     [0-9]+
IDENTIFIER     [a-zA-Z][a-zA-Z0-9_]*

%%
[ \t\n]         ; // ignore whitespace
{IDENTIFIER}    { SAVE_TOKEN; return TOK_IDENTIFIER; }
{FLOAT_NUMBER}  { SAVE_TOKEN; return TOK_DOUBLE; }
{NAT_NUMBER}    { SAVE_TOKEN; return TOK_INTEGER; }
"="             { return TOKEN(TOK_ASSIGN); }
"=="            { return TOKEN(TOK_EQUAL); }
"!="            { return TOKEN(TOK_NEQUAL); }
"("             { return TOKEN(TOK_LPAREN); }
")"             { return TOKEN(TOK_RPAREN); }
"{"             { return TOKEN(TOK_LBRACE); }
"}"             { return TOKEN(TOK_RBRACE); }
","             { return TOKEN(TOK_COMMA); }
"+"             { return TOKEN(TOK_PLUS); }
"-"             { return TOKEN(TOK_MINUS); }
"*"             { return TOKEN(TOK_MUL); }
"/"             { return TOKEN(TOK_DIV); }
.               { std::cerr << "Unknown token received\n"; yyterminate(); }

%%

int yywrap() {
  // see man flex for now
  return 1;
}

#pragma clang diagnostic pop