#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include <string.h>

#define TEMP_ZERO 0

#define TEMP_RD 1
#define TEMP_RS 2
#define TEMP_RT 3

#define TEMP_CP 27
#define TEMP_ENDP 28
#define TEMP_RET 29
#define TEMP_SP 30
#define TEMP_RA 31

#define PROGRAM_ADDRESS 100

const char * OpKindNames[] = {  "add", "sub", "mult", "div", 
								"eq", "dif", "lt", "let", "gt", "get", "and", "or", 
								"func", "end", "call", "iff", "li", "lw", "sw", "move",
								"ret", "input", "output", "param", "label", "goto", "halt",
								"cntxi", "cntxo", "swapi", "swapo",
								"execs", "execp", "endp", "getp"
							};

QuadList * quadList;

int nTemp = 4;
int nLabel = 1;

static void initQuadList () {
	quadList->size = 0;
	quadList->head = NULL;
	quadList->tail = NULL;
}

static void insertQuad (OpKind op, Operand rs, Operand rt, Operand rd) {
	Quad * newQuad = (Quad *) malloc (sizeof (Quad));
	newQuad->op = op;
	newQuad->rs = rs;
	newQuad->rt = rt;
	newQuad->rd = rd;
	newQuad->next = NULL;

	if (quadList->size == 0) {
		quadList->head = newQuad;
		quadList->tail = newQuad;
	}
	else {
		quadList->tail->next = newQuad;
		quadList->tail = newQuad;
	}

	quadList->size++;
}

Operand newOperand_Null () {
	Operand op;

	op.kind = Null;
	op.Content.name = NULL;

	return op;
}

Operand newOperand_Const (int value) {
	Operand op;

	op.kind = Const;
	op.Content.value = value;

	return op;
}

Operand newOperand_String (char * name) {
	Operand op;
	op.Content.name = (char *) malloc (strlen (name) * sizeof (char));

	op.kind = String;
	op.Content.name = name;

	return op;
}

char * newTemp (int regTemp) {
	char * temp = (char *) malloc (6 * sizeof (char));

	if (regTemp == 27) {
		sprintf (temp, "$pc");
	}
	else if (regTemp == 28) {
		sprintf (temp, "$end");
	}
	else if (regTemp == 29){
		sprintf (temp, "$ret");
	} 
	else if (regTemp == 30){
		sprintf (temp, "$sp");
	} 
	else if (regTemp == 31){
		sprintf (temp, "$ra");
	}
	else {
		sprintf (temp, "$t%d", regTemp);
	}

	return temp;
}

char * newLabel () {
	char * label = (char *) malloc (6 * sizeof (char));
	sprintf (label, "L%d", nLabel++);
	
	return label;
}

static void genStmt (TreeNode * tree) {
	TreeNode *p1, *p2, *p3;
	Operand rs, rt, rd;
	char * label, * label2;

	switch (tree->kind.exp) {

		case IfK:
			p1 = tree->child[0];
			p2 = tree->child[1];
			p3 = tree->child[2];

			label = newLabel ();

			cGen (p1);

			rs = newOperand_String (newTemp (TEMP_RD));
			rt = newOperand_String (label);
			rd = newOperand_Null ();
			insertQuad (opIFF, rs, rt, rd);

			cGen (p2);

			if (p3 != NULL) {
				label2 = newLabel ();

				rs = newOperand_String (label2);
				rt = newOperand_Null ();
				rd = newOperand_Null ();
				insertQuad (opGOTO, rs, rt, rd);
			}

			rs = newOperand_String (label);
			rt = newOperand_Null ();
			rd = newOperand_Null ();
			insertQuad (opLABEL, rs, rt, rd);

			if (p3 != NULL) {
				cGen (p3);

				rs = newOperand_String (label2);
				rt = newOperand_Null ();
				rd = newOperand_Null ();
				insertQuad (opLABEL, rs, rt, rd);
			}

			break;

		case WhileK:
			p1 = tree->child[0];
			p2 = tree->child[1];

			label = newLabel ();

			rs = newOperand_String (label);
			rt = newOperand_Null ();
			rd = newOperand_Null ();
			insertQuad (opLABEL, rs, rt, rd);

			cGen (p1);

			label2 = newLabel ();

			rs = newOperand_String (newTemp (TEMP_RD));
			rt = newOperand_String (label2);
			rd = newOperand_Null ();
			insertQuad (opIFF, rs, rt, rd);

			cGen (p2);

			rs = newOperand_String (label);
			rt = newOperand_Null ();
			rd = newOperand_Null ();
			insertQuad (opGOTO, rs, rt, rd);

			rs = newOperand_String (label2);
			rt = newOperand_Null ();
			rd = newOperand_Null ();
			insertQuad (opLABEL, rs, rt, rd);

			break;

		case AssignK:
			p1 = tree->child[0];
			p2 = tree->child[1];

			int memLoc = memLocVar (p1->name, p1->scope);

			if (p1->child[0] != NULL) {
				cGen (p1->child[0]);

				rs = newOperand_String (newTemp (TEMP_RD));
				rt = newOperand_String (newTemp (nTemp++));
				rd = newOperand_Null ();
				insertQuad (opMOVE, rs, rt, rd);

				cGen (p2);

				int param = isParam (p1->name, p1->scope);

				if (param) {
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					rs = newOperand_String (newTemp (nTemp-1));
					rt = newOperand_String (newTemp (TEMP_RS));
					rd = newOperand_String (newTemp (nTemp-1));
					insertQuad (opADD, rs, rt, rd);

					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (--nTemp));
					rd = newOperand_Const (0);
					insertQuad (opSW, rs, rt, rd);
				}
				else {
					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (--nTemp));
					rd = newOperand_Const (memLoc);
					insertQuad (opSW, rs, rt, rd);
				}
				
			}
			else {
				cGen (p2);

				rs = newOperand_String (newTemp (TEMP_RD));
				rt = newOperand_String (newTemp (TEMP_ZERO));
				rd = newOperand_Const (memLoc);
				insertQuad (opSW, rs, rt, rd);
			}

			break;

		case ReturnK:
			p1 = tree->child[0];

			if (p1 != NULL) {
				cGen (p1);

				rs = newOperand_String (newTemp (TEMP_RD));
				rt = newOperand_Null ();
				rd = newOperand_Null ();
				insertQuad (opRET, rs, rt, rd);
			}

			break;

		default:
			break;
	}
}

static void genExp (TreeNode * tree) {
	TreeNode *p1, *p2;
	Operand rs, rt, rd;

	switch (tree->kind.exp) {
		case TypeK:
			break;
		case OpK:
			p1 = tree->child[0];
			p2 = tree->child[1];

			cGen (p1);

			rs = newOperand_String (newTemp (TEMP_RD));
			rt = newOperand_String (newTemp (nTemp++));
			rd = newOperand_Null ();
			insertQuad (opMOVE, rs, rt, rd);

			cGen (p2);

			rs = newOperand_String (newTemp (TEMP_RD));
			rt = newOperand_String (newTemp (TEMP_RT));
			insertQuad (opMOVE, rs, rt, rd);

			rs = newOperand_String (newTemp (--nTemp));
			rt = newOperand_String (newTemp (TEMP_RS));
			insertQuad (opMOVE, rs, rt, rd);

			rs = newOperand_String (newTemp (TEMP_RS));
			rt = newOperand_String (newTemp (TEMP_RT));
			rd = newOperand_String (newTemp (TEMP_RD));

			switch (tree->attr.op) {
				case PLUS:
					insertQuad (opADD, rs, rt, rd);
					break;
				case MINUS:
					insertQuad (opSUB, rs, rt, rd);
					break;
				case TIMES:
					insertQuad (opMULT, rs, rt, rd);
					break;
				case OVER:
					insertQuad (opDIV, rs, rt, rd);
					break;
				case EQ:
					insertQuad (opEQ, rs, rt, rd);
					break;
				case DIF:
					insertQuad (opDIF, rs, rt, rd);
					break;
				case LT:
					insertQuad (opLT, rs, rt, rd);
					break;
				case LET:
					insertQuad (opLET, rs, rt, rd);
					break;
				case GT:
					insertQuad (opGT, rs, rt, rd);
					break;
				case GET:
					insertQuad (opGET, rs, rt, rd);
					break;
				default:
					break;
			}
			break;

		case ConstK:
			rs = newOperand_Const (tree->attr.val);
			rt = newOperand_String (newTemp(TEMP_RD));
			rd = newOperand_Null ();
			insertQuad (opLI, rs, rt, rd);
			break;

		case IdK:
			p1 = tree->child[0];

			int memLoc = memLocVar (tree->name, tree->scope);

			if (p1 != NULL) {
				cGen (p1);

				int param = isParam (tree->name, tree->scope);
			
				if (param) {
					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opMOVE, rs, rt, rd);

					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (TEMP_RS));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opADD, rs, rt, rd);

					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (TEMP_RD));
					rd = newOperand_Const (0);
					insertQuad (opLW, rs, rt, rd);
				}
				else {
					rs = newOperand_String (newTemp (TEMP_RD));
					rt = newOperand_String (newTemp (TEMP_RD));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);
				}
			}
			else {
				rs = newOperand_String (newTemp (TEMP_RD));
				rt = newOperand_String (newTemp (TEMP_ZERO));
				rd = newOperand_Const (memLoc);
				insertQuad (opLW, rs, rt, rd);
			}
			break;

		case ActivK:
			if (strcmp (tree->attr.name, "input") == 0) {
				rs = newOperand_Null ();
				rt = newOperand_String (newTemp(TEMP_RD));
				rd = newOperand_Null ();
				insertQuad (opIN, rs, rt, rd);
			}
			else if (strcmp (tree->attr.name, "output") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				rs = newOperand_Null ();
				rt = newOperand_Null ();
				rd = newOperand_Const (memLoc);
				insertQuad (opOUT, rs, rt, rd);
			}
			else if (strcmp (tree->attr.name, "contextIn") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);
				
				if (p1 != NULL) {
					//LW na variavel do parametro
					//RT = Mem [RS + offset]
					//rt = pID
					rs = newOperand_String (newTemp (TEMP_RT));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					//LI valor 100
					//RT = imediato
					//rs = 100
					rs = newOperand_Const (PROGRAM_ADDRESS);
					rt = newOperand_String (newTemp(TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opLI, rs, rt, rd);

					//multiplica pelo deslocamento de memória
					//RD = RT * RS
					//rd = pID * 100
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_RT));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opMULT, rs, rt, rd);
				}

				rt = newOperand_String (newTemp (TEMP_RD));

				//itera sobre 32 registradores (posicoes 0 a 31)
				for(int i = 0; i < 32; i++){
					rs = newOperand_String (newTemp (i));
					rd = newOperand_Const (i);
					insertQuad (opCNTXT_IN, rs, rt, rd);
				}
			}
			else if (strcmp (tree->attr.name, "contextOut") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				if (p1 != NULL) {
					//LW na variavel do parametro
					//RT = Mem [RS + offset]
					//rt = pID
					rs = newOperand_String (newTemp (TEMP_RT));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					//LI valor 100
					//RT = imediato
					//rs = 100
					rs = newOperand_Const (PROGRAM_ADDRESS);
					rt = newOperand_String (newTemp(TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opLI, rs, rt, rd);

					//multiplica pelo deslocamento de memória
					//RD = RT * RS
					//rd = pID * 100
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_RT));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opMULT, rs, rt, rd);
				}

				rt = newOperand_String (newTemp (TEMP_RD));
				
				//itera sobre 32 registradores (posicoes 0 a 31)
				for(int i = 0; i < 32; i++){
					rs = newOperand_String (newTemp (i));
					rd = newOperand_Const (i);
					insertQuad (opCNTXT_OUT, rs, rt, rd);
				}
			}
			else if (strcmp (tree->attr.name, "swapIn") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				if (p1 != NULL) {
					//LW na variavel do parametro
					//RT = Mem [RS + offset]
					//rt = pID
					rs = newOperand_String (newTemp (TEMP_RT));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					//LI valor 100
					//RT = imediato
					//rs = 100
					rs = newOperand_Const (PROGRAM_ADDRESS);
					rt = newOperand_String (newTemp(TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opLI, rs, rt, rd);

					//multiplica pelo deslocamento de memória
					//RD = RT * RS
					//rd = pID * 100
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_RT));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opMULT, rs, rt, rd);
				}

				rt = newOperand_String (newTemp (TEMP_RD));
				
				//gera offset de 0 a 31
				for(int i = 0; i < 32; i++){
					rs = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (i);
					insertQuad (opSWAP_IN, rs, rt, rd);
				}
			}
			else if (strcmp (tree->attr.name, "swapOut") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				if (p1 != NULL) {
					//LW na variavel do parametro
					//RT = Mem [RS + offset]
					//rt = pID
					rs = newOperand_String (newTemp (TEMP_RT));
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					//LI valor 100
					//RT = imediato
					//rs = 100
					rs = newOperand_Const (PROGRAM_ADDRESS);
					rt = newOperand_String (newTemp(TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opLI, rs, rt, rd);

					//multiplica pelo deslocamento de memória
					//RD = RT * RS
					//rd = pID * 100
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_RT));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opMULT, rs, rt, rd);
				}

				rt = newOperand_String (newTemp (TEMP_RD));
				
				//gera offset de 0 a 31
				for(int i = 0; i < 32; i++){
					rs = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (i);
					insertQuad (opSWAP_OUT, rs, rt, rd);
				}
			}
			else if (strcmp (tree->attr.name, "execSo") == 0) {
				rs = newOperand_Null ();
				rt = newOperand_Null ();
				rd = newOperand_Null ();
				insertQuad (opEXEC_SO, rs, rt, rd);
			}
			else if (strcmp (tree->attr.name, "execProcess") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				rs = newOperand_Null ();
				rt = newOperand_Null ();
				rd = newOperand_Const (memLoc);
				insertQuad (opEXEC_PROCESS, rs, rt, rd);
			}
			else if (strcmp (tree->attr.name, "checkProcess") == 0) {
				p1 = tree->child[0];

				int memLoc = memLocVar (p1->name, p1->scope);

				if (p1 != NULL) {
					//LW na variavel do parametro
					//RT = Mem [RS + offset]
					//rt = pID
					//tava RS (errado), mudei pra > RT <
					rs = newOperand_String (newTemp (TEMP_RT));
					//tava temp RT (errado eu acho); > TEMP_ZERO <
					rt = newOperand_String (newTemp (TEMP_ZERO));
					rd = newOperand_Const (memLoc);
					insertQuad (opLW, rs, rt, rd);

					//LI valor 100
					//RT = imediato
					//rs = 100
					rs = newOperand_Const (PROGRAM_ADDRESS);
					rt = newOperand_String (newTemp(TEMP_RS));
					rd = newOperand_Null ();
					insertQuad (opLI, rs, rt, rd);

					//multiplica pelo deslocamento de memória
					//RD = RT * RS
					//rd = pID * 100
					rs = newOperand_String (newTemp (TEMP_RS));
					rt = newOperand_String (newTemp (TEMP_RT));
					rd = newOperand_String (newTemp (TEMP_RD));
					insertQuad (opMULT, rs, rt, rd);
				}
				//TEMP_RS mudei pra > TEMP_RD <
				rs = newOperand_String (newTemp (TEMP_RD));
				rt = newOperand_String (newTemp (TEMP_RD));
				rd = newOperand_Const (28);

				insertQuad (opGET_PROCESS, rs, rt, rd);
			}
			else {
				char * aux;
				int params = 0;
				int memLoc = 0;
				int vetLoc = 0;
				int vetKind = 0;

				p1 = tree->child[0];

				if (p1 != NULL) 
					memLoc = memLocParam (tree->name);

				while (p1 != NULL) {
					if (params == 0) {
						aux = newTemp (nTemp);
					}

					if (p1->kind.exp != ConstK) {
						if (isVector (p1->name, tree->attr.name)){
							vetLoc = memLocVar (p1->name, p1->scope);

							rs = newOperand_Const (vetLoc);
							rt = newOperand_String (newTemp (TEMP_RD));
							rd = newOperand_Null ();
							insertQuad (opLI, rs, rt, rd);
						} 
						else {
							genExp (p1);
						} 
					}
					else 
						genExp (p1);

					rs = newOperand_Const (memLoc++);
					rt = newOperand_Null ();
					rd = newOperand_Null ();
					insertQuad (opPARAM, rs, rt, rd);

					params++;	
					p1 = p1->sibling;
				}

				rs = newOperand_String (tree->attr.name);
				rt = newOperand_Null ();
				rd = newOperand_Null ();
				insertQuad (opCALL, rs, rt, rd);
			}
			break;

		case FuncK:
			rs = newOperand_String (tree->attr.name);
			rt = newOperand_Null ();
			rd = newOperand_Null ();

      		if (strcmp(tree->attr.name, "input") != 0 && strcmp(tree->attr.name, "output") != 0
      			&& strcmp(tree->attr.name, "contextIn") != 0 && strcmp(tree->attr.name, "contextOut") != 0
      			&& strcmp(tree->attr.name, "swapIn") != 0 && strcmp(tree->attr.name, "swapOut") != 0
				&& strcmp(tree->attr.name, "execSo") != 0 && strcmp(tree->attr.name, "execProcess") != 0
				&& strcmp(tree->attr.name, "checkProcess") != 0)
      			insertQuad (opFUNC, rs, rt, rd);

      		p1 = tree->child[0];
			cGen (p1);

			p2 = tree->child[1];
			cGen (p2);

			insertQuad (opEND, rs, rt, rd);
			break;
		default:
			break;
	}
}

static void printICode () {
	Quad * quad = quadList->head;

	for (int i=0; i<quadList->size; i++) {
		printf ("\033[1;33m");
		printf ("%s,\t", OpKindNames[quad->op]);
		printf ("\033[0m");

		switch (quad->rs.kind) {

			case String:
				printf("%s", quad->rs.Content.name);
				break;

			case Const:
				printf("%d", quad->rs.Content.value);
				break;

			case Null:
				printf("-");
				break;

			default:
				break;
		}
		printf(",\t ");
		switch (quad->rt.kind) {
			case String:
				printf("%s", quad->rt.Content.name);
				break;
			case Const:
				printf("%d", quad->rt.Content.value);
				break;
			case Null:
				printf("-");
				break;
			default:
				break;
		}
		printf(",\t ");
		switch (quad->rd.kind) {
			case String:
				printf("%s", quad->rd.Content.name);
				break;
			case Const:
				printf("%d", quad->rd.Content.value);
				break;
			case Null:
				printf("-");
				break;
			default:
				break;
		}
		printf("\n");

		quad = quad->next;
	}
}

static void cGen (TreeNode * tree) {
	if (tree != NULL) {
		switch (tree->nodekind) {
			case StmtK:
				genStmt (tree);
				break;
			case ExpK:
				genExp (tree);
				break;
			default:
				break;
		}
		cGen (tree->sibling);
	}
}

void genIntermediate (TreeNode *syntaxTree, char * codefile) {
	char * s = malloc (strlen (codefile) + 7);
	strcpy (s, "File: ");
	strcat (s, codefile);
	quadList = (QuadList *) malloc (sizeof (QuadList));
	initQuadList ();
	cGen (syntaxTree);
	Operand null = newOperand_Null ();

	//sysCall para termino do processo
	insertQuad (opEND_PROCESS, newOperand_Const (1), newOperand_String (newTemp(TEMP_ENDP)), newOperand_Null ());

	insertQuad (opHLT, null, null, null);
	printICode ();
}

Quad * retIntermediate () {
	return quadList->head;
}