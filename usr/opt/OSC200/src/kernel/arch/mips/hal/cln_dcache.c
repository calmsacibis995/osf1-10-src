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
static char *rcsid = "@(#)$RCSfile: cln_dcache.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:33:02 $";
#endif

#include <hal/cpuconf.h>

extern	struct	cpusw	*cpup;      /* pointer to cpusw entry */

/*
 * use cpu switch for generic, else call system specific
 * clean_dcache for target kernel.
 */

/*
 * 	Modified clean_dcache to call wbflush rather
 * 	than (*(cpup->wbflush))(). The switch reference does not 
 *	work for target/custom kernels because the switch entry 
 *	is not defined. A bit slower for generic kernels, but
 *	high performance generic is an oxymoron.
 */

clean_dcache(addr,len)
{
/* A bug in the R3000 that causes the cache isolate to fail if the 
 * write buffers are full is worked around via wbflush.
 */
        int s;
        s = splhigh();
	wbflush();
#ifdef HAL_LIBRARY
        (*(cpup->clean_dcache))(addr,len);
#else
#if defined(DS3100) || defined(DS5100) || defined(DS5000) || \
    defined(DS5000_100) || defined(DS5000_300) || defined(DS5500) || \
    defined(DSPERSONAL_DECSTATION)
	kn_clean_dcache(addr,len);
#endif	/* DS3100 || DS5100 ... */
#endif	/* HAL_LIBRARY */
        splx(s);
}

