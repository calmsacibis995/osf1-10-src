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
 *	@(#)$RCSfile: lint2.h,v $ $Revision: 4.2.2.4 $ (DEC) $Date: 1993/01/15 17:53:59 $
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
 * lint2.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	Multiple include protection.
 *
 */

/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef	_LINT2_H_
#define	_LINT2_H_

/* Basic symbol table entry. */
typedef struct smtbl {
	char	*sname;		/* symbol name */
	TPTR	type;		/* type information */
	struct {
		char	*pfname;/* physical filename */
		char	*ifname;/* included filename */
		short	line;	/* line number occurance */
	} rd[2];
	short	usage;		/* usage flags */
	short	nmbrs;		/* number of member symbols */
	struct mbtbl *mbrs;	/* member symbols */
} SMTAB;

#define RUSE	0		/* reference usage */
#define DUSE	1		/* definition usage */
#define CRUSE	2		/* current reference usage */
#define CDUSE	3		/* current definition usage */
#define rpf	rd[RUSE].pfname
#define rif	rd[RUSE].ifname
#define rl	rd[RUSE].line
#define dpf	rd[DUSE].pfname
#define dif	rd[DUSE].ifname
#define dl	rd[DUSE].line

/* Member symbol table extension. */
typedef struct mbtbl {
	char	*mname;		/* member name */
	TPTR	type;		/* type information */
	char	*tagname;	/* struct tag name */
	struct mbtbl *next;	/* next member */
} MBTAB;

/* Allocation constants. */
#define MAXHASH	20		/* number of hash buckets */
#define HASHBLK	1013		/* numbers of symbols per hash bucket */
#define NAMEBLK	1024		/* string table allocation size */
#define MBRBLK	128		/* member symbol table allocation size */

/* Function code directives. */
#define LOOK	1		/* CheckSymbol() */
#define STORE	2
#define CHANGE	3
#define REJECT	4

#define GETNAME	1		/* GetName() */
#define GETMISC	2

/* Global symbols. */
SMTAB theSym;			/* current play symbol */
SMTAB *prevSym;			/* previous symbol */
SMTAB *curSym;			/* current symbol */
char *curPFname;		/* current cpp physical filename */
char *curIFname;		/* current cpp include filename */
char *prevIFname;		/* previous cpp include filename */
short curDLine;			/* current definition line number */
short curRLine;			/* current reference line number */
char sbuf[BUFSIZ];		/* temporary symbol name space */
char ibuf[BUFSIZ];		/* temporary file name space */
char *fname;			/* actual file name opened */
int markerEOF;			/* end of file marker */
int pflag;			/* extreme portability flag */
int debug;			/* debug mode status flag */
int fdebug;			/* debug mode status flag - reader*/
#define Xdollar		"Xdollar"	/* for Xdollard directive*/
int lintXdlr;    		/* for the XDOLLAR  directive 
				 * Not yet used, but set for future */

/* Function declarations. */
SMTAB *FindSymbol();
char *StoreSName();
char *StoreMName();
SMTAB *LookupSymbol();
MBTAB *MBMalloc();

char *GetName();
TPTR InType();

#endif	/* _LINT2_H_ */
