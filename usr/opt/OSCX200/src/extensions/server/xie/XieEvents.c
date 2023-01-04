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

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of DIX procedures for event
**	service requests.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Bernardo Tagariello
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      March 23, 1990
**
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
/*
**  Core X Includes
*/
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>
/*
**  XIE Includes
*/
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>

/*
**  Table of contents
*/
    /*
    **  Xie protocol proceedures called from XieMain
    */
int	ProcQueryEvents();
int	ProcSelectEvents();
    /*
    **  routines referenced by other modules
    */
int	xieDoSendEvent();
    /*
    **  routines used internally by XieEvents
    */

/*
**  Equated Symbols
*/

/*
**  MACRO definitions
*/

/*
**  External References
*/
externalref ClientTableRec  xieClientTable[MAXCLIENTS];

extern CARD32		    xieClients;	    /* num of clients served	    */
extern CARD32		    xieEventBase;   /* Base for Xie events          */
extern CARD32		    xieReqCode;	    /* XIE main opcode		    */
#if   defined(X11R3) && !defined(DWV3)
extern CARD16		    RC_xie;	    /* XIE Class		    */
extern CARD16		    RT_photo;	    /* Photo{flo|map|tap} resource  */
extern CARD16		    RT_idc;	    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
extern RESTYPE		    RC_xie;	    /* XIE Class		    */
extern RESTYPE		    RT_photo;	    /* Photo{flo|map|tap} resource  */
extern RESTYPE		    RT_idc;	    /* Cpp, Roi & Tmp resource types*/
#endif

externalref int (*xieDixSwap[X_ieLastRequest])(); /* Reply swap dispatch
						   * tables */
externalref int (*xieDixSwapReply[])();
void WriteEventToClient();

/*
**	Local Storage
*/

/*-----------------------------------------------------------------------
-------------------------  QueryEvents Procedure -------------------------
------------------------------------------------------------------------*/
int ProcQueryEvents( client )
 ClientPtr client;	/* client pointer				    */
{
    xieQueryEventsReply	rep;
    REQUEST( xieQueryEventsReq);
    REQUEST_SIZE_MATCH( xieQueryEventsReq );

    /*
    **	Build reply.
    */
    rep.type	       = X_Reply;
    rep.length	       = 0;
    rep.sequenceNumber = client->sequence;
    rep.event_mask     = xieClientTable[client->index].event_mask;

    WriteXieReplyToClient( client, sizeof(xieQueryEventsReply), &rep );

    return(Success);
}			    /* end ProcQueryEvents */

/*-----------------------------------------------------------------------
-------------------------  SelectEvents Procedure ------------------------
------------------------------------------------------------------------*/
int ProcSelectEvents( client )
 ClientPtr client;	/* client pointer				    */
{
    REQUEST( xieSelectEventsReq );
    REQUEST_SIZE_MATCH( xieSelectEventsReq );

    /*
    **	Stash new event mask.
    */
    xieClientTable[client->index].event_mask = stuff->event_mask;

    return(Success);
}				    /* end ProcSelectEvents */

/*-----------------------------------------------------------------------
-----------------------  routine:  Send an Event  -----------------------
------------------------------------------------------------------------*/
int xieDoSendEvent( client, res, type )
 ClientPtr	client;	    /* client pointer				    */
 CommonPartPtr  res;	    /* resource pointer				    */
 CARD8		type;	    /* event type				    */
{
    PhotomapPtr  img;
    xieSendEvent event;
    REQUEST( xieReq );

    if( type == XieK_ComputationEvent )
	{   /*
	    **  Remember that this sink is full of image data.
	    */
	img			    = (PhotomapPtr)res;
	SnkFull_(Sink_(img))	    =  TRUE;
	SnkUdpFinalMsk_(Sink_(img)) = (1<<CmpCnt_(img))-1;
	UtilSetPending( img, 0, XieK_BandByPixel, FALSE );
	}

    if( xieClientTable[client->index].event_mask & 1<<type )
	{   /*
	    **	Set up some kind of completion event and send it to the client.
	    */
	event.type	     = xieEventBase + type;
	event.detail	     = 0;
	event.sequenceNumber = client->sequence;
	event.resource_id    = ResId_(res);
	WriteEventToClient( client, sizeof(xieSendEvent), &event );
	}

    return(Success);
}			/* End of xieDoSendEvent */
/* end module XieEvents.c */
