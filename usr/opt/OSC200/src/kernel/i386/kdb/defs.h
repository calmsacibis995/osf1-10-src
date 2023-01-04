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
 *	@(#)$RCSfile: defs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * Copyright (c) 1986 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*	defs.h	4.3	82/12/19	*/

/*
 * adb - vax string table version; common definitions
 */

#include <i386/kdb/kdbdefine.h>

#include <i386/psl.h>
#include <i386/debug.h>
#include <mach/i386/vm_param.h>
#include <sys/param.h>
#include <sys/user.h>

#ifdef	wheeze
#include <i386/kdb/structs.h>
struct pte {
	int x;
};
#else	wheeze
#include <a.out.h>
#endif	wheeze

#define ADB
#define MULD2

#undef	TRUE
#undef	FALSE
#undef	MAXFILE
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

#include <i386/reg.h>
#include <i386/trap.h>
#include <vm/vm_map.h>

#include <i386/kdb/mac.h>
#include <i386/kdb/mode.h>
#include <i386/kdb/head.h>

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
 * One the VAX this all degenerates since I & D symbols are indistinct.
 * Basically we get NSYM==0 for `=' command, ISYM==DSYM otherwise.
 */
#define NSYM	0
#define DSYM	1		/* Data space symbol */
#define ISYM	DSYM		/* Instruction space symbol == DSYM on VAX */

#define BKPTSET	1
#define BKPTEXEC 2
#define BKPTTMP	4

#define USERPS	PSL
#define USERPC	PC
#define BPT	0xcc
#define TBIT	020
#define FD	0200
#define SETTRC	0
#define RDUSER	2
#define RIUSER	1
#define WDUSER	5
#define WIUSER	4
#define RUREGS	3
#define WUREGS	6
#define CONTIN	7
#define EXIT	8
#define SINGLE	9


#define MAXOFF	0x1000

#define MAXPOS	80
#define MAXLIN	128
#define EOF	0
#define EOR	'\n'
#define QUOTE	0200
#define EVEN	-2

/* long to ints and back (puns) */
union {
	INT	I[2];
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
bkpt_t		scanbkpt();
VOID		fault();

struct	pcb	pcb;
int	kernel;
int	kcore;

struct	pcb	*curpcb;	/* pcb for selected process */
vm_map_t	curmap;		/* vm map for selected process */
int		curpid;		/* process id when entering debugger */
