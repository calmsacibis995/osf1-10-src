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
 *	@(#)$RCSfile: linenum.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:05:28 $
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

 */

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
