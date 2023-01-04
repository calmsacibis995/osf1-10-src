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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmIDecor.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:24:41 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */


#include "WmGlobal.h"
#include <Xm/Xm.h>
#include <Xm/DrawP.h>   	/* for _XmClearBorder */
/*
 * include extern functions
 */
#include "WmIDecor.h"
#include "WmError.h"
#include "WmGraphics.h"
#include "WmIconBox.h"
#include "WmMenu.h"
#include "WmWinInfo.h"



/*
 * Global Variables:
 */
static unsigned int activeIconTextWidth = 1;
static unsigned int activeIconTextHeight = 1;
static RList *pActiveIconTopRects = NULL;
static RList *pActiveIconBotRects = NULL;

static int  iconShrinkX; 
static int  iconShrinkY;
static unsigned int iconShrinkWidth;
static unsigned int iconShrinkHeight;


/*************************************<->*************************************
 *
 *  MakeIcon (pWS, pcd)
 *
 *
 *  Description:
 *  -----------
 *  Create an icon frame and fill it in as appropriate for the client.
 *
 *
 *  Inputs:
 *  ------
 *  pWS		- pointer to workspace data
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *  Return 	- TRUE if success, FALSE if failure.
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean MakeIcon (pWS, pcd)
    WmWorkspaceData *pWS;
    ClientData *pcd;

#else /* _NO_PROTO */
Boolean MakeIcon (WmWorkspaceData *pWS, ClientData *pcd)
#endif /* _NO_PROTO */
{
    XSetWindowAttributes window_attribs;
    unsigned long attr_mask;
    int xOffset;
    int yOffset;


    /*
     * Common to all icons
     */

    /* compute dimensions of outer icon frame */
    /* create icon frame window */

    attr_mask = CWEventMask | CWCursor;
#ifdef AUTOMATION
    window_attribs.event_mask = (ButtonPressMask | ButtonReleaseMask |
				 KeyPressMask | KeyReleaseMask |
				 SELECT_BUTTON_MOTION_MASK | 
				 DMANIP_BUTTON_MOTION_MASK |
				 ExposureMask |
				 FocusChangeMask);
#else
    window_attribs.event_mask = (ButtonPressMask | ButtonReleaseMask |
				 SELECT_BUTTON_MOTION_MASK |
				 DMANIP_BUTTON_MOTION_MASK |
				 ExposureMask |
				 FocusChangeMask);
#endif /* AUTOMATION */
    window_attribs.cursor = wmGD.workspaceCursor;

    if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
	(wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER))
    {
	window_attribs.event_mask |= EnterWindowMask | LeaveWindowMask;
    }

    /* 
     * Use background pixmap if one is specified, otherwise set the
     * appropriate background color. 
     */

    if (ICON_APPEARANCE(pcd).backgroundPixmap)
    {
	attr_mask |= CWBackPixmap;
	window_attribs.background_pixmap = 
			    ICON_APPEARANCE(pcd).backgroundPixmap;
    }
    else
    {
	attr_mask |= CWBackPixel;
	window_attribs.background_pixel = 
				ICON_APPEARANCE(pcd).background;
    }
   
    if ((!pcd->pSD->useIconBox) || (pcd->clientFlags & ICON_BOX))
    {
	pcd->iconFrameWin = XCreateWindow (DISPLAY,
			       ROOT_FOR_CLIENT(pcd),	/* parent */
			       pcd->iconX,
			       pcd->iconY,
			       (unsigned int) ICON_WIDTH(pcd),
			       (unsigned int) ICON_HEIGHT(pcd),
			       0,		/* border width */
			       CopyFromParent,	/* depth */
			       InputOutput,	/* class */
			       CopyFromParent,	/* visual */
			       attr_mask,
			       &window_attribs);

    }
    else
    {
        /*
         * Insert the icon into the icon box.
         * Don't make icon in the box for any icon box (or any WM window)
	 * OR any client that doesn't have the MWM_FUNC_MINIMIZE bit set
	 * in pcd->clientFunctions
         */

        if ((pcd->pSD->useIconBox) && 
	    (!(pcd->clientFlags & ICON_BOX)) &&
	    (pcd->clientFunctions & MWM_FUNC_MINIMIZE) )
        {
            if (!InsertIconIntoBox(pWS->pIconBox, pcd))
		Warning("Could not make icon to go in icon box");

	}

    }


    /* make space for the top/bottom changing shadow rectangles */

    if ((pcd->piconTopShadows = 
	    AllocateRList ((unsigned)NUM_BOTH_TOP_RECTS)) == NULL)
    {
	/* Out of memory! */
	Warning ("Insufficient memory for icon creation");
	return(FALSE);
    }
    
    if ((pcd->piconBottomShadows = 
	 AllocateRList ((unsigned)NUM_BOTH_BOTTOM_RECTS)) == NULL)
    {
	/* Out of memory! */
	Warning ("Insufficient memory for icon creation");
	return(FALSE);
    }


    /*
     * Adjust for icons in the box 
     * Don't adjust the icon for the icon box itself
     */

    if (pWS->pIconBox && (pWS->pIconBox->pCD_iconBox != pcd))
    {
	xOffset = IB_MARGIN_WIDTH;
	yOffset = IB_MARGIN_HEIGHT;
    }
    else
    {
	xOffset = 0;
        yOffset = 0;
    }


    /*
     * Reparent the icon window if there is one
     */
    if ((ICON_DECORATION(pcd) & ICON_IMAGE_PART) && 
	(pcd->iconWindow))
    {
	ReparentIconWindow (pcd, xOffset, yOffset);
    }

    MakeIconShadows (pcd, xOffset, yOffset);

    return(TRUE);

} /* END OF FUNCTION MakeIcon */



/*************************************<->*************************************
 *
 *  MakeIconShadows (pcd, xOffset, yOffset)
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MakeIconShadows (pcd, xOffset, yOffset)
    ClientData *pcd;
    int xOffset;
    int yOffset;

#else /* _NO_PROTO */
void MakeIconShadows (ClientData *pcd, int xOffset, int yOffset)
#endif /* _NO_PROTO */
{        

    /*
     * Handle different icon styles
     */
    
    switch (ICON_DECORATION(pcd) & (ICON_LABEL_PART | ICON_IMAGE_PART)) 
    {
	case ICON_LABEL_PART:
	    BevelRectangle (pcd->piconTopShadows, 	/* label */
			    pcd->piconBottomShadows, 
			    0 + xOffset, (int)ICON_IMAGE_HEIGHT(pcd) + yOffset,
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_LABEL_HEIGHT(pcd),
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH);
	    break;

	case ICON_IMAGE_PART:
	    BevelRectangle (pcd->piconTopShadows, 	/* image outside */
			    pcd->piconBottomShadows, 
			    0 + xOffset, 0 + yOffset,
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_IMAGE_HEIGHT(pcd),
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH);

	    BevelRectangle (pcd->piconBottomShadows, 	/* image inside */
			    pcd->piconTopShadows, 
			    ICON_INNER_X_OFFSET + xOffset,
			    ICON_INNER_Y_OFFSET + yOffset,
			    (unsigned int) (ICON_IMAGE_MAXIMUM(pcd).width + 
				4*ICON_INTERNAL_SHADOW_WIDTH),
			    (unsigned int) (ICON_IMAGE_MAXIMUM(pcd).height + 
				4*ICON_INTERNAL_SHADOW_WIDTH),
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH);

	    break;

	case (ICON_IMAGE_PART | ICON_LABEL_PART):
	    BevelRectangle (pcd->piconTopShadows, 	/* image outside */
			    pcd->piconBottomShadows, 
			    0 + xOffset, 0 + yOffset, 
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_IMAGE_HEIGHT(pcd),
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH);

	    BevelRectangle (pcd->piconBottomShadows, 	/* image inside */
			    pcd->piconTopShadows, 
			    ICON_INNER_X_OFFSET + xOffset,
			    ICON_INNER_Y_OFFSET + yOffset,
			    (unsigned int) (ICON_IMAGE_MAXIMUM(pcd).width + 
				4*ICON_INTERNAL_SHADOW_WIDTH),
			    (unsigned int) (ICON_IMAGE_MAXIMUM(pcd).height + 
				4*ICON_INTERNAL_SHADOW_WIDTH),
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_INTERNAL_SHADOW_WIDTH);

	    BevelRectangle (pcd->piconTopShadows, 	/* label */
			    pcd->piconBottomShadows, 
			    0 + xOffset, (int)ICON_IMAGE_HEIGHT(pcd) + yOffset,
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_LABEL_HEIGHT(pcd),
			    ICON_INTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH);

	    break;
    }

} /* END OF FUNCTION MakeIconShadows */






/*************************************<->*************************************
 *
 *  IconExposureProc (pcd, expose)
 *
 *
 *  Description:
 *  -----------
 *  Repaint the icon.
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void IconExposureProc (pcd, expose)
    ClientData *pcd;
    Boolean expose;

#else /* _NO_PROTO */
void IconExposureProc (ClientData *pcd, Boolean expose)
#endif /* _NO_PROTO */
{

    Pixmap image;
    int dest_x, dest_y;
    int xOffset;
    int yOffset;
    unsigned int width, height;
    GC iconGC, topGC, botGC;
    static XRectangle	shrinkRects[4];



    /*
     * Adjust for icons in the iconBox
     */

    if (P_ICON_BOX(pcd))
    {
	xOffset = IB_MARGIN_WIDTH;
	yOffset = IB_MARGIN_HEIGHT;
    }
    else
    {
	xOffset = 0;
        yOffset = 0;
    }
    
    /* get appropriate GCs */

    if ((wmGD.keyboardFocus == pcd) && (pcd->clientState == MINIMIZED_STATE))
    {
	iconGC = ICON_APPEARANCE(pcd).activeGC;
	topGC = ICON_APPEARANCE(pcd).activeTopShadowGC;
	botGC = ICON_APPEARANCE(pcd).activeBottomShadowGC;
    }
    else
    {
	iconGC = ICON_APPEARANCE(pcd).inactiveGC;
	topGC = ICON_APPEARANCE(pcd).inactiveTopShadowGC;
	botGC = ICON_APPEARANCE(pcd).inactiveBottomShadowGC;
    }

    if (ACTIVE_PSD->useIconBox && P_ICON_BOX(pcd))
    {
	/* draw shadowing */

	if (expose)
	{
	    XClearArea (DISPLAY, 
			    ICON_FRAME_WIN(pcd), 
			    IB_MARGIN_WIDTH, 
			    IB_MARGIN_HEIGHT, 
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_HEIGHT(pcd), False);
	}

	if (pcd->clientState == MINIMIZED_STATE)
	{
	    /*
	     * This is the "raised" icon appearance
	     */

	    if (pcd->piconTopShadows)
	    {

	        XFillRectangles (DISPLAY, 
				    ICON_FRAME_WIN(pcd), 
				    topGC,
				    pcd->piconTopShadows->prect,
				    pcd->piconTopShadows->used);
	    }

	    if (pcd->piconBottomShadows)
	    { 
	        XFillRectangles (DISPLAY,
				    ICON_FRAME_WIN(pcd), 
				    botGC,
				    pcd->piconBottomShadows->prect,
				    pcd->piconBottomShadows->used);
	    }
	}
	else 
	{
	    shrinkRects[0].x = IB_MARGIN_WIDTH;
	    shrinkRects[0].y = IB_MARGIN_HEIGHT;
	    shrinkRects[0].width = (unsigned int) ICON_WIDTH(pcd);
	    shrinkRects[0].height = iconShrinkY - IB_MARGIN_HEIGHT;

	    shrinkRects[1].x = IB_MARGIN_WIDTH;
	    shrinkRects[1].y = iconShrinkY;
	    shrinkRects[1].width = iconShrinkX - IB_MARGIN_WIDTH;
	    shrinkRects[1].height = iconShrinkHeight;

	    shrinkRects[2].x = iconShrinkX + iconShrinkWidth;
	    shrinkRects[2].y = iconShrinkY;
	    shrinkRects[2].width = iconShrinkX - IB_MARGIN_WIDTH;
	    shrinkRects[2].height = iconShrinkHeight;

	    shrinkRects[3].x = IB_MARGIN_WIDTH;
	    shrinkRects[3].y = iconShrinkY + iconShrinkHeight;
	    shrinkRects[3].width = (unsigned int) ICON_WIDTH(pcd);
	    shrinkRects[3].height = iconShrinkY - IB_MARGIN_HEIGHT;

	    XFillRectangles (DISPLAY, 
			    ICON_FRAME_WIN(pcd), 
			    SHRINK_WRAP_GC(pcd),
			    &shrinkRects[0], 4);
			    
	}

    }
    else 
    {
	/* draw shadowing */

	if (pcd->clientState == MINIMIZED_STATE)
	{
	    /*
	     * This is the "raised" icon appearance
	     */

	    if (pcd->piconTopShadows->prect)
	    {

		XFillRectangles (DISPLAY, 
				ICON_FRAME_WIN(pcd), 
				topGC,
				pcd->piconTopShadows->prect,
				pcd->piconTopShadows->used);
	    }

	    if (pcd->piconBottomShadows->prect)
	    { 
		XFillRectangles (DISPLAY,
				ICON_FRAME_WIN(pcd), 
				botGC,
				pcd->piconBottomShadows->prect,
				pcd->piconBottomShadows->used);
	    }

	}
    }


    /* draw icon text */
/*
    if ((ICON_DECORATION(pcd) & ICON_LABEL_PART) &&
	(expose || !(ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART)))
*/
    if (ICON_DECORATION(pcd) & ICON_LABEL_PART)
    {
	DrawIconTitle (pcd);
    }

    /* 
     * Draw image if no icon window (client has to redraw that!) 
     *  OR if using the iconbox, draw the default image where
     *  the icon window was.
     */

    if (expose &&
	((!pcd->iconWindow && (ICON_DECORATION(pcd) & ICON_IMAGE_PART)) ||
         (ACTIVE_PSD->useIconBox && P_ICON_BOX(pcd) &&
		pcd->iconWindow && 
		pcd->clientState != MINIMIZED_STATE &&
		(ICON_DECORATION(pcd) & ICON_IMAGE_PART))))
    {
	if (pcd->iconWindow)
	{
	    image = DEFAULT_PIXMAP(pcd);
	}
	else
	{
	    image = pcd->iconPixmap;
	}

	if (image)
	{

	    if ((ACTIVE_PSD->useIconBox) && (P_ICON_BOX(pcd)))
	    {

		if (pcd->clientState != MINIMIZED_STATE)
		{
		    dest_x = ICON_IMAGE_X_OFFSET 
			    + ICON_INTERNAL_SHADOW_WIDTH
			    + xOffset;

		    dest_y = ICON_IMAGE_Y_OFFSET 
			    + ICON_INTERNAL_SHADOW_WIDTH
			    + yOffset;

		    width = ICON_IMAGE_MAXIMUM(pcd).width;
		    height= ICON_IMAGE_MAXIMUM(pcd).height;
		    XCopyArea (DISPLAY, image, 
				ICON_FRAME_WIN(pcd), 
				iconGC,
		                ICON_INTERNAL_SHADOW_WIDTH, 
				ICON_INTERNAL_SHADOW_WIDTH, 
				width, height, dest_x, dest_y);

		    if (FADE_NORMAL_ICON(pcd))
		    {
			iconGC = FADE_ICON_GC(pcd);
			XFillRectangle (DISPLAY, 
				    ICON_FRAME_WIN(pcd), 
				    iconGC,
				    dest_x, dest_y,
				    width, height);
		    }

		}
		else
		{
		    dest_x = ICON_IMAGE_X_OFFSET 
			    + xOffset;

		    dest_y = ICON_IMAGE_Y_OFFSET 
			    + yOffset;

		    width = ICON_IMAGE_MAXIMUM(pcd).width
		    	    + (2 * ICON_INTERNAL_SHADOW_WIDTH);
		    height= ICON_IMAGE_MAXIMUM(pcd).height
		    	    + (2 * ICON_INTERNAL_SHADOW_WIDTH);

	            XCopyArea (DISPLAY, image, 
				ICON_FRAME_WIN(pcd), 
				iconGC, 0, 0, width, height, 
				dest_x, dest_y);

		}
	    }
	    else

	    {
		width = ICON_IMAGE_MAXIMUM(pcd).width +
			2 * ICON_INTERNAL_SHADOW_WIDTH;

		height= ICON_IMAGE_MAXIMUM(pcd).height +
			2 * ICON_INTERNAL_SHADOW_WIDTH;

		dest_x = ICON_INNER_X_OFFSET + ICON_INTERNAL_SHADOW_WIDTH;
		dest_y = ICON_INNER_Y_OFFSET + ICON_INTERNAL_SHADOW_WIDTH;
		XCopyArea (DISPLAY, image, 
			    ICON_FRAME_WIN(pcd), 
			    iconGC, 0, 0, width, height, 
			    dest_x, dest_y);


	    }

	}
    }


} /* END OF FUNCTION IconExposureProc */




/*************************************<->*************************************
 *
 *  GetIconTitleBox (pcd, pBox)
 *
 *
 *  Description:
 *  -----------
 *  Returns a rectangle containing the icon text box
 *
 *
 *  Inputs:
 *  ------
 *  pcd - pointer to client data
 *  pBox - pointer to an XRectangle structure that gets returned data
 *
 *  Outputs:
 *  -------
 *  pBox - returned data
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void GetIconTitleBox (pcd, pBox)
    ClientData *pcd;
    XRectangle *pBox;

#else /* _NO_PROTO */
void GetIconTitleBox (ClientData *pcd, XRectangle *pBox)
#endif /* _NO_PROTO */
{
    int xOffset;
    int yOffset;


    /*
     * Adjust for icons in the iconBox
     */

    if (P_ICON_BOX(pcd))
    {
	xOffset = IB_MARGIN_WIDTH;
	yOffset = IB_MARGIN_HEIGHT;
    }
    else
    {
	xOffset = 0;
        yOffset = 0;
    }

    if ((P_ICON_BOX(pcd)) && (pcd->clientState != MINIMIZED_STATE))
    {
	/* move label up to enhance shrink wrap effect */
	pBox->x = ICON_EXTERNAL_SHADOW_WIDTH 
		+ ICON_IMAGE_LEFT_PAD
		+ (2 * ICON_INTERNAL_SHADOW_WIDTH)
		+ ICON_IMAGE_LEFT_PAD
		+ xOffset;

	pBox->y = ICON_IMAGE_HEIGHT(pcd) 
		+  yOffset
                + ((ICON_IMAGE_HEIGHT(pcd) > 0) 
			? - ICON_IMAGE_BOTTOM_PAD
			: ICON_EXTERNAL_SHADOW_WIDTH)
		+  ((ICON_IMAGE_HEIGHT(pcd) >0)
			? 0
			: WM_TOP_TITLE_PADDING );




	pBox->width = ICON_IMAGE_MAXIMUM(pcd).width 
			- ICON_IMAGE_LEFT_PAD
			- ICON_EXTERNAL_SHADOW_WIDTH;

	pBox->height = TEXT_HEIGHT(ICON_APPEARANCE(pcd).font);

    }
    else if ((P_ICON_BOX(pcd)) && (pcd->clientState == MINIMIZED_STATE))
    {
	/* move label up to enhance shrink wrap effect */
	pBox->x = ICON_EXTERNAL_SHADOW_WIDTH 
		+ ICON_IMAGE_LEFT_PAD
		+ (2 * ICON_INTERNAL_SHADOW_WIDTH)
		+ ICON_IMAGE_LEFT_PAD
		+ xOffset;

	pBox->y = ICON_IMAGE_HEIGHT(pcd) 
		+  yOffset
                + ((ICON_IMAGE_HEIGHT(pcd) > 0) 
			? ICON_INTERNAL_SHADOW_WIDTH 
			: ICON_EXTERNAL_SHADOW_WIDTH)
		+  WM_TOP_TITLE_PADDING ;


	pBox->width = ICON_IMAGE_MAXIMUM(pcd).width 
			- ICON_IMAGE_LEFT_PAD;

	pBox->height = TEXT_HEIGHT(ICON_APPEARANCE(pcd).font);

    }
    else
    {
	pBox->x = ICON_EXTERNAL_SHADOW_WIDTH 
		+ WM_TOP_TITLE_PADDING
		+ xOffset;

	pBox->y = ICON_IMAGE_HEIGHT(pcd) 
		+  WM_TOP_TITLE_PADDING 
		+  yOffset
                + ((ICON_IMAGE_HEIGHT(pcd) > 0) 
			? ICON_INTERNAL_SHADOW_WIDTH 
			: ICON_EXTERNAL_SHADOW_WIDTH);

	pBox->width = ICON_WIDTH(pcd) - 2 * ICON_EXTERNAL_SHADOW_WIDTH - 
		  WM_TOP_TITLE_PADDING - WM_BOTTOM_TITLE_PADDING;
	pBox->height = TEXT_HEIGHT(ICON_APPEARANCE(pcd).font);

    }


} /* END OF FUNCTION GetIconTitleBox */



/*************************************<->*************************************
 *
 *  DrawIconTitle (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Draws the title in the Icon title area
 *
 *
 *  Inputs:
 *  ------
 *  pcd - pointer to client data
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void DrawIconTitle (pcd)
    ClientData *pcd;

#else /* _NO_PROTO */
void DrawIconTitle (ClientData *pcd)
#endif /* _NO_PROTO */
{
    XRectangle textBox;
    GC iconGC;
    
    
    GetIconTitleBox (pcd, &textBox);

    /* get appropriate GCs */
    if ((ACTIVE_PSD->useIconBox && !(pcd->clientFlags & ICON_BOX)) || 
	!(wmGD.keyboardFocus == pcd)) 
    {
	iconGC = ICON_APPEARANCE(pcd).inactiveGC;
    }
    else 
    {
	iconGC = ICON_APPEARANCE(pcd).activeGC;
    }

    /* 
     * Dim text if this is in the icon box and the client is mapped 
     */

    if ((ACTIVE_PSD->useIconBox) && 
	(P_ICON_BOX(pcd)) &&
	(FADE_NORMAL_ICON(pcd)) && 
	(!(pcd->clientState == MINIMIZED_STATE)))
    {
	    iconGC = FADE_ICON_TEXT_GC(pcd);
    }




    /* paint the text */
    WmDrawXmString(DISPLAY, ICON_FRAME_WIN(pcd), ICON_APPEARANCE(pcd).fontList,
		   pcd->iconTitle, iconGC, 
		   textBox.x, textBox.y, textBox.width, &textBox);

} /* END OF FUNCTION DrawIconTitle */




/*************************************<->*************************************
 *
 *  RedisplayIconTitle (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Draws the title in the Icon title area
 *
 *
 *  Inputs:
 *  ------
 *  pcd - pointer to client data
 *
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void RedisplayIconTitle (pcd)
    ClientData *pcd;
#else /* _NO_PROTO */
void RedisplayIconTitle (ClientData *pcd)
#endif /* _NO_PROTO */
{
    XRectangle textBox;
    GC iconGC;

    /*
     * only proceed if we've got the right icon parts to work on
     */

    if (ICON_DECORATION(pcd) & ICON_LABEL_PART && ICON_FRAME_WIN(pcd))
    {

	/* nothing to do if no labels */
	if (!(ICON_DECORATION(pcd) & ICON_LABEL_PART))
	    return;

	/* get the box that the text sits in */
	GetIconTitleBox (pcd, &textBox);

	/* 
	 * Get appropriate GCs 
	 * Dim text if this is in the icon box and the client is mapped 
	 */
	if ((ACTIVE_PSD->useIconBox && (P_ICON_BOX(pcd)) &&
	    !(pcd->clientFlags & ICON_BOX)) || 
	    !(wmGD.keyboardFocus == pcd)) 
	{
	    iconGC = ICON_APPEARANCE(pcd).inactiveGC;
	}
	else 
	{
	    iconGC = ICON_APPEARANCE(pcd).activeGC;
	}

	if ((ACTIVE_PSD->useIconBox) && 
	    (P_ICON_BOX(pcd)) &&
	    (FADE_NORMAL_ICON(pcd)) && 
	    (!(pcd->clientState == MINIMIZED_STATE)))
	{
	    iconGC = FADE_ICON_TEXT_GC(pcd);
	}

	/* out with the old */
	XClearArea (DISPLAY, 
	    ICON_FRAME_WIN(pcd), 
	    textBox.x, textBox.y,
	    (unsigned int) textBox.width, (unsigned int) textBox.height, 
	    FALSE);

	/* in with the new */
	WmDrawXmString(DISPLAY, ICON_FRAME_WIN(pcd), 
		       ICON_APPEARANCE(pcd).fontList,
		       pcd->iconTitle, iconGC, 
		       textBox.x, textBox.y, textBox.width, &textBox);

	/* 
	 * Erase & paint text in the active icon text window
	 */
	if ((wmGD.keyboardFocus == pcd) && 
	    (ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART))
	{
	    PaintActiveIconText (pcd, True);
	}
    }
} /* END OF FUNCTION  RedisplayIconTitle */



/*************************************<->*************************************
 *
 *  GetIconDimensions (pSD, &pWidth, &pLabelHeight, &pImageHeight)
 *
 *
 *  Description:
 *  -----------
 *  returns dimensions of icon frame parts
 *
 *
 *  Inputs:
 *  ------
 *  pSD		  - pointer to screen data
 *  pWidth	  - pointer to width of frame
 *  pLabelHeight  - pointer to height of label part of icon
 *  pImageHeight  - pointer to height of image part of icon 
 *
 * 
 *  Outputs:
 *  -------
 *  *pWidth	  - width of frame
 *  *pLabelHeight - height of label part of icon
 *  *pImageHeight - height of image part of icon 
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void GetIconDimensions (pSD, pWidth, pLabelHeight, pImageHeight)

    WmScreenData *pSD;
    unsigned int *pWidth, *pLabelHeight, *pImageHeight;
#else /* _NO_PROTO */
void GetIconDimensions (WmScreenData *pSD, unsigned int *pWidth, unsigned int *pLabelHeight, unsigned int *pImageHeight)
#endif /* _NO_PROTO */
{
    /*
     * The icon width is always keyed to the icon image maximum regardless
     * of whether an icon image part appears or not.
     */
    *pWidth = pSD->iconImageMaximum.width + 
		ICON_IMAGE_LEFT_PAD +
		ICON_IMAGE_RIGHT_PAD +
		2 * ICON_EXTERNAL_SHADOW_WIDTH +
		4 * ICON_INTERNAL_SHADOW_WIDTH;

    switch (pSD->iconDecoration & (ICON_IMAGE_PART | ICON_LABEL_PART)) 
    {
	case ICON_LABEL_PART:
	    *pImageHeight = 0;

	    *pLabelHeight = ICON_EXTERNAL_SHADOW_WIDTH            +
			    WM_TOP_TITLE_PADDING                  +
			    TEXT_HEIGHT(pSD->iconAppearance.font) +
			    WM_BOTTOM_TITLE_PADDING               +
			    ICON_EXTERNAL_SHADOW_WIDTH;
	    break;

	case ICON_IMAGE_PART:
	    *pImageHeight = ICON_EXTERNAL_SHADOW_WIDTH   +
			    ICON_IMAGE_TOP_PAD           +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    pSD->iconImageMaximum.height + 
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_IMAGE_BOTTOM_PAD        +
			    ICON_EXTERNAL_SHADOW_WIDTH;


	    *pLabelHeight = 0;

	    break;

	case (ICON_IMAGE_PART | ICON_LABEL_PART):
	    *pImageHeight = ICON_EXTERNAL_SHADOW_WIDTH   +
			    ICON_IMAGE_TOP_PAD           +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    pSD->iconImageMaximum.height + 
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_INTERNAL_SHADOW_WIDTH   +
			    ICON_IMAGE_BOTTOM_PAD        +
			    ICON_INTERNAL_SHADOW_WIDTH;

	    *pLabelHeight = ICON_INTERNAL_SHADOW_WIDTH            +
			    WM_TOP_TITLE_PADDING                  +
			    TEXT_HEIGHT(pSD->iconAppearance.font) +
			    WM_BOTTOM_TITLE_PADDING               +
			    ICON_EXTERNAL_SHADOW_WIDTH;

	    break;

	default:
	    *pLabelHeight = *pImageHeight = 0;
	    break;
	    
    }
}


/*************************************<->*************************************
 *
 *  InitIconSize (pSD)
 *
 *
 *  Description:
 *  -----------
 *  set global icon size variables
 *
 *
 *  Inputs:
 *  ------
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void InitIconSize (pSD)

    WmScreenData *pSD;
#else /* _NO_PROTO */
void InitIconSize (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    Cardinal label, image;

    GetIconDimensions (pSD, (unsigned int *)&(pSD->iconWidth), 
	&label, &image);
    
    pSD->iconHeight = label+image;

    pSD->iconImageHeight = image;
    pSD->iconLabelHeight = label;


    iconShrinkX =   IB_MARGIN_WIDTH 
		  + ICON_EXTERNAL_SHADOW_WIDTH
		  + ICON_IMAGE_LEFT_PAD
		  + 2 * ICON_INTERNAL_SHADOW_WIDTH;

		
    iconShrinkY =   IB_MARGIN_HEIGHT
		  + ICON_EXTERNAL_SHADOW_WIDTH 
		  + ((pSD->iconDecoration & ICON_IMAGE_PART)
			? (ICON_IMAGE_TOP_PAD + 
			    (2 * ICON_INTERNAL_SHADOW_WIDTH))
			: (WM_TOP_TITLE_PADDING));


    iconShrinkWidth  =  pSD->iconImageMaximum.width ;




    switch (pSD->iconDecoration & (ICON_IMAGE_PART | ICON_LABEL_PART)) 
    {
	case ICON_LABEL_PART:
	    iconShrinkHeight = TEXT_HEIGHT(pSD->iconAppearance.font);
	    break;

	case ICON_IMAGE_PART:
	    iconShrinkHeight = pSD->iconImageMaximum.height;

	    break;

	case (ICON_IMAGE_PART | ICON_LABEL_PART):
	    iconShrinkHeight =  pSD->iconHeight
		    - ICON_EXTERNAL_SHADOW_WIDTH
		    - ICON_IMAGE_TOP_PAD
		    - ICON_INTERNAL_SHADOW_WIDTH
		    - ICON_INTERNAL_SHADOW_WIDTH
		    - ICON_IMAGE_BOTTOM_PAD
		    - WM_BOTTOM_TITLE_PADDING
		    - ICON_EXTERNAL_SHADOW_WIDTH;
	    break;

    }

    
} /* END OF FUNCTION InitIconSize */


/*************************************<->*************************************
 *
 *  ShowActiveIcon (pcd)
 *
 *
 *  Description:
 *  -----------
 *  Paint the icon to indicate an "active" state
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ShowActiveIcon (pcd)
    ClientData *pcd;

#else /* _NO_PROTO */
void ShowActiveIcon (ClientData *pcd)
#endif /* _NO_PROTO */
{
    unsigned long attr_mask;
    XSetWindowAttributes window_attribs;

    if (ICON_FRAME_WIN(pcd))
    {
	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (ICON_APPEARANCE(pcd).activeBackgroundPixmap)
	{
	    attr_mask = CWBackPixmap;
	    window_attribs.background_pixmap = 
				ICON_APPEARANCE(pcd).activeBackgroundPixmap;
	}
	else
	{
	    attr_mask = CWBackPixel;
	    window_attribs.background_pixel = 
				ICON_APPEARANCE(pcd).activeBackground;
	}
	

	/* set active window attributes */
	XChangeWindowAttributes (DISPLAY, 
				ICON_FRAME_WIN(pcd), 
				attr_mask, &window_attribs);

	/* clear the frame to the right background */
	if ((!ACTIVE_PSD->useIconBox) || 
	    (P_ICON_BOX(pcd) == NULL))
	{
#ifndef MOTIF_ONE_DOT_ONE
	    _XmClearBorder (DISPLAY, ICON_FRAME_WIN(pcd),
			    0, 0,
			    ICON_WIDTH(pcd), ICON_IMAGE_HEIGHT(pcd), 4);

	    XClearArea (DISPLAY, 
			ICON_FRAME_WIN(pcd), 
			0, ICON_IMAGE_HEIGHT(pcd),
			(unsigned int) ICON_WIDTH(pcd), 
			(unsigned int) ICON_HEIGHT(pcd), False);
#else
	    XClearWindow (DISPLAY, ICON_FRAME_WIN(pcd));
#endif
	}
	else
	{
	    /*
	     * clear only area of real frame, not highlight area
	     */

	    XClearArea (DISPLAY, 
			    ICON_FRAME_WIN(pcd), 
			    IB_MARGIN_WIDTH,
			    IB_MARGIN_HEIGHT,
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_HEIGHT(pcd), False);
	}


	/* 
	 * Put up a big icon text label.
	 */

	if (ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART)
	    
	{
	    if (wmGD.activeIconTextDisplayed)
		PaintActiveIconText(pcd, True);
	    else
		ShowActiveIconText(pcd);
	}

	/* simulate exposure of window */
#ifndef MOTIF_ONE_DOT_ONE
	IconExposureProc(pcd, False);
#else
	IconExposureProc(pcd, True);
#endif

    }

} /* END OF FUNCTION ShowActiveIcon */



/*************************************<->*************************************
 *
 *  ShowInactiveIcon (pcd, refresh)
 *
 *
 *  Description:
 *  -----------
 *  Make the icon appear "inactive"
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *
 *  refresh	- if True redraw the icon
 *
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ShowInactiveIcon (pcd, refresh)
    ClientData *pcd;
    Boolean refresh;

#else /* _NO_PROTO */
void ShowInactiveIcon (ClientData *pcd, Boolean refresh)
#endif /* _NO_PROTO */
{
    unsigned long attr_mask = 0;
    XSetWindowAttributes window_attribs;

    /* turn off the active icon text */
    if (ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART)
    {
	/* pass in screen to fix multiscreen bug [P3385] */
	HideActiveIconText(pcd->pSD);
    }
   
    if (ICON_FRAME_WIN(pcd))
    {
	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (ICON_APPEARANCE(pcd).backgroundPixmap)
	{
	    attr_mask |= CWBackPixmap;
	    window_attribs.background_pixmap = 
				ICON_APPEARANCE(pcd).backgroundPixmap;
	}
	else
	{
	    attr_mask |= CWBackPixel;
	    window_attribs.background_pixel = 
				ICON_APPEARANCE(pcd).background;
	}
	

	/* set active window attributes */
	XChangeWindowAttributes (DISPLAY, ICON_FRAME_WIN(pcd), attr_mask, 
				 &window_attribs);


	if (refresh)
	{
	    /* clear the frame to the right background */
	    if ((!ACTIVE_PSD->useIconBox) || 
	        (P_ICON_BOX(pcd) == NULL))
	    {
#ifndef MOTIF_ONE_DOT_ONE
		_XmClearBorder (DISPLAY, ICON_FRAME_WIN(pcd),
				0, 0,
				ICON_WIDTH(pcd), ICON_IMAGE_HEIGHT(pcd), 4);

		XClearArea (DISPLAY, 
			    ICON_FRAME_WIN(pcd), 
			    0, ICON_IMAGE_HEIGHT(pcd),
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_HEIGHT(pcd), False);
#else
		XClearWindow (DISPLAY, ICON_FRAME_WIN(pcd));
#endif
	    }
	    else
	    {
	        /*
	         * clear only area of real frame, not highlight area
	         */

	        XClearArea (DISPLAY, 
			    ICON_FRAME_WIN(pcd), 
	    	    	    IB_MARGIN_WIDTH,
			    IB_MARGIN_HEIGHT,
			    (unsigned int) ICON_WIDTH(pcd), 
			    (unsigned int) ICON_HEIGHT(pcd), False);
	    }
	

        /* simulate exposure of window */
#ifndef MOTIF_ONE_DOT_ONE
	    IconExposureProc(pcd, False);
#else
	    IconExposureProc(pcd, True);
#endif
	}

    }

} /* END OF FUNTION ShowInactiveIcon  */



/*************************************<->*************************************
 *
 *  ReparentIconWindow (pcd, xOffset, yOffset)
 *
 *
 *  Description:
 *  -----------
 *  Reparent the icon window in the center of the image area
 *
 *
 *  Inputs:
 *  ------
 *  pcd 	- pointer to client data
 *  xOffset     - adjusts for icons in the iconBox
 *  yOffset     - adjusts for icons in the iconBox
 *
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ReparentIconWindow(pcd, xOffset, yOffset)
    ClientData *pcd;
    int xOffset;
    int yOffset;

#else /* _NO_PROTO */
void ReparentIconWindow (ClientData *pcd, int xOffset, int yOffset)
#endif /* _NO_PROTO */
{
    int x, y, rpX, rpY;
    unsigned int width, height, bw, depth;
    Window root;
    XWindowChanges windowChanges;
    unsigned int mask;

    /*
     * Check if window size is too big
     */
    XGetGeometry (DISPLAY, pcd->iconWindow, &root, &x, &y, &width, &height, 
		  &bw, &depth);

    /*
     * strip off previous window border and set window geometry to 
     * fit inside icon frame
     */
    if (width != 0) {
	mask = CWBorderWidth;
	windowChanges.border_width = 0;
    }
    else
    {
	mask = 0;
    }

    if (width > ICON_IMAGE_MAXIMUM(pcd).width) {
	width = windowChanges.width = ICON_IMAGE_MAXIMUM(pcd).width;
	mask |= CWWidth;
    }
    else if (width < ICON_IMAGE_MINIMUM(pcd).width) {
	width = windowChanges.width = ICON_IMAGE_MINIMUM(pcd).width;
	mask |= CWWidth;
    }

    if (height > ICON_IMAGE_MAXIMUM(pcd).height) {
	height = windowChanges.height = ICON_IMAGE_MAXIMUM(pcd).height;
	mask |= CWHeight;
    }
    else if (height < ICON_IMAGE_MINIMUM(pcd).height) {
	height = windowChanges.height = ICON_IMAGE_MINIMUM(pcd).height;
	mask |= CWHeight;
    }

    if (mask)
	XConfigureWindow (DISPLAY, pcd->iconWindow, mask, &windowChanges);

    /*
     * Reparent the icon window to the center of the icon image frame
     */

    if (ICON_DECORATION(pcd) & ICON_LABEL_PART)
    {
        yOffset += ICON_INTERNAL_SHADOW_WIDTH;
    }

    rpX = ((ICON_WIDTH(pcd) - width)/2)    + xOffset;
    rpY = ((ICON_IMAGE_HEIGHT(pcd) - height)/2) + yOffset;




    XReparentWindow (DISPLAY, pcd->iconWindow, ICON_FRAME_WIN(pcd), rpX, rpY);
    pcd->clientFlags  |= ICON_REPARENTED;

    /*
     * Map the icon window when the icon frame is mapped.
     */
} /* END OF FUNCTION ReparentIconWindow */


/*************************************<->*************************************
 *
 *  PutBoxOnScreen (screen, px, py, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Changes the position of the passed box so that it is all on screen
 *
 *
 *  Inputs:
 *  ------
 *  screen 	- screen we're talking about
 *  px		- pointer to x-coord
 *  py		- pointer to y-coord
 *  width	- width of box
 *  height	- height of box
 * 
 *  Outputs:
 *  -------
 *  *px		- new x-coord
 *  *py		- new y-coord
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void PutBoxOnScreen (screen, px, py, width, height)

    int screen, *px, *py; 
    unsigned int width, height;
#else /* _NO_PROTO */
void PutBoxOnScreen (int screen, int *px, int *py, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    /*
     * Place active label text nicely on screen
     */

    if (*px+width+1 > DisplayWidth (DISPLAY, screen))
	*px -= (*px+width+1) - DisplayWidth (DISPLAY, screen);

    if (*py+height+1 > DisplayHeight (DISPLAY, screen))
	*py -= (*py+height+1) - DisplayHeight (DISPLAY, screen);

    if (*px < 1) *px = 1;

    if (*py < 1) *py = 1;

} /* END OF FUNCTION PutBoxOnScreen */


/*************************************<->*************************************
 *
 *  PutBoxInIconBox (pCD, px, py, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Changes the position of the passed box so that it is not
 *  clipped by the bulletin board
 *  
 *
 *
 *  Inputs:
 *  ------
 *  pCD		- pointer to client data
 *  px		- pointer to x-coord
 *  py		- pointer to y-coord
 *  width	- width of box
 *  height	- height of box
 * 
 *  Outputs:
 *  -------
 *  *px		- new x-coord
 *  *py		- new y-coord
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void PutBoxInIconBox (pCD, px, py, width, height)
    ClientData *pCD;
    int *px, *py; 
    unsigned int *width, *height;
#else /* _NO_PROTO */
void PutBoxInIconBox (ClientData *pCD, int *px, int *py, unsigned int *width, unsigned int *height)
#endif /* _NO_PROTO */
{

    int i;
    Arg getArgs[3];
    Dimension bBoardWidth;
    Dimension bBoardHeight;

    int clipWidth;
    int clipHeight;


    i=0;
    XtSetArg (getArgs[i], XmNwidth, (XtArgVal) &bBoardWidth ); i++;
    XtSetArg (getArgs[i], XmNheight, (XtArgVal) &bBoardHeight ); i++;
    XtGetValues (P_ICON_BOX(pCD)->bBoardWidget, getArgs, i);

    clipWidth = (int) bBoardWidth;
    clipHeight = (int) bBoardHeight;

    if (*px + *width-1 > clipWidth)
	*px -= (*px + *width-1) - clipWidth;

    if (*py + *height-1 > clipHeight)
	*py -= (*py + *height-1) - clipHeight;

    if (*px < 0) *px = 0;

    if (*py < 0) *py = 0;


} /* END OF FUNCTION PutBoxInIconBox */


/*************************************<->*************************************
 *
 *  CreateActiveIconTextWindow (pSD)
 *
 *
 *  Description:
 *  -----------
 *  creates the window that's popped up when an icon is activated
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void CreateActiveIconTextWindow (pSD)

    WmScreenData *pSD;

#else /* _NO_PROTO */
void CreateActiveIconTextWindow (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    XSetWindowAttributes window_attribs;
    unsigned long attr_mask;

    /* create active icon text window */
    attr_mask = CWEventMask| CWCursor;
    window_attribs.event_mask =  ExposureMask;
    window_attribs.cursor = wmGD.workspaceCursor;

    /* 
     * Use background pixmap if one is specified, otherwise set the
     * appropriate background color. 
     */

    if (pSD->iconAppearance.activeBackgroundPixmap)
    {
	attr_mask |= CWBackPixmap;
	window_attribs.background_pixmap = 
				pSD->iconAppearance.activeBackgroundPixmap;
    }
    else
    {
	attr_mask |= CWBackPixel;
	window_attribs.background_pixel = 
				pSD->iconAppearance.activeBackground;
    }
	

    pSD->activeIconTextWin = XCreateWindow (DISPLAY,
				       pSD->rootWindow,	/* parent */
				       0, 0,		/* x, y */
				       1, 1,		/* width, height */
				       0,		/* border width */
				       CopyFromParent,	/* depth */
				       InputOutput,	/* class */
				       CopyFromParent,	/* visual */
				       attr_mask,
				       &window_attribs);

    
    pSD->activeLabelParent = pSD->rootWindow;

} /* END OF FUNCTION CreateActiveIconTextWindow */



/*************************************<->*************************************
 *
 *  PaintActiveIconText (pcd, erase)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 *  pcd		- pointer to client data
 *  erase	- if true, then erase the area before repainting
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void PaintActiveIconText (pcd, erase)

    ClientData *pcd;
    Boolean erase;
#else /* _NO_PROTO */
void PaintActiveIconText (ClientData *pcd, Boolean erase)
#endif /* _NO_PROTO */
{
    XRectangle textBox;
    GC iconGC, topGC, botGC;

    if (!(ICON_DECORATION(pcd) & ICON_ACTIVE_LABEL_PART))
	return;

    /* get appropriate GCs */
    iconGC = ICON_APPEARANCE(pcd).activeGC;
    topGC = ICON_APPEARANCE(pcd).activeTopShadowGC;
    botGC = ICON_APPEARANCE(pcd).activeBottomShadowGC;

    /* draw shadowing */

    if (pActiveIconTopRects) {
	XFillRectangles (DISPLAY, 
			 pcd->pSD->activeIconTextWin,
			 topGC,
			 pActiveIconTopRects->prect,
			 pActiveIconTopRects->used);
    }

    if (pActiveIconBotRects) {
	XFillRectangles (DISPLAY,
			 pcd->pSD->activeIconTextWin,
			 botGC,
			 pActiveIconBotRects->prect,
			 pActiveIconBotRects->used);
    }

    /* paint the text */
    textBox.x = ICON_EXTERNAL_SHADOW_WIDTH;
    textBox.y = ICON_EXTERNAL_SHADOW_WIDTH;
    textBox.width = activeIconTextWidth - 2*ICON_EXTERNAL_SHADOW_WIDTH;
    textBox.height = activeIconTextHeight - 2*ICON_EXTERNAL_SHADOW_WIDTH;

    if (erase)
    {
	XClearArea (DISPLAY, pcd->pSD->activeIconTextWin, textBox.x, textBox.y,
		    (unsigned int) textBox.width, 
		    (unsigned int) textBox.height, 
		    FALSE);
    }

    WmDrawXmString(DISPLAY, pcd->pSD->activeIconTextWin, 
		   ICON_APPEARANCE(pcd).fontList,
		   pcd->iconTitle, iconGC, 
		   textBox.x, textBox.y, textBox.width, &textBox);


} /* END OF FUNCTION PaintActiveIconText */


/*************************************<->*************************************
 *
 *  ShowActiveIconText (pcd)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ShowActiveIconText (pcd)
    ClientData *pcd;
#else /* _NO_PROTO */
void ShowActiveIconText (ClientData *pcd)
#endif /* _NO_PROTO */
{
    XWindowAttributes iconFrameAttribs;
    XSetWindowAttributes window_attribs;
    XWindowChanges windowChanges;
    unsigned int mask;
    int x, y; 
    unsigned int junk;
    Window root;
    Dimension dWidth, dHeight;


    /* 
     * put up a big icon text label
     */
    if (pcd->pSD->activeIconTextWin) {
	/* copy event mask from icon frame window */
	XGetWindowAttributes (DISPLAY, ICON_FRAME_WIN(pcd), &iconFrameAttribs);

	/* set attributes of window */
	window_attribs.event_mask =  iconFrameAttribs.your_event_mask;
	XChangeWindowAttributes (DISPLAY, pcd->pSD->activeIconTextWin,
				 CWEventMask, &window_attribs);

	/* set up geometry for the window */

	XmStringExtent (ICON_APPEARANCE(pcd).fontList, pcd->iconTitle,
			&dWidth, &dHeight);

	activeIconTextHeight =  (unsigned int) dHeight + 
	          WM_BOTTOM_TITLE_PADDING +
		  2*ICON_EXTERNAL_SHADOW_WIDTH;

	activeIconTextWidth = (unsigned int) dWidth;

	if (activeIconTextWidth < (1.2 * ICON_WIDTH(pcd))) 
	{
	    activeIconTextWidth = 1.2 * ICON_WIDTH(pcd);
	}

	activeIconTextWidth += 2*ICON_EXTERNAL_SHADOW_WIDTH;

	XGetGeometry (DISPLAY, 
			(Drawable) ICON_FRAME_WIN(pcd), 
			&root, &x, &y, 
		        &junk, &junk, &junk, &junk);


	y += ICON_IMAGE_HEIGHT(pcd);
	x -= (activeIconTextWidth - ICON_WIDTH(pcd))/2;



	if (!(P_ICON_BOX(pcd)))
	{
	    /* 
	     * This is a normal icon
	     */
	    PutBoxOnScreen (SCREEN_FOR_CLIENT(pcd), &x, &y, 
		    activeIconTextWidth, activeIconTextHeight);
	    if (ACTIVE_LABEL_PARENT(pcd) != root)
	    {
		XReparentWindow(DISPLAY, pcd->pSD->activeIconTextWin , 
				root, x, y );
		ACTIVE_LABEL_PARENT(pcd) = root;
	    }
	    
	}
	else
	{
	    /* 
	     * This is an icon in an icon box
	     */
	    x = x + IB_MARGIN_WIDTH;
	    y = y + IB_MARGIN_HEIGHT;

	    if(!(pcd->pSD->iconDecoration & ( ICON_LABEL_PART)))
	    {
		y -= activeIconTextHeight;
	    }

	    PutBoxInIconBox (pcd, &x, &y, 
				&activeIconTextWidth, &activeIconTextHeight);
	    if (ACTIVE_LABEL_PARENT(pcd) != pcd->client)
	    {
		XReparentWindow(DISPLAY, pcd->pSD->activeIconTextWin , 
		    XtWindow(P_ICON_BOX(pcd)->bBoardWidget),
		    x, y );
		ACTIVE_LABEL_PARENT(pcd) = pcd->client;
	    }
	}


	mask = CWX | CWY | CWWidth | CWHeight; 
	windowChanges.x = x;
	windowChanges.y = y;
	windowChanges.width = activeIconTextWidth;
	windowChanges.height = activeIconTextHeight;
	XConfigureWindow (DISPLAY, pcd->pSD->activeIconTextWin, mask, 
			  &windowChanges);

	/* bevel the rectangle around the edges */
	if ((pActiveIconTopRects && pActiveIconBotRects) || 
	    (pActiveIconTopRects = 
		AllocateRList((unsigned)4*ICON_EXTERNAL_SHADOW_WIDTH)) &&
	    (pActiveIconBotRects = 
		AllocateRList((unsigned)4*ICON_EXTERNAL_SHADOW_WIDTH)))
	{
	    pActiveIconTopRects->used = 0;
	    pActiveIconBotRects->used = 0;
	    BevelRectangle (pActiveIconTopRects, 	
			    pActiveIconBotRects, 
			    0, 0, 
			    activeIconTextWidth, 
			    activeIconTextHeight,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH,
			    ICON_EXTERNAL_SHADOW_WIDTH);
	}

	XMapRaised (DISPLAY, pcd->pSD->activeIconTextWin);
	wmGD.activeIconTextDisplayed = True;

	/* save context for this window */
	XSaveContext (DISPLAY, pcd->pSD->activeIconTextWin, 
	    wmGD.windowContextType, (caddr_t) pcd);
    }
} /* END OF FUNCTION ShowActiveIconText */


/*************************************<->*************************************
 *
 *  HideActiveIconText ()
 *
 *
 *  Description:
 *  -----------
 *  Hides the big label shown over the active icon.
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void HideActiveIconText (pSD)
    WmScreenData *pSD;
#else /* _NO_PROTO */
void HideActiveIconText (WmScreenData *pSD)
#endif /* _NO_PROTO */
{


    if ((pSD && pSD->activeIconTextWin) || ACTIVE_ICON_TEXT_WIN)
    {
	/* disassociate the big label window with this client */
	XDeleteContext (DISPLAY, 
			pSD 
			 ? pSD->activeIconTextWin
			 : ACTIVE_PSD->activeIconTextWin, 
			wmGD.windowContextType);

	/* hide the big label */
	XUnmapWindow (DISPLAY,
		      pSD 
		       ? pSD->activeIconTextWin
		       : ACTIVE_PSD->activeIconTextWin);
	wmGD.activeIconTextDisplayed = False;
    }
}


/*************************************<->*************************************
 *
 *  MoveActiveIconText (pcd)
 *
 *
 *  Description:
 *  -----------
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void MoveActiveIconText (pcd)
    ClientData *pcd;
#else /* _NO_PROTO */
void MoveActiveIconText (ClientData *pcd)
#endif /* _NO_PROTO */
{
    int x, y; 
    unsigned int junk;
    Window root;
    Dimension dWidth, dHeight;
    
    
    /* 
     * put up a big icon text label
     */
    if (pcd->pSD->activeIconTextWin && wmGD.activeIconTextDisplayed) {
	/* set up geometry for the window */

	XmStringExtent (ICON_APPEARANCE(pcd).fontList, pcd->iconTitle,
			&dWidth, &dHeight);

	activeIconTextHeight =  (unsigned int) dHeight +
	          WM_BOTTOM_TITLE_PADDING +
		  2 * ICON_EXTERNAL_SHADOW_WIDTH;
	
	activeIconTextWidth = (unsigned int) dWidth;

	if (activeIconTextWidth < (1.2 * ICON_WIDTH(pcd))) 
	{
	    activeIconTextWidth = 1.2 * ICON_WIDTH(pcd);
	}
	
	activeIconTextWidth += 2 * ICON_EXTERNAL_SHADOW_WIDTH;

	XGetGeometry (DISPLAY, 
			(Drawable) ICON_FRAME_WIN(pcd), 
			&root, &x, &y, 
		        &junk, &junk, &junk, &junk);

	
	y += ICON_IMAGE_HEIGHT(pcd);
        x -= (activeIconTextWidth - ICON_WIDTH(pcd))/2;

	if (!(P_ICON_BOX(pcd)))
	{
	    /* This is a normal icon */
	    PutBoxOnScreen (SCREEN_FOR_CLIENT(pcd), &x, &y, 
		activeIconTextWidth, activeIconTextHeight);
	}
	else 
	{
	    /* icon box */
	    x = x + IB_MARGIN_WIDTH;
            y = y + IB_MARGIN_HEIGHT;

	    if(!(pcd->pSD->iconDecoration & ( ICON_LABEL_PART)))
	    {
		y -= activeIconTextHeight;
	    }

	    PutBoxInIconBox (pcd, &x, &y, 
			     &activeIconTextWidth, &activeIconTextHeight);
	}
	
	XMoveWindow(DISPLAY, pcd->pSD->activeIconTextWin, x, y );

    }
}  /* END OF FUNCTION  MoveActiveIconText */


