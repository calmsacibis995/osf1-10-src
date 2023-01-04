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
static char	*sccsid = "@(#)$RCSfile: in_cksum.c,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/14 18:04:47 $";
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
 * derived from in_cksum.c	2.1	(ULTRIX/OSF)	12/3/90";
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#include <sys/param.h>
#include <sys/mbuf.h>

/*
 * Checksum routine for Internet Protocol family headers (mips version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

in_cksum(m, len)
	register struct mbuf *m;
	register int len;
{
	register int ck, mlen, rlen;
	register char *addr;
	extern int in_checksum();	/* locore.s */

	for (ck = rlen = 0; len > 0 && m; m = m->m_next) {
		if ((mlen = m->m_len) <= 0)
			continue;
		if (mlen > len)
			mlen = len;

		if ((rlen ^ (int)(addr = mtod(m, char *))) & 1) {
			ck = ((ck << 8) | ((ck >> 8) & 0xff)) & 0xffff;
			ck = in_checksum(addr, mlen, ck);
			ck = ((ck << 8) | ((ck >> 8) & 0xff)) & 0xffff;
		} else
			ck = in_checksum(addr, mlen, ck);

		rlen += mlen;
		len  -= mlen;
	}
#if	INETPRINTFS
	if (len > 0 && inetprintfs)
		printf("in_cksum, ran out of data, %d bytes left\n", len);
#endif

	return(~ck & 0xFFFF);
}
