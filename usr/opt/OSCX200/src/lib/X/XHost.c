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
/* $XConsortium: XHost.c,v 11.11 91/01/06 11:46:28 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/* this might be rightly reguarded an os dependent file */

#include "Xlibint.h"

XAddHost (dpy, host)
    register Display *dpy;
    XHostAddress *host;
    {
    register xChangeHostsReq *req;
    register int length = (host->length + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostInsert;
    req->hostFamily = host->family;
    req->hostLength = host->length;
    bcopy (host->address, (char *) NEXTPTR(req,xChangeHostsReq), host->length);
    UnlockDisplay(dpy);
    SyncHandle();
    }

XRemoveHost (dpy, host)
    register Display *dpy;
    XHostAddress *host;
    {
    register xChangeHostsReq *req;
    register int length = (host->length + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostDelete;
    req->hostFamily = host->family;
    req->hostLength = host->length;
    bcopy (host->address, (char *) NEXTPTR(req,xChangeHostsReq), host->length);
    UnlockDisplay(dpy);
    SyncHandle();
    }


XAddHosts (dpy, hosts, n)
    register Display *dpy;
    XHostAddress *hosts;
    int n;
{
    register int i;
    for (i = 0; i < n; i++) {
	XAddHost(dpy, &hosts[i]);
      }
}

XRemoveHosts (dpy, hosts, n)
    register Display *dpy;
    XHostAddress *hosts;
    int n;
{
    register int i;
    for (i = 0; i < n; i++) {
	XRemoveHost(dpy, &hosts[i]);
      }
}
