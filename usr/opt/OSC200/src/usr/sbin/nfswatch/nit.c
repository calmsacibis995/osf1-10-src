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
static char *rcsid = "@(#)$RCSfile: nit.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:22:06 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/nit.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

#ifdef USE_NIT
/*
 * nit.c - routines for messing with the network interface tap.
 *
 * David A. Curry
 * Purdue University
 * Engineering Computer Network
 * 1285 Electrical Engineering Building
 * West Lafayette, IN 47907-1285
 * davy@ecn.purdue.edu
 *
 * log: nit.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.6  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.5  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.4  1993/01/15  14:34:33  davy
 * Changed to handle the default network interface even if the loopback
 * was ifconfig'd first.
 *
 * Revision 3.3  1993/01/14  15:51:16  davy
 * Added FDDI code and device type calculation to NIT and SNOOP.  The FDDI
 * stuff almost definitely won't work without modification on the SNOOP
 * side; it still needs to be tested on the NIT side.
 *
 * Revision 3.2  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.1  1993/01/13  13:03:01  davy
 * Fixed the SUNOS40 calculation, and also fixed to only use promiscuous
 * mode when needed.
 *
 * Revision 3.0  1991/01/23  08:23:14  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.4  90/12/04  08:25:22  davy
 * Fixed to automatically define SUNOS40.
 * 
 * Revision 1.3  90/12/04  08:11:40  davy
 * Changed ifdef for SunOS 4.0.x.
 * 
 * Revision 1.2  90/08/17  15:47:32  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:47  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>

#include <net/nit_if.h>
#include <net/nit_buf.h>

#include "nfswatch.h"
#include "externs.h"

#if NOFILE <= 64
#define SUNOS40 1
#endif

/*
 * setup_nit_dev - set up the network interface tap.
 */
int
setup_nit_dev(device)
char **device;
{
	int n, s, fd;
	u_int chunksz;
	u_long if_flags;
	char buf[BUFSIZ];
	struct ifconf ifc;
	struct strioctl si;
	struct timeval timeout;
	struct ifreq ifr, *ifrp;

	/*
	 * If the interface device was not specified,
	 * get the default one.
	 */
	if (*device == NULL) {
		/*
		 * Grab a socket.
		 */
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			error("socket");
			finish(-1);
		}

		ifc.ifc_buf = buf;
		ifc.ifc_len = sizeof(buf);

		/*
		 * See what devices we've got.
		 */
		if (ioctl(s, SIOCGIFCONF, (char *) &ifc) < 0) {
			error("ioctl: SIOCGIFCONF");
			finish(-1);
		}

		/*
		 * Take the first device we encounter.
		 */
		ifrp = ifc.ifc_req;
		for (n = ifc.ifc_len/sizeof(struct ifreq); n > 0; n--,ifrp++) {
			/*
			 * Skip the loopback interface.
			 */
			if (strcmp(ifrp->ifr_name, "lo0") == 0)
				continue;

			*device = savestr(ifrp->ifr_name);
			break;
		}

		(void) close(s);
	}

	/*
	 * We want the ethernet in promiscuous mode if we're looking
	 * at nodes other than ourselves, and we want to know about 
	 * dropped packets.
	 */
	if (allflag || dstflag)
		if_flags = NI_DROPS | NI_PROMISC | NI_TIMESTAMP;
	else
		if_flags = NI_DROPS | NI_TIMESTAMP;

	/*
	 * Open the network interface tap.
	 */
	if ((fd = open(NIT_DEV, O_RDONLY)) < 0) {
		error("nit: open");
		finish(-1);
	}

	/*
	 * Arrange to get discrete messages.
	 */
	if (ioctl(fd, I_SRDOPT, (char *) RMSGD) < 0) {
		error("ioctl: I_SRDOPT");
		finish(-1);
	}

	/*
	 * Push and configure the nit buffering module.
	 */
	if (ioctl(fd, I_PUSH, NIT_BUF) < 0) {
		error("ioctl: I_PUSH NIT_BUF");
		finish(-1);
	}

	/*
	 * Set the read timeout.
	 */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	si.ic_cmd = NIOCSTIME;
	si.ic_timout = INFTIM;
	si.ic_len = sizeof(timeout);
	si.ic_dp = (char *) &timeout;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR NIOCSTIME");
		finish(-1);
	}

	/*
	 * Set the chunk size.
	 */
	chunksz = NIT_CHUNKSIZE;

	si.ic_cmd = NIOCSCHUNK;
	si.ic_len = sizeof(chunksz);
	si.ic_dp = (char *) &chunksz;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR NIOCSCHUNK");
		finish(-1);
	}

	/*
	 * Configure the network interface tap by binding it
	 * to the underlying interface, setting the snapshot
	 * length, and setting the flags.
	 */
	(void) strncpy(ifr.ifr_name, *device, sizeof(ifr.ifr_name));
	ifr.ifr_name[sizeof(ifr.ifr_name)-1] = '\0';

	si.ic_cmd = NIOCBIND;
	si.ic_len = sizeof(ifr);
	si.ic_dp = (char *) &ifr;

	/*
	 * If the bind fails, there's no such device.
	 */
	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		close(fd);
		return(-1);
	}

	/*
	 * SNAP is buggy on SunOS 4.0.x
	 */
#ifndef SUNOS40
	si.ic_cmd = NIOCSSNAP;
	si.ic_len = sizeof(truncation);
	si.ic_dp = (char *) &truncation;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR NIOCSSNAP");
		finish(-1);
	}
#endif

	si.ic_cmd = NIOCSFLAGS;
	si.ic_len = sizeof(if_flags);
	si.ic_dp = (char *) &if_flags;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR NIOCSFLAGS");
		finish(-1);
	}

	return(fd);
}

/*
 * flush_nit - flush data from the nit.
 */
void
flush_nit(fd)
int fd;
{
	if (ioctl(fd, I_FLUSH, (char *) FLUSHR) < 0) {
		error("ioctl: I_FLUSH");
		finish(-1);
	}
}

/*
 * nit_devtype - determine the type of device we're looking at.
 */
int
nit_devtype(device)
char *device;
{
	/*
	 * This whole routine is a kludge.  Ultrix does it the
	 * right way; see pfilt.c.
	 */

	if (strncmp(device, "le", 2) == 0 || strncmp(device, "ie", 2) == 0)
		return(DLT_EN10MB);
	
	if (strncmp(device, "fddi", 4) == 0)
		return(DLT_FDDI);

	fprintf(stderr, "Unknown device type: %s -- assuming ethernet.\n",
		device);
	return(DLT_EN10MB);
}
#endif /* USE_NIT */
