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
static char     *sccsid = "@(#)$RCSfile: getcommon.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:57:34 $";
#endif
/*
 */

/************************************************************************
 *									*
 *			Copyright (c) 1989 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Modification History:
 *
 * 25-May-89	logcher
 *	Changed name of any() to getcommon_any() to avoid conflict of
 *	ftpd and others.
 *
 * 17-May-89	logcher
 *	Removed yellowup() and any() from each get*ent.c file.
 *	Made generic routines setent_bind() and endend_bind() to
 *	avoid code duplication.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak bind_initialized = __bind_initialized
#pragma weak bindup = __bindup
#pragma weak endent_bind = __endent_bind
#pragma weak getcommon_any = __getcommon_any
#pragma weak setent_bind = __setent_bind
#pragma weak yellowup = __yellowup
#endif
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_resolv_rmutex;
#endif	/* _THREAD_SAFE */

/* 
 * Check to see if yellow pages are initialized, and return 1 is so.
 * Otherwise return 0.  YP may have been selected, but not yet enabled.
 * If flag is set to 1, then force the check.
 */
char yp_domain_name[256];

char *
yellowup(flag)
	int flag;
{
	if (yp_domain_name[0] == 0 || flag != 0)
		if (getdomainname(yp_domain_name, sizeof(yp_domain_name)) < 0) {
			fprintf(stderr, "yellowup: getdomainname system call missing\n");
			return(NULL);
		}
	if ((yp_bind(yp_domain_name)) != 0)
		return(NULL);
	return(&yp_domain_name[0]);
}

int bind_initialized = 0;

bindup()
{
	TS_LOCK(&_resolv_rmutex);
	if (!bind_initialized) {
		if (((_res.options & RES_INIT) != 0 || res_init() != -1) && (_res.defdname[0] != '\0'))
			bind_initialized = 1;
	}
	TS_UNLOCK(&_resolv_rmutex);
	return(bind_initialized);
}

char *
getcommon_any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

setent_bind(stayopen)
	int stayopen;
{
	/*
	 * If already open, or Keep TCP socket open and Use
	 * Virtual Circuit (TCP)
	 */
	if (stayopen) {
		TS_LOCK(&_resolv_rmutex);
		_res.options |= RES_STAYOPEN | RES_USEVC;
		TS_UNLOCK(&_resolv_rmutex);
	}
}

endent_bind()
{
	/*
	 * And off Keep TCP socket open and Use Virtual Circuit (TCP)
	 * and close socket.
	 */
	TS_LOCK(&_resolv_rmutex);
	_res.options &= ~(RES_STAYOPEN | RES_USEVC);
	_res_close();
	TS_UNLOCK(&_resolv_rmutex);
}
