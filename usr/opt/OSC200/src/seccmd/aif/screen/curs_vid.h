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
 *	@(#)$RCSfile: curs_vid.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:38 $
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

/* Various video attributes */
#ifndef A_CURSES
#ifdef NLS
/*
   We start from the left and attributes bits from left to right to permit
   larger collating sequences.  Add attributes in this fashion if necessary.
*/

#define A_STANDOUT	0x80000000
#define A_UNDERLINE	0x40000000
#define A_REVERSE	0x20000000
#define A_BLINK		0x10000000
#define A_DIM		0x08000000
#define A_BOLD		0x04000000

#define A_INVIS		0x02000000
#define A_PROTECT	0x01000000
#define A_ALTCHARSET	0x00800000

#define A_NORMAL	0x00000000
#define A_ATTRIBUTES	0xff800000
#define A_CHARTEXT	0x007fffff

#else /* NLS */

#define A_STANDOUT	0000200
#define A_UNDERLINE	0000400
#define A_REVERSE	0001000
#define A_BLINK		0002000
#define A_DIM		0004000
#define A_BOLD		0010000

/* The next three are subject to change (perhaps to colors) so don't
					depend on them */
#define A_INVIS		0020000
#define A_PROTECT	0040000
#define A_ALTCHARSET	0100000

#define A_NORMAL	0000000
#define A_ATTRIBUTES	0377600
#define A_CHARTEXT	0000177

#endif /* NLS */
#endif /* A_CURSES */
