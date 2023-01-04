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
 *	@(#)$RCSfile: inline.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:22:46 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1984, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/*
 * COMMENTCHAR is the character delimiting comments in the assembler.
 * LABELCHAR is the character that separates labels from instructions.
 * ARGSEPCHAR is the character that separates arguments in instructions.
 */

#ifdef	sun
#define COMMENTCHAR	'|'
#else	/* sun */
#define COMMENTCHAR	'#'
#endif	/* sun */
#define LABELCHAR	':'
#define ARGSEPCHAR	','

/*
 * Expansion parameters:
 *   QUEUESIZE is the number of instructions to be considered for 
 *	integration of argument pushes and pops
 *   MAXLINELEN is the longest expected input line
 *   MAXARGS is the maximum number of arguments in an assembly instruction
 */
#define QUEUESIZE	16
#define MAXLINELEN	1024
#define MAXARGS		10

/*
 * The following global variables are used to manipulate the queue of
 * recently seen instructions.
 *	line - The queue of instructions.
 *	bufhead - Pointer to next availble queue slot. It is not
 *		considered part of te instruction stream until
 *		bufhead is advanced.
 *	buftail - Pointer to last instruction in queue.
 * Note that bufhead == buftail implies that the queue is empty.
 */
int pre_read, bufhead, buftail;
char line[QUEUESIZE][MAXLINELEN];

#define SUCC(qindex) ((qindex) + 1 == QUEUESIZE ? 0 : (qindex) + 1)
#define PRED(qindex) ((qindex) - 1 < 0 ? QUEUESIZE - 1 : (qindex) - 1)

/*
 * Hash table headers should be twice as big as the number of patterns.
 * They must be a power of two.
 */
#define HSHSIZ	128

/*
 * These tables specify the substitutions that are to be done.
 */
struct pats {
	int	args;
	char	*name;
	char	*replace;
	struct	pats *next;
	int	size;
};
struct pats *patshdr[HSHSIZ];
extern struct pats language_ptab[], libc_ptab[], machine_ptab[];
extern struct pats vax_libc_ptab[], vaxsubset_libc_ptab[];
extern struct pats vax_ptab[], vaxsubset_ptab[];

/*
 * This table defines the set of instructions that demark the
 * end of a basic block.
 */
struct inststoptbl {
	char	*name;
	struct	inststoptbl *next;
	int	size;
};
struct inststoptbl *inststoptblhdr[HSHSIZ];
extern struct inststoptbl inststoptable[];

/*
 * Miscellaneous functions.
 */
char *newline(), *copyline(), *doreplaceon();

#ifdef	multimax
/*
 *	Definitions and storage for local label expansion.  labelnum is the 
 *	number for the next generated label (.LInnn).  labelstrings holds
 *	the current translation from labels of the form num$ to real labels.
 */
#define LOC_LABEL_CHAR		'$'
#define LOC_LABEL_BUFLEN	256
#define LOC_LABEL_LIMIT		16

static	labelnum = 1;
char	*labelbufp;
char	labelbuf[LOC_LABEL_BUFLEN];
char	*labelstrings[LOC_LABEL_LIMIT];

#endif	/* multimax */

#ifdef	i386
#define index	strchr
#endif	/* i386 */
