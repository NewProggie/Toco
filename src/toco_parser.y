%{
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include "nodes.h"

extern int yylex();
void yyerror(const char*);
toco::Block *program;
%}

// All the different ways we can access our data
%union {
  toco::Node *node;
  toco::Block *block;
  toco::Expression *expr;
  toco::Statement *stmt;
  toco::Identifier *ident;
  toco::VariableDeclaration *var_decl;
  std::vector<toco::VariableDeclaration*> *var_vec;
  std::vector<toco::Expression*> *expr_vec;
  std::string *string;
  int token;
}

// Define all terminal symbols (tokens) matching the toco lexer file
%token <string> TOK_IDENTIFIER TOK_INTEGER TOK_DOUBLE
%token <token> TOK_ASSIGN TOK_EQUAL TOK_NEQUAL
%token <token> TOK_LPAREN TOK_RPAREN TOK_LBRACE TOK_RBRACE TOK_COMMA
%token <token> TOK_PLUS TOK_MINUS TOK_MUL TOK_DIV

// Define the types of nonterminal symbols. They refer to the union declaration
// above
%type <ident> ident
%type <expr> numeric expr
%type <var_vec> func_decl_args
%type <expr_vec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl
%type <token> comparison

// Operator precedence for math operations
%left TOK_PLUS TOK_MINUS
%left TOK_MUL TOK_DIV

%start program

%%

program : stmts { program = $1; };
        ;

stmts : stmt { $$ = new toco::Block(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl
      | expr { $$ = new toco::ExpressionStatement(*$1); }
      ;

block : TOK_LBRACE stmts TOK_RBRACE { $$ = $2; }
      | TOK_LBRACE TOK_RBRACE { $$ = new toco::Block(); }
      ;

var_decl : ident ident { $$ = new toco::VariableDeclaration(*$1, *$2); }
         | ident ident TOK_ASSIGN expr { $$ = new toco::VariableDeclaration (*$1, *$2, $4); }
         ;

func_decl : ident ident TOK_LPAREN func_decl_args TOK_RPAREN block
            { $$ = new toco::FunctionDeclaration(*$1, *$2, *$4, *$6); delete $4; }
          ;

func_decl_args : { $$ = new toco::Variables(); }
               | var_decl { $$ = new toco::Variables(); $$->push_back($<var_decl>1); }
               | func_decl_args TOK_COMMA var_decl { $1->push_back($<var_decl>3); }
               ;

ident : TOK_IDENTIFIER { $$ = new toco::Identifier(*$1); delete $1; }
      ;

numeric : TOK_INTEGER { $$ = new toco::Integer(atol($1->c_str())); delete $1; }
        | TOK_DOUBLE { $$ = new toco::Double(atof($1->c_str())); delete $1; }
        ;

expr : ident TOK_ASSIGN expr { $$ = new toco::Assignment(*$<ident>1, *$3); }
     | ident TOK_LPAREN call_args TOK_RPAREN { $$ = new toco::MethodCall(*$1, *$3); delete $3; }
     | ident { $<ident>$ = $1; }
     | numeric
     | expr comparison expr {$$ = new toco::BinaryOperator(*$1, $2, *$3); }
     | TOK_LPAREN expr TOK_RPAREN { $$ = $2; }
     ;

call_args : { $$ = new toco::Expressions(); }
          | expr { $$ = new toco::Expressions(); $$->push_back($1); }
          | call_args TOK_COMMA expr { $1->push_back($3); }
          ;

comparison : TOK_EQUAL | TOK_NEQUAL | TOK_PLUS | TOK_MINUS | TOK_MUL | TOK_DIV
           ;

%%

void yyerror(const char *str) {
  std::cerr << "Error: " << str << std::endl;
  std::exit(1);
}