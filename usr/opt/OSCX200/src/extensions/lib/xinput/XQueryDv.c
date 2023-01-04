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
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/lib/xinput/XQueryDv.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

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
 * XQueryDeviceState - Query the state of an extension input device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "XInput.h"
#include "extutil.h"

XDeviceState
*XQueryDeviceState (dpy, dev)
    register Display *dpy;
    XDevice *dev;
    {       
    int				i,j;
    int				rlen;
    int				size = 0;
    xQueryDeviceStateReq 	*req;
    xQueryDeviceStateReply 	rep;
    XDeviceState		*state = NULL;
    XInputClass			*any, *Any;
    char			*data;
    XExtDisplayInfo *info = (XExtDisplayInfo *) XInput_find_display (dpy);

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XDeviceState *) NoSuchExtension);

    GetReq(QueryDeviceState,req);		
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_QueryDeviceState;
    req->deviceid = dev->device_id;

    if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) 
	{
	UnlockDisplay(dpy);
	SyncHandle();
	return (XDeviceState *) NULL;
	}

    rlen = rep.length << 2;
    if (rlen > 0)
	{
	data = Xmalloc (rlen);
	if (!data)
	    {
	    _XEatData (dpy, (unsigned long) rlen);
    	    UnlockDisplay(dpy);
    	    SyncHandle();
    	    return ((XDeviceState *) NULL);
	    }
	_XRead (dpy, data, rlen);

	for (i=0, any=(XInputClass *) data; i<(int)rep.num_classes; i++)
	    {
	    switch (any->class)
		{
		case KeyClass:
		    size += sizeof (XKeyState);
		    break;
		case ButtonClass:
		    size += sizeof (XButtonState);
		    break;
		case ValuatorClass:
		    {
		    xValuatorState *v = (xValuatorState *) any;
		    size += (sizeof (XValuatorState) + 
			(v->num_valuators * sizeof(int)));
		    }
		    break;
		}
	    any = (XInputClass *) ((char *) any + any->length);
	    }
	state = (XDeviceState *) Xmalloc (size + sizeof(XDeviceState));
	if (!state)
	    {
    	    UnlockDisplay(dpy);
    	    SyncHandle();
    	    return ((XDeviceState *) NULL);
	    }
	state->device_id = dev->device_id;
	state->num_classes = rep.num_classes;
	state->data = (XInputClass *) (state + 1);

	Any = state->data; 
	for (i=0, any=(XInputClass *) data; i<(int)rep.num_classes; i++)
	    {
	    switch (any->class)
		{
		case KeyClass:
		    {
		    xKeyState *k = (xKeyState *) any;
		    XKeyState *K = (XKeyState *) Any;
		    K->class = k->class;
		    K->length = sizeof (XKeyState);
		    K->num_keys = k->num_keys;
		    bcopy ((char *) &k->keys[0], (char *) &K->keys[0], 32);
		    Any = (XInputClass *) (K+1);
		    }
		    break;
		case ButtonClass:
		    {
		    xButtonState *b = (xButtonState *) any;
		    XButtonState *B = (XButtonState *) Any;
		    B->class = b->class;
		    B->length = sizeof (XButtonState);
		    B->num_buttons = b->num_buttons;
		    bcopy ((char *) &b->buttons[0], (char *) &B->buttons[0], 
			32);
		    Any = (XInputClass *) (B+1);
		    }
		    break;
		case ValuatorClass:
		    {
		    xValuatorState *v = (xValuatorState *) any;
		    XValuatorState *V = (XValuatorState *) Any;
		    CARD32 *valuators = (CARD32 *) (v+1);
		    V->class = v->class;
		    V->length = sizeof (XValuatorState);
		    V->num_valuators = v->num_valuators;
		    V->mode = v->mode;
		    Any = (XInputClass *) (V+1);
		    V->valuators = (int *) Any;
		    for (j=0; j<(int)V->num_valuators; j++)
			*(V->valuators + j) = *valuators++;
		    Any = (XInputClass *)((char *) Any + 
			V->num_valuators * sizeof (int));
		    }
		    break;
		}
	    any = (XInputClass *) ((char *) any + any->length);
	    }
	Xfree(data);
	}

    UnlockDisplay(dpy);
    SyncHandle();
    return (state);
    }

XFreeDeviceState (list)
    XDeviceState *list;
    {
    XFree ((char *)list);
    }
