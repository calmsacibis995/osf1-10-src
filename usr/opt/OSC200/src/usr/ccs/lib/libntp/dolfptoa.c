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
static char     *sccsid = "@(#)$RCSfile: dolfptoa.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/07/07 16:12:08 $";
#endif
/*
 */

/*
 * dolfptoa - do the grunge work of converting an l_fp number to decimal
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <ntp/ntp_fp.h>
#include "lib_strbuf.h"

char *
dolfptoa(fpi, fpv, neg, ndec, msec)
	u_int fpi;
	u_int fpv;
	int neg;
	int ndec;
	int msec;
{
	register u_char *cp, *cpend;
	register u_int work_i;
	register int dec;
	u_char cbuf[24];
	u_char *cpdec;
	char *buf;
	char *bp;

	/*
	 * Get a string buffer before starting
	 */
	LIB_GETBUF(buf);

	/*
	 * Zero the character buffer
	 */
	bzero(cbuf, sizeof(cbuf));

	/*
	 * Work on the integral part.  This is biased by what I know
	 * compiles fairly well for a 68000.
	 */
	cp = cpend = &cbuf[10];
	work_i = fpi;
	if (work_i & 0xffff0000) {
		register u_int lten = 10;
		register u_int ltmp;

		do {
			ltmp = work_i;
			work_i /= lten;
			ltmp -= (work_i<<3) + (work_i<<1);
			*--cp = (u_char)ltmp;
		} while (work_i & 0xffff0000);
	}
	if (work_i != 0) {
		register u_short sten = 10;
		register u_short stmp;
		register u_short swork = (u_short)work_i;

		do {
			stmp = swork;
			swork /= sten;
			stmp -= (swork<<3) + (swork<<1);
			*--cp = (u_char)stmp;
		} while (swork != 0);
	}

	/*
	 * Done that, now deal with the problem of the fraction.  First
	 * determine the number of decimal places.
	 */
	if (msec) {
		dec = ndec + 3;
		if (dec < 3)
			dec = 3;
		cpdec = &cbuf[13];
	} else {
		dec = ndec;
		if (dec < 0)
			dec = 0;
		cpdec = &cbuf[10];
	}
	if (dec > 12)
		dec = 12;
	
	/*
	 * If there's a fraction to deal with, do so.
	 */
	if (fpv != 0) {
		register u_int work_f;

		work_f = fpv;
		while (dec > 0) {
			register u_int tmp_i;
			register u_int tmp_f;

			dec--;
			/*
			 * The scheme here is to multiply the
			 * fraction (0.1234...) by ten.  This moves
			 * a junk of BCD into the units part.
			 * record that and iterate.
			 */
			work_i = 0;
			M_LSHIFT(work_i, work_f);
			tmp_i = work_i;
			tmp_f = work_f;
			M_LSHIFT(work_i, work_f);
			M_LSHIFT(work_i, work_f);
			M_ADD(work_i, work_f, tmp_i, tmp_f);
			*cpend++ = (u_char)work_i;
			if (work_f == 0)
				break;
		}

		/*
		 * Rounding is rotten
		 */
		if (work_f & 0x80000000) {
			register u_char *tp = cpend;

			*(--tp) += 1;
			while (*tp >= 10) {
				*tp = 0;
				*(--tp) += 1;
			};
			if (tp < cp)
				cp = tp;
		}
	}
	cpend += dec;


	/*
	 * We've now got the fraction in cbuf[], with cp pointing at
	 * the first character, cpend pointing past the last, and
	 * cpdec pointing at the first character past the decimal.
	 * Remove leading zeros, then format the number into the
	 * buffer.
	 */
	while (cp < cpdec) {
		if (*cp != 0)
			break;
		cp++;
	}
	if (cp == cpdec)
		--cp;

	bp = buf;
	if (neg)
		*bp++ = '-';
	while (cp < cpend) {
		if (cp == cpdec)
			*bp++ = '.';
		*bp++ = (char)(*cp++ + '0');	/* ascii dependent? */
	}
	*bp = '\0';

	/*
	 * Done!
	 */
	return buf;
}
