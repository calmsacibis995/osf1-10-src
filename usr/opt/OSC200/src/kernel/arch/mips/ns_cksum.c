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
static char	*sccsid = "@(#)$RCSfile: ns_cksum.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:16:54 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from ns_cksum.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 * Copyright (c) 1985, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
#include <sys/param.h>
#include <sys/mbuf.h>


/*
 * Checksum routine for Network Systems Protocol Packets (MIPS Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

u_short
ns_cksum(m, len)
	register struct mbuf *m;
	register int len;
{

	register int ck;
	unsigned short ns_checksum();
	register int mlen;
	register char *addr;
	register odd = 0;

	ck = 0;
	while(m) {
		mlen = (m->m_len > len) ? len : m->m_len;
		addr = mtod(m, char *);

		if (mlen & 1)
			{
			odd++;
			mlen--;
			}

		ck = ns_checksum(addr, mlen, ck);

		if (odd)
			{
			odd--;
			mlen++;
			ck += *(u_char *)(addr + mlen) << 8;
			ck += ck;
			}

		len -= mlen;
		if (len == 0)
			break;
		while (m = m->m_next)
			if (m->m_len)
				break;
	}
	if (len)
		printf("ns_cksum, ran out of data, %d bytes left\n", len);

	if (ck == 0xffff)
		ck = 0;
	return(ck);
}
