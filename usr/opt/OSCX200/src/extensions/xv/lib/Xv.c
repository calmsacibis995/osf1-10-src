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
**   Xv.c --- Xv library extension module.
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   26.06.91 Carver
**     - changed XvFreeAdaptors to XvFreeAdaptorInfo
**     - changed XvFreeEncodings to XvFreeEncodingInfo
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   240.01.91 Carver
**     - version 1.4 upgrade
**
*/

#include "Xvlibint.h"

static Bool wire_to_event();
static int close_display();
static char *error_string();

static XExtensionInfo *xv_info;
static char *xv_extension_name = XvName;

#define XvCheckExtension(dpy,i,val) \
		XextCheckExtension(dpy, i, xv_extension_name, val)

static XExtensionHooks xv_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    close_display,			/* close_display */
    wire_to_event,			/* wire_to_event */
    NULL,				/* event_to_wire */
    NULL,				/* error */
    error_string			/* error_string */
};

static char *xv_error_list[] = {
    "BadPort",
    "BadEncoding"
};

static XEXT_GENERATE_FIND_DISPLAY(find_display, xv_info,
				   xv_extension_name, 
				   &xv_extension_hooks,
				   XvNumEvents, NULL)

static XEXT_GENERATE_CLOSE_DISPLAY(close_display, xv_info)

static XEXT_GENERATE_ERROR_STRING (error_string, xv_extension_name,
				   XvNumErrors, xv_error_list)

#define	PREAMBLE(stat) \
  XExtDisplayInfo *info = find_display(dpy); \
  XvCheckExtension(dpy,info,0); \
  LockDisplay(dpy)

#define POSTAMBLE \
  UnlockDisplay(dpy); \
  SyncHandle()

XvQueryExtension(dpy, p_version, p_revision, 
		 p_requestBase, p_eventBase, p_errorBase)
register Display *dpy;
unsigned int *p_version, *p_revision;
unsigned int *p_requestBase, *p_eventBase, *p_errorBase;

{
  register xvQueryExtensionReq *req;
  xvQueryExtensionReply rep;
  XExtDisplayInfo *info = find_display(dpy);

  if (XextHasExtension(info))
    {
      
      XvGetReq(QueryExtension, req);
      
      /* READ THE REPLY */
      
      if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) 
	{
	  UnlockDisplay(dpy);
	  SyncHandle();
	  return(XvBadExtension);
	}
      
      *p_version = rep.version;
      *p_revision = rep.revision;
      *p_requestBase = info->codes->major_opcode;
      *p_eventBase = info->codes->first_event;
      *p_errorBase = info->codes->first_error;
    }
  else
    {
      return XvBadExtension;
    }

  return Success;

}

XvQueryAdaptors(dpy, window, p_nAdaptors, p_pAdaptors)

register Display *dpy;
Window window;
unsigned int *p_nAdaptors;
XvAdaptorInfo **p_pAdaptors;
{
  register xvQueryAdaptorsReq *req;
  xvQueryAdaptorsReply rep;
  int size,ii,jj;
  char *name;
  XvAdaptorInfo *pas, *pa;
  XvFormat *pfs, *pf;
  char *buffer;
  union 
    {
      char *buffer;
      char *string;
      xvAdaptorInfo *pa;
      xvFormat *pf;
    } u;
  
  PREAMBLE(XvBadExtension);

  XvGetReq(QueryAdaptors, req);
  req->window = window;

  /* READ THE REPLY */

  if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) 
    {
      UnlockDisplay(dpy);
      SyncHandle();
      return(XvBadReply);
    }

  size = rep.length << 2;
  if ( (buffer = (char *)Xmalloc ((unsigned) size)) == (char *)NULL)
    {
      UnlockMutex(&lock);
      return(XvBadAlloc);
    }
  _XRead (dpy, buffer, size);

  u.buffer = buffer;

  /* GET INPUT ADAPTORS */

  size = rep.num_adaptors*sizeof(XvAdaptorInfo);
  if ((pas=(XvAdaptorInfo *)Xmalloc(size))==NULL)
    {
      Xfree(buffer);
      UnlockDisplay(dpy);
      SyncHandle();
      return(XvBadAlloc);
    }

  /* INIT ADAPTOR FIELDS */

  pa = pas;
  for (ii=0; ii<rep.num_adaptors; ii++)
    {
      pa->num_adaptors = 0;
      pa->name = (char *)NULL;
      pa->formats = (XvFormat *)NULL;
    }

  pa = pas;
  for (ii=0; ii<rep.num_adaptors; ii++)
    {
      pa->type = u.pa->type;
      pa->base_id = u.pa->base_id;
      pa->num_ports = u.pa->num_ports;
      pa->num_formats = u.pa->num_formats;
      pa->num_adaptors = rep.num_adaptors - ii;

      /* GET ADAPTOR NAME */

      size = u.pa->name_size;
      u.buffer += (sz_xvAdaptorInfo + 3) & ~3;

      if ( (name = (char *)Xmalloc(size+1)) == NULL)
	{
	  XvFreeAdaptorInfo(pas);
	  Xfree(buffer);
	  UnlockDisplay(dpy);
	  SyncHandle();
	  return(XvBadAlloc);
	}
      (void)strncpy(name, u.string, size);
      pa->name = name;

      u.buffer += (size + 3) & ~3;

      /* GET FORMATS */

      size = pa->num_formats*sizeof(XvFormat);
      if ((pfs=(XvFormat *)Xmalloc(size))==NULL)
	{
	  XvFreeAdaptorInfo(pas);
	  Xfree(buffer);
	  UnlockDisplay(dpy);
	  SyncHandle();
	  return(XvBadAlloc);
	}

      pf = pfs;
      for (ii=0; ii<pa->num_formats; ii++)
	{
	  pf->depth = u.pf->depth;
	  pf->visual_id = u.pf->visual;
	  pf++;
	  
	  u.buffer += (sz_xvFormat + 3) & ~3;
	}

      pa->formats = pfs;

      pa++;

    }

  *p_nAdaptors = rep.num_adaptors;
  *p_pAdaptors = pas;

  POSTAMBLE;

  return (Success);
}


void
XvFreeAdaptorInfo(pAdaptors)

XvAdaptorInfo *pAdaptors;

{

  XvEncodingInfo *pe;
  XvAdaptorInfo *pa;
  int ii;

  if (!pAdaptors) return;

  pa = pAdaptors;

  for (ii=0; ii<pAdaptors->num_adaptors; ii++, pa++)
    {
      if (pa->name)
	{
	  Xfree(pa->name);
	}
      if (pa->formats)
	{
	  Xfree(pa->formats);
	}
    } 

  Xfree(pAdaptors);

}


XvQueryEncodings(dpy, port, p_nEncodings, p_pEncodings)

register Display *dpy;
XvPortID port;
unsigned int *p_nEncodings;
XvEncodingInfo **p_pEncodings;
{
  register xvQueryEncodingsReq *req;
  xvQueryEncodingsReply rep;
  int size,ii,jj;
  char *name;
  XvEncodingInfo *pes, *pe;
  char *buffer;
  union 
    {
      char *buffer;
      char *string;
      xvEncodingInfo *pe;
    } u;
  
  PREAMBLE(XvBadExtension);

  XvGetReq(QueryEncodings, req);
  req->port = port;

  /* READ THE REPLY */

  if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) 
    {
      UnlockDisplay(dpy);
      SyncHandle();
      return(NULL);
    }

  size = rep.length << 2;
  if ( (buffer = (char *)Xmalloc ((unsigned) size)) == NULL)
    {
      UnlockMutex(&lock);
      return(XvBadAlloc);
    }
  _XRead (dpy, buffer, size);

  u.buffer = buffer;

  /* GET ENCODINGS */

  size = rep.num_encodings*sizeof(XvEncodingInfo);
  if ( (pes = (XvEncodingInfo *)Xmalloc(size)) == NULL)
    {
      Xfree(buffer);
      UnlockDisplay(dpy);
      SyncHandle();
      return(XvBadAlloc);
    }

  /* INITIALIZE THE ENCODING POINTER */

  pe = pes;
  for (jj=0; jj<rep.num_encodings; jj++)
    {
      pe->name = (char *)NULL;
      pe->num_encodings = 0;
      pe++;
    }

  pe = pes;
  for (jj=0; jj<rep.num_encodings; jj++)
    {
      pe->encoding_id = u.pe->encoding;
      pe->width = u.pe->width;
      pe->height = u.pe->height;
      pe->rate.numerator = u.pe->rate.numerator;
      pe->rate.denominator = u.pe->rate.denominator;
      pe->num_encodings = rep.num_encodings - jj;

      size = u.pe->name_size;
      u.buffer += (sz_xvEncodingInfo + 3) & ~3;

      if ( (name = (char *)Xmalloc(size+1)) == NULL)
	{
	  Xfree(buffer);
	  UnlockDisplay(dpy);
	  SyncHandle();
	  return(XvBadAlloc);
	}
      strncpy(name, u.string, size);
      pe->name = name;
      pe++;

      u.buffer += (size + 3) & ~3;

    }

  *p_nEncodings = rep.num_encodings;
  *p_pEncodings = pes;

  POSTAMBLE;

  return (Success);
}

void
XvFreeEncodingInfo(pEncodings)

XvEncodingInfo *pEncodings;

{

  XvEncodingInfo *pe;
  int ii;

  if (!pEncodings) return;

  pe = pEncodings;

  for (ii=0; ii<pEncodings->num_encodings; ii++, pe++)
    {
      if (pe->name) Xfree(pe->name);
    }

  Xfree(pEncodings);

}

XvPutVideo(dpy, port, d, gc, vx, vy, vw, vh, dx, dy, dw, dh)

register Display *dpy;
XvPortID port;
Drawable d;
GC gc;
int vx, vy, dx, dy;
unsigned int vw, vh;
unsigned int dw, dh;

{
  int size;
  register xvPutVideoReq *req;

  PREAMBLE(XvBadExtension);
  
  FlushGC(dpy, gc);

  XvGetReq(PutVideo, req);

  req->port = port;
  req->drawable = d;
  req->gc = gc->gid;
  req->vid_x = vx;
  req->vid_y = vy;
  req->vid_w = vw;
  req->vid_h = vh;
  req->drw_x = dx;
  req->drw_y = dy;
  req->drw_w = dw;
  req->drw_h = dh;

  POSTAMBLE;

  return Success;
}


XvPutStill(dpy, port, d, gc, vx, vy, vw, vh, dx, dy, dw, dh)

register Display *dpy;
XvPortID port;
Drawable d;
GC gc;
int vx, vy, dx, dy;
unsigned int vw, vh;
unsigned int dw, dh;

{
  int size;
  register xvPutStillReq *req;

  PREAMBLE(XvBadExtension);

  FlushGC(dpy, gc);

  XvGetReq(PutStill, req);
  req->port = port;
  req->drawable = d;
  req->gc = gc->gid;
  req->vid_x = vx;
  req->vid_y = vy;
  req->vid_w = vw;
  req->vid_h = vh;
  req->drw_x = dx;
  req->drw_y = dy;
  req->drw_w = dw;
  req->drw_h = dh;

  POSTAMBLE;

  return Success;
}

XvGetVideo(dpy, port, d, gc, vx, vy, vw, vh, dx, dy, dw, dh)

register Display *dpy;
XvPortID port;
Drawable d;
GC gc;
int vx, vy, dx, dy;
unsigned int vw, vh;
unsigned int dw, dh;

{
  int size;
  register xvGetVideoReq *req;

  PREAMBLE(XvBadExtension);

  FlushGC(dpy, gc);

  XvGetReq(GetVideo, req);
  req->port = port;
  req->drawable = d;
  req->gc = gc->gid;
  req->vid_x = vx;
  req->vid_y = vy;
  req->vid_w = vw;
  req->vid_h = vh;
  req->drw_x = dx;
  req->drw_y = dy;
  req->drw_w = dw;
  req->drw_h = dh;

  POSTAMBLE;

  return Success;
}

XvGetStill(dpy, port, d, gc, vx, vy, vw, vh, dx, dy, dw, dh)

register Display *dpy;
XvPortID port;
Drawable d;
GC gc;
int vx, vy, dx, dy;
unsigned int vw, vh;
unsigned int dw, dh;

{
  int size;
  register xvGetStillReq *req;

  PREAMBLE(XvBadExtension);

  FlushGC(dpy, gc);

  XvGetReq(GetStill, req);
  req->port = port;
  req->drawable = d;
  req->gc = gc->gid;
  req->vid_x = vx;
  req->vid_y = vy;
  req->vid_w = vw;
  req->vid_h = vh;
  req->drw_x = dx;
  req->drw_y = dy;
  req->drw_w = dw;
  req->drw_h = dh;

  POSTAMBLE;

  return Success;
}


XvStopVideo(dpy, port, draw)

register Display *dpy;
XvPortID port;
Drawable draw;

{
  register xvStopVideoReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(StopVideo, req);
  req->port = port;
  req->drawable = draw;

  POSTAMBLE;

  return Success;
}

XvGrabPort(dpy, port, time)
     register Display *dpy;
     XvPortID port;
     Time time;

{
  int result;
  xvGrabPortReply rep;
  register xvGrabPortReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(GrabPort, req);
  req->port = port;
  req->time = time;

  if (_XReply (dpy, (xReply *) &rep, 0, xTrue) == 0) 
    rep.result = GrabSuccess;

  result = rep.result;

  POSTAMBLE;

  return result;
}

XvUngrabPort(dpy, port, time)
     register Display *dpy;
     XvPortID port;
     Time time;

{
  register xvUngrabPortReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(UngrabPort, req);
  req->port = port;
  req->time = time;

  POSTAMBLE;

  return Success;
}

XvSelectVideoNotify(dpy, drawable, onoff)

register Display *dpy;
Drawable drawable;
Bool onoff;

{
  register xvSelectVideoNotifyReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(SelectVideoNotify, req);
  req->drawable = drawable;
  req->onoff = onoff;

  POSTAMBLE;

  return Success;
}

XvSelectPortNotify(dpy, port, onoff)

register Display *dpy;
XvPortID port;
Bool onoff;

{
  register xvSelectPortNotifyReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(SelectPortNotify, req);
  req->port = port;
  req->onoff = onoff;

  POSTAMBLE;

  return Success;
}


XvSetPortAttribute (dpy, port, attribute, value)
     register Display *dpy;
     XvPortID port;
     Atom attribute;
     int value;
{
  register xvSetPortAttributeReq *req;

  PREAMBLE(XvBadExtension);

  XvGetReq(SetPortAttribute, req);
  req->port = port;
  req->attribute = attribute;
  req->value = value;

  POSTAMBLE;

  return (Success);
}

XvGetPortAttribute (dpy, port, attribute, p_value)
     register Display *dpy;
     XvPortID port;
     Atom attribute;
     int *p_value;
{
  register xvGetPortAttributeReq *req;
  xvGetPortAttributeReply rep;

  PREAMBLE(XvBadExtension);

  XvGetReq(GetPortAttribute, req);
  req->port = port;
  req->attribute = attribute;

  /* READ THE REPLY */

  if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) 
    {
      UnlockDisplay(dpy);
      SyncHandle();
      return(XvBadReply);
    }

  *p_value = rep.value;
  
  POSTAMBLE;

  return (Success);
}

XvQueryBestSize(dpy, port, motion, vid_w, vid_h, drw_w, drw_h, 
		p_actual_width, p_actual_height)
     register Display *dpy;
     XvPortID port;
     Bool motion;
     unsigned int vid_w, vid_h;
     unsigned int drw_w, drw_h;
     unsigned int *p_actual_width, *p_actual_height;
{
  register xvQueryBestSizeReq *req;
  xvQueryBestSizeReply rep;

  PREAMBLE(XvBadExtension);

  XvGetReq(QueryBestSize, req);
  req->port = port;
  req->motion = motion;
  req->vid_w = vid_w;
  req->vid_h = vid_h;
  req->drw_w = drw_w;
  req->drw_h = drw_h;

  /* READ THE REPLY */

  if (_XReply(dpy, (xReply *)&rep, 0, xFalse) == 0) 
    {
      UnlockDisplay(dpy);
      SyncHandle();
      return(XvBadReply);
    }

  *p_actual_width = rep.actual_width;
  *p_actual_height = rep.actual_height;

  POSTAMBLE;

  return (Success);
}

static Bool
wire_to_event(dpy, re, event)
register Display *dpy;	/* pointer to display structure */
register XvEvent *re;	/* pointer to where event should be reformatted */
register xvEvent *event;	/* protocol event */

{

  XExtDisplayInfo *info = find_display(dpy);

  if (event->u.u.type == info->codes->first_event+XvVideoNotify)
    {
      re->xvvideo.type = event->u.u.type & 0x7f;
      re->xvvideo.serial = 
	_XSetLastRequestRead(dpy, (xGenericReply *)event);
      re->xvvideo.send_event = ((event->u.u.type & 0x80) != 0);
      re->xvvideo.display = dpy;
      re->xvvideo.time = event->u.videoNotify.time;
      re->xvvideo.reason = event->u.videoNotify.reason;
      re->xvvideo.drawable = event->u.videoNotify.drawable;
      re->xvvideo.port_id = event->u.videoNotify.port;
    }
  else if (event->u.u.type == info->codes->first_event+XvPortNotify)
    {
      re->xvport.type = event->u.u.type & 0x7f;
      re->xvport.serial = 
	_XSetLastRequestRead(dpy, (xGenericReply *)event);
      re->xvport.send_event = ((event->u.u.type & 0x80) != 0);
      re->xvport.display = dpy;
      re->xvport.time = event->u.portNotify.time;
      re->xvport.port_id = event->u.portNotify.port;
      re->xvport.attribute = event->u.portNotify.attribute;
      re->xvport.value = event->u.portNotify.value;
    }
  else
    {
      return (False);
    }


  return (True);
  
}
