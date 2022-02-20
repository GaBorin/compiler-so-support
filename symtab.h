/****************************************************/
/* Symbol table interface for the TINY compiler     */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* Inserts line numbers and memory locations into the symbol table
 * loc = memory location is inserted only the first time, otherwise ignored */
void st_insert ( char * name, char * dataType, char * idType, char * scope, int vector, int declared, int lineno, int loc );

/* Rreturns the memory location of a variable or -1 if not found */
int st_lookup ( char * name, char * scope );

/* Checks if the variable has been declared */
int st_var_decl (char * name, char * scope );

int st_func_decl ( char * name );

int st_var_func_decl (char *name);

int st_duplicate ( char * name, char * scope, char * idType, int lineno );

/* Checks if main function is declared */
int st_main_decl ();

int memLocVar (char * name, char * scope);

int memLocParam (char * scope);

/* Prints a formatted listing of the symbol table contents to the listing file */
void printSymTab(FILE * listing);

#endif
