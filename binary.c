#include "globals.h"
#include "symtab.h"
#include "cgen.h"
#include "assembly.h"
#include "binary.h"
#include "stdio.h"

const char * OpCodeNames[] =  { "add", "addi", "sub", "subi", "mult", "div", "and", "andi", "or", "ori", "xor", "xori",
								"slt", "slet", "sgt", "sget", "beq", "bnq", "blt", "blet", "bgt", "bget", 
								"bltz", "blez", "bgtz", "bgez", "srl", "sll", "li", "lw", "sw",
								"not", "move", "jump", "jal", "jr", "nop", "halt", "in", "out",
								"ctxi", "ctxo", "swpi", "swpo", "exso", "expr", "endp", "getp"};  

char * getBin (int im, int size) {
    int i = 0;
    char * bin = (char *) malloc(size + 2);
    size --;
    for (unsigned bit = 1u << size; bit != 0; bit >>= 1) {
        bin[i++] = (im & bit) ? '1' : '0';
    }
    bin[i] = '\0';
    return bin;
}

void printBinary (Line * line) {
	Instruction inst; 
	FILE *res;

	res = fopen("result.txt", "w");
	

	while (line != NULL) {
		switch (line->kind) {
			case funcK:
				printf ("\033[1;33m");
				printf ("// %s\n", line->Kind.name);
				fprintf (res, "// %s\n", line->Kind.name);
				printf ("\033[0m");
				break;
			case labelK:
				printf ("\033[1;33m");
				printf ("// %s\n", line->Kind.name);
				fprintf (res, "// %s\n", line->Kind.name);
				printf ("\033[0m");
				break;
			case instructionK:
				inst = line->Kind.instruction;

				printf ("Memory[%d] = 32'b", line->lineno);
				fprintf (res, "Memory[%d] = 32'b", line->lineno);
				switch (inst.format) {
					case typeR:
						printf ("%s_%s_%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg2, 5), getBin (inst.reg3, 5), getBin (inst.reg1, 5), "00000000000");
						fprintf (res, "%s_%s_%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg2, 5), getBin (inst.reg3, 5), getBin (inst.reg1, 5), "00000000000");
						break;
					case typeI:
						printf ("%s_%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg1, 5), getBin (inst.reg2, 5), getBin (inst.imediate, 16));
						fprintf (res, "%s_%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg1, 5), getBin (inst.reg2, 5), getBin (inst.imediate, 16));
						break;
					case typeJ:
						printf ("%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg1, 5), getBin (inst.imediate, 21));
						fprintf (res, "%s_%s_%s;", getBin (inst.opCode, 6), getBin (inst.reg1, 5), getBin (inst.imediate, 21));
						break;
					default:
						break;
				}
				printf ("\t//%s\n", OpCodeNames[inst.opCode]);
				fprintf (res, "\t//%s\n", OpCodeNames[inst.opCode]);
				break;
			default:
				break;
		}

		line = line->next;
	}

	fclose(res);
}