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
static char *sccsid = "@(#)$RCSfile: yppasswdxdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:00:49 $";
#endif

/*
 */

/****************************************************************
 *								*
 *  Licensed to Digital Equipment Corporation, Maynard, MA	*
 *		Copyright 1985 Sun Microsystems, Inc.		*
 *			All rights reserved.			*
 *								*
 ****************************************************************/

#include <stdio.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <rpcsvc/yppasswd.h>

yppasswd(oldpass, newpw)
	char *oldpass;
	struct passwd *newpw;
{
	int port, ok, ans;
	char domain[256];
	char *master;
	struct yppasswd yppasswd;
	
	yppasswd.oldpass = oldpass;
	yppasswd.newpw = *newpw;
	if (getdomainname(domain, sizeof(domain)) < 0)
		return(-1);
	if (yp_master(domain, "passwd.byname", &master) != 0)
		return (-1);
	port = getrpcport(master, YPPASSWDPROG, YPPASSWDPROC_UPDATE,
		IPPROTO_UDP);
	if (port == 0) {
		free(master);
		return (-1);
	}
	if (port >= IPPORT_RESERVED) {
		free(master);
		return (-1);
	}
	ans = callrpc(master, YPPASSWDPROG, YPPASSWDVERS,
	    YPPASSWDPROC_UPDATE, xdr_yppasswd, &yppasswd, xdr_int, &ok);
	free(master);
	if (ans != 0 || ok != 0)
		return (-1);
	else
		return (0);
}

xdr_yppasswd(xdrsp, pp)
	XDR *xdrsp;
	struct yppasswd *pp;
{
	if (xdr_wrapstring(xdrsp, &pp->oldpass) == 0)
		return (0);
	if (xdr_passwd(xdrsp, &pp->newpw) == 0)
		return (0);
	return (1);
}

xdr_passwd(xdrsp, pw)
	XDR *xdrsp;
	struct passwd *pw;
{
	if (xdr_wrapstring(xdrsp, &pw->pw_name) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_passwd) == 0)
		return (0);
	if (xdr_int(xdrsp, &pw->pw_uid) == 0)
		return (0);
	if (xdr_int(xdrsp, &pw->pw_gid) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_gecos) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_dir) == 0)
		return (0);
	if (xdr_wrapstring(xdrsp, &pw->pw_shell) == 0)
		return (0);
}










