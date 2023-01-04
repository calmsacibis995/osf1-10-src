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
static char *rcsid = "@(#)$RCSfile: snoop.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:24:08 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/snoop.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

#ifdef USE_SNOOP
/*
 * snoop.c - routines for snooping on network traffic via the SGI
 *	     provided snoop(7p) network monitoring protocol
 *	     -- works under IRIX 3.2.* and IRIX 3.3.[12]
 *
 * NOTE: must be "root" to use this
 *
 * Tim Hudson
 * Mincom Pty Ltd
 * Mincom Centre
 * Juliette Street
 * Greenslopes 4120
 * Brisbane Australia
 * tjh@mincom.oz.au
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

#include <net/soioctl.h>
#include <net/raw.h>
#include <netinet/if_ether.h>

#include "nfswatch.h"
#include "externs.h"

/*
 * setup_snoop_dev - set up the network interface tap.
 */
int
setup_snoop_dev(device)
char **device;
{
	int n, s;
	int i, res;
	u_int chunksz;
	u_long if_flags;
	char buf[BUFSIZ];
	struct ifconf ifc;
	struct snoopfilter sf;
	struct timeval timeout;
	struct ifreq ifr, *ifrp;
	struct sockaddr_raw sr, sn;

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
	 * Make the raw socket.
	 */
	if ((s = socket(PF_RAW,SOCK_RAW,RAWPROTO_SNOOP)) < 0) {
		error("snoop: socket");
		finish(-1);
	}

	/*
	 * Set it up for the chosen interface.
	 */
	bzero((char *)&sr, sizeof(sr));
	sr.sr_family = AF_RAW;	
	sr.sr_port = 0;
	strncpy(sr.sr_ifname, *device, sizeof(sr.sr_ifname));

	/*
	 * If the bind fails, there's no such device.
	 */
	if (bind(s, &sr, sizeof(sr)) < 0) {
		close(s);
		return(-1);
	}

	/*
	 * Set up NULL filter - this is not necessary ...
	 */
	bzero((char *)&sf, sizeof(sf));

	if (ioctl(s, SIOCADDSNOOP, &sf) < 0) {
		error("snoop: add filter");
		finish(-1);
	}

	/*
	 * Beef up the socket buffer to minimise packet losses
	 */
	i = SNOOP_BUFFER_SIZE;
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i)) < 0) {
		error("snoop: set rcvbuf");
		finish(-1);
	}

	return(s);
}

/*
 * flush_snoop - flush data from the snoop.
 */
void
flush_snoop(fd)
int fd;
{
	int on = 1;
	int off = 0;

	/*
	 * Off then on should do a flush methinks
	 */
	ioctl(fd, SIOCSNOOPING, &off);

	if (ioctl(fd, SIOCSNOOPING, &on) < 0) {
		error("snoop: snoop on");
		finish(-1);
	}
}

/*
 * snoop_devtype - determine the type of device we're looking at.
 */
int
snoop_devtype(device)
char *device;
{
	/*
	 * This whole routine is a kludge.  Ultrix does it the
	 * right way; see pfilt.c.
	 */

	if (strncmp(device, "et", 2) == 0 || strncmp(device, "ec", 2) == 0 ||
	    strncmp(device, "fxp", 3) == 0 || strncmp(device, "enp", 3) == 0)
		return(DLT_EN10MB);
	
	if (strncmp(device, "ipg", 4) == 0)
		return(DLT_FDDI);

	fprintf(stderr, "Unknown device type: %s -- assuming ethernet.\n",
		device);
	return(DLT_EN10MB);
}
#endif /* USE_SNOOP */
