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
static char *rcsid = "@(#)$RCSfile: cktodr.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:29:58 $";
#endif

#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <hal/clock.h>

extern int cpu;
extern struct cpusw *cpup;

/*
 * checktodr -- check for clock rollover and reset if necessary
 */
checktodr()
{
	timeout(checktodr, 0, SECDAY*hz);	/* Check again tomorrow */
	if ((read_todclk()/cpup->rdclk_divider) > SECYR+(2*SECDAY))
	    resettodr();
}
	

