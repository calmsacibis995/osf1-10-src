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

/*******************************************************************************
**  Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**
**                          All Rights Reserved
**
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**
*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension (XIE)
**
**  ABSTRACT:
**
**      This module contains XIE library routines for EVENTS.
**	
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**	Bernardo Tagariello
**
**  CREATION DATE:
**
**      March 23, 1990
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Definitions required by X11 include files
*/
#define NEED_EVENTS
#define NEED_REPLIES

/*
**  Definition to let include files know who's calling.
*/
#define _XieLibEvents

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <stdio.h>
    /*
    **  X11 and XIE include files
    */
#ifdef VMS
#include <Xlibint.h>			/* X11 internal lib/transport defs  */
#else
#include <X11/Xlibint.h>		/* X11 internal lib/transport defs  */
#endif
#include <XieProto.h>			/* XIE Req/Reply protocol defs	    */
#include <XieLib.h>			/* XIE public  definitions	    */
#include <XieLibint.h>			/* XIE private definitions	    */

/*
**  Table of contents
*/
unsigned long	XieQueryEvents();
void		XieSelectEvents();
    /*
    **  Low level global entry points
    */
Bool	_XieConvertEventProc();
    /*
    **  internal routines
    */

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**  Local Storage
*/

/*******************************************************************************
**  XieQueryEvents
**
**  FUNCTIONAL DESCRIPTION:
**
**	Retreives the event mask for Xie events enabled on the Xie server.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**
**  FUNCTION VALUE:
**
**	value of current Xie events mask
**
*******************************************************************************/
unsigned long XieQueryEvents(dpy)
 Display *dpy;
{
    xieQueryEventsReply  rep;
    xieQueryEventsReq   *req;
    XieSessionPtr ses = _XieGetSession(dpy);
    Status    status;

    /*
    **  Create request to query events
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieQueryEvents,req);

    status = _XReply( dpy, (xReply *) &rep, 0, True );

    if( !status )
	_XieSrvError( dpy, req->opcode, 0, rep.sequenceNumber,
		      BadAccess, "QueryEvents" );
    else
	XieReqDone_(dpy);

    return( rep.event_mask );
}				    /* end XieQueryEvents */

/*******************************************************************************
**  XieSelectEvents
**
**  FUNCTIONAL DESCRIPTION:
**
**	Sets the event mask to enabled specified Xie events on the Xie server.
**
**  FORMAL PARAMETERS:
**
**	dpy	    - pointer to X11 display structure
**	event_mask  - mask of events to enable
**
*******************************************************************************/
void XieSelectEvents(dpy, event_mask)
 Display       *dpy;
 unsigned long	event_mask;
{
    Status status;
    xieSelectEventsReq   *req;
    XieSessionPtr ses = _XieGetSession(dpy);

    /*
    **  Create request to Select Events.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieSelectEvents,req);
    req->event_mask = event_mask;
    XieReqDone_(dpy);

}				    /* end XieSelectEvents */

/*******************************************************************************
**  _XieConvertEventProc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Convert an event from wire format to client format.
**
**  FORMAL PARAMETERS:
**
**	dpy	- X11 display pointer
**	ce	- pointer to client event structure to be filled in
**	we	- pointer to wire   event structure to be converted
**
**  FUNCTION VALUE:
**
**	Boolean: true if event should be queued
**
*******************************************************************************/
Bool _XieConvertEventProc(dpy, ce, we)
 Display *dpy;
 XEvent  *ce;
 xEvent  *we;
{
    xieSendEvent *wireevent   = (xieSendEvent *) we;
    XieEventRec  *clientevent = (XieEventRec  *) ce;

    clientevent->type	      = wireevent->type & ~0x80;
    clientevent->send_event   =(wireevent->type &  0x80) == True;
    clientevent->serial	      = wireevent->sequenceNumber;
    clientevent->display      = dpy;
    clientevent->resource     = XieFindResource(dpy, wireevent->resource_id);

    return( clientevent->resource != NULL );
}				/* end _XieConvertEventProc */
/* end module XieLibEvents.c */
