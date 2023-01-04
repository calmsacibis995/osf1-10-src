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
 *	@(#)$RCSfile: defs.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:15 $
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
 * derived from defs.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 * adb - MIPS version; common definitions
 */

/*	defs.h	4.3	82/12/19	*/

#include <hal/kdb/kdbdefine.h>

#include <machine/pmap.h>
#include <sys/param.h>
#include <sys/user.h>

#include <sysV/filehdr.h>
#include <sysV/aouthdr.h>
#include <sysV/scnhdr.h>

#include <vm/vm_map.h>

/* we simulate nlist internally */
struct nlist {
    char *n_name;
    unsigned long n_value;
    short n_type;		/* 0 if not there, 1 if found */
    short reserved;
};


/* general defs */
#undef	TRUE
#undef	FALSE
#define TRUE     (-1)
#define FALSE   0
#define SP      ' '
#define TB      '\t'
#define NL      '\n'

/*	ctype.h	4.2	85/09/04	*/

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C	040
#define _X	0100
#define	_B	0200

extern	char	_ctype_[];

#define	isalpha(c)	((_ctype_+1)[c]&(_U|_L))
#define	isupper(c)	((_ctype_+1)[c]&_U)
#define	islower(c)	((_ctype_+1)[c]&_L)
#define	isdigit(c)	((_ctype_+1)[c]&_N)
#define	isxdigit(c)	((_ctype_+1)[c]&(_N|_X))
#define	isspace(c)	((_ctype_+1)[c]&_S)
#define ispunct(c)	((_ctype_+1)[c]&_P)
#define isalnum(c)	((_ctype_+1)[c]&(_U|_L|_N))
#define isprint(c)	((_ctype_+1)[c]&(_P|_U|_L|_N|_B))
#define isgraph(c)	((_ctype_+1)[c]&(_P|_U|_L|_N))
#define iscntrl(c)	((_ctype_+1)[c]&_C)
#define isascii(c)	((unsigned)(c)<=0177)
#define toupper(c)	((c)-'a'+'A')
#define tolower(c)	((c)-'A'+'a')
#define toascii(c)	((c)&0177)

#include <hal/kdb/mode.h>
#include <hal/kdb/head.h>

/* access modes */
#define RD	0
#define WT	1

#define NSP	0
#define ISP	1
#define DSP	2
#define STAR	4
#define STARCOM 0200

/*
 * Symbol types, used internally in calls to findsym routine.
 * Basically we get NSYM==0 for `=' command, ISYM==DSYM otherwise.
 */
#define NSYM	0
#define DSYM	1		/* Data space symbol */
#define ISYM	DSYM		/* Instruction space symbol == DSYM */
#define PSYM    3               /* Restricted search: procedures only */

#define BKPTSET		0x1
#define BKPTEXEC	0x2
#define BKPTSSTEP	0x4

#define CONTIN	1
#define EXIT	2
#define SINGLE	3

/* the quantities involving ctob() are located in the kernel stack. */
/* the others are in the pcb. */
#define R1      (ctob(UPAGES)-37*sizeof(int))
#define R2      (ctob(UPAGES)-36*sizeof(int))
#define R3      (ctob(UPAGES)-35*sizeof(int))
#define R4      (ctob(UPAGES)-34*sizeof(int))
#define R5      (ctob(UPAGES)-33*sizeof(int))
#define R6      (ctob(UPAGES)-32*sizeof(int))
#define R7      (ctob(UPAGES)-31*sizeof(int))
#define R8      (ctob(UPAGES)-30*sizeof(int))
#define R9      (ctob(UPAGES)-29*sizeof(int))
#define R10     (ctob(UPAGES)-28*sizeof(int))
#define R11     (ctob(UPAGES)-27*sizeof(int))
#define R12     (ctob(UPAGES)-26*sizeof(int))
#define R13     (ctob(UPAGES)-25*sizeof(int))
#define R14     (ctob(UPAGES)-24*sizeof(int))
#define R15     (ctob(UPAGES)-23*sizeof(int))
#define R16     (ctob(UPAGES)-22*sizeof(int))
#define R17     (ctob(UPAGES)-21*sizeof(int))
#define R18     (ctob(UPAGES)-20*sizeof(int))
#define R19     (ctob(UPAGES)-19*sizeof(int))
#define R20     (ctob(UPAGES)-18*sizeof(int))
#define R21     (ctob(UPAGES)-17*sizeof(int))
#define R22     (ctob(UPAGES)-16*sizeof(int))
#define R23     (ctob(UPAGES)-15*sizeof(int))
#define R24     (ctob(UPAGES)-14*sizeof(int))
#define R25     (ctob(UPAGES)-13*sizeof(int))
#define R26     (ctob(UPAGES)-12*sizeof(int))
#define R27     (ctob(UPAGES)-11*sizeof(int))
#define R28     (ctob(UPAGES)-10*sizeof(int))
#define R29     (ctob(UPAGES)-9*sizeof(int))
#define R30     (ctob(UPAGES)-8*sizeof(int))
#define R31     (ctob(UPAGES)-7*sizeof(int))

#define R_SR    (ctob(UPAGES)-6*sizeof(int))
#define R_LO    (ctob(UPAGES)-5*sizeof(int))
#define R_HI    (ctob(UPAGES)-4*sizeof(int))
#define R_BADV  (ctob(UPAGES)-3*sizeof(int))
#define R_CS    (ctob(UPAGES)-2*sizeof(int))
#define R_PC    (ctob(UPAGES)-1*sizeof(int))


/* floating point regs */
#define FP_DISP (sizeof(int)*(32+8+5))
#define FR0     (FP_DISP + 0*sizeof(int))
#define FR1     (FP_DISP + 1*sizeof(int))
#define FR2     (FP_DISP + 2*sizeof(int))
#define FR3     (FP_DISP + 3*sizeof(int))
#define FR4     (FP_DISP + 4*sizeof(int))
#define FR5     (FP_DISP + 5*sizeof(int))
#define FR6     (FP_DISP + 6*sizeof(int))
#define FR7     (FP_DISP + 7*sizeof(int))
#define FR8     (FP_DISP + 8*sizeof(int))
#define FR9     (FP_DISP + 9*sizeof(int))
#define FR10    (FP_DISP + 10*sizeof(int))
#define FR11    (FP_DISP + 11*sizeof(int))
#define FR12    (FP_DISP + 12*sizeof(int))
#define FR13    (FP_DISP + 13*sizeof(int))
#define FR14    (FP_DISP + 14*sizeof(int))
#define FR15    (FP_DISP + 15*sizeof(int))
#define FR16    (FP_DISP + 16*sizeof(int))
#define FR17    (FP_DISP + 17*sizeof(int))
#define FR18    (FP_DISP + 18*sizeof(int))
#define FR19    (FP_DISP + 19*sizeof(int))
#define FR20    (FP_DISP + 20*sizeof(int))
#define FR21    (FP_DISP + 21*sizeof(int))
#define FR22    (FP_DISP + 22*sizeof(int))
#define FR23    (FP_DISP + 23*sizeof(int))
#define FR24    (FP_DISP + 24*sizeof(int))
#define FR25    (FP_DISP + 25*sizeof(int))
#define FR26    (FP_DISP + 26*sizeof(int))
#define FR27    (FP_DISP + 27*sizeof(int))
#define FR28    (FP_DISP + 28*sizeof(int))
#define FR29    (FP_DISP + 29*sizeof(int))
#define FR30    (FP_DISP + 30*sizeof(int))
#define FR31    (FP_DISP + 31*sizeof(int))

#define MAXOFF	0x1000
#define MAXPOS	80
#define MAXLIN	128
#define EOF	0
#define EOR	'\n'
#define QUOTE	0200
#define EVEN	-2

/* long to ints and back (puns) */
union {
	short	I[2];
	long	L;
}
itolws;

#define leng(a)		itol(0,a)
#define shorten(a)	((short)(a))
#define itol(a,b)	(itolws.I[0]=(b), itolws.I[1]=(a), itolws.L)

/* result type declarations */
long		inkdot();
unsigned	get();
unsigned	chkget();
string_t	exform();
long		round();
bkpt_t		scanbkpt(), kdbbkpt_alloc();

struct	pcb	pcb;

struct	pcb	*curpcb;	/* pcb for selected process */
vm_map_t	curmap;		/* vm map for selected process */
int		curpid;		/* process id when entering debugger */
