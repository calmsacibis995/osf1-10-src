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
static char	*sccsid = "@(#)$RCSfile: in_cksum.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:57 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1985 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 * 	Internet checksum routine.
 *
 * Original Author: Sandy Palmer	Created on: 09/13/85
 */


#include <sys/param.h>
#include <sys/mbuf.h>

/*****************************************************************************
 *
 * NAME:
 *	in_cksum  --  Internet checksum routine
 *
 * DESCRIPTION:
 * Checksum routine for Internet Protocol family headers
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 *
 * ARGUMENTS:
 *	struct mbuf *m - Pointer to mbuf chain containing data
 *	int len	       - Length of data in bytes.
 *
 * RETURN VALUE:
 *	Returns checksum
 *
 * SIDE EFFECTS:
 *	None.
 *
 * EXCEPTIONS:
 *	None.
 *
 * ASSUMPTIONS:
 *	None.
 */

in_cksum(m, len)
	struct mbuf *m;
	int len;
{
	u_short *w;
	int sum = 0;
	int mlen = 0;
	int extra = 0;

	for (; (len > 0) && (m != NULL); m= m->m_next)
	{
		/*
		 * If this mbuf has no data, skip it.
		 */
		if (m->m_len == 0)
			continue;

		/*
		 * Each trip around loop adds in
		 * word from one mbuf segment.
		 */
		w = mtod(m, u_short *);
		if (extra)
		{
			/*
			 * There is a byte left from the last segment;
			 * add it into the checksum.  Don't have to worry
			 * about a carry-out here because we make sure
			 * that high part of (32 bit) sum is small below.
			 */
			sum += *(u_char *)w << 8;
			w = (u_short *)((char *)w + 1);
			mlen = m->m_len - 1;
			len--;
			extra = 0;
		} else
			mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;

		/*
		 * Add data from this mbuf into the checksum.
		 */
		extra = cksum_mbuf(w, mlen, &sum);
	}
	if ( (len > 0) && (m == NULL) )
		printf("cksum: out of data\n");
	/*
	 * Add together high and low parts of sum
	 * and carry to get cksum and return result.
	 */
	return (cksum_fold(sum));
}
