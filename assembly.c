#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "assembly.h"

const char * InstNames[] =  { "add", "addi", "sub", "subi", "mult", "div", "and", "andi", "or", "ori", "xor", "xori",
							"slt", "slet", "sgt", "sget", "beq", "bnq", "blt", "blet", "bgt", "bget", 
							"bltz", "blez", "bgtz", "bgez", "srl", "sll", "li", "lw", "sw",
							"not", "move", "jump", "jal", "jr", "nop", "halt", "in", "out",
							"ctxi", "ctxo", "swpi", "swpo", "exso", "expr", "endp", "getp"};  

const char * RegNames[] = { "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8",
							"$t9", "$t10", "$t11", "$t12", "$t13", "$t14", "$t15", "$t16",
							"$t17", "$t18", "$t19", "$t20", "$t21", "$t22", "$t23", "$t24",
							"$t25", "$t26", "$pc", "$end", "$ret", "$sp", "$ra"};

AssemblyCode * assemblyCode; 

int line = 0;
int inMain = 0;

static void initAssemblyCode () {
	assemblyCode->size = 0;
	assemblyCode->head = NULL;
	assemblyCode->tail = NULL;
}

void insertLine (Line * new) {
	if (assemblyCode->size == 0) {
		assemblyCode->head = new;
		assemblyCode->tail = new;
	}
	else {
		assemblyCode->tail->next = new;
		assemblyCode->tail = new;
	}

	assemblyCode->size++;
}

void createLabel (char * name, FormatKind kind) {
	Line * new = (Line *) malloc (sizeof (Line));

	new->lineno = line;
	new->kind = kind;
	new->Kind.name = name;
	new->next = NULL;

	insertLine (new);
}

void createInstruction (FormatKind format, InstKind opCode, Reg reg1, Reg reg2, Reg reg3, int imediate, char * b_label) {
	Instruction inst;

	inst.format = format;
	inst.opCode = opCode;
	inst.reg1 = reg1;
	inst.reg2 = reg2;
	inst.reg3 = reg3;
	inst.imediate = imediate;
	inst.b_label = b_label;

	Line * new = (Line *) malloc (sizeof (Line));

	new->lineno = line;
	new->kind = instructionK;
	new->Kind.instruction = inst;
	new->next = NULL;

	insertLine (new);

	line++;
}

void instFormatR (InstKind opCode, Reg reg1, Reg reg2, Reg reg3) {
	createInstruction (typeR, opCode, reg1, reg2, reg3, 0, NULL);
}

void instFormatI (InstKind opCode, Reg reg1, Reg reg2, int imediate, char * b_label) {
	createInstruction (typeI, opCode, reg1, reg2, $t0, imediate, b_label);
}

void instFormatJ (InstKind opCode, Reg reg, int imediate) {
	createInstruction (typeJ, opCode, reg, $t0, $t0, imediate, NULL);
}

int labelLine (char * name) {
	Line * l = assemblyCode->head;

	while (l != NULL) {
		if (((l->kind == labelK) || (l->kind == funcK)) && (strcmp (l->Kind.name, name) == 0))
			return l->lineno;

		l = l->next;
	}

	return -1;
}

void initCode () {
	instFormatJ (i_JUMP, $t0, -1);
	Line * a = assemblyCode->tail;
	a->Kind.instruction.b_label = "main";
}

Reg getReg (char * regName) {

	for (int i=0; i<N_REG; i++) {
		if (strcmp (regName, RegNames[i]) == 0)
			return (Reg) i;
	}

	return $t0;
}

void genInstructions (Quad * quad) {
	Quad * q = quad;
	Operand rs, rt, rd;

	while (q != NULL) {

		rs = q->rs;
		rt = q->rt;
		rd = q->rd;

		switch (q->op) {
			case opADD:
				instFormatR (i_ADD, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opSUB:
				instFormatR (i_SUB, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opMULT:
				instFormatR (i_MULT, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opDIV:
				instFormatR (i_DIV, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opEQ:
				break;

			case opDIF:
				break;

			case opLT:
				instFormatR (i_SLT, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opLET:
				instFormatR (i_SLET, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opGT:
				instFormatR (i_SGT, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opGET:
				instFormatR (i_SGET, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opAND:
				instFormatR (i_AND, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opOR:
				instFormatR (i_OR, getReg (rd.Content.name), getReg (rs.Content.name), getReg (rt.Content.name));
				break;

			case opFUNC:
				createLabel (rs.Content.name, funcK);
				if (strcmp (rs.Content.name, "main") == 0)
					inMain = 1;
				break;

			case opEND:
				if (strcmp (rs.Content.name, "main") != 0) {
					instFormatJ (i_JR, $ra, 0);
				}
				break;

			case opCALL:
				if (inMain == 0) {
					instFormatI (i_ADDI, $sp, $sp, -1, NULL);
					instFormatI (i_SW, $sp, $ra, 0, NULL);
					instFormatJ (i_JAL, $t0, labelLine (rs.Content.name));
					instFormatI (i_LW, $sp, $ra, 0, NULL);
					instFormatI (i_ADDI, $sp, $sp, 1, NULL);
				}
				else 
					instFormatJ (i_JAL, $t0, labelLine (rs.Content.name));
				break;

			case opIFF:
				instFormatI (i_BEQ, getReg (rs.Content.name), getReg (rt.Content.name), 0, rt.Content.name);
				break;

			case opLI:
				instFormatI (i_LI, $t0, getReg (rt.Content.name), rs.Content.value, NULL);
				break;

			case opLW:
				instFormatI (i_LW, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opSW:
				instFormatI (i_SW, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opMOVE:
				instFormatI (i_MOVE, getReg (rs.Content.name), getReg (rt.Content.name), 0, NULL);
				break;

			case opRET:
				instFormatI (i_MOVE, getReg (rs.Content.name), $ret, 0, NULL);
				break;

			case opIN:
				instFormatI (i_IN, $t0, getReg (rt.Content.name), 0, NULL);
				break;

			case opOUT:
				instFormatJ (i_OUT, $t0, rd.Content.value);
				break;

			case opPARAM:
				instFormatI (i_SW, $t0, $t1, rs.Content.value, NULL);
				break;

			case opLABEL:
				createLabel (rs.Content.name, labelK);
				break;

			case opGOTO:
				instFormatJ (i_JUMP, $t0, labelLine (rs.Content.name));
				break;

			case opHLT:
				instFormatJ (i_HALT, $t0, 0);
				break;

			case opCNTXT_IN:
				instFormatI (i_CNTXT_IN, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opCNTXT_OUT:
				instFormatI (i_CNTXT_OUT, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opSWAP_IN:
				instFormatI (i_SWAP_IN, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opSWAP_OUT:
				instFormatI (i_SWAP_OUT, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;

			case opEXEC_SO:
				instFormatJ (i_EXEC_SO, $t0, 0);
				break;

			case opEXEC_PROCESS:
				instFormatJ (i_EXEC_PROCESS, $t0, rd.Content.value);
				break;

			case opEND_PROCESS:
				instFormatI (i_END_PROCESS, $t0, getReg (rt.Content.name), rs.Content.value, NULL);//similar ao LI
				break;

			case opGET_PROCESS:
				instFormatI (i_GET_PROCESS, getReg (rt.Content.name), getReg (rs.Content.name), rd.Content.value, NULL);
				break;
		}

		q = q->next;
	}
}

void resetIm () {
	Line * l = assemblyCode->head;
	Instruction inst;

	while (l != NULL) {
		inst = l->Kind.instruction;

		if (l->Kind.instruction.opCode == i_BEQ || ((l->Kind.instruction.opCode == i_JUMP) && l->Kind.instruction.imediate == -1)) 
			l->Kind.instruction.imediate = labelLine (l->Kind.instruction.b_label);

		l = l->next;
	}
}

void printAssembly () {
	Line * line = assemblyCode->head;
	Instruction inst;

	for (int i=0; i<assemblyCode->size; i++) {
		switch (line->kind) {
			case funcK:
				printf ("\033[1;33m");
				printf (".%s\n", line->Kind.name);
				printf ("\033[0m");
				break;
			case labelK:
				printf ("\033[1;33m");
				printf ("\t.%s\n", line->Kind.name);
				printf ("\033[0m");
				break;
			case instructionK:
				inst = line->Kind.instruction;

				printf ("%d:\t%-6s", line->lineno, InstNames[inst.opCode]);
				switch (inst.format) {
					case typeR:
						printf ("%s, %s, %s", RegNames[inst.reg1], RegNames[inst.reg2], RegNames[inst.reg3]);
						break;
					case typeI:
						if (inst.opCode == i_IN)
							printf ("%s", RegNames[inst.reg2]);
						else if (inst.opCode == i_NOT || inst.opCode == i_MOVE)
							printf ("%s, %s", RegNames[inst.reg2], RegNames[inst.reg1]);
						else if (inst.opCode == i_LI || inst.opCode == i_END_PROCESS)
							printf ("%s, %d", RegNames[inst.reg2], inst.imediate);
						else if (inst.opCode == i_LW || inst.opCode == i_SW || inst.opCode == i_CNTXT_IN || inst.opCode == i_CNTXT_OUT || inst.opCode == i_SWAP_IN || inst.opCode == i_SWAP_OUT || inst.opCode == i_GET_PROCESS)
							printf ("%s, %d(%s)", RegNames[inst.reg2], inst.imediate, RegNames[inst.reg1]);
						else 
							printf ("%s, %s, %d", RegNames[inst.reg1], RegNames[inst.reg2], inst.imediate);
						break;
					case typeJ:
						if (inst.opCode == i_JUMP || inst.opCode == i_JAL || inst.opCode == i_OUT || inst.opCode == i_EXEC_PROCESS)
							printf ("%d", inst.imediate);
						else if (inst.opCode == i_EXEC_SO){
							//nao imprime nada (sysCall: exso)
						}
						else 
							printf ("%s", RegNames[inst.reg1]);
						break;
					default:
						break;
				}
				printf ("\n");
				break;
			default:
				break;
		}

		line = line->next;
	}
}

void genAssembly (Quad * quad) {
	assemblyCode = (AssemblyCode *) malloc (sizeof (AssemblyCode));
	initAssemblyCode ();
	initCode ();
	genInstructions (quad);
	resetIm ();
	printAssembly ();
}

Line * retAssembly () {
	return assemblyCode->head;
}



