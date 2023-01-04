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
static char     *sccsid = "@(#)$RCSfile: numtohost.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:39:32 $";
#endif
/*
 */

/*
 * numtohost - convert network number to host name.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "lib_strbuf.h"

#define	LOOPBACKNET	0x7f000000
#define	LOOPBACKHOST	0x7f000001
#define	LOOPBACKNETMASK	0xff000000

char *
numtohost(netnum)
	u_int netnum;
{
	char *bp;
	struct hostent *hp;
	extern char *numtoa();

	/*
	 * This is really gross, but saves lots of hanging looking for
	 * hostnames for the radio clocks.  Don't bother looking up
	 * addresses on the loopback network except for the loopback
	 * host itself.
	 */
	if ((((ntohl(netnum) & LOOPBACKNETMASK) == LOOPBACKNET)
	    && (ntohl(netnum) != LOOPBACKHOST))
	    || ((hp = gethostbyaddr((char *)&netnum, sizeof netnum, AF_INET))
	      == 0))
		return numtoa(netnum);
	
	LIB_GETBUF(bp);
	
	bp[LIB_BUFLENGTH-1] = '\0';
	(void) strncpy(bp, hp->h_name, LIB_BUFLENGTH-1);
	return bp;
}
