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
#ifdef XINPUT
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/server/xinput/xdevbell.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

/************************************************************
Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, California, and the 
Massachusetts Institute of Technology, Cambridge, Massachusetts.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Hewlett-Packard or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * Extension function to change the keyboard device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"

extern	int 		IReqCode;
extern	int 		BadDevice;
extern	void		(* ReplySwapVector[256]) ();
DeviceIntPtr		LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure is invoked to swap the request bytes if the server and
 * client have a different byte order.
 *
 */

int
SProcXDeviceBell(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xDeviceBellReq);
    swaps(&stuff->length, n);
    return(ProcXDeviceBell(client));
    }

/***********************************************************************
 *
 * This procedure rings a bell on an extension device.
 *
 */

ProcXDeviceBell (client)
    register ClientPtr client;
    {
    DeviceIntPtr dev;
    KbdFeedbackPtr k;
    BellFeedbackPtr b;
    int base;
    int newpercent;
    CARD8 class;
    pointer ctrl;
    void (*proc)();

    REQUEST(xDeviceBellReq);
    REQUEST_SIZE_MATCH(xDeviceBellReq);

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	client->errorValue = stuff->deviceid;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadDevice);
	return Success;
	}

    if (stuff->percent < -100 || stuff->percent > 100)
	{
	client->errorValue = stuff->percent;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	return Success;
	}
    if (stuff->feedbackclass == KbdFeedbackClass)
	{
	for (k=dev->kbdfeed; k; k=k->next)
	    if (k->ctrl.id == stuff->feedbackid)
		break;
	if (!k)
	    {
	    client->errorValue = stuff->feedbackid;
	    SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	    return Success;
	    }
	base = k->ctrl.bell;
	proc = k->BellProc;
	ctrl = (pointer) &(k->ctrl);
	class = KbdFeedbackClass;
	}
    else if (stuff->feedbackclass == BellFeedbackClass)
	{
	for (b=dev->bell; b; b=b->next)
	    if (b->ctrl.id == stuff->feedbackid)
		break;
	if (!b)
	    {
	    client->errorValue = stuff->feedbackid;
	    SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	    return Success;
	    }
	base = b->ctrl.percent;
	proc = b->BellProc;
	ctrl = (pointer) &(b->ctrl);
	class = BellFeedbackClass;
	}
    else
	{
	client->errorValue = stuff->feedbackclass;
	SendErrorToClient(client, IReqCode, X_DeviceBell, 0, BadValue);
	return Success;
	}
    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;
    (*proc)(newpercent, dev, ctrl, class);

    return Success;
    }

#endif /* XINPUT */
