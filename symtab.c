/*******************************************************/
/* Symbol table implementation for the TINY compiler   */
/* (allows only one symbol table)                      */
/* Symbol table is implemented as a chained hash table */
/*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* size of the hash table */
#define SIZE 211

/* the power of two used as multiplier in hash function  */
#define SHIFT 4

typedef struct ScopeList {
  char * name;
  char * type;
} ScopeList;

typedef struct LineListRec { 
  int lineno;
  struct LineListRec * next;
} * LineList;

typedef struct BucketListRec { 
  char * name;
  char * idType;
  char * dataType;
  char * scope;
  int vector;
  int declared;
  int duplicated;
  int memloc; /* memory location for variable */
  LineList lines;
  struct BucketListRec * next;
} * BucketList;

static BucketList hashTable[SIZE];

/* the hash function */
static int hash ( char * key ) { 
  int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { 
    temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

ScopeList scopeList[50];
int scopeListSize = 1;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert ( char * name, char * dataType, char * idType, char * scope, int vector, int declared, int lineno, int loc) {
  int h = hash(name);
  BucketList l =  hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0 || (strcmp(scope, l->scope)) != 0))
    l = l->next;
  if (l == NULL || ((idType != l->idType) && (strcmp(idType, "") != 0))) {
    l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->name = name;
    l->idType = idType;
    if(strcmp(name, "output") == 0)
      l->dataType = "void";
    else if(strcmp(name, "input") == 0)
      l->dataType = "int";
    else if(strcmp(name, "contextIn") == 0)
      l->dataType = "void";
    else if(strcmp(name, "contextOut") == 0)
      l->dataType = "void";
    else if(strcmp(name, "swapIn") == 0)
      l->dataType = "void";
    else if(strcmp(name, "swapOut") == 0)
      l->dataType = "void";
    else if(strcmp(name, "execSo") == 0)
      l->dataType = "void";
    else if(strcmp(name, "execProcess") == 0)
      l->dataType = "void";
    else if(strcmp(name, "checkProcess") == 0)
      l->dataType = "int";
    else
      l->dataType = dataType;
    l->scope = scope;
    l->vector = vector;
    l->declared = declared;
    l->duplicated = -1;
    l->memloc = loc;
    l->lines->lineno = lineno;
    l->lines->next = NULL;
    l->next = hashTable[h];
    hashTable[h] = l; 

    if(strcmp(idType, "funcDecl") == 0){
      scopeList[scopeListSize].name = name;
      scopeList[scopeListSize++].type = dataType;
    }
  }
  else {
    LineList t = l->lines;
    int flag = -1;
    while (t != NULL) {
      if (t->lineno == lineno)
        flag = 0;
      t = t->next;
    }

    if ((strcmp(idType, "") == 0) || (lineno == l->lines->lineno) || (flag == 0)) {
        l->duplicated= lineno;
    }
    else {
      if ((strcmp(l->idType, "varDecl") == 0) && (lineno != l->lines->lineno))
        l->duplicated = lineno;
      
      LineList aux = l->lines;

      while (aux->next != NULL)
        aux = aux->next;

      aux->next = (LineList) malloc(sizeof(struct LineListRec));
      aux->next->lineno = lineno;
      aux->next->next = NULL;
    }
  }
} 

/* Function st_lookup returns the memory location of a variable or -1 if not found */
int st_lookup ( char * name, char * scope ) {
  int h = hash(name);
  BucketList l =  hashTable[h];

  while ((l != NULL) && ((strcmp (name, l->name) != 0) || (strcmp (scope, l->scope) != 0)))
    l = l->next;
  if (l == NULL) 
    return -1;
  else 
    return l->memloc;
}

int st_var_decl ( char * name, char * scope ) {
  int h = hash (name);
  BucketList l = hashTable[h];

  while ((l != NULL) && ((strcmp(name,l->name) != 0 || (strcmp(l->idType, "varDecl") != 0 && strcmp(l->idType, "param") != 0)  || (strcmp (l->scope, scope) != 0) && (strcmp (l->scope, "global") != 0))))
    l = l->next;
  if (l == NULL) 
    return 0;
  else 
    return 1;
}

int st_func_decl ( char * name ) {
  char * idType = "funcDecl";
  int h = hash (name);
  BucketList l = hashTable[h];

  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(idType, l->idType) != 0) ))
    l = l->next;
  if (l == NULL) 
    return 0;
  else 
    return 1;
}

int st_var_func_decl ( char * name ) {
  char * idType = "funcDecl";
  int h = hash (name);
  BucketList l = hashTable[h];

  while ((l != NULL) && ((strcmp (name, l->name) != 0) || strcmp (idType, l->idType) != 0))
    l = l->next;
  if (l != NULL)
    return 0;
  else
    return -1;
}

int st_duplicated ( char * name, char * scope, char * idType, int lineno) {
  int h = hash (name);
  BucketList l = hashTable[h];

  while ((l != NULL) && ((strcmp (name, l->name) != 0) || (strcmp (scope, l->scope) != 0) || (strcmp (l->idType, "param") != 0)))
    l = l->next;

  if (l != NULL)
    return 1;

  l = hashTable[h];

  while ((l != NULL) && ((strcmp (name, l->name) != 0) || (strcmp (scope, l->scope) != 0) || (strcmp (idType, l->idType) != 0)))
    l = l->next;

  if (l == NULL)
    return 0;
  else if (l->lines->next != NULL){
    if (l->lines->lineno == lineno)
      return 0;
    else 
      return 1;
  }
  return 0;
}

int st_return (char * name, char * scope, int lineno){ 
  int h = hash(name);
  char * idType = "funcDecl";
  char * datatype = "void";

  BucketList l = hashTable[h];

  if (strcmp (l->name, "input") == 0)
    return 0;
  if (strcmp (l->name, "output") == 0)
    return 1;
  if (strcmp (l->name, "contextIn") == 0)
    return 1;
  if (strcmp (l->name, "contextOut") == 0)
    return 1;
  if (strcmp (l->name, "swapIn") == 0)
    return 1;
  if (strcmp (l->name, "swapOut") == 0)
    return 1;
  if (strcmp (l->name, "execSo") == 0)
    return 1;
  if (strcmp (l->name, "execProcess") == 0)
    return 1;
  if (strcmp (l->name, "checkProcess") == 0)
    return 0;

  if (l != NULL){
    while ((l != NULL) && ((strcmp(idType,l->idType) != 0))){
      l = l->next;
    }
  }
  if (l == NULL) 
    return 1;
  else{
    if ((strcmp(datatype,l->dataType)) == 0)
      return 1;
    else 
      return 0;
  }
}

int st_main_decl () {
  char * name = "main";
  char * idType = "funcDecl";
  int h = hash(name);
  BucketList l =  hashTable[h];

  while ((l != NULL) && ((strcmp(name,l->name) != 0) || (strcmp(idType,l->idType) != 0)))
    l = l->next;
  if (l == NULL) 
    return 0;
  else 
    return 1;
}

int memLocVar (char * name, char * scope) {
  int h = hash(name);
  BucketList l =  hashTable[h];

  while ((l != NULL) && (((strcmp (l->name, name) != 0) || ((strcmp (l->scope, scope) != 0) && (strcmp (l->scope, "global") != 0))) ||  
        ((strcmp (l->idType, "varDecl") != 0)) && (strcmp (l->idType, "param") != 0)))
    l = l->next;
  if (l == NULL)
    return -1;
  else 
    return l->memloc;
}

int memLocParam (char * scope) {
  int n = 0, min = 0;
  int memLocs[10];

  for (int j=0; j<SIZE; ++j) {
    if (hashTable[j] != NULL) {
      BucketList aux = hashTable[j];
      while (aux != NULL) {
        if (strcmp (aux->scope, scope) == 0) {
          if (strcmp (aux->idType, "param") == 0) {
            memLocs[n] = aux->memloc;
            n++;
          }
        }
        aux = aux->next;
      }
    }
  }

  min = memLocs[0];

  for (int i=1; i<n; i++){
    if (min > memLocs[i])
      min = memLocs[i];
  }

  return min;
}

int isVector (char * name, char * scope) {
  int h = hash (name);
  BucketList l = hashTable[h];

  while (l != NULL && (((strcmp (l->scope, scope) != 0) && (strcmp (l->scope, "global") != 0)) || (strcmp (l->name, name) != 0)))
    l = l->next;
  if (l != NULL) {
    if (l->vector != -1) {
      return 1;
    }
  }

  return 0;
}

int isParam (char * name, char * scope) {
  int h = hash (name);
  BucketList l = hashTable[h];

  while (l != NULL && ((strcmp (l->scope, scope) != 0) || (strcmp (l->name, name) != 0) || strcmp (l->idType, "param")))
    l = l->next;
  if (l != NULL) {
    return 1;
  }

  return 0;
}  

/* Prints a formatted listing of the symbol table contents to the listing file */
void printSymTab(FILE * listing) {
  int i, j;

  scopeList[0].name = "global";
  scopeList[0].type = "-";

  for (i=0; i<scopeListSize; i++) {
    printf("\033[1;33m");
    fprintf(listing, "SCOPE: %s \t TYPE: %s\n\n", scopeList[i].name, scopeList[i].type);
    printf("\033[0m");
    fprintf(listing, "-------------  --------  ---------  -------  ------------\n");
    fprintf(listing, "Variable Name  Id Type   Data Type  Mem Loc  Line Numbers\n");
    fprintf(listing, "-------------  --------  ---------  -------  ------------\n");
    for (j=0; j<SIZE; ++j) {
      if (hashTable[j] != NULL) {
        BucketList aux = hashTable[j];
        while (aux != NULL) {
          if (strcmp (aux->scope, scopeList[i].name) == 0) {
            LineList t = aux->lines;

            if (aux->vector == -1)
              fprintf (listing, "%-15s", aux->name);
            else if (aux->vector == -2)
              fprintf (listing, "%s[]%-12s", aux->name, "");
            else
              fprintf (listing, "%s[%d]%-11s", aux->name, aux->vector, "");

            fprintf (listing, "%-10s", aux->idType);
            fprintf (listing, "%-11s", aux->dataType);

            if (strcmp (aux->idType, "varDecl") == 0 || strcmp (aux->idType, "param") == 0 )
              fprintf(listing, "%-9d", aux->memloc);
            else
              fprintf(listing, "%-9s", "-" );

            while (t != NULL) {
              fprintf(listing,"%d ", t->lineno);
              t = t->next;
            }
            fprintf(listing,"\n");
          }
          
          aux = aux->next;
        }
      }
    }
    fprintf(listing,"\n\n");
  }
}
