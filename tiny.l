%option noyywrap

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
static int yylex(void);
%}

number      [0-9]+
identifier  [a-zA-Z]+
newline     \n
whitespace  [ \t\r]+

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"return"        {return RETURN;}
"int"           {return INT;}
"void"          {return VOID;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"!="            {return DIF;}
"<"             {return LT;}
"<="            {return LET;}
">"             {return GT;}
">="            {return GET;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"["             {return LBRACKET;}
"]"             {return RBRACKET;}
"{"             {return LBRACES;}
"}"             {return RBRACES;}
";"             {return SEMI;}
","             {return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            {char a, b;
                  a = input();
                  if (a == '\n') lineno++;
                  while(TRUE){
                    if (a == EOF) return ERROR;
                    else {
                      b = input();
                      if (b == EOF) return ERROR;
                      else{
                        if (b == '\n') lineno++;
                        else if (a == '*' && b == '/') break;
                        else{
                          a = b;
                        }
                      }
                    }
                  }
                }
.               {return ERROR;}

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);

  }
  return currentToken;
}

