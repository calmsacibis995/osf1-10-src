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
/****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*****************************************************************************/
/*
 *  ABSTRACT:
 *
 *      This module is the device independent module responsible for all
 *      routines required for proper communication in a heterogeneous
 *      networking environment (i.e. client & server on different endian
 *      machines).  The bulk of this module is patterned after X11/R4's
 *      server/dix/swapreq.c ; however, they infact swap fields
 *      in the exact opposite order since XTrap requires "unswapped" data
 *      to become "swapped" before sending it to a "swapped" client.
 *
 *  CONTRIBUTORS:
 *
 *      Ken Miller
 *      Marc Evans
 *
 */
#ifndef lint
static char RCSID[] = "$Header: /alphabits/u3/x11/ode/rcs/x11/src/extensions/server/xtrap/xtrapdiswp.c,v 1.1.2.4 92/11/24 10:34:52 Jim_Ludwig Exp $";
#endif

#include "X.h"
#define NEED_REPLIES
#define NEED_EVENTS
#include "Xproto.h"
#include "Xprotostr.h"
#include "xtrapdi.h"
#include "input.h"          /* Server DevicePtr definitions */
#include "misc.h"
#include "dixstruct.h"
#include "xtrapddmi.h"      /* has to be after misc.h & dixstruct.h for U*IX */
#include "xtrapproto.h"


#undef LengthRestB
#undef LengthRestS
#undef LengthRestL
#define LengthRestB(stuff) \
    ((stuff->length << 2) - sizeof(*stuff))

#define LengthRestS(stuff) \
    ((stuff->length << 1) - (sizeof(*stuff) >> 1))

#define LengthRestL(stuff) \
    (stuff->length - (sizeof(*stuff) >> 2))


extern void_function EventSwapVector[128L];  /* for SendEvent */

/* In-coming XTrap requests needing to be swapped to native format */

#ifdef FUNCTION_PROTOS
int sXETrapReset(xXTrapReq *request, ClientPtr client)
#else
int sXETrapReset(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapReset(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapGetAvailable(xXTrapGetReq *request, ClientPtr client)
#else
int sXETrapGetAvailable(request,client)
    xXTrapGetReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->protocol),n);
    return(XETrapGetAvailable(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapConfig(xXTrapConfigReq *request, ClientPtr client)
#else
int sXETrapConfig(request,client)
    xXTrapConfigReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->config_max_pkt_size),n);
    return(XETrapConfig(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapStartTrap(xXTrapReq *request, ClientPtr client)
#else
int sXETrapStartTrap(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapStartTrap(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapStopTrap(xXTrapReq *request, ClientPtr client)
#else
int sXETrapStopTrap(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapStopTrap(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapGetCurrent(xXTrapReq *request, ClientPtr client)
#else
int sXETrapGetCurrent(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetCurrent(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapGetStatistics(xXTrapReq *request, ClientPtr client)
#else
int sXETrapGetStatistics(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetStatistics(request,client));
}

#ifndef _XINPUT
#ifdef FUNCTION_PROTOS
int sXETrapSimulateXEvent(xXTrapInputReq *request, ClientPtr client)
#else
int sXETrapSimulateXEvent(request,client)
    xXTrapInputReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->input.x),n);
    swaps(&(request->input.y),n);
    return(XETrapSimulateXEvent(request,client));
}
#endif

#ifdef FUNCTION_PROTOS
int sXETrapGetVersion(xXTrapGetReq *request, ClientPtr client)
#else
int sXETrapGetVersion(request,client)
    xXTrapGetReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->protocol),n);
    return(XETrapGetVersion(request,client));
}

#ifdef FUNCTION_PROTOS
int sXETrapGetLastInpTime(xXTrapReq *request, ClientPtr client)
#else
int sXETrapGetLastInpTime(request,client)
    xXTrapReq *request;
    ClientPtr client;
#endif
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetLastInpTime(request,client));
}


/* Out-going XTrap replies needing to be swapped *from* native format */

#ifdef FUNCTION_PROTOS
void sReplyXETrapGetAvail(ClientPtr client, int size, char *reply)
#else
void sReplyXETrapGetAvail(client, size, reply)
    ClientPtr client;
    int       size;
    char      *reply;
#endif
{
    xXTrapGetAvailReply *rep = (xXTrapGetAvailReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swapl(&(rep->data.pf_ident),n);
    swaps(&(rep->data.xtrap_release),n);
    swaps(&(rep->data.xtrap_version),n);
    swaps(&(rep->data.xtrap_revision),n);
    swaps(&(rep->data.max_pkt_size),n);
    swapl(&(rep->data.major_opcode),n);
    swapl(&(rep->data.event_base),n);
    swaps(&(rep->data.cur_x),n);
    swaps(&(rep->data.cur_y),n);
    (void)WriteToClient(client,size,reply);
    return;
}
#ifdef FUNCTION_PROTOS
void sReplyXETrapGetVers(ClientPtr client, int size, char *reply)
#else
void sReplyXETrapGetVers(client,size,reply)
    ClientPtr client;
    int       size;
    char      *reply;
#endif
{
    xXTrapGetVersReply *rep = (xXTrapGetVersReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swaps(&(rep->data.xtrap_release),n);
    swaps(&(rep->data.xtrap_version),n);
    swaps(&(rep->data.xtrap_revision),n);
    (void)WriteToClient(client,size,reply);
    return;
}
#ifdef FUNCTION_PROTOS
void sReplyXETrapGetLITim(ClientPtr client, int size, char *reply)
#else
void sReplyXETrapGetLITim(client,size,reply)
    ClientPtr client;
    int       size;
    char      *reply;
#endif
{
    xXTrapGetLITimReply *rep = (xXTrapGetLITimReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swapl(&(rep->data_last_time),n);
    (void)WriteToClient(client,size,reply);
    return;
}
#ifdef FUNCTION_PROTOS
void sReplyXETrapGetCur(ClientPtr client, int size, char *reply)
#else
void sReplyXETrapGetCur(client,size,reply)
    ClientPtr client;
    int       size;
    char      *reply;
#endif
{
    xXTrapGetCurReply *rep = (xXTrapGetCurReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swaps(&(rep->data_config_max_pkt_size),n);
    (void)WriteToClient(client,size,reply);
    return;
}
#ifdef FUNCTION_PROTOS
void sReplyXETrapGetStats(ClientPtr client, int size, char *reply)
#else
void sReplyXETrapGetStats(client,size,reply)
    ClientPtr client;
    int       size;
    char      *reply;
#endif
{
    xXTrapGetStatsReply *rep = (xXTrapGetStatsReply *)reply;
    register char n;
    register int i;
    CARD32 *p;

    swaps(&(rep->sequenceNumber),n);
    swapl(&(rep->length),n);
    for (i=0L, p = (CARD32 *)rep->data.requests; i<256L; i++, p++)
    { 
        swapl(p,n);
    }
    for (i=0L, p = (CARD32 *)rep->data.events; i<XETrapCoreEvents; i++, p++)
    {
        swapl(p,n); 
    }
    (void)WriteToClient(client,size,reply);
    return;
}

/* Out-going XTrap I/O header needing to be swapped *from* native format */

#ifdef FUNCTION_PROTOS
void sXETrapHeader(XETrapHeader *hdr)
#else
void sXETrapHeader(hdr)
    XETrapHeader *hdr;
#endif
{
    register char n;

    swapl(&(hdr->count), n);
    swapl(&(hdr->timestamp), n);
    swaps(&(hdr->win_x), n);
    swaps(&(hdr->win_y), n);
    swaps(&(hdr->client), n);
}

    /* Out-going requests needing to be swapped *from* native format
     * aka swapreq.c "equivalents" 
     */

/* The following is used for all requests that have
   no fields to be swapped (except "length") */
#ifdef FUNCTION_PROTOS
void XETSwSimpleReq(register xReq *data)
#else
void XETSwSimpleReq(data)
    register xReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
}

/* The following is used for all requests that have
   only a single 32-bit field to be swapped, coming
   right after the "length" field */

#ifdef FUNCTION_PROTOS
void XETSwResourceReq(register xResourceReq *data)
#else
void XETSwResourceReq(data)
    register xResourceReq *data;
#endif
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->id), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreateWindow(register xCreateWindowReq *data)
#else
void XETSwCreateWindow(data)
    register xCreateWindowReq *data;
#endif
{
    register char n;

    swapl(&(data->wid), n);
    swapl(&(data->parent), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swaps(&(data->borderWidth), n);
    swaps(&(data->class), n);
    swapl(&(data->visual), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeWindowAttributes(register xChangeWindowAttributesReq *data)
#else
void XETSwChangeWindowAttributes(data)
    register xChangeWindowAttributesReq *data;
#endif
{
    register char n;

    swapl(&(data->window), n);
    swapl(&(data->valueMask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwReparentWindow(register xReparentWindowReq *data)
#else
void XETSwReparentWindow(data)
    register xReparentWindowReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->parent), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

#ifdef FUNCTION_PROTOS
void XETSwConfigureWindow(xConfigureWindowReq *data)
#else
void XETSwConfigureWindow(data)
    xConfigureWindowReq *data;
#endif
{
    register char n;
    swapl(&(data->window), n);
    swaps(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}


#ifdef FUNCTION_PROTOS
void XETSwInternAtom(register xInternAtomReq *data)
#else
void XETSwInternAtom(data)
    register xInternAtomReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeProperty(register xChangePropertyReq *data)
#else
void XETSwChangeProperty(data)
    register xChangePropertyReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
    swapl(&(data->type), n);
    switch ( data->format ) {
        case 8L : break;
        case 16L:
            SwapShorts((short *)(data + 1), data->nUnits);
        break;
    case 32L:
            SwapLongs((CARD32 *)(data + 1), data->nUnits);
        break;
    }
    swapl(&(data->nUnits), n);
}

#ifdef FUNCTION_PROTOS
void XETSwDeleteProperty(register xDeletePropertyReq *data)
#else
void XETSwDeleteProperty(data)
    register xDeletePropertyReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
              
}
#ifdef FUNCTION_PROTOS
void XETSwGetProperty(register xGetPropertyReq *data)
#else
void XETSwGetProperty(data)
    register xGetPropertyReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
    swapl(&(data->type), n);
    swapl(&(data->longOffset), n);
    swapl(&(data->longLength), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSetSelectionOwner(register xSetSelectionOwnerReq *data)
#else
void XETSwSetSelectionOwner(data)
    register xSetSelectionOwnerReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->selection), n);
    swapl(&(data->time), n);
}

#ifdef FUNCTION_PROTOS
void XETSwConvertSelection(register xConvertSelectionReq *data)
#else
void XETSwConvertSelection(data)
    register xConvertSelectionReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->requestor), n);
    swapl(&(data->selection), n);
    swapl(&(data->target), n);
    swapl(&(data->property), n);
    swapl(&(data->time), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSendEvent(register xSendEventReq *data)
#else
void XETSwSendEvent(data)
    register xSendEventReq *data;
#endif
{
    register char n;
    xEvent eventT;
    void (*proc)(), NotImplemented();
    swapl(&(data->destination), n);
    swapl(&(data->eventMask), n);

    /* Swap event */
    proc = EventSwapVector[data->event.u.u.type & 0177];
    if (!proc || (int (*)()) proc == (int (*)()) NotImplemented)   
        (*proc)(&(data->event), &eventT);
    data->event = eventT;
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwGrabPointer(register xGrabPointerReq *data)
#else
void XETSwGrabPointer(data)
    register xGrabPointerReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->eventMask), n);
    swapl(&(data->confineTo), n);
    swapl(&(data->cursor), n);
    swapl(&(data->time), n);
}

#ifdef FUNCTION_PROTOS
void XETSwGrabButton(register xGrabButtonReq *data)
#else
void XETSwGrabButton(data)
    register xGrabButtonReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->eventMask), n);
    swapl(&(data->confineTo), n);
    swapl(&(data->cursor), n);
    swaps(&(data->modifiers), n);
}

#ifdef FUNCTION_PROTOS
void XETSwUngrabButton(register xUngrabButtonReq *data)
#else
void XETSwUngrabButton(data)
    register xUngrabButtonReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeActivePointerGrab(register xChangeActivePointerGrabReq *data)
#else
void XETSwChangeActivePointerGrab(data)
    register xChangeActivePointerGrabReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cursor), n);
    swapl(&(data->time), n);
    swaps(&(data->eventMask), n);
}

#ifdef FUNCTION_PROTOS
void XETSwGrabKeyboard(register xGrabKeyboardReq *data)
#else
void XETSwGrabKeyboard(data)
    register xGrabKeyboardReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swapl(&(data->time), n);
}

#ifdef FUNCTION_PROTOS
void XETSwGrabKey(register xGrabKeyReq *data)
#else
void XETSwGrabKey(data)
    register xGrabKeyReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

#ifdef FUNCTION_PROTOS
void XETSwUngrabKey(register xUngrabKeyReq *data)
#else
void XETSwUngrabKey(data)
    register xUngrabKeyReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

#ifdef FUNCTION_PROTOS
void XETSwGetMotionEvents(register xGetMotionEventsReq *data)
#else
void XETSwGetMotionEvents(data)
    register xGetMotionEventsReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->start), n);
    swapl(&(data->stop), n);
}

#ifdef FUNCTION_PROTOS
void XETSwTranslateCoords(register xTranslateCoordsReq *data)
#else
void XETSwTranslateCoords(data)
    register xTranslateCoordsReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcWid), n);
    swapl(&(data->dstWid), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
}

#ifdef FUNCTION_PROTOS
void XETSwWarpPointer(register xWarpPointerReq *data)
#else
void XETSwWarpPointer(data)
    register xWarpPointerReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcWid), n);
    swapl(&(data->dstWid), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->srcWidth), n);
    swaps(&(data->srcHeight), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSetInputFocus(register xSetInputFocusReq *data)
#else
void XETSwSetInputFocus(data)
    register xSetInputFocusReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->focus), n);
    swapl(&(data->time), n);
}

#ifdef FUNCTION_PROTOS
void XETSwOpenFont(register xOpenFontReq *data)
#else
void XETSwOpenFont(data)
    register xOpenFontReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->fid), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwListFonts(register xListFontsReq *data)
#else
void XETSwListFonts(data)
    register xListFontsReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->maxNames), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwListFontsWithInfo(register xListFontsWithInfoReq *data)
#else
void XETSwListFontsWithInfo(data)
    register xListFontsWithInfoReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->maxNames), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSetFontPath(register xSetFontPathReq *data)
#else
void XETSwSetFontPath(data)
    register xSetFontPathReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nFonts), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreatePixmap(register xCreatePixmapReq *data)
#else
void XETSwCreatePixmap(data)
    register xCreatePixmapReq *data;
#endif
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->pid), n);
    swapl(&(data->drawable), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreateGC(register xCreateGCReq *data)
#else
void XETSwCreateGC(data)
    register xCreateGCReq *data;
#endif
{
    register char n;
    swapl(&(data->gc), n);
    swapl(&(data->drawable), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeGC(register xChangeGCReq *data)
#else
void XETSwChangeGC(data)
    register xChangeGCReq *data;
#endif
{
    register char n;
    swapl(&(data->gc), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCopyGC(register xCopyGCReq *data)
#else
void XETSwCopyGC(data)
    register xCopyGCReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcGC), n);
    swapl(&(data->dstGC), n);
    swapl(&(data->mask), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSetDashes(register xSetDashesReq *data)
#else
void XETSwSetDashes(data)
    register xSetDashesReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->gc), n);
    swaps(&(data->dashOffset), n);
    swaps(&(data->nDashes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwSetClipRectangles(register xSetClipRectanglesReq *data)
#else
void XETSwSetClipRectangles(data)
    register xSetClipRectanglesReq *data;
#endif
{
    register char n;
    swapl(&(data->gc), n);
    swaps(&(data->xOrigin), n);
    swaps(&(data->yOrigin), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwClearToBackground(register xClearAreaReq *data)
#else
void XETSwClearToBackground(data)
    register xClearAreaReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCopyArea(register xCopyAreaReq *data)
#else
void XETSwCopyArea(data)
    register xCopyAreaReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcDrawable), n);
    swapl(&(data->dstDrawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCopyPlane(register xCopyPlaneReq *data)
#else
void XETSwCopyPlane(data)
    register xCopyPlaneReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcDrawable), n);
    swapl(&(data->dstDrawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swapl(&(data->bitPlane), n);
}

/* The following routine is used for all Poly drawing requests
   (except FillPoly, which uses a different request format) */
#ifdef FUNCTION_PROTOS
void XETSwPoly(register xPolyPointReq *data)
#else
void XETSwPoly(data)
    register xPolyPointReq *data;
#endif
{
    register char n;

    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}
     /* cannot use XETSwPoly for this one, because xFillPolyReq
      * is longer than xPolyPointReq, and we don't want to swap
      * the difference as shorts! 
      */
#ifdef FUNCTION_PROTOS
void XETSwFillPoly(register xFillPolyReq *data)
#else
void XETSwFillPoly(data)
    register xFillPolyReq *data;
#endif
{
    register char n;

    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwPutImage(register xPutImageReq *data)
#else
void XETSwPutImage(data)
    register xPutImageReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    /* Image should already be swapped */
}

#ifdef FUNCTION_PROTOS
void XETSwGetImage(register xGetImageReq *data)
#else
void XETSwGetImage(data)
    register xGetImageReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swapl(&(data->planeMask), n);
}

/* ProcPolyText used for both PolyText8 and PolyText16 */

#ifdef FUNCTION_PROTOS
void XETSwPolyText(register xPolyTextReq *data)
#else
void XETSwPolyText(data)
    register xPolyTextReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

/* ProcImageText used for both ImageText8 and ImageText16 */

#ifdef FUNCTION_PROTOS
void XETSwImageText(register xImageTextReq *data)
#else
void XETSwImageText(data)
    register xImageTextReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreateColormap(register xCreateColormapReq *data)
#else
void XETSwCreateColormap(data)
    register xCreateColormapReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->mid), n);
    swapl(&(data->window), n);
    swapl(&(data->visual), n);
}


#ifdef FUNCTION_PROTOS
void XETSwCopyColormapAndFree(register xCopyColormapAndFreeReq *data)
#else
void XETSwCopyColormapAndFree(data)
    register xCopyColormapAndFreeReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->mid), n);
    swapl(&(data->srcCmap), n);

}

#ifdef FUNCTION_PROTOS
void XETSwAllocColor                (register xAllocColorReq *data)
#else
void XETSwAllocColor                (data)
    register xAllocColorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->red), n);
    swaps(&(data->green), n);
    swaps(&(data->blue), n);
}

#ifdef FUNCTION_PROTOS
void XETSwAllocNamedColor           (register xAllocNamedColorReq *data)
#else
void XETSwAllocNamedColor           (data)
    register xAllocNamedColorReq *data;
#endif
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwAllocColorCells           (register xAllocColorCellsReq *data)
#else
void XETSwAllocColorCells           (data)
    register xAllocColorCellsReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->colors), n);
    swaps(&(data->planes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwAllocColorPlanes(register xAllocColorPlanesReq *data)
#else
void XETSwAllocColorPlanes(data)
    register xAllocColorPlanesReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->colors), n);
    swaps(&(data->red), n);
    swaps(&(data->green), n);
    swaps(&(data->blue), n);
}

#ifdef FUNCTION_PROTOS
void XETSwFreeColors          (register xFreeColorsReq *data)
#else
void XETSwFreeColors          (data)
    register xFreeColorsReq *data;
#endif
{
    register char n;
    swapl(&(data->cmap), n);
    swapl(&(data->planeMask), n);
    SwapRestL(data);
    swaps(&(data->length), n);

}

#ifdef FUNCTION_PROTOS
void XETSwStoreColors               (register xStoreColorsReq *data)
#else
void XETSwStoreColors               (data)
    register xStoreColorsReq *data;
#endif
{
    register char n;
    unsigned long count;
    xColorItem     *pItem;

    swapl(&(data->cmap), n);
    pItem = (xColorItem *) &(data[1]);
    for(count = LengthRestB(data)/sizeof(xColorItem); count != 0; count--)
        SwapColorItem(pItem++);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwStoreNamedColor           (register xStoreNamedColorReq *data)
#else
void XETSwStoreNamedColor           (data)
    register xStoreNamedColorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swapl(&(data->pixel), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwQueryColors(register xQueryColorsReq *data)
#else
void XETSwQueryColors(data)
    register xQueryColorsReq *data;
#endif
{
    register char n;
    swapl(&(data->cmap), n);
    SwapRestL(data);
    swaps(&(data->length), n);
} 

#ifdef FUNCTION_PROTOS
void XETSwLookupColor(register xLookupColorReq *data)
#else
void XETSwLookupColor(data)
    register xLookupColorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreateCursor(register xCreateCursorReq *data)
#else
void XETSwCreateCursor(data)
    register xCreateCursorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cid), n);
    swapl(&(data->source), n);
    swapl(&(data->mask), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

#ifdef FUNCTION_PROTOS
void XETSwCreateGlyphCursor(register xCreateGlyphCursorReq *data)
#else
void XETSwCreateGlyphCursor(data)
    register xCreateGlyphCursorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cid), n);
    swapl(&(data->source), n);
    swapl(&(data->mask), n);
    swaps(&(data->sourceChar), n);
    swaps(&(data->maskChar), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
}


#ifdef FUNCTION_PROTOS
void XETSwRecolorCursor(register xRecolorCursorReq *data)
#else
void XETSwRecolorCursor(data)
    register xRecolorCursorReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cursor), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
}

#ifdef FUNCTION_PROTOS
void XETSwQueryBestSize   (register xQueryBestSizeReq *data)
#else
void XETSwQueryBestSize   (data)
    register xQueryBestSizeReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);

}

#ifdef FUNCTION_PROTOS
void XETSwQueryExtension (register xQueryExtensionReq *data)
#else
void XETSwQueryExtension (data)
    register xQueryExtensionReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nbytes), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeKeyboardMapping   (register xChangeKeyboardMappingReq *data)
#else
void XETSwChangeKeyboardMapping   (data)
    register xChangeKeyboardMappingReq *data;
#endif
{
    register char n;
    register CARD32 *p;
    register int i, count;

    swaps(&(data->length), n);
    p = (CARD32 *)&(data[1]);
    count = data->keyCodes * data->keySymsPerKeyCode;
    for(i = 0; i < count; i++)
    {
        swapl(p, n);
        p++;
    }
}


#ifdef FUNCTION_PROTOS
void XETSwChangeKeyboardControl   (register xChangeKeyboardControlReq *data)
#else
void XETSwChangeKeyboardControl   (data)
    register xChangeKeyboardControlReq *data;
#endif
{
    register char n;
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangePointerControl   (register xChangePointerControlReq *data)
#else
void XETSwChangePointerControl   (data)
    register xChangePointerControlReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->accelNum), n);
    swaps(&(data->accelDenum), n);
    swaps(&(data->threshold), n);
}


#ifdef FUNCTION_PROTOS
void XETSwSetScreenSaver            (register xSetScreenSaverReq *data)
#else
void XETSwSetScreenSaver            (data)
    register xSetScreenSaverReq *data;
#endif
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->timeout), n);
    swaps(&(data->interval), n);
}

#ifdef FUNCTION_PROTOS
void XETSwChangeHosts(register xChangeHostsReq *data)
#else
void XETSwChangeHosts(data)
    register xChangeHostsReq *data;
#endif
{
    register char n;

    swaps(&(data->length), n);
    swaps(&(data->hostLength), n);

}
#ifdef FUNCTION_PROTOS
void XETSwRotateProperties(register xRotatePropertiesReq *data)
#else
void XETSwRotateProperties(data)
    register xRotatePropertiesReq *data;
#endif
{
    register char n;
    swapl(&(data->window), n);
    swaps(&(data->nAtoms), n);
    swaps(&(data->nPositions), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

/*ARGSUSED*/
#ifdef FUNCTION_PROTOS
void XETSwNoOperation(xReq *data)
#else
void XETSwNoOperation(data)
    xReq *data;
#endif
{
    /* noop -- don't do anything */
}

/* Byte swap a list of longs */
#if defined vms && !defined MITR5
#ifndef LINKED_IN
#ifdef FUNCTION_PROTOS
void SwapLongs ( register CARD32 *list, register unsigned long count)
#else
void SwapLongs (list, count)
    register CARD32 *list;
    register unsigned long count;
#endif
{
    register char n;

    while (count >= 8) {
        swapl(list+0, n);
        swapl(list+1, n);
        swapl(list+2, n);
        swapl(list+3, n);
        swapl(list+4, n);
        swapl(list+5, n);
        swapl(list+6, n);
        swapl(list+7, n);
        list += 8;
        count -= 8;
    }
    if (count != 0) {
        do {
        swapl(list, n);
        list++;
        } while (--count != 0);
    }
}

/* Byte swap a list of shorts */

#ifdef FUNCTION_PROTOS
void SwapShorts (register short *list, register unsigned long count)
#else
void SwapShorts (list, count)
    register short *list;
    register unsigned long count;
#endif
{
    register char n;

    while (count >= 16) {
        swaps(list+0, n);
        swaps(list+1, n);
        swaps(list+2, n);
        swaps(list+3, n);
        swaps(list+4, n);
        swaps(list+5, n);
        swaps(list+6, n);
        swaps(list+7, n);
        swaps(list+8, n);
        swaps(list+9, n);
        swaps(list+10, n);
        swaps(list+11, n);
        swaps(list+12, n);
        swaps(list+13, n);
        swaps(list+14, n);
        swaps(list+15, n);
        list += 16;
        count -= 16;
    }
    if (count != 0) {
        do {
        swaps(list, n);
        list++;
        } while (--count != 0);
    }
}

#ifdef FUNCTION_PROTOS
SwapColorItem(xColorItem *pItem)
#else
SwapColorItem(pItem)
    xColorItem    *pItem;
#endif
{
    register char n;
    swapl(&pItem->pixel, n);
    swaps(&pItem->red, n);
    swaps(&pItem->green, n);
    swaps(&pItem->blue, n);
}
#endif /* LINKED_IN */
#endif /* vms */
