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
static char *rcsid = "@(#)$RCSfile: kn220_delay.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/06/03 11:01:05 $";
#endif


/* taken from vax/machdep -- burns 
 * 	(taken from ssc.c -- dutile)
 * delay for n microseconds, limited to somewhat over 2000 microseconds
 * using SSC (CVAX system support chip) programmable timer for lack of ICR.
 */

#include <hal/clock.h>
#include <machine/ssc.h>

extern	struct	ssc_regs	*ssc_ptr;

#if defined(DS5500)
#ifdef	HAL_LIBRARY
uSSCdelay(n)
#else	/* Building per-platform library */
microdelay(n)
#endif	/* HAL_LIBRARY */
int n;
{
	int s;
	register struct ssc_regs *addr;
	if (n==0)
		return(0);
	addr = ssc_ptr;

	s = splclock();
	addr->ssc_tnir0 = -n; 	/* load neg n */
	wbflush();
	addr->ssc_tcr0 = ICCS_RUN+ICCS_TRANS+ICCS_INT+ICCS_ERR+TCR_STP;
	wbflush();
	while ( !(addr->ssc_tcr0 & ICCS_INT))
		;			/* wait */
	splx(s);
	return(0);
}
#endif	/* DS5500 */
