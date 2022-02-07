#ifndef _CGEN_H_
#define _CGEN_H_

typedef enum {
	opADD, opSUB, opMULT, opDIV, 
	opEQ, opDIF, opLT, opLET, opGT, opGET, opAND, opOR, 
	opFUNC, opEND, opCALL, opIFF, opLI, opLW, opSW, opMOVE,
	opRET, opIN, opOUT, opPARAM, opLABEL, opGOTO, opHLT,
	opCNTXT_IN, opCNTXT_OUT, opSWAP_IN, opSWAP_OUT, 
	opEXEC_SO, opEXEC_PROCESS, opEND_PROCESS, opGET_PROCESS
} OpKind;

typedef enum {
	String, Const, Null
} OperandKind;

typedef struct Operand{
	OperandKind kind;
	union {
		int value;
		char * name;
	} Content;
} Operand;

typedef struct Quad{
	OpKind op;
	Operand rs, rt, rd;
	struct Quad * next;
} Quad;

typedef struct QuadList{
	int size;
	Quad * head;
	Quad * tail;
} QuadList;

static void cGen (TreeNode * tree);
void genIntermediate (TreeNode *syntaxtree, char * codefile);
Quad * retIntermediate ();

#endif