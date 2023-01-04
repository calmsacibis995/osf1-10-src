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
**   xvplx.c --- Xv Parallax device dependent module.
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   03.11.91 Carver
**     - added code to restart video after stills.  Change port init to
**       include pointer to devPriv struct. See xvplx.h changes.
**
**   03.07.91 Carver
**     - changed procedure XvplxStopVideo to call plxVideoStop rather
**       than plxVideoProc which needed a GC that doesn't exist.
**
** Notes: 
**
**   This Xv dd module binds to the Parallax video extension.  This
**   allowed me to get things running quickly, though not everything
**   in the parallax extension behaves as I need it to.
**
*/

#include <stdio.h>

#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include "Xv.h"
#include "Xvproto.h"
#include "xvdix.h"

#define GLOBAL
#include "xvplx.h"
#undef GLOBAL

#include "plxvideo.h"

/* EXTERNAL */

extern DepthRec plxdepths[];

/* INTERNAL */

static Bool XvplxCloseScreen();
static int XvplxQueryScreen();
static int XvplxInitAdaptors();

static int XvplxAllocatePort();
static int XvplxFreePort();
static int XvplxPutVideo();
static int XvplxPutStill();
static int XvplxGetVideo();
static int XvplxGetStill();
static int XvplxStopVideo();
static int XvplxSetPortControls();
static int XvplxQueryBestSize();

int
XvddScreenInit(pScreen)

ScreenPtr pScreen;

{
  XvScreenPtr pxvs;
  int status;

  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  if ((status = XvplxInitAdaptors(pScreen)) != Success) 
    return status;

  pxvs->ddCloseScreen = XvplxCloseScreen;
  pxvs->ddQueryScreen = XvplxQueryScreen;

  return Success;

}

static int
XvplxInitAdaptors(pScreen)

ScreenPtr pScreen;

{
  int status,count,np,ne,nd,nv,nf,na;
  XID id;
  XvAdaptorPtr pa;
  XvPortPtr pp;
  XvEncodingPtr pe;
  XvFormatPtr pf;
  unsigned long *pv;
  DepthPtr pd;

  pa = XvplxAdaptors;
  for (na = 0; na<XVPLX_NUM_ADAPTORS; na++)
    {

      pe = XvplxEncodings[na];
      for (ne = 0; ne<XvplxNumEncodings[na]; ne++)
	{
	  id = FakeClientID(0);
	  if (!AddResource(id, XvRTEncoding, pe))
	    {
	      FatalError("XvddInitScreen: coudn't add Encoding resource\n");
	    }
	  pe->id = id;
	  pe->pScreen = pScreen;
	  pe->name = "-NTSC-";
	  pe->width = VIDEO_MAX_WIDTH;
	  pe->height = VIDEO_MAX_HEIGHT;
	  pe->rate.numerator = 5994;
	  pe->rate.denominator = 100;
	  
	  pe++;
	}

      pp = XvplxPorts[na];
      for (np = 0; np<XvplxNumPorts[na]; np++)
	{
	  id = FakeClientID(0);
	  
	  if (!AddResource(id, XvRTPort, pp))
	    {
	      FatalError("XvddInitScreen: couldn't add Port resource\n");
	    }
	  
	  /* MANUALLY FILL IN PORT STRUCTURE */
	  
	  pp->id = id;
	  pp->pAdaptor = pa;
	  pp->pDraw = (DrawablePtr)NULL;
	  pp->grab.client = (ClientPtr)NULL;
	  pp->time = currentTime;
	  pp->controls.pEncoding = XvplxEncodings[na];
	  pp->controls.hue = 0;
	  pp->controls.saturation = 0;
	  pp->controls.brightness = 0;
	  pp->controls.contrast = 0;
	  pp->devPriv.ptr = (pointer)&XvplxPortPrivs[na][np];

	  pp++;
	  
	}

      /* ASSUME THAT ALL VISUAL TYPES SUPPORTED BY PARALLAX DDX ARE SUPPORTED
	 BY THE VIDEO EXTENSION */

      pf = XvplxFormats[na];
      nf = 0;

      pd = pScreen->allowedDepths;
      for (nd=0; nd<pScreen->numDepths; nd++)
	{
	  pv = pd->vids;
	  for (nv=0; nv<pd->numVids; nv++)
	    {
	      if (nf >= XvplxNumFormats[na]) break;
	      pf->depth = pd->depth;
	      pf->visual = *pv;
	      pf++;
	      nf++;
	      pv++;
	    }
	  pd++;
	}
      XvplxNumFormats[na] = nf;

      pa->base_id = XvplxPorts[0][na].id;
      pa->type = XvInputMask;
      pa->nEncodings = XvplxNumEncodings[na];
      pa->pEncodings = XvplxEncodings[na];
      pa->nPorts = XvplxNumPorts[na];
      pa->pPorts = XvplxPorts[na];
      pa->nFormats = XvplxNumFormats[na];
      pa->pFormats = XvplxFormats[na];
      pa->select = XvManual;
      pa->pScreen = pScreen;
      pa->ddAllocatePort = XvplxAllocatePort;
      pa->ddFreePort = XvplxFreePort;
      pa->ddPutVideo = XvplxPutVideo;
      pa->ddPutStill = XvplxPutStill;
      pa->ddGetVideo = XvplxGetVideo;
      pa->ddGetStill = XvplxGetStill;
      pa->ddStopVideo = XvplxStopVideo;
      pa->ddSetPortControls = XvplxSetPortControls;
      pa->ddQueryBestSize = XvplxQueryBestSize;

    }

  return Success;

}


static Bool
XvplxCloseScreen(ii, pScreen)
int ii;
ScreenPtr pScreen;

{

  return TRUE;

}

static int
XvplxQueryScreen(pScreen, p_version, p_revision,
		 p_nAdaptors, p_pAdaptors)

ScreenPtr pScreen;
int *p_version, *p_revision;
XvAdaptorPtr *p_pAdaptors;
int *p_nAdaptors;

{

  *p_version = XvProtocolVersion;
  *p_revision = XvProtocolRevision;

  *p_nAdaptors = XVPLX_NUM_ADAPTORS;
  *p_pAdaptors = XvplxAdaptors;

  return (Success);

}


static int
XvplxAllocatePort(port, pPort, p_pPort)

unsigned long port;
XvPortPtr pPort,*p_pPort;

{
  return Success;
}

static int
XvplxFreePort(pPort)

XvPortPtr pPort;

{
  return Success;
}



static int
XvplxPutVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{

  xVideoArgs xva;
  XvplxPortPtr ppp;

  ppp = (XvplxPortPtr)pPort->devPriv.ptr;

  ppp->pGC = pGC;
  ppp->vx = vid_x;
  ppp->vy = vid_y;
  ppp->vw = vid_w;
  ppp->vh = vid_h;
  ppp->dx = drw_x;
  ppp->dy = drw_y;
  ppp->dw = drw_w;
  ppp->dh = drw_h;

  /* CALL PLXVIDEO EXTENSION */

  xva.vx = vid_x;
  xva.vy = vid_y;
  xva.vw = vid_w;
  xva.vh = vid_h;
  xva.x = drw_x;
  xva.y = drw_y;
  xva.w = drw_w;
  xva.h = drw_h;

  if ((drw_w != vid_w) || (drw_h != vid_h))
    return BadValue;

  return plxVideoProc(pDraw, pGC, X_VideoLive, &xva);

}

static int
XvplxPutStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  int status;
  xVideoArgs xva;
  XvplxPortPtr ppp;

  ppp = (XvplxPortPtr)pPort->devPriv.ptr;

  /* CALL PLXVIDEO EXTENSION */

  xva.vx = vid_x;
  xva.vy = vid_y;
  xva.vw = vid_w;
  xva.vh = vid_h;
  xva.x = drw_x;
  xva.y = drw_y;
  xva.w = drw_w;
  xva.h = drw_h;

  if ((drw_w != vid_w) || (drw_h != vid_h))
    {
      status = plxVideoProc(pDraw, pGC, X_VideoScale, &xva);
    }
  else
    {
      status = plxVideoProc(pDraw, pGC, X_VideoStill, &xva);
    }

  /* RESTART VIDEO SINCE PUTTING A STILL STOPS IT */

  if (pPort->pDraw)
    {
      xva.vx = ppp->vx;
      xva.vy = ppp->vy;
      xva.vw = ppp->vw;
      xva.vh = ppp->vh;
      xva.x = ppp->dx;
      xva.y = ppp->dy;
      xva.w = ppp->dw;
      xva.h = ppp->dh;

      if (plxVideoProc(pPort->pDraw, ppp->pGC, X_VideoLive, &xva) != Success)
	{
	  XvdiVideoStopped(pPort, XvPreempted);
	}
    }
  return status;
}

static int
XvplxGetVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
}

static int
XvplxGetStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
}


static int
XvplxStopVideo(client, pPort, pDraw)
ClientPtr client;
XvPortPtr pPort;
DrawablePtr pDraw;

{

  return plxVideoStop(pDraw);

}


static int
XvplxSetPortControls(client, pPort, mask, pval)
ClientPtr client;
XvPortPtr pPort;
BITS32 mask;
XID *pval;

{
  /* THIS PROCEEDURE SHOULD UPDATE THE PORT STRUCTURE */

  return Success;
}

static int
XvplxQueryBestSize(client, pPort, motion, w, h, p_lw, p_lh, p_uw, p_uh)
ClientPtr client;
XvPortPtr pPort;
CARD8 motion;
CARD16 w,h;
unsigned int *p_lw, *p_lh, *p_uw, *p_uh;

{

  if (motion)
    {
      *p_lw = VIDEO_MAX_WIDTH;
      *p_lh = VIDEO_MAX_HEIGHT;
      *p_uw = VIDEO_MAX_WIDTH;
      *p_uh = VIDEO_MAX_HEIGHT;
      return Success;
    }
  else
    {

      if (w >= VIDEO_MAX_WIDTH)
	{
	  *p_lw = VIDEO_MAX_WIDTH;
	  *p_uw = VIDEO_MAX_WIDTH;
	}
      else if (w < 16)
	{
	  *p_lw = 16;
	  *p_uw = 16;
	}
      else if (w & 15)
	{
	  *p_lw = w;
	  *p_uw = w+16;
	}
      else
	{
	  *p_lw = w & ~15;
	  *p_uw = (w + 15) & ~15;
	}

      if (h >= VIDEO_MAX_HEIGHT)
	{
	  *p_lh = VIDEO_MAX_HEIGHT;
	  *p_uh = VIDEO_MAX_HEIGHT;
	}
      else if (h < 1)
	{
	  *p_lh = 1;
	  *p_uh = 1;
	}
      else
	{
	  *p_lh = h;
	  *p_uh = h+1;
	}

    }

  return Success;

}

