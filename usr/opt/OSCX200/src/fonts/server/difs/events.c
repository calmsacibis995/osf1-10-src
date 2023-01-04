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
/* $XConsortium: events.c,v 1.4 92/11/18 21:30:09 gildea Exp $ */
/*
 * event handling stuff
 */
/*
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation and the
 * Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, Digital or
 * M.I.T. not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, DIGITAL AND M.I.T. DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * DIGITAL OR M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#include	"clientstr.h"
#include	"FSproto.h"
#include	"globals.h"
#include	"events.h"

extern void (*EventSwapVector[NUM_EVENT_VECTORS]) ();

static Mask lastEventMask = FontChangeNotifyMask;

#define	AllEventMasks	(lastEventMask | (lastEventMask - 1))

void
WriteErrorToClient(client, error)
    ClientPtr   client;
    fsError    *error;
{
    if (client->swapped) {
	fsError     errorTo;

	SErrorEvent(error, &errorTo);
	(void) WriteToClient(client, SIZEOF(fsError), (char *) &errorTo);
    } else {
	(void) WriteToClient(client, SIZEOF(fsError),
			     (char *) error);
    }
}

int
ProcSetEventMask(client)
    ClientPtr   client;
{
    REQUEST(fsSetEventMaskReq);
    REQUEST_AT_LEAST_SIZE(fsSetEventMaskReq);

    if (stuff->event_mask & ~AllEventMasks) {
	SendErrToClient(client, FSBadEventMask, (pointer) stuff->event_mask);
	return FSBadEventMask;
    }
    client->eventmask = stuff->event_mask;
    return client->noClientException;
}

int
ProcGetEventMask(client)
    ClientPtr   client;
{
    fsGetEventMaskReply rep;

    REQUEST(fsGetEventMaskReq);
    REQUEST_AT_LEAST_SIZE(fsGetEventMaskReq);

    rep.type = FS_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = SIZEOF(fsGetEventMaskReply) >> 2;
    rep.event_mask = client->eventmask;

    return client->noClientException;
}

void
SendKeepAliveEvent(client)
    ClientPtr   client;
{
    fsKeepAliveEvent ev;

    ev.type = FS_Event;
    ev.event_code = KeepAlive;
    ev.sequenceNumber = client->sequence;
    ev.length = SIZEOF(fsKeepAliveEvent) >> 2;
    ev.timestamp = GetTimeInMillis();

#ifdef DEBUG
    fprintf(stderr, "client #%d is getting a KeepAlive\n", client->index);
#endif

    if (client->swapped) {
	fsKeepAliveEvent evTo;

	SErrorEvent((fsError *) & ev, (fsError *) & evTo);
	(void) WriteToClient(client, SIZEOF(fsKeepAliveEvent), (char *) &evTo);
    } else {
	(void) WriteToClient(client, SIZEOF(fsKeepAliveEvent), (char *) &ev);
    }
}
