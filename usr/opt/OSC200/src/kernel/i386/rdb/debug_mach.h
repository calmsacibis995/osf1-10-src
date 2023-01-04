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
 *	@(#)$RCSfile: debug_mach.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:59 $
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
#define lookup db_lookup		/* avoid conflict with filesystem */

#ifdef MACH
/* handle symbol conflicts with mach kernel */
#define prsym db_prsym
#define trap db_trap
#define prhex db_prhex
#define vtop db_vtop
#define gets _gets
#define printf db_printf		/* use our version - not kernel's */
#define sprintf db_sprintf		/* use our version - not kernel's */
#define putchar db_putchar		/* use our version - not kernel's */
#endif /* MACH */

int step_count;			       /* count for step instructions */
#define MAX_BREAK	16	       /* max number of break points */
int debug_state;		       /* the current state of debug */
int debug_option;			/* option flags */
int dispsize;
int db_offset;				/* what's considered "close" */
int cs_size;
int db_16bit;
extern int screen_lines;		/* lines on the screen */

#define NORMAL_STATE	-1	       /* we are just executing commands */
#define DIVERR_STATE	0	       /* divide error */
#define STEP_STATE	1   		/* we've just finished a step */
#define PCALL_STATE	1   		/* we've just finished a pcall */
#define CALL_STATE	2		/* fake interrupt vector */
#define BREAK_STATE	3		/* encountered break point */
#define OVERFLOW_STATE	4		/* overflow error */
#define BOUNDS_STATE	5		/* bounds check failed */
#define BADOP_STATE	6		/* invalid op code */
#define NODEV_STATE	7		/* device (387) not available */
#define DOUBLE_STATE	8		/* double fault */
#define COPROC_STATE	9		/* coprocessor segment overrun */
#define TSS_STATE	10		/* invalid TSS */
#define SEGMENT_STATE	11		/* segment not present */
#define STACK_STATE	12		/* stack segment */
#define GP_STATE	13		/* general protection */
#define PAGE_STATE	14		/* page fault */
#define ERR387_STATE	16		/* coprocessor error */

#define PM_STATE	17		/* post mortem debugging */

#define SCR_IAR		13		/* the IP */
#define MAXSYMLEN 	12		/* symbol length */
/* #define SYM_FORMAT	"%.20s"		/* symbol format */

/*
 * NOTE: last symbol table entry must have a 'symbol[0] == 0'
 * in order to properly terminate the symbol table lookups.
 */
struct symtab {
	unsigned int value;	       /* it's value */
	char symbol[MAXSYMLEN];	       /* the symbol */
} *symtab, *symtab2, deb_internal[], sys_internal[], *next_sym();

#define SYM(name,value) { value, name },	/* symbol table entry */

#define ALLSYMS(s) s=symtab; s && s->symbol[0]; s->symbol[MAXSYMLEN-1] ? (s = next_sym(s)) : ++s
#define ALLSYMS2(s) s=symtab2; s && s->symbol[0]; s->symbol[MAXSYMLEN-1] ? (s = next_sym(s)) : ++s

int *regsave;				/* pointer to saved registers */
#define GETREG(n) ((int *) regsave)[n]	/* pick up register value */

#define BPT	0xcc			/* INT 3 */
#define WATCH_ALIGN 0			/* no alignment required */

/* GET_REG maps the offset (in internal symtab) to register value */
#define GET_REG(n) (* (int *) ((char *) regsave + (n)))
/* SET_REG stores into the register specified in internal offset */
#define SET_REG(n,v) (* (int *) ((char *) regsave + (n))) = (v)

#include <sys/types.h>
#ifdef MACH
#define offsetof(struct_s, member_m) (size_t)(&((struct_s *)0)->member_m)
typedef unsigned long ulong;
#include "i386/rdb/tss386.h"			/* OSF/1 compile */
#else	/* MACH */
#ifdef _KERNEL
#include <sys/tss386.h>			/* version 3 */
#else
#include <sys/i386/tss386.h>		/* gen 1 */
#include <stddef.h>
#endif /* KERNEL */
#endif /* MACH */

#define R(member) offsetof(struct tss386,member)

#define REG(name)  GET_REG(offsetof(struct tss386,name))

typedef unsigned char break_t;		/* use 1 byte for breakpoint */

#define OPTION_FMT "\20\1DEBUG_DEBUG=1\2SHOW_REGS=2\3SCAN_SYM=4\4CS_PRINT=10\6-TRACE_TABLE=20\7MODIFY=40\10EXPAND=80\11UNASM=100\12-CR3=200\13-SYM=400"


#define SYS_CR0		0
#define SYS_CR1		1
#define SYS_CR2		2
#define SYS_CR3		3

#define SYS_DR0		4
#define SYS_DR1		5
#define SYS_DR2		6
#define SYS_DR3		7
#define SYS_DR4		8
#define SYS_DR5		9
#define SYS_DR6		10
#define SYS_DR7		11

#define SYS_GDT		12
#define SYS_GDTL	13

#define SYS_IDT		14
#define SYS_IDTL	15

#define SYS_TASK	16

#define DR_WRITE 01
#define DR_WORD (03<<2)
#define DR_G0	2
#define DR_L0	1
#define DR_GE	(1<<9)

#define DR_B0	1
#define DR_B1	2
#define DR_B2	4
#define DR_B3	8

#define DR_BD	(1<<13)
#define DR_BS	(1<<14)
#define DR_BT	(1<<15)

#define WTYPE_DEFAULT (DR_WRITE|DR_WORD)
#define MAX_LEVEL	25

#define rm_watch() mtdr(7,0)	/* a macro since its so small */

char *db_gets();
#define EDIT_CHANGE	1
#define EDIT_UP		2
#define EDIT_DOWN	4
typedef short word;

