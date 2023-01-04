/*
 *	*****************************************************************
 *	*                                                               *
 *	*    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 *	*                                                               *
 *	*   All Rights Reserved.  Unpublished rights  reserved  under   *
 *	*   the copyright laws of the United States.                    *
 *	*                                                               *
 *	*   The software contained on this media  is  proprietary  to   *
 *	*   and  embodies  the  confidential  technology  of  Digital   *
 *	*   Equipment Corporation.  Possession, use,  duplication  or   *
 *	*   dissemination of the software and media is authorized only  *
 *	*   pursuant to a valid written license from Digital Equipment  *
 *	*   Corporation.                                                *
 *	*                                                               *
 *	*   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 *	*   by the U.S. Government is subject to restrictions  as  set  *
 *	*   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 *	*   or  in  FAR 52.227-19, as applicable.                       *
 *	*                                                               *
 *	*****************************************************************
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: config.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/06/11 21:07:22 $ */

/* EUNICE:
 *	This symbol, if defined, indicates that the program is being compiled
 *	under the EUNICE package under VMS.  The program will need to handle
 *	things like files that don't go away the first time you unlink them,
 *	due to version numbering.  It will also need to compensate for lack
 *	of a respectable link() command.
 */
/* VMS:
 *	This symbol, if defined, indicates that the program is running under
 *	VMS.  It is currently only set in conjunction with the EUNICE symbol.
 */
#/*undef	EUNICE		/**/
#/*undef	VMS		/**/

/* CPPSTDIN:
 *	This symbol contains the first part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.	 Typical value of "cc -E" or "/lib/cpp".
 */
/* CPPMINUS:
 *	This symbol contains the second part of the string which will invoke
 *	the C preprocessor on the standard input and produce to standard
 *	output.  This symbol will have the value "-" if CPPSTDIN needs a minus
 *	to specify standard input, otherwise the value is "".
 */
#define CPPSTDIN "/usr/lib/cpp"
#define CPPMINUS ""

/* CHARSPRINTF:
 *	This symbol is defined if this system declares "char *sprintf()" in
 *	stdio.h.  The trend seems to be to declare it as "int sprintf()".  It
 *	is up to the package author to declare sprintf correctly based on the
 *	symbol.
 */
#/*undef	CHARSPRINTF 	/**/

/* FLEXFILENAMES:
 *	This symbol, if defined, indicates that the system supports filenames
 *	longer than 14 characters.
 */
#define	FLEXFILENAMES		/**/

/* index:
 *	This preprocessor symbol is defined, along with rindex, if the system
 *	uses the strchr and strrchr routines instead.
 */
/* rindex:
 *	This preprocessor symbol is defined, along with index, if the system
 *	uses the strchr and strrchr routines instead.
 */
#/*undef	index strchr	/* cultural */
#/*undef	rindex strrchr	/*  differences? */

/* VOIDSIG:
 *	This symbol is defined if this system declares "void (*signal())()" in
 *	signal.h.  The old way was to declare it as "int (*signal())()".  It
 *	is up to the package author to declare things correctly based on the
 *	symbol.
 */
#define	VOIDSIG 	/**/

/* Reg1:
 *	This symbol, along with Reg2, Reg3, etc. is either the word "register"
 *	or null, depending on whether the C compiler pays attention to this
 *	many register declarations.  The intent is that you don't have to
 *	order your register declarations in the order of importance, so you
 *	can freely declare register variables in sub-blocks of code and as
 *	function parameters.  Do not use Reg<n> more than once per routine.
 */

#define Reg1 register		/**/
#define Reg2 register		/**/
#define Reg3 register		/**/
#define Reg4 register		/**/
#define Reg5 register		/**/
#define Reg6 register		/**/
#define Reg7 		/**/
#define Reg8 		/**/
#define Reg9 		/**/
#define Reg10 		/**/
#define Reg11 		/**/
#define Reg12 		/**/
#define Reg13 		/**/
#define Reg14 		/**/
#define Reg15 		/**/
#define Reg16 		/**/

/* VOIDFLAGS:
 *	This symbol indicates how much support of the void type is given by this
 *	compiler.  What various bits mean:
 *
 *	    1 = supports declaration of void
 *	    2 = supports arrays of pointers to functions returning void
 *	    4 = supports comparisons between pointers to void functions and
 *		    addresses of void functions
 *
 *	The package designer should define VOIDUSED to indicate the requirements
 *	of the package.  This can be done either by #defining VOIDUSED before
 *	including config.h, or by defining defvoidused in Myinit.U.  If the
 *	level of void support necessary is not present, defines void to int.
 */
#ifndef VOIDUSED
#define VOIDUSED 7
#endif
#define VOIDFLAGS 7
#if (VOIDFLAGS & VOIDUSED) != VOIDUSED
#define void int		/* is void to be avoided? */
#define M_VOID		/* Xenix strikes again */
#endif

