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
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/server/xinput/xchgprop.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

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
 * Function to modify the dont-propagate-list for an extension input device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "windowstr.h"
#include "XI.h"
#include "XIproto.h"

extern	int 	BadMode;
extern	int 	BadClass;
extern	int 	IReqCode;
DeviceIntPtr	LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure returns the extension version.
 *
 */

int
SProcXChangeDeviceDontPropagateList(client)
    register ClientPtr client;
    {
    register char n;
    register long *p;
    register int i;

    REQUEST(xChangeDeviceDontPropagateListReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->count, n);
    p = (long *) &stuff[1];
    for (i=0; i<stuff->count; i++)
        {
        swapl(p, n);
	p++;
        }
    return(ProcXChangeDeviceDontPropagateList(client));
    }

/***********************************************************************
 *
 * This procedure changes the dont-propagate list for the specified window.
 *
 */

ProcXChangeDeviceDontPropagateList (client)
    register ClientPtr client;
    {
    int			i;
    WindowPtr		pWin;
    struct 		tmask tmp[EMASKSIZE];
    OtherInputMasks	*others;

    REQUEST(xChangeDeviceDontPropagateListReq);
    REQUEST_AT_LEAST_SIZE(xChangeDeviceDontPropagateListReq);

    if (stuff->length !=(sizeof(xChangeDeviceDontPropagateListReq)>>2) + 
	stuff->count)
	{
	SendErrorToClient (client, IReqCode, X_ChangeDeviceDontPropagateList, 0,
	    BadLength);
	return Success;
	}

    pWin = (WindowPtr) LookupWindow (stuff->window, client);
    if (!pWin)
        {
	client->errorValue = stuff->window;
	SendErrorToClient(client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadWindow);
	return Success;
        }

    if (stuff->mode != AddToList && stuff->mode != DeleteFromList)
        {
	client->errorValue = stuff->window;
	SendErrorToClient(client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadMode);
	return Success;
        }

    if (CreateMaskFromList (client, (XEventClass *)&stuff[1], 
	stuff->count, tmp, NULL, X_ChangeDeviceDontPropagateList) != Success)
	    return Success;

    others = wOtherInputMasks(pWin);
    if (!others && stuff->mode == DeleteFromList)
	return Success;
    for (i=0; i<EMASKSIZE; i++)
	{
	if (tmp[i].mask == 0)
	    continue;

	if (stuff->mode == DeleteFromList)
	    tmp[i].mask = (others->dontPropagateMask[i] & ~tmp[i].mask);
	else if (others)
	    tmp[i].mask |= others->dontPropagateMask[i];

	if (DeviceEventSuppressForWindow (pWin,client,tmp[i].mask,i) != Success)
	    {
	    SendErrorToClient ( client, IReqCode, X_ChangeDeviceDontPropagateList, 0, 
		BadClass);
	    return Success;
	    }
	}

    return Success;
    }
