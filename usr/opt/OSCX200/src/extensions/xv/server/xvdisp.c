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
Copyright 1991 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/*
** File: 
**
**   xvdisp.c --- Xv server extension dispatch module.
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**
*/

#include <stdio.h>

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"

#include "Xv.h"
#include "Xvproto.h"
#include "xvdix.h"

/* EXTERNAL */

extern void (* ReplySwapVector[256]) ();

/* INTERNAL */

static int ProcXvQueryExtension();
static int ProcXvQueryAdaptors();
static int ProcXvQueryEncodings();
static int ProcXvPutVideo();
static int ProcXvPutStill();
static int ProcXvGetVideo();
static int ProcXvGetStill();
static int ProcXvGrabPort();
static int ProcXvUngrabPort();
static int ProcXvSelectVideoNotify();
static int ProcXvSelectPortNotify();
static int ProcXvStopVideo();
static int ProcXvSetPortAttribute();
static int ProcXvGetPorAttribute();
static int ProcXvQueryBestSize();

static int SProcXvQueryExtension();
static int SProcXvQueryAdaptors();
static int SProcXvQueryEncodings();
static int SProcXvPutVideo();
static int SProcXvPutStill();
static int SProcXvGetVideo();
static int SProcXvGetStill();
static int SProcXvGrabPort();
static int SProcXvUngrabPort();
static int SProcXvSelectVideoNotify();
static int SProcXvSelectPortNotify();
static int SProcXvStopVideo();
static int SProcXvSetPortAttribute();
static int SProcXvGetPortAttribute();
static int SProcXvQueryBestSize();

static int SWriteQueryVideoReply();
static int SWriteAdaptorInfo();
static int SWriteEncodingInfo();
static int SWriteFormat();
static int SWriteGrabPortReply();
static int SWriteGetPortAttributeReply();
static int SWriteQueryBestSizeReply();

#define _WriteQueryAdaptorsReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryAdaptorsReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryAdaptorsReply, _d)

#define _WriteQueryExtensionReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryExtensionReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryExtensionReply, _d)

#define _WriteQueryEncodingsReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryEncodingsReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryEncodingsReply, _d)

#define _WriteAdaptorInfo(_c,_d) \
  if ((_c)->swapped) SWriteAdaptorInfo(_c, _d); \
  else WriteToClient(_c, sz_xvAdaptorInfo, _d)

#define _WriteEncodingInfo(_c,_d) \
  if ((_c)->swapped) SWriteEncodingInfo(_c, _d); \
  else WriteToClient(_c, sz_xvEncodingInfo, _d)

#define _WriteFormat(_c,_d) \
  if ((_c)->swapped) SWriteFormat(_c, _d); \
  else WriteToClient(_c, sz_xvFormat, _d)

#define _WriteGrabPortReply(_c,_d) \
  if ((_c)->swapped) SWriteGrabPortReply(_c, _d); \
  else WriteToClient(_c, sz_xvGrabPortReply, _d)

#define _WriteGetPortAttributeReply(_c,_d) \
  if ((_c)->swapped) SWriteGetPortAttributeReply(_c, _d); \
  else WriteToClient(_c, sz_xvGetPortAttributeReply, _d)

#define _WriteQueryBestSizeReply(_c,_d) \
  if ((_c)->swapped) SWriteQueryBestSizeReply(_c, _d); \
  else WriteToClient(_c, sz_xvQueryBestSizeReply, _d)

#define _AllocatePort(_i,_p) \
  ((_p)->id != _i) ? (* (_p)->pAdaptor->ddAllocatePort)(_i,_p,&_p) : Success

/*
** ProcXvDispatch
**
**
**
*/

int
ProcXvDispatch(client)
register ClientPtr client;
{
  REQUEST(xReq);

  UpdateCurrentTime();

  switch (stuff->data) 
    {
    case xv_QueryExtension: return(ProcXvQueryExtension(client));
    case xv_QueryAdaptors: return(ProcXvQueryAdaptors(client));
    case xv_QueryEncodings: return(ProcXvQueryEncodings(client));
    case xv_PutVideo: return(ProcXvPutVideo(client));
    case xv_PutStill: return(ProcXvPutStill(client));
    case xv_GetVideo: return(ProcXvGetVideo(client));
    case xv_GetStill: return(ProcXvGetStill(client));
    case xv_GrabPort: return(ProcXvGrabPort(client));
    case xv_UngrabPort: return(ProcXvUngrabPort(client));
    case xv_SelectVideoNotify: return(ProcXvSelectVideoNotify(client));
    case xv_SelectPortNotify: return(ProcXvSelectPortNotify(client));
    case xv_StopVideo: return(ProcXvStopVideo(client));
    case xv_SetPortAttribute: return(ProcXvSetPortAttribute(client));
    case xv_GetPortAttribute: return(ProcXvGetPortAttribute(client));
    case xv_QueryBestSize: return(ProcXvQueryBestSize(client));
    default:
      if (stuff->data < xvNumRequests)
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, 
			    BadImplementation);
	  return(BadImplementation);
	}
      else
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, BadRequest);
	  return(BadRequest);
	}
    }
}

int
SProcXvDispatch(client)
register ClientPtr client;
{
  REQUEST(xReq);

  UpdateCurrentTime();

  switch (stuff->data) 
    {
    case xv_QueryExtension: return(SProcXvQueryExtension(client));
    case xv_QueryAdaptors: return(SProcXvQueryAdaptors(client));
    case xv_QueryEncodings: return(SProcXvQueryEncodings(client));
    case xv_PutVideo: return(SProcXvPutVideo(client));
    case xv_PutStill: return(SProcXvPutStill(client));
    case xv_GetVideo: return(SProcXvGetVideo(client));
    case xv_GetStill: return(SProcXvGetStill(client));
    case xv_GrabPort: return(SProcXvGrabPort(client));
    case xv_UngrabPort: return(SProcXvUngrabPort(client));
    case xv_SelectVideoNotify: return(SProcXvSelectVideoNotify(client));
    case xv_SelectPortNotify: return(SProcXvSelectPortNotify(client));
    case xv_StopVideo: return(SProcXvStopVideo(client));
    case xv_SetPortAttribute: return(SProcXvSetPortAttribute(client));
    case xv_GetPortAttribute: return(SProcXvGetPortAttribute(client));
    case xv_QueryBestSize: return(SProcXvQueryBestSize(client));
    default:
      if (stuff->data < xvNumRequests)
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, 
			    BadImplementation);
	  return(BadImplementation);
	}
      else
	{
	  SendErrorToClient(client, XvReqCode, stuff->data, 0, BadRequest);
	  return(BadRequest);
	}
    }
}

static int
ProcXvQueryExtension(client)
register ClientPtr client;

{
  xvQueryExtensionReply rep;
  REQUEST(xvQueryExtensionReq);
  REQUEST_SIZE_MATCH(xvQueryExtensionReq);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.version = XvVersion;
  rep.revision = XvRevision;

  _WriteQueryExtensionReply(client, &rep);

  return Success;

}

static int
ProcXvQueryAdaptors(client)
register ClientPtr client;

{
  xvFormat format;
  xvEncodingInfo einfo;
  xvAdaptorInfo ainfo;
  xvQueryAdaptorsReply rep;
  int totalSize;
  int na;
  XvAdaptorPtr pa;
  int nf;
  XvFormatPtr pf;
  WindowPtr pWin;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;

  REQUEST(xvQueryAdaptorsReq);
  REQUEST_SIZE_MATCH(xvQueryAdaptorsReq);

  if(!(pWin = (WindowPtr)LookupWindow(stuff->window, client) ))
    {
      client->errorValue = stuff->window;
      return (BadWindow);
    }

  pScreen = pWin->drawable.pScreen;
  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  if (!pxvs)
    {
      rep.type = X_Reply;
      rep.sequenceNumber = client->sequence;
      rep.num_adaptors = 0;
      rep.length = 0;

      _WriteQueryAdaptorsReply(client, &rep);

      return Success;
    }

  (* pxvs->ddQueryAdaptors)(pScreen, &pxvs->nAdaptors, &pxvs->pAdaptors);

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_adaptors = pxvs->nAdaptors;

  /* CALCULATE THE TOTAL SIZE OF THE REPLY IN BYTES */

  totalSize = pxvs->nAdaptors * sz_xvAdaptorInfo;

  /* FOR EACH ADPATOR ADD UP THE BYTES FOR ENCODINGS AND FORMATS */

  na = pxvs->nAdaptors;
  pa = pxvs->pAdaptors;
  while (na--)
    {
      totalSize += (strlen(pa->name) + 3) & ~3;
      totalSize += pa->nFormats * sz_xvFormat;
      pa++;
    }

  rep.length = totalSize >> 2;

  _WriteQueryAdaptorsReply(client, &rep);

  na = pxvs->nAdaptors;
  pa = pxvs->pAdaptors;
  while (na--)
    {

      ainfo.base_id = pa->base_id;
      ainfo.num_ports = pa->nPorts;
      ainfo.type = pa->type;
      ainfo.name_size = strlen(pa->name);
      ainfo.num_formats = pa->nFormats;

      _WriteAdaptorInfo(client, &ainfo);

      WriteToClient(client, strlen(pa->name), pa->name);

      nf = pa->nFormats;
      pf = pa->pFormats;
      while (nf--)
	{
	  format.depth = pf->depth;
	  format.visual = pf->visual;
	  _WriteFormat(client, &format);
	  pf++;
	}

      pa++;

    }

  return (client->noClientException);

}

static int
ProcXvQueryEncodings(client)
register ClientPtr client;

{
  xvEncodingInfo einfo;
  xvQueryEncodingsReply rep;
  int totalSize;
  XvPortPtr pPort;
  int ne;
  XvEncodingPtr pe;
  int status;

  REQUEST(xvQueryEncodingsReq);
  REQUEST_SIZE_MATCH(xvQueryEncodingsReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.num_encodings = pPort->pAdaptor->nEncodings;

  /* FOR EACH ENCODING ADD UP THE BYTES FOR ENCODING NAMES */

  ne = pPort->pAdaptor->nEncodings;
  pe = pPort->pAdaptor->pEncodings;
  totalSize = ne * sz_xvEncodingInfo;
  while (ne--)
    {
      totalSize += (strlen(pe->name) + 3) & ~3;
      pe++;
    }

  rep.length = totalSize >> 2;

  _WriteQueryEncodingsReply(client, &rep);

  ne = pPort->pAdaptor->nEncodings;
  pe = pPort->pAdaptor->pEncodings;
  do
    {
      einfo.encoding = pe->id;
      einfo.name_size = strlen(pe->name);
      einfo.width = pe->width;
      einfo.height = pe->height;
      einfo.rate.numerator = pe->rate.numerator;
      einfo.rate.denominator = pe->rate.denominator;
      _WriteEncodingInfo(client, &einfo);
      WriteToClient(client, strlen(pe->name), pe->name);
      pe++;
    } while (--ne);

  return (client->noClientException);

}

static int
ProcXvPutVideo(client)
register ClientPtr client;

{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvPutVideoReq);
  REQUEST_SIZE_MATCH(xvPutVideoReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvInputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diPutVideo)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}

static int
ProcXvPutStill(client)
register ClientPtr client;

{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvPutStillReq);
  REQUEST_SIZE_MATCH(xvPutStillReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvInputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diPutStill)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}


static int
ProcXvGetVideo(client)
register ClientPtr client;

{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvGetVideoReq);
  REQUEST_SIZE_MATCH(xvGetVideoReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvOutputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diGetVideo)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}


static int
ProcXvGetStill(client)
register ClientPtr client;

{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  register GCPtr pGC;
  int status;

  REQUEST(xvGetStillReq);
  REQUEST_SIZE_MATCH(xvGetStillReq);

  VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, pGC, client);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!(pPort->pAdaptor->type & XvOutputMask))
    {
      client->errorValue = stuff->port;
      return (BadMatch);
    }

  status = XVCALL(diMatchPort)(pPort, pDraw);
  if (status != Success)
    {
      return status;
    }

  return XVCALL(diGetStill)(client, pDraw, pPort, pGC,
			    stuff->vid_x, stuff->vid_y,
			    stuff->vid_w, stuff->vid_h,
			    stuff->drw_x, stuff->drw_y,
			    stuff->drw_w, stuff->drw_h);

}

static int
ProcXvSelectVideoNotify(client)
register ClientPtr client;

{
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  REQUEST(xvSelectVideoNotifyReq);
  REQUEST_SIZE_MATCH(xvSelectVideoNotifyReq);

  if(!(pDraw = (DrawablePtr)LOOKUP_DRAWABLE(stuff->drawable, client) ))
    {
      client->errorValue = stuff->drawable;
      return (BadWindow);
    }

  return XVCALL(diSelectVideoNotify)(client, pDraw, stuff->onoff);

}

static int
ProcXvSelectPortNotify(client)
register ClientPtr client;

{
  int status;
  XvPortPtr pPort;
  REQUEST(xvSelectPortNotifyReq);
  REQUEST_SIZE_MATCH(xvSelectPortNotifyReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  return XVCALL(diSelectPortNotify)(client, pPort, stuff->onoff);

}

static int
ProcXvGrabPort(client)
register ClientPtr client;

{
  int result, status;
  XvPortPtr pPort;
  xvGrabPortReply rep;
  REQUEST(xvGrabPortReq);
  REQUEST_SIZE_MATCH(xvGrabPortReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  status = XVCALL(diGrabPort)(client, pPort, stuff->time, &result);

  if (status != Success)
    {
      return status;
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.result = result;

  _WriteGrabPortReply(client, &rep);

  return Success;

}

static int
ProcXvUngrabPort(client)
register ClientPtr client;

{
  int status;
  XvPortPtr pPort;
  REQUEST(xvGrabPortReq);
  REQUEST_SIZE_MATCH(xvGrabPortReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  return XVCALL(diUngrabPort)(client, pPort, stuff->time);

}


static int
ProcXvStopVideo(client)
register ClientPtr client;

{
  int status;
  register DrawablePtr pDraw;
  XvPortPtr pPort;
  REQUEST(xvStopVideoReq);
  REQUEST_SIZE_MATCH(xvStopVideoReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if(!(pDraw = LOOKUP_DRAWABLE(stuff->drawable, client) ))
    {
      client->errorValue = stuff->drawable;
      return (BadDrawable);
    }

  return XVCALL(diStopVideo)(client, pPort, pDraw);

}

static int
ProcXvSetPortAttribute(client)
register ClientPtr client;

{
  int status;
  register len;
  XvPortPtr pPort;
  REQUEST(xvSetPortAttributeReq);
  REQUEST_SIZE_MATCH(xvSetPortAttributeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!ValidAtom(stuff->attribute))
    {
      client->errorValue = stuff->attribute;
      return(BadAtom);
    }

  status = XVCALL(diSetPortAttribute)(client, pPort, 
				    stuff->attribute, stuff->value);

  if (status == BadMatch) 
      client->errorValue = stuff->attribute;
  else
      client->errorValue = stuff->value;

  return status;

}

static int
ProcXvGetPortAttribute(client)
register ClientPtr client;

{
  int value;
  int status;
  XvPortPtr pPort;
  xvGetPortAttributeReply rep;
  REQUEST(xvGetPortAttributeReq);
  REQUEST_SIZE_MATCH(xvGetPortAttributeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  if (!ValidAtom(stuff->attribute))
    {
      client->errorValue = stuff->attribute;
      return(BadAtom);
    }

  status = XVCALL(diGetPortAttribute)(client, pPort, stuff->attribute, &value);
  if (status != Success)
    {
      client->errorValue = stuff->attribute;
      return status;
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;
  rep.value = value;
 
  _WriteGetPortAttributeReply(client, &rep);

  return Success;
}

static int
ProcXvQueryBestSize(client)
register ClientPtr client;

{
  int status;
  CARD16 actual_width, actual_height;
  XvPortPtr pPort;
  xvQueryBestSizeReply rep;
  REQUEST(xvQueryBestSizeReq);
  REQUEST_SIZE_MATCH(xvQueryBestSizeReq);

  if(!(pPort = LOOKUP_PORT(stuff->port, client) ))
    {
      client->errorValue = stuff->port;
      return (_XvBadPort);
    }

  if ((status = _AllocatePort(stuff->port, pPort)) != Success)
    {
      client->errorValue = stuff->port;
      return (status);
    }

  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.length = 0;

  (* pPort->pAdaptor->ddQueryBestSize)(client, pPort, stuff->motion,
				       stuff->vid_w, stuff->vid_h, 
				       stuff->drw_w, stuff->drw_h, 
				       &actual_width, &actual_height);

  rep.actual_width = actual_width;
  rep.actual_height = actual_height;
 
  _WriteQueryBestSizeReply(client, &rep);

  return Success;
}

/* Swapped Procs */

static int
SProcXvQueryExtension(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvQueryExtensionReq);
  swaps(&stuff->length, n);
  return ProcXvQueryExtension(client);
}

static int
SProcXvQueryAdaptors(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvQueryAdaptorsReq);
  swaps(&stuff->length, n);
  swapl(&stuff->window, n);
  return ProcXvQueryAdaptors(client);
}

static int
SProcXvQueryEncodings(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvQueryEncodingsReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvQueryEncodings(client);
}

static int
SProcXvGrabPort(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvGrabPortReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->time, n);
  return ProcXvGrabPort(client);
}

static int
SProcXvUngrabPort(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvUngrabPortReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->time, n);
  return ProcXvUngrabPort(client);
}

static int
SProcXvPutVideo(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvPutVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvPutVideo(client);
}

static int
SProcXvPutStill(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvPutStillReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvPutStill(client);
}

static int
SProcXvGetVideo(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvGetVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvGetVideo(client);
}

static int
SProcXvGetStill(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvGetStillReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  swapl(&stuff->gc, n);
  swaps(&stuff->vid_x, n);
  swaps(&stuff->vid_y, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_x, n);
  swaps(&stuff->drw_y, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvGetStill(client);
}

static int
SProcXvSelectVideoNotify(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvSelectVideoNotifyReq);
  swaps(&stuff->length, n);
  swapl(&stuff->drawable, n);
  return ProcXvSelectVideoNotify(client);
}

static int
SProcXvSelectPortNotify(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvSelectPortNotifyReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  return ProcXvSelectPortNotify(client);
}

static int
SProcXvStopVideo(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvStopVideoReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->drawable, n);
  return ProcXvStopVideo(client);
}

static int
SProcXvSetPortAttribute(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvSetPortAttributeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->attribute, n);
  swapl(&stuff->value, n);
  return ProcXvSetPortAttribute(client);
}

static int
SProcXvGetPortAttribute(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvGetPortAttributeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swapl(&stuff->attribute, n);
  return ProcXvGetPortAttribute(client);
}

static int
SProcXvQueryBestSize(client)
register ClientPtr client;

{
  register char n;
  REQUEST(xvQueryBestSizeReq);
  swaps(&stuff->length, n);
  swapl(&stuff->port, n);
  swaps(&stuff->vid_w, n);
  swaps(&stuff->vid_h, n);
  swaps(&stuff->drw_w, n);
  swaps(&stuff->drw_h, n);
  return ProcXvQueryBestSize(client);
}

static int
SWriteQueryExtensionReply(client, rep)
register ClientPtr client;
xvQueryExtensionReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->version, n);
  swaps(&rep->revision, n);
  
  (void)WriteToClient(client, sz_xvQueryExtensionReply, (char *)rep);

}

static int
SWriteQueryAdaptorsReply(client, rep)
register ClientPtr client;
xvQueryAdaptorsReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->num_adaptors, n);
  
  (void)WriteToClient(client, sz_xvQueryAdaptorsReply, (char *)rep);

}

static int
SWriteQueryEncodingsReply(client, rep)
register ClientPtr client;
xvQueryEncodingsReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->num_encodings, n);
  
  (void)WriteToClient(client, sz_xvQueryEncodingsReply, (char *)rep);

}

static int
SWriteAdaptorInfo(client, pAdaptor)
register ClientPtr client;
xvAdaptorInfo *pAdaptor;

{
  register char n;

  swapl(&pAdaptor->base_id, n);
  swaps(&pAdaptor->name_size, n);
  swaps(&pAdaptor->num_ports, n);
  swaps(&pAdaptor->num_formats, n);

  (void)WriteToClient(client, sz_xvAdaptorInfo, (char *)pAdaptor);

}

static int
SWriteEncodingInfo(client, pEncoding)
register ClientPtr client;
xvEncodingInfo *pEncoding;

{
  register char n;
  
  swapl(&pEncoding->encoding, n);
  swaps(&pEncoding->name_size, n);
  swaps(&pEncoding->width, n);
  swaps(&pEncoding->height, n);
  swapl(&pEncoding->rate.numerator, n);
  swapl(&pEncoding->rate.denominator, n);
  (void)WriteToClient(client, sz_xvEncodingInfo, (char *)pEncoding);
}

static int
SWriteFormat(client, pFormat)
register ClientPtr client;
xvFormat *pFormat;

{
  register char n;

  swapl(&pFormat->visual, n);
  (void)WriteToClient(client, sz_xvFormat, (char *)pFormat);
}

static int
SWriteGrabPortReply(client, rep)
register ClientPtr client;
xvGrabPortReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);

  (void)WriteToClient(client, sz_xvGrabPortReply, (char *)rep);

}

static int
SWriteGetPortAttributeReply(client, rep)
register ClientPtr client;
xvGetPortAttributeReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swapl(&rep->value, n);

  (void)WriteToClient(client, sz_xvGetPortAttributeReply, (char *)rep);

}

static int
SWriteQueryBestSizeReply(client, rep)
register ClientPtr client;
xvQueryBestSizeReply *rep;

{
  register char n;

  swaps(&rep->sequenceNumber, n);
  swapl(&rep->length, n);
  swaps(&rep->actual_width, n);
  swaps(&rep->actual_height, n);

  (void)WriteToClient(client, sz_xvQueryBestSizeReply, (char *)rep);

}
