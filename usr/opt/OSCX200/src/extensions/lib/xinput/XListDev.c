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
/* $Header: /usr/sde/x11/rcs/x11/src/./extensions/lib/xinput/XListDev.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

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
 * XListInputDevices - Request the server to return a list of 
 *			 available input devices.
 *
 */

#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "Xlibint.h"
#include "XIproto.h"
#include "XI.h"
#include "XInput.h"
#include "extutil.h"

XDeviceInfo 
*XListInputDevices(dpy, ndevices)
    register Display *dpy;
    int *ndevices;
    {	
    int				size;
    xListInputDevicesReq 	*req;
    xListInputDevicesReply	rep;
    xDeviceInfo 		*list, *slist;
    XDeviceInfo 		*sclist;
    XDeviceInfo 		*clist = NULL;
    xAnyClassPtr 		any, sav_any;
    XAnyClassPtr 		Any;
    char			*nptr, *Nptr;
    register int 		i,j,k;
    register long 		rlen;
    XExtDisplayInfo *info = (XExtDisplayInfo *) XInput_find_display (dpy);

    LockDisplay (dpy);
    if (CheckExtInit(dpy, XInput_Initial_Release) == -1)
	return ((XDeviceInfo *) NoSuchExtension);

    GetReq(ListInputDevices,req);		
    req->reqType = info->codes->major_opcode;
    req->ReqType = X_ListInputDevices;

    if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) 
	{
	UnlockDisplay(dpy);
	SyncHandle();
	return (XDeviceInfo *) NULL;
	}

    if (*ndevices = rep.ndevices) /* at least 1 input device */
	{
	size = *ndevices * sizeof (XDeviceInfo);
	rlen = rep.length << 2;   /* multiply length by 4    */
	list = (xDeviceInfo *) Xmalloc (rlen);
	slist = list;
        if (!slist)
	    {
	    _XEatData (dpy, (unsigned long) rlen);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (XDeviceInfo *) NULL;
	    }
	_XRead (dpy, list, rlen);

	any = (xAnyClassPtr) ((char *) list + 
		(*ndevices * sizeof(xDeviceInfo)));
	sav_any = any;
	for (i=0; i<*ndevices; i++, list++) 
	    {
	    for (j=0; j<(int)list->num_classes; j++)
		{
		switch (any->class)
		    {
		    case KeyClass:
			size += sizeof (XKeyInfo);
			break;
		    case ButtonClass:
			size += sizeof (XButtonInfo);
			break;
		    case ValuatorClass:
			{
			xValuatorInfoPtr v;
			v = (xValuatorInfoPtr) any;
			size += sizeof (XValuatorInfo) + 
				(v->num_axes * sizeof (XAxisInfo));
			break;
			}
		    default:
			break;
		    }
		any = (xAnyClassPtr) ((char *) any + any->length); 
		}
	    }

	for (i=0, nptr = (char *) any; i<*ndevices; i++) 
	    {
	    size += *nptr +1;
	    nptr += (*nptr + 1);
	    }

	clist = (XDeviceInfoPtr) Xmalloc (size);
        if (!clist)
	    {
	    XFree ((char *)slist);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (XDeviceInfo *) NULL;
	    }
	sclist = clist;
	Any = (XAnyClassPtr) ((char *) clist + 
		(*ndevices * sizeof (XDeviceInfo)));
	list = slist;
	any = sav_any;
	for (i=0; i<*ndevices; i++, list++, clist++) 
	    {
	    clist->type	= list->type;
	    clist->id = list->id;
	    clist->use = list->use;
	    clist->num_classes = list->num_classes;
	    clist->inputclassinfo = Any;
	    for (j=0; j<(int)list->num_classes; j++)
		{
		switch (any->class)
		    {
		    case KeyClass:
			{
			XKeyInfoPtr K = (XKeyInfoPtr) Any;
			xKeyInfoPtr k = (xKeyInfoPtr) any;
			K->class = KeyClass;
			K->length = sizeof (XKeyInfo);
			K->min_keycode = k->min_keycode;
			K->max_keycode = k->max_keycode;
			K->num_keys = k->num_keys;
			break;
			}
		    case ButtonClass:
			{
			XButtonInfoPtr B = (XButtonInfoPtr) Any;
			xButtonInfoPtr b = (xButtonInfoPtr) any;
			B->class = ButtonClass;
			B->length = sizeof (XButtonInfo);
			B->num_buttons = b->num_buttons;
			break;
			}
		    case ValuatorClass:
			{
			XValuatorInfoPtr V = (XValuatorInfoPtr) Any;
			xValuatorInfoPtr v = (xValuatorInfoPtr) any;
			XAxisInfoPtr A;
			xAxisInfoPtr a;

			V->class = ValuatorClass;
			V->length = sizeof (XValuatorInfo) +
				(v->num_axes * sizeof (XAxisInfo));
			V->num_axes = v->num_axes;
			V->motion_buffer = v->motion_buffer_size;
			V->mode = v->mode;
			A = (XAxisInfoPtr) ((char *) V + sizeof(XValuatorInfo));
			V->axes = A;
			a = (xAxisInfoPtr) ((char *) any + 
				sizeof (xValuatorInfo)); 
			for (k=0; k<(int)v->num_axes; k++,a++,A++) 
			    {
			    A->min_value = a->min_value;
			    A->max_value = a->max_value;
			    A->resolution = a->resolution;
			    }
			break;
			}
		    default:
			break;
		    }
		any = (xAnyClassPtr) ((char *) any + any->length); 
		Any = (XAnyClassPtr) ((char *) Any + Any->length); 
		}
	    }

	clist = sclist;
	nptr = (char *) any;
	Nptr = (char *) Any;
	for (i=0; i<*ndevices; i++,clist++) 
	    {
	    clist->name = (char *) Nptr;
	    bcopy ( nptr+1, Nptr, *nptr);
	    Nptr += (*nptr);
	    *Nptr++ = '\0';
	    nptr += (*nptr + 1);
	    }
	}

    XFree ((char *)slist);
    UnlockDisplay(dpy);
    SyncHandle();
    return (sclist);
    }

/***********************************************************************
 *
 * Free the list of input devices.
 *
 */

XFreeDeviceList (list)
    XDeviceInfo *list;
    {
    if (list != NULL) 
	{
        XFree ((char *) list);
        }
    }
