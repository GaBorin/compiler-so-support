/**********************************************************/
/* Semantic analyzer implementation for the TINY compiler */
/**********************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* Counter for variable memory locations */
static int location = 0;
int returnVoid = 0;
int call = 0;

/* Procedure traverse is a generic recursive syntax tree traversal routine: 
 * it applies preProc in preorder and postProc in postorder to tree pointed to by */
static void traverse( TreeNode * t, void (* preProc) (TreeNode *), void (* postProc) (TreeNode *) ) {
  if (t != NULL) {
    preProc(t);
    { 
      int i;
      for (i=0; i < MAXCHILDREN; i++){
        traverse(t->child[i],preProc,postProc);
      }
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* Do-nothing procedure to generate preorder-only or postorder-only traversals from traverse */
static void nullProc(TreeNode * t) {
  if (t==NULL) 
    return;
  else 
    return;
}

/* Inserts identifiers stored in t into the symbol table */
static void insertNode( TreeNode * t) { 
  switch (t->nodekind) { 
    case StmtK:
      switch (t->kind.stmt) { 
        case AssignK:
          //if (st_lookup(t->name, t->scope) == -1)
          /* not yet in table, so treat as new definition */
            //st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,location++);
          //else
          /* already in table, so ignore location, add line number of use only */ 
            //st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
            break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp) {
        case IdK:
          if (st_lookup(t->name, t->scope) == -1)
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          else
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          break;
        case ActivK:
          st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          break;
        case VarK:
          if (st_lookup(t->name, t->scope) == -1){
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,location);
            if (t->vector > 0)
              location += t->vector;
            else
              location++;
          }
          else
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          break;
        case FuncK:
          st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          break;
        case ParamK:
          if (st_lookup(t->name, t->scope) == -1 && strcmp (t->name, "void") != 0){
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,location++);
          }
          else if (strcmp (t->name, "void") != 0){
            st_insert(t->name,t->dataType,t->idType,t->scope,t->vector,t->declared,t->lineno,0);
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void returnType( TreeNode *t ){
  switch (t->nodekind){
    case ExpK:
      switch (t->kind.exp){
          case ActivK:
            call = 1;
            if(st_return (t->attr.name, t->scope, t->lineno))
              returnVoid = 1;
            else
              returnVoid = 0;
            break;
          default:
            break;
      }
      break;
  default:
      break;
  }
}

static void typeError (TreeNode * t, char * message) { 
  printf("\033[0;31m");
  fprintf (listing, "Type error at line %d: %s\n", t->lineno, message);
  printf("\033[0m"); 
  Error = TRUE;
}

static void stmtError (TreeNode * t, char * message) {
  printf("\033[0;31m");
  fprintf(listing,"Statement error at line %d: \"%s\" %s.\n", t->lineno, t->name, message);
  printf("\033[0m"); 
  Error = TRUE;
}

static void checkNode(TreeNode * t) {
  switch (t->nodekind) { 
    case StmtK:
      switch (t->kind.stmt) {
        case AssignK:
          call = 0;
          traverse(t->child[1], nullProc, returnType);
            if(returnVoid && call){
              typeError(t->child[0], "Invalid assignment");
            }
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp) {
        case IdK:
          if (st_var_decl(t->name, t->scope) == 0)
            stmtError(t,"not declared in this scope");
          break;
        case VarK:
          if ((st_var_func_decl (t->name) == 0) && (strcmp(t->idType, "varDecl") == 0))
            stmtError (t, "is already declared as function name");
          if ((strcmp (t->idType, "varDecl") == 0) && (strcmp (t->dataType, "void") == 0))
            stmtError (t, "declared as void type");
          if (st_duplicated (t->name, t->scope, t->idType, t->lineno) != 0)
            stmtError (t,"is already declared");
          break;
        case ActivK:
          if (st_func_decl (t->name) ==  0 && strcmp (t->name, "input") != 0 && strcmp (t->name, "output") != 0 
            && strcmp (t->name, "switchData") != 0 
            && strcmp (t->name, "execSo") != 0  && strcmp (t->name, "execProcess") != 0
            && strcmp (t->name, "checkProcess") != 0 && strcmp (t->name, "setPC") != 0)
            stmtError (t, "not declared");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking by a postorder syntax tree traversal */
void typeCheck(TreeNode * syntaxTree) { 
  traverse(syntaxTree,nullProc,checkNode);
  if(st_main_decl() == 0) { 
    fprintf(listing,"Main function not declared.\n");
    Error = TRUE;
  }
}

/* Constructs the symbol table by preorder traversal of the syntax tree */
void buildSymtab(TreeNode * syntaxTree) {
  traverse(syntaxTree,insertNode,nullProc);
  if (TraceAnalyze) {
    printf("\033[1;36m");
    fprintf(listing,"\n------ SYMBOL TABLE ------\n\n");
    printf("\033[0m");
    printSymTab(listing);
  }
}
