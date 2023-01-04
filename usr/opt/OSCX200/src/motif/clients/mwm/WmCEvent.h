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
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmCEvent.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:19:03 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
void		CheckButtonPressBuiltin ();
void		CheckButtonReleaseBuiltin ();
Window		GetParentWindow ();
WmScreenData   *GetScreenForWindow ();
Boolean		HandleCButtonPress ();
void		HandleCButtonRelease ();
void		HandleCColormapNotify ();
void		HandleCConfigureRequest ();
void		HandleCEnterNotify ();
void		HandleCLeaveNotify ();
Boolean		HandleCFocusIn ();
Boolean		HandleCFocusOut ();
Boolean		HandleCKeyPress ();
void		HandleClientMessage ();
void		HandleCMotionNotify ();
void		HandleCPropertyNotify ();
#ifndef NO_SHAPE
void            HandleCShapeNotify();
#endif /* NO_SHAPE */
Boolean		HandleEventsOnClientWindow ();
Boolean		HandleEventsOnSpecialWindows ();
void		HandleIconBoxButtonPress ();
void		HandleIconButtonPress ();
void		ProcessButtonGrabOnClient ();
void		SetupCButtonBindings ();
void		DetermineActiveScreen ();
Boolean		WmDispatchClientEvent ();

#else /* _NO_PROTO */

extern void CheckButtonPressBuiltin (XButtonEvent *buttonEvent, 
				     Context context, Context subContext, 
				     int partContext, ClientData *pCD);
extern void CheckButtonReleaseBuiltin (XButtonEvent *buttonEvent, 
				       Context context, Context subContext, 
				       ClientData *pCD);
extern Window GetParentWindow (Window window);
extern WmScreenData *GetScreenForWindow (Window win);
extern Boolean HandleCButtonPress (ClientData *pCD, XButtonEvent *buttonEvent);
extern void HandleCButtonRelease (ClientData *pCD, XButtonEvent *buttonEvent);
extern void HandleCColormapNotify (ClientData *pCD, 
				   XColormapEvent *colorEvent);
extern void HandleCConfigureRequest (ClientData *pCD, 
				     XConfigureRequestEvent *configureRequest);
extern void HandleCEnterNotify (ClientData *pCD, 
				XEnterWindowEvent *enterEvent);
extern void HandleCLeaveNotify (ClientData *pCD, 
				XLeaveWindowEvent *leaveEvent);
extern Boolean HandleCFocusIn (ClientData *pCD, 
			       XFocusChangeEvent *focusChangeEvent);
extern Boolean HandleCFocusOut (ClientData *pCD, 
				XFocusChangeEvent *focusChangeEvent);
extern Boolean HandleCKeyPress (ClientData *pCD, XKeyEvent *keyEvent);
extern void HandleClientMessage (ClientData *pCD, 
				 XClientMessageEvent *clientEvent);
extern void HandleCMotionNotify (ClientData *pCD, XMotionEvent *motionEvent);
extern void HandleCPropertyNotify (ClientData *pCD, 
				   XPropertyEvent *propertyEvent);
#ifndef NO_SHAPE
extern void HandleCShapeNotify (ClientData *pCD,  XShapeEvent *shapeEvent);
#endif /* NO_SHAPE */
extern Boolean HandleEventsOnClientWindow (ClientData *pCD, XEvent *pEvent);
extern Boolean HandleEventsOnSpecialWindows (XEvent *pEvent);


extern void SetupCButtonBindings (Window window, ButtonSpec *buttonSpecs);
extern Boolean WmDispatchClientEvent (XEvent *event);
extern void HandleIconBoxButtonPress (ClientData *pCD, 
				      XButtonEvent *buttonEvent, 
				      Context subContext);
extern void HandleIconButtonPress (ClientData *pCD, XButtonEvent *buttonEvent);


extern void ProcessButtonGrabOnClient (ClientData *pCD, 
				       XButtonEvent *buttonEvent, 
				       Boolean replayEvent);
extern void SetupCButtonBindings (Window window, ButtonSpec *buttonSpecs);
extern void DetermineActiveScreen (XEvent *pEvent);
extern Boolean WmDispatchClientEvent (XEvent *event);
#endif /* _NO_PROTO */

#define SetActiveScreen(psd) (ACTIVE_PSD=(psd), wmGD.queryScreen=False)
