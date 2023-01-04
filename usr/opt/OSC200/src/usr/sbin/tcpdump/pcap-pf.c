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
static char *rcsid = "@(#)$RCSfile: pcap-pf.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/21 23:24:15 $";
#endif
/*
 * Copyright (c) 1990 The Regents of the University of California.
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
 * static  char rcsid[] = "pcap-pf.c,v 1.3 92/06/29 11:06:13 mogul Exp $ (LBL)";
 */

/*
 * packet filter subroutines for tcpdump
 *	Extraction/creation by Jeffrey Mogul, DECWRL
 *
 * Extracted from tcpdump.c.
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
#include <net/pfilt.h>

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

static	u_long	TotPkts = 0;		/* can't oflow for 79 hrs on ether */
static	u_long	TotAccepted = 0;	/* count accepted by filter */
static	u_long	TotDrops = 0;		/* count of dropped packets */
static	long	TotMissed = 0;		/* missed by i/f during this run */
static	long	OrigMissed = -1;	/* missed by i/f before this run */

void pfReadError();
static void PrintPFStats();

extern int errno;

/*
 * BUFSPACE is the size in bytes of the packet read buffer.  Most tcpdump
 * applications aren't going to need more than 200 bytes of packet header
 * and the read shouldn't return more packets than packetfilter's internal
 * queue limit (bounded at 256).
 */
#define BUFSPACE (200*256)

void
readloop(cnt, if_fd, fp, printit)
	int cnt;
	int if_fd;
	struct bpf_program *fp;
	void (*printit)();
{
	u_char *p;
	struct bpf_insn *fcode;
	int cc;
	int use_bpf;

	/* This funny declaration forces buf to be properly aligned.
	   We really just want a u_char buffer that is BUFSPACE
	   bytes large. */
	struct enstamp buf[BUFSPACE / sizeof(struct enstamp)];

	/*
	 * See if BIOCSETF works.  If it does, the kernel supports
	 * BPF-style filters, and we do not need to do post-filtering.
	 */
	use_bpf = (ioctl(if_fd, BIOCSETF, (caddr_t)fp) >= 0);

	if (use_bpf) {
	    struct bpf_version bv;

	    if (ioctl(if_fd, BIOCVERSION, (caddr_t)&bv) < 0)
		warning("kernel bpf interpreter may be out of date");
	    else if (bv.bv_major != BPF_MAJOR_VERSION ||
		 	bv.bv_minor < BPF_MINOR_VERSION) {
		warning(
		      "requires bpf language %d.%d or higher; kernel is %d.%d",
			      BPF_MAJOR_VERSION, BPF_MINOR_VERSION,
			      bv.bv_major, bv.bv_minor);
		use_bpf = 0;	/* don't give up, just be inefficient */
	    }
	}

	if (use_bpf)
	    fprintf(stderr, "Using kernel BPF filter\n");
	else
	    fprintf(stderr, "Filtering in user process\n");

	fcode = fp->bf_insns;
	while (1) {
		register u_char *bp;
		int buflen, inc;
		struct enstamp stamp;

		if ((cc = read(if_fd, (char *)buf, sizeof(buf))) < 0) {
			pfReadError(if_fd, "reader");
		}
		/*
		 * Loop through each packet.
		 */
		bp = (u_char *)buf;
		while (cc > 0) {
			/* avoid alignment issues here */
			bcopy((char *)bp, (char *)&stamp, sizeof(stamp));

			if (stamp.ens_stamplen != sizeof(stamp)) {
				/* buffer is garbage, treat it as poison */
				break;
			}

			p = bp + stamp.ens_stamplen;

			buflen = stamp.ens_count;
			if (buflen > snaplen)
				buflen = snaplen;

			/*
			 * Short-circuit evaluation: if using BPF filter
			 * in kernel, no need to do it now.
			 */
			if (use_bpf ||
			    bpf_filter(fcode, p, stamp.ens_count, buflen)) { 
			    	TotAccepted++;
				if (cnt >= 0 && --cnt < 0)
					goto out;
				(*printit)(p, &stamp.ens_tstamp,
					   stamp.ens_count, buflen);
			}
			TotPkts++;
			TotDrops += stamp.ens_dropped;
			TotMissed = stamp.ens_ifoverflows;
			if (OrigMissed < 0)
				OrigMissed = TotMissed;

			inc = ENALIGN(buflen + stamp.ens_stamplen);
			cc -= inc;
			bp += inc;
		}
	}
 out:
	wrapup(if_fd);
}

/* Call ONLY if read() has returned an error on packet filter */
void
pfReadError(fid, msg)
	int fid;
	char *msg;
{
	if (errno == EINVAL) {	/* read MAXINT bytes already! */
		if (lseek(fid, 0L, 0) < 0) {
			perror("tcpdump: pfReadError/lseek");
			exit(-1);
		}
		else
			return;
	}
	else {
		(void) fprintf(stderr, "tcpdump: ");
		perror(msg);
		exit(-1);
	}
}

wrapup(fd)
	int fd;
{
	PrintPFStats();
	(void)close(fd);
}

static void
PrintPFStats()
{
	int missed = TotMissed - OrigMissed;
	
	(void)printf("%d packets", TotAccepted);
	if (TotAccepted != TotPkts)
	    (void)printf(" (out of %d examined)", TotPkts);
	if (TotDrops)
		(void)printf(" + %d discarded by kernel", TotDrops);
	if (missed)
		(void)printf(" + %d discarded by interface", missed);
	(void)printf("\n");
}

int
initdevice(device, pflag, linktype)
	char *device;
	int pflag;
	int *linktype;
{
	struct timeval timeout;
	short enmode;
	int backlog = -1;	/* request the most */
	struct enfilter Filter;
	struct endevp devparams;
	int if_fd;
	
	if_fd = pfopen(device, 0);
	if (if_fd < 0) {
		(void) fprintf(stderr, "tcpdump: pfopen: ");
		perror(device);
		error(
"your system may not be properly configured; see \"man packetfilter\"");
	}

	/* set timeout */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (ioctl(if_fd, EIOCSRTIMEOUT, &timeout) < 0) {
		perror("tcpdump: EIOCSRTIMEOUT");
		exit(-1);
	}

	enmode = ENTSTAMP|ENBATCH|ENNONEXCL;
	/* set promiscuous mode if requested */
	if (pflag == 0) {
		enmode |= ENPROMISC;
	}
	if (ioctl(if_fd, EIOCMBIS, &enmode) < 0) {
		perror("tcpdump: EIOCMBIS");
		exit(-1);
	}

#ifdef	ENCOPYALL
	/* Try to set COPYALL mode so that we see packets to ourself */
	enmode = ENCOPYALL;
	ioctl(if_fd, EIOCMBIS, &enmode);	/* OK if this fails */
#endif	ENCOPYALL

	/* set the backlog */
	if (ioctl(if_fd, EIOCSETW, &backlog) < 0) {
		perror("tcpdump: EIOCSETW");
		exit(-1);
	}

	/* set truncation */
	if (ioctl(if_fd, EIOCTRUNCATE, &snaplen) < 0) {
		perror("tcpdump: EIOCTRUNCATE");
		exit(-1);
	}
	/* accept all packets */
	Filter.enf_Priority = 37;	/* anything > 2 */
	Filter.enf_FilterLen = 0;	/* means "always true" */
	if (ioctl(if_fd, EIOCSETF, &Filter) < 0) {
		perror("tcpdump: EIOCSETF");
		exit(-1);
	}

	/* discover interface type */
	if (ioctl(if_fd, EIOCDEVP, &devparams) < 0) {
		perror(device);
		exit(-1);
	}

	/* HACK: to compile prior to Ultrix 4.2 */
#ifndef	ENDT_FDDI
#define	ENDT_FDDI	4
#endif	ENDT_FDDI

	switch (devparams.end_dev_type) {
	case ENDT_10MB:
		*linktype = DLT_EN10MB;
		break;
		
	case ENDT_FDDI:
		*linktype = DLT_FDDI;
		break;
		
	default:
		/*
		 * XXX
		 * Currently, the Ultrix packet filter supports only
		 * Ethernet and FDDI.  Eventually, support for SLIP and PPP
		 * (and possibly others: T1?) should be added.
		 */
		fprintf(stderr, "Packet filter data-link type %d unknown\n",
				devparams.end_dev_type);
		fprintf(stderr, "Assuming Ethernet\n");
		*linktype = DLT_EN10MB;
		break;
	}

	return(if_fd);
}

#ifndef have_pfopen

/* JSD: stolen from /usr/examples/packetfilter/pfopen.c, for now */

/*
 *
 * static char *rcsid = "@(#)$RCSfile: pcap-pf.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/21 23:24:15 $";
 */

#ifdef dog
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <net/if.h>
#include <net/pfilt.h>
#include <stdio.h>
#endif

#define	PFPREFIX	"/dev/pf/pfilt"		/* prefix for device names */
#define	PFMAXMINORS	256			/* 8-bit minor device field */
extern int errno;

/*
 * pfopen(ifname, flags): to support access to the Ethernet Packet Filter.
 * (using kernel options PACKETFILTER, pseudo-device packetfilter)
 *
 * ifname is a ptr to the Ethernet device name ("ln0", "xna1", "pf0", etc.)
 *	or NULL for default
 * flags are passed to the open() system call.
 *
 * return value:
 *	special device file descriptor on success
 *	-1 on failure with errno set to indicate the error
 *
 */
pfopen(ifname, flags)
char *ifname;			/* "ln0", "pf0", etc. or NULL */
int flags;
{
	int i;			/* loop counter */
	int fd;			/* file descriptor */
	char tryname[128];	/* device name: "/dev/pf/pfiltnn" */
	static int setif();

	if (ifname && (ifname[0] == 0))
	    ifname = NULL;	/* change empty string to NULL string */

	/* find next available device under the /dev/pf directory */
	for (i = 0; i < PFMAXMINORS; i++) {
		sprintf(tryname, "%s%d", PFPREFIX, i);
		fd = open(tryname, flags, 0);
		if (fd < 0) {
			switch (errno) {
			case EBUSY:	/* device in use */
				continue;	/* try the next entry */
			case ENOENT:	/* ran out of filenames */
			case ENXIO:	/* no more configured in kernel */
			default:	/* something else went wrong */
				return(-1);
			}
		}
		/* open succeeded, set the interface name */
		return(setif(fd, ifname));
	}
	return(-1);	/* didn't find an openable device */
}

static int setif(fd, ifname)
int fd;
char *ifname;
{
	if (ifname == NULL)	/* use default */
	    return(fd);

	if (ioctl(fd, EIOCSETIF, ifname) < 0) {
		close(fd);
		return(-1);
	}
	/* return the file descriptor */
	return(fd);
}

#endif /* JSD: pfopen */
