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
/* (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmFeedback.c,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:56:20 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */
#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmBitmap.h"
#include <Xm/Xm.h>
#include <X11/Shell.h>
#include <Xm/Label.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/MessageB.h>
          
#ifdef DEC_MOTIF_BUG_FIX
#include <stdio.h>
#endif
                      
#ifdef DEC_MOTIF_EXTENSION
/* For DEC watch cursor */
#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#include <X11/cursorfont.h>
#include <Mrm/MrmPublic.h>
#include "mwm_internal.h" 
#include "mwm_dialog.h" 
#endif /* DEC_MOTIF_EXTENSION */
#ifdef VMS
#include "WmVms.h"
#endif
#define MOVE_OUTLINE_WIDTH	2
#define FEEDBACK_BEVEL		2

#define DEFAULT_POSITION_STRING	"(0000x0000)"

#define  CB_HIGHLIGHT_THICKNESS  3

/*
 * include extern functions
 */
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmGraphics.h"
#include "WmManage.h"
#include "WmColormap.h"
#include "stdio.h"


/*
 * Global Variables:
 */
#ifdef DEC_MOTIF_EXTENSION
/* See wmglobal.h. The cursor is used in other modules */
#else
static Cursor  waitCursor = (Cursor)0L;
#endif

/* see WmGlobal.h for index defines: */

#ifdef DEC_MOTIF_EXTENSION
typedef struct
  {
    XmString str;
    char name[ 32 ];
  } uil_text_type;
static uil_text_type confirm_mess[ 4 ] = { NULL, "k_mwm_toggle_def_env_text",
                                           NULL, "k_mwm_toggle_cust_env_text",
                                           NULL, "k_mwm_restart_text",
                                           NULL, "k_mwm_exit_text" };
/* Backup strings */
static char *confirm_mesg[4] = {"Toggle the window manager to\n\
the Motif default environment ?",
                                "Toggle the window manager to\n\
your customized environment ?",
                                "Restart the window manager ?",
                                "Exit the window manager ?"};
#else
static char *confirm_mesg[4] = {"Toggle to Default Behavior?",
				"Toggle to Custom Behavior?",
                                "Restart Mwm?",
                                "QUIT Mwm?"};
#endif /* DEC_MOTIF_EXTENSION */

static char *confirm_widget[4] = {"confirmDefaultBehavior",
				  "confirmCustomBehavior",
				  "confirmRestart",
				  "confirmQuit"};


#ifdef _NO_PROTO
typedef void (*ConfirmFunc)();
#else
typedef void (*ConfirmFunc)(Boolean);
#endif
static ConfirmFunc confirm_func[4] = {Do_Set_Behavior,
				      Do_Set_Behavior,
				      Do_Restart,
				      Do_Quit_Mwm};


/*************************************<->*************************************
 *
 *  ShowFeedbackWindow(pSD, x, y, width, height, style)
 *
 *
 *  Description:
 *  -----------
 *  Pop up the window for moving and sizing feedback
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  x		- initial x-value
 *  y		- initial y-value
 *  width 	- initial width value
 *  height	- initial height value
 *  style	- show size, position, or both
 *  
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void ShowFeedbackWindow (pSD, x, y, width, height, style)
    
    WmScreenData *pSD;
    int           x, y; 
    unsigned int  width, height;
    unsigned long style;
#else /* _NO_PROTO */
void ShowFeedbackWindow (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height, unsigned long style)
#endif /* _NO_PROTO */
{
    unsigned long        mask = 0;
    XSetWindowAttributes win_attribs;
    XWindowChanges       win_changes;
    int                  direction, ascent, descent;
    XCharStruct          xcsLocation;
    int                  winX, winY;
    int                  tmpX, tmpY;

    if ( (pSD->fbStyle = style) == FB_OFF)
	return;

    pSD->fbLastX = x;
    pSD->fbLastY = y;
    pSD->fbLastWidth = width;
    pSD->fbLastHeight = height;

    /*
     * Derive the size and position of the window from the text extents
     * Set starting position of each string 
     */
    XTextExtents(pSD->feedbackAppearance.font, DEFAULT_POSITION_STRING, 
		 strlen(DEFAULT_POSITION_STRING), &direction, &ascent, 
		 &descent, &xcsLocation);
    
    pSD->fbWinWidth = xcsLocation.width + 4*FEEDBACK_BEVEL;

    switch (pSD->fbStyle) 
    {
	case FB_SIZE:
	    pSD->fbSizeY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbWinHeight = (ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;

	case FB_POSITION:
	    pSD->fbLocY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbWinHeight = (ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;

	default:
	case (FB_SIZE | FB_POSITION):
	    pSD->fbLocY = 2*FEEDBACK_BEVEL + ascent;
	    pSD->fbSizeY = pSD->fbLocY + ascent + descent;
	    pSD->fbWinHeight = 2*(ascent + descent) + 4*FEEDBACK_BEVEL;
	    break;
    }

    if (pSD->feedbackGeometry) /* set by user */
    {
	unsigned int junkWidth, junkHeight;

	mask = XParseGeometry(pSD->feedbackGeometry, &tmpX, &tmpY,
			      &junkWidth, &junkHeight);
    }

    if (mask & (XValue|YValue))
    {
	winX = (mask & XNegative) ? 
	    DisplayWidth(DISPLAY, pSD->screen)  + tmpX - pSD->fbWinWidth : tmpX;
	winY = (mask & YNegative) ? 
	    DisplayHeight(DISPLAY, pSD->screen) + tmpY -pSD->fbWinHeight : tmpY;
    }
    else
    {
	winX = (DisplayWidth(DISPLAY, pSD->screen) - pSD->fbWinWidth)/2;
	winY = (DisplayHeight(DISPLAY, pSD->screen) -pSD->fbWinHeight)/2;
    }

    /* 
     * Put new text into the feedback strings
     */
    UpdateFeedbackText (pSD, x, y, width, height);

    /*
     * bevel the window border for a 3-D look
     */
    if ( (pSD->fbTop && pSD->fbBottom) ||
	 ((pSD->fbTop = AllocateRList((unsigned)2*FEEDBACK_BEVEL)) &&
	  (pSD->fbBottom = AllocateRList((unsigned)2*FEEDBACK_BEVEL))) )
    {
	pSD->fbTop->used = 0;
	pSD->fbBottom->used = 0;
	BevelRectangle (pSD->fbTop,
			pSD->fbBottom,
			0, 0, 
			pSD->fbWinWidth, pSD->fbWinHeight,
			FEEDBACK_BEVEL, FEEDBACK_BEVEL,
			FEEDBACK_BEVEL, FEEDBACK_BEVEL);
    }

    /*
     * Create window if not yet created, otherwise fix size and position
     */

    if (!pSD->feedbackWin)
    {

	/*
	 * Create the window
	 */

	mask = CWEventMask | CWOverrideRedirect | CWSaveUnder;
	win_attribs.event_mask = ExposureMask;
	win_attribs.override_redirect = TRUE;
	win_attribs.save_under = TRUE;

	/* 
	 * Use background pixmap if one is specified, otherwise set the
	 * appropriate background color. 
	 */

	if (pSD->feedbackAppearance.backgroundPixmap)
	{
	    mask |= CWBackPixmap;
	    win_attribs.background_pixmap =
				pSD->feedbackAppearance.backgroundPixmap;
	}
	else
	{
	    mask |= CWBackPixel;
	    win_attribs.background_pixel =
				pSD->feedbackAppearance.background;
	}

	pSD->feedbackWin = XCreateWindow (DISPLAY, pSD->rootWindow, 
					  winX, winY,
					  pSD->fbWinWidth, 
					  pSD->fbWinHeight,
					  0, CopyFromParent, 
					  InputOutput, CopyFromParent, 
					  mask, &win_attribs);
    }
    else
    {
	win_changes.x = winX;
	win_changes.y = winY;
	win_changes.width = pSD->fbWinWidth;
	win_changes.height = pSD->fbWinHeight;
	win_changes.stack_mode = Above;

	mask = CWX | CWY | CWWidth | CWHeight | CWStackMode;

	XConfigureWindow(DISPLAY, pSD->feedbackWin, (unsigned int) mask, 
	    &win_changes);
    }


    /*
     * Make the feedback window visible (map it)
     */

    if (pSD->feedbackWin)
    {
	/* Make sure the feedback window doesn't get buried */
	XRaiseWindow(DISPLAY, pSD->feedbackWin);
	XMapWindow (DISPLAY, pSD->feedbackWin);
	PaintFeedbackWindow(pSD);
    }

} /* END OF FUNCTION ShowFeedbackWindow */



/*************************************<->*************************************
 *
 *  PaintFeedbackWindow(pSD)
 *
 *
 *  Description:
 *  -----------
 *  Repaints the feedback window in response to exposure events
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 * 
 *  Outputs:
 *  -------
 *
 *
 *  Comments:
 *  --------
 *************************************<->***********************************/
#ifdef _NO_PROTO
void PaintFeedbackWindow (pSD)

    WmScreenData *pSD;
#else /* _NO_PROTO */
void PaintFeedbackWindow (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    if (pSD->feedbackWin)
    {
	/* 
	 * draw beveling 
	 */
	if (pSD->fbTop->used > 0) 
	{
	    XFillRectangles (DISPLAY, pSD->feedbackWin, 
			     pSD->feedbackAppearance.inactiveTopShadowGC,
			     pSD->fbTop->prect, pSD->fbTop->used);
	}
	if (pSD->fbBottom->used > 0) 
	{
	    XFillRectangles (DISPLAY, pSD->feedbackWin, 
			     pSD->feedbackAppearance.inactiveBottomShadowGC,
			     pSD->fbBottom->prect, 
			     pSD->fbBottom->used);
	}

	/*
	 * clear old text 
	 */
	XClearArea (DISPLAY, pSD->feedbackWin, 
		    FEEDBACK_BEVEL, FEEDBACK_BEVEL,
		    pSD->fbWinWidth-2*FEEDBACK_BEVEL, 
		    pSD->fbWinHeight-2*FEEDBACK_BEVEL,
		    FALSE);

	/*
	 * put up new text
	 */
	if (pSD->fbStyle & FB_POSITION) 
	{
	    WmDrawString (DISPLAY, pSD->feedbackWin, 
			 pSD->feedbackAppearance.inactiveGC,
			 pSD->fbLocX, pSD->fbLocY, 
			 pSD->fbLocation, strlen(pSD->fbLocation));
	}
	if (pSD->fbStyle & FB_SIZE) 
	{
	    WmDrawString (DISPLAY, pSD->feedbackWin, 
			 pSD->feedbackAppearance.inactiveGC,
			 pSD->fbSizeX, pSD->fbSizeY, 
			 pSD->fbSize, strlen(pSD->fbSize));
	}
    }
}



/*************************************<->*************************************
 *
 *  HideFeedbackWindow (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Hide the feedback window
 *
 *
 *  Inputs:
 *  ------
 *  pDS		- pointer to screen data
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
void HideFeedbackWindow (pSD)

    WmScreenData *pSD;
#else /* _NO_PROTO */
void HideFeedbackWindow (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    if (pSD->feedbackWin)
    {
	XUnmapWindow (DISPLAY, pSD->feedbackWin);
#ifndef OLD_COLORMAP
	ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    }
    pSD->fbStyle = FB_OFF;
}




/*************************************<->*************************************
 *
 *  UpdateFeedbackInfo (pSD, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Update the information in the feedback window
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen info
 *  x		- x-value
 *  y		- y-value
 *  width 	- width value
 *  height	- height value
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
void UpdateFeedbackInfo (pSD, x, y, width, height)

    WmScreenData *pSD;
    int          x, y; 
    unsigned int width, height;
#else /* _NO_PROTO */
void UpdateFeedbackInfo (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    /*
     * Currently the feedback window must always be redrawn to (potentially)
     * repair damage done by moving the configuration outline.  The feedback
     * repainting generally only needs to be done when the information
     * changes or the feedback window is actually overwritten by the
     * configuration outline.
     */

    {
	pSD->fbLastX = x;
	pSD->fbLastY = y;
	pSD->fbLastWidth = width;
	pSD->fbLastHeight = height;

	UpdateFeedbackText (pSD, x, y, width, height);

	PaintFeedbackWindow(pSD);
    }
}




/*************************************<->*************************************
 *
 *  UpdateFeedbackText (pSD, x, y, width, height)
 *
 *
 *  Description:
 *  -----------
 *  Update the information in the feedback strings
 *
 *
 *  Inputs:
 *  ------
 *  pSD		- pointer to screen data
 *  x		- x-value
 *  y		- y-value
 *  width 	- width value
 *  height	- height value
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
void UpdateFeedbackText (pSD, x, y, width, height)

    WmScreenData *pSD;
    int          x, y; 
    unsigned int width, height;
#else /* _NO_PROTO */
void UpdateFeedbackText (WmScreenData *pSD, int x, int y, unsigned int width, unsigned int height)
#endif /* _NO_PROTO */
{
    int         direction, ascent, descent;
    XCharStruct xcs;

    if (pSD->fbStyle & FB_POSITION) 
    {
	sprintf (pSD->fbLocation, "(%4d,%-4d)", x, y);
	XTextExtents(pSD->feedbackAppearance.font, pSD->fbLocation,
		 strlen(pSD->fbLocation), &direction, &ascent, 
		 &descent, &xcs);
	pSD->fbLocX = (pSD->fbWinWidth - xcs.width)/2;
    }

    if (pSD->fbStyle & FB_SIZE) 
    {
	sprintf (pSD->fbSize,     "%4dx%-4d", width, height);
	XTextExtents(pSD->feedbackAppearance.font, pSD->fbSize,
		 strlen(pSD->fbSize), &direction, &ascent, 
		 &descent, &xcs);
	pSD->fbSizeX = (pSD->fbWinWidth - xcs.width)/2;
    }
}



/*************************************<->*************************************
 *
 *  static void
 *  OkCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  QuestionBox Ok callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void OkCB (w, client_data, call_data)

   Widget w;
   caddr_t client_data;
   caddr_t call_data;
{
    WithdrawDialog (w);

    confirm_func[((WmScreenData *)client_data)->actionNbr] (False);

    wmGD.confirmDialogMapped = False;

} /* END OF FUNCTION OkCB */


/*************************************<->*************************************
 *
 *  static void
 *  CancelCB (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  QuestionBox Cancel callback.
 *
 *
 *  Inputs:
 *  ------
 *  None.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

static void CancelCB (w, client_data, call_data)

   Widget w;
   caddr_t client_data;
   caddr_t call_data;
{
    WithdrawDialog (w);

    wmGD.confirmDialogMapped = False;

} /* END OF FUNCTION CancelCB */



/*************************************<->*************************************
 *
 *  void
 *  ConfirmAction (pSD,nbr)
 *
 *
 *  Description:
 *  -----------
 *  Post a QuestionBox and ask for confirmation.  If so, executes the
 *  appropriate action.
 *
 *
 *  Inputs:
 *  ------
 *  nbr = action number
 *  pSD->screen
 *  pSD->screenTopLevel
 *
 * 
 *  Outputs:
 *  -------
 *  actionNbr = current QuestionBox widget index.
 *  confirmW[actionNbr]  = QuestionBox widget.
 *
 *
 *  Comments:
 *  --------
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ConfirmAction (pSD,nbr)

    WmScreenData *pSD;
    int	     nbr;
#else /* _NO_PROTO */
void ConfirmAction (WmScreenData *pSD, int nbr)
#endif /* _NO_PROTO */
{
    Arg           args[8];
    register int  n;
    int           x, y;
    Dimension     width, height;
    Widget        dialogShellW = NULL;
    XmString	  messageString;
    static XmString	  defaultMessageString = NULL;

    if (pSD->confirmboxW[nbr] == NULL)
    /* First time for this one */
    {

        /* 
         * Create a dialog popup shell with explicit keyboard policy.
         */

        n = 0;
        XtSetArg(args[n], XmNx, (XtArgVal)
	         (DisplayWidth (DISPLAY, pSD->screen)/2)); n++;
        XtSetArg(args[n], XmNy, (XtArgVal)
	         (DisplayHeight (DISPLAY, pSD->screen)/2)); n++;
        XtSetArg(args[n], XtNallowShellResize, (XtArgVal) TRUE);  n++;
        XtSetArg(args[n], XtNkeyboardFocusPolicy, (XtArgVal) XmEXPLICIT);  n++;
        XtSetArg(args[n], XtNdepth, 
		(XtArgVal) DefaultDepth(DISPLAY, pSD->screen));  n++;
        XtSetArg(args[n], XtNscreen, 
		(XtArgVal) ScreenOfDisplay(DISPLAY, pSD->screen));  n++;
#ifdef DEC_MOTIF_BUG_FIX
        /* Make sure that this window is override Redirect
           and that geometry requests are not made.
           This is a fix for an alpha/osf1 problem where
           the session manager kill mwm and mwm goes
           into a infinite loop in a geometry request. */
	XtSetArg( args[n], XtNoverrideRedirect, (XtArgVal) TRUE ); n++;
	XtSetArg( args[n], XmNwaitForWm, (XtArgVal) FALSE ); n++;
#endif
        dialogShellW =
    	        XtCreatePopupShell ((String) WmNfeedback, 
				    transientShellWidgetClass,
		                    pSD->screenTopLevelW, args, n);

        /* 
         * Create a QuestionBox as a child of the popup shell.
	 * Set traversalOn and add callbacks for the OK and CANCEL buttons.
	 * Unmanage the HELP button.
         */

        n = 0;
        XtSetArg(args[n], XmNdialogType, (XtArgVal) XmDIALOG_QUESTION); n++;
        XtSetArg(args[n], XmNmessageAlignment, (XtArgVal) XmALIGNMENT_CENTER);
	   n++;
        XtSetArg(args[n], XmNtraversalOn, (XtArgVal) TRUE); n++;

	/*
	 * In 1.2 confirmbox's widget name changed from the generic
	 * WmNconfirmbox (ie. 'confirmbox') to a more descriptive name
	 * so that each confirm dialog can be customized separately (e.g.
	 * "Mwm*confirmRestart*messageString: restart it?").
	 */

        pSD->confirmboxW[nbr] = 
	    XtCreateManagedWidget (confirm_widget[nbr], xmMessageBoxWidgetClass,
                                   dialogShellW, args, n);
	
        n = 0;
        XtSetArg(args[n], XmNmessageString, &messageString); n++;
        XtGetValues(pSD->confirmboxW[nbr], (ArgList) args, n);

	if (defaultMessageString == NULL)
	{
	    defaultMessageString = XmStringCreateLtoR ("",
						      XmFONTLIST_DEFAULT_TAG);
	}

        n = 0;

	/*
	 * If the message string is the default, then put something
	 * 'reasonable' in instead.
	 */

	if (XmStringCompare( messageString, defaultMessageString ))
	{
#ifdef DEC_MOTIF_EXTENSION
            /* Get the value from UIL */
            if ( mwm_dialog_text_get( confirm_mess[ nbr ].name, 
                                      &confirm_mess[ nbr ].str ))
              {
                XtSetArg(args[n], XmNmessageString, 
                                  (XtArgVal)confirm_mess[ nbr ].str );
                n++;
              }
            /* Not found in UIL. Create multi-line strings. */
            else
              {
                XtSetArg(args[n], XmNmessageString, (XtArgVal)
                XmStringCreateLtoR( confirm_mesg[nbr],
        	    		    XmFONTLIST_DEFAULT_TAG)); n++;
              }
#else
   	    XtSetArg(args[n], XmNmessageString, (XtArgVal)
	             XmStringCreate(confirm_mesg[nbr],
	             XmFONTLIST_DEFAULT_TAG)); n++;
#endif /* DEC_MOTIF_EXTENSION */                         
	    XtSetValues(pSD->confirmboxW[nbr], (ArgList) args, n);
	}

        n = 0;
        XtSetArg (args[n], XmNtraversalOn, (XtArgVal) TRUE); n++;
        XtSetArg (args[n], XmNhighlightThickness, 
		  (XtArgVal) CB_HIGHLIGHT_THICKNESS); n++;
        XtSetValues ( XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
			    XmDIALOG_OK_BUTTON), args, n);
        XtSetValues ( XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
			    XmDIALOG_CANCEL_BUTTON), args, n);
        XtAddCallback (pSD->confirmboxW[nbr], XmNokCallback, 
		       (XtCallbackProc) OkCB, 
			    (caddr_t)pSD); 
        XtAddCallback (pSD->confirmboxW[nbr], XmNcancelCallback, 
	    (XtCallbackProc)CancelCB, (caddr_t)NULL); 

        XtUnmanageChild
	    (XmMessageBoxGetChild (pSD->confirmboxW[nbr], 
		XmDIALOG_HELP_BUTTON));

        XtRealizeWidget (dialogShellW);

        /* 
         * Center the DialogShell in the display.
         */

        n = 0;
        XtSetArg(args[n], XmNheight, &height); n++;
        XtSetArg(args[n], XmNwidth, &width); n++;
        XtGetValues (dialogShellW, (ArgList) args, n);

        x = (DisplayWidth (DISPLAY, pSD->screen) - ((int) width))/2;
        y = (DisplayHeight (DISPLAY, pSD->screen) - ((int) height))/2;
        n = 0;
        XtSetArg(args[n], XmNx, (XtArgVal) x); n++;
        XtSetArg(args[n], XmNy, (XtArgVal) y); n++;
        XtSetValues (dialogShellW, (ArgList) args, n);

        ManageWindow (pSD, XtWindow(dialogShellW), MANAGEW_CONFIRM_BOX);
    }
    else
    {
        ReManageDialog (pSD, pSD->confirmboxW[nbr]);
    }

    pSD->actionNbr = nbr;

    XFlush(DISPLAY);

    wmGD.confirmDialogMapped = True;

} /* END OF FUNCTION ConfirmAction */



/*************************************<->*************************************
 *
 *  ShowWaitState (flag)
 *
 *
 *  Description:
 *  -----------
 *  Enter/Leave the wait state.
 *
 *
 *  Inputs:
 *  ------
 *  flag = TRUE for Enter, FALSE for Leave.
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO                                     
void ShowWaitState (flag)

    Boolean  flag;
#else /* _NO_PROTO */
void ShowWaitState (Boolean flag)
#endif /* _NO_PROTO */
{
#ifdef DEC_MOTIF_EXTENSION
/* For DEC wait cursor */

/* local variables */
Font cursor_font;
XColor cursor_colors[ 2 ];
int cursor_wait;
int error_code;

/********************************/

#else

    char        *bits;
    char        *maskBits;
    unsigned int width;
    unsigned int height;
    unsigned int xHotspot;
    unsigned int yHotspot;
    Pixmap       pixmap;
    Pixmap       maskPixmap;
    XColor       xcolors[2];

#endif /* DEC_MOTIF_EXTENSION */

#ifdef DEC_MOTIF_EXTENSION
    if ( !wmGD.waitCursor )
    {
#else
    if (!waitCursor)
    {
#endif

#ifdef DEC_MOTIF_EXTENSION

    /* Get the DEC watch cursor */
    if ( cursor_font = XLoadFont( DISPLAY, "decw$cursor" ) )
      {     
        cursor_colors[0].pixel = WhitePixelOfScreen( DefaultScreenOfDisplay(
                                                     DISPLAY ));
        cursor_colors[1].pixel = BlackPixelOfScreen( DefaultScreenOfDisplay(
                                                     DISPLAY ));
        XQueryColors( DISPLAY, 
                      DefaultColormapOfScreen( DefaultScreenOfDisplay(DISPLAY)),
                      cursor_colors, 2 );
	cursor_wait = decw$c_wait_cursor;
        wmGD.waitCursor = XCreateGlyphCursor( DISPLAY, cursor_font, cursor_font, 
                                         cursor_wait, cursor_wait + 1,
                                         &cursor_colors[ 0 ],
                                         &cursor_colors[ 1 ] );
	XUnloadFont( DISPLAY, cursor_font );
      }
    /* Use the MIT cursor */
    else wmGD.waitCursor = XCreateFontCursor( DISPLAY, XC_watch );

#else         
#ifdef LARGECURSORS
	if (wmGD.useLargeCursors)
	{
	    width = time32_width;
	    height = time32_height;
	    bits = time32_bits;
	    maskBits = time32m_bits;
	    xHotspot = time32_x_hot;
	    yHotspot = time32_y_hot;
	}
	else
#endif /* LARGECURSORS */

	{
	    width = time16_width;
	    height = time16_height;
	    bits = time16_bits;
	    maskBits = time16m_bits;
	    xHotspot = time16_x_hot;
	    yHotspot = time16_y_hot;
	}

        pixmap = XCreateBitmapFromData (DISPLAY, 
		         DefaultRootWindow(DISPLAY), bits, 
			 width, height);

        maskPixmap = XCreateBitmapFromData (DISPLAY, 
		         DefaultRootWindow(DISPLAY), maskBits, 
			 width, height);

        xcolors[0].pixel = BlackPixelOfScreen(DefaultScreenOfDisplay(DISPLAY));
        xcolors[1].pixel = WhitePixelOfScreen(DefaultScreenOfDisplay(DISPLAY));

        XQueryColors (DISPLAY, 
		      DefaultColormapOfScreen(DefaultScreenOfDisplay
					      (DISPLAY)), 
		      xcolors, 2);
	waitCursor = XCreatePixmapCursor (DISPLAY, pixmap, maskPixmap
	                                  &(xcolors[0]), &(xcolors[1]),
                                          xHotspot, yHotspot);
        XFreePixmap (DISPLAY, pixmap);
        XFreePixmap (DISPLAY, maskPixmap);
#endif /* DEC_MOTIF_EXTENSIONS */
    }

    if (flag)
#ifdef DEC_MOTIF_EXTENSION
    {
	error_code = XGrabPointer( DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			           0, GrabModeAsync, GrabModeAsync, None, 
                                   wmGD.waitCursor, CurrentTime );
        if ( wmGD.debug )
          {
            fprintf( wmGD.debug_file, "Grab pointer\n" );
            fflush( wmGD.debug_file );
          }
        if ( error_code != GrabSuccess )
	{	
            if ( error_code == AlreadyGrabbed )
                /* Print error message */
                fprintf( stderr, "Mwm: Pointer is already grabbed.\n" );
            /* Print error message */
            else fprintf( stderr, "Mwm: Could not grab pointer; Error %d \n",
                          error_code );
            if ( wmGD.debug )
              {
                fprintf( wmGD.debug_file, 
                         "**** grab pointer failed %d \n", error_code );
                fflush( wmGD.debug_file );
              }
	}

	error_code = XGrabKeyboard( DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			            GrabModeAsync, GrabModeAsync, CurrentTime);
        if ( wmGD.debug )
          {
            fprintf( wmGD.debug_file, "Grab keyboard\n" );
            fflush( wmGD.debug_file );
          }
        if ( error_code != GrabSuccess )
	{	
            if ( error_code == AlreadyGrabbed )
                /* Print error message */
                fprintf( stderr, "Mwm: Keyboard is already grabbed\n" );
            /* Print error message */
            else fprintf( stderr, "Mwm: Could not grab keyboard; Error %d \n",
                          error_code );
            if ( wmGD.debug )
              {
                fprintf( wmGD.debug_file, 
                         "**** grab keyboard failed %d \n", error_code );
                fflush( wmGD.debug_file );
              }
	}
    }
#else
    {
	XGrabPointer (DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			0, GrabModeAsync, GrabModeAsync, None, 
			waitCursor, CurrentTime);
	XGrabKeyboard (DISPLAY, DefaultRootWindow(DISPLAY), FALSE, 
			GrabModeAsync, GrabModeAsync, CurrentTime);
    }
#endif /* DEC_MOTIF_EXTENSION */
    else
    {
	XUngrabPointer (DISPLAY, CurrentTime);
	XUngrabKeyboard (DISPLAY, CurrentTime);
#ifdef DEC_MOTIF_EXTENSION
        if ( wmGD.debug )
          {
            fprintf( wmGD.debug_file, "ungrab keyboard and pointer \n" );
            fflush( wmGD.debug_file );
          }
#endif /* DEC_MOTIF_EXTENSION */
    }

} /* END OF FUNCTION ShowWaitState */



/*************************************<->*************************************
 *
 *  InitCursorInfo ()
 *
 *
 *  Description:
 *  -----------
 *  This function determines whether a server supports large cursors.  It it
 *  does large feedback cursors are used in some cases (wait state and
 *  system modal state); otherwise smaller (16x16) standard cursors are used.
 *
 *  Outputs:
 *  -------
 *  wmGD.useLargeCusors = set to True if larger cursors are supported.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void InitCursorInfo ()

#else /* _NO_PROTO */
void InitCursorInfo (void)
#endif /* _NO_PROTO */
{
    unsigned int cWidth;
    unsigned int cHeight;

    wmGD.useLargeCursors = False;

    if (XQueryBestCursor (DISPLAY, DefaultRootWindow(DISPLAY), 
	32, 32, &cWidth, &cHeight))
    {
	if ((cWidth >= 32) && (cHeight >= 32))
	{
	    wmGD.useLargeCursors = True;
	}
    }

} /* END OF FUNCTION InitCursorInfo */






