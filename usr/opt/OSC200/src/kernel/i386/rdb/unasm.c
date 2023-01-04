/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: unasm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:24 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
#ifdef notdef
#ifndef SCCSID
#include <sccs.h>
#endif /* ! SCCSID */

#endif

/*
 * Instruction decoding and machine dependent data access.
 */

#include "i386/rdb/debug_mach.h"

#define public /* global */

#define Address unsigned int
#define Byte unsigned char
#define Word unsigned int
#define boolean char
#define true 1
#define false 0
#define private static
#define fflush(stdout) /* dummy function */
#define warning(msg) { printf(msg); printf("\n"); }
#define error(msg,value) printf(msg,value), ++err_flag

int traceasm = 0;
int objsize = 0xffffffff;
extern int err_flag;

#include "i386/rdb/ops.h"

#include "i386/rdb/ops.c"

public Address CODESTART = 0xd4;                                   /*CH0025a*/
public Address CODEBASE  = 0;                                   /*CH0032*/

/*Changed ESP to USP for ptrace interface reg.h with E30A kernel*/
extern int rloc[];

#define SIB_BYTE_PRESENT 4
#define EXCEPTION_16 6
#define EXCEPTION_32 5

typedef enum {
    general_8,       /* 8 bit registers e.g. AL */
    general_16,      /* 16 bit registers e.g. AX */
    general_32,      /* 32 bit registers e.g. EAX */
    segment, control, debug, test, i387_stack
} register_type;

typedef enum {
    no_displacement,
    displacement_8, displacement_ASD, register_effective_address
} mod_field;

typedef struct {
    Regname base;
    Regname index;
    int scale;
} Sib;

#define mask(n) ((1 << (n)) - 1)
#define tst(x, n) (((x) & mask(n)) != 0)

/*
 * Extract bits n through m (unsigned or signed) from a 32-bit word x.
 * N.B.:  These two macros depend on the machine bit ordering.
 */

#define bits(x, n, m) (((x) >> (n)) & mask((m)-(n)+1))
#define signed(x, n, m) (((x) << (31-(m))) >> (31-(m)+(n)))

#define byte(x, n) bits(x, n << 3, n << 3 + 7)

#define get_R_M_value() bits(getRM(), 0, 2)
#define get_reg_value() bits(getRM(), 3, 5)
#define get_base_register(b) regtable[general_32][bits(b, 0, 2)]
#define get_R_M_register(t) regtable[t][get_R_M_value()]
#define get_reg_field_operand(t) regtable[t][get_reg_value()]
#define mod_is_register() (get_mod_value() == register_effective_address)

static Regname regtable[][8] = {
    {R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH},
    {R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI},
    {R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI},
    {R_ES, R_CS, R_SS, R_DS, R_FS, R_GS, NO_REG, NO_REG},
    {R_CR0, NO_REG, R_CR2, R_CR3, NO_REG, NO_REG, NO_REG, NO_REG},
    {R_DR0, R_DR1, R_DR2, R_DR3, NO_REG, NO_REG, R_DR6, R_DR7},
    {NO_REG, NO_REG, NO_REG, NO_REG, NO_REG, NO_REG, R_TR6, R_TR7},
    {R_ST0, R_ST1, R_ST2, R_ST3, R_ST4, R_ST5, R_ST6, R_ST7}
};

#define NO_REG_NAME "???"                                         /* CH 0004 */
static char *regname[] = {
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
    "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
    "es", "cs", "ss", "ds", "fs", "gs", "cr0", "cr2",
    "cr3", "dr0", "dr1", "dr2", "dr3", "dr6", "dr7", "tr6",       /* CH 0004 */
    "tr7", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)",/*CH0034*/
    "st(7)", NO_REG_NAME                                          /* CH 0004 */
};

static Address curaddr;	/* address of current instruction */
static Byte curprefix;	/* current instruction prefix */
static int cursize;	/* current operand size: 16 or 32 */
static int addrsize;	/* current address size: 16 or 32 */
static Regname curseg;	/* current segment override, or NO_REG */
static Byte curRMbyte;	/* current instruction's R/M byte */
static boolean RMread;	/* curRMbyte is up-to-date */

static badop (s)
char *s;
{
    warning(s);
}

static Byte getbyte ()
{
    Byte b;

    b = bfetch(curaddr);
    yprintf("%02x ",b);
    curaddr += sizeof(b);
    return b;
}

static short getshort ()
{
    unsigned short s;

    s = hfetch(curaddr);
    yprintf("%04x ",s);
    curaddr += sizeof(s);
    return s;
}

static long getlong ()
{
    long d;

    d = wfetch(curaddr);
    yprintf("%08x ",d);
    curaddr += sizeof(d);
    return d;
}

static Byte getRM ()
{
    if (!RMread) {
	curRMbyte = getbyte();
	RMread = true;
    }
    return curRMbyte;
}

static Byte readinst ()
{
    Byte op;
    extern int db_16bit;

    cursize = db_16bit ? 16 : 32;
    addrsize = db_16bit ? 16 : 32;
    curseg = NO_REG;
    RMread = false;
    if (traceasm) {
	printf("-- starting readinst at 0x%x\n", curaddr);
        fflush (stdout);
    }
    while (curaddr < CODEBASE + objsize) {             /* CH 0032 */
	op = getbyte();
	if (traceasm) {
	    printf("-- read opcode byte 0x%x\n", op);
	}
	switch (op) {
	    /* instruction prefixes */
	    case 0xf3: /* REPE */
	    case 0xf2: /* REPNE */
	    case 0xf0: /* LOCK */
		curprefix = op;
		break;

	    /* operand prefix */
	    case 0x66: cursize = db_16bit ? 32 : 16; break;

	    /* address prefix */
	    case 0x67: addrsize = db_16bit ? 32 : 16; break;

	    /* segment override prefixes */
	    case 0x2e: curseg = R_CS; break;
	    case 0x36: curseg = R_SS; break;
	    case 0x3e: curseg = R_DS; break;
	    case 0x26: curseg = R_ES; break;
	    case 0x64: curseg = R_FS; break;
	    case 0x65: curseg = R_GS; break;

	    default:
		return op;
	}
    }
    error("couldn't find instruction byte at 0x%x", curaddr);
    return op;
}

private boolean getinstruction (addr, i)
Address addr;
Instruction *i;
{
    byte_1_index_values t;                                          /*CH0034*/
    Byte b;

    if (traceasm)
    {
	printf("-- starting getinstruction at 0x%x\n",addr);
	fflush(stdout);
    }
    curaddr = addr;
    b = readinst();
    t = i386_byte_1_indexes[b];
    switch (t) {
	case extended1:
	    if (traceasm) {
		printf(" (extended 1 byte)\n");
		fflush(stdout);
	    }
	    if (!DoExtended1(b, i))
		return(false);
	    break;
	case two_byte:
	    if (traceasm) {
		printf(" (two byte)\n");
		fflush(stdout);
	    }
	    if (!DoTwoByte(getbyte(), i))
		return(false);
	    break;
	case i387_11b:
	    if (traceasm) {
		printf(" (387 op)\n");
		fflush(stdout);
	    }
	    Do387(b, i);
	    break;
	case ILL_8_BIT:
	    if (traceasm) {
		putchar('\n');
		fflush(stdout);
	    }
	    return false;
	default:
	    if (traceasm) {
		putchar('\n');
		fflush(stdout);
	    }
	    Decode(&i386_1_byte_instructions[t], i);
	    break;
    }
    return true;
}

static char *unasm_ptr;
static char *unasm_ptr2;
#define MAXLEN	33

public Address printop (addr, buff)
Address addr;
char *buff;
{
    Instruction inst;
    extern int debug_option;

    unasm_ptr2 = buff;
    unasm_ptr = buff + MAXLEN;
    traceasm = debug_option & 0x100;
    if (traceasm)
    {
	printf("-- starting printop at 0x%x\n",addr);
	fflush(stdout);
    }
    yprintf("%08x ", addr);
    if (getinstruction(addr, &inst)) {
	PrintInst(addr, &inst);
    } else {
	zprintf("badop");
    }
    while (unasm_ptr2 < buff+MAXLEN)
	*unasm_ptr2++ = ' ';
    *unasm_ptr++ = 0;
    return curaddr;
}

static boolean PrintOperand (prev, o)
boolean prev;
Operand *o;
{
    if (prev && o->format != no_operand) {
	xprintf(",");                                               /*CH0034*/
    }
    switch (o->format) {
	case no_operand:
	    return false;
	case immed_data:
	    xprintf("$%d", o->value.data);
	    break;
	case memory_address:
	    if (curseg != NO_REG) {
		xprintf("%%%s:", regname[curseg]);
	    }
	    print_R_M_address(&o->value.components);
	    break;
	case gregister:
	    xprintf("%%%s", regname[o->value.name]);
	    break;
	case memory_offs:
	    if (curseg != NO_REG) {
		xprintf("%%%s:", regname[curseg]);
	    }
	    xprintf("0x%x", o->value.offset);
	    break;
	case relative_addr:
	    if (curseg != NO_REG) {
		xprintf("%%%s:", regname[curseg]);
	    }
	    print_absolute(curaddr + o->value.offset);
	    break;
	case constant:
	    xprintf("%d", o->value.constant_val);
	    break;
	case string_operand:
	    if (curseg != NO_REG && (
		    o->value.str_cmpts.index == R_SI ||
		    o->value.str_cmpts.index == R_ESI
		)
	    ) {
		xprintf("%%%s:", regname[curseg]);
	    }
	    print_R_M_address(&o->value.str_cmpts);
	    break;
	default:
	    xprintf("???");
	    break;
    }
    return true;
}

static PrintInst (addr, i)
Address addr;
register Instruction *i;
{
    boolean b;

    xprintf("%-10s", i386_mnemonics[(int)(i->mnemonic)]);
    zprintf("  ");
    b = PrintOperand(false, &i->source_immediate);
    if (PrintOperand(b, &i->source_register_memory)) {
	PrintOperand(true, &i->destination);
    } else {
	PrintOperand(b, &i->destination);
    }
}

static mod_field get_mod_value ()
{
    switch (bits(getRM(), 6, 7)) {
	case 0: return no_displacement;
	case 1: return displacement_8;
	case 2: return displacement_ASD;
	case 3: return register_effective_address;
    }
    /* NOTREACHED */
}

static boolean HasDummy (op)
Opcode op;
{
    return op == AAD || op == AAM;
}

static DoExtended1 (op, i)
Byte op;
Inst386 *i;
{
    int n, ext;
    group_names g;

    ext = get_reg_value();
    if (traceasm)
	printf("DoExtended1: op=%x ext=%x\n", op, ext);
    switch (op) {
	case group_1a:	g = extended_1a; break;
	case group_1b:	g = extended_1b; break;
	case group_1c:	g = extended_1c; break;
	case group_2a:	g = extended_2a; break;
	case group_2b:	g = extended_2b; break;
	case group_2c:	g = extended_2c; break;
	case group_2d:	g = extended_2d; break;
	case group_2e:	g = extended_2e; break;
	case group_2f:	g = extended_2f; break;
	case group_3a:	g = extended_3a; break;
	case group_3b:	g = extended_3b; break;
	case group_4:	g = extended_4; break;
	case group_5:	g = extended_5; break;
	default:
	    badop("illegal 1st byte for 11 bit opcode");
	    return;
    }
    n = i386_11_bit_indexes[g][ext];
    if (n == ILL_11_BIT) {
	badop("illegal 11 bit opcode");
	return(false);
    } else {
	Decode(&i386_11_bit_instructions[n], i);
	return(true);
    }
}

static DoTwoByte (op, i)
Byte op;
Inst386 *i;
{
    int n, ext;
    group_names2 g;                                                 /*CH0034*/

    n = i386_byte_2_indexes[op];
    switch (n) {
	case extended2:
	    ext = get_reg_value();                                  /*CH0034*/
	    switch (op) {
		case group_6: g = extended_6; break;
		case group_7: g = extended_7; break;
		case group_8: g = extended_8; break;
		default:
		    badop("illegal 2nd byte for 19 bit opcode");
		    return(false);
	    }
	    n = i386_19_bit_indexes[g][ext];
	    if (n == ILL_19_BIT) {
		badop("illegal 19 bit opcode");
		return(false);
	    } else {
		Decode(&i386_19_bit_instructions[n], i);
	    }
	    break;
	case two_b_80: case two_b_81: case two_b_82: case two_b_83:
	case two_b_84: case two_b_85: case two_b_86: case two_b_87:
	case two_b_88: case two_b_89: case two_b_8a: case two_b_8b:
	case two_b_8c: case two_b_8d: case two_b_8e: case two_b_8f:
	case two_b_06: case two_b_a0: case two_b_a1: case two_b_a8:
	case two_b_a9:
	    Decode(&i386_2_byte_instructions[n], i);
	    break;
	case ILL_16_BIT:
	    badop("illegal 2-byte opcode");
	    return(false);
	    break;
	default:
	    ext = get_reg_value();                                  /*CH0034*/
	    Decode(&i386_2_byte_instructions[n], i);
	    break;
    }
    return(true);
}

static Do387 (op, i)
Byte op;
Instruction *i;
{
    register i387_indexes t;

    t = i387_reg_indexes[op-0xd8][get_reg_value()];
    if (t >= d8_0a && t <= df_7) {
	Decode387(&i387_instructions[t], i, t);                     /*CH0034*/
    } else if (t >= d8_0 && t <= df_4) {
	t = i387_mod_indexes[(int)t-(int)d8_0][mod_is_register()];
	if (t >= d8_0a && t <= df_7) {
	    Decode387(&i387_instructions[t], i, t);                 /*CH0034*/
	} else if (t >= d9_4b && t <= db_4b) {
	    t = i387_R_M_indexes[(int)t-(int)d9_4b][get_R_M_value()];
	    if (t >= d8_0a && t <= df_7) {
		Decode387(&i387_instructions[t], i, t);             /*CH0034*/
	    } else {
		badop("invalid index in i387_R_M_indexes");
	    }
	} else {
	    badop("invalid index in i387_mod_indexes");
	}
    } else if (t == ILL_387_INST) {
	badop("illegal 387 opcode");
    } else {
	badop("invalid index in i387_reg_indexes");
    }
}

static Regname get_index_register (b)
int b;
{
    int r;

    r = bits(b, 3, 5);
    if (r == 4) {
	return NO_REG;
    } else {
	return regtable[general_32][r];
    }
}

static get_SIB (b, s)
Byte b;
register Sib* s;
{
    s->scale = bits(b, 6, 7);
    s->index = get_index_register(b);
    s->base = get_base_register(b);
}

static Decode (format, i)
Inst386 *format;
Instruction *i;
{
    i->mnemonic = format->mnemonic[cursize == 16 ? 0 : 1];
    if (traceasm) {
	printf("-- decoding op[%d] at 0x%x\n", i->mnemonic, curaddr);
	fflush(stdout);
    }
    if (i->mnemonic == AAD || i->mnemonic == AAM) {
	curaddr += 1;
    }
    get_i386_operand(&i->destination, &format->destination);
    get_i386_operand(&i->source_register_memory, &format->source_reg_mem);
    get_i386_operand(&i->source_immediate, &format->source_immediate);
}

static Decode387 (format, i, t)                                     /*CH0034*/
Inst387 *format;
Instruction *i;
{
    if (traceasm) {
	printf("-- decoding 387 op %s at 0x%x\n",
	    i386_mnemonics[(int)(format->opcode)], curaddr
	);
	fflush(stdout);
    }
    i->mnemonic = format->opcode;
    switch (t) {                                                    /*CH0034*/
    case dc_0b: case de_0b: /* FADD,  FADDP  */                     /*CH0034*/
    case dc_1b: case de_1b: /* FMUL,  FMULP  */                     /*CH0034*/
    case dc_4b: case de_4b: /* FSUB,  FSUBP  */                     /*CH0034*/
    case dc_5b: case de_5b: /* FSUBR, FSUBRP */                     /*CH0034*/
    case dc_6b: case de_6b: /* FDIV,  FDIVP  */                     /*CH0034*/
    case dc_7b: case de_7b: /* FDIVR, FDIVRP */                     /*CH0034*/
        get_i387_operand(&i->source_register_memory,                /*CH0034*/
                         format->destination_type);                 /*CH0034*/
        get_i387_operand(&i->destination, format->source_type);     /*CH0034*/
        break;                                                      /*CH0034*/
    default:                                                        /*CH0034*/
        get_i387_operand(&i->destination, format->destination_type);/*CH0034*/
        get_i387_operand(&i->source_register_memory,                /*CH0034*/
                         format->source_type);                      /*CH0034*/
    }                                                               /*CH0034*/
    i->source_immediate.format = no_operand;
}

static get_R_M_address (o)
register R_M_address *o;
{
    Sib s;
    int m, v;

    m = get_mod_value();
    v = get_R_M_value();
    if (addrsize == 16) {
	o->scale = 0;
	if (m == no_displacement && v == EXCEPTION_16) {
	    o->base = NO_REG;
	    o->index = NO_REG;
	    o->displacement = getshort();
	} else {
	    switch (m) {
		case no_displacement:
		    o->displacement = 0;
		    break;
		case displacement_8:
		    o->displacement = getbyte();
		    break;
		case displacement_ASD:
		    o->displacement = getshort();
		    break;
		case register_effective_address:
		    warning("getting memory ref when mod is register");
		    return(false);
	    }
	    switch (v) {
		case 0: o->base = R_BX; o->index = R_SI; break;
                case 1: o->base = R_BX; o->index = R_DI; break;
		case 2: o->base = R_BP; o->index = R_SI; break;
		case 3: o->base = R_BP; o->index = R_DI; break;
                case 4: o->base = R_SI; o->index = NO_REG; break;
		case 5: o->base = R_DI; o->index = NO_REG; break;
		case 6: o->base = R_BP; o->index = NO_REG; break;
		case 7: o->base = R_BX; o->index = NO_REG; break;
	    }
	}
    } else {
	if (v == SIB_BYTE_PRESENT) {
            get_SIB(getbyte(), &s);
	    if (m == no_displacement) {
		if (s.base == R_EBP) {
		    o->base = NO_REG;
		    o->displacement = getlong();
		} else {
		    o->base = s.base;
		    o->displacement = 0;
		}
		o->index = s.index;
		o->scale = s.scale;
	    } else {
		o->base = s.base;
		o->index = s.index;
		o->scale = s.scale;
		if (m == displacement_8) {
		    o->displacement = getbyte();
		} else {
		    o->displacement = getlong();
		}
	    }
	} else {
	    o->index = NO_REG;
	    o->scale = 0;
	    if (m == no_displacement && v == EXCEPTION_32) {
		o->base = NO_REG;
		o->displacement = getlong();
	    } else {
		o->base = regtable[general_32][v];
		switch (m) {
		    case no_displacement:
			o->displacement = 0;
			break;
		    case displacement_8:
			o->displacement = getbyte();
			break;
		    case displacement_ASD:
			o->displacement = getlong();
			break;
		    case register_effective_address:
			warning("getting memory ref when mod is register");
			break;
		}
	    }
	}
    }
}

static print_absolute (addr)
Address addr;
{
#ifdef DBX
    Symbol f;

    f = whatblock(addr);
    if (f != nil && codeloc(f) == addr) {
	printname(stdout, f);
    } else {
	printf("<0x%x>", addr);
    }
#else
    if (xprsym(addr, 0x10000, (char *) 0) == 0)
	    xprintf("<0x%x>", addr);
#endif /* DBX */
}

static print_R_M_address (a)
register R_M_address *a;
{
    if (a->base == NO_REG && a->index == NO_REG) {
	print_absolute(a->displacement);
    } else {
	if (a->displacement != 0) {
	    xprintf("%d", a->displacement);
	}
	zprintf("(");
	if (a->base != NO_REG) {
	    xprintf("%%%s", regname[a->base]);
	}
	if (a->index != NO_REG) {
	    if (a->base != NO_REG) {
		zprintf(",");
	    }
	    xprintf("%%%s", regname[a->index]);
	    if (a->scale != 0) {
		xprintf(",%d", (1 << a->scale));
	    }
	}
	zprintf(")");
    }
}

static long getoffset (s, islong)
format_size s;
boolean islong;
{
    long o;

    switch (s) {
	case BYTE: o = ((long) getbyte() << 24) >> 24; break;
	case WORD: o = getshort(); break;
	case LONG: o = getlong(); break;
	case ANY:  o = islong ? getshort() : getlong(); break;
	default:
	    warning("bad size in getoffset");
	    o = 0;
	    break;
    }
    return o;
}

static get_i386_operand (o, f)
register Operand *o;
Opnd386 *f;
{
    int r;

    if (traceasm) {
	printf("-- getting operand type %d at 0x%x\n",
	    f->format_type, curaddr
	);
	fflush(stdout);
    }
    switch (f->format_type) {
	case NULL_OP:
	    o->format = no_operand;
	    break;
	case IM:
	    o->format = immed_data;
	    o->value.data = getoffset(f->size, cursize == 16);
	    break;
	case RX:
	    if (mod_is_register()) {
		o->format = gregister;
		switch (f->size) {
		    case BYTE:
			o->value.name = get_R_M_register(general_8);
			break;
		    case WORD:
			o->value.name = get_R_M_register(general_16);
			break;
		    case LONG:
			o->value.name = get_R_M_register(general_32);
			break;
		    case ANY:
			o->value.name = get_R_M_register(
			    cursize == 16 ? general_16 : general_32
			);
			break;
		}
	    } else {
		o->format = memory_address;
		get_R_M_address(&o->value.components);
	    }
	    break;
	case RR:
	    o->format = gregister;
	    switch (f->size) {
		case BYTE:
		    o->value.name = get_reg_field_operand(general_8);
		    break;
		case WORD:
		    o->value.name = get_reg_field_operand(general_16);
		    break;
		case LONG:
		    o->value.name = get_reg_field_operand(general_32);
		    break;
		case ANY:
		    o->value.name = get_reg_field_operand(
			cursize == 16 ? general_16 : general_32
		    );
		    break;
	    }
	    break;
	case SR:
	    o->format = gregister;
	    o->value.name = get_reg_field_operand(segment);
	    break;
	case CR:
	    o->format = gregister;
	    o->value.name = get_reg_field_operand(control);
	    break;
	case DR:
	    o->format = gregister;
	    o->value.name = get_reg_field_operand(debug);
	    break;
	case TR:
	    o->format = gregister;
	    o->value.name = get_reg_field_operand(test);
	    break;
	case IR1:
	case IR2:
	case IR3:
	case IR4:
	case IR5:
	case IR6:
	case IR7:
	case IR8:
	    o->format = gregister;
	    r = (int)(f->format_type) - (int)IR1;
	    switch (f->size) {
		case BYTE:
		    o->value.name = regtable[general_8][r];
		    break;
		case WORD:
		    o->value.name = regtable[general_16][r];
		    break;
		case ANY:
		    o->value.name =
			regtable[cursize==16 ? general_16:general_32][r];
		    break;
	    }
	    break;
	case IES:
	case ICS:
	case ISS:
	case IDS:
	case IFS:
	case IGS:
	    o->format = gregister;
	    r = (int)(f->format_type) - (int)IES;
	    o->value.name = regtable[segment][r];
	    break;
	case ISI:
	    o->format = string_operand;
	    o->value.str_cmpts.base = NO_REG;
	    o->value.str_cmpts.scale = 0;
	    o->value.str_cmpts.displacement = 0;
	    o->value.str_cmpts.index = addrsize == 16 ? R_SI : R_ESI;
	    break;
	case IDI:
	    o->format = string_operand;
	    o->value.str_cmpts.base = NO_REG;
	    o->value.str_cmpts.scale = 0;
	    o->value.str_cmpts.displacement = 0;
	    o->value.str_cmpts.index = addrsize == 16 ? R_DI : R_EDI;
	    break;
	case OFF:
	    o->format = memory_offs;
	    o->value.offset = getoffset(f->size, addrsize == 16);
	    break;
	case REL:
	    o->format = relative_addr;
	    o->value.offset = getoffset(f->size, addrsize == 16);
	    break;
	case I1:
	    o->format = constant;
	    o->value.constant_val = 1;
	    break;
	case I3:
	    o->format = constant;
	    o->value.constant_val = 3;
	    break;
    }
}

static get_i387_operand (o, f)
register Operand *o;
i387_format_type f;
{
    if (traceasm) {
	printf("-- getting operand type %d at 0x%x\n", f, curaddr);
	fflush(stdout);
    }
    switch (f) {
	case i387_NULL_OP:
	    o->format = no_operand;
	    break;
	case i387_imp_ST:
	    o->format = gregister;
	    o->value.name = R_ST0;
	    break;
	case i387_imp_reg_1:
	    o->format = gregister;
	    o->value.name = R_AX;
	    break;
	case i387_rm:
	    if (mod_is_register()) {
		o->format = gregister;
		o->value.name = get_R_M_register(i387_stack);
	    } else {
		o->format = memory_address;
		get_R_M_address(&o->value.components);
	    }
	    break;
    }
}

#ifdef DBX
/*
 * Check to see if execution stopped unusually or is at a breakpoint.
 * Update pc as a side-effect.
 */

private void chkbp ()
{
    pc = reg(PROGCTR);
    if (!isbperr()) {
	printstatus();
    }
    bpact();
}

/*
 * Compute the next address that will be executed from the given one.
 * If "isnext" is true then consider a procedure call as straight line code.
 *
 * We must unfortunately do much of the same work that is necessary
 * to print instructions.  In addition we have to deal with branches.
 * We continue execution to the current location and then single step
 * the machine across the branch.
 */

private Address dojump (startaddr)
Address startaddr;
{
    stepto(startaddr);
    pstep(process, DEFSIG);
    chkbp();
    return pc;
}

public Address nextaddr (startaddr, isnext)
Address startaddr;
Boolean isnext;
{
    Address addr;
    Instruction i;
    Opcode op;

    if (traceasm)
    {
	printf("-- starting nextaddr at 0x%x\n", startaddr);
        fflush (stdout);
    }
    addr = usignal(process);
    if (traceasm)
    {
	printf("-- address from usignal(process): 0x%x\n", addr);
        fflush (stdout);
    }
    if (addr == 0 || addr == 1) {
	if (getinstruction(startaddr, &i)) {
	    op = i.mnemonic;
	    switch (op) {
		/*
		 * Skip over a call if isnext is true OR it is a call
		 * to a procedure without source information and we are
		 * not tracing instructions.  A hazardous side-effect
		 * of this is that if the procedure then calls a procedure
		 * that does have source information, we skip over those
		 * source lines.  This is preferable, however, to waiting
		 * for instruction-level reading over library routines
		 * such as printf.
		 */
		case CALL:
		    if (isnext) {
			addr = curaddr;
		    } else {
			addr = dojump(startaddr);
			if (!inst_tracing) {
			    Symbol f = whatblock(addr);
			    if (f != nil && nosource(f)) {
				/*
				 * Execute through the "assembler" procedure
				 * with breakpoints on.  If we don't do it
				 * here, the main control will may run
				 * with breakpoints off.  That is usually
				 * the right thing to do, but we would lose
				 * a breakpoint (like _exit) if we don't
				 * catch them in this case.
				 */
				addr = curaddr;
				setallbps();
				setbp(addr);
				resume(DEFSIG);
				unsetbp(addr);
				unsetallbps();
				chkbp();
			    }
			}
		    }
		    break;
		/*
		 * The ptrace step command doesn't seem to work across
		 * system calls (is this a bug or feature?).  We can't
		 * let these calls be handled by the default case below
		 * because the ops are within the jump range.
		 */
		case LCALL:                                         /*CH0034*/
		    addr = curaddr;
		    break;
		case LEAVE: case RET:
		case HLT: case INT: case INTO: case IRET:           /*CH0034*/
		    addr = dojump(startaddr);
		    break;
		default:
		    if ((op >= JB && op <= JZ) || op == LJMP) {     /*CH0034*/
			addr = dojump(startaddr);
		    } else {
			addr = curaddr;
		    }
		    break;
	    }
	} else {
	    addr = startaddr;
	}
    }
    return addr;
}
#endif /* DBX */

/*
 * Compute how many arguments were actually passed by looking
 * at the addl instruction at the return address.
 */

public int getnumargs (addr)
Address addr;
{
    int n;
    Instruction i;

    if (traceasm)
    {
	printf("-- starting getnumargs at 0x%x\n", addr);
        fflush (stdout);
    }
    n = 0;
    if (getinstruction(addr, &i)) {
	Operand* o = &i.source_immediate;
	if (i.mnemonic == POPL) {
	    n = 1;
	} else if (i.mnemonic == ADDL && o->format == immed_data) {
	    n = o->value.data / sizeof(Word);
	}
    }
    return n;
}

#ifdef DBX
/*
 * Determine whether the stack pointer or base pointer should be
 * used to access an activation record in a procedure, given
 * the pc within the procedure.  This routine relies directly
 * on the specific instructions that the compiler generates in a prolog.
 *
 * Return true the stack pointer should be used and set stkoff
 * to the offset from the stack pointer where the activation record begins.
 */

public boolean entrycode (start, addr, stkoff)
Address start, addr;
int *stkoff;
{
    Address a;
    Instruction i;
    Operand *o;

    if (traceasm)
    {
	printf("-- starting entrycode at 0x%x to 0x%x\n", start, addr);
        fflush (stdout);
    }
    *stkoff = -1*sizeof(Word);
    for (a = start; a < addr; a = curaddr) {
	if (!getinstruction(a, &i)) {
	    warning("can't disassemble at 0x%x", a);
	    return true;
	}
	switch (i.mnemonic) {
	    case CALL:
		break;
	    case FLDCW:
		break;
	    case PUSHL:
		o = &i.destination;
		if (o->format == gregister) {
		    *stkoff += sizeof(Word);
		}
		break;
	    case MOVL:
		o = &i.destination;
		if (o->format == gregister && o->value.name == R_EBP) {
		    return false;
		}
		break;
	    case LEAL:
		o = &i.destination;
		if (o->format == gregister && o->value.name == R_EBP) {
		    return false;
		}
		break;
	    case SUBL:
		o = &i.destination;
		if (o->format == gregister && o->value.name == R_ESP) {
		    o = &i.source_immediate;
		    if (o->format == immed_data) {
			*stkoff += o->value.data;
		    }
		}
		break;
	    default:
		return true;
	}
    }
    return true;
}

/*
 * Enter a procedure by creating and executing a call instruction.
 */

#define CALLSIZE 5

public beginproc (p, argc)
Symbol p;
int argc;
{
    Address addr;
    Byte save[CALLSIZE];
    static Byte opcode = 0xe8;
    Word dst;

    addr = CODESTART;
    iread(save, addr, sizeof(save));
    iwrite(&opcode, addr, sizeof(opcode));
    dst = codeloc(p) - addr - CALLSIZE;
    iwrite(&dst, addr + sizeof(opcode), sizeof(dst));
    if (reg(FRP) == 0) {
	setreg(FRP, reg(STKP));
    }
    setreg(PROGCTR, addr);
    pstep(process, DEFSIG);
    iwrite(save, addr, sizeof(save));
    pc = reg(PROGCTR);
    if (!isbperr()) {
	printstatus();
    }
}

/*
 * Extract a bit field from an integer.
 */

public int extractField (s)
Symbol s;
{
    int nbytes, n, r, off, len;

    off = s->symvalue.field.offset;
    len = s->symvalue.field.length;
    nbytes = size(s);
    n = 0;
    if (nbytes > sizeof(n)) {
	zprintf("[bad size in extractField -- word assumed]\n");
	nbytes = sizeof(n);
    }
    popn(nbytes, (char *)&n);
    r = n >> (off % BITSPERBYTE);
    r &= ((1 << len) - 1);
    return r;
}


/*
 * Change the length of a value in memory according to a given difference
 * in the lengths of its new and old types.
 */

public loophole (oldlen, newlen)
int oldlen, newlen;
{
    int i, n;
    Stack *oldsp;

    n = newlen - oldlen;
    oldsp = sp - oldlen;
    if (n > 0) {
	for (i = oldlen; i < newlen; i++) {
	    oldsp[i] = '\0';
	}
    }
/* For some reason, the following code caused loophole to fail. -- mjs 
    } else {
	for (i = 0; i < newlen; i++) {
	    oldsp[i] = oldsp[i - n];
	}
    }
*/
    sp += n;
}
#endif /* DBX */


xprintf(fmt,value)
char *fmt;
{
	if (traceasm)
	    printf(fmt, value);
	sprintf(unasm_ptr, fmt, value);
	unasm_ptr += strlen(unasm_ptr);
}

static zprintf(fmt)
char *fmt;
{
	if (traceasm)
	    printf(fmt);
	sprintf(unasm_ptr, fmt);
	unasm_ptr += strlen(unasm_ptr);
}

static yprintf(fmt,value)
char *fmt;
{
	if (traceasm)
	    printf(fmt,value);
	sprintf(unasm_ptr2, fmt, value);
	unasm_ptr2 += strlen(unasm_ptr2);
}

