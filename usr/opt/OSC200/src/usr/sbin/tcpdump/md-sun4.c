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
static char *rcsid = "@(#)$RCSfile: md-sun4.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:37:05 $";
#endif
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Based on:
 * static char rcsid[] = "md-sun4.c,v 1.2 90/09/21 02:20:17 mccanne Exp $ (LBL)";
 */

/* The sun includes are arranged too stupidly to help us */

#define ARCH_MASK	0xf0000000
#define MACH_MASK	0x0f000000
#define ARCH_MACH_MASK	(ARCH_MASK + MACH_MASK)

#define ARCH_SUN4	0x20000000
#define ARCH_SUN4C	0x50000000
#define MACH_60		0x01000000	/* Sparcstation 1 */

#define MACH_4_330	(ARCH_SUN4 + 0x03000000)
#define MACH_4_460	(ARCH_SUN4 + 0x04000000)

#define MACH_260	0x01000000
#define MACH_110	0x02000000
#define MACH_330	0x03000000
#define MACH_460	0x04000000

int
clock_sigfigs()
{
	int good = 0;
	long id = gethostid();

	/*
	 * Apparently, all sun4c's, 4/300's, and 4/400's have the
	 * Mostek 48T02 clock chip.
	 */
	if ((id & ARCH_MASK) == ARCH_SUN4C)
		++good;
	else
		switch (id & ARCH_MACH_MASK) {

		case MACH_4_330:
		case MACH_4_460:
			++good;
			break;
		}
	/* On machines with "good" clocks, timestamps to microseconds.
	   Default is hundreths of a second. */

	if (good)
		return 6;
	else 
		return 2;
}
