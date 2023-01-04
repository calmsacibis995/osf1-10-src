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
static char     *sccsid = "@(#)$RCSfile: get_myaddress.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:57:01 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.5 88/02/08 
 */


/*
 * get_myaddress.c
 *
 * Get client's IP address via ioctl.  This avoids using the NIS.
 * Copyright (C) 1990, Sun Microsystems, Inc.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak get_myaddress = __get_myaddress
#endif
#endif
#include <rpc/types.h>
#include <rpc/pmap_prot.h>
#include <sys/socket.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/syslog.h>

/*
 * don't use gethostbyname, which would invoke NIS
 */
get_myaddress(addr)
	struct sockaddr_in *addr;
{
	int s;
	char buf[BUFSIZ*8];
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	int len;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    (void) syslog(LOG_ERR, "get_myaddress: socket: %m");
	    exit(1);
	}
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		(void) syslog(LOG_ERR, "get_myaddress: ioctl (get interface configuration): %m");
		exit(1);
	}
	ifr = ifc.ifc_req;
	for (len = ifc.ifc_len; len; len -= sizeof ifreq) {
		ifreq = *ifr;
		if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			(void) syslog(LOG_ERR, "get_myaddress: ioctl: %m");
			exit(1);
		}
		if ((ifreq.ifr_flags & IFF_UP) &&
		    ifr->ifr_addr.sa_family == AF_INET) {
			*addr = *((struct sockaddr_in *)&ifr->ifr_addr);
			addr->sin_port = htons(PMAPPORT);
			break;
		}
		ifr++;
	}
	(void) close(s);
}
