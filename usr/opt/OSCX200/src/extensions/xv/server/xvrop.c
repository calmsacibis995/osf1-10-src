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
**   xvrop.c --- Xv RasterOps device dependent module.
**
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   18.09.92 Ludwig
**     - changes ultrix to be both ultrix and osf for dec. 
**     - changed from using tfb code to cfb32 code from Packard
**
**   26.11.91 Carver
**     - optimized PutStill to not redraw enable plane between like stills.
**     - fixed multi-screen support
**     - added new call to pip_installed()
**
**   18.09.91 Carver
**     - changed interface to libpip.c; now uses pip_src_area and pip_dst_area
**       instead of pip_source_area, pip_origin, and pip_size.
**     - fixed constrain procedure to clip video destination to screen bounds
**
**   10.09.91 Carver
**     - fixed calls to fill_video_enable some were overrunning bounds
**       of screen.
**
**   29.08.91 Carver
**     - change: unrealizing windows no longer preempts video
**     - added support for duty cycle (video priority) management
**     - added support for video in StaticGray windows
**     - added include of cfb.h; needed to access 8 plane gc priv structure
**     - changed interfaces to rop.c and libpip.c to use RopPtr uniformly
**     - changed ultrix conditional compile so this file works for both
**       sun and dec boards
**
**   26.06.91 Carver
**     - fixed GC wrapping logic in a big way
**     - fixed XvropValidateGC; it had the wrong argument definition
**       can't believe that it ever worked.
**     - fixed XvropStopVideo to check to see if the GC still exists
**       before it references a pointer to it.
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   04.06.91 Carver
**     - completed implementing port controls
**     - used code from R. Ulichney to to contrast and brightness calc's
**     - changed interface to libpip.c, use new libpip.h header file
**
**   31.05.91 Carver
**     - made big fixes to occlusion stuff
**     - fixed video image offset and size
**
**   23.05.91 Carver
**     - check for pip existance before initializing the adaptor
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   24.04.91 Carver
**     - updated to use 24 plane support.
**
**   19.03.91 Carver
**     - origional version coded to v1r4.
**
*/

#include <stdio.h>

#include <sys/types.h>

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "scrnintstr.h"
#include "validate.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "dixstruct.h"

#include "Xv.h"
#include "Xvproto.h"
#include "xvdix.h"

#if defined(ultrix) || defined(__osf__)
#include <sys/workstation.h>        /* ULTRIX WORKSTATION DRIVER HEADER */
#ifdef __osf__
#ifdef __alpha
#include "../../../server/ddx/dec/tx/pmagro.h"
#else
#include <io/dec/ws/pmagro.h>
#endif
#else
#include <io/ws/pmagro.h>           /* ULTRIX RASTEROPS DRIVER HEADER */
#endif
#endif /* ultrix or osf */
#include "cfb.h"                    /* 8 PLANE DDX HEADER */
extern int cfbGCPrivateIndex;	    /* 24 PLANE DDX */

#define GLOBAL
#include "xvrop.h"
#undef GLOBAL

/* INTERNAL */

static Bool XvropCloseScreen();
static int XvropQueryAdaptors();
static int XvropInitAdaptor();
static int XvropAllocatePort();
static int XvropFreePort();
static int XvropPutVideo();
static int XvropPutStill();
static int XvropGetVideo();
static int XvropGetStill();
static int XvropStopVideo();
static int XvropSetPortAttribute();
static int XvropGetPortAttribute();
static int XvropQueryBestSize();

static pointer XvropPipInit();

static void XvropClipNotify();
static void XvropCopyWindow();
static Bool XvropUnrealizeWindow();
static void XvropWindowExposures();

static void XvropDestroyGC();
static void XvropValidateGC();

/* NOTE THAT WE ONLY NEED TO WRAP ONE GC AT A TIME; FOR THIS REASON WE DON'T
   USE A GC PRIVATE STRUCTURE TO SAVE THE WRAP FUNCS; WHEN WE ARE FINISHED
   USING A GC FOR VIDEO WE WANT TO UNWRAP IT, BUT WE NEED TO BE IN A FUNC IN 
   ORDER TO UNWRAP IT, SO WE USE ChangeGC WITH A NULL MASK TO DO THE
   UNWRAPPING; ITS A HACK BUT SAVES UNNECESSARY ALLOCATIONS */

static void XvropValidateGC (),  XvropCopyGC (),      XvropDestroyGC();
static void XvropChangeGC();
static void XvropChangeClip(),   XvropDestroyClip(),  XvropCopyClip();

static GCFuncs  XvropGCFuncs = 
{
  XvropValidateGC,
  XvropChangeGC,
  XvropCopyGC,
  XvropDestroyGC,
  XvropChangeClip,
  XvropDestroyClip,
  XvropCopyClip
};

static XvPortPtr XvropPorts[MAXSCREENS];

int
XvropScreenInit(pScreen)

ScreenPtr pScreen;

{
  XvropScreenPtr props;
  XvScreenPtr pxvs;
  int status;

  if (!pip_installed(wsPhysScreenNum(pScreen)))
    return Success;

  /* ALL XV DD SCREEN INIT PROCS NEED TO CALL THE DI SCREEN INIT PROC FIRST */

  XvScreenInit(pScreen);

  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  if ((status = XvropInitAdaptor(pScreen)) != Success)
    return status;

  /* XXX --- THESE SHOULD BE CHAINED FOR MULTIPLE ADAPTOR SUPPORT */

  pxvs->ddCloseScreen = XvropCloseScreen;
  pxvs->ddQueryAdaptors = XvropQueryAdaptors;

  if (!(props = (XvropScreenPtr)xalloc(sizeof(XvropPortRec))))
    {
      return BadAlloc;
    }
  pxvs->devPriv.ptr = (pointer)props;

  props->WindowExposures = pScreen->WindowExposures;
  pScreen->WindowExposures = XvropWindowExposures;

  props->UnrealizeWindow = pScreen->UnrealizeWindow;
  pScreen->UnrealizeWindow = XvropUnrealizeWindow;

  props->CopyWindow = pScreen->CopyWindow;
  pScreen->CopyWindow = XvropCopyWindow;

  props->ClipNotify = pScreen->ClipNotify;
  pScreen->ClipNotify = XvropClipNotify;

  return Success;
}

static int
XvropInitAdaptor(pScreen)

ScreenPtr pScreen;

{
  int status,count,np,ne,nd,nv,nf,tf,na,nvis;
  XID id;
  XvScreenPtr pxvs;
  XvAdaptorPtr pa,pas;
  XvPortPtr pp,pps;
  XvropPortPtr ppp;
  XvEncodingPtr pe,pes;
  XvFormatPtr pf,pfs;
  unsigned long *pv;
  VisualPtr pvis;
  DepthPtr pd;
  pointer prop;

  if (!(prop = XvropPipInit(pScreen)))
    return Success;

  pxvs = (XvScreenPtr) pScreen->devPrivates[XvScreenIndex].ptr;

  pas = (XvAdaptorPtr)xalloc(sizeof(XvAdaptorRec));
  if (!pas)
    {
      FatalError("XvropInitAdaptors: coudn't alloc Adaptor struct\n");
    }
  pa = pas;

  for (na = 0; na<XVROP_NUM_ADAPTORS; na++)
    {

      pes = (XvEncodingPtr)xalloc(sizeof(XvEncodingRec)*XvropNumEncodings[na]);
      if (!pes)
	{
	  FatalError("XvropInitAdaptors: coudn't alloc Encoding struct\n");
	}

      pe = pes;

      for (ne = 0; ne<XvropNumEncodings[na]; ne++)
	{
	  *pe = XvropEncodings[na][ne];
	  pe->pScreen = pScreen;
	  
	  pe++;
	}

      pps = (XvPortPtr)xalloc(sizeof(XvPortRec)*XvropNumPorts[na]);
      if (!pps)
	{
	  FatalError("XvropInitAdaptors: coudn't alloc Port struct\n");
	}
      pp = pps;

      for (np = 0; np<XvropNumPorts[na]; np++)
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
	  pp->pNotify = (XvPortNotifyPtr)NULL;
	  ppp = (XvropPortPtr)xalloc(sizeof(XvropPortRec));
	  pp->devPriv.ptr = (pointer)ppp;

	  ppp->pGC = (GCPtr)NULL;
	  ppp->pEncoding = pes; /* XXX---SHOULD BE A COMMAND LINE ARG */
	  ppp->hue = 0;
	  ppp->saturation = 0;
	  ppp->brightness = 0;
	  ppp->contrast = 0;
	  ppp->serialNumber = 0;
	  ppp->pDrawStill = (DrawablePtr)NULL;

	  /* SAVE HANDLE TO DEVICE DRIVER */

	  ppp->prop = prop; 

	  /* SET DEFAULT ENCODING; NOT GRAY */

	  ppp->gray = FALSE;

	  pip_source_type(ppp->prop, ppp->pEncoding->id);

	  pp++;
	  
	}

      nf = 0;
      tf = XvropNumFormats[na];
      pfs = (XvFormatPtr)xalloc(sizeof(XvFormatRec)*tf);
      if (!pfs)
	{
	  FatalError("XvropInitAdaptors: couldn't alloc Format struct\n");
	}
      pf = pfs;

      pd = pScreen->allowedDepths;
      for (nd=0; nd<pScreen->numDepths; nd++)
	{
	  if (pd->depth == 24)
	    {
	      pv = pd->vids;
	      for (nv=0; nv<pd->numVids; nv++)
		{
		  if (nf >= tf) 
		    {
		      tf *= 2;
		      pfs = (XvFormatPtr)xrealloc(pfs, sizeof(XvFormatRec)*tf);
		      pf = &pfs[nf];
		    }
		  
		  pf->depth = pd->depth;
		  pf->visual = *pv;
		  pf++;
		  nf++;
		  pv++;
		}
	    }
	  pd++;
	}

      /* FIND STATIC GRAY VISUAL */

      pvis = pScreen->visuals;
      for (nvis=0; nvis<pScreen->numVisuals; nvis++)
	{
	  if (pvis->class == StaticGray) break;
	  pvis++;
	}

      /* IF WE FOUND ONE THEN FIND MATCHING DEPTH VISUALID */

      if (nvis < pScreen->numVisuals)
	{
	  pd = pScreen->allowedDepths;
	  for (nd=0; nd<pScreen->numDepths; nd++)
	    {
	      if (pd->depth == 8)
		{
		  pv = pd->vids;
		  for (nv=0; nv<pd->numVids; nv++)
		    {
		      if (pvis->vid == *pv)
			{
			  if (nf >= tf) 
			    {
			      tf *= 2;
			      pfs = (XvFormatPtr)
				xrealloc(pfs, sizeof(XvFormatRec)*tf);
			      pf = &pfs[nf];
			    }
			  pf->depth = pd->depth;
			  pf->visual = *pv;
			  pf++;
			  nf++;
			  break;
			}
		      pv++;
		    }
		}
	      pd++;
	    }
	}
      XvropNumFormats[na] = nf;

      pp = pps+na;
      pa->base_id = pp->id;
      pa->type = XvInputMask;
      pa->name = "RasterOps";
      pa->nEncodings = ne;
      pa->pEncodings = pes;
      pa->nPorts = np;
      pa->pPorts = pps;
      pa->nFormats = nf;
      pa->pFormats = pfs;
      pa->pScreen = pScreen;
      pa->ddAllocatePort = XvropAllocatePort;
      pa->ddFreePort = XvropFreePort;
      pa->ddPutVideo = XvropPutVideo;
      pa->ddPutStill = XvropPutStill;
      pa->ddGetVideo = XvropGetVideo;
      pa->ddGetStill = XvropGetStill;
      pa->ddStopVideo = XvropStopVideo;
      pa->ddSetPortAttribute = XvropSetPortAttribute;
      pa->ddGetPortAttribute = XvropGetPortAttribute;
      pa->ddQueryBestSize = XvropQueryBestSize;

      /* ALLOWS QUICK ACCESS TO PORTS BY ROP DD MODULE; THIS
         ASSUMES NO MORE THAN ONE ROP ADAPTOR PER SCREEN */

      XvropPorts[pScreen->myNum] = pps;
    }

  pxvs->nAdaptors = na;
  pxvs->pAdaptors = pas;

  /* ALLOCATE ATOMS FOR PORT ATTRIBUTES --- MORE LATER */

  XvropEncoding = MakeAtom("XV_ENCODING", strlen("XV_ENCODING"), xTrue);
  XvropHue = MakeAtom("XV_HUE", strlen("XV_HUE"), xTrue);
  XvropSaturation = MakeAtom("XV_SATURATION", strlen("XV_SATURATION"), xTrue);
  XvropBrightness = MakeAtom("XV_BRIGHTNESS", strlen("XV_BRIGHTNESS"), xTrue);
  XvropContrast = MakeAtom("XV_CONTRAST", strlen("XV_CONTRAST"), xTrue);

  return Success;

}

static pointer
XvropPipInit(pScreen)
ScreenPtr pScreen;

{
  pointer prop;

  /* XXX---INIT TO USE NTSC; THIS SHOULD BE SERVER OPTION */

  if (!(prop = (pointer)pip_init(wsPhysScreenNum(pScreen), 
	PIP_NTSC | PIP_COMPOSITE)))
    return prop;

  fill_video_enable(prop, 0, 0, pScreen->width, pScreen->height, 00);

  return prop;

}

static Bool
XvropCloseScreen(ii, pScreen)
int ii;
ScreenPtr pScreen;

{
  int np,na;
  XvAdaptorPtr pa;
  XvPortPtr pp;
  XvScreenPtr pxvs;
  XvropScreenPtr props;

  pxvs = (XvScreenPtr) pScreen->devPrivates[XvScreenIndex].ptr;
  props = (XvropScreenPtr)pxvs->devPriv.ptr;

  if (!pxvs) return TRUE;

  pScreen->WindowExposures = props->WindowExposures;
  pScreen->UnrealizeWindow = props->UnrealizeWindow;

  pScreen->CopyWindow = props->CopyWindow;
  pScreen->ClipNotify = props->ClipNotify;

  pa = pxvs->pAdaptors;

  for (na=0; na<pxvs->nAdaptors; na++, pa++)
    {
      if (pa->pFormats) xfree(pa->pFormats);
      if (pa->pEncodings) xfree(pa->pEncodings);
      pp = pa->pPorts;
      for (np=0; np<pa->nPorts; np++, pp++)
	{
	  if (pp->devPriv.ptr) pip_close(ii);
	}
      if (pa->pPorts) xfree(pa->pPorts);
    }

  xfree(pxvs->pAdaptors);
  pxvs->pAdaptors = (XvAdaptorPtr)NULL;

  return TRUE;
}

static int
XvropAllocatePort(port, pPort, ppPort)

unsigned long port;
XvPortPtr pPort;
XvPortPtr *ppPort;

{
  *ppPort = pPort;
  return Success;
}

static int
XvropFreePort(pPort)

XvPortPtr pPort;

{
  return Success;
}

static int
XvropQueryAdaptors(pScreen, p_nAdaptors, p_pAdaptors)

ScreenPtr pScreen;
XvAdaptorPtr *p_pAdaptors;
int *p_nAdaptors;

{

  XvScreenPtr pxvs;

  pxvs = (XvScreenPtr) pScreen->devPrivates[XvScreenIndex].ptr;

  *p_nAdaptors = pxvs->nAdaptors;
  *p_pAdaptors = pxvs->pAdaptors;

  return (Success);

}

static void
XvropConstrainVideo(pWin, pe,
		    vid_x, vid_y, vid_w, vid_h,
		    drw_x, drw_y, drw_w, drw_h,
		    p_vid_x, p_vid_y, p_vid_w, p_vid_h,
		    p_drw_x, p_drw_y, p_drw_w, p_drw_h)

WindowPtr pWin;
XvEncodingPtr pe;
INT16 vid_x, vid_y, drw_x, drw_y;
INT16 vid_w, vid_h, drw_w, drw_h;
INT16 *p_vid_x, *p_vid_y, *p_drw_x, *p_drw_y;
INT16 *p_vid_w, *p_vid_h, *p_drw_w, *p_drw_h;

{
  ScreenPtr pScreen;
  int u,v;

  /* NOTE WE ARE USING SIGNED VALUES FOR EXTENTS; THE CALCULATIONS BELOW COULD
     PRODUCE NEGATIVE VALUES WHICH SHOULD BE INTERPRETED AS A NULL VIDEO
     REGION */

  /* INITIALIZE RESULTS TO ZERO IN CASE WE ENCOUNTER A ZERO EXTENT SOMEWHERE
     ALONG THE WAY */

  /* USE A LOOP TO HANDLE SIZE ERROR */

  for (;;)
    {
      
      /* FIRST SEE IF VID X,Y ARE OUT OF BOUNDS; NULL REGION RESULTS */
      
      if (vid_x >= pe->width || vid_y >= pe->height || 
	  vid_w <= 0 || vid_h <= 0) break;
      
      /* CLIP LOWER BOUNDS OF SOURCE REGION */
      
      if (vid_x < 0)
	{
	  u = (drw_w*(vid_w + vid_x))/vid_w;
	  drw_x += drw_w - u;
	  drw_w = u;
	  vid_w += vid_x;
	  vid_x = 0;
	  if (drw_w <= 0 || vid_w <= 0) break;
	}
      
      if (vid_y < 0)
	{
	  u = (drw_h*(vid_h + vid_y))/vid_h;
	  drw_y += drw_h - u;
	  drw_h = u;
	  vid_h += vid_y;
	  vid_y = 0;
	  if (drw_h <= 0 || vid_h <= 0) break;
	}
      
      /* CLIP UPPER BOUNDS OF SOURCE REGION */
      
      if (vid_x + vid_w > pe->width)
	{
	  drw_w = (drw_w*(pe->width - vid_x))/vid_w;
	  vid_w = pe->width - vid_x;
	  if (drw_w <= 0 || vid_w <= 0) break;
	}
      
      if (vid_y + vid_h > pe->height)
	{
	  drw_h = (drw_h*(pe->height - vid_y))/vid_h;
	  vid_h = pe->height - vid_y;
	  if (drw_h <= 0 || vid_h <= 0) break;
	}
      
      /* CLIP THE DST AREA TO THE LOWER BOUNDS OF SCREEN */
      
      if (pWin->drawable.x + drw_x < 0)
	{
	  u = drw_w + (pWin->drawable.x + drw_x);
	  v = (vid_w*u)/drw_w;
	  vid_x += vid_w - v;
	  vid_w = v;
	  drw_x = -pWin->drawable.x;
	  drw_w = u;
	  if (drw_w <= 0 || vid_w <= 0) break;
	}
      
      if (pWin->drawable.y + drw_y < 0)
	{
	  if (drw_h <= 0) break;
	  u = drw_h + (pWin->drawable.y + drw_y);
	  v = (vid_h*u)/drw_h;
	  vid_y += vid_h - v;
	  vid_h = v;
	  drw_y = -pWin->drawable.y;
	  drw_h = u;
	  if (drw_h <= 0 || vid_h <= 0) break;
	}
      
      /* CLIP THE DST AREA TO THE UPPER BOUNDS OF SCREEN */
      
      pScreen = pWin->drawable.pScreen;
      
      if (pWin->drawable.x + drw_x + drw_w > pScreen->width)
	{
	  u = pScreen->width - pWin->drawable.x - drw_x;
	  vid_w = (vid_w*u)/drw_w;
	  drw_w = u;
	  if (drw_w <= 0 || vid_w <= 0) break;
	}
      
      if (pWin->drawable.y + drw_y + drw_h > pScreen->height)
	{
	  u = pScreen->height - pWin->drawable.y - drw_y;
	  vid_h = (vid_h*u)/drw_h;
	  drw_h = u;
	  if (drw_h <= 0 || vid_h <= 0) break;
	}
      
      /* CHECK DESTINATION SIZE AGAINST SOURCE VIDEO SIZE */
      
      if (drw_w > vid_w)
	{
	  drw_h = (drw_h * vid_w)/drw_w;
	  drw_w = vid_w;
	  if (drw_w <= 0 || vid_w <= 0) break;
	}
      
      if (drw_h > vid_h)
	{
	  drw_w = (drw_w * vid_h)/drw_h;
	  drw_h = vid_h;
	  if (drw_h <= 0 || vid_h <= 0) break;
	}
      
      *p_vid_x = vid_x;
      *p_vid_y = vid_y;
      *p_vid_w = vid_w;
      *p_vid_h = vid_h;
      *p_drw_x = drw_x;
      *p_drw_y = drw_y;
      *p_drw_w = drw_w;
      *p_drw_h = drw_h;

      return;

    }

  *p_vid_x = 0;
  *p_vid_y = 0;
  *p_vid_w = 0;
  *p_vid_h = 0;
  *p_drw_x = 0;
  *p_drw_y = 0;
  *p_drw_w = 0;
  *p_drw_h = 0;

}

static int
XvropPutVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	       drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  XvropPortPtr ppp;
  cfbPrivGCPtr pTFBGC;
  cfbPrivGCPtr pCFBGC;
  GCPtr pGCTemp;
  int vis,nBox,x1,x2,y1,y2;
  BoxPtr pBox;
  RegionPtr pClip;
  Bool src_area_changed = FALSE;
  Bool dst_area_changed = FALSE;

  if (pDraw->type != DRAWABLE_WINDOW)
    {
      pPort->pDraw = (DrawablePtr)NULL;
      return BadAlloc;
    }

  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  ppp->vx = vid_x;
  ppp->vy = vid_y;
  ppp->vw = vid_w;
  ppp->vh = vid_h;
  ppp->dx = drw_x;
  ppp->dy = drw_y;
  ppp->dw = drw_w;
  ppp->dh = drw_h;

  /* CONSTRAIN THE REGIONS TO THOSE THAT ARE SUPPORTED; MAINTAIN ASPECT
     RATIO WHEN POSSIBLE */

  XvropConstrainVideo(pDraw, ppp->pEncoding, 
		      vid_x, vid_y, vid_w, vid_h,
		      drw_x, drw_y, drw_w, drw_h,
		      &vid_x, &vid_y, &vid_w, &vid_h,
		      &drw_x, &drw_y, &drw_w, &drw_h);

  /* COMPARE AND SAVE CONSTRAINED PARAMETERS */

  if ((vid_x != ppp->cvx) || (vid_y != ppp->cvy) || 
      (vid_w != ppp->cvw) || (vid_h != ppp->cvh))
    src_area_changed = TRUE;
  
  if ((pDraw != pPort->pDraw) ||
      (drw_x != ppp->cdx) || (drw_y != ppp->cdy) || 
      (drw_w != ppp->cdw) || (drw_h != ppp->cdh) ||
      (pDraw->x != ppp->Dx) || (pDraw->y != ppp->Dy))
    dst_area_changed = TRUE;

  ppp->Dx = pDraw->x;
  ppp->Dy = pDraw->y;

  ppp->cvx = vid_x;
  ppp->cvy = vid_y;
  ppp->cvw = vid_w;
  ppp->cvh = vid_h;
  ppp->cdx = drw_x;
  ppp->cdy = drw_y;
  ppp->cdw = drw_w;
  ppp->cdh = drw_h;

  /* UNWRAP THE OLD GC PROCEDURES IF NECESSARY */

  if ((ppp->pGC) && (pGC != ppp->pGC))
    {
      pGCTemp = ppp->pGC;
      ppp->pGC = (GCPtr)NULL;
      ChangeGC(pGCTemp, 0, 0); /* USE CHANGE GC TO UNWRAP GC */
    }

  /* TURN PIP OFF SO WE CAN CHANGE PARAMETERS */

  if (dst_area_changed) pip_off_with_timeout(ppp->prop, 1);

  /* IF THE DRAWABLE DEPTH IS 8 WE KNOW IT MUST BE A StaticGray WINDOW */

  if (pDraw->depth == 8)
    {
      if (!ppp->gray)
	{
	  pip_source_type(ppp->prop, ppp->pEncoding->id | PIP_GRAY);
	  ppp->gray = TRUE;
	}

      /* GET COMPOSITE CLIP LIST */

      pCFBGC = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr; 
      pClip = pCFBGC->pCompositeClip;

    }
  else
    {
      if (ppp->gray)
	{
	  pip_source_type(ppp->prop, ppp->pEncoding->id);
	  ppp->gray = FALSE;
	}

      /* GET COMPOSITE CLIP LIST */

      pTFBGC = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr; 
      pClip = pTFBGC->pCompositeClip;

    }

  /* THIS IS A LITTLE UNCLEAR; BUT WE WANT TO UPDATE THE VIDEO MASK
     IF EITHER THE DRAWABLE OR GC HAS CHANGED; WE TRAP GC VALIDATES
     AND CHANGE THE PORT SERIAL NUMBER SO THAT WE KNOW IF THE GC
     HAS CHANGED */

  if ((pGC != ppp->pGC) || (pDraw->serialNumber != ppp->serialNumber))
    {

      ppp->serialNumber = pDraw->serialNumber;

      if ((pPort->pDraw) || (ppp->pDrawStill))
	{

	  /* CLEAR THE OLD VIDEO ENABLED REGION */

	  fill_video_enable(ppp->prop, 
			    ppp->enabled_box.x1, 
			    ppp->enabled_box.y1, 
			    ppp->enabled_box.x2 - ppp->enabled_box.x1,
			    ppp->enabled_box.y2 - ppp->enabled_box.y1, 00);
	}

      for (nBox=0, pBox=REGION_RECTS(pClip);
	   nBox<(int)REGION_NUM_RECTS(pClip); 
	   nBox++, pBox++)
	{
	  
	  x1 = pBox->x1;
	  y1 = pBox->y1;
	  x2 = pBox->x2;
	  y2 = pBox->y2;
	  
	  fill_video_enable(ppp->prop, x1, y1, x2 - x1, y2 - y1, 0xff);
	  
	}

      /* SAVE THE BOUNDING BOX OF THE ENABLED VIDEO REGION */

      ppp->enabled_box = pClip->extents;

    }

  ppp->pDrawStill = (DrawablePtr)NULL;

  vis = ((WindowPtr)pDraw)->visibility;

  if (ppp->vis != vis) 
    {
      ppp->vis = vis;
      if (vis == VisibilityUnobscured)
	pip_priority(ppp->prop, 0);
      else 
	pip_priority(ppp->prop, 256);
    }

  if (src_area_changed)
    pip_src_area(ppp->prop, vid_x, vid_y, vid_w, vid_h);

  if (dst_area_changed)
    pip_dst_area(ppp->prop, pDraw->x+drw_x, pDraw->y+drw_y, drw_w, drw_h);

  /* TURN PIP ON IF IT WASN'T LEFT ON */

  if (!pip_on_with_timeout(ppp->prop, 1))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvHardError);
      
      if (pGCTemp = ppp->pGC) {
	  ppp->pGC = (GCPtr)NULL;
	  ChangeGC(pGCTemp, 0, 0); 	/* USE CHANGE GC TO UNWRAP GC */
      }

      pPort->pDraw = (DrawablePtr)NULL;
      
      return Success;
    }

  /* WRAP THE NEW GC PROCEDURES IF NECESSARY */

  if (pGC != ppp->pGC)
    {
      ppp->pGC = pGC;
      ppp->wrapFuncs = pGC->funcs;
      pGC->funcs = &XvropGCFuncs;
    }

  pPort->pDraw = pDraw;

  return Success;
}

static int
XvropPutStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	       drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  XvropPortPtr ppp;
  XvEncodingPtr pe;
  cfbPrivGCPtr pTFBGC;
  cfbPrivGCPtr pCFBGC;
  int vis,nBox,x1,x2,y1,y2;
  BoxPtr pBox;
  RegionPtr pClip;
  GCPtr pGCTemp;
  Bool src_area_changed = FALSE;
  Bool dst_area_changed = FALSE;

  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  if (pDraw->type != DRAWABLE_WINDOW)
    {
      return BadAlloc;
    }

  /* OPTIMIZE FOR REPEATED STILLS */

  if (!pPort->pDraw)
    {
      /* SAVE PARAMETERS */
      
      ppp->pDrawStill = pDraw;

      ppp->vx = vid_x;
      ppp->vy = vid_y;
      ppp->vw = vid_w;
      ppp->vh = vid_h;
      ppp->dx = drw_x;
      ppp->dy = drw_y;
      ppp->dw = drw_w;
      ppp->dh = drw_h;

      /* IF NEW GC THEN UNWRAP OLD */

      if ((ppp->pGC) && (pGC != ppp->pGC))
	{
	  pGCTemp = ppp->pGC;
	  ppp->pGC = (GCPtr)NULL;
	  ChangeGC(pGCTemp, 0, 0); /* USE CHANGE GC TO UNWRAP GC */
	}
    }

  /* CONSTRAIN THE REGIONS TO THOSE THAT ARE SUPPORTED */

  pe = ppp->pEncoding;

  XvropConstrainVideo(pDraw, pe, 
		      vid_x, vid_y, vid_w, vid_h,
		      drw_x, drw_y, drw_w, drw_h,
		      &vid_x, &vid_y, &vid_w, &vid_h,
		      &drw_x, &drw_y, &drw_w, &drw_h);

  /* COMPARE AND SAVE CONSTRAINED PARAMETERS */

  if ((vid_x != ppp->cvx) || (vid_y != ppp->cvy) || 
      (vid_w != ppp->cvw) || (vid_h != ppp->cvh))
    src_area_changed = TRUE;
  
  if ((pDraw != pPort->pDraw) ||
      (drw_x != ppp->cdx) || (drw_y != ppp->cdy) || 
      (drw_w != ppp->cdw) || (drw_h != ppp->cdh) ||
      (pDraw->x != ppp->Dx) || (pDraw->y != ppp->Dy))
    dst_area_changed = TRUE;

  if (!pPort->pDraw)
    {
      /* SAVE CONSTRAINED PARAMETERS */

      ppp->Dx = pDraw->x;
      ppp->Dy = pDraw->y;
      
      ppp->cvx = vid_x;
      ppp->cvy = vid_y;
      ppp->cvw = vid_w;
      ppp->cvh = vid_h;
      ppp->cdx = drw_x;
      ppp->cdy = drw_y;
      ppp->cdw = drw_w;
      ppp->cdh = drw_h;
    }

  /* GET COMPOSITE CLIP LIST */

  pTFBGC = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr;

  /* TURN PIP OFF SO WE CAN CHANGE PIP PARAMETERS AND GET A CLEAN PICTURE */

  if (dst_area_changed) pip_off_with_timeout(ppp->prop, 1);

  /* IF THE DRAWABLE DEPTH IS 8 WE KNOW IT MUST BE A StaticGray WINDOW */

  if (pDraw->depth == 8)
    {
      if (!ppp->gray)
	{
	  pip_source_type(ppp->prop, ppp->pEncoding->id | PIP_GRAY);
	  ppp->gray = TRUE;
	}

      /* GET COMPOSITE CLIP LIST */

      pCFBGC = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr; 
      pClip = pCFBGC->pCompositeClip;

    }
  else
    {
      if (ppp->gray)
	{
	  pip_source_type(ppp->prop, ppp->pEncoding->id);
	  ppp->gray = FALSE;
	}

      /* GET COMPOSITE CLIP LIST */

      pTFBGC = (cfbPrivGCPtr)pGC->devPrivates[cfbGCPrivateIndex].ptr; 
      pClip = pTFBGC->pCompositeClip;
    } 

  /* THIS IS A LITTLE UNCLEAR; BUT WE WANT TO UPDATE THE VIDEO MASK
     IF EITHER THE DRAWABLE OR GC HAS CHANGED; WE TRAP GC VALIDATES
     AND CHANGE THE PORT SERIAL NUMBER SO THAT WE KNOW IF THE GC
     HAS CHANGED */

  if ((pGC != ppp->pGC) || (pDraw->serialNumber != ppp->serialNumber))
    {

      ppp->serialNumber = pDraw->serialNumber;

      if ((pPort->pDraw) || (ppp->pDrawStill))
	{
	  /* CLEAR THE OLD VIDEO ENABLED REGION */

	  fill_video_enable(ppp->prop, 
			    ppp->enabled_box.x1, 
			    ppp->enabled_box.y1, 
			    ppp->enabled_box.x2 - ppp->enabled_box.x1,
			    ppp->enabled_box.y2 - ppp->enabled_box.y1, 00);

	}

      for (nBox=0, pBox=REGION_RECTS(pClip);
	   nBox<(int)REGION_NUM_RECTS(pClip); 
	   nBox++, pBox++)
	{
	  
	  x1 = pBox->x1;
	  y1 = pBox->y1;
	  x2 = pBox->x2;
	  y2 = pBox->y2;
	  
	  fill_video_enable(ppp->prop, x1, y1, x2 - x1, y2 - y1, 0xff);
	  
	}

      /* SAVE THE BOUNDING BOX OF THE ENABLED VIDEO REGION, IF WE ARE 
	 DRAWING INTO THE VIDEO DRAWABLE OR IF THERE IS NO VIDEO DRAWABLE */

      if ((!pPort->pDraw) || (pDraw == pPort->pDraw))
	ppp->enabled_box = pClip->extents;

    }

  vis = ((WindowPtr)pDraw)->visibility;

  if (ppp->vis != vis) 
    {
      ppp->vis = vis;
      if (vis == VisibilityUnobscured)
	pip_priority(ppp->prop, 0);
      else 
	pip_priority(ppp->prop, 256);
    }

  /* SET THE POSITION OF THE VIDEO */

  if (src_area_changed)
    pip_src_area(ppp->prop, vid_x, vid_y, vid_w, vid_h);

  if (dst_area_changed)
    pip_dst_area(ppp->prop, pDraw->x+drw_x, pDraw->y+drw_y, drw_w, drw_h);

  /* IF NO VIDEO IS ACTIVE THEN SAVE PARAMETERS */

  if (!pip_on_with_timeout(ppp->prop, 1))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvHardError);
    }

  /* WAIT FOR THE ONE SHOT TO COMPLETE */

  pip_off_with_timeout(ppp->prop, 1);

  /* CLEAR THE ENABLE PLANE IF WE ARE GOING BACK TO ANOTHER DRAWABLE */

  if ((pPort->pDraw) && (pDraw != pPort->pDraw))
    {
      pBox = &((WindowPtr)pDraw)->winSize.extents;
      fill_video_enable(ppp->prop, pBox->x1, pBox->y1, 
			pBox->x2-pBox->x1, pBox->y2-pBox->y1, 00);
    }

  if (pGC != ppp->pGC)
    {
      ppp->serialNumber = NEXT_SERIAL_NUMBER;
    }

  /* RESTART VIDEO SINCE WE STOPPED IT */

  if (pPort->pDraw)
    {

      /* IF THE GC HAS BEEN DESTROYED THEN JUST PREEMPT THE VIDEO */

      if (!ppp->pGC)
	{
	  XvdiSendVideoNotify(pPort, pDraw, XvPreempted);
	  return Success;
	}

      if (ppp->pGC->serialNumber != pPort->pDraw->serialNumber)
	ValidateGC(pPort->pDraw, ppp->pGC);

      /* FAKEOUT THE PUTVIDEO SO IT WILL RESET SRC AND DST AREAS */

      ppp->cvx++;
      ppp->cdx++;

      XvropPutVideo((ClientPtr)NULL, pPort->pDraw, pPort, ppp->pGC, 
		    ppp->vx, ppp->vy, ppp->vw, ppp->vh,
		    ppp->dx, ppp->dy, ppp->dw, ppp->dh);

    }
  else
    {

      ppp->pDrawStill = pDraw;

      /* WRAP THE NEW GC PROCEDURES IF NECESSARY */
      
      if (pGC != ppp->pGC)
	{
	  ppp->pGC = pGC;
	  ppp->wrapFuncs = pGC->funcs;
	  pGC->funcs = &XvropGCFuncs;
	}
    }

  return Success;

}

static int
XvropGetVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  return BadMatch;
}

static int
XvropGetStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	      drw_x, drw_y, drw_w, drw_h)
ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  return BadMatch;
}


static int
XvropStopVideo(client, pPort, pDraw)
ClientPtr client;
XvPortPtr pPort;
DrawablePtr pDraw;

{
  XvropPortPtr ppp;
  GCPtr pGCTemp;

  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  /* CLEAR THE VIDEO ENABLE PLANE */

  pip_off_with_timeout(ppp->prop, 1);

  /* CLEAR THE VIDEO ENABLE PLANE */

  fill_video_enable(ppp->prop, 
		    ppp->enabled_box.x1, 
		    ppp->enabled_box.y1, 
		    ppp->enabled_box.x2 - ppp->enabled_box.x1,
		    ppp->enabled_box.y2 - ppp->enabled_box.y1, 00);

  /* UNWRAP GC DESTROY PROCEDURE IF NECESSARY */

  if (ppp->pGC)
    {
      pGCTemp = ppp->pGC;
      ppp->pGC = (GCPtr)NULL;
      ChangeGC(pGCTemp, 0, 0); /* USE CHANGE GC TO UNWRAP GC */
    }

  return Success;
}


static int
XvropSetPortAttribute(client, pPort, attribute, value)
ClientPtr client;
XvPortPtr pPort;
Atom attribute;
INT32 value;

{
  int ii;
  unsigned char lut[256];
  XvAdaptorPtr pa;
  XvEncodingPtr pe;
  XvropPortPtr ppp;
  float contrast, brightness;
  int on_off;
  int rop_value;

  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  on_off = pip_off(ppp->prop,1);

  /* FIND THE ATTRIBUTE AND SET IT; XXX --- MORE LATER */

  if (attribute == XvropEncoding)
    {
      pa = pPort->pAdaptor;
      pe = pa->pEncodings;
      for (ii=0; ii<pa->nEncodings; ii++)
	{
	  if (pe->id == (XvEncodingID)value)
	    {

	      /* SAVE NEW ENCODING ID */

	      ppp->pEncoding = pe;

	      if (ppp->gray)
		pip_source_type(ppp->prop, pe->id | PIP_GRAY);
	      else
		pip_source_type(ppp->prop, pe->id);

	      break;
	    }
	  pe++;
	}
      if (ii>=pa->nEncodings) return _XvBadEncoding;
    }
  else if (attribute == XvropHue)
    {

      if (value < -1000) value = -1000;
      if (value > 1000) value = 1000;

      /* HUE IS THE LOCATION AROUND THE COLOR CIRCLE */
      /* MAP 0..1000 INTO 0..127 AND -1..-1000 INTO 255..128 */

      if (value >= 0) rop_value = (127*value)/1000;
      else rop_value = (127*(1000+value))/999 + 128;

      pip_hue(ppp->prop,rop_value); 
      ppp->hue = value;
    }
  else if (attribute == XvropSaturation)
    {
      if (value < -1000) value = -1000;
      if (value > 1000) value = 1000;

      /* MAP -1000..1000 into 0..255 */

      pip_saturation(ppp->prop,(255*(value+1000))/2000); 
      ppp->saturation = value;
    }
  else if (attribute == XvropBrightness)
    {
      if (value < -1000) value = -1000;
      if (value > 1000) value = 1000;
      ppp->brightness = value;
#ifdef sun
      pip_brightness(ppp->prop, value);
#endif
#if defined(ultrix) || defined(__osf__)
      contrast = ppp->contrast;
      brightness = value;
      GetAdjustLUT(lut,contrast,brightness,0.0,FALSE,FALSE);
      pip_load_dcsc(ppp->prop,lut);
#endif
    }
  else if (attribute == XvropContrast)
    {
      if (value < -1000) value = -1000;
      if (value > 1000) value = 1000;
      ppp->contrast = value;
#ifdef sun
      pip_contrast(ppp->prop, value);
#endif
#if defined(ultrix) || defined(__osf__)
      contrast = value + 120.0; /* ADJUST FOR RASTEROPS VIDEO PIXEL RANGE */
      brightness = ppp->brightness;
      GetAdjustLUT(lut,contrast,brightness,0.0,FALSE,FALSE);
      pip_load_dcsc(ppp->prop,lut);
#endif
    }
  else
      return BadMatch;

  if (on_off) pip_on(ppp->prop);

  XvdiSendPortNotify(pPort, attribute, value);

  return Success;
}

static int
XvropGetPortAttribute(client, pPort, attribute, p_value)
ClientPtr client;
XvPortPtr pPort;
Atom attribute;
INT32 *p_value;

{

  XvropPortPtr ppp;
  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  /* FIND THE ATTRIBUTE AND RETURN ITS VALUE; XXX --- MORE LATER */

  if (attribute == XvropEncoding)
    {
      *p_value = ppp->pEncoding->id;
    }
  else if (attribute == XvropHue)
    {
      *p_value = ppp->hue;
    }
  else if (attribute == XvropSaturation)
    {
      *p_value = ppp->saturation;
    }
  else if (attribute == XvropBrightness)
    {
      *p_value = ppp->brightness;
    }
  else if (attribute == XvropContrast)
    {
      *p_value = ppp->contrast;
    }
  else 
      return BadMatch;

  return Success;

}

static int
XvropQueryBestSize(client, pPort, motion, vid_w, vid_h, drw_w, drw_h, p_w, p_h)
ClientPtr client;
XvPortPtr pPort;
CARD8 motion;
CARD16 vid_w,vid_h;
CARD16 drw_w,drw_h;
CARD16 *p_w, *p_h;

{
  XvropPortPtr ppp;
  XvEncodingPtr pe;
  int vid_x,vid_y,drw_x,drw_y;
  extern WindowPtr *WindowTable;

  vid_x = vid_y = drw_x = drw_y = 0;

  ppp = (XvropPortPtr)pPort->devPriv.ptr;
  pe = ppp->pEncoding;

  /* USE THE SAME GEOMETRY CONSTRAINT PROC THAT WE USE FOR GET AND PUT */

  XvropConstrainVideo(WindowTable[pPort->pAdaptor->pScreen->myNum], pe, 
		      vid_x, vid_y, vid_w, vid_h,
		      drw_x, drw_y, drw_w, drw_h,
		      &vid_x, &vid_y, &vid_w, &vid_h,
		      &drw_x, &drw_y, p_w, p_h);

  return Success;

}

static void
XvropDestroyGC(pGC)

GCPtr pGC;

{

  Bool status;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvropScreenPtr props;
  XvropPortPtr ppp;
  
  ppp = (XvropPortPtr)XvropPorts[pGC->pScreen->myNum]->devPriv.ptr;

  /* JUST CHECKING */

  if (ppp->pGC == pGC)
    {
      ppp->pGC = (GCPtr)NULL;
    }

  pGC->funcs = ppp->wrapFuncs;
  (* pGC->funcs->DestroyGC)(pGC);

  /* DON'T REWRAP THE FUNCS */

}

static void
XvropValidateGC(pGC, changes, pDraw)

GCPtr pGC;
Mask changes;
DrawablePtr pDraw;

{

  Bool status;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvropScreenPtr props;
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pGC->pScreen->myNum]->devPriv.ptr;

  pGC->funcs = ppp->wrapFuncs;

  (* pGC->funcs->ValidateGC)(pGC, changes, pDraw);

  /* save funcs as ValidateGC may have changed them (edg 30oct91) */

  ppp->wrapFuncs = pGC->funcs;

  /* IF THIS GC IS BEING USED FOR VIDEO THEN UPDATE THE VIDEO SERIAL NUMBER */

  if (ppp->pGC == pGC)
    {
      ppp->serialNumber = NEXT_SERIAL_NUMBER;
    }

  pGC->funcs = &XvropGCFuncs;

}

static void
XvropChangeGC (pGC, mask)
    GCPtr   pGC;
    unsigned long   mask;
{
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pGC->pScreen->myNum]->devPriv.ptr;

  pGC->funcs = ppp->wrapFuncs;

  (*pGC->funcs->ChangeGC) (pGC, mask);

  /* CHECK TO SEE IF GC IS STILL BEING USED FOR VIDEO; IF NOT THEN
     LEAVE IT UNWRAPPED */

  if (ppp->pGC == pGC)
    {
      pGC->funcs = &XvropGCFuncs;
    }

}

static void
XvropCopyGC (pGCSrc, mask, pGCDst)
    GCPtr   pGCSrc, pGCDst;
    unsigned long   mask;
{
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pGCSrc->pScreen->myNum]->devPriv.ptr;

  pGCDst->funcs = ppp->wrapFuncs;

  (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);

  pGCDst->funcs = &XvropGCFuncs;
}

static void
XvropChangeClip(pGC, type, pvalue, nrects)
    GCPtr       pGC;
    int         type;
    pointer     pvalue;
    int         nrects;
{
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pGC->pScreen->myNum]->devPriv.ptr;

  pGC->funcs = ppp->wrapFuncs;

  (* pGC->funcs->ChangeClip)(pGC, type, pvalue, nrects);

  pGC->funcs = &XvropGCFuncs;
}

static void
XvropCopyClip(pgcDst, pgcSrc)
    GCPtr pgcDst, pgcSrc;
{
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pgcDst->pScreen->myNum]->devPriv.ptr;

  pgcDst->funcs = ppp->wrapFuncs;

  (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);

  pgcDst->funcs = &XvropGCFuncs;
}

static void
XvropDestroyClip(pGC)
    GCPtr       pGC;
{
  XvropPortPtr ppp;

  ppp = (XvropPortPtr)XvropPorts[pGC->pScreen->myNum]->devPriv.ptr;

  pGC->funcs = ppp->wrapFuncs;

  (* pGC->funcs->DestroyClip)(pGC);

  pGC->funcs = &XvropGCFuncs;
}


static void
XvropClipNotify(pWin, dx, dy)

WindowPtr pWin;
int dx,dy;

{

  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvPortPtr pp;
  XvropScreenPtr props;
  Bool noExposures;

  pScreen = pWin->drawable.pScreen;

  pxvs = (XvScreenPtr)pWin->drawable.pScreen->devPrivates[XvScreenIndex].ptr;

  /* XXX---NEXT STATEMENT IS ONLY NECESSARY IF WE ARE USING miClipNotify */

  if (!pxvs) return;

  props = (XvropScreenPtr)pxvs->devPriv.ptr;

  pScreen->ClipNotify = props->ClipNotify;

  /* STOP VIDEO IN WINDOW IF NECESSARY */

  pp = XvropPorts[pWin->drawable.pScreen->myNum];

  if (pp->pDraw == (DrawablePtr)pWin)
    {

      XvropHaltVideo(pp);

    }

  if (pScreen->ClipNotify) (* pScreen->ClipNotify)(pWin, dx, dy);

  pScreen->ClipNotify = XvropClipNotify;

}

static void
XvropCopyWindow(pWin, lastposition, pRegionSrc)
WindowPtr pWin;
DDXPointRec lastposition;
RegionPtr pRegionSrc;

{

  ScreenPtr pScreen;
  WindowPtr pChild;
  XvScreenPtr pxvs;
  XvPortPtr pp;
  XvropScreenPtr props;

  pp = XvropPorts[pWin->drawable.pScreen->myNum];

  pScreen = pWin->drawable.pScreen;

  pxvs = (XvScreenPtr)pWin->drawable.pScreen->devPrivates[XvScreenIndex].ptr;
  props = (XvropScreenPtr)pxvs->devPriv.ptr;

  pScreen->CopyWindow = props->CopyWindow;

  pChild = pWin;
  while (1)
    {
      if (pChild->viewable)
	{
	  if (pp->pDraw == (DrawablePtr)pChild)
	    XvropHaltVideo(pp);

	  if (pChild->firstChild)
	    {
	      pChild = pChild->firstChild;
	      continue;
	    }
	}
      while (!pChild->nextSib && (pChild != pWin))
	pChild = pChild->parent;
      if (pChild == pWin)
	break;
      pChild = pChild->nextSib;
    }

  (* pScreen->CopyWindow)(pWin, lastposition, pRegionSrc);

  pScreen->CopyWindow = XvropCopyWindow;

}

static void
XvropWindowExposures(pWin, pReg, pOtherReg)
WindowPtr pWin;
RegionPtr pReg;
RegionPtr pOtherReg;

{

  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvropScreenPtr props;
  XvPortPtr pp;
  XvropPortPtr ppp;
  
  pScreen = pWin->drawable.pScreen;

  pxvs = (XvScreenPtr)pWin->drawable.pScreen->devPrivates[XvScreenIndex].ptr;
  props = (XvropScreenPtr)pxvs->devPriv.ptr;

  pScreen->WindowExposures = props->WindowExposures;

  pp = XvropPorts[pWin->drawable.pScreen->myNum];

  if ((pp->pDraw) && (((WindowPtr)pp->pDraw)->realized))
    {
      ppp = (XvropPortPtr)pp->devPriv.ptr;
      if (pp->pDraw->serialNumber != ppp->serialNumber)
	XvropRestartVideo(pp);
    }

  (* pScreen->WindowExposures)(pWin, pReg, pOtherReg);

  pScreen->WindowExposures = XvropWindowExposures;

}

static Bool
XvropUnrealizeWindow(pWin)
WindowPtr pWin;

{

  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvropScreenPtr props;
  XvPortPtr pp;
  int status;

  pScreen = pWin->drawable.pScreen;

  pxvs = (XvScreenPtr)pWin->drawable.pScreen->devPrivates[XvScreenIndex].ptr;
  props = (XvropScreenPtr)pxvs->devPriv.ptr;

  pScreen->UnrealizeWindow = props->UnrealizeWindow;

  pp = XvropPorts[pWin->drawable.pScreen->myNum];

  if (pp->pDraw == (DrawablePtr)pWin)
    {
      XvropHaltVideo(pp);
    }

  status = (* pScreen->UnrealizeWindow)(pWin);

  pScreen->UnrealizeWindow = XvropUnrealizeWindow;

  return status;

}

XvropHaltVideo(pPort)

XvPortPtr pPort;

{

  XvropPortPtr ppp;
  
  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  pip_off_with_timeout(ppp->prop, 1);
  
  /* CLEAR THE VIDEO ENABLE PLANE */

  fill_video_enable(ppp->prop, 
		    ppp->enabled_box.x1, 
		    ppp->enabled_box.y1, 
		    ppp->enabled_box.x2 - ppp->enabled_box.x1,
		    ppp->enabled_box.y2 - ppp->enabled_box.y1, 00);

}

XvropRestartVideo(pPort)

XvPortPtr pPort;

{
  XvropPortPtr ppp;
  
  ppp = (XvropPortPtr)pPort->devPriv.ptr;

  if (ppp->pGC)
    {

      if (ppp->pGC->serialNumber != pPort->pDraw->serialNumber)
	ValidateGC(pPort->pDraw, ppp->pGC);

      XvropPutVideo((ClientPtr)NULL, 
		    pPort->pDraw, pPort, ppp->pGC, 
		    ppp->vx, ppp->vy, ppp->vw, ppp->vh,
		    ppp->dx, ppp->dy, ppp->dw, ppp->dh);
    }
  else
    {
      XvdiSendVideoNotify(pPort, pPort->pDraw, XvPreempted);
    }

}


/*
** CARVER --- 04.06.91
**
**   I wanted to note that the following code, given to us by Robert
**   Ulichney, is used to control RasterOps contrast and brightness only.
**   Hue and saturation are not table driven in the RasterOps.
**
*/


/*
**++
**  FACILITY:
**
**	GetAdjustLUT, 
**	A fast routine to generate a general color component adjust LUT.
**
**  ABSTRACT:
**
**	This routine very quickly and efficiently generates a look-up
**	table of 256 input values to 256 output values to be used for
**	adjusting a color component of pixels in an image. It can be
**	used to adjust chromanance, as well as luminance values.
**
**	It is convenient to picture the mapping as a curve on a graph
**	where input values are along the "x" axis, and output values are
**	along the "y" axis. The identity or "no-change" function would
**	be a line of unity slope passing through the origin.
**
**	The arguments of the routine are described in detail in the code
**	comments. Inputs are contrast ("slope" in the case of luminace,
**	"saturation" or "chroma" in the case of chrominance), brightness
**	(y-offset), an x-offset to be used on chrominace components for
**	the purpose of changing the white point in combination with the
**	y-offset, and controls for negating the input and/or the output.
**
**
**
**
**  AUTHOR:
**
**	Robert Ulichney
**		Internet: ulichney@gauss.enet.dec.com
**		Phone:	  503-493-2503
**
**  CREATION DATE:	15-May-1991
**
**  MODIFICATION HISTORY:
**
**--
*/



/*
**  INCLUDE FILES: (none)
*/

/* 
**  MACRO DEFINITIONS: (none)
*/

/*
**  OTHER DEFINITIONS: 
*/

typedef int	boolean;	/* 0=false, 1=true */

/*	GetAdjustLUT	*/

GetAdjustLUT(AdjustLUT,contrast,brightness,Xoffset,negIn,negOut)
unsigned char	AdjustLUT[256];
float		contrast;	/* "Slope" of the transfer function.
				** 0 => no change, +1000 => vertical slope, 
				** -1000 => zero slope (flat).  While the 
				** the variable name is clear for use as a 
				** luminance adjustment, it takes on the 
				** meaning of "saturation" of "chroma" control
				** when used as an adjustment to a chrominance
				** value. The maximum allowable contrast
				** magnitude is 1000. 
				*/
float		brightness;	/* "Y" offset of the transfer function.
				** 0 => no change, +100 => increase all inputs 
				** by half the range, -100 => decreass all 
				** inputs by half the range. This can also
				** be used for chrominance adjustment in 
				** combination with Xoffset to change the 
				** white point. 
				** The maximum allowable brightness magnitude 
				** is 1000. Because most all adjustments will
				** involve magnitudes between 0 and 100, 
				** user interfaces, such as sliders, should
				** give as much vernier in this range as
				** for magnitudes between 100 and 1000. 
				** When this parameter is used to adjust white
				** point, the user interface should only provide
				** means to input magnitudes between 0 and 100.
				*/
float		Xoffset;	/* The same as brightness, but effecting a
				** translation of the transfer function in the 
				** "X" direction.  This is intended for use
				** with chrominance adjustment in combination
				** with brightness to change the white point. 
				*/
boolean		negIn;		/* A flag to indicate whether the input data is
				** to be negated BEFORE adjustment occurs. 
				** This would be necessary if the input data
				** intends 0 to represent white. 
				*/
boolean		negOut;		/* A flag to indicate whether the output data 
				** to be negated AFTER adjustment occurs.  This 
				** would be needed when the output device 
				** interprets 0 as white. 
				*/
				/* NOTE: If negation is to occur, it is 
				** important to correctly choose negIn or 
				** negOut, as they are not interchangeable.
				** A wronge choice will lead to adjustments
				** producing unexpected results. */
{	
	float		m;	/* slope */
	float		b;	/* y-intercept */
	int		x;	/* input value to AdjustLUT */
	float		y;	/* pre-truncated value y=m*x+b */
	int		maxV;	/* upper-bound for table-loading loop */
	int		lo;	/* x value at cross of y=m*x+b with y=0 */
	int		hi;	/* x value at cross of y=m*x+b with y=255 */

/* The strategy is to first form the equation of a line, y=m*x+b, and 
** establish the appropriate m and b. */

/* Range checks */
	/* Check for the special case of a vertical slope */
	if(contrast>999) contrast = 999; 
		/* slope = 1000 is effectively infinite */
	
	/* Correct for under-range contrast values */
	if(contrast<(-1000)) contrast = (-1000);  /* zero slope */

/* Find the slope, m */
	if(contrast>0)  m = 1000 / (1000-contrast);
	else		m = (1000+contrast)/1000;

/* Find the y-intercept */
/* The y-intercept, b, is influenced by three factors.
** First, contrast effectively rotates the line about a pivot at (127.5,127.5).
** In general, a line pivited at point (a,a) with slope m, has a y-intercept
** b=a*(1-m).  So, we start with b=127.5*(1-m).
** 
** Secondly, the effect of brightness adds (127.5/100)*brightness to b.
** The effect of brightness on a line y=m*x+b is:
** 		y = m*x + (b + (127.5/100)*brightness)
** 
** Thirdly, the effect of Xoffset on a line y=m*x+b is:
** 		y = m*(x-(127.5/100)*Xoffset) + b
**		  = m*x + (b - m*(127.5/100)*Xoffset)
** 
** Together, these three factors yield a y-intercept 
**	b = 127.5*(1-m) + (127.5/100)*brightness - m*(127.5/100)*Xoffset
** or, more simply:
*/
	b = (1.275)*(100 + brightness - m*(100+Xoffset));

/* Check if input data is to be negated.  If so, reflect the function about 
** the line x=127.5.  
** Note: In general, reflecting y=mx+b about the line x=a results in the line
** y=(-m)*x + (2*a*m+b).
*/
	if(negIn) {
		b = 255*m + b;
		m = (-m);
	}

/* Check if output data is to be negated.  If so, reflect the function about
** the line y=127.5.
** Note: In general, reflecting y=mx+b about the line y=c results in the line 
** y=(-m)*x + (2*c-b).
*/
	if(negOut) {
		b = 255 - b;
		m = (-m);
	}

/* Find the value of x at the intersection of the function y=mx+b with the
** lines at y=0 and y=255. These values of "lo" and "hi" will be used to more
** quickly generate the AdjustLUT with out needing to check for out-of-bounds
** at each value. */
	if(m != 0) {
		hi = (255-b)/m;		/* intersection with y=255 */
		lo = (-b/m);		/* intersection with y=0 */
	/* Note that both of these values are truncated to the integer x value 
	** just below the point of intersection.
	*/
	}
	else hi=1000, lo=(-1000);	/* For the case of zero slope,
					** force outside the range of (0,256).
					*/

/******************** Now load the table (finally!) *********************/
/* Treat the cases of positive and negative slope separately. */

/* Non-negative slope case */
	if(m>=0) {	
		if(lo>255) lo=255;	/* The entire table is clipped to 0. */

		/* Fill the LUT in order, from x=0 through x=255 */
		for(x=0; x<=lo; x++) AdjustLUT[x]=0; 	/* clip low values */

		maxV = (hi>255)? 255:hi; /* Fill the table with values of 
					** y=m*x+b only up to the point where
					** the function crosses the line y=255,
					** or, if it never reaches y=255, stop
					** at x=255. */
		for(y=m*x+b+.5; x<=maxV; x++,y+=m) AdjustLUT[x]=y;
		
		while(x<=255) AdjustLUT[x++]=255;	/* clip high values */
	}

/* Negative slope case */
	else {
		if(hi>255) hi=255;	/* The entire table is clipped to 255.*/

		/* Fill the LUT in order, from x=0 through x=255 */
		for(x=0; x<=hi; x++) AdjustLUT[x]=255; 	/* clip high values */

		maxV = (lo>255)? 255:lo; /* Fill the table with values of 
					** y=m*x+b only up to the point where
					** the function crosses the line y=0,
					** or, if it never reaches y=0, stop
					** at x=255. */
		for(y=m*x+b+.5; x<=maxV; x++,y+=m) AdjustLUT[x]=y;
		
		while(x<=255) AdjustLUT[x++]=0;	/* clip low values */
	}
		
}  /* End of GetAdjustLUT */
