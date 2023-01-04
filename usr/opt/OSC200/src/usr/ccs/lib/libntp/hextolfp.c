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
static char     *sccsid = "@(#)$RCSfile: hextolfp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:47 $";
#endif
/*
 */

/*
 * hextolfp - convert an ascii hex string to an l_fp number
 */
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>

int
hextolfp(str, lfp)
	char *str;
	l_fp *lfp;
{
	register char *cp;
	register char *cpstart;
	register u_int dec_i;
	register u_int dec_f;
	char *ind;
	static char *digits = "0123456789abcdefABCDEF";

	dec_i = dec_f = 0;
	cp = str;

	/*
	 * We understand numbers of the form:
	 *
	 * [spaces]8_hex_digits[.]8_hex_digits[spaces|\n|\0]
	 */
	while (isspace(*cp))
		cp++;
	
	cpstart = cp;
	while (*cp != '\0' && (cp - cpstart) < 8 &&
	    (ind = index(digits, *cp)) != NULL) {
		dec_i = dec_i << 4;	/* multiply by 16 */
		dec_i += ((ind - digits) > 15) ? (ind - digits) - 6
		    : (ind - digits);
		cp++;
	}

	if ((cp - cpstart) < 8 || ind == NULL)
		return 0;
	if (*cp == '.')
		cp++;

	cpstart = cp;
	while (*cp != '\0' && (cp - cpstart) < 8 &&
	    (ind = index(digits, *cp)) != NULL) {
		dec_f = dec_f << 4;	/* multiply by 16 */
		dec_f += ((ind - digits) > 15) ? (ind - digits) - 6
		    : (ind - digits);
		cp++;
	}

	if ((cp - cpstart) < 8 || ind == NULL)
		return 0;
	
	if (*cp != '\0' && !isspace(*cp))
		return 0;

	lfp->l_ui = dec_i;
	lfp->l_uf = dec_f;
	return 1;
}
