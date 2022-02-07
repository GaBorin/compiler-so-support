#define STACK_SIZE 32
#define N_REG 32
#define REG_PC 27
#define REG_END 28
#define REG_RET 29
#define REG_SP 30
#define REG_RA 31

typedef enum  { 
	i_ADD, i_ADDI, i_SUB, i_SUBI, i_MULT, i_DIV, i_AND, i_ANDI, i_OR, i_ORI, i_XOR, iXORI,
	i_SLT, i_SLET, i_SGT, i_SGET, i_BEQ, i_BNQ, i_BLT, i_BLET, i_BGT, i_BGET, 
	i_BLTZ, i_BLEZ, i_BGTZ, i_BGEZ, i_SRL, i_SLL, i_LI, i_LW, i_SW,
	i_NOT, i_MOVE, i_JUMP, i_JAL, i_JR, i_NOP, i_HALT, i_IN, i_OUT,
	i_CNTXT_IN, i_CNTXT_OUT, i_SWAP_IN, i_SWAP_OUT,
	i_EXEC_SO, i_EXEC_PROCESS, i_END_PROCESS, i_GET_PROCESS
} InstKind;

typedef enum  { 
	$t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7, $t8,
	$t9, $t10, $t11, $t12, $t13, $t14, $t15, $t16,
	$t17, $t18, $t19, $t20, $t21, $t22, $t23, $t24,
	$t25, $t26, $pc, $end, $ret, $sp, $ra
} Reg;

typedef enum {
	typeR, typeI, typeJ
} FormatKind;

typedef enum {
	instructionK, funcK, labelK
} LineKind;

typedef struct Instruction{
	FormatKind format;
	InstKind opCode;
	Reg reg1;
	Reg reg2;
	Reg reg3;
	int imediate;
	char * b_label;
} Instruction;

typedef struct Line{
	int lineno;
	LineKind kind;
	union {
		Instruction instruction;
		char * name; 
	} Kind;
	struct Line * next;
} Line;

typedef struct AssemblyCode {
	int size;
	Line * head;
	Line * tail;
} AssemblyCode;

void genAssembly (Quad * quad);

Line * retAssembly ();