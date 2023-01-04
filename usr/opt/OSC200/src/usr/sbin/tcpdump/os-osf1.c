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
static char *rcsid = "@(#)$RCSfile: os-osf1.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:33:57 $";
#endif

#include <sys/types.h>

#include "os.h"

#ifdef	ETHER_SERVICE

u_char *
ETHER_hostton(name)
	char *name;
{
	u_char *ep;

	ep = (u_char *)malloc(6);
	if (ether_hostton(name, ep) == 0)
		return ep;
	free((char *)ep);
	return 0;
}

char *
ETHER_ntohost(ep)
	u_char *ep;
{
	char buf[128], *cp;

	if (ether_ntohost(buf, ep) == 0) {
		cp = (char *)malloc(strlen(buf) + 1);
		strcpy(cp, buf);
		return cp;
	}
	return 0;
}

#endif	/* ETHER_SERVICE */
