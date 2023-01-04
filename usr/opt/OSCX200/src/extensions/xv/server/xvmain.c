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
**   xvmain.c --- Xv server extension main device independent module.
**   
** Author: 
**
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   04.09.91 Carver
**     - change: stop video always generates an event even when video
**       wasn't active
**
**   29.08.91 Carver
**     - change: unrealizing windows no longer preempts video
**
**   11.06.91 Carver
**     - changed SetPortControl to SetPortAttribute
**     - changed GetPortControl to GetPortAttribute
**     - changed QueryBestSize
**
**   28.05.91 Carver
**     - fixed Put and Get requests to not preempt operations to same drawable
**
**   15.05.91 Carver
**     - version 2.0 upgrade
**
**   19.03.91 Carver
**     - fixed Put and Get requests to honor grabbed ports.
**     - fixed Video requests to update di structure with new drawable, and
**       client after calling ddx.
**
**   24.01.91 Carver
**     - version 1.4 upgrade
**       
** Notes:
**
**   Port structures reference client structures in a two different
**   ways: when grabs, or video is active.  Each reference is encoded
**   as fake client resources and thus when the client is goes away so
**   does the reference (it is zeroed).  No other action is taken, so
**   video doesn't necessarily stop.  It probably will as a result of
**   other resources going away, but if a client starts video using
**   none of its own resources, then the video will continue to play
**   after the client disappears.
**
**
*/

#include <stdio.h>

#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "os.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "gc.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include "input.h"

#define GLOBAL

#include "Xv.h"
#include "Xvproto.h"
#include "xvdix.h"

/* EXTERNAL */

extern Atom MakeAtom();
extern WindowPtr *WindowTable;
extern XID clientErrorValue;
extern void (* EventSwapVector[128])();

static void WriteSwappedVideoNotifyEvent();
static void WriteSwappedPortNotifyEvent();
static Bool CreateResourceTypes();

static Bool XvScreens[MAXSCREENS];


/*
** XvExtensionInit
**
**
*/

void 
XvExtensionInit()
{
  int ii;
  register ExtensionEntry *extEntry;

  /* LOOK TO SEE IF ANY SCREENS WERE INITIALIZED; IF NOT THEN
     INIT GLOBAL VARIABLES SO THE EXTENSION CAN FUNCTION */

  if (XvScreenGeneration != serverGeneration)
    {
      if (!CreateResourceTypes())
	{
	  ErrorF("XvExtensionInit: Unable to allocate resource types\n");
	  return;
	}
      XvScreenIndex = AllocateScreenPrivateIndex ();
      if (XvScreenIndex < 0)
	{
	  ErrorF("XvExtensionInit: Unable to allocate screen private index\n");
	  return;
	}

      XvScreenGeneration = serverGeneration;
    }

  /* 
   * For loadable server, initialize screens from list of available 
   * device dependent components.
   * For static server, do what we can.
   */
  for (ii=0; ii<screenInfo.numScreens; ii++)  {
      XvScreens[ii] = FALSE;
      XvQueryAndLoadScreen(screenInfo.screens[ii]);
  }

  for (ii=0; ii<screenInfo.numScreens; ii++)
    if (!XvScreens[ii]) 
      screenInfo.screens[ii]->devPrivates[XvScreenIndex].ptr = (pointer)NULL;

#ifdef nomore

  /* ITS A LITTLE HARD TO UNDERSTAND WHAT THIS DOES, BUT ESSENTIALLY I WANT 
     ALL SCREEN THAT HAVE ADAPTORS TO HAVE A VALID DEVPRIVATE POINTER AND ANY 
     THAT DON'T TO HAVE NULL DEVPRIVATE POINTERS; THIS WAY I DON'T HAVE TO 
     CALL A SPECIAL XV SCREEN INITIALIZE FOR SCREENS WITHOUT ADAPTORS; THERE
     SHOULD BE AN EASIER WAY TO DO THIS, BUT main.c DOESN'T INITIALIZE THE 
     DEVPRIVATE POINTERS FOR A NEWLY CREATED SCREEN, NOR ARE THEY INITIALIZED
     WHEN A NEW SCREEN PRIVATE INDEX IS ALLOCATED */

  for (ii=lastScreenWithAdaptors; ii<screenInfo.numScreens; ii++)
    screenInfo.screens[ii]->devPrivates[XvScreenIndex].ptr = (pointer)NULL;
  lastScreenWithAdaptors = screenInfo.numScreens;

#endif

  if (XvExtensionGeneration != serverGeneration)
    {
      XvExtensionGeneration = serverGeneration;

      extEntry = AddExtension(XvName, XvNumEvents, XvNumErrors, 
			      ProcXvDispatch, SProcXvDispatch,
			      XvResetProc, StandardMinorOpcode);
      if (!extEntry) 
	{
	  FatalError("XvExtensionInit: AddExtensions failed\n");
	}

      XvReqCode = extEntry->base;
      XvEventBase = extEntry->eventBase;
      XvErrorBase = extEntry->errorBase;

      EventSwapVector[XvEventBase+XvVideoNotify] = 
	WriteSwappedVideoNotifyEvent;
      EventSwapVector[XvEventBase+XvPortNotify] = 
	WriteSwappedPortNotifyEvent;

      (void)MakeAtom(XvName, strlen(XvName), xTrue);

    }
}

static Bool
CreateResourceTypes()

{
  
  if (XvResourceGeneration == serverGeneration) return TRUE;

  XvResourceGeneration = serverGeneration;

  if (!(XvRTPort = CreateNewResourceType(XvdiDestroyPort)))
    {
      ErrorF("CreateResourceTypes: failed to allocate port resource.\n");
      return FALSE;
    }
  
  if (!(XvRTGrab = CreateNewResourceType(XvdiDestroyGrab)))
    {
      ErrorF("CreateResourceTypes: failed to allocate grab resource.\n");
      return FALSE;
    }
  
  if (!(XvRTEncoding = CreateNewResourceType(XvdiDestroyEncoding)))
    {
      ErrorF("CreateResourceTypes: failed to allocate encoding resource.\n");
      return FALSE;
    }
  
  if (!(XvRTVideoNotify = CreateNewResourceType(XvdiDestroyVideoNotify)))
    {
      ErrorF("CreateResourceTypes: failed to allocate video notify resource.\n");
      return FALSE;
    }
  
  if (!(XvRTVideoNotifyList = CreateNewResourceType(XvdiDestroyVideoNotifyList)))
    {
      ErrorF("CreateResourceTypes: failed to allocate video notify list resource.\n");
      return FALSE;
    }

  if (!(XvRTPortNotify = CreateNewResourceType(XvdiDestroyPortNotify)))
    {
      ErrorF("CreateResourceTypes: failed to allocate port notify resource.\n");
      return FALSE;
    }

  return TRUE;

}

int
XvScreenInit(pScreen)
ScreenPtr pScreen;

{

  int ii;
  XvScreenPtr pxvs;

  if (XvScreenGeneration != serverGeneration)
    {
      if (!CreateResourceTypes())
	{
	  ErrorF("XvScreenInit: Unable to allocate resource types\n");
	  return BadAlloc;
	}
      XvScreenIndex = AllocateScreenPrivateIndex ();
      if (XvScreenIndex < 0)
	{
	  ErrorF("XvScreenInit: Unable to allocate screen private index\n");
	  return BadAlloc;
	}
      XvScreenGeneration = serverGeneration; 
    }

  XvScreens[pScreen->myNum] = TRUE;

#ifdef nomore

  /* ITS A LITTLE HARD TO UNDERSTAND WHAT THIS DOES, BUT ESSENTIALLY I WANT 
     ALL SCREEN THAT HAVE ADAPTORS TO HAVE A VALID DEVPRIVATE POINTER AND ANY 
     THAT DON'T TO HAVE NULL DEVPRIVATE POINTERS; THIS WAY I DON'T HAVE TO 
     CALL A SPECIAL XV SCREEN INITIALIZE FOR SCREENS WITHOUT ADAPTORS; THERE
     SHOULD BE AN EASIER WAY TO DO THIS, BUT main.c DOESN'T INITIALIZE THE 
     DEVPRIVATE POINTERS FOR A NEWLY CREATED SCREEN, NOR ARE THEY INITIALIZED
     WHEN A NEW SCREEN PRIVATE INDEX IS ALLOCATED */

  for (ii=lastScreenWithAdaptors; ii<screenInfo.numScreens; ii++)
    screenInfo.screens[ii]->devPrivates[XvScreenIndex].ptr = (pointer)NULL;
  lastScreenWithAdaptors = screenInfo.numScreens;

  if (pScreen->devPrivates[XvScreenIndex].ptr)
    {
      ErrorF("XvScreenInit: screen devPrivates ptr non-NULL before init\n");
    }

#endif

  /* ALLOCATE SCREEN PRIVATE RECORD */
  
  pxvs = (XvScreenPtr) xalloc (sizeof (XvScreenRec));
  if (!pxvs)
    {
      ErrorF("XvScreenInit: Unable to allocate screen private structure\n");
      return BadAlloc;
    }
  
  pxvs->DestroyPixmap = pScreen->DestroyPixmap;
  pxvs->DestroyWindow = pScreen->DestroyWindow;
  pxvs->CloseScreen = pScreen->CloseScreen;
  
  pScreen->DestroyPixmap = XvDestroyPixmap;
  pScreen->DestroyWindow = XvDestroyWindow;
  pScreen->CloseScreen = XvCloseScreen;

  pScreen->devPrivates[XvScreenIndex].ptr = (pointer)pxvs;

  return Success;

}

Bool
XvCloseScreen(ii, pScreen)
int ii;
ScreenPtr pScreen;

{

  XvScreenPtr pxvs;

  pxvs = (XvScreenPtr) pScreen->devPrivates[XvScreenIndex].ptr;

  pScreen->DestroyPixmap = pxvs->DestroyPixmap;
  pScreen->DestroyWindow = pxvs->DestroyWindow;
  pScreen->CloseScreen = pxvs->CloseScreen;

  (* pxvs->ddCloseScreen)(ii, pScreen);

  xfree(pxvs);

  pScreen->devPrivates[XvScreenIndex].ptr = (pointer)NULL;

  return (*pScreen->CloseScreen)(ii, pScreen);

}

void
XvResetProc()
{
  int ii;

  XvUnLoadScreens();

  for (ii=0; ii<screenInfo.numScreens; ii++)
    XvScreens[ii] = FALSE;

}

Bool
XvDestroyPixmap(pPix)

PixmapPtr pPix;

{
  Bool status;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvAdaptorPtr pa;
  int na;
  XvPortPtr pp;
  int np;

  pScreen = pPix->drawable.pScreen;

  SCREEN_PROLOGUE(pScreen, DestroyPixmap);

  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  /* CHECK TO SEE IF THIS PORT IS IN USE */

  pa = pxvs->pAdaptors;
  na = pxvs->nAdaptors;
  while (na--)
    {
      np = pa->nPorts;
      pp = pa->pPorts;

      while (np--)
	{
	  if (pp->pDraw == (DrawablePtr)pPix)
	    {
	      XvdiSendVideoNotify(pp, pp->pDraw, XvPreempted);

	      (void)(* pp->pAdaptor->ddStopVideo)((ClientPtr)NULL, pp, 
						  pp->pDraw);

	      pp->pDraw = (DrawablePtr)NULL;
	      pp->client = (ClientPtr)NULL;
	      pp->time = currentTime;
	    }
	  pp++;
	}
      pa++;
    }
  
  status = (* pScreen->DestroyPixmap)(pPix);

  SCREEN_EPILOGUE(pScreen, DestroyPixmap, XvDestroyPixmap);

  return status;

}

Bool
XvDestroyWindow(pWin)

WindowPtr pWin;

{
  Bool status;
  ScreenPtr pScreen;
  XvScreenPtr pxvs;
  XvAdaptorPtr pa;
  int na;
  XvPortPtr pp;
  int np;

  pScreen = pWin->drawable.pScreen;

  SCREEN_PROLOGUE(pScreen, DestroyWindow);

  pxvs = (XvScreenPtr)pScreen->devPrivates[XvScreenIndex].ptr;

  /* CHECK TO SEE IF THIS PORT IS IN USE */

  pa = pxvs->pAdaptors;
  na = pxvs->nAdaptors;
  while (na--)
    {
      np = pa->nPorts;
      pp = pa->pPorts;

      while (np--)
	{
	  if (pp->pDraw == (DrawablePtr)pWin)
	    {
	      XvdiSendVideoNotify(pp, pp->pDraw, XvPreempted);

	      (void)(* pp->pAdaptor->ddStopVideo)((ClientPtr)NULL, pp, 
						  pp->pDraw);

	      pp->pDraw = (DrawablePtr)NULL;
	      pp->client = (ClientPtr)NULL;
	      pp->time = currentTime;
	    }
	  pp++;
	}
      pa++;
    }
  
  status = (* pScreen->DestroyWindow)(pWin);

  SCREEN_EPILOGUE(pScreen, DestroyWindow, XvDestroyWindow);

  return status;

}

/* The XvdiVideoStopped procedure is a hook for the device dependent layer.
   It provides a way for the dd layer to inform the di layer that video has
   stopped in a port for reasons that the di layer had no control over; note
   that it doesn't call back into the dd layer */

int
XvdiVideoStopped(pPort, reason)

XvPortPtr pPort;
int reason;

{
  
  /* IF PORT ISN'T ACTIVE THEN WE'RE DONE */

  if (!pPort->pDraw) return Success;

  XvdiSendVideoNotify(pPort, pPort->pDraw, reason);

  pPort->pDraw = (DrawablePtr)NULL;
  pPort->client = (ClientPtr)NULL;
  pPort->time = currentTime;

  return Success;

}

int 
XvdiDestroyPort(pPort, pid)
XvPortPtr pPort;
XID pid;
{
  return (* pPort->pAdaptor->ddFreePort)(pPort);
}

int
XvdiDestroyGrab(pGrab, id)
XvGrabPtr pGrab;
XID id;
{
  pGrab->client = (ClientPtr)NULL;
}

int
XvdiDestroyVideoNotify(pn, id)
XvVideoNotifyPtr pn;
XID id;

{
  /* JUST CLEAR OUT THE client POINTER FIELD */

  pn->client = (ClientPtr)NULL;
}

int
XvdiDestroyPortNotify(pn, id)
XvPortNotifyPtr pn;
XID id;

{
  /* JUST CLEAR OUT THE client POINTER FIELD */

  pn->client = (ClientPtr)NULL;
}

int
XvdiDestroyVideoNotifyList(pn, id)
XvVideoNotifyPtr pn;
XID id;

{
  XvVideoNotifyPtr npn,cpn;

  /* ACTUALLY DESTROY THE NOTITY LIST */

  cpn = pn;

  while (cpn)
    {
      npn = cpn->next;
      if (cpn->client) FreeResource(cpn->id, XvRTVideoNotify);
      xfree(cpn);
      cpn = npn;
    }
}

int
XvdiDestroyEncoding(pe, id)
XvEncodingPtr pe;
XID id;
{
}

int
XvdiSendVideoNotify(pPort, pDraw, reason)

XvPortPtr pPort;
DrawablePtr pDraw;
int reason;

{
  xvEvent event;
  XvVideoNotifyPtr pn;

  pn = (XvVideoNotifyPtr)LookupIDByType(pDraw->id, XvRTVideoNotifyList);

  while (pn) 
    {
      if (pn->client)
	{
	  event.u.u.type = XvEventBase + XvVideoNotify;
	  event.u.u.sequenceNumber = pn->client->sequence;
	  event.u.videoNotify.time = currentTime.milliseconds;
	  event.u.videoNotify.drawable = pDraw->id;
	  event.u.videoNotify.port = pPort->id;
	  event.u.videoNotify.reason = reason;
	  (void) TryClientEvents(pn->client, &event, 1, NoEventMask,
				 NoEventMask, NullGrab);
	}
      pn = pn->next;
    }

  return Success;

}


int
XvdiSendPortNotify(pPort, attribute, value)

XvPortPtr pPort;
Atom attribute;
INT32 value;

{
  xvEvent event;
  XvPortNotifyPtr pn;

  pn = pPort->pNotify;

  while (pn) 
    {
      if (pn->client)
	{
	  event.u.u.type = XvEventBase + XvPortNotify;
	  event.u.u.sequenceNumber = pn->client->sequence;
	  event.u.portNotify.time = currentTime.milliseconds;
	  event.u.portNotify.port = pPort->id;
	  event.u.portNotify.attribute = attribute;
	  event.u.portNotify.value = value;
	  (void) TryClientEvents(pn->client, &event, 1, NoEventMask,
				 NoEventMask, NullGrab);
	}
      pn = pn->next;
    }

  return Success;

}

int
XvdiPutVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	     drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  int status, id;
  DrawablePtr pOldDraw;

  /* UPDATE TIME VARIABLES FOR USE IN EVENTS */

  UpdateCurrentTime();

  /* CHECK FOR GRAB; IF THIS CLIENT DOESN'T HAVE THE PORT GRABBED THEN
     INFORM CLIENT OF ITS FAILURE */

  if (pPort->grab.client && (pPort->grab.client != client))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvBusy);
      return Success;
    }

  /* CHECK TO SEE IF PORT IS IN USE; IF SO THEN WE MUST DELIVER INTERRUPTED
     EVENTS TO ANY CLIENTS WHO WANT THEM */

  pOldDraw = pPort->pDraw;
  if ((pOldDraw) && (pOldDraw != pDraw))
    {
      XvdiSendVideoNotify(pPort, pPort->pDraw, XvPreempted);
    }

  status = (* pPort->pAdaptor->ddPutVideo)(client, pDraw, pPort, pGC, 
					   vid_x, vid_y, vid_w, vid_h, 
					   drw_x, drw_y, drw_w, drw_h);

  if ((pPort->pDraw) && (pOldDraw != pDraw))
    {
      pPort->client = client;
      XvdiSendVideoNotify(pPort, pPort->pDraw, XvStarted);
    }

  pPort->time = currentTime;

  return (Success);

}

int
XvdiPutStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	     drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  int status, id;

  /* UPDATE TIME VARIABLES FOR USE IN EVENTS */

  UpdateCurrentTime();

  /* CHECK FOR GRAB; IF THIS CLIENT DOESN'T HAVE THE PORT GRABBED THEN
     INFORM CLIENT OF ITS FAILURE */

  if (pPort->grab.client && (pPort->grab.client != client))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvBusy);
      return Success;
    }

  pPort->time = currentTime;

  status = (* pPort->pAdaptor->ddPutStill)(client, pDraw, pPort, pGC, 
					   vid_x, vid_y, vid_w, vid_h, 
					   drw_x, drw_y, drw_w, drw_h);

  return status;

}
int
XvdiGetVideo(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	     drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  int status, id;
  DrawablePtr pOldDraw;

  /* UPDATE TIME VARIABLES FOR USE IN EVENTS */

  UpdateCurrentTime();

  /* CHECK FOR GRAB; IF THIS CLIENT DOESN'T HAVE THE PORT GRABBED THEN
     INFORM CLIENT OF ITS FAILURE */

  if (pPort->grab.client && (pPort->grab.client != client))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvBusy);
      return Success;
    }

  /* CHECK TO SEE IF PORT IS IN USE; IF SO THEN WE MUST DELIVER INTERRUPTED
     EVENTS TO ANY CLIENTS WHO WANT THEM */

  pOldDraw = pPort->pDraw;
  if ((pOldDraw) && (pOldDraw != pDraw))
    {
      XvdiSendVideoNotify(pPort, pPort->pDraw, XvPreempted);
    }

  status = (* pPort->pAdaptor->ddGetVideo)(client, pDraw, pPort, pGC,
					   vid_x, vid_y, vid_w, vid_h, 
					   drw_x, drw_y, drw_w, drw_h);

  if ((pPort->pDraw) && (pOldDraw != pDraw))
    {
      pPort->client = client;
      XvdiSendVideoNotify(pPort, pPort->pDraw, XvStarted);
    }

  pPort->time = currentTime;

  return (Success);

}

int
XvdiGetStill(client, pDraw, pPort, pGC, vid_x, vid_y, vid_w, vid_h,
	     drw_x, drw_y, drw_w, drw_h)

ClientPtr client;
DrawablePtr pDraw;
XvPortPtr pPort;
GCPtr pGC;
INT16 vid_x, vid_y, drw_x, drw_y;
CARD16 vid_w, vid_h, drw_w, drw_h;

{
  int status, id;

  /* UPDATE TIME VARIABLES FOR USE IN EVENTS */

  UpdateCurrentTime();

  /* CHECK FOR GRAB; IF THIS CLIENT DOESN'T HAVE THE PORT GRABBED THEN
     INFORM CLIENT OF ITS FAILURE */

  if (pPort->grab.client && (pPort->grab.client != client))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvBusy);
      return Success;
    }

  status = (* pPort->pAdaptor->ddGetStill)(client, pDraw, pPort, pGC, 
					   vid_x, vid_y, vid_w, vid_h, 
					   drw_x, drw_y, drw_w, drw_h);

  pPort->time = currentTime;

  return status;

}

int
XvdiGrabPort(client, pPort, ctime, p_result)

ClientPtr client;
XvPortPtr pPort;
Time ctime;
int *p_result;

{
  XID id;
  TimeStamp time;

  UpdateCurrentTime();
  time = ClientTimeToServerTime(ctime);

  if (pPort->grab.client && (client != pPort->grab.client))
    {
      *p_result = XvAlreadyGrabbed;
      return Success;
    }

  if ((CompareTimeStamps(time, currentTime) == LATER) ||
      (CompareTimeStamps(time, pPort->time) == EARLIER))
    {
      *p_result = XvInvalidTime;
      return Success;
    }

  if (client == pPort->grab.client)
    {
      *p_result = Success;
      return Success;
    }

  id = FakeClientID(client->index);

  if (!AddResource(id, XvRTGrab, &pPort->grab))
    {
      return BadAlloc;
    }

  /* IF THERE IS ACTIVE VIDEO THEN STOP IT */

  if ((pPort->pDraw) && (client != pPort->client))
    {
      XVCALL(diStopVideo)((ClientPtr)NULL, pPort, pPort->pDraw);
    }

  pPort->grab.client = client;
  pPort->grab.id = id;

  pPort->time = currentTime;

  *p_result = Success;

  return Success;

}

int
XvdiUngrabPort(client, pPort, ctime)

ClientPtr client;
XvPortPtr pPort;
Time ctime;

{
  TimeStamp time;

  UpdateCurrentTime();
  time = ClientTimeToServerTime(ctime);

  if ((!pPort->grab.client) || (client != pPort->grab.client))
    {
      return Success;
    }

  if ((CompareTimeStamps(time, currentTime) == LATER) ||
      (CompareTimeStamps(time, pPort->time) == EARLIER))
    {
      return Success;
    }

  /* FREE THE GRAB RESOURCE; AND SET THE GRAB CLIENT TO NULL */

  FreeResource(pPort->grab.id, XvRTGrab);
  pPort->grab.client = (ClientPtr)NULL;

  pPort->time = currentTime;

  return Success;

}


int
XvdiSelectVideoNotify(client, pDraw, onoff)

ClientPtr client;
DrawablePtr pDraw;
BOOL onoff;

{
  register int ii;
  int id;
  XvVideoNotifyPtr pn,tpn,fpn;

  /* FIND VideoNotify LIST */

  pn = (XvVideoNotifyPtr)LookupIDByType(pDraw->id, XvRTVideoNotifyList);

  /* IF ONE DONES'T EXIST AND NO MASK, THEN JUST RETURN */

  if (!onoff && !pn) return Success;

  /* IF ONE DOESN'T EXIST CREATE IT AND ADD A RESOURCE SO THAT THE LIST
     WILL BE DELETED WHEN THE DRAWABLE IS DESTROYED */

  if (!pn) 
    {
      if (!(tpn = (XvVideoNotifyPtr)xalloc(sizeof(XvVideoNotifyRec))))
	return BadAlloc;
      tpn->next = (XvVideoNotifyPtr)NULL;
      if (!AddResource(pDraw->id, XvRTVideoNotifyList, tpn))
	{
	  xfree(tpn);
	  return BadAlloc;
	}
    }
  else
    {
      /* LOOK TO SEE IF ENTRY ALREADY EXISTS */

      fpn = (XvVideoNotifyPtr)NULL;
      tpn = pn;
      while (tpn)
	{
	  if (tpn->client == client) 
	    {
	      if (!onoff) tpn->client = (ClientPtr)NULL;
	      return Success;
	    }
	  if (!tpn->client) fpn = tpn; /* TAKE NOTE OF FREE ENTRY */
	  tpn = tpn->next;
	}

      /* IF TUNNING OFF, THEN JUST RETURN */

      if (!onoff) return Success;

      /* IF ONE ISN'T FOUND THEN ALLOCATE ONE AND LINK IT INTO THE LIST */

      if (fpn)
	{
	  tpn = fpn;
	}
      else
	{
	  if (!(tpn = (XvVideoNotifyPtr)xalloc(sizeof(XvVideoNotifyRec))))
	    return BadAlloc;
	  tpn->next = pn->next;
	  pn->next = tpn;
	}
    }

  /* INIT CLIENT PTR IN CASE WE CAN'T ADD RESOURCE */
  /* ADD RESOURCE SO THAT IF CLIENT EXITS THE CLIENT PTR WILL BE CLEARED */

  tpn->client = (ClientPtr)NULL;
  tpn->id = FakeClientID(client->index);
  AddResource(tpn->id, XvRTVideoNotify, tpn);

  tpn->client = client;
  return Success;

}

int
XvdiSelectPortNotify(client, pPort, onoff)

ClientPtr client;
XvPortPtr pPort;
BOOL onoff;

{
  register int ii;
  int id;
  XvPortNotifyPtr pn,tpn;

  /* SEE IF CLIENT IS ALREADY IN LIST */

  tpn = (XvPortNotifyPtr)NULL;
  pn = pPort->pNotify;
  while (pn)
    {
      if (!pn->client) tpn = pn; /* TAKE NOTE OF FREE ENTRY */
      if (pn->client == client) break;
      pn = pn->next;
    }

  /* IS THE CLIENT ALREADY ON THE LIST? */

  if (pn)
    {
      /* REMOVE IT? */

      if (!onoff)
	{
	  pn->client = (ClientPtr)NULL;
	  FreeResource(pn->id, XvRTPortNotify);
	}

      return Success;
    }

  /* DIDN'T FIND IT; SO REUSE LIST ELEMENT IF ONE IS FREE OTHERWISE 
     CREATE A NEW ONE AND ADD IT TO THE BEGINNING OF THE LIST */

  if (!tpn)
    {
      if (!(tpn = (XvPortNotifyPtr)xalloc(sizeof(XvPortNotifyRec))))
	return BadAlloc;
      tpn->next = pPort->pNotify;
      pPort->pNotify = tpn;
    }

  tpn->client = client;
  tpn->id = FakeClientID(client->index);
  AddResource(tpn->id, XvRTPortNotify, tpn);

  return Success;

}

int
XvdiStopVideo(client, pPort, pDraw)

ClientPtr client;
XvPortPtr pPort;
DrawablePtr pDraw;

{
  int status;

  /* IF PORT ISN'T ACTIVE THEN WE'RE DONE */

  if (!pPort->pDraw || (pPort->pDraw != pDraw)) 
    {
      XvdiSendVideoNotify(pPort, pDraw, XvStopped);
      return Success;
    }

  /* CHECK FOR GRAB; IF THIS CLIENT DOESN'T HAVE THE PORT GRABBED THEN
     INFORM CLIENT OF ITS FAILURE */

  if ((client) && (pPort->grab.client) && (pPort->grab.client != client))
    {
      XvdiSendVideoNotify(pPort, pDraw, XvBusy);
      return Success;
    }

  XvdiSendVideoNotify(pPort, pDraw, XvStopped);

  status = (* pPort->pAdaptor->ddStopVideo)(client, pPort, pDraw);

  pPort->pDraw = (DrawablePtr)NULL;
  pPort->client = (ClientPtr)client;
  pPort->time = currentTime;

  return status;

}

int
XvdiPreemptVideo(client, pPort, pDraw)

XvPortPtr pPort;
DrawablePtr pDraw;

{
  int status;

  /* IF PORT ISN'T ACTIVE THEN WE'RE DONE */

  if (!pPort->pDraw || (pPort->pDraw != pDraw)) return Success;

  XvdiSendVideoNotify(pPort, pPort->pDraw, XvPreempted);

  status = (* pPort->pAdaptor->ddStopVideo)(client, pPort, pPort->pDraw);

  pPort->pDraw = (DrawablePtr)NULL;
  pPort->client = (ClientPtr)client;
  pPort->time = currentTime;

  return status;

}

XvdiMatchPort(pPort, pDraw)
XvPortPtr pPort;
DrawablePtr pDraw;

{

  XvAdaptorPtr pa;
  XvFormatPtr pf;
  int nf;

  pa = pPort->pAdaptor;

  if (pa->pScreen != pDraw->pScreen) return BadMatch;

  nf = pa->nFormats;
  pf = pa->pFormats;

  while (nf--)
    {
      if ((pf->depth == pDraw->depth) &&
	  ((pDraw->type == DRAWABLE_PIXMAP) || 
	   (wVisual(((WindowPtr)pDraw)) == pf->visual)))
	return Success;
      pf++;
    }

  return BadMatch;

}

int
XvdiSetPortAttribute(client, pPort, attribute, value)
ClientPtr client;
XvPortPtr pPort;
Atom attribute;
INT32 value;

{

  return 
    (* pPort->pAdaptor->ddSetPortAttribute)(client, pPort, attribute, value);

}

int
XvdiGetPortAttribute(client, pPort, attribute, p_value)
ClientPtr client;
XvPortPtr pPort;
Atom attribute;
INT32 *p_value;

{

  return 
    (* pPort->pAdaptor->ddGetPortAttribute)(client, pPort, attribute, p_value);

}

static void
WriteSwappedVideoNotifyEvent(from, to)

xvEvent *from, *to;

{

  to->u.u.type = from->u.u.type;
  to->u.u.detail = from->u.u.detail;
  cpswaps(from->u.videoNotify.sequenceNumber, 
	  to->u.videoNotify.sequenceNumber);
  cpswapl(from->u.videoNotify.time, to->u.videoNotify.time);
  cpswapl(from->u.videoNotify.drawable, to->u.videoNotify.drawable);
  cpswapl(from->u.videoNotify.port, to->u.videoNotify.port);

}

static void
WriteSwappedPortNotifyEvent(from, to)

xvEvent *from, *to;

{

  to->u.u.type = from->u.u.type;
  to->u.u.detail = from->u.u.detail;
  cpswaps(from->u.portNotify.sequenceNumber, to->u.portNotify.sequenceNumber);
  cpswapl(from->u.portNotify.time, to->u.portNotify.time);
  cpswapl(from->u.portNotify.port, to->u.portNotify.port);
  cpswapl(from->u.portNotify.attribute, to->u.portNotify.attribute);
  cpswapl(from->u.portNotify.value, to->u.portNotify.value);

}
