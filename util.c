/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

/* Procedure printToken prints a token 
 * and its lexeme to the listing file
 */
void printToken (TokenType token, const char * tokenString) { 
  switch (token) { 
    case IF:
    case ELSE:
    case WHILE:
    case RETURN:
    case INT:
    case VOID:
      printf ("\033[1;33m");
      printf ("RW: ");
      printf ("\033[0m");
      fprintf(listing, "%s\n", tokenString);
      break;
    case ASSIGN: 
      fprintf (listing,"=\n"); 
      break;
    case EQ: 
      fprintf (listing,"==\n"); 
      break;
    case DIF: 
      fprintf (listing,"!=\n"); 
      break;
    case LT: 
      fprintf (listing,"<\n"); 
      break;
    case LET: 
      fprintf (listing,"<=\n"); 
      break;
    case GT: 
      fprintf (listing,">\n"); 
      break;
    case GET: 
      fprintf (listing,">=\n"); 
      break;
    case LPAREN: 
      fprintf (listing,"(\n"); 
      break;
    case RPAREN: 
      fprintf (listing,")\n"); 
      break;
    case LBRACKET: 
      fprintf (listing,"[\n"); 
      break;
    case RBRACKET: 
      fprintf (listing,"]\n"); 
      break;
    case LBRACES: 
      fprintf (listing,"{\n"); 
      break;
    case RBRACES: 
      fprintf (listing,"}\n"); 
      break;
    case PLUS: 
      fprintf (listing,"+\n"); 
      break;
    case MINUS: 
      fprintf (listing,"-\n"); 
      break;
    case TIMES: 
      fprintf (listing,"*\n"); 
      break;
    case OVER: 
      fprintf (listing,"/\n"); 
      break;
    case SEMI: 
      fprintf (listing,";\n"); 
      break;
    case COMMA: 
      fprintf (listing,",\n"); 
      break;
    case ENDFILE: 
      printf("\033[1;33m");
      fprintf (listing,"EOF\n"); 
      printf("\033[0m");
      break;
    case NUM:
      printf ("\033[1;33m");
      printf ("NUM: ");
      printf ("\033[0m");
      fprintf(listing, "%s\n", tokenString);
      break;
    case ID:
      printf ("\033[1;33m");
      printf ("ID: ");
      printf ("\033[0m");
      fprintf(listing, "%s\n", tokenString);
      break;
    case ERROR:
      printf("\033[0;31m");
      fprintf (listing, "LEXICAL ERROR: %s\n", tokenString);
      printf("\033[0m");
      break;
    default: /* should never happen */
      printf("\033[0;31m");
      fprintf (listing,"Unknown token: %d\n", token);
      printf("\033[0m");
      break;
  }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL) {
    printf("\033[0;31m");
    fprintf(listing,"Out of memory error at line %d\n",lineno);
    printf("\033[0m");
  }
  else {
    for (i=0;i<MAXCHILDREN;i++) 
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
  }
  return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL) {
    printf("\033[0;31m");
    fprintf(listing,"Out of memory error at line %d\n",lineno);
    printf("\033[0m");
  }
  else {
    for (i=0;i<MAXCHILDREN;i++) 
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL) {
    printf("\033[0;31m");
    fprintf(listing,"Out of memory error at line %d\n",lineno);
    printf("\033[0m");
  }
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    fprintf(listing," ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 */
void printTree( TreeNode * tree ) { 
  int i;
  INDENT;
  while (tree != NULL) {
    printSpaces();
    if (tree->nodekind==StmtK)
    { switch (tree->kind.stmt) {
        case IfK:
          printf ("\033[1;33m");
          fprintf(listing,"If\n");
          printf ("\033[0m");
          break;
        case WhileK:
          printf ("\033[1;33m");
          fprintf(listing,"While\n");
          printf ("\033[0m");
          break;
        case AssignK:
          printf ("\033[1;33m");
          printf("Assign to: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n", tree->attr.name);
          break;
        case ReturnK:
          printf ("\033[1;33m");
          fprintf(listing,"Return\n");
          printf ("\033[0m");
        break;
        default:
          printf("\033[0;31m");
          fprintf(listing,"Unknown StmtNode kind\n");
          printf("\033[0m");
          break;
      }
    }
    else if (tree->nodekind==ExpK)
    { switch (tree->kind.exp) {
        case TypeK:
          printf ("\033[1;33m");
          printf("Type: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n",tree->dataType);
          break;
        case OpK:
          printf ("\033[1;33m");
          printf("Op: ");
          printf ("\033[0m");
          printToken(tree->attr.op,"\0");
          break;
        case ConstK:
          printf ("\033[1;33m");
          printf("Const: ");
          printf ("\033[0m");
          fprintf(listing,"%d\n",tree->attr.val);
          break;
        case IdK:
          printf ("\033[1;33m");
          printf("Id: ");
          printf ("\033[0m");
          fprintf(listing,"%s - Line: %d\n",tree->attr.name, tree->lineno);
          break;
        case ActivK:
          printf ("\033[1;33m");
          printf("Call: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n",tree->attr.type);
          break;
        case VarK:
          printf ("\033[1;33m");
          printf("Var: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n",tree->attr.name);
          break;
        case FuncK:
          printf ("\033[1;33m");
          printf("FUNCTION: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n", tree->attr.type);
          break;
        case ParamK:
          printf ("\033[1;33m");
          printf("Param: ");
          printf ("\033[0m");
          fprintf(listing,"%s\n",tree->attr.name);
          break;
        default:
          printf("\033[0;31m");
          fprintf(listing,"Unknown ExpNode kind\n");
          printf("\033[0m");
          break;
      }
    }
    else {
      printf("\033[0;31m");
      fprintf(listing,"Unknown node kind\n");
      printf("\033[0m");
    }
    for (i=0;i<MAXCHILDREN;i++)
         printTree(tree->child[i]);
    tree = tree->sibling;
  }
  UNINDENT;
}
