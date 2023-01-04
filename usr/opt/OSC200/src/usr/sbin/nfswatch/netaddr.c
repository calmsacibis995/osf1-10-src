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
static char *rcsid = "@(#)$RCSfile: netaddr.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/21 23:35:40 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/netaddr.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * netaddr.c - routines for working with network addresses.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: netaddr.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.4  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.3  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.2  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.1  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.0  1991/01/23  08:23:06  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/08/17  15:47:24  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:35  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <netdb.h>
#include <stdio.h>

#include "nfswatch.h"
#include "externs.h"

/*
 * get_net_addrs - get network addresses of source and destination
 *		   hosts, along with official host names.
 */
void
get_net_addrs()
{
	register int n;
	char *inet_ntoa();
	register char **cp;
	struct hostent *hp;

	/*
	 * Look up the local host.
	 */
	if ((hp = gethostbyname(myhost)) == NULL) {
		(void) fprintf(stderr, "%s: %s: unknown host.\n", pname,
			myhost);
		finish(-1);
	}

	/*
	 * Save the official host name.
	 */
	(void) strcpy(myhost, hp->h_name);

	/*
	 * If one was specified, look up the destination host.
	 * Otherwise, we can use what we have.
	 */
	if (allflag) {
		(void) sprintf(dsthost, "all hosts");
	}
	else if (dstflag) {
		if ((hp = gethostbyname(dsthost)) == NULL) {
			(void) fprintf(stderr, "%s: %s: unknown host.\n", pname,
				dsthost);
			finish(-1);
		}

		/*
		 * Save the official host name.
		 */
		(void) strcpy(dsthost, hp->h_name);
	}
	else {
		/*
		 * Host name is the same as the local
		 * host.
		 */
		(void) strcpy(dsthost, myhost);
	}

	/*
	 * Copy destination host's network addresses.
	 */
	n = 0;
	(void) bzero((char *) dstaddrs, MAXHOSTADDR * sizeof(ipaddrt));

	for (cp = hp->h_addr_list; *cp != NULL; cp++) {
		if (n >= MAXHOSTADDR)
			break;

		(void) bcopy(*cp, (char *) &dstaddrs[n], hp->h_length);
		n++;
	}

	/*
	 * If they specified a server host, get its addresses.
	 */
	if (serverflag) {
		if ((hp = gethostbyname(serverhost)) == NULL) {
			fprintf(stderr, "%s: %s: unknown host.\n", pname,
				serverhost);
			finish(-1);
		}

		/*
		 * Save the official host name.
		 */
		(void) strcpy(serverhost, hp->h_name);

		/*
		 * Copy the server's network addresses.
		 */
		n = 0;
		(void) bzero((char *) serveraddrs, MAXHOSTADDR *
			     sizeof(ipaddrt));

		for (cp = hp->h_addr_list; *cp != NULL; cp++) {
			if (n >= MAXHOSTADDR)
				break;

			(void) bcopy(*cp, (char *) &serveraddrs[n],
				     hp->h_length);
			n++;
		}
	}

	/*
	 * If they didn't specify a source host,
	 * we're done.
	 */
	if (!srcflag)
		return;

	/*
	 * Look up the source host.
	 */
	if ((hp = gethostbyname(srchost)) == NULL) {
		(void) fprintf(stderr, "%s: %s: unknown host.\n", pname,
			srchost);
		finish(-1);
	}

	/*
	 * Save the official host name.
	 */
	(void) strcpy(srchost, hp->h_name);

	/*
	 * Copy source host's network addresses.
	 */
	n = 0;
	(void) bzero((char *) srcaddrs, MAXHOSTADDR * sizeof(ipaddrt));

	for (cp = hp->h_addr_list; *cp != NULL; cp++) {
		if (n >= MAXHOSTADDR)
			break;

		(void) bcopy(*cp, (char *) &srcaddrs[n], hp->h_length);
		n++;
	}
}

/*
 * want_packet - determine if we're interested in a packet by examining
 *		 its source and destination addresses.
 */
int
want_packet(src, dst)
ipaddrt src, dst;
{
	register int i, want;

	want = FALSE;
	thisdst = dst;

	/*
	 * Check that the source or destination is the server.
	 */
	if (serverflag) {
		for (i=0; (serveraddrs[i] != 0) && (i < MAXHOSTADDR); i++) {
			if (!bcmp((char *) &src, (char *) &serveraddrs[i],
				  sizeof(ipaddrt)) ||
			    !bcmp((char *) &dst, (char *) &serveraddrs[i],
				  sizeof(ipaddrt))) {
				want = TRUE;
				break;
			}
		}
		return(want);
	}
	 
	/*
	 * Any source or destination is okay.
	 */
	if (allflag) {
		return(TRUE);
	}

	/*
	 * Check source address first.
	 */
	if (srcflag) {
		for (i = 0; (srcaddrs[i] != 0) && (i < MAXHOSTADDR); i++) {
			if (!bcmp((char *) &src, (char *) &srcaddrs[i],
			    sizeof(ipaddrt))) {
				want = TRUE;
				break;
			}
		}

		/*
		 * If it's not from our source, we
		 * don't even need to check the destination.
		 */
		if (!want)
			return(FALSE);
	}

	want = FALSE;

	/*
	 * Check destination address.
	 */
	for (i = 0; (dstaddrs[i] != 0) && (i < MAXHOSTADDR); i++) {
		if (!bcmp((char *) &dst, (char *) &dstaddrs[i],
		    sizeof(ipaddrt))) {
			want = TRUE;
			break;
		}
	}

	return(want);
}
