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
static char	*sccsid = "@(#)$RCSfile: mbuf.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/08/03 15:48:20 $";
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

#include <stdio.h>
#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>

struct	mbstat mbstat;

/* 
 * Note: using struct kmemtypes as defined in sys/malloc is incorrect 
 * due to the lock being included for #ifdef _KERNEL only.  Defining 
 * _KERNEL is not satisfactory, since several other headers then
 * need inclusion, ad nauseum.  This needs to be kept synchronised with
 * sys/malloc.h
 */
struct kmemtype {
	long	kt_memuse;	/* # of bytes of this type in use */
	long	kt_limit;	/* # of bytes of this type allowed to exist */
	long	kt_wait;	/* blocking flag for this type of memory */
	long	kt_limblocks;	/* stat: # of times blocked for hitting limit */
	long	kt_failed;	/* stat: # of times failed for hitting
							limit with NOWAIT */
        long    kt_maxused;     /* max # of bytes of this type ever used*/
	int	kt_lock;	/* simple_lock_data_t */
} kmemtype[M_LAST];

#define	YES	1
typedef int bool;

extern	int kmem;

static struct mbtypes {
	int	mt_type;
	char	*mt_name;
} mbtypes[] = {
	{ MT_DATA,	"data" },
	{ MT_OOBDATA,	"oob data" },
	{ MT_CONTROL,	"ancillary data" },
	{ MT_HEADER,	"packet headers" },
	{ MT_SOCKET,	"socket structures" },
	{ MT_PCB,	"protocol control blocks" },
	{ MT_RTABLE,	"routing table entries" },
	{ MT_HTABLE,	"IMP host table entries" },
	{ MT_ATABLE,	"address resolution tables" },
	{ MT_FTABLE,	"fragment reassembly queue headers" },
	{ MT_SONAME,	"socket names and addresses" },
	{ MT_SOOPTS,	"socket options" },
	{ MT_RIGHTS,	"access rights" },
	{ MT_IFADDR,	"interface addresses" },
	{ 0, 0 }
};

static struct ktypes {
	int	k_type;
	char	*k_name;
} ktypes[] = {
	{ M_MBUF,	"small mbufs" },
	{ M_CLUSTER,	"mbuf clusters" },
	{ M_SOCKET,	"sockets" },
	{ M_PCB,	"protocol control blocks" },
	{ M_RTABLE,	"routing table" },
	{ M_FTABLE,	"fragment table" },
	{ M_IFADDR,	"interface addresses" },
	{ M_SOOPTS,	"socket options" },
	{ M_SONAME,	"socket names" },
	{ M_IPMOPTS,	"ip multicast options" },
	{ M_IPMADDR,	"ip multicast addresses" },
	{ M_IFMADDR,	"interface multicast addresses" },
	{ M_MRTABLE,	"multicast routing table" },
	{ 0, 0}
};

int nmbtypes = sizeof(mbstat.m_mtypes) / sizeof(mbstat.m_mtypes[0]);
bool seen[256];			/* "have we seen this type yet?" */

/*
 * Print mbuf statistics.
 */

mbpr(mbaddr, mclbytaddr, kmemtypesaddr)
	off_t mbaddr, mclbytaddr, kmemtypesaddr;
{
	register int totmbufs = 0;
	register int totktype = 0;
	register int tmpuse = 0;
	register int tmpmax = 0;
	register int i;
	register struct mbtypes *mp;
	register struct ktypes *kp;
	int mclbytes = 0;

	if (mbaddr == 0) {
		printf("netstat: symbol mbstat not in namelist\n");
		return;
	}
	klseek(kmem, mbaddr, 0);
	if (read(kmem, (char *)&mbstat, sizeof (mbstat)) != sizeof (mbstat)) {
		printf("netstat: bad read on mbstat\n");
		return;
	}
	if (mclbytaddr) {
		klseek(kmem, mclbytaddr, 0);
		(void) read(kmem, (char *)&mclbytes, sizeof (mclbytes));
	}
	if (mclbytes <= 0)
		mclbytes = MCLBYTES;
	for (mp = mbtypes; mp->mt_name; mp++)
		totmbufs += mbstat.m_mtypes[mp->mt_type];
	/* 
	printf("%u mbufs in use:\n", mbstat.m_mbufs);
	if (mbstat.m_mbufs != totmbufs)
		printf("\tmbuf count %u doesn't agree\n", totmbufs); 
	for (mp = mbtypes; mp->mt_name; mp++)
		if (mbstat.m_mtypes[mp->mt_type]) {
			seen[mp->mt_type] = YES;
			printf("\t%u mbufs allocated to %s\n",
			    mbstat.m_mtypes[mp->mt_type], mp->mt_name);
		}
	seen[MT_FREE] = YES;
	for (i = 0; i < nmbtypes; i++)
		if (!seen[i] && mbstat.m_mtypes[i]) {
			printf("\t%u mbufs allocated to <mbuf type %d>\n",
			    mbstat.m_mtypes[i], i);
		}
	*/

	/* 
	 * OSF/1.1 streams and sockets allocate everything from the
	 * kernel mallocator.  Not all the stats are metwork-related
	 * - pick those that are relevant.
	 */
	if (kmemtypesaddr == 0) {
		printf("netstat: symbol kmemtypes not in namelist\n");
		return;
	}
	klseek(kmem, kmemtypesaddr, 0);
	if (read(kmem, (char *)&kmemtype, sizeof (kmemtype)) 
					!= sizeof (kmemtype)) {
		printf("netstat: bad read on kmemtypes\n");
		return;
	}
	for (kp = ktypes; kp->k_name; kp++)
		if (kmemtype[kp->k_type].kt_memuse) {
		    totktype += kmemtype[kp->k_type].kt_memuse;
		    tmpuse = (kmemtype[kp->k_type].kt_memuse + 1023) / 1024;
		    tmpmax = (kmemtype[kp->k_type].kt_maxused + 1023) / 1024;
		    if (tmpuse < 2)
			printf("    < 1 Kbyte for %s ", kp->k_name);
		    else
			printf("  %5u Kbytes for %s ", tmpuse, kp->k_name);
		    if (tmpmax < 2)
			printf("(peak usage < 1 Kbyte)\n");
		    else
			printf("(peak usage %u Kbytes)\n", tmpmax);
		}

	printf("%7u requests for mbufs denied\n", mbstat.m_drops);
	printf("%7u calls to protocol drain routines\n", mbstat.m_drain);

	printf("%u Kbytes allocated to network\n", (totktype + 1023)/1024);
}
