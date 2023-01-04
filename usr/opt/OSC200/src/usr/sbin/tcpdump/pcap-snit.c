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
static char *rcsid = "@(#)$RCSfile: pcap-snit.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:42:49 $";
#endif
/*
 * Copyright (c) 1989, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * Based on:
 * static  char rcsid[] = "pcap-snit.c,v 1.15 92/06/02 17:57:39 mccanne Exp $ (LBL)";
 */

/*
 * Modifications made to accomodate the new SunOS4.0 NIT facility by
 * Micky Liu, micky@cunixc.cc.columbia.edu, Columbia University in May, 1989.
 * This module now handles the STREAMS based NIT.
 */

#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <net/nit.h>

/* Sun OS 4.x includes */

#include <sys/fcntlcom.h>

#include <sys/dir.h>
#include <net/nit_if.h>
#include <net/nit_pf.h>
#include <net/nit_buf.h>
#include <sys/stropts.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <net/bpf.h>

#include "interface.h"

/*
 * The chunk size for NIT.  This is the amount of buffering
 * done for read calls.
 */
#define CHUNKSIZE (2*1024)

/*
 * The total buffer space used by NIT.
 */
#define BUFSPACE (4*CHUNKSIZE)

struct nit_stat {
	int dcount;
	int rcount;
} nstat;

static void
snit_stats()
{
	fflush(stdout);
	(void)fprintf(stderr, "%d packets received by filter\n", nstat.rcount);
	(void)fprintf(stderr, "%d packets dropped by kernel\n", nstat.dcount);
}

void
readloop(cnt, if_fd, fp, printit)
	int cnt;
	int if_fd;
	struct bpf_program *fp;
	void (*printit)();
{
	u_char buf[CHUNKSIZE];
	int cc;
	int drops;
	struct nit_stat *nsp = &nstat;
	register struct bpf_insn *fcode = fp->bf_insns;

	while ((cc = read(if_fd, (char*)buf, sizeof buf)) >= 0) {
		register u_char *bp, *cp, *bufstop;
		struct nit_bufhdr *hdrp;
		struct nit_iftime *ntp;
		struct nit_iflen *nlp;
		struct nit_ifdrops *ndp;
		int caplen;

		bp = buf;
		bufstop = buf + cc;
		/*
		* loop through each snapshot in the chunk
		*/
		while (bp < bufstop) {
			++nsp->rcount;
			cp = bp;
			/* get past NIT buffer  */
			hdrp = (struct nit_bufhdr *)cp;
			cp += sizeof(*hdrp);

			/* get past NIT timer   */
			ntp = (struct nit_iftime *)cp;
			cp += sizeof(*ntp);

			ndp = (struct nit_ifdrops *)cp;
			nsp->dcount = ndp->nh_drops;
			cp += sizeof *ndp;

			/* get past packet len  */
			nlp = (struct nit_iflen *)cp;
			cp += sizeof(*nlp);

			/* next snapshot        */
			bp += hdrp->nhb_totlen;

			caplen = nlp->nh_pktlen;
			if (caplen > snaplen)
				caplen = snaplen;

			if (bpf_filter(fcode, cp, nlp->nh_pktlen, caplen)) {
				if (cnt >= 0 && --cnt < 0) {
					wrapup(if_fd);
					return;
				}
				(*printit)(cp, &ntp->nh_timestamp,
					   nlp->nh_pktlen, caplen);
			}
		}
	}
	perror("tcpdump: read");
	exit(-1);
}

wrapup(fd)
	int fd;
{
	snit_stats();
	close(fd);
}

int
initdevice(device, pflag, linktype)
	char *device;
	int pflag;
	int *linktype;
{
	struct strioctl si;		/* struct for ioctl() */
	struct timeval timeout;		/* timeout for ioctl() */
	struct ifreq ifr;		/* interface request struct */
	u_long if_flags;		/* modes for interface             */
	int  ret;
	int chunksize = CHUNKSIZE;
	int if_fd;
	char *dev = "/dev/nit";

	if (snaplen < 96)
		/*
		 * NIT requires a snapshot length of at least 96.
		 */
		snaplen = 96;

	if ((if_fd = open(dev, O_RDONLY)) < 0) {
		(void) fprintf(stderr, "tcpdump: open: ");
		perror(dev);
		exit(-1);
	}

	/* arrange to get discrete messages from the STREAM and use NIT_BUF */
	ioctl(if_fd, I_SRDOPT, (char*)RMSGD);
	ioctl(if_fd, I_PUSH, "nbuf");

	/* set the timeout */
	si.ic_timout = INFTIM;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	si.ic_cmd = NIOCSTIME;
	si.ic_len = sizeof(timeout);
	si.ic_dp = (char*)&timeout;
	if ((ret = ioctl(if_fd, I_STR, (char*)&si)) < 0) {
		perror("tcpdump: NIOCSTIME");
		exit(-1);
	}

	/* set the chunksize */
	si.ic_cmd = NIOCSCHUNK;
	si.ic_len = sizeof(chunksize);
	si.ic_dp = (char*)&chunksize;
	if ((ret = ioctl(if_fd, I_STR, (char*)&si)) < 0) {
		perror("tcpdump: NIOCSCHUNK");
		exit(-1);
	}

	/* request the interface */
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = ' ';
	si.ic_cmd = NIOCBIND;
	si.ic_len = sizeof(ifr);
	si.ic_dp = (char*)&ifr;
	if ((ret = ioctl(if_fd, I_STR, (char*)&si)) < 0) {
		(void) fprintf(stderr, "tcpdump: NIOCBIND");
		perror(ifr.ifr_name);
		exit(1);
	}

	/* set the snapshot length */
	si.ic_cmd = NIOCSSNAP;
	si.ic_len = sizeof(snaplen);
	si.ic_dp = (char*)&snaplen;
	if ((ret = ioctl(if_fd, I_STR, (char*)&si)) < 0) {
		perror("tcpdump: NIOCSSNAP");
		exit(1);
	}

	/* set the interface flags */
	si.ic_cmd = NIOCSFLAGS;
	if_flags = NI_TIMESTAMP | NI_LEN | NI_DROPS;
	if (pflag == 0)
		if_flags |= NI_PROMISC;
	si.ic_len = sizeof(if_flags);
	si.ic_dp = (char*)&if_flags;
	if ((ret = ioctl(if_fd, I_STR, (char*)&si)) < 0) {
		perror("tcpdump: NIOCSFLAGS");
		exit(1);
	}
	ioctl(if_fd, I_FLUSH, (char*)FLUSHR);
	/*
	 * NIT supports only ethernets.
	 */
	*linktype = DLT_EN10MB;

	return if_fd;
}
