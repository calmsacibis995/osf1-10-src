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
static char     *sccsid = "@(#)$RCSfile: atolfp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:36:41 $";
#endif
/*
 */

/*
 * atolfp - convert an ascii string to an l_fp number
 */
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>

/*
 * Powers of 10
 */
static u_int ten_to_the_n[10] = {
		   0,
		  10,
		 100,
		1000,
	       10000,
	      100000,
	     1000000,
	    10000000,
	   100000000,
	  1000000000,
};


int
atolfp(str, lfp)
	char *str;
	l_fp *lfp;
{
	register char *cp;
	register u_int dec_i;
	register u_int dec_f;
	char *ind;
	int ndec;
	int isneg;
	static char *digits = "0123456789";

	isneg = 0;
	dec_i = dec_f = 0;
	ndec = 0;
	cp = str;

	/*
	 * We understand numbers of the form:
	 *
	 * [spaces][-][digits][.][digits][spaces|\n|\0]
	 */
	while (isspace(*cp))
		cp++;
	
	if (*cp == '-') {
		cp++;
		isneg = 1;
	}

	if (*cp != '.' && !isdigit(*cp))
		return 0;

	while (*cp != '\0' && (ind = index(digits, *cp)) != NULL) {
		dec_i = (dec_i << 3) + (dec_i << 1);	/* multiply by 10 */
		dec_i += (ind - digits);
		cp++;
	}

	if (*cp != '\0' && !isspace(*cp)) {
		if (*cp++ != '.')
			return 0;
	
		while (ndec < 9 && *cp != '\0'
		    && (ind = index(digits, *cp)) != NULL) {
			ndec++;
			dec_f = (dec_f << 3) + (dec_f << 1);	/* *10 */
			dec_f += (ind - digits);
			cp++;
		}

		while (isdigit(*cp))
			cp++;
		
		if (*cp != '\0' && !isspace(*cp))
			return 0;
	}

	if (ndec > 0) {
		register u_int tmp;
		register u_int bit;
		register u_int ten_fact;

		ten_fact = ten_to_the_n[ndec];

		tmp = 0;
		bit = 0x80000000;
		while (bit != 0) {
			dec_f <<= 1;
			if (dec_f >= ten_fact) {
				tmp |= bit;
				dec_f -= ten_fact;
			}
			bit >>= 1;
		}
		if ((dec_f << 1) > ten_fact)
			tmp++;
		dec_f = tmp;
	}

	if (isneg)
		M_NEG(dec_i, dec_f);
	
	lfp->l_ui = dec_i;
	lfp->l_uf = dec_f;
	return 1;
}
