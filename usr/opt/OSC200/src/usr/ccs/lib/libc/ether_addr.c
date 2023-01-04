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

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define ether_aton __ether_aton
#define ether_hostton __ether_hostton
#define ether_line __ether_line
#define ether_ntoa __ether_ntoa
#define ether_ntohost __ether_ntohost
#pragma weak ether_aton = __ether_aton
#pragma weak ether_hostton = __ether_hostton
#pragma weak ether_line = __ether_line
#pragma weak ether_ntoa = __ether_ntoa
#pragma weak ether_ntohost = __ether_ntohost
#endif

#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ether_addr.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/06/22 19:43:38 $";
#endif 

#if !defined(lint) && !defined(_NOIDENT)

#endif


/* Based on:
 *   ether_addr.c	5.1  (ULTRIX)        3/29/91
 *   ether_addr.c	1.2 87/09/11 3.2/4.3NFSSRC
 */

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

/*
/*
 * All routines necessary to deal with the file /etc/ethers.  The file
 * contains mappings from 48 bit ethernet addresses to their corresponding
 * hosts name.  The addresses have an ascii representation of the form
 * "x:x:x:x:x:x" where x is a hex number between 0x00 and 0xff;  the
 * bytes are always in network order.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

static int useyp();  /* tells whether we use yp or a local file */
static char domain[256]; /* yp domain name */
static char *ethers_filename = "/etc/ethers";

/*
 * Parses a line from /etc/ethers into its components.  The line has the form
 * 8:0:20:1:17:c8	krypton
 * where the first part is a 48 bit ethernet address and the second is
 * the corresponding hosts name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_line(s, e, hostname)
	char *s;		/* the string to be parsed */
	struct ether_addr *e;	/* ethernet address struct to be filled in */
	char *hostname;		/* hosts name to be set */
{
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x %s",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], hostname);
	if (i != 7) {
		return (7 - i);
	}
	for (i = 0; i < 6; i++)
		e->ether_addr_octet[i] = t[i];
	return (0);
}

/*
 * Converts a 48 bit ethernet number to its string representation.
 */
#define EI(i)	(unsigned int)(e->ether_addr_octet[(i)])
char *
ether_ntoa(e)
	struct ether_addr *e;
{
	static char s[18];
	
	s[0] = 0;
	sprintf(s, "%x:%x:%x:%x:%x:%x",
	    EI(0), EI(1), EI(2), EI(3), EI(4), EI(5));
	return (s);
}

/*
 * Converts a ethernet address representation back into its 48 bits.
 */
struct ether_addr *
ether_aton(s)
	char *s;
{
	static struct ether_addr e;
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6)
	    return ((struct ether_addr *)NULL);
	for (i = 0; i < 6; i++) {
		e.ether_addr_octet[i] = t[i];
	}
	return(&e);
}

/*
 * Given a host's name, this routine returns its 48 bit ethernet address.
 * Returns zero if successful, non-zero otherwise.
 */
ether_hostton(host, e)
	char *host;		/* function input */
	struct ether_addr *e;	/* function output */
{
	char currenthost[256];
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byname", host,
		    strlen(host), &val, &vallen)) {
			return (reason);
		} else {
			return (ether_line(val, e, currenthost));
		}
	} else {
		if ((f = fopen(ethers_filename, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((ether_line(val, e, currenthost) == 0) &&
			    (strcmp(currenthost, host) == 0)) {
				reason = 0;
				break;
			}
		}
		fclose(f);
		return (reason);
	}
}

/*
 * Given a 48 bit ethernet address, this routine return its host name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_ntohost(host, e)
	char *host;		/* function output */
	struct ether_addr *e;	/* function input */
{
	struct ether_addr currente;
	char *a = ether_ntoa(e);
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byaddr", a,
		    strlen(a), &val, &vallen)) {
			return (reason);
		} else {
			return (ether_line(val, &currente, host));
		}
	} else {
		if ((f = fopen(ethers_filename, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((ether_line(val, &currente, host) == 0) &&
			    (bcmp(e, &currente, sizeof(currente)) == 0)) {
				reason = 0;
				break;
			}
		}
		fclose(f);
		return (reason);
	}
}

/*
 * Determines whether or not to use the yellow pages service to do lookups.
 */
static int initted;
static int usingyp;
static int
useyp()
{
	if (!initted) {
		getdomainname(domain, sizeof(domain));
		usingyp = !yp_bind(domain);
		initted = 1;
	}
	return (usingyp);
}
