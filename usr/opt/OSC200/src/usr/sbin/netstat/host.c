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
static char	*sccsid = "@(#)$RCSfile: host.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 17:26:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint


#endif /* not lint */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <net/if.h>

#include <netinet/in.h>
#if	MACH
#include <netinet/if_ether.h>
#endif

extern	int kmem;
extern 	int nflag;
extern	char *inetname();


#if	MACH
/*
 * Print the ARP cache protocol/hardware address cache.
 * Symbolic addresses are shown unless the nflag is given.
 */
arppr(arptabaddr, bsizaddr, nbaddr)
	off_t arptabaddr;
	off_t bsizaddr;
	off_t nbaddr;
{
	int bsize = 5;
	int nb = 19;
	caddr_t arptabptr;
	register struct arptab *arptab;
	register int i, j = 0, m;
	char flagbuf[10], *flags;
	extern int Aflag;

	if (arptabaddr == 0) {
		printf("arptab: symbol(s) not in namelist\n");
		return;
	}
	if (arptabaddr != 0)
	{
		klseek(kmem, (off_t)arptabaddr, 0);
		read(kmem, &arptabptr, sizeof (arptabptr));
	}
	if (bsizaddr != 0)
	{
		klseek(kmem, (off_t)bsizaddr, 0);
		read(kmem, &bsize, sizeof (bsize));
	}
	if (nbaddr != 0)
	{
		klseek(kmem, (off_t)nbaddr, 0);
		read(kmem, &nb, sizeof (nb));
	}
	m = bsize*nb;
        arptab = (struct arptab *)malloc(sizeof(struct arptab)*m);
	if (arptab == 0)
	{
		printf("arptab: could not allocate %d table entries\n", m);
		return;
	}
	klseek(kmem, (off_t)arptabptr, 0);
	read(kmem, arptab, sizeof (struct arptab)*m);
	printf("Address Resolution Mapping Table: %d Buckets %d Deep %d Bytes\n",
		nb, bsize, sizeof (struct arptab));
	printf("Cnt  ");
	if (Aflag)
		printf("%-8.8s ", "ADDR");
	printf("%-8.8s %-5.5s %-24.24s %s\n",
		"Flags", "Timer", "Host", "Phys Addr");
	for (i=0; i<m; i++) {
		register struct arptab *at = &arptab[i];

		if (at->at_flags == 0)
			continue;
		flags = flagbuf;
		if (at->at_flags&ATF_INUSE)
			*flags++ = 'U';
		if (at->at_flags&ATF_COM)
			*flags++ = 'C';
		if (at->at_flags&ATF_PERM)
			*flags++ = 'P';
		if (at->at_flags&ATF_PUBL)
			*flags++ = 'B';
		if (at->at_flags&ATF_USETRAILERS)
			*flags++ = 'T';
#ifdef	ATF_STALE
		if (at->at_flags & ATF_USE802)	/* not used */
			*flags++ = 'I';
		if (at->at_flags & ATF_STALE)
			*flags++ = 'S';
		if (at->at_flags & ATF_DEAD)
			*flags++ = 'D';
#endif
		*flags = '\0';
		printf("%3d: ", ++j);
		if (Aflag)
			printf("%8x ",arptabaddr+((caddr_t)at-(caddr_t)arptab));
		printf("%-8.8s  %3d  %-24.24s ", flagbuf, at->at_timer, inetname(at->at_iaddr));
		if (at->at_enaddr[5] == 0 && at->at_enaddr[4] == 0 && at->at_enaddr[3] == 0 && at->at_enaddr[2] == 0 && at->at_enaddr[1] == 0)
			printf("#%o", at->at_enaddr[0]);
		else
			printf("%02x:%02x:%02x:%02x:%02x:%02x",
			       at->at_enaddr[0], at->at_enaddr[1], at->at_enaddr[2],
			       at->at_enaddr[3], at->at_enaddr[4], at->at_enaddr[5]);
		printf("\n");
	}
}
#endif	/* MACH */
