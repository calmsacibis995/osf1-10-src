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
static char *rcsid = "@(#)$RCSfile: kn_whatspl.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:24:33 $";
#endif

#include <machine/cpu.h>

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

/*
 * Determines what spl we are currently set to.
 * Returns a define const (see cpu.h) for the integer number 
 * corresponding to the interrupt bits set in the Status Register.
 */
#if defined(DS3100) || defined(DS5000) || defined(DSPERSONAL_DECSTATION) || \
    defined(DS5000_300) || defined(DS5100) || defined(DS5500)
#ifdef HAL_LIBRARY
kn_whatspl(sr)
#else /* Building per-platform library */
whatspl(sr)
#endif	/* HAL_LIBRARY */
unsigned sr;
{
        register int    index;          /* index into the splm array */
        register int    imask;          /* mask value looking for */
        register int    found = 0;      /* Gets us out of the loop */
        extern   int    splm[];

        imask = (sr & SR_IMASK);
        index = (SPLMSIZE);
        while ((index >= 0) && !found) {
                index--;
                switch (index) {
                      case 4:   /* not used */
                      case 3:   /* not used */
                        break;
                      default:
                        if ((splm[index] & SR_IMASK) == imask) {
                                found = 1;
                        }
                        break;
                }
                if (found)
                        return(index);
        }
        return (0);
}
#endif	/* DS3100 || DS5000 ... */



/* On 3min, the value returned by getspl is the actual ipllevel */
#ifdef  DS5000_100
#ifdef HAL_LIBRARY
kn02ba_whatspl(ipl)
#else /* Building per-platform library */
whatspl(ipl)
#endif	/* HAL_LIBRARY */
unsigned int ipl;
{
    return (ipl);
}
#endif	/* DS5000_100 */
