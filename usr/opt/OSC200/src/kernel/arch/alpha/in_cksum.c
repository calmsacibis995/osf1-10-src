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
static char *rcsid = "@(#)$RCSfile: in_cksum.c,v $ $Revision: 1.2.2.5 $ (DEC) $Date: 1992/08/31 12:16:16 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)in_cksum.c	9.1	(ULTRIX/OSF)	10/21/91";
#endif	lint

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * Modification History: alpha/in_cksum.c
 *
 * 30-Aug-90 -- afd
 *	Created this file for Alpha support.
 */

#include <sys/param.h>
#include <sys/mbuf.h>

/*
 * Checksum routine for Internet Protocol family headers.
 *
 * This routine is very heavily used in the network code
 * and should be modified for each architecture to be as fast as possible.
 */

int nocksum;

in_cksum(m, len)
	register struct mbuf *m;
	register int len;
{
	register int ck;
	unsigned short in_checksum();
	register int mlen;
	register char *addr;
	register rlen;

	if (nocksum)
		return(0);

	ck = rlen = 0;
	while(m) {
		mlen = (m->m_len > len) ? len : m->m_len;
		addr = mtod(m, char *);

		if ((rlen^(int)addr)&1)
			ck = nuxi_16(in_checksum(addr, mlen, nuxi_16(ck)));
		else
			ck = in_checksum(addr, mlen, ck);

		rlen += mlen;
		len -= mlen;
		if (len == 0)
			break;
		while (m = m->m_next)
			if (m->m_len)
				break;
	}
	if (len)
		printf("in_cksum, ran out of data, %d bytes left\n", len);

	return(~ck & 0xFFFF);
}
