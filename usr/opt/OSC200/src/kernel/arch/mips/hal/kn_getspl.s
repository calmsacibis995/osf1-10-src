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
 * @(#)$RCSfile: kn_getspl.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:15:45 $
 */

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>

/*
 * ifdef-ing instructions:
 *	(1) add ifdef/endif DSxxxx around the per-platform routine;
 *	    logically OR multiple DSxxxx's if 1 routine serves
 *	    more than 1 platform.
 *	(2) add ifdef/endif HAL_LIBRARY within the ifdef DSxxxx
 *	    so that the per-platform function name is used for the HAL
 *	    library (used by cpu-switch to the function), and the 
 *	    "getspl" name is used for the per-platform library.
 */

#if defined(DS3100) || defined(DS5000) || defined(DSPERSONAL_DECSTATION) || \
    defined(DS5000_300) || defined(DS5100) || defined(DS5500)
#ifdef HAL_LIBRARY
LEAF(kn_getspl)
#else /* Building per-platform library */
LEAF(getspl)
#endif	/* HAL_LIBRARY */
	.set noreorder
	nop
	mfc0	v0,C0_SR
	nop
	.set reorder
	j	ra
#ifdef HAL_LIBRARY
	END(kn_getspl)
#else /* Building per-platform library */
	END(getspl)
#endif	/* HAL_LIBRARY */
#endif	/* DS3100 || DS5000 ... */
