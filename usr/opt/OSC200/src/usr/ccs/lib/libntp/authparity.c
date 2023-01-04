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
static char     *sccsid = "@(#)$RCSfile: authparity.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:37:27 $";
#endif
/*
 */

/*
 * auth_parity - set parity on a key/check for odd parity
 */
#include <sys/types.h>

int
auth_parity(key)
	u_int *key;
{
	u_int mask;
	int parity_err;
	int bitcount;
	int half;
	int byte;
	int i;

	/*
	 * Go through counting bits in each byte.  Check to see if
	 * each parity bit was set correctly.  If not, note the error
	 * and set it right.
	 */
	parity_err = 0;
	for (half = 0; half < 2; half++) {		/* two halves of key */
		mask = 0x80000000;
		for (byte = 0; byte < 4; byte++) {	/* 4 bytes per half */
			bitcount = 0;
			for (i = 0; i < 7; i++) {	/* 7 data bits / byte */
				if (key[half] & mask)
					bitcount++;
				mask >>= 1;
			}

			/*
			 * If bitcount is even, parity must be set.  If
			 * bitcount is odd, parity must be clear.
			 */
			if ((bitcount & 0x1) == 0) {
				if (!(key[half] & mask)) {
					parity_err++;
					key[half] |= mask;
				}
			} else {
				if (key[half] & mask) {
					parity_err++;
					key[half] &= ~mask;
				}
			}
			mask >>= 1;
		}
	}

	/*
	 * Return the result of the parity check.
	 */
	return (parity_err == 0);
}


