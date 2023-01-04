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
/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/include/smt.h,v 1.1.2.4 92/07/21 07:22:58 Jim_Ludwig Exp $
 *  smt.h
 *	Shared Memory Transport definitions
 */


#ifndef _SMT_H_
#define _SMT_H_

#if defined(SMT_DEBUG)
#define SMT_DIAG(s) s
#else
#define SMT_DIAG(s)
#endif /* SMT_DEBUG */

#define SMT_PROTOCOL_NAME "Shared-Memory Transport"

/*
 * Requests
 */
#define X_SmtQueryVersion		0
#define X_SmtAttach			1
#define X_SmtWakeupServer		2

/*
 * event stuff
 */
#define XSmtWakeupNotifyMask	0x02000000

#define XSmtWakeupNotify	0
#define XSmtNumberOfEvents	(XSmtWakeupNotify + 1)

#define XSmtBadSegment		0
#define XSmtNumberOfErrors	(XSmtBadSegment + 1)

#define SMT_REQ		(1 << 0)
#define SMT_REP		(1 << 1)
#define SMT_NEED_WAKEUP (1 << 2)
#define SMT_NEWVER      (1 << 3)

#ifndef _SMT_SERVER_
/*
 * Extra definitions that will only be needed in the client
 */
typedef XID XSmt;

extern Bool XSmtQueryVersion();	/* is extension on server? */
extern Bool XSmtAttach();	/* attach shared memory transport */
extern Bool XWakeupServer();	/* wakeup server */

typedef struct {
    int	type;		    /* of event */
    unsigned long serial;   /* # of last request processed by server */
    int send_event;	    /* true if this came frome a SendEvent request */
    struct _XDisplay  *display;	    /* Display the event was read from */
} XSmtWakeupEvent;

#endif /* _SMT_SERVER_ */
#endif /* _SMT_H_ */
