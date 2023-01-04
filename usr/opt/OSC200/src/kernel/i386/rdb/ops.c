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
static char	*sccsid = "@(#)$RCSfile: ops.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:39 $";
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
 * 386 opcode information.
 */

#define public /* global */

#include "i386/rdb/ops.h"


public char *i386_mnemonics[] = {
    "aaa",     "aad",     "aam",     "aas",     "adcb",    "adcl",  /*CH0034*/
    "adcw",    "addb",    "addl",    "addw",    "andb",    "andl",  /*CH0034*/
    "andw",    "arpl",    "boundl",  "boundw",  "bsfl",    "bsfw",  /*CH0034*/
    "bsrl",    "bsrw",    "btcl",    "btcw",    "btl",     "btrl",  /*CH0034*/
    "btrw",    "btsl",    "btsw",    "btw",     "call",    "cbtw",  /*CH0034*/
    "clc",     "cld",     "cli",     "cltd",    "clts",    "cmc",   /*CH0034*/
    "cmpb",    "cmpl",    "cmpw",                                   /*CH0034*/
    "cwtd",    "cwtl",    "daa",     "das",     "decb",    "decl",  /*CH0034*/
    "decw",    "divb",    "divl",    "divw",    "enter",   "f2xm1", /*CH0034*/
    "fabs",    "fadd",    "faddl",   "faddp",   "fadds",   "fbld",  /*CH0034*/
    "fbstp",   "fchs",    "fcom",    "fcoml",   "fcomp",   "fcompl",/*CH0034*/
    "fcompp",  "fcomps",  "fcoms",   "fcos",    "fdecstp", "fdisi", /*CH0034*/
    "fdiv",    "fdivl",   "fdivp",   "fdivr",   "fdivrl",  "fdivrp",/*CH0034*/
    "fdivrs",  "fdivs",   "feni",    "ffree",   "fiadd",   "fiaddl",/*CH0034*/
    "ficom",   "ficoml",  "ficomp",  "ficompl", "fidiv",            /*CH0034*/
    "fidivl",  "fidivr",  "fidivrl", "fild",    "fildl",   "fildll",/*CH0034*/
    "fimul",   "fimull",  "fincstp", "fist",    "fistl",            /*CH0034*/
    "fistp",   "fistpl",  "fistpll", "fisub",   "fisubl",  "fisubr",/*CH0034*/
    "fisubrl", "fld",     "fld1",    "fldcw",   "fldenv",  "fldl",  /*CH0034*/
    "fldl2e",  "fldl2t",  "fldlg2",  "fldln2",  "fldpi",   "flds",  /*CH0034*/
    "fldt",    "fldz",    "fmul",    "fmull",   "fmulp",   "fmuls", /*CH0034*/
    "fnclex",  "fninit",  "fnop",    "fnsave",  "fnstcw",  "fnstenv",/*CH0034*/
    "fnstsw",  "fpatan",  "fprem",   "fprem1",  "fptan",   "frndint",/*CH0034*/
    "frstor",  "fscale",  "fsetpm",  "fsin",    "fsincos", "fsqrt", /*CH0034*/
    "fst",     "fstl",    "fstp",    "fstpl",   "fstps",   "fstpt", /*CH0034*/
    "fsts",    "fsub",    "fsubl",   "fsubp",   "fsubr",   "fsubrl",/*CH0034*/
    "fsubrp",  "fsubrs",  "fsubs",   "ftst",    "fucom",   "fucomp",/*CH0034*/
    "fucompp", "fwait",   "fxam",    "fxch",    "fxtract", "fyl2x", /*CH0034*/
    "fyl2xp1", "hlt",     "idivb",   "idivl",   "idivw",   "imulb", /*CH0034*/
    "imull",   "imulw",   "inb",     "incb",    "incl",    "incw",  /*CH0034*/
    "inl",     "insb",    "insl",    "insw",    "int",     "into",  /*CH0034*/
    "inw",     "iret",    "jb",      "jbe",     "jcxz",             /*CH0034*/
    "jecxz",   "jl",      "jle",     "jmp",     "jnb",     "jnbe",  /*CH0034*/
    "jnl",     "jnle",    "jno",     "jns",     "jnz",     "jo",    /*CH0034*/
    "jpe",     "jpo",     "js",      "jz",      "lahf",    "larl",  /*CH0034*/
    "larw",    "lcall",   "ldsl",    "ldsw",    "leal",    "leave", /*CH0034*/
    "leaw",    "lesl",    "lesw",    "lfsl",    "lfsw",    "lgdt",  /*CH0034*/
    "lgsl",    "lgsw",    "lidt",    "ljmp",    "lldt",    "lmsw",  /*CH0034*/
    "lock",    "loop",    "loope",                                  /*CH0034*/
    "loopne",  "lsll",    "lslw",    "lssl",    "lssw",    "ltr",   /*CH0034*/
    "movb",    "movl",    "movw",    "movswl",  "movsbw",  "movsbl",/*CH0034*/
    "movzwl",  "movzbw",  "movzbl",  "mulb",    "mull",             /*CH0034*/
    "mulw",    "negb",    "negl",    "negw",    "nop",     "notb",  /*CH0034*/
    "notl",    "notw",    "orb",     "orl",     "orw",     "outb",  /*CH0034*/
    "outl",    "outw",    "outsb",   "outsl",   "outsw",   "seg_cs",/*CH0034*/
    "seg_ds",  "seg_es",  "seg_fs",  "seg_gs",  "seg_ss",  "popal", /*CH0034*/
    "popaw",   "popfl",   "popfw",   "popl",    "popw",    "pushal",/*CH0034*/
    "pushaw",  "pushfl",  "pushfw",  "pushl",   "pushw",   "rclb",  /*CH0034*/
    "rcll",    "rclw",    "rcrb",    "rcrl",    "rcrw",    "repe",  /*CH0034*/
    "repne",   "ret",     "rolb",    "roll",    "rolw",    "rorb",  /*CH0034*/
    "rorl",    "rorw",    "sahf",    "salb",    "sall",    "salw",  /*CH0034*/
    "sarb",    "sarl",    "sarw",    "sbbb",    "sbbl",    "sbbw",  /*CH0034*/
    "setb",    "setbe",   "setl",                                   /*CH0034*/
    "setle",   "setnb",   "setnbe",  "setnl",   "setnle",  "setno", /*CH0034*/
    "setnp",   "setns",   "setnz",   "seto",    "setp",    "sets",  /*CH0034*/
    "setz",    "sgdt",    "shldl",   "shldw",   "shrb",    "shrdl", /*CH0034*/
    "shrdw",   "shrl",    "shrw",    "sidt",    "sldt",    "smsw",  /*CH0034*/
    "scmpb",   "scmpl",   "scmpw",   "slodb",   "slodl",   "slodw", /*CH0034*/
    "smovb",   "smovl",   "smovw",   "sscab",   "sscal",   "sscaw", /*CH0034*/
    "sstob",   "sstol",   "sstow",   "stc",     "std",     "sti",   /*CH0034*/
    "str",     "subb",    "subl",    "subw",    "testb",   "testl", /*CH0034*/
    "testw",   "verr",    "verw",    "xchgb",   "xchgl",   "xchgw", /*CH0034*/
    "xlat",    "xorb",    "xorl",    "xorw"                         /*CH0034*/
};

/*
 * IM = immediate, RR = gen_reg, RX = reg_mem
 * IR1 = imp_reg_1, IES = imp_regES, ICS = imp_regCS
 * ISS = imp_regSS
 * ANY = ATTR_DEPEND
 * NA = NO_DATA
 * IDI = imp_idxDI
 * ISI = imp_idxSI
 * REL = relative
 * SR = seg_reg
 * OFF = m_offset
 * I3 = implied_3
 */
public Inst386 i386_1_byte_instructions[256] = {
/* 00 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {ADDB, ADDB}},
/* 01 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {ADDW, ADDL}},
/* 02 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {ADDB, ADDB}},
/* 03 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {ADDW, ADDL}},
/* 04 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {ADDB, ADDB}},
/* 05 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {ADDW, ADDL}},
/* 06 */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IES}, {PUSHL, PUSHL}},/*CH0034*/
/* 07 */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IES}, {POPL, POPL}}, /*CH0034*/
/* 08 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {ORB, ORB}},
/* 09 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {ORW, ORL}},
/* 0a */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {ORB, ORB}},
/* 0b */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {ORW, ORL}},
/* 0c */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {ORB, ORB}},
/* 0d */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {ORW, ORL}},
/* 0e */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, ICS}, {PUSHW, PUSHW}},
/* 10 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {ADCB, ADCB}},
/* 11 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {ADCW, ADCL}},
/* 12 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {ADCB, ADCB}},
/* 13 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {ADCW, ADCL}},
/* 14 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {ADCB, ADCB}},
/* 15 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {ADCW, ADCL}},
/* 16 */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, ISS}, {PUSHL, PUSHL}},/*CH0034*/
/* 17 */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, ISS}, {POPL, POPL}}, /*CH0034*/
/* 18 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {SBBB, SBBB}},
/* 19 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {SBBW, SBBL}},
/* 1a */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {SBBB, SBBB}},
/* 1b */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {SBBW, SBBL}},
/* 1c */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {SBBB, SBBB}},
/* 1d */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {SBBW, SBBL}},
/* 1e */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, ISS}, {PUSHL, PUSHL}},/*CH0034*/
/* 1f */ {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, ISS}, {POPL, POPL}}, /*CH0034*/
/* 20 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {ANDB, ANDB}},
/* 21 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {ANDW, ANDL}},
/* 22 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {ANDB, ANDB}},
/* 23 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {ANDW, ANDL}},
/* 24 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {ANDB, ANDB}},
/* 25 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {ANDW, ANDL}},
/* 26 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_ES}},  /*CH0034*/
/* 27 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {DAA, DAA}},
/* 28 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {SUBB, SUBB}},
/* 29 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {SUBW, SUBL}},
/* 2a */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {SUBB, SUBB}},
/* 2b */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {SUBW, SUBL}},
/* 2c */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {SUBB, SUBB}},
/* 2d */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {SUBW, SUBL}},
/* 2e */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_CS}},  /*CH0034*/
/* 2f */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {DAS, DAS}},
/* 30 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {XORB, XORB}},
/* 31 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {XORW, XORL}},
/* 32 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {XORB, XORB}},
/* 33 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {XORW, XORL}},
/* 34 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {XORB, XORB}},
/* 35 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {XORW, XORL}},
/* 36 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_SS}},  /*CH0034*/
/* 37 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {AAA, AAA}},
/* 38 */ {{NA, NULL_OP}, {BYTE, RR}, {BYTE, RX}, {CMPB, CMPB}},
/* 39 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {CMPW, CMPL}},
/* 3a */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {CMPB, CMPB}},
/* 3b */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {CMPW, CMPL}},
/* 3c */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {CMPB, CMPB}},
/* 3d */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {CMPW, CMPL}},
/* 3e */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_DS}},  /*CH0034*/
/* 3f */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {AAS, AAS}},
/* 40 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR1}, {INCW, INCL}},
/* 41 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR2}, {INCW, INCL}},
/* 42 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR3}, {INCW, INCL}},
/* 43 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR4}, {INCW, INCL}},
/* 44 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR5}, {INCW, INCL}},
/* 45 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR6}, {INCW, INCL}},
/* 46 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR7}, {INCW, INCL}},
/* 47 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR8}, {INCW, INCL}},
/* 48 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR1}, {DECW, DECL}},
/* 49 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR2}, {DECW, DECL}},
/* 4a */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR3}, {DECW, DECL}},
/* 4b */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR4}, {DECW, DECL}},
/* 4c */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR5}, {DECW, DECL}},
/* 4d */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR6}, {DECW, DECL}},
/* 4e */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR7}, {DECW, DECL}},
/* 4f */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR8}, {DECW, DECL}},
/* 50 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR1}, {PUSHW, PUSHL}},
/* 51 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR2}, {PUSHW, PUSHL}},
/* 52 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR3}, {PUSHW, PUSHL}},
/* 53 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR4}, {PUSHW, PUSHL}},
/* 54 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR5}, {PUSHW, PUSHL}},
/* 55 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR6}, {PUSHW, PUSHL}},
/* 56 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR7}, {PUSHW, PUSHL}},
/* 57 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR8}, {PUSHW, PUSHL}},
/* 58 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR1}, {POPW, POPL}},
/* 59 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR2}, {POPW, POPL}},
/* 5a */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR3}, {POPW, POPL}},
/* 5b */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR4}, {POPW, POPL}},
/* 5c */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR5}, {POPW, POPL}},
/* 5d */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR6}, {POPW, POPL}},
/* 5e */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR7}, {POPW, POPL}},
/* 5f */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, IR8}, {POPW, POPL}},
/* 60 */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{PUSHAW, PUSHAL}},/*CH0034*/
/* 61 */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{POPAW, POPAL}},/*CH0034*/
/* 62 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {BOUNDW, BOUNDL}},   /*CH0034*/
/* 63 */ {{NA, NULL_OP}, {WORD, RR}, {WORD, RX}, {ARPL, ARPL}},
/* 64 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_FS}},  /*CH0034*/
/* 65 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {OVRD_GS}},  /*CH0034*/
/* 68 */ {{ANY, IM}, {NA, NULL_OP}, {NA, NULL_OP}, {PUSHW, PUSHL}},
/* 69 */ {{ANY, IM}, {ANY, RX}, {ANY, RR}, {IMULW, IMULL}},
/* 6a */ {{BYTE, IM}, {NA, NULL_OP}, {NA, NULL_OP}, {PUSHW, PUSHL}},/*CH0034*/
/* 6b */ {{BYTE, IM}, {ANY, RX}, {ANY, RR}, {IMULW, IMULL}},        /*CH0034*/
/* 6c */ {{NA, NULL_OP}, {WORD, IR3}, {ANY, IDI}, {INSB, INSB}},
/* 6d */ {{NA, NULL_OP}, {WORD, IR3}, {ANY, IDI}, {INSW, INSL}},
/* 6e */ {{NA, NULL_OP}, {ANY, ISI}, {WORD, IR3}, {OUTSB, OUTSB}},
/* 6f */ {{NA, NULL_OP}, {ANY, ISI}, {WORD, IR3}, {OUTSW, OUTSL}},  /*CH0034*/
/* 70 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JO, JO}},
/* 71 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNO, JNO}},
/* 72 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JB, JB}},
/* 73 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNB, JNB}},
/* 74 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JZ, JZ}},
/* 75 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNZ, JNZ}},
/* 76 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JBE, JBE}},
/* 77 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNBE, JNBE}},
/* 78 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JS, JS}},
/* 79 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNS, JNS}},
/* 7a */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JPE, JPE}},   /*CH0034*/
/* 7b */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JPO, JPO}},   /*CH0034*/
/* 7c */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JL, JL}},
/* 7d */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNL, JNL}},
/* 7e */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JLE, JLE}},
/* 7f */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JNLE, JNLE}},
/* 84 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {TESTB, TESTB}},
/* 85 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {TESTW, TESTL}},
/* 86 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {XCHGB, XCHGB}},
/* 87 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {XCHGW, XCHGL}},
/* 88 */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {MOVB, MOVB}},
/* 89 */ {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {MOVW, MOVL}},
/* 8a */ {{NA, NULL_OP}, {BYTE, RX}, {BYTE, RR}, {MOVB, MOVB}},
/* 8b */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {MOVW, MOVL}},
/* 8c */ {{NA, NULL_OP}, {WORD, SR}, {WORD, RX}, {MOVW, MOVW}},
/* 8d */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {LEAW, LEAL}},
/* 8e */ {{NA, NULL_OP}, {WORD, RX}, {WORD, SR}, {MOVW, MOVW}},
/* 8f */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {POPW, POPL}},
/* 90 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {NOP, NOP}},
/* 91 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR2}, {XCHGW, XCHGL}},
/* 92 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR3}, {XCHGW, XCHGL}},
/* 93 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR4}, {XCHGW, XCHGL}},
/* 94 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR5}, {XCHGW, XCHGL}},
/* 95 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR6}, {XCHGW, XCHGL}},
/* 96 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR7}, {XCHGW, XCHGL}},
/* 97 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IR8}, {XCHGW, XCHGL}},
/* 98 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CBTW, CWTL}},/*CH0034*/
/* 99 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CWTD, CLTD}},/*CH0034*/
/* 9a */ {{NA, NULL_OP}, {WORD, OFF}, {ANY, OFF}, {LCALL, LCALL}},  /*CH0034*/
/* 9b */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{FWAIT, FWAIT}},/*CH0034*/
/* 9c */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{PUSHFW, PUSHFL}},/*CH0034*/
/* 9d */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{POPFW, POPFL}},/*CH0034*/
/* 9e */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {SAHF, SAHF}},
/* 9f */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {LAHF, LAHF}},
/* a0 */ {{NA, NULL_OP}, {BYTE, OFF}, {BYTE, IR1}, {MOVB, MOVB}},
/* a1 */ {{NA, NULL_OP}, {ANY, OFF}, {ANY, IR1}, {MOVW, MOVL}},
/* a2 */ {{NA, NULL_OP}, {BYTE, IR1}, {BYTE, OFF}, {MOVB, MOVB}},
/* a3 */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, OFF}, {MOVW, MOVL}},
/* a4 */ {{NA, NULL_OP}, {BYTE, ISI}, {BYTE, IDI}, {SMOVB, SMOVB}}, /*CH0034*/
/* a5 */ {{NA, NULL_OP}, {ANY, ISI}, {ANY, IDI}, {SMOVW, SMOVL}},   /*CH0034*/
/* a6 */ {{NA, NULL_OP}, {BYTE, ISI}, {BYTE, IDI}, {SCMPB, SCMPB}}, /*CH0034*/
/* a7 */ {{NA, NULL_OP}, {ANY, ISI}, {ANY, IDI}, {SCMPW, SCMPL}},   /*CH0034*/
/* a8 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {TESTB, TESTB}},
/* a9 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {TESTW, TESTL}},
/* aa */ {{NA, NULL_OP}, {BYTE, IR1}, {BYTE, IDI}, {SSTOB, SSTOB}}, /*CH0034*/
/* ab */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IDI}, {SSTOW, SSTOL}},   /*CH0034*/
/* ac */ {{NA, NULL_OP}, {BYTE, ISI}, {BYTE, IR1}, {SLODB, SLODB}}, /*CH0034*/
/* ad */ {{NA, NULL_OP}, {ANY, ISI}, {ANY, IR1}, {SLODW, SLODL}},   /*CH0034*/
/* ae */ {{NA, NULL_OP}, {BYTE, IR1}, {BYTE, IDI}, {SSCAB, SSCAB}}, /*CH0034*/
/* af */ {{NA, NULL_OP}, {ANY, IR1}, {ANY, IDI}, {SSCAW, SSCAL}},   /*CH0034*/
/* b0 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {MOVB, MOVB}},
/* b1 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR2}, {MOVB, MOVB}},
/* b2 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR3}, {MOVB, MOVB}},
/* b3 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR4}, {MOVB, MOVB}},
/* b4 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR5}, {MOVB, MOVB}},
/* b5 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR6}, {MOVB, MOVB}},
/* b6 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR7}, {MOVB, MOVB}},
/* b7 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR8}, {MOVB, MOVB}},
/* b8 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {MOVW, MOVL}},
/* b9 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR2}, {MOVW, MOVL}},
/* ba */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR3}, {MOVW, MOVL}},
/* bb */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR4}, {MOVW, MOVL}},
/* bc */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR5}, {MOVW, MOVL}},
/* bd */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR6}, {MOVW, MOVL}},
/* be */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR7}, {MOVW, MOVL}},
/* bf */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR8}, {MOVW, MOVL}},
/* c2 */ {{WORD, IM}, {NA, NULL_OP}, {NA, NULL_OP}, {RET, RET}},
/* c3 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {RET, RET}},
/* c4 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {LESW, LESL}},
/* c5 */ {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {LDSW, LDSL}},
/* c6 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {MOVB, MOVB}},
/* c7 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {MOVW, MOVL}},
/* c8 */ {{BYTE, IM}, {WORD, IM}, {NA, NULL_OP}, {ENTER, ENTER}},
/* c9 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {LEAVE, LEAVE}},
/* ca */ {{WORD, IM}, {NA, NULL_OP}, {NA, NULL_OP}, {RET, RET}},
/* cb */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {RET, RET}},
/* cc */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, I3}, {INT, INT}},
/* cd */ {{BYTE, IM}, {NA, NULL_OP}, {NA, NULL_OP}, {INT, INT}},
/* ce */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {INTO, INTO}},
/* cf */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {IRET, IRET}},/*CH0034*/
/* d4 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {AAM, AAM}},
/* d5 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {AAD, AAD}},
/* d7 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {XLAT, XLAT}},/*CH0034*/
/* e0 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {LOOPNE, LOOPNE}},
/* e1 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {LOOPE, LOOPE}},
/* e2 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {LOOP, LOOP}},
/* e3 */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JCXZ, JECXZ}},
/* e4 */ {{BYTE, IM}, {NA, NULL_OP}, {BYTE, IR1}, {INB, INB}},
/* e5 */ {{ANY, IM}, {NA, NULL_OP}, {ANY, IR1}, {INW, INL}},
/* e6 */ {{NA, NULL_OP}, {BYTE, IR1}, {BYTE, IM}, {OUTB, OUTB}},
/* e7 */ {{NA, NULL_OP}, {ANY, IR1}, {BYTE, IM}, {OUTW, OUTL}},
/* e8 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {CALL, CALL}},
/* e9 */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JMP, JMP}},
/* ea */ {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LJMP, LJMP}},   /*CH0034*/
/* eb */ {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, REL}, {JMP, JMP}},
/* ec */ {{NA, NULL_OP}, {WORD, IR3}, {BYTE, IR1}, {INB, INB}},
/* ed */ {{NA, NULL_OP}, {WORD, IR3}, {ANY, IR1}, {INW, INL}},
/* ee */ {{NA, NULL_OP}, {BYTE, IR1}, {WORD, IR3}, {OUTB, OUTB}},
/* ef */ {{NA, NULL_OP}, {ANY, IR1}, {WORD, IR3}, {OUTW, OUTL}},
/* f0 */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{LOCK, LOCK}},  /*CH0034*/
/* f2 */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{REPNE, REPNE}},/*CH0034*/
/* f3 */ {{NA, NULL_OP},{NA, NULL_OP},{NA, NULL_OP},{REPE, REPE}},  /*CH0034*/
/* f4 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {HLT, HLT}},
/* f5 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CMC, CMC}},
/* f8 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CLC, CLC}},
/* f9 */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {STC, STC}},
/* fa */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CLI, CLI}},
/* fb */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {STI, STI}},
/* fc */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CLD, CLD}},
/* fd */ {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {STD, STD}},
};

public byte_1_index_values i386_byte_1_indexes[] = {
    one_b_00, one_b_01, one_b_02, one_b_03,
    one_b_04, one_b_05, one_b_06, one_b_07,
    one_b_08, one_b_09, one_b_0a, one_b_0b,
    one_b_0c, one_b_0d, one_b_0e, two_byte,
    one_b_10, one_b_11, one_b_12, one_b_13,
    one_b_14, one_b_15, one_b_16, one_b_17,
    one_b_18, one_b_19, one_b_1a, one_b_1b,
    one_b_1c, one_b_1d, one_b_1e, one_b_1f,
    one_b_20, one_b_21, one_b_22, one_b_23,
    one_b_24, one_b_25, one_b_26, one_b_27,                         /*CH0034*/
    one_b_28, one_b_29, one_b_2a, one_b_2b,
    one_b_2c, one_b_2d, one_b_2e, one_b_2f,                         /*CH0034*/
    one_b_30, one_b_31, one_b_32, one_b_33,
    one_b_34, one_b_35, one_b_36, one_b_37,                         /*CH0034*/
    one_b_38, one_b_39, one_b_3a, one_b_3b,
    one_b_3c, one_b_3d, one_b_3e, one_b_3f,                         /*CH0034*/
    one_b_40, one_b_41, one_b_42, one_b_43,
    one_b_44, one_b_45, one_b_46, one_b_47,
    one_b_48, one_b_49, one_b_4a, one_b_4b,
    one_b_4c, one_b_4d, one_b_4e, one_b_4f,
    one_b_50, one_b_51, one_b_52, one_b_53,
    one_b_54, one_b_55, one_b_56, one_b_57,
    one_b_58, one_b_59, one_b_5a, one_b_5b,
    one_b_5c, one_b_5d, one_b_5e, one_b_5f,
    one_b_60, one_b_61, one_b_62, one_b_63,
    one_b_64, one_b_65, ILL_8_BIT, ILL_8_BIT,                       /*CH0034*/
    one_b_68, one_b_69, one_b_6a, one_b_6b,
    one_b_6c, one_b_6d, one_b_6e, one_b_6f,
    one_b_70, one_b_71, one_b_72, one_b_73,
    one_b_74, one_b_75, one_b_76, one_b_77,
    one_b_78, one_b_79, one_b_7a, one_b_7b,
    one_b_7c, one_b_7d, one_b_7e, one_b_7f,
    extended1, extended1, ILL_8_BIT, extended1,
    one_b_84, one_b_85, one_b_86, one_b_87,
    one_b_88, one_b_89, one_b_8a, one_b_8b,
    one_b_8c, one_b_8d, one_b_8e, one_b_8f,
    one_b_90, one_b_91, one_b_92, one_b_93,
    one_b_94, one_b_95, one_b_96, one_b_97,
    one_b_98, one_b_99, one_b_9a, one_b_9b,
    one_b_9c, one_b_9d, one_b_9e, one_b_9f,
    one_b_a0, one_b_a1, one_b_a2, one_b_a3,
    one_b_a4, one_b_a5, one_b_a6, one_b_a7,
    one_b_a8, one_b_a9, one_b_aa, one_b_ab,
    one_b_ac, one_b_ad, one_b_ae, one_b_af,
    one_b_b0, one_b_b1, one_b_b2, one_b_b3,
    one_b_b4, one_b_b5, one_b_b6, one_b_b7,
    one_b_b8, one_b_b9, one_b_ba, one_b_bb,
    one_b_bc, one_b_bd, one_b_be, one_b_bf,
    extended1, extended1, one_b_c2, one_b_c3,
    one_b_c4, one_b_c5, one_b_c6, one_b_c7,
    one_b_c8, one_b_c9, one_b_ca, one_b_cb,
    one_b_cc, one_b_cd, one_b_ce, one_b_cf,
    extended1, extended1, extended1, extended1,
    one_b_d4, one_b_d5, ILL_8_BIT, one_b_d7,
    i387_11b, i387_11b, i387_11b, i387_11b,
    i387_11b, i387_11b, i387_11b, i387_11b,
    one_b_e0, one_b_e1, one_b_e2, one_b_e3,
    one_b_e4, one_b_e5, one_b_e6, one_b_e7,
    one_b_e8, one_b_e9, one_b_ea, one_b_eb,
    one_b_ec, one_b_ed, one_b_ee, one_b_ef,
    one_b_f0, ILL_8_BIT, one_b_f2, one_b_f3,                        /*CH0034*/
    one_b_f4, one_b_f5, extended1, extended1,
    one_b_f8, one_b_f9, one_b_fa, one_b_fb,
    one_b_fc, one_b_fd, extended1, extended1                        /*CH0034*/
};

public modRM_extended_11_bit_indexes i386_11_bit_indexes[][8] = {
    {grp_1a_0, grp_1a_1, grp_1a_2, grp_1a_3,
    grp_1a_4, grp_1a_5, grp_1a_6, grp_1a_7},
    {grp_1b_0, grp_1b_1, grp_1b_2, grp_1b_3,
    grp_1b_4, grp_1b_5, grp_1b_6, grp_1b_7},
    {grp_1c_0, grp_1c_1, grp_1c_2, grp_1c_3,
    grp_1c_4, grp_1c_5, grp_1c_6, grp_1c_7},
    {grp_2a_0, grp_2a_1, grp_2a_2, grp_2a_3,
    grp_2a_4, grp_2a_5, ILL_11_BIT, grp_2a_7},
    {grp_2b_0, grp_2b_1, grp_2b_2, grp_2b_3,
    grp_2b_4, grp_2b_5, ILL_11_BIT, grp_2b_7},
    {grp_2c_0, grp_2c_1, grp_2c_2, grp_2c_3,
    grp_2c_4, grp_2c_5, ILL_11_BIT, grp_2c_7},
    {grp_2d_0, grp_2d_1, grp_2d_2, grp_2d_3,
    grp_2d_4, grp_2d_5, ILL_11_BIT, grp_2d_7},
    {grp_2a_0, grp_2e_1, grp_2e_2, grp_2e_3,
    grp_2e_4, grp_2e_5, ILL_11_BIT, grp_2e_7},
    {grp_2f_0, grp_2f_1, grp_2f_2, grp_2f_3,
    grp_2f_4, grp_2f_5, ILL_11_BIT, grp_2f_7},
    {grp_3a_0, ILL_11_BIT, grp_3a_2, grp_3a_3,
    grp_3a_4, grp_3a_5, grp_3a_6, grp_3a_7},
    {grp_3b_0, ILL_11_BIT, grp_3b_2, grp_3b_3,
    grp_3b_4, grp_3b_5, grp_3b_6, grp_3b_7},
    {grp_4_0, grp_4_1, ILL_11_BIT, ILL_11_BIT,
    ILL_11_BIT, ILL_11_BIT, ILL_11_BIT, ILL_11_BIT},
    {grp_5_0, grp_5_1, grp_5_2, grp_5_3,
    grp_5_4, grp_5_5, grp_5_6, ILL_11_BIT},
};

public Inst386 i386_11_bit_instructions[] = {
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {ADDB, ADDB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {ORB, ORB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {ADCB, ADCB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {SBBB, SBBB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {ANDB, ANDB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {SUBB, SUBB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {XORB, XORB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {CMPB, CMPB}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {ADDW, ADDL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {ORW, ORL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {ADCW, ADCL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {SBBW, SBBL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {ANDW, ANDL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {SUBW, SUBL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {XORW, XORL}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {CMPW, CMPL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {ADDW, ADDL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {ORW, ORL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {ADCW, ADCL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {SBBW, SBBL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {ANDW, ANDL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {SUBW, SUBL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {XORW, XORL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {CMPW, CMPL}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {ROLB, ROLB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {RORB, RORB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {RCLB, RCLB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {RCRB, RCRB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {SALB, SALB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {SHRB, SHRB}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {SARB, SARB}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {ROLW, ROLL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {RORW, RORL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {RCLW, RCLL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {RCRW, RCRL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {SALW, SALL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {SHRW, SHRL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {SARW, SARL}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {ROLB, ROLB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {RORB, RORB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {RCLB, RCLB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {RCRB, RCRB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {SALB, SALB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {SHRB, SHRB}},
    {{BYTE, I1}, {NA, NULL_OP}, {BYTE, RX}, {SARB, SARB}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {ROLW, ROLL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {RORW, RORL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {RCLW, RCLL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {RCRW, RCRL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {SALW, SALL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {SHRW, SHRL}},
    {{BYTE, I1}, {NA, NULL_OP}, {ANY, RX}, {SARW, SARL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {ROLB, ROLB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {RORB, RORB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {RCLB, RCLB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {RCRB, RCRB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {SALB, SALB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {SHRB, SHRB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {BYTE, RX}, {SARB, SARB}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {ROLW, ROLL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {RORW, RORL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {RCLW, RCLL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {RCRW, RCRL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {SALW, SALL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {SHRW, SHRL}},
    {{BYTE, IR2}, {NA, NULL_OP}, {ANY, RX}, {SARW, SARL}},
    {{BYTE, IM}, {NA, NULL_OP}, {BYTE, RX}, {TESTB, TESTB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {NOTB, NOTB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {NEGB, NEGB}},
    {{NA, NULL_OP}, {BYTE, RX}, {BYTE, IR1}, {MULB, MULB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {IMULB, IMULB}},
    {{NA, NULL_OP}, {BYTE, RX}, {BYTE, IR1}, {DIVB, DIVB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {IDIVB, IDIVB}},
    {{ANY, IM}, {NA, NULL_OP}, {ANY, RX}, {TESTW, TESTL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {NOTW, NOTL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {NEGW, NEGL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, IR1}, {MULW, MULL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {IMULW, IMULL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, IR1}, {DIVW, DIVL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, IR1}, {IDIVW, IDIVL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {INCB, INCB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {DECB, DECB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {INCW, INCL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {DECW, DECL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {CALL, CALL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LCALL, LCALL}},      /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {JMP, JMP}},          /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LJMP, LJMP}},        /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {PUSHW, PUSHL}},
};

public Inst386 i386_2_byte_instructions[] = {
    {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {LARW, LARL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {LSLW, LSLL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {NA, NULL_OP}, {CLTS, CLTS}},
    {{NA, NULL_OP}, {LONG, CR}, {LONG, RX}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {LONG, DR}, {LONG, RX}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {LONG, RX}, {LONG, CR}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {LONG, RX}, {LONG, DR}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {LONG, TR}, {LONG, RX}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {LONG, RX}, {LONG, TR}, {MOVL, MOVL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JO, JO}},           /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNO, JNO}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JB, JB}},           /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNB, JNB}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JZ, JZ}},           /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNZ, JNZ}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JBE, JBE}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNBE, JNBE}},       /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JS, JS}},           /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNS, JNS}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JPE, JPE}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JPO, JPO}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JL, JL}},           /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNL, JNL}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JLE, JLE}},         /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, REL}, {JNLE, JNLE}},       /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETO, SETO}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNO, SETNO}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETB, SETB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNB, SETNB}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETZ, SETZ}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNZ, SETNZ}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETBE, SETBE}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNBE, SETNBE}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETS, SETS}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNS, SETNS}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETP, SETP}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNP, SETNP}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETL, SETL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNL, SETNL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETLE, SETLE}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {BYTE, RX}, {SETNLE, SETNLE}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IFS}, {PUSHL, PUSHL}},    /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IFS}, {POPL, POPL}},      /*CH0034*/
    {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {BTW, BTL}},
    {{BYTE, IM}, {ANY, RR}, {ANY, RX}, {SHLDW, SHLDL}},
    {{BYTE, IR2}, {ANY, RR}, {ANY, RX}, {SHLDW, SHLDL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IGS}, {PUSHL, PUSHL}},    /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, IGS}, {POPL, POPL}},      /*CH0034*/
    {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {BTSW, BTSL}},
    {{BYTE, IM}, {ANY, RR}, {ANY, RX}, {SHRDW, SHRDL}},
    {{BYTE, IR2}, {ANY, RR}, {ANY, RX}, {SHRDW, SHRDL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {IMULW, IMULL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LSSW, LSSL}},
    {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {BTRW, BTRL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LFSW, LFSL}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LGSW, LGSL}},
    {{NA, NULL_OP}, {BYTE, RX}, {ANY, RR}, {MOVZBW, MOVZBL}},       /*CH0034*/
    {{NA, NULL_OP}, {WORD, RX}, {LONG, RR}, {MOVZWL, MOVZWL}},      /*CH0034*/
    {{NA, NULL_OP}, {ANY, RR}, {ANY, RX}, {BTCW, BTCL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {BSFW, BSFL}},
    {{NA, NULL_OP}, {ANY, RX}, {ANY, RR}, {BSRW, BSRL}},
    {{NA, NULL_OP}, {BYTE, RX}, {ANY, RR}, {MOVSBW, MOVSBL}},       /*CH0034*/
    {{NA, NULL_OP}, {WORD, RX}, {LONG, RR}, {MOVSWL, MOVSWL}},      /*CH0034*/
};

public byte_2_index_values i386_byte_2_indexes[] = {
    extended2, extended2, two_b_02, two_b_03,
    ILL_16_BIT, ILL_16_BIT, two_b_06, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    two_b_20, two_b_21, two_b_22, two_b_23,
    two_b_24, ILL_16_BIT, two_b_26, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    two_b_80, two_b_81, two_b_82, two_b_83,
    two_b_84, two_b_85, two_b_86, two_b_87,
    two_b_88, two_b_89, two_b_8a, two_b_8b,
    two_b_8c, two_b_8d, two_b_8e, two_b_8f,
    two_b_90, two_b_91, two_b_92, two_b_93,
    two_b_94, two_b_95, two_b_96, two_b_97,
    two_b_98, two_b_99, two_b_9a, two_b_9b,
    two_b_9c, two_b_9d, two_b_9e, two_b_9f,
    two_b_a0, two_b_a1, ILL_16_BIT, two_b_a3,
    two_b_a4, two_b_a5, ILL_16_BIT, ILL_16_BIT,
    two_b_a8, two_b_a9, ILL_16_BIT, two_b_ab,
    two_b_ac, two_b_ad, ILL_16_BIT, two_b_af,
    ILL_16_BIT, ILL_16_BIT, two_b_b2, two_b_b3,
    two_b_b4, two_b_b5, two_b_b6, two_b_b7,
    ILL_16_BIT, ILL_16_BIT, extended2, two_b_bb,
    two_b_bc, two_b_bd, two_b_be, two_b_bf,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
    ILL_16_BIT, ILL_16_BIT, ILL_16_BIT, ILL_16_BIT,
};

public ext_19_bit_indexes i386_19_bit_indexes[][8] = {
    grp_6_0, grp_6_1, grp_6_2, grp_6_3,
    grp_6_4, grp_6_5, ILL_19_BIT, ILL_19_BIT,
    grp_7_0, grp_7_1, grp_7_2, grp_7_3,
    grp_7_4, ILL_19_BIT, grp_7_6, ILL_19_BIT,
    ILL_19_BIT, ILL_19_BIT, ILL_19_BIT, ILL_19_BIT,
    grp_8_4, grp_8_5, grp_8_6, grp_8_7,
};

public Inst386 i386_19_bit_instructions[] = {
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {SLDT, SLDT}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {STKR, STKR}},       /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {LLDT, LLDT}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {LTR, LTR}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {VERR, VERR}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {VERW, VERW}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {SGDT, SGDT}},        /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {SIDT, SIDT}},        /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LGDT, LGDT}},        /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {ANY, RX}, {LIDT, LIDT}},        /*CH0034*/
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {SMSW, SMSW}},
    {{NA, NULL_OP}, {NA, NULL_OP}, {WORD, RX}, {LMSW, LMSW}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {BTW, BTL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {BTSW, BTSL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {BTRW, BTRL}},
    {{BYTE, IM}, {NA, NULL_OP}, {ANY, RX}, {BTCW, BTCL}},
};

public i387_indexes i387_reg_indexes[][8] = {
    /* starts at 216 */
    {d8_0, d8_1, d8_2, d8_3, d8_4, d8_5, d8_6, d8_7},
    {d9_0, d9_1, d9_2, d9_3, d9_4, d9_5, d9_6, d9_7},
    {da_0, da_1, da_2, da_3, da_4, da_5, da_6, da_7},
    {db_0, ILL_387_INST, db_2, db_3, db_4, db_5, ILL_387_INST, db_7},
    {dc_0, dc_1, dc_2, dc_3, dc_4, dc_5, dc_6, dc_7},
    {dd_0, ILL_387_INST, dd_2, dd_3, dd_4, dd_5, dd_6, dd_7},
    {de_0, de_1, de_2, de_3, de_4, de_5, de_6, de_7},
    {df_0, ILL_387_INST, df_2, df_3, df_4, df_5, df_6, df_7},
};

public i387_indexes i387_mod_indexes[][2] = {
    /* starts at d8_0 */
    /* true, false */
    {d8_0a, d8_0b}, {d8_1a, d8_1b},
    {d8_2a, d8_2b}, {d8_3a, d8_3b},                                 /*CH0034*/
    {d8_4a, d8_4b}, {d8_5a, d8_5b},
    {d8_6a, d8_6b}, {d8_7a, d8_7b},
    {d9_0a, d9_0b}, {d9_2a, d9_2b},                                 /*CH0034*/
    {d9_4a, d9_4b}, {d9_5a, d9_5b},                                 /*CH0034*/
    {d9_6a, d9_6b}, {d9_7a, d9_7b},                                 /*CH0034*/
    {da_5a, da_5b}, {ILL_387_INST, db_4b},                          /*CH0034*/
    {dc_0a, dc_0b}, {dc_1a, dc_1b},                                 /*CH0034*/
    {dc_4a, dc_4b}, {dc_5a, dc_5b},                                 /*CH0034*/
    {dc_6a, dc_6b}, {dc_7a, dc_7b},                                 /*CH0034*/
    {dd_0a, dd_0b}, {dd_2a, dd_2b},                                 /*CH0034*/
    {dd_3a, dd_3b}, {dd_4a, dd_4b},                                 /*CH0034*/
    {de_0a, de_0b}, {de_1a, de_1b},                                 /*CH0034*/
    {de_3a, de_3b}, {de_4a, de_4b},                                 /*CH0034*/
    {de_5a, de_5b}, {de_6a, de_6b},                                 /*CH0034*/
    {de_7a, de_7b}, {df_4a, df_4b}                                  /*CH0034*/
};

public i387_indexes i387_R_M_indexes[][8] = {
    /* starts at d9_4b */
    {d9_4b0, d9_4b1, ILL_387_INST, ILL_387_INST,
    d9_4b4, d9_4b5, ILL_387_INST, ILL_387_INST},
    {d9_5b0, d9_5b1, d9_5b2, d9_5b3,
    d9_5b4, d9_5b5, d9_5b6, ILL_387_INST},
    {d9_6b0, d9_6b1, d9_6b2, d9_6b3,
    d9_6b4, d9_6b5, d9_6b6, d9_6b7},
    {d9_7b0, d9_7b1, d9_7b2, d9_7b3,
    d9_7b4, d9_7b5, d9_7b6, d9_7b7},
    {db_4b0, db_4b1, db_4b2, db_4b3,
    db_4b4, ILL_387_INST, ILL_387_INST, ILL_387_INST}
};

public Inst387 i387_instructions[] = {
/* d8_0a */ {i387_rm, i387_NULL_OP, FADDS},                         /*CH0034*/
/* d8_0b */ {i387_rm, i387_imp_ST, FADD},
/* d8_1a */ {i387_rm, i387_NULL_OP, FMULS},                         /*CH0034*/
/* d8_1b */ {i387_rm, i387_imp_ST, FMUL},
/* d8_2a */ {i387_rm, i387_NULL_OP, FCOMS},                         /*CH0034*/
/* d8_2b */ {i387_rm, i387_NULL_OP, FCOM},                          /*CH0034*/
/* d8_3a */ {i387_rm, i387_NULL_OP, FCOMPS},                        /*CH0034*/
/* d8_3b */ {i387_rm, i387_NULL_OP, FCOMP},                         /*CH0034*/
/* d8_4a */ {i387_rm, i387_NULL_OP, FSUBS},                         /*CH0034*/
/* d8_4b */ {i387_rm, i387_imp_ST, FSUB},
/* d8_5a */ {i387_rm, i387_NULL_OP, FSUBRS},                        /*CH0034*/
/* d8_5b */ {i387_rm, i387_imp_ST, FSUBR},
/* d8_6a */ {i387_rm, i387_NULL_OP, FDIVS},                         /*CH0034*/
/* d8_6b */ {i387_rm, i387_imp_ST, FDIV},
/* d8_7a */ {i387_rm, i387_NULL_OP, FDIVRS},                        /*CH0034*/
/* d8_7b */ {i387_rm, i387_imp_ST, FDIVR},
/* d9_0a */ {i387_rm, i387_NULL_OP, FLDS},                          /*CH0034*/
/* d9_0b */ {i387_rm, i387_NULL_OP, FLD},                           /*CH0034*/
/* d9_1  */ {i387_rm, i387_NULL_OP, FXCH},
/* d9_2a */ {i387_NULL_OP, i387_rm, FSTS},                          /*CH0034*/
/* d9_2b */ {i387_NULL_OP, i387_NULL_OP, FNOP},
/* d9_3  */ {i387_NULL_OP, i387_rm, FSTPS},                         /*CH0034*/
/* d9_4a */ {i387_rm, i387_NULL_OP, FLDENV},
/* d9_4b0*/ {i387_NULL_OP, i387_NULL_OP, FCHS},
/* d9_4b1*/ {i387_NULL_OP, i387_NULL_OP, FABS},
/* d9_4b4*/ {i387_NULL_OP, i387_NULL_OP, FTST},
/* d9_4b5*/ {i387_NULL_OP, i387_NULL_OP, FXAM},
/* d9_5a */ {i387_rm, i387_NULL_OP, FLDCW},
/* d9_5b0*/ {i387_NULL_OP, i387_NULL_OP, FLD1},
/* d9_5b1*/ {i387_NULL_OP, i387_NULL_OP, FLDL2T},
/* d9_5b2*/ {i387_NULL_OP, i387_NULL_OP, FLDL2E},
/* d9_5b3*/ {i387_NULL_OP, i387_NULL_OP, FLDPI},
/* d9_5b4*/ {i387_NULL_OP, i387_NULL_OP, FLDLG2},
/* d9_5b5*/ {i387_NULL_OP, i387_NULL_OP, FLDLN2},
/* d9_5b6*/ {i387_NULL_OP, i387_NULL_OP, FLDZ},
/* d9_6a */ {i387_rm, i387_NULL_OP, FNSTENV},                       /*CH0034*/
/* d9_6b0*/ {i387_NULL_OP, i387_NULL_OP, F2XM1},
/* d9_6b1*/ {i387_NULL_OP, i387_NULL_OP, FYL2X},
/* d9_6b2*/ {i387_NULL_OP, i387_NULL_OP, FPTAN},
/* d9_6b3*/ {i387_NULL_OP, i387_NULL_OP, FPATAN},
/* d9_6b4*/ {i387_NULL_OP, i387_NULL_OP, FXTRACT},
/* d9_6b5*/ {i387_NULL_OP, i387_NULL_OP, FPREM1},
/* d9_6b6*/ {i387_NULL_OP, i387_NULL_OP, FDECSTP},
/* d9_6b7*/ {i387_NULL_OP, i387_NULL_OP, FINCSTP},
/* d9_7a */ {i387_NULL_OP, i387_rm, FNSTCW},                       /*CH0034*/
/* d9_7b0*/ {i387_NULL_OP, i387_NULL_OP, FPREM},
/* d9_7b1*/ {i387_NULL_OP, i387_NULL_OP, FYL2XP1},
/* d9_7b2*/ {i387_NULL_OP, i387_NULL_OP, FSQRT},
/* d9_7b3*/ {i387_NULL_OP, i387_NULL_OP, FSINCOS},
/* d9_7b4*/ {i387_NULL_OP, i387_NULL_OP, FRNDINT},
/* d9_7b5*/ {i387_NULL_OP, i387_NULL_OP, FSCALE},
/* d9_7b6*/ {i387_NULL_OP, i387_NULL_OP, FSIN},
/* d9_7b7*/ {i387_NULL_OP, i387_NULL_OP, FCOS},
/* da_0  */ {i387_rm, i387_NULL_OP, FIADDL},                        /*CH0034*/
/* da_1  */ {i387_rm, i387_NULL_OP, FIMULL},                        /*CH0034*/
/* da_2  */ {i387_rm, i387_NULL_OP, FICOML},                        /*CH0034*/
/* da_3  */ {i387_rm, i387_NULL_OP, FICOMPL},                       /*CH0034*/
/* da_4  */ {i387_rm, i387_NULL_OP, FISUBL},                        /*CH0034*/
/* da_5a */ {i387_rm, i387_NULL_OP, FISUBRL},                       /*CH0034*/
/* da_5b */ {i387_NULL_OP, i387_NULL_OP, FUCOMPP},                  /*CH0034*/
/* da_6  */ {i387_rm, i387_NULL_OP, FIDIVL},                        /*CH0034*/
/* da_7  */ {i387_rm, i387_NULL_OP, FIDIVRL},                       /*CH0034*/
/* db_0  */ {i387_rm, i387_NULL_OP, FILDL},                         /*CH0034*/
/* db_2  */ {i387_NULL_OP, i387_rm, FISTL},                         /*CH0034*/
/* db_3  */ {i387_NULL_OP, i387_rm, FISTPL},                        /*CH0034*/
/* db_4b0*/ {i387_NULL_OP, i387_NULL_OP, FENI},
/* db_4b1*/ {i387_NULL_OP, i387_NULL_OP, FDISI},
/* db_4b2*/ {i387_NULL_OP, i387_NULL_OP, FNCLEX},                   /*CH0034*/
/* db_4b3*/ {i387_NULL_OP, i387_NULL_OP, FNINIT},                   /*CH0034*/
/* db_4b4*/ {i387_NULL_OP, i387_NULL_OP, FSETPM},
/* db_5  */ {i387_NULL_OP, i387_rm, FLDT},                          /*CH0034*/
/* db_7  */ {i387_NULL_OP, i387_rm, FSTPT},                         /*CH0034*/
/* dc_0a */ {i387_rm, i387_NULL_OP, FADDL},                         /*CH0034*/
/* dc_0b */ {i387_rm, i387_imp_ST, FADD},
/* dc_1a */ {i387_rm, i387_NULL_OP, FMULL},                         /*CH0034*/
/* dc_1b */ {i387_rm, i387_imp_ST, FMUL},
/* dc_2  */ {i387_rm, i387_NULL_OP, FCOML},                         /*CH0034*/
/* dc_3  */ {i387_rm, i387_NULL_OP, FCOMPL},                        /*CH0034*/
/* dc_4a */ {i387_rm, i387_NULL_OP, FSUBL},                         /*CH0034*/
/* dc_4b */ {i387_rm, i387_imp_ST, FSUB},                           /*CH0034*/
/* dc_5a */ {i387_rm, i387_NULL_OP, FSUBRL},                        /*CH0034*/
/* dc_5b */ {i387_rm, i387_imp_ST, FSUBR},                          /*CH0034*/
/* dc_6a */ {i387_rm, i387_NULL_OP, FDIVL},                         /*CH0034*/
/* dc_6b */ {i387_rm, i387_imp_ST, FDIV},                           /*CH0034*/
/* dc_7a */ {i387_rm, i387_NULL_OP, FDIVRL},                        /*CH0034*/
/* dc_7b */ {i387_rm, i387_imp_ST, FDIVR},                          /*CH0034*/
/* dd_0a */ {i387_rm, i387_NULL_OP, FLDL},                          /*CH0034*/
/* dd_0b */ {i387_rm, i387_imp_ST, FFREE},
/* dd_2a */ {i387_NULL_OP, i387_rm, FSTL},                          /*CH0034*/
/* dd_2b */ {i387_NULL_OP, i387_rm, FST},                           /*CH0034*/
/* dd_3a */ {i387_NULL_OP, i387_rm, FSTPL},                         /*CH0034*/
/* dd_3b */ {i387_NULL_OP, i387_rm, FSTP},                          /*CH0034*/
/* dd_4a */ {i387_rm, i387_NULL_OP, FRSTOR},
/* dd_4b */ {i387_rm, i387_NULL_OP, FUCOM},                         /*CH0034*/
/* dd_5  */ {i387_NULL_OP, i387_rm, FUCOMP},
/* dd_6  */ {i387_NULL_OP, i387_rm, FNSAVE},                        /*CH0034*/
/* dd_7  */ {i387_NULL_OP, i387_rm, FNSTSW},                        /*CH0034*/
/* de_0a */ {i387_rm, i387_NULL_OP, FIADD},
/* de_0b */ {i387_rm, i387_imp_ST, FADDP},
/* de_1a */ {i387_rm, i387_NULL_OP, FIMUL},
/* de_1b */ {i387_rm, i387_imp_ST, FMULP},
/* de_2  */ {i387_rm, i387_NULL_OP, FICOM},
/* de_3a */ {i387_rm, i387_NULL_OP, FICOMP},
/* de_3b */ {i387_NULL_OP, i387_NULL_OP, FCOMPP},                   /*CH0034*/
/* de_4a */ {i387_rm, i387_NULL_OP, FISUB},
/* de_4b */ {i387_rm, i387_imp_ST, FSUBP},                          /*CH0034*/
/* de_5a */ {i387_rm, i387_NULL_OP, FISUBR},
/* de_5b */ {i387_rm, i387_imp_ST, FSUBRP},                         /*CH0034*/
/* de_6a */ {i387_rm, i387_NULL_OP, FIDIV},
/* de_6b */ {i387_rm, i387_imp_ST, FDIVP},                          /*CH0034*/
/* de_7a */ {i387_rm, i387_NULL_OP, FIDIVR},
/* de_7b */ {i387_rm, i387_imp_ST, FDIVRP},
/* df_0  */ {i387_rm, i387_NULL_OP, FILD},
/* df_2  */ {i387_NULL_OP, i387_rm, FIST},
/* df_3  */ {i387_NULL_OP, i387_rm, FISTP},
/* df_4a */ {i387_rm, i387_NULL_OP, FBLD},
/* df_4b */ {i387_rm, i387_imp_ST, FNSTSW},                         /*CH0034*/
/* df_5  */ {i387_rm, i387_NULL_OP, FILDLL},                        /*CH0034*/
/* df_6  */ {i387_NULL_OP, i387_rm, FBSTP},
/* df_7  */ {i387_NULL_OP, i387_rm, FISTPLL},                       /*CH0034*/
};
