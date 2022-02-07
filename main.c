/****************************************************/
/* Main program for TINY compiler                   */
/****************************************************/

#include "globals.h"

#define NO_PARSE FALSE
#define NO_ANALYZE FALSE
#define NO_CODE FALSE
#define NO_INTERMEDIATE FALSE
#define NO_ASSEMBLY FALSE
#define NO_BINARY FALSE

#include "util.h"
#if NO_PARSE
  #include "scan.h"
#else
  #include "parse.h"
#if !NO_ANALYZE
  #include "analyze.h"
#if !NO_CODE
  #include "cgen.h"
  #include "assembly.h"
  #include "binary.h"
#endif
#endif
#endif

int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;		
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

int main( int argc, char * argv[] )
{ TreeNode * syntaxTree;
  char pgm[120]; /* source code file name */
  if (argc != 2)
    { fprintf(stderr,"usage: %s <filename>\n",argv[0]);
      exit(1);
    }
  strcpy(pgm,argv[1]) ;
  if (strchr (pgm, '.') == NULL)
     strcat(pgm,".tny");
  source = fopen(pgm,"r");
  if (source==NULL)
  { fprintf(stderr,"File %s not found\n",pgm);
    exit(1);
  }
  listing = stdout; /* send listing to screen */
  printf("\033[1;36m");
  fprintf(listing,"\nC- COMPILATION: %s\n",pgm);
  printf("\033[0m");
if (TraceScan) {
  printf("\033[1;36m");
  fprintf(listing,"\n------ LEXYCAL ANALYSIS ------\n");
  printf("\033[0m");
}

#if NO_PARSE 
  while (getToken()!=ENDFILE);
#else
  syntaxTree = parse();
  if (TraceParse) {
    printf("\033[1;36m");
    fprintf(listing,"\n------ SYNTAX TREE ------\n");
    printf("\033[0m");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE
  if (! Error)
  { 
    buildSymtab(syntaxTree);
    typeCheck(syntaxTree);
  }
#if !NO_CODE
  if (! Error)
  { char * codefile;
    int fnlen = strcspn(pgm,".");
    codefile = (char *) calloc(fnlen+4, sizeof(char));
    strncpy(codefile,pgm,fnlen);
    strcat(codefile,"_IC.txt");
    code = fopen(codefile,"w");
    if (code == NULL) { 
      printf("Unable to open %s\n",codefile);
      exit(1);
    }
    if ( !NO_INTERMEDIATE ) {
      printf("\033[1;36m");
      fprintf(listing,"\n------ INTERMEDIATE CODE ------\n");
      printf("\033[0m");
      genIntermediate (syntaxTree,codefile);

      if ( !NO_ASSEMBLY) {
        printf("\033[1;36m");
        fprintf(listing,"\n------ ASSEMBLY CODE ------\n");
        printf("\033[0m");
        genAssembly (retIntermediate ());
      
        if ( !NO_BINARY ){
          printf("\033[1;36m");
          fprintf(listing,"\n------ BINARY CODE ------\n");
          printf("\033[0m");
          printBinary (retAssembly ());
        }
      }
    }
    fclose(code);
  }
#endif
#endif
#endif
  fclose(source);
  return 0;
}

