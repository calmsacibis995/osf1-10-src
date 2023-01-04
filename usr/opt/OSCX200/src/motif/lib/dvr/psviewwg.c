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
#define Module PSVIEWWG
#define Ident "V03-006"

/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988, 1992 BY                      *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
** THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      PS Previewer widget -- DECwindows widget for viewing PostScript 
**
**  AUTHOR:
**
**      
**	Terry Weissman
**	Joel Gringorten
**
**  MAJOR MODIFICATIONS BY TO SUPPORT VMS AND CDA VIEWER BETTER
**
**	Burns Fisher
**
**  ABSTRACT:
**
**      This module contains the widget
**
**  ENVIRONMENT:
**
**      User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	NAME		DATE		REASON
**	----		----		------
**	Stephen Munyan	28-Jun-1990     Conversion to Motif
**      dam             01-Oct-1990     remove #ifdef __vms__'s to fix problem
**					viewing last page on ultrix
**      dam             21-Oct-1990     restore x error handler rtn correctly
**	dam		28-mar-1991	remove excess includes
**	dam		03-apr-1991	cleanup includes
**	dam		06-may-1991	make sure file is closed when set-file
**					is called with NULL file spec
**	dam		15-may-1991	correct proto to match decl
**	dam		10-jun-1991	update error abort to match uws
**	sjm		27-Jun-1991	DEC C Cleanups also added Module and Ident
**	dam		18-jul-1991	ask for ZOMBIE events from server
**					as well; fix needed to fix error
**					reporting as of vms decw v3
**
**	dam		05-aug-1991	rename headers, remove dollar signs
**	dam		12-aug-1991	remove message on zombie state, should
**					not be needed
**	jjt		16-sep-1991	remove XtIsRealized local definition, 
**					not needed now that NULL is back to 0.
**	dam		03-oct-1991	cleanup x calls to match protos
**	rmm		06-Apr-1992	Changes to support incremental reading
**					of the PS file.
**--
**/
/*
 * Main widget code for the Postscript Previewer Widget.
 */

#include <cdatrans.h>				

#ifdef __vms__
#include <cdaityp.h>				/* CDA Type definitions to get the PROTO macro */
#pragma nostandard /* turn off /stand=port for "nonclean" X includes */
#endif

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */
#include <Mrm/MrmPublic.h>			/* Motif Resource Manager public definitions */

#ifdef __vms__
#pragma standard
#endif

#include <stdio.h>
#include <string.h>

#ifdef __vms__
#pragma nostandard
#include <xdps$include/dpsXclient.h>
#include <xdps$include/dpsops.h>
#include <xdps$include/XDPSlib.h>
#pragma standard
#else
#include <dpsXclient.h>
#include <dpsops.h>
#include <XDPSlib.h>
#include <cdaityp.h>				/* CDA Type definitions to get the PROTO macro */
#endif

#include <psviewwg.h>
#include "psviewP.h"

#ifdef __vms__
#pragma nostandard
#endif

#include "psviewW.h"

#ifdef __vms__
#pragma standard
#endif

static int pixmapError = 0;

/****************************************************************
 *
 * PSView Resources
 *
 ****************************************************************/

static void DefaultTextProc();
static void MakeItTrue();
static void PSViewHandleStatusEvent();
 
void PSViewTextBackstop();

static Dimension default_width    = 10;
static Dimension default_height   = 10;

static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
	 XtOffset(PSViewWidget, psview.foreground), XtRString,
	 (caddr_t) "XtDefaultForeground"},
    {PsNfileName, PsCFileName, XtRString, sizeof(char *),
	 XtOffset(PSViewWidget, psview.filename), XtRImmediate, (caddr_t)NULL},
    {PsNpage, PsCPage, XtRInt, sizeof(int),
	 XtOffset(PSViewWidget, psview.page), XtRImmediate, (caddr_t) NOPAGE},
    {PsNinitString, PsCInitString, XtRString, sizeof(char *),
	 XtOffset(PSViewWidget, psview.initstring),XtRImmediate,(caddr_t)NULL},
    {PsNerrorProc, PsCErrorProc, XtRPointer, sizeof(PSViewProc),
	 XtOffset(PSViewWidget, psview.errorProc), XtRImmediate,
	 (caddr_t) DPSDefaultErrorProc},
    {PsNtextProc, PsCTextProc, XtRPointer, sizeof(PSViewProc),
	 XtOffset(PSViewWidget, psview.textProc), XtRImmediate,
/* 	 (caddr_t) DefaultTextProc}, */
	 (caddr_t) PSViewTextBackstop }, 
    {PsNparseComments, PsCParseComments, XtRBoolean, sizeof(Boolean),
	 XtOffset(PSViewWidget,psview.parseComments), XtRImmediate,
	 (caddr_t) TRUE},
    {PsNpixmapCacheSize, PsCPixmapCacheSize, XtRInt, sizeof(int),
	 XtOffset(PSViewWidget, psview.cacheSize), XtRImmediate, (caddr_t) 0},
    {PsNorientation, PsCOrientation, XtRInt, sizeof(int),
	 XtOffset(PSViewWidget, psview.orientation), XtRImmediate, (caddr_t)0},
    {"copyrightFont", "Font", XtRFont, sizeof (Font),
	XtOffset(PSViewWidget, psview.copyrightFont), XtRString, "fixed"},
    {XmNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
	 XtOffset (PSViewWidget, core.width),
	 XtRDimension, (caddr_t) &default_width},
    {XmNheight, XtCHeight, XtRDimension, sizeof(Dimension),
	 XtOffset (PSViewWidget, core.height),
	 XtRDimension, (caddr_t) &default_height},

};

/****************************************************************
 *
 * Private Routines
 *
 ****************************************************************/

/*  extern int debug;
 *
 *  debug is now turned on by compiling with -DDEBUG
 *  to eliminate sharable image problems caused by having
 *  debug be a global int.
 */

static Boolean CalculateDrawingOffsets(widget,psview,drawXoffset,drawYoffset)
    Widget widget;
    PSViewPart *psview;
    int *drawXoffset,*drawYoffset;    

/* Calculate the X and Y offset which should be used for both DPS's context
   given the dpsPixmap copy offsets.*/
{
    Boolean retVal;
    if (psview->subsetPixmaps)
	{
	*drawXoffset = (widget->core.x + 1);
	if (psview->pixmapWidth - *drawXoffset > widget->core.width)
	    {
	    /*
		Here, we were going to waste pixmap space hanging over the
	        right of the window.  Line up right of pixmap with right of
	        window.
	    */
	    *drawXoffset = psview->pixmapWidth - widget->core.width;
	    }
	*drawYoffset = widget->core.height + (widget->core.y + 1);
	if (*drawYoffset < (psview->pixmapHeight))
	    {
	    /*
		Here we are going to be wasting a lot off the bottom of the
		pixmap, so line up the bottom of the pm with the bottom of 
		the window.
	    */
	    *drawYoffset = (psview->pixmapHeight);
	    }
	if (*drawXoffset != psview->ctxOffsetX ||
            *drawYoffset != psview->ctxOffsetY) retVal=TRUE;
	else retVal = FALSE;
	}
    else
	{
	*drawXoffset = 0;                      
	*drawYoffset = widget->core.height;
	retVal = FALSE;
	}
    return retVal;
}

static void CalculatePixmapOffsets(widget,psview,drawXoffset,drawYoffset)
    Widget widget;
    PSViewPart *psview;
    int drawXoffset,drawYoffset;
{
    psview->dpsPixmapOffsetX = drawXoffset;
    psview->dpsPixmapOffsetY = drawYoffset - widget->core.height;
}    


static void HandlePixmapError(dpy,event)
    Display *dpy;
    XErrorEvent *event;
{
    pixmapError = 1;
}

static void InvalidateCache(psview)
  PSViewPart *psview;
{
    int i;
    if (psview->pixmapCache) {
	for (i = 0; i < psview->cacheSize; i++)
	    psview->pixmapCache[i].page = 0;
	psview->curPD = -1;
    }
}

RebuildCache(widget)
  PSViewWidget widget;
{
    int i,start=1,screenNum;
    PSViewPart *psview = &(widget->psview);
    if (!psview->pixmapCache && psview->cacheSize > 0)
        psview->pixmapCache = (PixmapDesc) XtCalloc(
	    psview->cacheSize,sizeof(PixmapDescRec));
    for (i = 0; i < psview->cacheSize; i++) {
	if (psview->pixmapCache[i].pixmap)
	     XFreePixmap (XtDisplay(widget), psview->pixmapCache[i].pixmap);
    }
    if (psview->cacheSize >= 1)
	{
	int (*previous_x_error_handler) ();

	XSync(XtDisplay(widget),0); /*Make sure no other errors in the pipe*/
	previous_x_error_handler = XSetErrorHandler((XErrorHandler)HandlePixmapError);
	pixmapError = 0;
	psview->pixmapCache[0].lastUse = 1;
        psview->pixmapCache[0].page = 0;
	psview->pixmapCache[0].pixmap =
	    XCreatePixmap(XtDisplay(widget), 
			  XtWindow(widget),
			  psview->pixmapWidth, psview->pixmapHeight,
			  widget->core.depth);
	XSync(XtDisplay(widget),0); /*Force possible error to happen now*/

	/* reset x error handler to previous state */	
	XSetErrorHandler(previous_x_error_handler); 

	if (pixmapError)
	    {

	     /* Could not make pixmap...try again with size maxed at display
	        size.  If this fails, boom*/

	    int screenNum = XDefaultScreen(XtDisplay(widget));
	    int scrWidth = XDisplayWidth(XtDisplay(widget),screenNum);
	    int scrHeight = XDisplayHeight(XtDisplay(widget),screenNum);
	    start=0;
	    if (psview->pixmapWidth > scrWidth)
		psview->pixmapWidth = scrWidth;
	    if (psview->pixmapHeight > scrHeight)
		psview->pixmapHeight = scrHeight;
	    psview->subsetPixmaps = TRUE;
	    }
	}
    for (i = start; i < psview->cacheSize; i++) {
	psview->pixmapCache[i].pixmap =  XCreatePixmap(XtDisplay(widget),
	    XtWindow(widget),
	    psview->pixmapWidth, psview->pixmapHeight,
	    widget->core.depth);
	psview->pixmapCache[i].lastUse = 1;
        psview->pixmapCache[i].page = 0;
    }
    psview->pixmapCacheLRU = 2;
}


/*
 * Make a copy of the given string in a newly malloced area.  If given NULL,
 * return NULL.
 */

static char *MallocACopy(str)
char *str;
{
    if (str) return strcpy(XtMalloc((unsigned) (strlen(str) + 1)), str);
    else return NULL;
}

/*
 * Make sure the orientation is a reasonable value.  If not, generate
 * a warning, and make it a reasonable value.
 */

static void RationalizeOrientation(widget)
PSViewWidget widget;
{
    int orientation = widget->psview.orientation;
    if (orientation < 0 || orientation >= 360)
	orientation = 1;	/* Generates error, will be zero. */
    if (orientation % 90 != 0) {
	widget->psview.orientation = 90 * (orientation / 90);
	XtWarning("Bad orientation in PS previewer widget");
    }
}


void NeedPixmap(widget)
  PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    PixmapDesc temp = psview->pixmapCache;
    int i=0,index=0;
    if (psview->cacheSize == 0) {
	psview->curPD = -1;
	return;
    }
    for (i = 1; i < psview->cacheSize; i++) {
	if (temp->lastUse > psview->pixmapCache[i].lastUse)
	    {
	    temp = psview->pixmapCache + i;
	    index = i;
	    }
    }

#ifdef DEBUG 
	fprintf(stderr, "NeedPixmap - using # %d\n",
		       temp - psview->pixmapCache);
#endif

    temp->page = psview->page;
    temp->lastUse = psview->pixmapCacheLRU++;
    psview->curPD = index;
}


Boolean GetFromCache(widget,x,y,width,height)
    PSViewWidget widget;
    int x,y,width,height;
{
    PixmapDesc temp;
    PSViewPart *psview = &(widget->psview);
    int i;
    if (psview->subsetPixmaps && x < 0 && psview->page == psview->curpage)
	return FALSE; /*Here, we are building a different section of the
		        same page.  That means we must have already looked
			in the cache, so don't even try*/
    if (psview->windowDrawMode)
	return FALSE;
    for (i = 0; i < psview->cacheSize; i++) {
	if (psview->page == psview->pixmapCache[i].page) {
	    temp = &psview->pixmapCache[i];
	    if (psview->subsetPixmaps && x >= 0 &&
		(
		x < -temp->offsetX ||
		y < -temp->offsetY ||
                (width + x >  psview->pixmapWidth - temp->offsetX) ||
		(height + y > psview->pixmapHeight - temp->offsetY)
		))
	        continue; /*If the areas does not match, look to next one*/
	    psview->curPD = i;
	    temp->lastUse = psview->pixmapCacheLRU++;
	    psview->redisplayPixmap = temp->pixmap;
	    psview->redisplayPixmapOffsetX = temp->offsetX;
	    psview->redisplayPixmapOffsetY = temp->offsetY;

	    if (psview->subsetPixmaps)
		{
		/* Send an exposure to get the stuff copied to the window
		   This ensures that we copy the right amount if we have a
		   partial pixmap.
		*/

		if (x < 0)
		    {
		    /* We are redoing a page...mash the whole thing*/
		    XClearArea(XtDisplay(widget),XtWindow(widget),0,0,
			   widget->core.width,widget->core.height,1);
		    }
		else
		    /* This is from an expose...just do the copy*/
		    {
		    (*psview->AppStatusProc)(psview->AppStatusData, starting,
			psview->page); /*It may take a while!*/
		    XCopyArea(XtDisplay(widget),psview->redisplayPixmap,
		              XtWindow(widget),psview->copyGC,
			      x + psview->redisplayPixmapOffsetX,
			      y + psview->redisplayPixmapOffsetY,
			      width, height, x, y);
		    (*psview->AppStatusProc)(psview->AppStatusData,finished,
				psview->page); /*It may take a while!*/

		    }		
		}
	    else
		{
		XCopyArea(XtDisplay(widget), psview->redisplayPixmap,
			  XtWindow(widget), psview->copyGC,
			  psview->redisplayPixmapOffsetX,
			  psview->redisplayPixmapOffsetY,
			  widget->core.width, widget->core.height, 0, 0);
		}

            (*psview->AppStatusProc)
		(psview->AppStatusData, ok, psview->page);

#ifdef DEBUG
		fprintf(stderr, "GetFromCache # %d \n", i);
#endif

	    return TRUE;
	}
    }
    return FALSE;
}


/*
 * Set the widget to not draw (that is, make the context's drawable be
 * NULL).  This must be called at a time when the main context is frozen
 * or waiting for input; otherwise, bizarre race conditions can occur.
 */

static void DeactivateDrawing(widget)
    PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    if (!psview->drawingdeactivated) {
#ifdef DEBUG
	fprintf(stderr, "Deactivating drawing.\n");
#endif
	if (!psview->secondcontext) {
	    psview->secondcontext =
		XDPSCreateSimpleContext(XtDisplay(widget),
					(Drawable) NULL, psview->copyGC,
					0, (int) widget->core.height,
					DefaultTextProc, psview->errorProc,
					NULL);
	    DPSsetshared(psview->secondcontext, TRUE);
	}
	_PSViewDeactivateDrawing(psview->secondcontext, psview->contextid);
	DPSWaitContext(psview->secondcontext);
	psview->drawingdeactivated = TRUE;
    }
}



/*
 * Allow the widget to start drawing again (make its context's drawable
 * be the appropriate window or pixmap).  This must be called at a time
 * when the main context is frozen or waiting for input; otherwise,
 * bizarre race conditions can occur.
 */

static void ReactivateDrawing(widget)
    PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    if (psview->secondcontext && psview->drawingdeactivated) {
#ifdef DEBUG
	fprintf(stderr, "Reactivating drawing.\n");
#endif

	/* The following is a temporary patch to prevent core dumps */

	psview->secondcontext->priv = (char *) widget;

	/* End of temporary patch */

	_PSViewReactivateDrawing(psview->secondcontext, psview->contextid);
	DPSWaitContext(psview->secondcontext);
	psview->drawingdeactivated = FALSE;
    }
}



static void
NoticeFrozen(widget)
  PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    if (psview->skipcount > 0) {
        psview->curpage++;
#ifdef DEBUG
	fprintf(stderr, "Page %d thrown away.\n", psview->curpage);
#endif
        psview->skipcount--;
	if (psview->skipcount == 0)
	    ReactivateDrawing(widget);
	XDPSUnfreezeContext(psview->context);
    } else {
        if (!psview->showpageisok) { /* if we're sending the prolog, ERROR!! */
	    psview->rebuildcontext = TRUE;
	    (*psview->AppStatusProc)(psview->AppStatusData, badComments,
				     psview->page);
	    FreeStructure(psview->sinfo);
	    psview->sinfo = NULL;		/* *** RMM 4/1/92 *** */
	    psview->numpages = NOPAGE;	    
	    return;
        }
	/*  code to set psview->curPD to be right entry from cache. */
	if (!psview->windowDrawMode && psview->cacheSize)
	    NeedPixmap(widget);
        psview->frozen = TRUE;
	psview->curpage = psview->page;
	if (!psview->windowDrawMode) {
	    psview->redisplayPixmap = psview->dpsPixmap;
	    if (psview->subsetPixmaps)
		{
		/* Send an exposure to get the stuff copied to the window
		   This ensures that we copy the right amount if we have a
		   partial pixmap.
		*/

		XClearArea(XtDisplay(widget),XtWindow(widget),0,0,
			   widget->core.width,widget->core.height,1);

		}
	    else
		{
		XCopyArea(XtDisplay(widget), psview->redisplayPixmap,
			  XtWindow(widget), psview->copyGC,
			  psview->redisplayPixmapOffsetX,
			  psview->redisplayPixmapOffsetY,
			  widget->core.width, widget->core.height, 0, 0);
		}

	    if (psview->curPD >= 0) {
		int i = psview->curPD;
		/*
		  Make the current Pixmap Cache entry be the pixmap we just
		  filled up.  Shove the old pixmap from the cache back into
		  dpsPixmap to get written into next.  We know that we just
		  copied dpsPixmap into redisplayPixmap, so we don't need to
		  use a temp for the swap operation.
		*/
		psview->pixmapCache[i].offsetX = psview->dpsPixmapOffsetX;
		psview->pixmapCache[i].offsetY = psview->dpsPixmapOffsetY;
		XCopyArea(XtDisplay(widget),
                          psview->dpsPixmap, psview->pixmapCache[i].pixmap,
                          psview->pixmapGC, 0, 0,
                          psview->pixmapWidth, psview->pixmapHeight, 0, 0);

		psview->redisplayPixmap = psview->pixmapCache[i].pixmap;
		psview->redisplayPixmapOffsetX = psview->dpsPixmapOffsetX;
		psview->redisplayPixmapOffsetY = psview->dpsPixmapOffsetY;
	    }
	}
	(*psview->AppStatusProc)(psview->AppStatusData, ok, psview->page);
	(*psview->AppStatusProc)(psview->AppStatusData, finished,
				 psview->page);
    }
}


/*
    This routine actually retrieves PostScript and sends it to XDPS
*/
void
ProcessInputStatus(widget)
  PSViewWidget widget;
{
#define CHUNK 2048		/* Max chars to send at one time. */
    long numRead, count;
    int i;
    char buf[CHUNK+1];
    PSViewPart *psview = &(widget->psview);
    if (!psview->context)
	{
#ifdef DEBUG
	   fprintf(stderr, "\nentered ProcessInputStatus with null context\n");
#endif
	return;
	}
#ifdef DEBUG
    fprintf(stderr,
	"\nProcessInputStatus:  start %d, length, %d, end %d\n",
	    psview->posInfo[0].start,psview->posInfo[0].length,
	    psview->posInfo[0].end);
#endif
    while (psview->numPos > 0 &&
	   (
	   psview->posInfo[0].length == 0 ||
	   psview->posInfo[0].start >= psview->posInfo[0].end)) {
	if (psview->posInfo[0].proc)
	    (*psview->posInfo[0].proc)(widget);
	psview->numPos--;
	for (i=0 ; i<psview->numPos ; i++) {
	    psview->posInfo[i] = psview->posInfo[i+1];
	}
    }
    if (psview->numPos > 0) {
	    if (
            (psview->posInfo[0].offsetX != psview->ctxOffsetX)
			||
	    (psview->posInfo[0].offsetY != psview->ctxOffsetY))
	
	    {
	    /*If the current context is not using the same drawable and
		offsets as what we want, change it...*/
	    DPSsetXgcdrawable(psview->context,XGContextFromGC(psview->copyGC),
		psview->posInfo[0].drawable,
		psview->posInfo[0].offsetX,
		psview->posInfo[0].offsetY);
	    /*...and remember that is what it is doing*/
	    psview->ctxDrawable = psview->posInfo[0].drawable;
	    psview->ctxOffsetX = psview->posInfo[0].offsetX;
	    psview->ctxOffsetY = psview->posInfo[0].offsetY;
	    /*Also set parameters to remember the offsets in the Pixmap we
	      are working on*/
	    CalculatePixmapOffsets(widget,psview,psview->ctxOffsetX,
		psview->ctxOffsetY);
	    }
	count = psview->posInfo[0].length;
	if (count > CHUNK)
	    count = CHUNK;
	if (psview->posInfo[0].start != psview->curSeekPos)
	    myfseek(psview->file, psview->posInfo[0].start, 0);
	numRead = myfread(buf, 1, count, psview->file);
	if (numRead < count) {
	    psview->curSeekPos = -1;
	    psview->posInfo[0].start = psview->posInfo[0].end;
	    psview->posInfo[0].length = 0;
	} else {
	    psview->posInfo[0].start = psview->curSeekPos =
	    ftell(psview->file);
	    psview->posInfo[0].length -= numRead;

	}

	if (numRead > 0) {
	    buf[numRead] = 0;
#ifdef WRITEALL
		{
		fprintf(stderr,"\n");
		for (i = 0; i < numRead; i++) fputc(buf[i],stderr);
		fprintf(stderr,"\n");
		fprintf(stderr, "sending %d bytes\n", numRead);
		}
#endif
	    DPSWritePostScript(psview->context, buf, (unsigned int) numRead);
            psview->waitingInput = FALSE;
	} else {
	    ProcessInputStatus(widget);	/* We didn't send anything; try to */
					/* get some more. */
	}
    } else {
        psview->waitingInput = TRUE;
	psview->skipcount = 0; /*Can't be skipping any more if we didn't give
			         it any data.  This is mainly for if the
				 prolog is real fast.*/
	(*psview->AppStatusProc)(psview->AppStatusData, finished,
				 psview->curpage);
    }

    if (psview->context)
        DPSFlushContext(psview->context); 
}



/*
 * Send the text between the given start and end positions to the
 * context.  Actually, the sending is done in the background.  When the
 * text has been sent, and the context is awaiting more input (implying
 * that it has also executed the text), then the given proc (if any) will
 * be called.
 */

static Boolean _PSViewExecutePostscript(widget, start, length, end, proc)
    PSViewWidget widget;
    long start, length, end;
    void (*proc)();		/* Proc to call when this chunk finishes
				   executing. */
{
    PSViewPart *psview = &(widget->psview);
    int status;
    int xoffset,yoffset;

    (*psview->AppStatusProc)(psview->AppStatusData, starting, psview->page);
    if (psview->sinfo && psview->frozen) {
	XDPSUnfreezeContext(psview->context);
	psview->frozen = FALSE;
    }

    CalculateDrawingOffsets(widget,psview,&xoffset,&yoffset);
    if (psview->numPos == 0 ||
      psview->posInfo[psview->numPos - 1].offsetX != xoffset ||
      psview->posInfo[psview->numPos - 1].offsetY != yoffset ||
      psview->posInfo[psview->numPos - 1].end != start ||
      psview->posInfo[psview->numPos - 1].proc != NULL) {

	/*This code crushes what would be a new posInfo record into the
	  last one if possible.  Actually, this code creates the new
	  posInfo record if it has to.  If this code is skipped, the
	  new posInfo data is added to the last posInfo record */

	psview->numPos++;
	if (psview->numPos >= psview->maxPos) {
	    psview->maxPos = psview->numPos;
	    psview->posInfo = (PosInfo)
		XtRealloc((char *) psview->posInfo,
			  psview->maxPos * sizeof(PosInfoRec));
	}

	psview->posInfo[psview->numPos - 1].start = start;
    	psview->posInfo[psview->numPos - 1].length = 0;
	psview->posInfo[psview->numPos - 1].drawable = 
	    psview->windowDrawMode ?XtWindow(widget): psview->dpsPixmap;
	psview->posInfo[psview->numPos - 1].offsetX = xoffset;
	psview->posInfo[psview->numPos - 1].offsetY = yoffset;
    }

    psview->posInfo[psview->numPos - 1].length += length;
    psview->posInfo[psview->numPos - 1].end = end;
    psview->posInfo[psview->numPos - 1].proc = proc;

    XDPSSetStatusMask(psview->context,
		      (unsigned int) (PSNEEDSINPUTMASK | 
				      PSFROZENMASK |
				      PSZOMBIEMASK),
		      (unsigned int) 0,
		      (unsigned int) 0);
    status = XDPSGetContextStatus(psview->context);
    if (status == PSNEEDSINPUT)
	ProcessInputStatus(widget);
    return TRUE;
}

void PSViewTextBackstop(ctx, buf, count)
  DPSContext ctx;
  char *buf;
  int count;
{
    PSViewWidget widget = (PSViewWidget) ctx->priv;
    PSViewPart *psview = &(widget->psview);
    (*psview->appErrorProc)((Widget) widget, buf, count);
}


/*
 * Called when the prologue of a structured file has been executed.
 */

static void SetShowpageToBeOK(widget)
    PSViewWidget widget;
{
    widget->psview.showpageisok = TRUE;
}


static Boolean WorkMakeItTrue(data)
    Opaque data;
{
    MakeItTrue((PSViewWidget) data);
    return TRUE;
}


/*
 * Called when we've processed all the text in a non-structured file.
 */

static void WentPastEnd(widget)
    PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    psview->numpages = psview->curpage;
    psview->page = psview->curpage;
    psview->redopage = TRUE;
    (*psview->AppStatusProc)(psview->AppStatusData, noPage, psview->numpages);
    ReactivateDrawing(widget);
    XtAddWorkProc(WorkMakeItTrue, (Opaque) widget);
}

/*
 * Make what's being displayed in the widget actually be the same as what is
 * specified in the widget record.  Returns immediately if disabledepth > 0.
 * Returns TRUE iff it was able to meet the specification.
 */

static void MakeItTrue(widget)
    PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    long start, length, end;
    XSetWindowAttributes attributes;
    XEvent event;
    int xoffset,yoffset,tmp1,tmp2;

    if (psview->disabledepth > 0) return;
    if (psview->filename == NULL || psview->page <= 0) {
	if (XtIsRealized(widget))
	    XClearWindow(XtDisplay(widget), XtWindow(widget));
	if (psview->file)
	  {
	    myfclose(psview->file);
	    psview->file = 0;
            if (psview->context) 
	      {	
		/* free space if we're closing file */
	        DPSDestroySpace(DPSSpaceFromContext(psview->context));
		psview->context = 0;
	      }	
	  }
	return;
    }

    psview->errorRedisplay = FALSE;
    (*psview->AppStatusProc)(psview->AppStatusData, starting,
			     psview->page);
    psview->NeedToDisplayCopyrightNotice = FALSE;
    if (psview->rebuilddevice) {
#ifdef DEBUG
	fprintf(stderr, "Rebuilding device.\n");
#endif
	psview->windowDrawMode = psview->newWindowDrawMode;
	if (!psview->windowDrawMode)
	    RebuildCache(widget);
	psview->redisplayPixmap = 0;
	psview->rebuilddevice = FALSE;
        psview->rebuildcontext = TRUE;
	if (psview->dpsPixmap) {
	    XFreePixmap(XtDisplay(widget), psview->dpsPixmap);
	    psview->dpsPixmap = 0;
	}
	attributes.backing_store =
	    (psview->windowDrawMode ? WhenMapped : NotUseful);
	XChangeWindowAttributes(XtDisplay(widget), XtWindow(widget),
				CWBackingStore, &attributes);
	if (psview->windowDrawMode) {
	    /*
	     * Changing the backing store can result in extra expose
	     * events that we don't really care about.  Eat them now.
	     */
	    XSync(XtDisplay(widget), 0);
	    while (XCheckTypedWindowEvent(XtDisplay(widget), XtWindow(widget),
					  Expose, &event)) ;
	} else {
	    int (*previous_x_error_handler) ();
	    /*
	     * Here we are supposed to use a pixmap.  If we can't create a
	     * pixmap, though, set to indicate that we are doing window display
	     * and start the whole thing over again.
	     */
	    XSync(XtDisplay(widget),0); /*Make sure no other errors in the pipe*/
	    previous_x_error_handler = XSetErrorHandler((XErrorHandler) HandlePixmapError);
	    pixmapError = 0;
	    psview->dpsPixmap =
		XCreatePixmap(XtDisplay(widget), 
	    		      XtWindow(widget),
			      psview->pixmapWidth, psview->pixmapHeight,
			      widget->core.depth);
	    XSync(XtDisplay(widget),0); /*Force possible error to happen now*/

	    /* reset x error handler to previous state */	
	    XSetErrorHandler(previous_x_error_handler); 

	    if (pixmapError) /* Could not make pixmap...start again*/
/**********Fall back to small pixmap if no cache***********/
		{
		psview->dpsPixmap = 0;
		psview->newWindowDrawMode = 1;
		psview->rebuilddevice=TRUE;
		MakeItTrue(widget); /*Recursive call...can only go 1 deep*/

		/*Want to keep trying pixmap mode.  Otherwise, application and
		  widget are in different states.*/

		psview->newWindowDrawMode = 0; 
		return;
		}
	}
    }
    if (!psview->rebuildcontext && GetFromCache(widget,-1,-1,-1,-1)) {
	(*psview->AppStatusProc)(psview->AppStatusData, ok, psview->page);
	(*psview->AppStatusProc)(psview->AppStatusData, finished,
				 psview->page);
	return;
    }
    if (psview->sinfo == NULL && (psview->page < psview->curpage ||
				  (psview->page == psview->curpage &&
				   psview->redopage) || 
                                   CalculateDrawingOffsets(widget,psview,
							   &xoffset,&yoffset)
				   )) {
        psview->redopage = FALSE;
	psview->rebuildcontext = TRUE;
    }
    if (psview->context == NULL || psview->rebuildcontext) {
#ifdef DEBUG
	fprintf(stderr, "Rebuilding context.\n");
#endif
	psview->curSeekPos = 0;
	if (psview->file)
	    myfclose(psview->file);
	psview->file = myfopen(psview->filename, "r");
	if (psview->file == NULL) {
	    (*psview->AppStatusProc)(psview->AppStatusData, badfile,
				     psview->page);
	    (*psview->AppStatusProc)(psview->AppStatusData, finished,
				     psview->page);
	    return;
	}

	    /* 
	     * Check for %! header.  If it is not there, warn the application
	     * and then abort if the application says it is required
	     */

	    {
	    char buf[2];
	    if (
		(fread(buf,1,2,psview->file) != 2)
			    ||
		      (buf[0] != '%')
			    ||
		      (buf[1] != '!')
		)
		{
                (*psview->AppStatusProc)(psview->AppStatusData, noHeader,
                                     0);
		if (psview->headerRequired)
		    {
		    (*psview->AppStatusProc)(psview->AppStatusData, finished,
				     psview->page);
		    fclose(psview->file);
		    psview->file = NULL;
		    return;
		    } 
		}
	    rewind (psview->file);
	    }
	psview->numPos = 0;
/*	InvalidateCache(psview); */
/* Moved this to individual routines which cause a rebuild*/
        if (psview->context) 
	    DPSDestroySpace(DPSSpaceFromContext(psview->context));
	CalculateDrawingOffsets(widget,psview,&xoffset,&yoffset);
	CalculatePixmapOffsets(widget,psview,xoffset,yoffset);
	psview->context =
	    XDPSCreateSimpleContext(XtDisplay(widget),
				    (psview->windowDrawMode ?
					XtWindow(widget):psview->dpsPixmap),
				    psview->copyGC,
				    xoffset,yoffset,
				    DefaultTextProc, psview->errorProc, NULL);
	if (psview->context == NULL) {
	    XtError("Server does not have PostScript extension.");
	    exit(1);
	}
	psview->ctxOffsetX = xoffset;
	psview->ctxOffsetY = yoffset;
	psview->ctxDrawable = psview->windowDrawMode ?
					XtWindow(widget):psview->dpsPixmap;

	DPSSetTextBackstop(PSViewTextBackstop);
/*	DPSSetErrorProc(PSViewErrorProc); */

/* 	_PSViewLoadServer(psview->context); */
	_PSViewLoadPrelude(psview->context);
/* 	_PSViewExecServer(psview->context); */

	psview->context->priv = (char *) widget;
	XDPSRegisterStatusProc(psview->context, PSViewHandleStatusEvent);

	if (psview->contextid == 0) {
	    /*
	     * Really, I should do this regardless of whether the
	     * contextid is currently zero.  This way saves time;
	     * however, it counts on the interpreter to not reuse context
	     * ids.
	     */
	    DPScurrentcontext(psview->context, &(psview->contextid));
	}

	_PSViewInitialize
	    (psview->context, psview->orientation,
	     ((psview->orientation == 90 || psview->orientation == 180) ?
	      widget->core.width * 72.0 / psview->xdpi : 0),
	     ((psview->orientation == 180 || psview->orientation == 270) ?
	      widget->core.height * 72.0 / psview->ydpi : 0),
	     psview->scale, psview->contextid);
	      
	if (psview->fakeTrays)
	    _PSViewFakeTrays(psview->context);
	if (psview->bitmapWidths)
	    _PSViewBitmapWidths(psview->context);
        DPSWaitContext(psview->context);
	DPSerasepage(psview->context);

/* 	_PSViewExecServer(psview->context); */

	FreeStructure(psview->sinfo);
	psview->sinfo = NULL;
	if (psview->parseComments) {
	    int error;

	    /* Call DetermineStructure to parse comments and read the
	       first PostScript page (%%Page) */
	    psview->sinfo =
		DetermineStructure(psview->file, &error, NULL);
			/* RMM 4/1/92 added third argument -- pointer
			   to current structure info instance, if any
			*/
	    if (error) {
		psview->rebuildcontext = TRUE;
		(*psview->AppStatusProc)(psview->AppStatusData, badComments,
					 psview->page);
		return;
	    }
	if (psview->sinfo)		/* Structured comments found? */
	    psview->numpages = psview->sinfo->numPages;
	}

#ifdef DEBUG
	    if (psview->sinfo)
		fprintf(stderr, "structured comments.......\n");
#endif

	psview->rebuildcontext = FALSE;
	psview->curpage = 0;
        psview->frozen = FALSE;
	ReactivateDrawing(widget);
	if (psview->sinfo) {	/* Execute the prologue */
	    psview->showpageisok = FALSE;
	    _PSViewExecutePostscript(widget, 0L,
				     psview->sinfo->pagestarts[0].length,
				     psview->sinfo->pagestarts[0].end,
				     SetShowpageToBeOK);
#ifdef DEBUG
	    printf(stderr, "skipcount set to -1\n");
#endif
	    psview->skipcount = -1; /* Cancel tweaking of skipcount below. */

	    /* Here we are doing the file for the first time.  Make sure we
	       have a valid page */

	    if (psview->page < 1) psview->page = 1;
	    if ((psview->numpages != NOPAGE) &&
			(psview->page >= psview->numpages))
		psview->page = psview->numpages;
	    else
	      {
	    /*
	     * If the document is structured and the page does not exist,
	     * find the next one that does exist. We are aware that only
	     * part of the partial document may have read thus far, so we
	     * MUST know when we run off the current end so that we can
	     */
		while(psview->sinfo->pagestarts[(psview->page)].start < 0
			&& psview->page <= psview->sinfo->curlastpage )
		  {
		    psview->page++;
		  }
		/* Do we need to read more? */
		if ( psview->page > psview->sinfo->curlastpage )
		  {
		    int error;
		    if ( psview->sinfo->atEnd )
		      psview->page = psview->sinfo->numPages;
		    else
		      {
			psview->sinfo = DetermineStructure(psview->file,
			  &error, psview->sinfo);
			if (error)
			  {
			    psview->rebuildcontext = TRUE;
			    (*psview->AppStatusProc)(psview->AppStatusData,
			      badComments, psview->page);
			    return;
			  }
		      }
		    if ( psview->numpages < psview->sinfo->numPages )
		      {
			psview->numpages = psview->sinfo->numPages;
			(*psview->AppStatusProc)
			  (psview->AppStatusData, noPage, psview->numpages);
		      }
		  }
		}

	} else {
	    if (psview->page > 1) {
		_PSViewSetDrawableToNULL(psview->context);
#ifdef DEBUG
		fprintf(stderr, "Drawable set to NULL.\n");
#endif
		DeactivateDrawing(widget); /*
					    * Must call now, before main
					    * context ever gets text to
					    * start executing.
					    */
	    }
	    psview->showpageisok = TRUE;
	    _PSViewExecutePostscript(widget, 0L, MAXINT, MAXINT, WentPastEnd);
	}
    } /* end of rebuild context */

    if (psview->page == psview->curpage && !psview->redopage) {
	(*psview->AppStatusProc)(psview->AppStatusData, ok, psview->page);
	(*psview->AppStatusProc)(psview->AppStatusData, finished,
				 psview->page);
	return;			/* We're already showing it. */
    }

    psview->redopage = FALSE;
    if (psview->sinfo) {
	if ((psview->page > psview->sinfo->numPages && psview->sinfo->atEnd)
				||
	    (psview->sinfo->pagestarts[psview->page].start == -1))
	    return;
	/*
	    Remember the start/end/length of the page to display now
	*/
	start = psview->sinfo->pagestarts[psview->page].start;
	end = psview->sinfo->pagestarts[psview->page].end;
	length = psview->sinfo->pagestarts[psview->page].length;
	if (!(psview->frozen || psview->waitingInput)) {
	    psview->skipcount++;
#ifdef DEBUG
	    fprintf(stderr, "Incrementing skipcount to %d.  Page is %d\n",
			    psview->skipcount,psview->page);
#endif /* DEBUG */
	    if (psview->skipcount > 0)
		DeactivateDrawing(widget);
	}  /* End of { if (!psview->frozen...) } */
#ifdef DEBUG
	    else
	    fprintf(stderr, "No increment.  Frozen is %d, input is %d, Page %d\n",
		psview->frozen,psview->waitingInput,psview->page);
#endif /* DEBUG */
	_PSViewExecutePostscript(widget, start, length, end, NULL);
    }  /* End of { if (psview->sinfo) } */

      else {
	psview->skipcount = psview->page - psview->curpage - 1;
	if (psview->skipcount) {
	    DeactivateDrawing(widget);
	}
	if (psview->frozen) {
	    XDPSUnfreezeContext(psview->context);
	    psview->frozen = FALSE;
	}
    }
}

static void DefaultTextProc(ctx, buf, count)
DPSContext ctx;
char *buf;
int count;
{
    PSViewWidget widget = (PSViewWidget) ctx->priv;
    PSViewPart *psview = &(widget->psview);
    (*psview->textProc)(widget, buf, count); 
}


static void NullProc() {}

/****************************************************************
 *
 * Class Routines
 *
 ****************************************************************/

static void ClassInitialize()
{
}

static void Initialize(request, w)
Widget request, w;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    Screen *screen = XtScreen(widget);
    int scrNum = XDefaultScreen(XtDisplay(widget));
    
#ifdef DEBUG
    int ijk = 0;
    if(ijk) /*Change ijk with debugger here to get synchronization*/
    XSynchronize(XtDisplay(widget),1);
#endif

    if (widget->core.width == 0)
        widget->core.width = 10;
    if (widget->core.height == 0)
	widget->core.height = 10;

    RationalizeOrientation(widget);

    psview->filename = MallocACopy(psview->filename);
    psview->initstring = MallocACopy(psview->initstring);

    if (psview->cacheSize < 0) psview->cacheSize = 0;

    psview->curpage = NOPAGE;
    psview->NeedToDisplayCopyrightNotice = TRUE;
    psview->fakeTrays = FALSE;
    psview->bitmapWidths = FALSE;
    psview->windowDrawMode = FALSE;
    psview->newWindowDrawMode = psview->windowDrawMode ;
    psview->subsetPixmaps = FALSE;
    psview->pixmapCacheLRU = 0;
    psview->pixmapCache = NULL;
    psview->curPD = -1;
    psview->xdpi = 
	((double) screen->width) * MMPERINCH / (double) screen->mwidth;
    psview->ydpi = 
	((double) screen->height) * MMPERINCH / (double) screen->mheight;
    psview->sinfo = NULL;
    psview->numpages = NOPAGE;
    psview->rebuildcontext = TRUE;
    psview->rebuilddevice = TRUE;
    psview->redopage = TRUE;
    psview->disabledepth = 0;
    psview->redisplayPixmap = 0;
    psview->dpsPixmap = 0;
    psview->pixmapGC = NULL;
    psview->copyGC = NULL;
    psview->context = NULL;
    psview->secondcontext = NULL;
    psview->drawingdeactivated = FALSE;
    psview->contextid = 0;
    psview->file = NULL;
    psview->scale = 1.0;
    psview->appErrorProc = NullProc;
    psview->AppStatusProc = NullProc;
    psview->AppStatusData = NULL;
    psview->frozen = FALSE;
    psview->waitingInput = TRUE;
    psview->curSeekPos = 0;
    psview->posInfo = 0;
    psview->numPos = 0;
    psview->maxPos = 0;
    psview->skipcount = 0;
    psview->showpageisok = TRUE;
    psview->headerRequired = FALSE;
    psview->pixmapHeight = widget->core.height;
    psview->pixmapWidth = widget->core.width;
    psview->visibleX=0;
    psview->visibleY=0;
    psview->visibleWidth = XDisplayWidth(XtDisplay(widget),scrNum);
    psview->visibleHeight = XDisplayHeight(XtDisplay(widget),scrNum);
    psview->errorRedisplay = FALSE;
}


static void Realize(w, valueMask, attributes)
    register Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    PSViewWidget widget = (PSViewWidget) w;
    XGCValues gcv;
    Mask mask;
    attributes->bit_gravity = SouthWestGravity;
    mask = *valueMask | CWBitGravity;
    XtCreateWindow(w, (unsigned int) InputOutput, (Visual *) CopyFromParent,
		   mask, attributes);
    gcv.font =	widget->psview.copyrightFont;
    gcv.foreground = 1;
    gcv.background = 0;
    gcv.function = GXcopy;
    gcv.subwindow_mode = ClipByChildren;
    gcv.clip_mask = None;
    gcv.plane_mask = ~(0L);
    gcv.graphics_exposures = FALSE;
    mask =  GCForeground | GCBackground | GCFunction | GCFont |
	GCPlaneMask | GCSubwindowMode | GCClipMask | GCGraphicsExposures;
    widget->psview.pixmapGC = XtGetGC(w, mask, &gcv);
    gcv.foreground = widget->psview.foreground;
    gcv.background = widget->core.background_pixel;
    widget->psview.copyGC = XtGetGC(w, mask, &gcv);
    widget->psview.redopage = TRUE;

    /*  if a filename has been specified before the widget was realized,
     *  then enable redisplay now that we have a window id for XDPS to write
     *  to; NOTE: this assumes that anytime a file has been specified before
     *  the widget has been realized; the display will automatically be
     *  enabled once the widget is realized here.
     */

    if (widget->psview.filename != NULL)
      {
	PSViewEnableRedisplay((Widget) widget);
	if (PSViewIsStructured(widget))
	  {
	     /*  if the file is structured, call back user to notify
	      *  how many pages are in file
	      */

	     int 	page 	= PSViewGetNumPages((Widget) widget);
    	     PSViewPart *psview = &(widget->psview);

             (*psview->AppStatusProc)
		(psview->AppStatusData, noPage, page);
	  }
      }

              
 
}

static Boolean SetValues (current, request, new, last)
Widget current, request, new;
Boolean last;
{
    return FALSE;		/* %%% Needs to be written! */
}

static void Resize(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->pixmapHeight = widget->core.height;
    psview->pixmapWidth = widget->core.width;
    psview->rebuilddevice = TRUE;
    MakeItTrue(widget);
}

void
DisplayCopyrightInfo(w)
    Widget w;
{
    static char *info[] = {
	"PostScript\256 Previewer",
	"\251 1989 Digital Equipment Corporation ",
	"All Rights Reserved",
	"This software uses an Adobe PostScript",
	"interpreter.  PostScript and Adobe are",
	"registered trademarks of Adobe Systems",
	"Incorporated.",
    };
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    int x, y, i;
    
    x = 50;
    y = 125;
    for (i=0 ; i < XtNumber(info) ; i++) {
	XDrawString(XtDisplay(widget), XtWindow(widget), psview->copyGC, x, y,
		    info[i], strlen(info[i]));
	y += 25;
    }
    XFlush(XtDisplay(widget));	/* Paint ASAP so the user gets some chance
				   to see it before a context comes along
				   and blows it away... */
}

static void Redisplay(w, event, region)
Widget w;
XEvent *event;
Region region;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    XExposeEvent *e = (XExposeEvent *) event;
    if (psview->NeedToDisplayCopyrightNotice && !psview->context) {
        DisplayCopyrightInfo(w);
	return;
    }
    if (psview->subsetPixmaps)
	{
	/* Take a guess at the visible area.  It is certainly the visible
	   are that we care about now at least!*/

	psview->visibleX = -(widget->core.x + 1);
	psview->visibleY = -(widget->core.y + 1);
	psview->visibleWidth = e->width + e->x + (widget->core.x + 1);
	psview->visibleHeight = e->height + e->y + (widget->core.y + 1);
	
	if (psview->redisplayPixmap == 0 ||
	    (!psview->frozen && psview->redisplayPixmap == psview->dpsPixmap))
	    return;
        if (
	    /* Check if the requested redisplay area is within the current
	       redisplay pixmap*/
	    e->x < -psview->redisplayPixmapOffsetX ||
	    e->y < -psview->redisplayPixmapOffsetY ||
	    (e->width + e->x) > 
		(psview->pixmapWidth - psview->redisplayPixmapOffsetX) ||
	    (e->height + e->y) > 
		(psview->pixmapHeight - psview->redisplayPixmapOffsetY))
	    {
	    /*Here, we need to find a new pixmap or else redo it*/
	    
            if(GetFromCache(widget,e->x,e->y,e->width,e->height))
	        {
		return; /*Found it.  Done.*/
		}
	    if (psview->frozen || psview->waitingInput)
		{ /* Do it only if we are not already working on the page*/
		psview->redopage = TRUE;
		MakeItTrue(widget);
		}
	    }
	else /*Here we found it all in redisplayPixmap*/
	    {
	    XCopyArea(XtDisplay(widget), psview->redisplayPixmap,
	       XtWindow(widget),
	       psview->copyGC,
	       e->x + psview->redisplayPixmapOffsetX,
	       e->y + psview->redisplayPixmapOffsetY,
	       e->width, e->height,
	       e->x, e->y);
	    }

	}
    else if (!psview->windowDrawMode){
	if (psview->redisplayPixmap == 0 ||
	    (psview->context && !psview->frozen && 
		psview->redisplayPixmap == psview->dpsPixmap))
	    /* If there is not a redisplay Pixmap, or if there exists a
	       context which is not currently frozen and it is writing into
	       the redisplay pixmap, then don't redisplay it*/
              {
	        if (!psview->errorRedisplay)
		    /* if there was an error on this page, then ok to 
		       redisplay */
	            return;
	      }	
        XCopyArea(XtDisplay(widget), psview->redisplayPixmap, XtWindow(widget),
	       psview->copyGC,
	       e->x, e->y,
	       e->width, e->height,
	       e->x, e->y);
    } else {
	if (e->count == 0) {
	    psview->redopage = TRUE;
	    MakeItTrue(widget);
	}
    }
}

static void Abort(widget)
PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);

    if (!psview->windowDrawMode) {
	    /* Show what happened so far. */
 	    psview->redisplayPixmap = psview->dpsPixmap;
	    psview->redisplayPixmapOffsetX = psview->dpsPixmapOffsetX;
	    psview->redisplayPixmapOffsetY = psview->dpsPixmapOffsetY;
	    XCopyArea(XtDisplay(widget), psview->redisplayPixmap,
		      XtWindow(widget), psview->copyGC,
		      psview->redisplayPixmapOffsetX,
		      psview->redisplayPixmapOffsetY,
		      widget->core.width, widget->core.height, 0, 0);
    }
    if (psview->skipcount)
      {
	/* Abort in the middle of skipping.  No idea what we might be
	   displaying now*/
	psview->page = ++(psview->curpage);
      }
    (*psview->AppStatusProc)(psview->AppStatusData, ok, psview->page);
    (*psview->AppStatusProc)(psview->AppStatusData, finished, psview->page);
}

static void ViewAbort(widget)
PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    if (psview->context) {
        DPSDestroySpace(DPSSpaceFromContext(psview->context));
	psview->rebuildcontext = TRUE;
        psview->context = 0;
	Abort(widget);
    }
}

static void ErrorAbort(widget)
PSViewWidget widget;
{
    PSViewPart *psview = &(widget->psview);
    if (psview->context) {
	psview->rebuildcontext = TRUE;
	Abort(widget);
        psview->errorRedisplay = TRUE;
    }
}



/****************************************************************
 *
 * Public Routines.  A description of these routines can be found
 * in the header file.
 *
 ****************************************************************/

void PSViewSetFile(w, name, page, initstring)
Widget w;
char *name;
int page;
char *initstring;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    XtFree(widget->psview.filename);
    XtFree(widget->psview.initstring);
    widget->psview.filename = MallocACopy(name);
    widget->psview.page = page;
    widget->psview.initstring = MallocACopy(initstring);
    widget->psview.rebuildcontext = TRUE;
    widget->psview.numpages = NOPAGE;
    InvalidateCache(psview);
    MakeItTrue(widget);
}

void PSViewAbort(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    ViewAbort(widget);
}

void PSErrorAbort(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    ErrorAbort(widget);
}

void PSViewShowPage(w, page)
Widget w;
int page;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);

    widget->psview.page = page;

    if (psview->page < 1) psview->page = 1;
    if ((psview->numpages != NOPAGE) && (psview->page >= psview->numpages))
	psview->page = psview->numpages;
    else
      {
      /*
       * If the document is structured and the page does not exist, find
       * the next one that does exist.
       * Our new algorithm knows that only a partial document may have
       * been read thus far, so we MUST know when we run off the current
       * end so that we can read in the next page of the document
       */
	if (psview->sinfo)
	  {
	    int error;
	    /* Read pages incrementally until we have reached the end
		   or have read up to or beyond the desired page */
	    while(!psview->sinfo->atEnd &&
		(psview->page > psview->sinfo->curlastpage))
	      {
		psview->sinfo = DetermineStructure(psview->file,
		    &error, psview->sinfo);
		if (error)
		  {
		    psview->rebuildcontext = TRUE;
		    (*psview->AppStatusProc)(psview->AppStatusData,
			badComments, psview->page);
		    return;
		  }
	      }

	    /* If we're at the end, and the desired page is past the end,
	       set page to number of pages (which is now known), otherwise
	       move forward until the desired page is found
	    */
	    if (psview->sinfo->atEnd && psview->page > psview->sinfo->numPages)
	      psview->page = psview->sinfo->numPages;
	    else
	      {
		while(psview->sinfo->pagestarts[(psview->page)].start < 0
		    && psview->page <= psview->sinfo->curlastpage )
		  {
		    psview->page++;
		  }
	      }
	    /* If the known number of pages is less than the new (meaning
		we've reached the end of the file) then update it */
	    if ( psview->numpages < psview->sinfo->numPages )
	      {
		psview->numpages = psview->sinfo->numPages;
		(*psview->AppStatusProc)
		  (psview->AppStatusData, noPage, psview->numpages);
	      }

	  }  /* End of { if(psview->sinfo) } */
      }
    MakeItTrue(widget);
}


void PSViewGetPageAndFile(w, file, page)
Widget w;
char **file;
int *page;
{
    /* %%% May not work if disabledepth > 0! */
    PSViewWidget widget = (PSViewWidget) w;
    *file = MallocACopy(widget->psview.filename);
    *page = widget->psview.page;
}


int PSViewGetNumPages(w)
Widget w;
{
    /* %%% May not work if disabledepth > 0! */
    PSViewWidget widget = (PSViewWidget) w;
    if (widget->psview.sinfo) return widget->psview.sinfo->numPages;
    else return widget->psview.numpages;
}


void PSViewSetProcs(w, textProc, errorProc)
Widget w;
PSViewProc textProc, errorProc;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    widget->psview.textProc = textProc;
    psview->appErrorProc = errorProc;
}


void PSViewGetProcs(w, textProc, errorProc)
Widget w;
PSViewProc *textProc, *errorProc;
{
    PSViewWidget widget = (PSViewWidget) w;
    *textProc = widget->psview.textProc;
    *errorProc = widget->psview.errorProc;
}



void PSViewSetOrientation(w, orientation)
Widget w;
int orientation;	/* One of 0, 90, 180, 270 */
{
  PSViewWidget widget = (PSViewWidget) w;
  PSViewPart *psview = &(widget->psview);
#ifdef NOTDEF
    if (psview->orientation == 0 || psview->orientation == 180) {
        if (orientation == 90 || orientation == 270)
	    XtMakeResizeRequest (w, widget->core.height, widget->core.width,
		NULL, NULL);
    } else if (psview->orientation == 90 || psview->orientation == 270) {
	if (orientation == 0 || orientation == 180)
	    XtMakeResizeRequest (w, widget->core.height, widget->core.width,
		NULL, NULL);
    }
#endif /* NOTDEF */
    psview->orientation = orientation;
    psview->rebuildcontext = TRUE;
    InvalidateCache(psview);
    MakeItTrue(widget);
}

void PSViewSetResolution(w, xdpi, ydpi)
Widget w;
double xdpi;	/* Pixels per inch in the horizontal direction. */
double ydpi;	/* Pixels per inch in the vertical direction. */
{
    PSViewWidget widget = (PSViewWidget) w;
    widget->psview.xdpi = xdpi;
    widget->psview.ydpi = ydpi;
    widget->psview.rebuilddevice = TRUE;
    MakeItTrue(widget);
}


void PSViewGetResolution(w, xdpi, ydpi)
Widget w;
double *xdpi;	/* Pixels per inch in the horizontal direction. */
double *ydpi;	/* Pixels per inch in the vertical direction. */
{
    PSViewWidget widget = (PSViewWidget) w;
    *xdpi = widget->psview.xdpi;
    *ydpi = widget->psview.ydpi;
}

#if CDA_EXPAND_PROTO == 1

void PSViewSetParseComments(Widget	w,
			    Boolean	value)

#else

void PSViewSetParseComments(w, value)
Widget w;
Boolean value;

#endif

{
    PSViewWidget widget = (PSViewWidget) w;
    if (value != widget->psview.parseComments) {
	widget->psview.parseComments = value;
	widget->psview.rebuildcontext = TRUE;
	MakeItTrue(widget);
    }
}

Boolean PSViewGetParseComments(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    return widget->psview.parseComments;
}

Boolean PSViewIsStructured(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    if (widget->psview.sinfo)
        return TRUE;
    else 
	return FALSE;
}

#if CDA_EXPAND_PROTO == 1

void PSViewSetHeaderRequired(Widget		w,
			     Boolean		flag)

#else

void PSViewSetHeaderRequired(w, flag)
	Widget w;
       Boolean flag;
#endif
{
       PSViewWidget widget = (PSViewWidget) w;

       PSViewPart *psview = &(widget->psview);
       psview->headerRequired = flag;
}
#if CDA_EXPAND_PROTO == 1

Boolean PSViewGetHeaderRequired(Widget w)

#else

Boolean PSViewGetHeaderRequired(w)
       PSViewWidget  w;

#endif

{
       PSViewWidget widget = (PSViewWidget) w;
       PSViewPart *psview = &(widget->psview);
       return psview->headerRequired;
}

void PSViewNextPage(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);

    /* We should not have to worry about running
     * off the end, since the current page must not be the last one, but just
     * in case someone else uses this widget, we'll check anyway and wrap around
     * if necessary.  We really don't need to worry about there being no valid
     * pages, though because otherwise, it would not be considered structured.
    */

    if (psview->page < 1) psview->page = 1;
    if ((psview->numpages != NOPAGE) && (psview->page >= psview->numpages))
	psview->page = psview->numpages;
    else
	{
    /*
     * If the document is structured and the page does not exist, find
     * the next one that does exist.  We don't need to worry about running
     * off the end, because we are guaranteed that we are starting with a
     * page less than last page, and last page always exists.
     */
	if (psview->sinfo)
	    {
	    while(psview->sinfo->pagestarts[++(psview->page)].start < 0
		&& psview->page <= psview->sinfo->curlastpage );
	    }
	else
	    psview->page++;

	if (psview->sinfo)
	  {
	    if ( psview->page > psview->sinfo->curlastpage ) /* Need more */
	      {
	        int error;
	        if ( psview->sinfo->atEnd )
	          psview->page = psview->sinfo->numPages;
	        else
	          {
	            psview->sinfo = DetermineStructure(psview->file,
		      &error, psview->sinfo);
	            if (error)
	              {
		        psview->rebuildcontext = TRUE;
		        (*psview->AppStatusProc)(psview->AppStatusData,
		          badComments, psview->page);
		        return;
	              }
	          }
	        /* Adjust known number of pages if it changes downward */
	        if ( psview->numpages < psview->sinfo->numPages )
	          {
		    psview->numpages = psview->sinfo->numPages;
		    (*psview->AppStatusProc)
		      (psview->AppStatusData, noPage, psview->numpages);
	          }
	      }
	  } /* End of { if (psview->sinfo) } */

	}  /* End of else */
    MakeItTrue(widget);
} 


void PSViewPreviousPage(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    int oldpage = psview->page;

    if (psview->numpages != NOPAGE)
	if (psview->page > psview->numpages)
	    psview->page = psview->numpages;
    if (psview->page > 1)
	{
	/*
	 * Here we are starting on a valid and non-extreme page number so it
	 * is ok to try to find a page
	 */

	if (psview->sinfo)
	    {
	    /*
	     * Here, we have a structured document.  Look for the previous
	     * page which is valid.  If we run off the beginning, that means
	     * we were already at the first valid page, so restore the original
	     */
	    while(psview->sinfo->pagestarts[--(psview->page)].start < 0)
		{
		if (psview->page == 1)
		    {
		    psview->page = oldpage;
		    break;
		    }
		}
	    }
	else
	    psview->page--;
	}
    MakeItTrue(widget);
}

void PSViewDisableRedisplay(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    widget->psview.disabledepth++;
}




void PSViewEnableRedisplay(w)
Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
   
    if (XtIsRealized(widget))
      /*  if the widget is not yet realized, this is deferred til
       *  the widget's realize proc
       */

      {
    	if (widget->psview.disabledepth > 0)
	   widget->psview.disabledepth--;
    	MakeItTrue(widget);
      }
}


void PSWindowFlushPostScript(w)
    Widget w;
{
    PSViewWidget widget = (PSViewWidget) w;
    DPSFlushContext(widget->psview.context);
}    


void PSWindowSendPostScript(w, ptr, length)
    Widget w;
    char *ptr;
    int length;
{
        PSViewWidget widget = (PSViewWidget) w;
    DPSWritePostScript(widget->psview.context, ptr, (unsigned int) length);
}


#define MAX_STATUS 100
#define LOOP_INCREMENT(arg) arg = (arg+1) % MAX_STATUS
static int status_keeper[MAX_STATUS];
static DPSContext ctx_keeper[MAX_STATUS];
static int status_first_used = 0,status_first_free=0;
static int entry = 0;

static void PSViewHandleStatusEvent(ctx, status)
  DPSContext ctx;
  int status;
{
    PSViewWidget widget = (PSViewWidget) ctx->priv;

	switch (status) {
	    case PSFROZEN:
		NoticeFrozen(widget);
		break;
	    case PSNEEDSINPUT:
		ProcessInputStatus(widget);
		break;
	    case PSZOMBIE:
		{
		/* server is in a zombied wait state, break it out;
		 * we should still get error messages via text backstop
		 */
		ErrorAbort(widget);
		}
		break;
			
	}


}

void PSViewSetStatusProc(w, proc, pv)
  Widget w;
  void (*proc)();
  Opaque pv;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->AppStatusProc = proc;
    psview->AppStatusData = pv;    
}


void PSViewSetScale(w, scale)
  Widget w;
  double scale;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->scale = scale;
    psview->rebuildcontext = TRUE;
    InvalidateCache(psview);
    MakeItTrue(widget);
}

void PSViewFakeTrays(w, enable)
  Widget w;
  Boolean enable;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->fakeTrays = enable;
    psview->rebuildcontext = TRUE;
}

void PSViewBitmapWidths(w, enable)
    Widget w;
    Boolean enable;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->bitmapWidths = enable;
    psview->rebuildcontext = TRUE;
    MakeItTrue(widget);
}    

void PSViewSetWindowDrawMode(w, enable)
  Widget w;
  Boolean enable;
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);
    psview->newWindowDrawMode = enable;
    psview->rebuilddevice = TRUE;
}

int PSViewGetDocumentInfo(w,infoPtr)
    Widget w;
    char **infoPtr[];
{
    PSViewWidget widget = (PSViewWidget) w;
    PSViewPart *psview = &(widget->psview);

    if (psview->sinfo)
	{
	*infoPtr = psview->sinfo->docInfo;
	return psview->sinfo->numDocInfo;
	}
    else
	return 0;
}
/****************************************************************
 *
 * Class record definition and initialization.
 *
 ****************************************************************/

#ifdef __vms__
#pragma nostandard
#endif

externaldef(psviewClassRec) PSViewClassRec psviewClassRec = {

#ifdef __vms__
#pragma standard
#endif

  {
/* core_class fields      */
    /* superclass         */    (WidgetClass) &widgetClassRec,
    /* class_name         */    "PSView",
    /* widget_size        */    sizeof(PSViewRec),
    /* class_initialize   */    ClassInitialize,
    /* class_part_initiali*/	NULL,
    /* class_inited       */	FALSE,
    /* initialize         */    (XtInitProc) Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    Realize,
    /* actions            */    NULL,
    /* num_actions	  */	0,
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion	  */	TRUE,
    /* compress_exposure  */	FALSE,
    /* compress_enterleave*/	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    Redisplay,
    /* set_values         */    (XtSetValuesFunc) SetValues,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	NULL,
    /* get_values_hook    */	NULL,
    /* accept_focus       */    NULL,
    /* version    	  */	XtVersion,
    /* callback_private   */	NULL,
    /* tm_table		  */	NULL,
    /* query_geometry     */	NULL,
    /* display_accelerator*/	NULL,
    /* extension          */	NULL,
  },{
    /* psInited		  */	FALSE,
  }
};

#ifdef __vms__
#pragma nostandard
#endif

externaldef(psviewWidgetClass) WidgetClass psviewWidgetClass = (WidgetClass)&psviewClassRec;

#ifdef __vms__
#pragma standard
#endif

Widget PSViewWidgetCreate(parent, name, args, num_args)
Widget parent;
char *name;
Arg *args;
Cardinal num_args;
{
    return XtCreateWidget(name, psviewWidgetClass , parent, args, num_args);
}


void PSViewInitializeForDRM ()
{
  int	stat;
    stat = MrmRegisterClass (MrmwcUnknown, "ps",
	    "PSViewWidgetCreate", PSViewWidgetCreate,  psviewWidgetClass);
    if (stat != MrmSUCCESS) {
	fprintf (stderr, "PSView widget registration failed\n");
	/*exit(1);*/
    }
}
