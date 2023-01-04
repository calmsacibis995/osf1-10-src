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
/*	
 *	@(#)$RCSfile: ops.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:13:44 $
 */ 
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
#ifndef H_SCCSID
#include <sccs.h>
#endif /* ! H_SCCSID */

#endif

#ifndef ops_h
#define ops_h

typedef enum {                                                      /*CH0034*/
    AAA,      AAD,      AAM,      AAS,      ADCB,     ADCL,         /*CH0034*/
    ADCW,     ADDB,     ADDL,     ADDW,     ANDB,     ANDL,         /*CH0034*/
    ANDW,     ARPL,     BOUNDL,   BOUNDW,   BSFL,     BSFW,         /*CH0034*/
    BSRL,     BSRW,     BTCL,     BTCW,     BTL,      BTRL,         /*CH0034*/
    BTRW,     BTSL,     BTSW,     BTW,      CALL,     CBTW,         /*CH0034*/
    CLC,      CLD,      CLI,      CLTD,     CLTS,     CMC,          /*CH0034*/
    CMPB,     CMPL,     CMPW,                                       /*CH0034*/
    CWTD,     CWTL,     DAA,      DAS,      DECB,     DECL,         /*CH0034*/
    DECW,     DIVB,     DIVL,     DIVW,     ENTER,    F2XM1,        /*CH0034*/
    FABS,     FADD,     FADDL,    FADDP,    FADDS,    FBLD,         /*CH0034*/
    FBSTP,    FCHS,     FCOM,     FCOML,    FCOMP,    FCOMPL,       /*CH0034*/
    FCOMPP,   FCOMPS,   FCOMS,    FCOS,     FDECSTP,  FDISI,        /*CH0034*/
    FDIV,     FDIVL,    FDIVP,    FDIVR,    FDIVRL,   FDIVRP,       /*CH0034*/
    FDIVRS,   FDIVS,    FENI,     FFREE,    FIADD,    FIADDL,       /*CH0034*/
    FICOM,    FICOML,   FICOMP,   FICOMPL,  FIDIV,                  /*CH0034*/
    FIDIVL,   FIDIVR,   FIDIVRL,  FILD,     FILDL,    FILDLL,       /*CH0034*/
    FIMUL,    FIMULL,   FINCSTP,  FIST,     FISTL,                  /*CH0034*/
    FISTP,    FISTPL,   FISTPLL,  FISUB,    FISUBL,   FISUBR,       /*CH0034*/
    FISUBRL,  FLD,      FLD1,     FLDCW,    FLDENV,   FLDL,         /*CH0034*/
    FLDL2E,   FLDL2T,   FLDLG2,   FLDLN2,   FLDPI,    FLDS,         /*CH0034*/
    FLDT,     FLDZ,     FMUL,     FMULL,    FMULP,    FMULS,        /*CH0034*/
    FNCLEX,   FNINIT,   FNOP,     FNSAVE,   FNSTCW,   FNSTENV,      /*CH0034*/
    FNSTSW,   FPATAN,   FPREM,    FPREM1,   FPTAN,    FRNDINT,      /*CH0034*/
    FRSTOR,   FSCALE,   FSETPM,   FSIN,     FSINCOS,  FSQRT,        /*CH0034*/
    FST,      FSTL,     FSTP,     FSTPL,    FSTPS,    FSTPT,        /*CH0034*/
    FSTS,     FSUB,     FSUBL,    FSUBP,    FSUBR,    FSUBRL,       /*CH0034*/
    FSUBRP,   FSUBRS,   FSUBS,    FTST,     FUCOM,    FUCOMP,       /*CH0034*/
    FUCOMPP,  FWAIT,    FXAM,     FXCH,     FXTRACT,  FYL2X,        /*CH0034*/
    FYL2XP1,  HLT,      IDIVB,    IDIVL,    IDIVW,    IMULB,        /*CH0034*/
    IMULL,    IMULW,    INB,      INCB,     INCL,     INCW,         /*CH0034*/
    INL,      INSB,     INSL,     INSW,     INT,      INTO,         /*CH0034*/
    INW,      IRET,     JB,       JBE,      JCXZ,                   /*CH0034*/
    JECXZ,    JL,       JLE,      JMP,      JNB,      JNBE,         /*CH0034*/
    JNL,      JNLE,     JNO,      JNS,      JNZ,      JO,           /*CH0034*/
    JPE,      JPO,      JS,       JZ,       LAHF,     LARL,         /*CH0034*/
    LARW,     LCALL,    LDSL,     LDSW,     LEAL,     LEAVE,        /*CH0034*/
    LEAW,     LESL,     LESW,     LFSL,     LFSW,     LGDT,         /*CH0034*/
    LGSL,     LGSW,     LIDT,     LJMP,     LLDT,     LMSW,         /*CH0034*/
    LOCK,     LOOP,     LOOPE,                                      /*CH0034*/
    LOOPNE,   LSLL,     LSLW,     LSSL,     LSSW,     LTR,          /*CH0034*/
    MOVB,     MOVL,     MOVSWL,   MOVSBW,   MOVSBL,   MOVW,         /*CH0034*/
    MOVZWL,   MOVZBW,   MOVZBL,   MULB,     MULL,                   /*CH0034*/
    MULW,     NEGB,     NEGL,     NEGW,     NOP,      NOTB,         /*CH0034*/
    NOTL,     NOTW,     ORB,      ORL,      ORW,      OUTB,         /*CH0034*/
    OUTL,     OUTSB,    OUTSL,    OUTSW,    OUTW,     OVRD_CS,      /*CH0034*/
    OVRD_DS,  OVRD_ES,  OVRD_FS,  OVRD_GS,  OVRD_SS,  POPAL,        /*CH0034*/
    POPAW,    POPFL,    POPFW,    POPL,     POPW,     PUSHAL,       /*CH0034*/
    PUSHAW,   PUSHFL,   PUSHFW,   PUSHL,    PUSHW,    RCLB,         /*CH0034*/
    RCLL,     RCLW,     RCRB,     RCRL,     RCRW,     REPE,         /*CH0034*/
    REPNE,    RET,      ROLB,     ROLL,     ROLW,     RORB,         /*CH0034*/
    RORL,     RORW,     SAHF,     SALB,     SALL,     SALW,         /*CH0034*/
    SARB,     SARL,     SARW,     SBBB,     SBBL,     SBBW,         /*CH0034*/
    SETB,     SETBE,    SETL,                                       /*CH0034*/
    SETLE,    SETNB,    SETNBE,   SETNL,    SETNLE,   SETNO,        /*CH0034*/
    SETNP,    SETNS,    SETNZ,    SETO,     SETP,     SETS,         /*CH0034*/
    SETZ,     SGDT,     SHLDL,    SHLDW,    SHRB,     SHRDL,        /*CH0034*/
    SHRDW,    SHRL,     SHRW,     SIDT,     SLDT,     SMSW,         /*CH0034*/
    SCMPB,    SCMPL,    SCMPW,    SLODB,    SLODL,    SLODW,        /*CH0034*/
    SMOVB,    SMOVL,    SMOVW,    SSCAB,    SSCAL,    SSCAW,        /*CH0034*/
    SSTOB,    SSTOL,    SSTOW,    STC,      STD,      STI,          /*CH0034*/
    STKR,     SUBB,     SUBL,     SUBW,     TESTB,    TESTL,        /*CH0034*/
    TESTW,    VERR,     VERW,     XCHGB,    XCHGL,    XCHGW,        /*CH0034*/
    XLAT,     XORB,     XORL,     XORW                              /*CH0034*/
} Opcode;                                                           /*CH0034*/

typedef enum {
    byte_1_complete, i386_1_byte_extended, i386_2_byte, i387_NPX
} byte_1_processing_status;

typedef enum {
    NA, BYTE, WORD, LONG, ANY
} format_size;

typedef enum {
    NULL_OP,
    IM,
    RX,
    RR,
    SR,
    TR,
    DR,
    CR,
    IR1,  /* rb, rw, or rd = 0 -- AL, AX, or EAX */
    IR2,  /* rb, rw, or rd = 1 -- CL, CX, or ECX */
    IR3,  /* rb, rw, or rd = 2 -- DL, DX, o rEDX */
    IR4,  /* rb, rw, or rd = 3 -- BL, BX, or EBX */
    IR5,  /* rb, rw, or rd = 4 -- AH, SP, or ESP */
    IR6,  /* rb, rw, or rd = 5 -- CH, BP, or EBP */
    IR7,  /* rb, rw, or rd = 6 -- DH, SI, or ESI */
    IR8,  /* rb, rw, or rd = 7 -- BH, DI, or EDI */
    IES,  /* segment register ES                 */
    ICS,  /* segment register CS                 */
    ISS,  /* segment register SS                 */
    IDS,  /* segment register DS                 */
    IFS,  /* segment register FS                 */
    IGS,  /* segment register GS                 */
    ISI,  /* m8, m16, or m32 -- SI or ESI        */
    IDI,  /* m8, m16, or m32 -- DI or EDI        */
    OFF,  /* moffs8, moffs16, or moffs32         */
    REL,  /* rel8, rel16, or rel32               */
    I1,   /* 1 -- see shift & rotate ops         */
    I3
} i386_format_type;

typedef enum {
    i387_NULL_OP,
    i387_imp_ST,    /* ST (0) */
    i387_imp_reg_1, /* AX */
    i387_rm
} i387_format_type;

typedef struct {
    format_size size;
    i386_format_type format_type;
} Opnd386;

typedef struct {
    Opnd386 source_immediate;
    Opnd386 source_reg_mem;
    Opnd386 destination;
    Opcode mnemonic[2];
} Inst386;

typedef struct {
    i387_format_type source_type;
    i387_format_type destination_type;
    Opcode opcode;
} Inst387;

/*
 * Note: the REG_MEM operand format is not a valid INSTRUCTION_OPERAND_TYPE.
 * This is because the MOD field of the modRM byte is used to determine 
 * whether the operand is a register of a memory reference.
 */

typedef enum {
    no_operand,
    immed_data,
    memory_address,
    gregister,
    memory_offs,
    relative_addr,
    string_operand,
    constant
} OpndType;

typedef enum {
    R_AL,  R_CL,  R_DL,  R_BL,  R_AH,  R_CH,  R_DH,  R_BH,
    R_AX,  R_CX,  R_DX,  R_BX,  R_SP,  R_BP,  R_SI,  R_DI,
    R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI,
    R_ES,  R_CS,  R_SS,  R_DS,  R_FS,  R_GS,  R_CR0, R_CR2,
    R_CR3, R_DR0, R_DR1, R_DR2, R_DR3, R_DR6, R_DR7, R_TR6,
    R_TR7, R_ST0, R_ST1, R_ST2, R_ST3, R_ST4, R_ST5, R_ST6,
    R_ST7, NO_REG
} Regname;

typedef enum {
    NO_SCALE, two, four, eight
} Scale;

typedef struct {
    Regname base;
    Regname index;
    Scale scale;
    long displacement;
} R_M_address;

typedef struct {
    OpndType format;
    union {
	long data;		/* immed_data */
	R_M_address components;	/* memory_address */
	Regname name;	/* gregister */
	long offset;		/* memory_offs or relative_addr */
	R_M_address str_cmpts;	/* string_operand */
	int constant_val;	/* constant */
    } value;
} Operand;

typedef struct {
    Opcode mnemonic;
    Operand source_immediate;
    Operand source_register_memory;
    Operand destination;
} Instruction;

typedef enum {
    one_b_00, one_b_01, one_b_02, one_b_03,
    one_b_04, one_b_05, one_b_06, one_b_07,
    one_b_08, one_b_09, one_b_0a, one_b_0b,
    one_b_0c, one_b_0d, one_b_0e,
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
    one_b_64, one_b_65,                                             /*CH0034*/
    one_b_68, one_b_69, one_b_6a, one_b_6b,
    one_b_6c, one_b_6d, one_b_6e, one_b_6f,
    one_b_70, one_b_71, one_b_72, one_b_73,
    one_b_74, one_b_75, one_b_76, one_b_77,
    one_b_78, one_b_79, one_b_7a, one_b_7b,
    one_b_7c, one_b_7d, one_b_7e, one_b_7f,

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
    one_b_c2, one_b_c3,                                             /*CH0034*/
    one_b_c4, one_b_c5, one_b_c6, one_b_c7,
    one_b_c8, one_b_c9, one_b_ca, one_b_cb,
    one_b_cc, one_b_cd, one_b_ce, one_b_cf,
				      
    one_b_d4, one_b_d5,           one_b_d7,
				      
				      
    one_b_e0, one_b_e1, one_b_e2, one_b_e3,
    one_b_e4, one_b_e5, one_b_e6, one_b_e7,
    one_b_e8, one_b_e9, one_b_ea, one_b_eb,
    one_b_ec, one_b_ed, one_b_ee, one_b_ef,
    one_b_f0,           one_b_f2, one_b_f3,                         /*CH0034*/
    one_b_f4, one_b_f5,
    one_b_f8, one_b_f9, one_b_fa, one_b_fb,
    one_b_fc, one_b_fd,

    extended1, i387_11b, two_byte, ILL_8_BIT
} byte_1_index_values;

typedef enum {
    two_b_02, two_b_03,                                             /*CH0034*/
    two_b_06,                                                       /*CH0034*/
    two_b_20, two_b_21, two_b_22, two_b_23,
    two_b_24, two_b_26,

    two_b_80, two_b_81, two_b_82, two_b_83,
    two_b_84, two_b_85, two_b_86, two_b_87,
    two_b_88, two_b_89, two_b_8a, two_b_8b,
    two_b_8c, two_b_8d, two_b_8e, two_b_8f,
    two_b_90, two_b_91, two_b_92, two_b_93,
    two_b_94, two_b_95, two_b_96, two_b_97,
    two_b_98, two_b_99, two_b_9a, two_b_9b,
    two_b_9c, two_b_9d, two_b_9e, two_b_9f,
    two_b_a0, two_b_a1,           two_b_a3,
    two_b_a4, two_b_a5,
    two_b_a8, two_b_a9,           two_b_ab,
    two_b_ac, two_b_ad,           two_b_af,
                        two_b_b2, two_b_b3,                         /*CH0034*/
    two_b_b4, two_b_b5, two_b_b6, two_b_b7,
                                  two_b_bb,                         /*CH0034*/
    two_b_bc, two_b_bd, two_b_be, two_b_bf, 
    extended2, ILL_16_BIT
} byte_2_index_values;

#define group_1a 0x80
#define group_1b 0x81
#define group_1c 0x83
#define group_2a 0xc0
#define group_2b 0xc1
#define group_2c 0xd0
#define group_2d 0xd1
#define group_2e 0xd2
#define group_2f 0xd3
#define group_3a 0xf6
#define group_3b 0xf7
#define group_4 0xfe
#define group_5 0xff
#define group_6 0x00
#define group_7 0x01
#define group_8 0xba

typedef enum {
    grp_1a_0, grp_1a_1, grp_1a_2, grp_1a_3,
    grp_1a_4, grp_1a_5, grp_1a_6, grp_1a_7,
    grp_1b_0, grp_1b_1, grp_1b_2, grp_1b_3,
    grp_1b_4, grp_1b_5, grp_1b_6, grp_1b_7,
    grp_1c_0, grp_1c_1, grp_1c_2, grp_1c_3,
    grp_1c_4, grp_1c_5, grp_1c_6, grp_1c_7,
    grp_2a_0, grp_2a_1, grp_2a_2, grp_2a_3,
    grp_2a_4, grp_2a_5,           grp_2a_7,
    grp_2b_0, grp_2b_1, grp_2b_2, grp_2b_3,
    grp_2b_4, grp_2b_5,           grp_2b_7,
    grp_2c_0, grp_2c_1, grp_2c_2, grp_2c_3,
    grp_2c_4, grp_2c_5,           grp_2c_7,
    grp_2d_0, grp_2d_1, grp_2d_2, grp_2d_3,
    grp_2d_4, grp_2d_5,           grp_2d_7,
    grp_2e_0, grp_2e_1, grp_2e_2, grp_2e_3,
    grp_2e_4, grp_2e_5,           grp_2e_7,
    grp_2f_0, grp_2f_1, grp_2f_2, grp_2f_3,
    grp_2f_4, grp_2f_5,           grp_2f_7,
    grp_3a_0,           grp_3a_2, grp_3a_3,
    grp_3a_4, grp_3a_5, grp_3a_6, grp_3a_7,
    grp_3b_0,           grp_3b_2, grp_3b_3,
    grp_3b_4, grp_3b_5, grp_3b_6, grp_3b_7,
    grp_4_0,  grp_4_1,

    grp_5_0,  grp_5_1,  grp_5_2,  grp_5_3,
    grp_5_4,  grp_5_5,  grp_5_6,  
    ILL_11_BIT
} modRM_extended_11_bit_indexes;

typedef enum {
    extended_1a, extended_1b, extended_1c,
    extended_2a, extended_2b, extended_2c,
    extended_2d, extended_2e, extended_2f,
    extended_3a, extended_3b, extended_4, extended_5                /*CH0034*/
} group_names;

typedef enum {                                                      /*CH0034*/
    extended_6, extended_7, extended_8                              /*CH0034*/
} group_names2;                                                     /*CH0034*/

typedef enum {
    grp_6_0, grp_6_1, grp_6_2, grp_6_3,
    grp_6_4, grp_6_5,
    grp_7_0, grp_7_1, grp_7_2, grp_7_3,
    grp_7_4,          grp_7_6, grp_7_7,

    grp_8_4, grp_8_5, grp_8_6, grp_8_7,
    ILL_19_BIT
} ext_19_bit_indexes;

typedef enum {
    d8_0a,       d8_0b,       d8_1a,       d8_1b,
    d8_2a,       d8_2b,       d8_3a,       d8_3b,                   /*CH0034*/
    d8_4a,       d8_4b,       d8_5a,       d8_5b,                   /*CH0034*/
    d8_6a,       d8_6b,       d8_7a,       d8_7b,                   /*CH0034*/
    d9_0a,       d9_0b,       d9_1,        d9_2a,                   /*CH0034*/
    d9_2b,       d9_3,        d9_4a,       d9_4b0,                  /*CH0034*/
    d9_4b1,      d9_4b4,      d9_4b5,      d9_5a,                   /*CH0034*/
    d9_5b0,      d9_5b1,      d9_5b2,      d9_5b3,                  /*CH0034*/
    d9_5b4,      d9_5b5,      d9_5b6,      d9_6a,                   /*CH0034*/
    d9_6b0,      d9_6b1,      d9_6b2,      d9_6b3,                  /*CH0034*/
    d9_6b4,      d9_6b5,      d9_6b6,      d9_6b7,                  /*CH0034*/
    d9_7a,       d9_7b0,      d9_7b1,      d9_7b2,                  /*CH0034*/
    d9_7b3,      d9_7b4,      d9_7b5,      d9_7b6,                  /*CH0034*/
    d9_7b7,      da_0,        da_1,        da_2,                    /*CH0034*/
    da_3,        da_4,        da_5a,       da_5b,                   /*CH0034*/
    da_6,        da_7,        db_0,        db_2,                    /*CH0034*/
    db_3,        db_4b0,      db_4b1,      db_4b2,                  /*CH0034*/
    db_4b3,      db_4b4,      db_5,        db_7,                    /*CH0034*/
    dc_0a,       dc_0b,       dc_1a,       dc_1b,                   /*CH0034*/
    dc_2,        dc_3,        dc_4a,       dc_4b,                   /*CH0034*/
    dc_5a,       dc_5b,       dc_6a,       dc_6b,                   /*CH0034*/
    dc_7a,       dc_7b,       dd_0a,       dd_0b,                   /*CH0034*/
    dd_2a,       dd_2b,       dd_3a,       dd_3b,                   /*CH0034*/
    dd_4a,       dd_4b,       dd_5,        dd_6,                    /*CH0034*/
    dd_7,        de_0a,       de_0b,       de_1a,                   /*CH0034*/
    de_1b,       de_2,        de_3a,       de_3b,                   /*CH0034*/
    de_4a,       de_4b,       de_5a,       de_5b,                   /*CH0034*/
    de_6a,       de_6b,       de_7a,       de_7b,                   /*CH0034*/
    df_0,        df_2,        df_3,        df_4a,                   /*CH0034*/
    df_4b,       df_5,        df_6,        df_7,                    /*CH0034*/

    d8_0,        d8_1,        d8_2,        d8_3,                    /*CH0034*/
    d8_4,        d8_5,        d8_6,        d8_7,                    /*CH0034*/
    d9_0,        d9_2,        d9_4,        d9_5,                    /*CH0034*/
    d9_6,        d9_7,        da_5,        db_4,                    /*CH0034*/
    dc_0,        dc_1,        dc_4,        dc_5,                    /*CH0034*/
    dc_6,        dc_7,        dd_0,        dd_2,                    /*CH0034*/
    dd_3,        dd_4,        de_0,        de_1,                    /*CH0034*/
    de_3,        de_4,        de_5,        de_6,                    /*CH0034*/
    de_7,        df_4,                                              /*CH0034*/

    d9_4b,       d9_5b,       d9_6b,       d9_7b,
    db_4b,

    ILL_387_INST
} i387_indexes;

extern char *i386_mnemonics[] ;
extern Inst386 i386_1_byte_instructions[256] ;
extern byte_1_index_values i386_byte_1_indexes[] ;
extern modRM_extended_11_bit_indexes i386_11_bit_indexes[][8] ;
extern Inst386 i386_11_bit_instructions[] ;
extern Inst386 i386_2_byte_instructions[] ;
extern byte_2_index_values i386_byte_2_indexes[] ;
extern ext_19_bit_indexes i386_19_bit_indexes[][8] ;
extern Inst386 i386_19_bit_instructions[] ;
extern i387_indexes i387_reg_indexes[][8] ;
extern i387_indexes i387_mod_indexes[][2] ;
extern i387_indexes i387_R_M_indexes[][8] ;
extern Inst387 i387_instructions[] ;
#endif
