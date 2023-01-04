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
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/lib/xinput/XGetFCtl.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

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
 * XGetFeedbackControl - get the feedback attributes of an extension device.
 *
 */

#include "XI.h"
#include "XIproto.h"
#include "Xlibint.h"
#include "Xlib.h"
#include "XInput.h"
#include "extutil.h"

XFeedbackState
*XGetFeedbackControl (dpy, dev, num_feedbacks)
    register	Display 	*dpy;
    XDevice			*dev;
    int				*num_feedbacks;
    {
    int	size = 0;
    int	nbytes, i;
    XFeedbackState *Feedback = NULL;
    XFeedbackState *Sav = NULL;
    xFeedbackState *f = NULL;
    xFeedbackState *sav = NULL;
    xGetFeedbackControlReq *req;
    xGetFeedbackControlReply rep;
    XExtDisplayInfo *info = (XExtDisplayInfo *) XInput_find_display (dpy);

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XFeedbackState *) NoSuchExtension);

    GetReq(GetFeedbackControl,req);
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_GetFeedbackControl;
    req->deviceid = dev->device_id;

    if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) 
	{
	UnlockDisplay(dpy);
	SyncHandle();
	return (XFeedbackState *) NULL;
	}
    if (rep.length > 0) 
	{
	*num_feedbacks = rep.num_feedbacks;
	nbytes = (long)rep.length << 2;
	f = (xFeedbackState *) Xmalloc((unsigned) nbytes);
        if (!f)
	    {
	    _XEatData (dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (XFeedbackState *) NULL;
	    }
	sav = f;
	_XRead (dpy, (char *) f, nbytes);

	for (i=0; i<*num_feedbacks; i++)
	    {
	    switch (f->class)
		{
		case KbdFeedbackClass:
		    size += sizeof (XKbdFeedbackState);
		    break;
		case PtrFeedbackClass:
		    size += sizeof (XPtrFeedbackState);
		    break;
		case IntegerFeedbackClass:
		    size += sizeof (XIntegerFeedbackState);
		    break;
		case StringFeedbackClass:
		    {
		    xStringFeedbackState *strf = (xStringFeedbackState *) f;

		    size += sizeof (XStringFeedbackState) + 
			(strf->num_syms_supported * sizeof (KeySym));
		    }
		    break;
		case LedFeedbackClass:
		    size += sizeof (XLedFeedbackState);
		    break;
		case BellFeedbackClass:
		    size += sizeof (XBellFeedbackState);
		    break;
		default:
		    size += f->length;
		    break;
		}
	    f = (xFeedbackState *) ((char *) f + f->length);
	    }

	Feedback = (XFeedbackState *) Xmalloc((unsigned) size);
        if (!Feedback)
	    {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (XFeedbackState *) NULL;
	    }
	Sav = Feedback;

	f = sav;
	for (i=0; i<*num_feedbacks; i++)
	    {
	    switch (f->class)
		{
		case KbdFeedbackClass:
		    {
		    xKbdFeedbackState *k;
		    XKbdFeedbackState *K;
		    k = (xKbdFeedbackState *) f;
		    K = (XKbdFeedbackState *) Feedback;

		    K->class = k->class;
		    K->length = sizeof (XKbdFeedbackState);
		    K->id = k->id;
		    K->click = k->click;
		    K->percent = k->percent;
		    K->pitch = k->pitch;
		    K->duration = k->duration;
		    K->led_mask = k->led_mask;
		    K->global_auto_repeat = k->global_auto_repeat;
		    bcopy ((char *) &k->auto_repeats[0], 
			(char *) &K->auto_repeats[0], 32);
		    break;
		    }
		case PtrFeedbackClass:
		    {
		    xPtrFeedbackState *p;
		    XPtrFeedbackState *P;
		    p = (xPtrFeedbackState *) f;
		    P = (XPtrFeedbackState *) Feedback;

		    P->class = p->class;
		    P->length = sizeof (XPtrFeedbackState);
		    P->id = p->id;
		    P->accelNum = p->accelNum;
		    P->accelDenom = p->accelDenom;
		    P->threshold = p->threshold;
		    break;
		    }
		case IntegerFeedbackClass:
		    {
		    xIntegerFeedbackState *i;
		    XIntegerFeedbackState *I;
		    i = (xIntegerFeedbackState *) f;
		    I = (XIntegerFeedbackState *) Feedback;

		    I->class = i->class;
		    I->length = sizeof (XIntegerFeedbackState);
		    I->id = i->id;
		    I->resolution = i->resolution;
		    I->minVal = i->min_value;
		    I->maxVal = i->max_value;
		    break;
		    }
		case StringFeedbackClass:
		    {
		    xStringFeedbackState *s;
		    XStringFeedbackState *S;
		    s = (xStringFeedbackState *) f;
		    S = (XStringFeedbackState *) Feedback;

		    S->class = s->class;
		    S->length = sizeof (XStringFeedbackState) + 
			(s->num_syms_supported * sizeof (KeySym));
		    S->id = s->id;
		    S->max_symbols = s->max_symbols;
		    S->num_syms_supported = s->num_syms_supported;
		    S->syms_supported = (KeySym *) (S+1);
		    bcopy ((char *) (s+1), (char *) S->syms_supported,
			(S->num_syms_supported * sizeof (KeySym)));
		    break;
		    }
		case LedFeedbackClass:
		    {
		    xLedFeedbackState *l;
		    XLedFeedbackState *L;
		    l = (xLedFeedbackState *) f;
		    L = (XLedFeedbackState *) Feedback;

		    L->class = l->class;
		    L->length = sizeof (XLedFeedbackState);
		    L->id = l->id;
		    L->led_values = l->led_values;
		    L->led_mask = l->led_mask;
		    break;
		    }
		case BellFeedbackClass:
		    {
		    xBellFeedbackState *b;
		    XBellFeedbackState *B;
		    b = (xBellFeedbackState *) f;
		    B = (XBellFeedbackState *) Feedback;

		    B->class = b->class;
		    B->length = sizeof (XBellFeedbackState);
		    B->id = b->id;
		    B->percent = b->percent;
		    B->pitch = b->pitch;
		    B->duration = b->duration;
		    break;
		    }
		default:
		    break;
		}
	    f = (xFeedbackState *) ((char *) f + f->length);
	    Feedback = (XFeedbackState *) ((char *) Feedback+Feedback->length);
	    }
	XFree ((char *)sav);
	}

    UnlockDisplay(dpy);
    SyncHandle();
    return (Sav);
    }

XFreeFeedbackList (list)
    XFeedbackState *list;
    {
    XFree ((char *)list);
    }
