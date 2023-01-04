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
static char *rcsid = "@(#)$RCSfile: pfilt.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:22:30 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/pfilt.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

#ifdef USE_PFILT
/*
 * pfilt.c - routines for messing with the packet filter
 *
 * Jeffrey Mogul
 * DECWRL
 *
 * log: pfilt.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.5  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.4  1993/01/13  21:25:05  davy
 * #endif cleanup.
 *
 * Revision 3.3  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.2  1992/07/24  18:47:57  mogul
 * Added FDDI support
 *
 * Revision 3.1  1992/07/23  00:02:18  mogul
 * Fixed improper use of if_fd[] variable.
 *
 * Revision 3.0  1991/01/23  08:23:15  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/12/04  08:02:43  davy
 * Changes from Jeff Mogul for Ultrix 4.1 and higher.
 * 
 * Revision 1.1  90/08/17  15:47:34  davy
 * Initial revision
 * 
 * Revision 1.1  90/04/20  13:59:36  mogul
 * Initial revision
 * 
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>

#include <net/pfilt.h>

#include "nfswatch.h"
#include "externs.h"

static struct ifreq ifr;			/* holds interface name	*/

/*
 * setup_pfilt_dev - set up the packet filter
 */
int
setup_pfilt_dev(device)
char **device;
{
	int fd;
	struct timeval timeout;
	short enmode;
	short backlog = -1;	/* request the most */
	struct enfilter Filter;

	/*
	 * Open the packetfilter.  If it fails, we're out of
	 * devices.
	 */
	if ((fd = pfopen(*device, 0)) < 0) {
		return(-1);
	}

	/*
	 * We want the ethernet in promiscuous mode
	 */
	enmode = ENBATCH|ENTSTAMP|ENNONEXCL|ENPROMISC;
	if (ioctl(fd, EIOCMBIS, &enmode) < 0) {
		error("ioctl: EIOCMBIS");
		finish(-1);
	}

#ifdef ENCOPYALL
	/*
	 * Attempt to set "copyall" mode (see our own packets).
	 * Okay if this fails.
	 */
	enmode = ENCOPYALL;
	(void) ioctl(fd, EIOCMBIS, &enmode);
#endif

	/*
	 * Set the read timeout.
	 */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (ioctl(fd, EIOCSRTIMEOUT, &timeout) < 0) {
		error("ioctl: EIOCSRTIMEOUT");
		finish(-1);
	}

	/* set the backlog */
	if (ioctl(fd, EIOCSETW, &backlog) < 0) {
		error("ioctl: EIOCSETW");
		finish(-1);
	}

	/* set the truncation */
	if (ioctl(fd, EIOCTRUNCATE, &truncation) < 0) {
		error("ioctl: EIOCTRUNCATE");
		finish(-1);
	}

	/* find out the actual device name */
	if (*device == NULL) {
		if (ioctl(fd, EIOCIFNAME, &ifr) >= 0) {
			*device = ifr.ifr_name;
		}
		else {
			*device = "pf0";
		}
	}

	/* accept all packets */
	Filter.enf_Priority = 37;	/* anything > 2 */
	Filter.enf_FilterLen = 0;	/* means "always true" */
	if (ioctl(fd, EIOCSETF, &Filter) < 0) {
		error("ioctl: EIOCSETF");
		finish(-1);
	}

	return(fd);
}

/*
 * pfilt_devtype - return device type code for packet filter device
 */
int
pfilt_devtype(fd)
int fd;
{
	struct endevp devparams;

	if (ioctl(fd, EIOCDEVP, &devparams) < 0) {
		error("ioctl: EIOCDEVP");
		finish(-1);
	}

	switch (devparams.end_dev_type) {
	case ENDT_10MB:
		return(DLT_EN10MB);
		
#ifdef	ENDT_FDDI		/* HACK: to compile prior to Ultrix 4.2 */
	case ENDT_FDDI:
		return(DLT_FDDI);
#endif
		
	default:
		/*
		 * Currently, the Ultrix packet filter supports only
		 * Ethernet and FDDI.
		 */
		fprintf(stderr, "Packet filter data-link type %d unknown\n",
				devparams.end_dev_type);
		fprintf(stderr, "Assuming Ethernet\n");
		return(DLT_EN10MB);
	}
}

/*
 * flush_pfilt - flush data from the packet filter
 */
void
flush_pfilt(fd)
int fd;
{
	if (ioctl(fd, EIOCFLUSH) < 0) {
		error("ioctl: EIOCFLUSH");
		finish(-1);
	}
}


/* JSD: pfopen.c taken from /usr/examples/packetfilter/pfopen.c for now */
#include <sys/errno.h>
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

#endif /* USE_PFILT */
