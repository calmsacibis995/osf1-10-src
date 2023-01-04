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
static char rcsid[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smtutil.c,v 1.1.2.5 92/09/25 18:08:05 Jim_Ludwig Exp $";
#endif

/***********************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/smt/smtutil.c,v 1.1.2.5 92/09/25 18:08:05 Jim_Ludwig Exp $ */

/*
 * Shared-memory transport utility routines.  This code
 * is O.S. dependent.
 */

#define NEED_REPLIES
#define NEED_EVENTS
#include <stdio.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#define _SMT_SERVER_
#include "smtstr.h"
#include "os.h"
#include "osdep.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
/* #include "resourcest.h" */
#include "opaque.h"
#include "inputstr.h"
#include "regionstr.h"
#include "gcstruct.h"
#include <sys/types.h>
#include <sys/socket.h>

Bool
SmtLocalConnection(client)
ClientPtr client;
{
    int fd = ((OsCommPtr)client->osPrivate)->fd;
    int len;
    struct sockaddr name;

    len = sizeof(struct sockaddr);
    if (getsockname(fd, &name, &len) != 0)
	return FALSE;
    return (((char)name.sa_family == AF_UNIX) ? TRUE : FALSE);

} /* end SmtLocalConnection() */
