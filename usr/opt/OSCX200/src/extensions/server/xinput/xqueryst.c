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
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/server/xinput/xqueryst.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

/***********************************************************************
 *
 * Request to query the state of an extension input device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "windowstr.h"			/* window structure  */
#include "XI.h"
#include "XIproto.h"

extern	int 		IReqCode;
extern	int 		BadDevice;
extern	void		(* ReplySwapVector[256]) ();
DeviceIntPtr		LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure allows a client to query the state of a device.
 *
 */

int
SProcXQueryDeviceState(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xQueryDeviceStateReq);
    swaps(&stuff->length, n);
    return(ProcXQueryDeviceState(client));
    }

/***********************************************************************
 *
 * This procedure allows frozen events to be routed.
 *
 */

int
ProcXQueryDeviceState(client)
    register ClientPtr client;
    {
    register char 		n;
    int 			i;
    int 			num_classes = 0;
    int 			total_length = 0;
    char			*buf, *savbuf;
    KeyClassPtr 		k;
    xKeyState			*tk;
    ButtonClassPtr 		b;
    xButtonState		*tb;
    ValuatorClassPtr 		v;
    xValuatorState		*tv;
    xQueryDeviceStateReply	rep;
    DeviceIntPtr		dev;
    int				*values;

    REQUEST(xQueryDeviceStateReq);
    REQUEST_SIZE_MATCH(xQueryDeviceStateReq);

    rep.repType = X_Reply;
    rep.RepType = X_QueryDeviceState;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	SendErrorToClient(client, IReqCode, X_QueryDeviceState, 0, 
		BadDevice);
	return Success;
	}


    k = dev->key;
    if (k != NULL)
	{
	total_length += sizeof (xKeyState);
	num_classes++;
	}

    b = dev->button;
    if (b != NULL)
	{
	total_length += sizeof (xButtonState);
	num_classes++;
	}

    v = dev->valuator;
    if (v != NULL)
	{
	total_length += (sizeof(xValuatorState) + 
			(v->numAxes * sizeof(int)));
	num_classes++;
	}
    buf = (char *) Xalloc (total_length);
    if (!buf)
	{
	SendErrorToClient(client, IReqCode, X_QueryDeviceState, 0, 
		BadAlloc);
	return Success;
	}
    savbuf = buf;

    if (k != NULL)
	{
	tk = (xKeyState *) buf;
	tk->class = KeyClass;
	tk->length = sizeof (xKeyState);
	tk->num_keys = k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode + 1;
	for (i = 0; i<32; i++)
	    tk->keys[i] = k->down[i];
	buf += sizeof (xKeyState);
	}

    if (b != NULL)
	{
	tb = (xButtonState *) buf;
	tb->class = ButtonClass;
	tb->length = sizeof (xButtonState);
	tb->num_buttons = b->numButtons;
	for (i = 0; i<32; i++)
	    tb->buttons[i] = b->down[i];
	buf += sizeof (xButtonState);
	}

    if (v != NULL)
	{
	tv = (xValuatorState *) buf;
	tv->class = ValuatorClass;
	tv->length = sizeof (xValuatorState);
	tv->num_valuators = v->numAxes;
	tv->mode = v->mode;
	buf += sizeof(xValuatorState);
	for (i=0, values=v->axisVal; i<v->numAxes; i++)
	    {
	    *((int *) buf) = *values++;
	    if (client->swapped)
		{
		swapl ((int *) buf, n);/* macro - braces needed */
		}
	    buf += sizeof(int);
	    }
	}

    rep.num_classes = num_classes;
    rep.length = (total_length + 3) >> 2;
    WriteReplyToClient (client, sizeof(xQueryDeviceStateReply), &rep);
    if (total_length > 0)
	WriteToClient (client, total_length, savbuf);
    Xfree (savbuf);
    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XQueryDeviceState function,
 * if the client and server have a different byte ordering.
 *
 */

SRepXQueryDeviceState (client, size, rep)
    ClientPtr	client;
    int		size;
    xQueryDeviceStateReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, rep);
    }
