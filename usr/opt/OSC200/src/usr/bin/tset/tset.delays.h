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
static char rcsid[] = "@(#)$RCSfile: tset.delays.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 19:51:09 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2

/*
 * COMPONENT_NAME: CMDTTY tty control commands
 *
 * FUNCTIONS: header file for tset
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* tset-posix.h	1.1  com/cmd/tty/tset,3.1,9021 2/20/90 18:08:29 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
**  SYSTEM DEPENDENT TERMINAL DELAY TABLES
**
**	Evans Hall VAX
**
**	This file maintains the correspondence between the delays
**	defined in /etc/termcap and the delay algorithms on a
**	particular system.  For each type of delay, the bits used
**	for that delay must be specified (in XXbits) and a table
**	must be defined giving correspondences between delays and
**	algorithms.  Algorithms which are not fixed delays (such
**	as dependent on current column or line number) must be
**	cludged in some way at this time.
*/

/*
**  Carriage Return delays
*/

int	CRbits = CRDLY;
struct delay	CRdelay[] = {
    0,	CR0,
    80,	CR1,
    100, CR2,
    150, CR3,
    -1,
};

/*
**  TaB delays
*/

int	TBbits = TABDLY;
struct delay	TBdelay[] = {
    0,	TAB0,
    11,	TAB1,				/* special M37 delay */
    100, TAB2,
    -1,
};

/*
**  Back Space delays
*/

int	BSbits = BSDLY;
struct delay	BSdelay[] = {
    0,	BS0,
    50, BS1,
    -1,
};

/*
**  Form Feed delays
*/

int	FFbits = FFDLY;
struct delay	FFdelay[] = {
    0,	FF0,
    2000, FF1,
    -1,
};

/*
**  New Line delays
*/

int	NLbits = NLDLY;
struct delay	NLdelay[] = {
    0,	NL0,
    100, NL1,
    -1,
};

/*
**  Verticle tab delays
*/

int	VTbits = VTDLY;
struct delay	VTdelay[] = {
    0,	VT0,
    2000, VT1
    -1,
};
