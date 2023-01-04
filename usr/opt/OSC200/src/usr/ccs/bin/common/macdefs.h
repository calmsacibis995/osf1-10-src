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
 *	@(#)$RCSfile: macdefs.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/11/22 21:21:41 $
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
 * COMPONENT_NAME: (CMDPROG) macdefs.h
 *
 * FUNCTIONS: CCTRANS, ENUMSIZE, FIXDEF, FIXSTRUCT, IsRegVar, SETDCON, makecc
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

#ifndef TWOPASS
#define ONEPASS
#endif

# define TRUE 1
# define FALSE 0

extern void *getmem(int sizetoget);
extern void *reallocmem( void *ptr, int sizetoget );
	/* chars are unsigned */
# define makecc(val,i)  lastcon = (lastcon<<8)|val;

# define ARGINIT 0      /* args start at 0(ap)      */
# define AUTOINIT 0     /* autos start at 0(STKPTR) */
# define SZCHAR 8
# define SZINT 32
# define SZFLOAT 32
# define SZDOUBLE 64
# define SZLDOUBLE 64
#ifdef __alpha || __mips64
# define SZLONG 64
# define SZPOINT 64
#else
# define SZPOINT 32
# define SZLONG 32
#endif
# define SZLLONG 64 
# define SZSHORT 16
# define ALCHAR 8
# define ALINT 32
# define ALFLOAT 32
# define ALDOUBLE 64
# define ALLDOUBLE 64
#ifdef __alpha || __mips64
# define ALLONG 64
# define ALPOINT 64
#else
# define ALLONG 32
# define ALPOINT 32
#endif
# define ALLLONG 64
# define ALSHORT 16
# define ALSTRUCT 8
# define ALSTACK 32
# define ALFTN 16	/* see bfcode() */

/*	size in which constants are converted */
/*	should be long if feasable */

# define CONSZ long
# define UCONSZ unsigned long
# define FCONSZ double
# define CONFMT "%ld"

/*	size in which offsets are kept */
/*	should be large enough to cover address space in bits */

# define OFFSZ unsigned

/* 	character set macro */

# define  CCTRANS(x) x

/* register cookie for stack poINTer */

#         define MINRVAR  7     /* 7 free registers! + 1 saved scratch */
#         define MAXRVAR 13
#         define STKREG  22     /* pseudo-registers, xlated by adrput() */
#         define ARGREG  23
#         define NARGREG 24
#         define MAXTEMPS 6     /* killed registers: 0,2,3,4,5,15 */
#ifdef XCOFF
#         define RRMAX    9     /* round robin register pool size */
#else
#         define RRMAX   10     /* round robin register pool size */
#endif
#         define REGSZ 16+6+3   /* normal, float, psuedo-registers  */

# define IsRegVar(x) ((x) >= MINRVAR && (x) <= MAXRVAR )

# define TMPREG   STKREG
# define MINFPREG 16
# define MAXFPREG 21
# define MINFPVAR 18            /* 4 fp register vars */

# define FORCEREG 2		/* usual register for FORCE nodes */
# define FORCEFLOATREG 16	/* register for arith if FORCE */

	/* various standard pieces of code are used */
# define STDPRTREE
# define LABSIZE 10	/* number of chars in label - used to malloc space */
# define LABFMT "L.%d"

/* show stack grows positively */
#undef BACKAUTO
#undef BACKTEMP

/* show no field hardware support on Kimono */
/* or at least not much */
#undef FIELDOPS

/* bytes are numbered from left to right */
#define LTORBYTES

/* we want prtree included */
# define STDPRTREE

# define ENUMSIZE(high,low) INT

# define FIXDEF(p) fixdef(p)
# define FIXSTRUCT(p,q) strend(p)
# define SETDCON(p) 1		/*  setdcon(p) no need for side effects */
# define LAST_P2_LABEL	32767	/* f77 uses 16 bits for labels */

/* This character introduces an assembler comment */

#define ASM_CCHAR	'#'

/* The name of the General Data CSect */
#define DATA_SECT	"..STATIC"
