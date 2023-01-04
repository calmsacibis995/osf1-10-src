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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/alpha/linenum.h,v 1.1.2.2 1992/04/01 15:40:28 Ken_Lesniak Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#if __mips || __alpha
   /* linenum.h should not be included for "mips" machines */
#else   /* !__mips */

#ifndef _LINENUM_H
#define _LINENUM_H

/*  There is one line number entry for every 
    "breakpointable" source line in a section.
    Line numbers are grouped on a per function
    basis; the first entry in a function grouping
    will have l_lnno = 0 and in place of physical
    address will be the symbol table index of
    the function name.
*/
struct lineno
{
	union
	{
		long	l_symndx ;	/* sym. table index of function name
						iff l_lnno == 0      */
		long	l_paddr ;	/* (physical) address of line number */
	}		l_addr ;
	unsigned short	l_lnno ;	/* line number */
} ;

#define	LINENO	struct lineno
#define	LINESZ	6	/* sizeof(LINENO) */

#endif  /* _LINENUM_H */
#endif  /* __mips */
