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
static char *rcsid = "@(#)$RCSfile: sr_masks.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/07/08 08:46:31 $";
#endif

/*
 * Functions to decode processor status word;
 * Use to be macros in machparam.h; need to be real functions
 * due to (significant) differences between the R2000/3000
 * psw (status reg.) and the R4000 psw.
 */

#include <machine/cpu.h>

usermode(sr)
	unsigned int sr;
{
	return((((sr) & SR_KUP) == SR_KUP));
}

basepri(sr)
	int sr;
{
	extern unsigned int sr_usermask;

	/*
	 * Note: since the 8 bits we look at are the same 
	 *	 between sr_kernelmask & sr_usermask, we
	 *	 don't need to check what type of thread
	 * 	 we are (kernel or user) in order to get
	 * 	 the correct basepri.
	 */
	return((((sr) & SR_IMASK) == (sr_usermask & SR_IMASK)));
}
