/****************************************************/
/* The TINY Yacc/Bison specification file           */
/****************************************************/

%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "symtab.h"
#include <string.h>

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
char * funcName = "global";
%}

%token IF ELSE WHILE RETURN INT VOID 
%token ID NUM 
%token ASSIGN EQ DIF LT LET GT GET PLUS MINUS TIMES OVER LPAREN RPAREN SEMI COMMA LBRACKET RBRACKET LBRACES RBRACES
%token ERROR 

%% /* Grammar for TINY */

program       : decl_list {
		              savedTree = $1;
		            };

decl_list     : decl_list decl { 
                  YYSTYPE t = $1;
                  if (t != NULL){ 
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $2;
                    $$ = $1; 
                  }
                  else 
                    $$ = $2;
                }
              | decl { 
                $$ = $1; };

decl          : var_decl {
                  $$ = $1;
                }
              | func_decl {
                  $$ = $1;
                  funcName = "global";
                };

type_espec    : INT {
                  $$ = newExpNode (TypeK);
                  $$->attr.type = "int";
                }
              | VOID {
                  $$ = newExpNode (TypeK);
                  $$->attr.type = "void";
                };

id          : ID {
                $$ = newExpNode (IdK);
                $$->attr.name = copyString (tokenString);
                savedName = copyString (tokenString);
                $$->name = copyString (tokenString);
                $$->idType = "";
                $$->dataType = "";
                $$->scope = funcName;
                $$->vector = -1;
                $$->declared = 0;
              };

fid         : {
                funcName = savedName;
              };

vet_size    : NUM {
                $$ = newExpNode (ConstK);
                $$->attr.val = atoi (tokenString);
              };

var_decl      : type_espec id SEMI {
                  $$ = newExpNode (VarK);
                  $$->attr.name = $2->attr.name;
                  $$->name = $2->attr.name;
                  $$->idType = "varDecl";
                  $$->dataType = $1->attr.type;
                  $$->scope = funcName;
                  $$->lineno = $2->lineno;
                  $$->vector = -1;
                  $$->declared = 1;
                }
              | type_espec id LBRACKET vet_size RBRACKET SEMI {
                  $$ = newExpNode (VarK);
                  $$->attr.name = $2->attr.name;
                  $$->name = $2->attr.name;
                  $$->idType = "varDecl";
                  $$->dataType = $1->attr.type;
                  $$->scope = funcName;
                  $$->lineno = $2->lineno;
                  $$->vector = $4->attr.val;
                  $$->declared = 1;
                }
              | error {
                  yyerrok;
                };
              
func_decl     : type_espec id fid LPAREN params RPAREN compost_decl {
                  $$ = newExpNode (FuncK);
                  $$->attr.name = funcName;
                  $$->name = funcName;
                  $$->idType = "funcDecl";
                  $$->dataType = $1->attr.type;
                  $$->scope = funcName;
                  $$->lineno = $2->lineno;
                  $$->vector = -1;
                  $$->declared = 1;
                  $$->child[0] = $5;
                  $$->child[1] = $7;
                };

params        : param_list {
                  $$ = $1;
                }
              | VOID {
                  $$ = newExpNode(ParamK);
                  $$->attr.name = "void";
                  $$->name = "void";
                  $$->idType = "param";
                  $$->dataType = "void";
                  $$->scope = funcName;
                  $$->vector = -1;
                  $$->declared = 0;
                };

param_list    : param_list COMMA param {
                  YYSTYPE t = $1;
                  if (t != NULL) {
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $3;
                    $$ = $1;
                  }
                  else {
                    $$ = $3;
                  }
                }
              | param {
                  $$ = $1;
                };

param         : INT id {
                  $$ = newExpNode (ParamK);
                  $$->attr.name = $2->attr.name;
                  $$->name = $2->attr.name;
                  $$->idType = "param";
                  $$->dataType = "int";
                  $$->scope = funcName;
                  $$->vector = -1;
                  $$->declared = 1;
                }
              | INT id LBRACKET RBRACKET {
                  $$ = newExpNode (ParamK);
                  $$->attr.name = $2->attr.name;
                  $$->name = $2->attr.name;
                  $$->idType = "param";
                  $$->dataType = "int";
                  $$->scope = funcName;
                  $$->vector = -2;
                  $$->declared = 1;
                };

compost_decl  : LBRACES local_decl stmt_list RBRACES {
                  YYSTYPE t = $2;
                  if (t != NULL) {
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $3;
                    $$ = $2;
                  }
                  else 
                    $$ = $3;
                }
              | LBRACES local_decl RBRACES {
                  $$ = $2;
                }
              | LBRACES stmt_list RBRACES {
                  $$ = $2;
                }
              | LBRACES RBRACES {
                  // do nothing
                };

local_decl    : local_decl var_decl {
                  YYSTYPE t = $1;
                  if (t != NULL) {
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
                  }
                  else 
                    $$ = $2;
                }
              | var_decl {
                  $$ = $1;
                };

stmt_list     : stmt_list stmt {
                  YYSTYPE t = $1;
                  if (t != NULL) {
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $2;
                    $$ = $1;
                  }
                  else 
                    $$ = $2;
                }
              | stmt {
                  $$ = $1;
                };

stmt          : exp_decl {
                  $$ = $1;
                }
              | compost_decl {
                  $$ = $1;
                }
              | sel_decl {
                  $$ = $1;
                }
              | it_decl {
                  $$ = $1;
                }
              | ret_decl {
                  $$ = $1;
                };

exp_decl      : exp SEMI{
                  $$ = $1;
                }
              | SEMI {
                  //do nothing
                }
              | error {
                  yyerrok;
              };

sel_decl      : IF LPAREN exp RPAREN stmt {
                  $$ = newStmtNode(IfK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                }
              | IF LPAREN exp RPAREN stmt ELSE stmt {
                  $$ = newStmtNode(IfK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                  $$->child[2] = $7;
                };

it_decl       : WHILE LPAREN exp RPAREN stmt {
                  $$ = newStmtNode(WhileK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                };

ret_decl      : RETURN SEMI {
                  $$ = newStmtNode (ReturnK);
                }
              | RETURN exp SEMI {
                  $$ = newStmtNode (ReturnK);
                  $$->child[0] = $2;
                };

exp           : var ASSIGN exp {
                  $$ = newStmtNode (AssignK);
                  $$->attr.name = $1->attr.name;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
              | simple_exp {
                  $$ = $1;
                };

var           : id {
                  $$ = $1;
                  $$->dataType = "int";
                  $$->idType = "var";
                  $$->declared = 0;
                }
              | id LBRACKET exp RBRACKET {
                  $$ = $1;
                  $$->dataType = "int";
                  $$->idType = "var";
                  $$->child[0] = $3;
                };

simple_exp    : sum_exp relational sum_exp {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
              | sum_exp {
                  $$ = $1;
                };

relational    : EQ {
                  $$ = newExpNode (OpK);
                  $$->attr.op = EQ;
                }
              | DIF {
                  $$ = newExpNode (OpK);
                  $$->attr.op = DIF;
                }
              | LT {
                  $$ = newExpNode (OpK);
                  $$->attr.op = LT;
                }
              | LET {
                  $$ = newExpNode (OpK);
                  $$->attr.op = LET;
                }
              | GT {
                  $$ = newExpNode (OpK);
                  $$->attr.op = GT;
                }
              | GET {
                  $$ = newExpNode (OpK);
                  $$->attr.op = GET;
                };

sum_exp       : sum_exp sum term {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
              | term {
                  $$ = $1;
                };

sum           : PLUS {
                  $$ = newExpNode (OpK);
                  $$->attr.op = PLUS;
                }
              | MINUS {
                  $$ = newExpNode (OpK);
                  $$->attr.op = MINUS;
                };

term          : term mult factor {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                }
              | factor {
                  $$ = $1;
                };

mult          : TIMES {
                  $$ = newExpNode(OpK);
                  $$->attr.op = TIMES;
                }
              | OVER {
                  $$ = newExpNode(OpK);
                  $$->attr.op = OVER;
                };

factor        : LPAREN exp RPAREN {
                  $$ = $2;
                }
              | var {
                  $$ = $1;
                }
              | activation {
                  $$ = $1;
                }
              | NUM {
                  $$ = newExpNode(ConstK);
                  $$->attr.val = atoi(tokenString);
                };

activation    : id LPAREN arg_list RPAREN {
                  $$ = newExpNode (ActivK);
                  $$->attr.name = $1->attr.name;
                  $$->name = $1->attr.name;
                  $$->idType = "call";
                  $$->dataType = "int";
                  $$->scope = funcName;
                  $$->vector = -1;
                  $$->declared = 0;
                  $$->child[0] = $3;
                }
              | id LPAREN RPAREN {
                  $$ = newExpNode (ActivK);
                  $$->attr.name = $1->attr.name;
                  $$->name = $1->attr.name;
                  $$->idType = "call";
                  $$->dataType = "int";
                  $$->scope = funcName;
                  $$->vector = -1;
                  $$->declared = 0;
                };

arg_list      : arg_list COMMA exp {
                  YYSTYPE t = $1;
                  if (t != NULL) {
                    while (t->sibling != NULL)
                      t = t->sibling;
                    t->sibling = $3;
                    $$ = $1;
                  }
                  else {
                    $$ = $3;
                  }
                }
              | exp {
                  $$ = $1;
                };
%%

int yyerror(char * message) {
  printf("\033[0;31m");
  fprintf(listing,"%s > \tLINE: %d - ", message, lineno);
  printToken(yychar,tokenString);
  printf("\033[0m");
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output compatible with ealier versions of the TINY scanner */
static int yylex(void) { 
  return getToken(); 
}

TreeNode * parse(void) {
  yyparse();
  return savedTree;
}

