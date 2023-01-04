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
static char     *sccsid = "@(#)$RCSfile: numtoa.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:39:25 $";
#endif
/*
 */

/*
 * numtoa - return asciized network numbers store in local array space
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib_strbuf.h"

char *
numtoa(num)
	u_int num;
{
	register u_int netnum;
	register char *buf;

	netnum = ntohl(num);
	LIB_GETBUF(buf);

	(void) sprintf(buf, "%d.%d.%d.%d", (netnum>>24)&0xff,
	    (netnum>>16)&0xff, (netnum>>8)&0xff, netnum&0xff);

	return buf;
}
