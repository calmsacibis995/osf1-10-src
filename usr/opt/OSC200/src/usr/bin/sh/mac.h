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
static char rcsid[] = "@(#)$RCSfile: mac.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:43:13 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.9  com/cmd/sh/sh/mac.h, cmdsh, bos320, 9125320 6/6/91 23:10:43
 */

#define LOBYTE	0377
#define STRIP   0177
#define QUOTE	0200

#define WORDEOF	0
#define NL	'\n'
#define SP	' '
#define LQ	'`'
#define RQ	'\''
#define MINUS	'-'
#define COLON	':'
#define TAB	'\t'
/*
 * The values of FSH0 and FNLS are arbitrary, in theory, but trouble does
 * in fact occur if either one appears in a string.  The current workaround
 * is to use ^? and ^V, which are the default INTR and QUIT characters, so
 * that the user is unlikely to encounter the problem.  The correct fix is
 * probably to use arrays of NLchar instead of "encoded strings"; this would
 * make both of these constants obsolete.
 */
#define FNLS    '\026'
#ifndef _SBCS
#define FSH0	'\35'
#define FSH20	'\36'
#define FSH21	'\37'
#define FNLS    '\026'
#else
#define FSH0    '\177'
#endif

#define blank()		prc(SP)
#define	tab()		prc(TAB)
#define newline()	prc(NL)

