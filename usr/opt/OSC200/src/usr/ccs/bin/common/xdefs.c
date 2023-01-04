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
static char	*sccsid = "@(#)$RCSfile: xdefs.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/11/22 21:23:22 $";
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
 * COMPONENT_NAME: (CMDPROG) xdefs.c
 *
 * FUNCTIONS: getmem, scnames, reallocmem
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

/* AIWS C compiler */

# include "mfile1.h"

/*	communication between lexical routines	*/

char	ftitle[100] = "\"\"";   	/* title of the file */
int	lineno;		/* line number of the input file */

#if	defined (LINT) || defined (CFLOW)
char	*pfname;		/* physical filename (stripped) */
char	*ifname;		/* included filename (stripped) */
FILE	*tmplint;		/* temporary output file */
#endif

CONSZ lastcon;  /* the last constant read by the lexical analyzer */
#ifdef HOSTIEEE
double dcon;   /* the last double read by the lexical analyzer */
#else
FP_DOUBLE dcon;   /* the last double read by the lexical analyzer */
#endif

/*	symbol table maintainence */

int ndiments = DIMTABSZ;
int nstabents = SYMTSZ;

struct symtab *stab;  /* (lec) changed to a dynamically allocated array */

#ifdef XCOFF
/*      storage for saving strings and labels to spill at end of functions */
int *saved_str;
int max_chars = 2000;
int saved_chars = 0;
int *saved_lab;
int max_strings = 100;
int saved_strings = 0;
#endif

int	curftn = -1;  /* "current" function */
int	ftnno;  /* "current" function number */

int	curLevel;	/* current parsing level */

int	curclass,	/* current storage class */
	instruct,	/* "in structure" flag */
	stwart,		/* for accessing names which are structure members or names */
	funcstyle,	/* style of current function: old or new */
	funcConflict,	/* true if old style func defn occurs in
			 * presence of prototype.
			 */
 	paramFlg,	/* detects misuse of void/ellipsis in parameter list */
	blevel,		/* block level: 0 for extern, 1 for ftn args,
			 * >=2 inside function
			 */
	tempBlevel,	/* hold value of blevel while in
			 * prototype parameter list
			 */
	curdim,		/* current offset into the dimension table */
	volEmitted;	/* ".copt volatile" emitted for function */

int	*dimtab = 0;

struct dnode dimrec[BNEST];
struct dnode *dimptr = &dimrec[-1];
int	paramlevel = 0;	/* current nesting level for parameters */
int     *paramstk;      /* used in the definition of function parameters */
int	*protostk;	/* mark position of new prototypes in paramstk */
int     paramno;        /* the number of parameters */
int	paramchk;	/* 1 if parameters should be checked for usage */
#ifdef LINT
int	vflag=1;	/* 0 if user turns off parameters usage checking */
#endif
#if defined(LINT) || defined(CFLOW) || defined(CXREF)
int	func_ptr=0;
#endif
int	autooff,	/* the next unused automatic offset */
	argoff,         /* the next unused argument offset */
	strucoff;	/*  the next structure offset position */
int	regvar;		/* the next free register for register variables */
int     fpregvar;       /* the largest fp registers used in a function */
OFFSZ	inoff;		/* offset of external element being initialized */

struct lnm lnames[LNAMES], *lnp;

struct sw swtab[SWITSZ];  /* table for cases within a switch */
struct sw *swp;  /* pointer to next free entry in swtab */
int swx;  /* index of beginning of cases for current switch */

struct tyinfo labltype;	/* type of goto labels */

int nGlobTyEnts = GLOBTYSZ;
int nLoclTyEnts = LOCLTYSZ;
TPTR globTyTab = TNIL;	/* table for global type nodes */
TPTR loclTyTab = TNIL;	/* table for block type nodes */

/* debugging flag */
int xdebug = 0;

int strflg;  /* if on, strings are to be treated as lists */
int strsize; /* string size, to get the array type correct */

int reached;	/* true if statement can be reached... */

int devdebug[MXDBGFLG+1];
int warnlevel[MXDBGFLG+1];


/* LINT flags. */
#ifdef LINT
int alphalevel[MXDBGFLG+1];
#endif
int lintargu = 0;	/* for the ARGSUSED directive */
int lintvarg = -1;	/* for the VARARGS directive */
int lintlib = 0;	/* for the LINTLIBRARY directive */
int lintnrch = 0;	/* for the NOTREACHED directive */
int lintused = 0;	/* for the NOTUSED directive */
int lintdefd = 0;	/* for the NOTDEFINED directive */
int lintrsvd = 0;	/* for the LINTSTDLIB directive */

int ininit = 0;	/* true if initializing a variable */
int idname;	/* tunnel to buildtree for name id's */

NODE *node;

int brklab;
int contlab;
int flostat;
int retlab = NOLAB;
int retstat;
int loop_level;
unsigned int foldMask = 0x0;
int foldadd = 0;
unsigned int Initializer = 0;

/* save array for break, continue labels, and flostat */

int asavbc[BCSZ];
int *psavbc = asavbc ;

# ifndef BUG1
static char *
ccnames[] = { /* names of storage classes */
	"SNULL",
	"AUTO",
	"AUTOREG",
	"PARAM",
	"PARAMREG",
	"PARAMFAKE",
	"REGISTER",
	"LABEL",
	"ULABEL",
	"STATIC",
	"USTATIC",
	"FORTRAN",
	"UFORTRAN",
	"EXTDEF",
	"EXTENT",
	"EXTERN",
	"STNAME",
	"UNAME",
	"ENAME",
	"MOS",
	"MOU",
	"MOE",
	"TYPEDEF",
	"TCSYM",
	};

#include <stdlib.h> /* for malloc */
#include <stdio.h>
/* -------------------- scnames -------------------- */

char *
scnames( c )
	register c;
{
	/* return the name for storage class c */
	static char buf[12];
	if( c&FIELD ){
		sprintf( buf, TOOLSTR(M_MSG_303, "FIELD[%d]"), c&FLDSIZ );
		return( buf );
		}
	return( ccnames[c] );
	}
# endif

/* -------------------- getmem -------------------- */

/* do all mallocing through these functions so we can do error checking */
void *
getmem( int sizetoget )
{
	register void *retval;

	if( !(retval = malloc( sizetoget )) )
		cerror(TOOLSTR(M_MSG_200, "no memory to malloc\n"));
	return(retval);
}

/* -------------------- reallocmem -------------------- */

void *reallocmem( void *ptr, int sizetoget )
{
	if( !(ptr = realloc( ptr, sizetoget )) )
		cerror(TOOLSTR(M_MSG_200, "no memory to malloc\n"));
	return ptr;
}
