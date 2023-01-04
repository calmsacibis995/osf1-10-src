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
static char	*sccsid = "@(#)$RCSfile: vec_intr.c,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/03/18 15:24:51 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from vec_intr.c	4.1	(ULTRIX)	7/2/90";
 */

/*
 * Revision History:
 *
 * 14-Jan-90	Don Dutile
 *	Merge OSF/1 to v4.2.
 *
 * 15-Jun-90    Mark Parenti
 *      Moved stray() and passive_release() to machdep.c
 *
 * 07-Jun-90	Paul Grist
 *	Added conditionals for vaxbi and vba scb pages and fixed 
 *	vaxbi to CVAXBI, the number of confured vaxbi instead of
 *	NVAXBI the largest# plus one.
 *
 * 06-Feb-90	Robin
 *	added DS5500 support
 *
 * 20-Dec-89    Paul Grist
 *      Added VMEbus support (vba adapters). added line to increase the
 *      size of the vector table by 256 entries for each configured
 *      vba adapter.
 *
 * 02-Feb-89	Kong
 *	Created the file.  This file declares a vector table
 *	for handling interrupts.  The vector table is called
 *	"scb".  The code path is as follow:
 *	On mips machine such as ISIS, CMAX, and MIPSFAIR, read a register
 *	to get the value of the vector.  Use this vector as a byte
 *	offset into the vector table "scb".  Call the address stored
 *	in the "scb".  Unlike in a VAX where the scb is used for all
 *	interrupts and exceptions, here the scb is used only for
 *	device interrupts.  
 *	TODO: perhaps error interrupts should go through the scb as well.
 */
#include "vaxbi.h"
#include "vba.h"

extern stray(),cnxint(),cnrint(),passive_release();
extern int cold;
int (*(scb[128]))() =	/* Declare an array of pointers to int functions */
{
	passive_release,stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		cnrint,		cnxint,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray,
	stray,		stray,		stray,		stray
};

/*
 * After the first page of SCB, reserve another n pages
 * of SCB for the Unibus devices, where n is the number
 * of Unibus adapters.  It is initialised by "unifind"
 * to point to stray, then point to the interrupt handlers
 * as devices are found.
 *
 * Here we initialise 1 element to zero just to make sure
 * UNIvec is allocated in data space and therefore follows
 * scb. Ditto for vax8800bivec.
 */
int (*(UNIvec[2 * 128]))() = {0};
#if	(CVAXBI > 0)
int (*(vax8800bivec[CVAXBI * 512]))() = {0};
#else
int (*(vax8800bivec[1 * 512]))() = {0};
#endif	/* (CVAXBI > 0) */

#if (CVBA > 0)
int (*(VMEvec[CVBA * 256]))() = {0};
#endif /* CVBA */

volatile int cvec = {0};/* Device interrupt vector			*/
volatile int br = {0};	/* Bus request (IPL level on vax)		*/

