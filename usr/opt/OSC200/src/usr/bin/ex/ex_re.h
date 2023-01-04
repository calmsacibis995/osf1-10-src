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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: ex_re.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 20:15:26 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDEDIT) ex_re.h
 *
 * FUNCTION: none
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.4 com/cmd/edit/vi/ex_re.h, cmdedit, bos320 6/5/91 23:31:45
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

/*
 * Regular expression definitions.
 * The regular expressions in ex are similar to those in ed,
 * with the addition of the word boundaries from Toronto ed
 * and allowing character classes to have [a-b] as in the shell.
 * The numbers for the nodes below are spaced further apart then
 * necessary because I at one time partially put in + and | (one or
 * more and alternation.)
 */
struct	regexp {
	wchar_t Expbuf[ESIZE + 2];
	short	Circfl;
	short	Nbra;
};

/*
 * There are three regular expressions here, the previous (in re),
 * the previous substitute (in subre) and the previous scanning (in scanre).
 * It would be possible to get rid of "re" by making it a stack parameter
 * to the appropriate routines.
 */
var struct	regexp re;		/* Last re */
var struct	regexp scanre;		/* Last scanning re */
var struct	regexp subre;		/* Last substitute re */

/*
 * Defining circfl and expbuf like this saves us from having to change
 * old code in the ex_re.c stuff.
 */
#define	expbuf	re.Expbuf
#define	circfl	re.Circfl
#define	nbra	re.Nbra
#define savere(a)	((a) = re)
#define resre(a)	(re = (a))

/*
 * Definitions for substitute
 */
var wchar_t	*braslist[NBRA];	/* Starts of \(\)'ed text in lhs */
var wchar_t *braelist[NBRA];	/* Ends... */
var wchar_t rhsbuf[RHSSIZE];	/* Rhs of last substitute */

/*
 * Definitions of codes for the compiled re's.
 * The re algorithm is described in a paper
 * by K. Thompson in the CACM about 10 years ago
 * and is the same as in ed.
 */
#define	STAR	1

#define	CBRA	1
#define	CDOT	4
#define	CCL	8
#define	NCCL	12
#define	CDOL	16
#define	CEOFC	17
#define	CKET	18
#define	CCHR	20
#define	CBRC	24
#define	CLET	25

/*
 * Definitions of things, other than simple characters, that can appear in a
 * character class.  The internal representation is a backslash followed by
 * one of these codes.  Most of these represent ctype classes.
 */
#define CL_BADCLASS	0x00	/* Not stored; used only as an error return */
#define CL_BACKSLASH	0x01	/* Real backslash */
#define CL_RANGE	0x02	/* a-z becomes '\\' CL_RANGE 'a' 'z' */
#define CL_GOODCLASS	0x03
