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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmEvent.c,v $ $Revision: 1.1.4.4 $ $Date: 1993/10/18 17:15:59 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
/*
 * include extern functions
 */
#include "WmEvent.h"
#include "WmCDInfo.h"
#include "WmCDecor.h"
#include "WmCEvent.h"
#include "WmColormap.h"
#include "WmFunction.h"
#include "WmKeyFocus.h"
#include "WmManage.h"
#include "WmMenu.h"
#include "WmWinInfo.h"
#include "WmWinState.h"

/*
 * Global Variables:
 */

extern unsigned int buttonModifierMasks[];

#ifndef MOTIF_ONE_DOT_ONE
#include <Xm/MenuShellP.h>
#endif



/*************************************<->*************************************
 *
 *  InitEventHandling ()
 *
 *
 *  Description:
 *  -----------
 *  This function initializes window manager event handling in preparation
 *  for managing client windows.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD = (keySpecs)
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void InitEventHandling ()

#else /* _NO_PROTO */
void InitEventHandling (void)
#endif /* _NO_PROTO */
{
    WmScreenData *pSD;
    XSetWindowAttributes setAttributes;
    unsigned long base_mask;
    unsigned int n, scr;


    /*
     * Prepare to get root (workspace) window events that are used to
     * manage client windows.  Setup accelerator event processing.
     */

    base_mask = SubstructureRedirectMask | FocusChangeMask;

    /* handle entry of root window */
    base_mask |= EnterWindowMask | LeaveWindowMask;
#ifdef DEC_MOTIF_EXTENSION
#ifndef VMS
    /* For save yourself. */
    base_mask |= PropertyChangeMask;
#endif
#endif

    for (scr=0; scr<wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

        if (pSD->managed) 
        {
	    setAttributes.event_mask = base_mask;

	    if (pSD->buttonBindings)
	    {
		/*
		 * The desktop menu and button bindings for window 
		 * manager functions use button press and button 
		 * release events.
		 */
		setAttributes.event_mask |= 
		    (ButtonPressMask | ButtonReleaseMask);
	    }

            XChangeWindowAttributes (DISPLAY, pSD->rootWindow, 
                CWEventMask, &setAttributes);


	    /*
	     * Setup event handling for "accelerated" window management 
	     * functions done with key bindings.
	     */

            if (pSD->keySpecs)
            {
        	SetupKeyBindings (pSD->keySpecs, pSD->rootWindow, 
		    GrabModeSync, F_CONTEXT_ALL);
            }
        
	    if (pSD->acceleratorMenuCount)
	    {
		for (n = 0; n < pSD->acceleratorMenuCount; n++)
		{
		SetupKeyBindings (
		    pSD->acceleratorMenuSpecs[n]->accelKeySpecs,
		    pSD->rootWindow, GrabModeSync, F_CONTEXT_ALL);
		}
	    }
	} /* end if (managed) */
    }  /* end for (all screens) */
} /* END OF FUNCTION InitEventHandling */



#ifdef DEC_MOTIF_EXTENSION

/*************************************<->*************************************
 *
 *  WmModKeyCheck( keySpec, grabWindow, keyboardMode, ModMask )
 *
 *
 *  Description:
 *  -----------
 *  Grab the keys based on the mod masks.  Look at the
 *  at the resource and the mode switch.
 *  If the resource, ignoremodkeys is set, then check the
 *  mode_switch in the display to set the grab the button
 *  with the appropriate modifier.  
 *  If the mod mask was set before and is not set now,
 *  ungrab the key.
 *                
 *  If it's not set, then always grab the button.
 *
 *
 *  Inputs:
 *  ------
 *  key spec
 *  grab window
 *  key mode
 *  mask
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmModKeyCheck( keySpec, grabWindow, keyboardMode, ModMask )

KeySpec *keySpec;
Window grabWindow;
int keyboardMode;
unsigned int ModMask;

#else /* _NO_PROTO */
void WmModKeyCheck( KeySpec *keySpec, Window grabWindow, 
                    int keyboardMode, unsigned int ModMask )

#endif /* _NO_PROTO */

{

/********************************/

    if ( wmGD.ignoreAllModKeys ||                    
         ( wmGD.ignoreModKeys && !( ~ModeSwitchOfDisplay( DISPLAY ) & ModMask )))
      {
        XGrabKey( DISPLAY, keySpec->keycode, (keySpec->state | ModMask),
                  grabWindow, False, GrabModeAsync, keyboardMode );
        XGrabKey( DISPLAY, keySpec->keycode, 
                  (keySpec->state | ModMask | LockMask ), 
                  grabWindow, False, GrabModeAsync, keyboardMode );
      }

    /* If this has changed since the last mod key, 
       then ungrab the previous one. */
    else if ( !wmGD.ignoreAllModKeys &&
             ( wmGD.ignoreModKeys && (!( ~wmGD.mode_switch & ModMask ) &&
              ( ~ModeSwitchOfDisplay( DISPLAY ) & ModMask ))))
      { 
        XUngrabKey( DISPLAY, keySpec->keycode, (keySpec->state | ModMask),
                    grabWindow );
        XUngrabKey( DISPLAY, keySpec->keycode, 
                    (keySpec->state | ModMask | LockMask ),
                    grabWindow );
      }

}

#endif /* DEC_MOTIF_EXTENSION */
/*************************************<->*************************************
 *
 *  SetupKeyBindings (keySpecs, grabWindow, keyboardMode, context)
 *
 *
 *  Description:
 *  -----------
 *  This function sets up the event handling necessary to support user
 *  specified key bindings for window manager functions.
 *
 *
 *  Inputs:
 *  ------
 *  keySpecs = list of key bindings for window manager functions.
 *
 *  grabWindow = window that is to be associated with the passive key grab.
 *
 *  keyboardMode = indicates keyboard mode for grab.
 *
 *  context = context of key binding to set
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = number of key bindings set
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
int SetupKeyBindings (keySpecs, grabWindow, keyboardMode, context)
    KeySpec *keySpecs;
    Window grabWindow;
    int keyboardMode;
    long context;

#else /* _NO_PROTO */
int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, int keyboardMode, long context)
#endif /* _NO_PROTO */
{
    KeySpec *keySpec;
    int setCount = 0;
    Boolean iconContext;


    /*
     * Use key grabs to get the keys that invoke window manger functions.
     */

    iconContext = (context == F_CONTEXT_ICON);
    keySpec = keySpecs;
    while (keySpec)
    {
	if (((F_CONTEXT_ICON == (keySpec->context ^
				 (F_CONTEXT_ICONBOX     |
				  F_SUBCONTEXT_IB_IICON |
				  F_SUBCONTEXT_IB_WICON))) &&
             iconContext) ||
            ((F_CONTEXT_ICON != (keySpec->context ^
				 (F_CONTEXT_ICONBOX     |
				  F_SUBCONTEXT_IB_IICON |
				  F_SUBCONTEXT_IB_WICON))) &&
             !iconContext))
	{

	    XGrabKey (DISPLAY, keySpec->keycode, keySpec->state, grabWindow,
	        False, GrabModeAsync, keyboardMode);

	    XGrabKey (DISPLAY, keySpec->keycode, (keySpec->state | LockMask),
		grabWindow, False, GrabModeAsync, keyboardMode);

#ifdef DEC_MOTIF_EXTENSION
            /* For internationalization, if the mod keys are on and if
               the resource is set, ignore mod keys.
               Ignore MOD2, MOD3, MOD2|MOD3, LOCK|MOD2, LOCK|MOD3,
                      LOCK|MOD2|MOD3. */
            /* Is the resource set ? */
            if ( wmGD.ignoreModKeys || wmGD.ignoreAllModKeys )
              /* Yes, do the grabs. */
              {
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, Mod2Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, Mod3Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, Mod4Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, 
                                 Mod2Mask | Mod3Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, 
                                 Mod2Mask | Mod4Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, 
                                 Mod3Mask | Mod4Mask );
                  WmModKeyCheck( keySpec, grabWindow, keyboardMode, 
                                 Mod2Mask | Mod3Mask | Mod4Mask );
              } 
	    else
	      {
		  if (wmGD.ignoreNumLockMod) {
		      XGrabKey (DISPLAY, keySpec->keycode,
				(keySpec->state | NumLockMask),
				grabWindow, False, GrabModeAsync, keyboardMode);
		      XGrabKey (DISPLAY, keySpec->keycode,
				(keySpec->state | LockMask | NumLockMask),
				grabWindow, False, GrabModeAsync, keyboardMode);
		  }
	      }
#endif /* DEC_MOTIF_EXTENSION */
	    setCount++;
	}

	keySpec = keySpec->nextKeySpec;
    }

    return (setCount);

} /* END OF FUNCTION SetupKeyBindings */



/*************************************<->*************************************
 *
 *  WmDispatchMenuEvent (event)
 *
 *
 *  Description:
 *  -----------
 *  This function detects and processes events that affect menu behavior that
 *  are NOT dispatched (processed) by the toolkit.  The events may cause the 
 *  menu to be unposted, may trigger hotspot processing, or may represent 
 *  menu accelerators.  This processing is generally done when the system 
 *  menu is posted in "sticky" mode.
 *
 *
 *  Inputs:
 *  ------
 *  event = This is an X event that has been retrieved by XtNextEvent.
 *  wmGD.menuActive == nonNULL
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean WmDispatchMenuEvent (event)
    XButtonEvent *event;

#else /* _NO_PROTO */
Boolean WmDispatchMenuEvent (XButtonEvent *event)
#endif /* _NO_PROTO */
{
    ClientData *pCD = wmGD.menuClient;
    Boolean     doXtDispatchEvent = True;
    Boolean     checkContext;
    Context     context = 0;


    if (event->type == KeyPress)
    {
        if (wmGD.menuActive->accelKeySpecs)
        {
	    /*
	     * Check to see if the KeyPress is a menu accelerator
	     * (don't require context match for system menu accelerators).
	     * If so, the active menu will be unposted and the KeyPress event 
	     * will not be sent on to the toolkit.
	     */

  	    checkContext = (!pCD || (pCD->systemMenuSpec != wmGD.menuActive));
	    if (checkContext)
	    {
                if (pCD)
                {
            	    if (pCD->clientState == MINIMIZED_STATE)
            	    {
            	        context = F_CONTEXT_ICON;
            	    }
            	    else if (pCD->clientState == NORMAL_STATE)
            	    {
            	        context = F_CONTEXT_NORMAL;
            	    }
            	    else
            	    {
            	        context = F_CONTEXT_MAXIMIZE;
            	    }
                }
                else
                {
            	    context = F_CONTEXT_ROOT;
                }
	    }
	    doXtDispatchEvent = 
		HandleKeyPress ((XKeyEvent *)event, 
				wmGD.menuActive->accelKeySpecs,
				checkContext, context, 
				TRUE, (ClientData *)NULL);
        }

        if (wmGD.menuActive && wmGD.menuUnpostKeySpec)
        {
	    if ((wmGD.menuUnpostKeySpec->state == event->state) &&
	        (wmGD.menuUnpostKeySpec->keycode == event->button))
	    {
	        /*
	         * This is an alternate key for unposting a menu from the
	         * keyboard (in addition to [ESC]).
	         */

	        UnpostMenu (wmGD.menuActive);
	        doXtDispatchEvent = False;
	    }
        }
    }

    else if (wmGD.checkHotspot &&
	     ((event->type == ButtonPress) || 
	      (event->type == ButtonRelease)) &&
	     (event->x_root >= wmGD.hotspotRectangle.x) &&
	     (event->y_root >= wmGD.hotspotRectangle.y) &&
	     (event->x_root <
		(wmGD.hotspotRectangle.x + wmGD.hotspotRectangle.width)) &&
	     (event->y_root <
		(wmGD.hotspotRectangle.y + wmGD.hotspotRectangle.height))&&
	     pCD)
    {
	/*   ^^^
	 * Added check for NULL pCD in the above condition.
	 * We should never get here with a NULL pCD, but, 
	 * sometimes our UnmapCallback for a menu does not
	 * get called, so..., we get to this point because
	 * wmGD.menuActive is not cleared, but, wmGD.menuClient 
	 * is set to NULL when we unmanage the client window.
	 */
	
	/*
	 * The event triggers hotspot processing for the system menu button
	 * or an icon.
	 */

	if (event->type == ButtonRelease)
	{
	    /*
	     * The system menu is posted from a system menu button or an
	     * icon.  By doing a button release over the system menu button
	     * or icon the system menu that is posted is put into keyboard
	     * traversal mode.
	     */

	    ProcessClickBRelease (event, pCD, wmGD.clickData.context,
		wmGD.clickData.subContext);

	    if (wmGD.clickData.context == F_SUBCONTEXT_W_SYSTEM)
	    {
		PopGadgetOut (pCD, FRAME_SYSTEM);
	    }
#ifdef MOTIF_ONE_DOT_ONE
	    TraversalOn (pCD->systemMenuSpec);
	    doXtDispatchEvent = False;
#else
 	    _XmSetLastManagedMenuTime ((Widget)XtParent(pCD->systemMenuSpec->menuWidget),
				       ((XButtonEvent *)event)->time);
	    doXtDispatchEvent = True;
#endif
	}
	else
	{
	    /*
	     * A button press over a system menu button or an icon when the
	     * system menu is posted indicates that a double-click action is
	     * to be done if appropriate and the menu is to be taken
	     * out of traversal mode (done by the menu widget).
	     */

	    ProcessClickBPress (event, pCD, wmGD.clickData.context,
				wmGD.clickData.subContext);

	    if (wmGD.clickData.subContext == F_SUBCONTEXT_W_SYSTEM)
	    {
		PushGadgetIn (pCD, FRAME_SYSTEM);
	    }

	    if (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_W_SYSTEM)
	    {
		if (wmGD.systemButtonClick2 &&
		    (pCD->clientFunctions & MWM_FUNC_CLOSE))
		{
		    /*
		     * Close the client window.  Cancel other system menu
		     * button actions.
		     */

		    UnpostMenu (pCD->systemMenuSpec);
		    F_Kill (NULL, pCD, (XEvent *) event);
		    doXtDispatchEvent = False;
		}
#ifdef DEC_MOTIF_EXTENSION
/* Double-default on icon box is pack */
        	else if ( pCD->clientFlags && ICON_BOX )
                {
                    UnpostMenu (pCD->systemMenuSpec);
                    if (( pCD->clientState == NORMAL_STATE ) ||
                        ( pCD->clientState == MAXIMIZED_STATE ))
                      {
                        /* Yup, pack it. */
                        F_Pack_Icons( (String)NULL, pCD, (XEvent *)NULL);
                        doXtDispatchEvent = False;
                      }
                }
#endif /* DEC_MOTIF_EXTENSION */
	    }
	    else
	    if (wmGD.clickData.doubleClickContext == F_SUBCONTEXT_I_ALL)
	    {
		/*
		 * Normalize the icon.
		 */
		int newState;

		UnpostMenu (pCD->systemMenuSpec);
		if (pCD->maxConfig)
		{
		    newState = MAXIMIZED_STATE;
		}
		else
		{
		    newState = NORMAL_STATE;
		}

		SetClientState (pCD, newState, event->time);
		wmGD.clickData.clickPending = False;
		wmGD.clickData.doubleClickPending = False;
		doXtDispatchEvent = False;
	    }
	    else
	    if ((wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_IICON)||
		(wmGD.clickData.doubleClickContext == F_SUBCONTEXT_IB_WICON))
            {
                /*
                 * Raise the Window and Normalize
                 */
		
                UnpostMenu (pCD->systemMenuSpec);
		F_Restore_And_Raise ((String)NULL, pCD, (XEvent *)NULL);
/*		F_Normalize_And_Raise ((String)NULL, pCD, (XEvent *)NULL);
*/                doXtDispatchEvent = False;
            }
	    /*
	     * Else no special button press processing; have the toolkit
	     * dispatch the event to the menu widgets.
	     */
	}
    }

    return (doXtDispatchEvent);


} /* END OF FUNCTION WmDispatchMenuEvent */



/*************************************<->*************************************
 *
 *  WmDispatchWsEvent (event)
 *
 *
 *  Description:
 *  -----------
 *  This function detects and dispatches events that are reported to the root
 *  (workspace) window and that are not widget-related (i.e. they would not be
 *  dispatched by the Xtk intrinsics).
 *
 *
 *  Inputs:
 *  ------
 *  event = This is an X event that has been retrieved by XtNextEvent.
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True the event should be dispatched by the toolkit,
 *      otherwise the event should not be dispatched.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean WmDispatchWsEvent (event)
    XEvent *event;

#else /* _NO_PROTO */
Boolean WmDispatchWsEvent (XEvent *event)
#endif /* _NO_PROTO */
{
    ClientData *pCD;
    Boolean dispatchEvent = False;
    WmScreenData *pSD;
#ifdef DEC_MOTIF_BUG_FIX
#ifndef VMS
    XPropertyEvent *pevent = (XPropertyEvent *)event;
#endif /* not VMS */
#endif /* DEC_MOTIF_BUG_FIX */


    /*
     * Detect and dispatch non-widget events that have been reported to
     * the root window.
     */

    switch (event->type)
    {
#ifdef DEC_MOTIF_BUG_FIX
#ifndef VMS                          
/* Fix for core dump when session manager exits on Ultrix based systems.
   The session manager will send Mwm a save yourself on the root window. */
	case PropertyNotify:
	{
            /* Is this a save yourself on the root ? */
	    if (pevent->atom == wmGD.xa_WM_SAVE_YOURSELF)
                /* Yes, cleanup and exit */
                Do_Quit_Mwm( FALSE );
  	    dispatchEvent = False;
	    break;
	}
#endif /* not VMS */
#endif /* DEC_MOTIF_BUG_FIX */
	case KeyPress:
	{
	    /*
	     * The key press is to initiate some window management
	     * function (e.g., shuffle the client windows).
	     */

	    dispatchEvent = HandleWsKeyPress ((XKeyEvent *)event);
	    break;
	}

	case ButtonPress:
	{
	    /*
	     * The button press is to initiate some window management
	     * function (e.g., pop up the desktop menu).
	     */

	    if (wmGD.menuActive)
	    {
		dispatchEvent = True; /* have the toolkit dispatch the event */
	    }
	    else
	    {
		HandleWsButtonPress ((XButtonEvent *)event);
	    }
	    break;
	}

	case ButtonRelease:
	{
	    /*
	     * The button release may do some window management
	     * function.
	     */

	    if (wmGD.menuActive)
	    {
		dispatchEvent = True; /* have the toolkit dispatch the event */
	    }
	    else
	    {
		HandleWsButtonRelease ((XButtonEvent *)event);
	    }
	    break;
	}

	case UnmapNotify:
	{
	  /* BEGIN CR 5183 */
	  if ( (!XFindContext (DISPLAY, event->xunmap.window,
			       wmGD.windowContextType,
			       (XPointer *)&pCD)
		)
	      && (((XUnmapEvent *)event)->window == pCD->client)
	      )
	  /* END CR 5183 */
	    {
		/*
		 * This is a synthetic UnmapNotity used to withdraw a client
		 * window form window manager control.
		 */

		UnManageWindow (pCD);
	    }
	    break;
	}

	case EnterNotify:
	{
	    HandleWsEnterNotify ((XEnterWindowEvent *)event);
	    break;
	}

	case LeaveNotify:
	{
	    HandleWsLeaveNotify ((XLeaveWindowEvent *)event);
	    break;
	}

	case ConfigureRequest:
	{
	    HandleWsConfigureRequest ((XConfigureRequestEvent *)event);
	    break;
	}

	case MapRequest:
	{
	    /*
	     * Determine if the window is already being managed:
	     */

	    if ((XFindContext (DISPLAY, event->xmaprequest.window,
		    wmGD.windowContextType, (XPointer *)&pCD)) &&
		(pSD = GetScreenForWindow (event->xmaprequest.window)))
	    {
	        /*
                 * The window is not yet managed and it's parented to a 
		 * screen/root window that we manage. Start to manage the
		 * new window.  Management details are dependent on the
		 * type of the window.  For a typical top-level application
		 * window reparent the window to a window frame, add it to
		 * the wm saveset, ...
                 */

                ManageWindow (pSD, event->xmaprequest.window, MANAGEW_NORMAL);
	    }
	    /* else ...
	     * The context information on the window WAS found.
	     * The window is already managed by the window manager
	     * so this is redundant request to have the client      
	     * window managed.
	     */

      	    break;
	}

	case FocusIn:
	{
	    HandleWsFocusIn ((XFocusInEvent *)event);
	    break;
	}

	case FocusOut:
	{
	    break;
	}

#ifdef DEC_MOTIF_EXTENSION
        /* Mapping notify event */
        case MappingNotify:
          WmManageModReset( event );
          break;
/* add events for the mwm dialog boxes */
        default:
             dispatchEvent = event->xany.display == wmGD.dialog_display;
             break;
#endif             
    } /* end of event.type switch */


    return (dispatchEvent);

} /* END OF FUNCTION WmDispatchWsEvent */



/*************************************<->*************************************
 *
 *  HandleWsKeyPress (keyEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes KeyPress events that are reported to the root
 *  window.  These events are generally associated with accelerators.
 *
 *
 *  Inputs:
 *  ------
 *  keyEvent = pointer to a key press event on the root window.
 *
 *  Output:
 *  ------
 *  RETURN = True is the event is to be dispatched by XtDispatch.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean HandleWsKeyPress (keyEvent)
    XKeyEvent *keyEvent;

#else /* _NO_PROTO */
Boolean HandleWsKeyPress (XKeyEvent *keyEvent)
#endif /* _NO_PROTO */
{
    Boolean      dispatchEvent = False;
    Boolean      checkKeyEvent = True;
    unsigned int n;
    Context      context;

    if (wmGD.menuActive)
    {
	/*
	 *  The active menu accelerators have been checked and keyEvent was
	 *  not one of them.  We will check for pass keys mode and then 
	 *  have the toolkit dispatch the event, without searching any other 
	 *  key or accelerator specification list.
	 */

	dispatchEvent = True;
	checkKeyEvent = False;
    }

    /*
     * If pass keys is active then only check for getting out of the
     * pass keys mode.  Unfreeze the keyboard and replay the key if
     * pass keys is active.
     */

    if (wmGD.passKeysActive)
    {
	if (wmGD.passKeysKeySpec &&
	    (wmGD.passKeysKeySpec->state == keyEvent->state) &&
	    (wmGD.passKeysKeySpec->keycode == keyEvent->keycode))
	{
	    /*
	     * Get out of the pass keys mode.
	     */

	    F_Pass_Key (NULL, (ClientData *) NULL, (XEvent *) NULL);
	    XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);
	}
	else
	{
	    XAllowEvents (DISPLAY, ReplayKeyboard, CurrentTime);
	}
	checkKeyEvent = False;
    }
    else
    {
	XAllowEvents (DISPLAY, AsyncKeyboard, CurrentTime);
    }


    /*
     * Search through the key specification list and the menu 
     * accelerator lists until these lists are exhausted or
     * the event is handled.
     */

    if (checkKeyEvent)
    {
        if (wmGD.keyboardFocus)
        {
	    if (wmGD.keyboardFocus->clientState == MINIMIZED_STATE)
	    {
	        context = F_CONTEXT_ICON;
	    }
	    else if (wmGD.keyboardFocus->clientState == NORMAL_STATE)
	    {
	        context = F_CONTEXT_NORMAL;
	    }
	    else
	    {
	        context = F_CONTEXT_MAXIMIZE;
	    }
        }
        else
        {
	    context = F_CONTEXT_ROOT;
        }

        if (HandleKeyPress (keyEvent, ACTIVE_PSD->keySpecs, 
	                    TRUE, context, FALSE, (ClientData *)NULL) &&
	    ACTIVE_PSD->acceleratorMenuCount)
	{
	    for (n = 0; ((keyEvent->keycode != 0) &&
			 (n < ACTIVE_PSD->acceleratorMenuCount)); n++)
	    {
	        if (!HandleKeyPress (keyEvent,
		     ACTIVE_PSD->acceleratorMenuSpecs[n]->accelKeySpecs, 
				 TRUE, context, TRUE,(ClientData *)NULL))
	        {
		    break;
		}
	    }
        }
    }

    return (dispatchEvent);

} /* END OF FUNCTION HandleWsKeyPress */



/*************************************<->*************************************
 *
 *  HandleKeyPress (keyEvent, keySpecs, checkContext, context, onlyFirst, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies window manager functions that are triggered by
 *  a KeyPress event.  The window manager functions are done if appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  keyEvent = pointer to a key press event on the root window
 *  keySpecs = pointer to a key specification list to search
 *  checkContext = TRUE iff the context must match the keySpec context.
 *  context = context to match keySpec context.
 *  onlyFirst = TRUE iff key processing should stop with the first match.
 *
 *  Output:
 *  ------
 *  RETURN = False if key binding processing should be terminated; True if
 *	key binding processing can continue
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean HandleKeyPress (keyEvent, keySpecs, checkContext, context, onlyFirst, pCD)
    
    XKeyEvent *keyEvent;
    KeySpec   *keySpecs;
    Boolean    checkContext;
    Context    context;
    Boolean    onlyFirst;
    ClientData *pCD;
    

#else /* _NO_PROTO */
Boolean HandleKeyPress (XKeyEvent *keyEvent, 
			KeySpec *keySpecs, 
			Boolean checkContext, 
			Context context, 
			Boolean onlyFirst, 
			ClientData *pCD)
#endif /* _NO_PROTO */
{
    Boolean     processKey = True;
    ClientData *functionClient;

    /*
     * Search for matching key specification.
     */

    while (processKey && keySpecs)
    {
	if ((keyEvent->state == keySpecs->state) &&
	    (keyEvent->keycode == keySpecs->keycode) &&
            ((!checkContext) || (context & keySpecs->context)))
	{
	    /*
	     * A matching key binding has been found.
	     * Determine the client to which the key binding function is to
	     *   apply.
	     * Unpost any active menu and specify that no futher key binding 
	     *   processing should be done.
	     * Do the function associated with the matching key binding.
	     * Stop if onlyFirst == TRUE
	     */

            if (pCD)
            {
                functionClient = pCD;
            }
            else
            {
                functionClient = wmGD.keyboardFocus;
            }

            if (wmGD.menuActive)
	    {
                functionClient = wmGD.menuClient;  /* might not have focus! */
	        UnpostMenu (wmGD.menuActive);
		processKey = False;
	    }
            else if (onlyFirst)
	    {
		processKey = False;
	    }

	    if ((keySpecs->wmFunction == F_Menu) ||
	        (keySpecs->wmFunction == F_Post_SMenu))
	    {
	        wmGD.menuUnpostKeySpec = keySpecs;  /* menu unpost key spec */
	    }
	    else if (keySpecs->wmFunction == F_Pass_Key)
     	    {
		wmGD.passKeysKeySpec = keySpecs;
	    }
	    if (!(keySpecs->wmFunction (keySpecs->wmFuncArgs,
                                        functionClient, keyEvent)))
	    {
		/*
		 * The window manager function return indicates that further
		 * key binding processing should not be done.
		 */

		processKey = False;
	    }
	}
	keySpecs = keySpecs->nextKeySpec;
    }

    return (processKey);


} /* END OF FUNCTION HandleKeyPress */



/*************************************<->*************************************
 *
 *  HandleWsButtonPress (buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies button events that are associated with window
 *  manager functions.  Window manager functions are done if appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event on the root window
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsButtonPress (buttonEvent)
    XButtonEvent *buttonEvent;

#else /* _NO_PROTO */
void HandleWsButtonPress (XButtonEvent *buttonEvent)
#endif /* _NO_PROTO */
{
    ClientData *pCD;
    Context context;
    int partContext;
    Context subContext;


    /*
     * Determine if the top-level window that contains the pointer is a
     * client managed by the window manager (there may be no window under
     * the pointer or it may be an "override-redirect" window).
     */

    if ((buttonEvent->subwindow == None) ||
	(XFindContext (DISPLAY, buttonEvent->subwindow, wmGD.windowContextType,
	     (XPointer *)&pCD)))
    {
	/* no managed window under the pointer */
	pCD = NULL;
    }
    

    /*
     * Look through the window manager function button binding list for
     * matches with the event:
     */

    IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
    subContext = (1L << partContext);

    ProcessClickBPress (buttonEvent, pCD, context, subContext);

    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) && pCD)
    {
	/*
	 * Button bindings have been processed, now check for bindings that
	 * are associated with the built-in semantics of the window frame
	 * decorations.
	 */

	CheckButtonPressBuiltin (buttonEvent, context, subContext, partContext,
	    pCD);
    }
    /*
     * Else skip built-in processing due to execution of a function that
     * does on-going event processing or that has changed the client state
     * (e.g., f.move or f.minimize).
     */


} /* END OF FUNCTION HandleWsButtonPress */



/*************************************<->*************************************
 *
 *  HandleWsButtonRelease (buttonEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies button release events that are associated with
 *  window manager functions.  Window manager functions are done if
 *  appropriate.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button release event
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsButtonRelease (buttonEvent)
    XButtonEvent *buttonEvent;

#else /* _NO_PROTO */
void HandleWsButtonRelease (XButtonEvent *buttonEvent)
#endif /* _NO_PROTO */
{
    ClientData *pCD;
    Context context;
    int  partContext;
    Context subContext;


    /*
     * Determine if the top-level window that contains the pointer is a
     * client managed by the window manager (there may be no window under
     * the pointer or it may be an "override-redirect" window).
     */

    if ((buttonEvent->subwindow == None) ||
	(XFindContext (DISPLAY, buttonEvent->subwindow, wmGD.windowContextType,
	     (XPointer *)&pCD)))
    {
	/* no managed window under the pointer */
	pCD = NULL;
    }
    

    /*
     * Look for a builtin function that may be done by this event.
     */

    IdentifyEventContext (buttonEvent, pCD, &context, &partContext);
    subContext = (1L << partContext);

    ProcessClickBRelease (buttonEvent, pCD, context, subContext);

    if (CheckForButtonAction (buttonEvent, context, subContext, pCD) && pCD)
    {
	/*
	 * Button bindings have been processed, now check for bindings that
	 * are associated with the built-in semantics of the window frame
	 * decorations.
	 */

	CheckButtonReleaseBuiltin (buttonEvent, context, subContext, pCD);
    }
    /*
     * Else skip built-in processing due to execution of a function that
     * does on-going event processing or that has changed the client state
     * (e.g., f.move or f.minimize).
     */


} /* END OF FUNCTION HandleWsButtonRelease */



/*************************************<->*************************************
 *
 *  CheckForButtonAction (buttonEvent, context, subContext, pCD)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a button event is to do a button binding
 *  action.  The action is done if specified.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = a button event handled by the window manager
 *
 *  context = button event context (root, icon, window)
 *
 *  subContext = button event subContext (title, system button, etc.)
 *
 *  pCD = a pointer to client data that is associated with the button event
 *
 *
 *  Outputs:
 *  -------
 *  RETURN = If True then further button binding processing can be done;
 *      if false then a state change function, menu function, or
 *      configuration function is ongoing and further button binding
 *      processing should not be done.
 *
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Boolean CheckForButtonAction (buttonEvent, context, subContext, pCD)
    XButtonEvent *buttonEvent;
    Context context;
    Context subContext;
    ClientData *pCD;

#else /* _NO_PROTO */
Boolean CheckForButtonAction (XButtonEvent *buttonEvent, Context context, Context subContext, ClientData *pCD)
#endif /* _NO_PROTO */
{
    ButtonSpec *buttonSpec;

    /*
     * Look through the window manager function button binding list for
     * matches with the event:
     */

    buttonSpec = ACTIVE_PSD->buttonSpecs;
    while (buttonSpec)
    {
	if ((buttonEvent->button == buttonSpec->button) &&
	    ((buttonEvent->state == buttonSpec->state) ||
	     (wmGD.currentEventState == buttonSpec->state)))
	{
	    /*
	     * See if the event context matches the binding context.
	     */

	    if ((buttonEvent->type == buttonSpec->eventType) &&
	        (context & buttonSpec->context) &&
		(subContext & buttonSpec->subContext))
	    {

		/*
		 * For click type bindings check for a match between the
		 * event context and the click / double-click context.
		 */

	        if (buttonEvent->type == ButtonRelease)
	        {
		    /*
		     * Clicks occur on button releases.  A button release
		     * binding is always treated as a click binding.
		     */

		    if ((buttonSpec->subContext | wmGD.clickData.clickContext)
			 != buttonSpec->subContext)
		    {
		        /* click binding and event contexts do not match */
			buttonSpec = buttonSpec->nextButtonSpec;
		        continue;
		    }
		    /* else there is a click match */
	        }
		else if (buttonSpec->click && (buttonEvent->type==ButtonPress))
		{
		    /*
		     * Double-clicks occur on button presses.
		     */

		    if ((buttonSpec->subContext |
					wmGD.clickData.doubleClickContext)
			!= buttonSpec->subContext)
		    {
			/* click binding and event contexts do not match */
			buttonSpec = buttonSpec->nextButtonSpec;
			continue;
		    }
		    else
		    {
		        /*
			 * The is a double-click match.  Don't do any click
			 * or double-click matches for the following button
			 * press and release.
			 */

			wmGD.clickData.clickPending = False;
			wmGD.clickData.doubleClickPending = False;
		    }
		}

	        if (!(buttonSpec->wmFunction (buttonSpec->wmFuncArgs, pCD,
					      buttonEvent)))
		{
		    /*
		     * The window manager function return indicates that
		     * further button binding processing should not be done.
		     */

		    return (False);
		}
	    }
	}
	buttonSpec = buttonSpec->nextButtonSpec;
    }

    return (True);


} /* END OF FUNCTION CheckForButtonAction */



/*************************************<->*************************************
 *
 *  IdentifyEventContext (event, pCD, pContext, pPartContext)
 *
 *
 *  Description:
 *  -----------
 *  This function identifies the context in which an event occured.  The 
 *  pointer position is used to identify the context if the event is a
 *  button event.  If the context and the window state are incompatible
 *  (e.g., the context is window and the window is minimized) then the
 *  context is reset to 0 (none).
 *
 *
 *  Inputs:
 *  ------
 *  event = find the context of this X event
 *
 *  pCD = client data (maybe NULL) that the event is associated with
 *
 * 
 *  Outputs:
 *  -------
 *  pContext = event context
 *
 *  pPartContext = part (e.g, frame) context associated with the event
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void IdentifyEventContext (event, pCD, pContext, pPartContext)
    XButtonEvent *event;
    ClientData *pCD;
    Context *pContext;
    int *pPartContext;

#else /* _NO_PROTO */
void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, Context *pContext, int *pPartContext)
#endif /* _NO_PROTO */
{
    Boolean eventOnRoot;
    Window actionWindow;
    int clientX;
    int clientY;
    int framePart;


    eventOnRoot = (event->window == ACTIVE_ROOT) ? 
				True : False;

    if (pCD)
    {
	actionWindow = (eventOnRoot) ? event->subwindow : event->window;
	if (actionWindow == pCD->clientFrameWin)
	{
	    *pContext = F_CONTEXT_WINDOW;

	    if (eventOnRoot)
	    {
	        clientX = event->x -
			  (pCD->maxConfig ? pCD->maxX : pCD->clientX) +
			  pCD->clientOffset.x;
	        clientY = event->y -
			  (pCD->maxConfig ? pCD->maxY : pCD->clientY) +
			  pCD->clientOffset.y;
	    }
	    else
	    {
		clientX = event->x;
		clientY = event->y;
	    }
	    framePart = IdentifyFramePart (pCD, clientX, clientY);
	    *pPartContext = framePart;
	}
	else if (actionWindow == pCD->clientBaseWin)
	{
	    *pContext = F_CONTEXT_WINDOW;
	    *pPartContext = FRAME_CLIENT;
	}
	else if ((actionWindow == ICON_FRAME_WIN(pCD)) ||
		 (actionWindow == ACTIVE_PSD->activeIconTextWin))
	{
	    if (P_ICON_BOX(pCD))
	    {
	        *pContext = F_CONTEXT_ICONBOX;
		if (pCD->clientState == MINIMIZED_STATE)
		{
		    *pPartContext = ICONBOX_PART_IICON;
		}
		else
		{
		    *pPartContext = ICONBOX_PART_WICON;
		}
	    }
	    else
	    {
	        *pContext = F_CONTEXT_ICON;
	        *pPartContext = ICON_PART_ALL;
	    }
	}
	else
	{
	    *pContext = F_CONTEXT_ROOT;
	    *pPartContext = ROOT_PART_ALL;
	}

	/*
	 * Check for an incompatible context and window state.
	 */

	if (((*pContext & F_CONTEXT_WINDOW) &&
	     (pCD->clientState != NORMAL_STATE) &&
	     (pCD->clientState != MAXIMIZED_STATE)) ||
	    ((*pContext & F_CONTEXT_ICON) &&
	     (pCD->clientState != MINIMIZED_STATE)))
	{
	    *pContext = F_CONTEXT_NONE;
	}
    }
    else
    {
	*pContext = F_CONTEXT_ROOT;
	*pPartContext = ROOT_PART_ALL;
    }


} /* END OF FUNCTION IdentifyEventContext */



/*************************************<->*************************************
 *
 *  ProcessClickBPress (buttonEvent, pCD, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  This function checks for a double-click match and saves state information
 *  to do click and double-click processing.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event
 *
 *  pCD = pointer to client data (identifies client window)
 *
 *  context = root/window/icon context for the event
 *
 *  subContext = subcontext for the event (title, system button, etc.)
 *
 * 
 *  Outputs:
 *  -------
 *  (wmGD.clickData) = click processing information
 *
 *  (wmGD.clickData.doubleClickContext) = set if double click occured
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ProcessClickBPress (buttonEvent, pCD, context, subContext)
    XButtonEvent *buttonEvent;
    ClientData *pCD;
    Context context;
    Context subContext;

#else /* _NO_PROTO */
void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, Context context, Context subContext)
#endif /* _NO_PROTO */
{
    Time timeDiff;
    Boolean passButton;


    /*
     * Check for a double-click.  If a double click has occurred then
     * save the double-click context.
     */

    wmGD.clickData.doubleClickContext = F_SUBCONTEXT_NONE;
    if (wmGD.clickData.doubleClickPending &&
	(buttonEvent->button == wmGD.clickData.button) &&
        (buttonEvent->state == wmGD.clickData.state) &&
	(pCD == wmGD.clickData.pCD) &&
	(context == wmGD.clickData.context))
    {
	/*
	 * Check the time between button release events.
	 */

	if (buttonEvent->time > wmGD.clickData.time)
	{
	    timeDiff = buttonEvent->time - wmGD.clickData.time;
	}
	else
	{
	    timeDiff = ~wmGD.clickData.time + buttonEvent->time + 1;
	}

	if (timeDiff < wmGD.doubleClickTime)
	{
	    /*
	     * A double-click has been done; save the context.
	     */

	    wmGD.clickData.doubleClickContext = subContext |
						wmGD.clickData.subContext;
	}
    }


    /*
     * Save state data for click checking.  If a button binding match
     * occurs for a double-click then clear out the clickData (don't
     * do any click/double-click matches for the following button press
     * and release).  If the button press is done on the client area and
     * is used to set the focus to the window then don't use it in
     * setting up clickData.
     */

    if ((buttonEvent->button == SELECT_BUTTON) && (buttonEvent->state == 0))
    {
	passButton = wmGD.passSelectButton;
    }
    else
    {
	passButton = wmGD.passButtons;
    }

    if (!(pCD && (buttonEvent->window == pCD->clientBaseWin) && passButton))
    {
        wmGD.clickData.button = buttonEvent->button;
        wmGD.clickData.state = buttonEvent->state;
        /* add in event button mask (this will show up in the button release */
        wmGD.clickData.releaseState = buttonEvent->state |
				    buttonModifierMasks[buttonEvent->button];
        wmGD.clickData.pCD = pCD;
        wmGD.clickData.context = context;
        wmGD.clickData.subContext = subContext;
        wmGD.clickData.time = buttonEvent->time;
        wmGD.clickData.clickPending = True;
        wmGD.clickData.doubleClickPending = True;
    }


} /* END OF FUNCTION ProcessClickBPress */



/*************************************<->*************************************
 *
 *  ProcessClickBRelease (buttonEvent, pCD, context, subContext)
 *
 *
 *  Description:
 *  -----------
 *  This function checks to see if a "click" was done.  The button release
 *  completes a click if there is a click pending and the button release
 *  context is the same as the button press context.  Configuration or
 *  menu activity cancels a pending click.
 *
 *
 *  Inputs:
 *  ------
 *  buttonEvent = pointer to a button press event
 *
 *  pCD = pointer to client data (identifies client window)
 *
 *  context = root/window/icon context for the event
 *
 *  subContext = window subcontext for the event (title, system button, etc.)
 *
 *  (wmGD.clickData) = click processing information
 *
 * 
 *  Outputs:
 *  -------
 *  (wmGD.clickData) = click processing information
 *
 *  (wmGD.clickData.clickContext) = set if click occured
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ProcessClickBRelease (buttonEvent, pCD, context, subContext)
    XButtonEvent *buttonEvent;
    ClientData *pCD;
    Context context;
    Context subContext;

#else /* _NO_PROTO */
void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, Context context, Context subContext)
#endif /* _NO_PROTO */
{

    /*
     * Restore the state of the last "depressed" frame gadget
     */

    if (pCD && (wmGD.gadgetClient == pCD) && (pCD->decorFlags))
    {
	PopGadgetOut(pCD, wmGD.gadgetDepressed);
    }
	

    /*
     * Check to see if a click has been done.
     */

    wmGD.clickData.clickContext = F_SUBCONTEXT_NONE;
    if (wmGD.clickData.clickPending &&
	(buttonEvent->button == wmGD.clickData.button) &&
	(buttonEvent->state == wmGD.clickData.releaseState) &&
	(pCD == wmGD.clickData.pCD) &&
	(context == wmGD.clickData.context))
    {
	wmGD.clickData.clickContext = subContext | wmGD.clickData.subContext;
	/* !!! check for double click time? !!! */
    }
    else
    {
	wmGD.clickData.doubleClickPending = False;
    }

    wmGD.clickData.clickPending = False;


} /* END OF FUNCTION ProcessClickBRelease */



/*************************************<->*************************************
 *
 *  HandleWsEnterNotify (enterEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes EnterNotify events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  enterEvent = pointer to an enter notify event on the root window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsEnterNotify (enterEvent)
    XEnterWindowEvent *enterEvent;

#else /* _NO_PROTO */
void HandleWsEnterNotify (XEnterWindowEvent *enterEvent)
#endif /* _NO_PROTO */
{
    WmScreenData *pSD;

    /*
     * If the pointer entered a screen that we manage, then set the
     * new active screen.
     */
    if (wmGD.queryScreen &&
	(!XFindContext (DISPLAY, enterEvent->window, wmGD.screenContextType,
	    (XPointer *)&pSD)))
    {
	SetActiveScreen (pSD);
    }

    /*
     * The root window was entered; do focus processing
     * if necessary:
     */
    

    if (!wmGD.menuActive &&
	((enterEvent->mode == NotifyNormal) ||
	 (enterEvent->mode == NotifyUngrab) ||
	 (enterEvent->mode == NotifyWhileGrabbed)))
    {
        if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	{
	    Do_Focus_Key ((ClientData *) NULL, enterEvent->time, 
			ALWAYS_SET_FOCUS);
	}
	else if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT) &&
	         ((enterEvent->detail == NotifyNonlinearVirtual) ||
	          (enterEvent->detail == NotifyNonlinear)) &&
		 (wmGD.keyboardFocus == NULL) &&
		 enterEvent->focus)
	{
	    /*
	     * Reset the explicit selection focus to the workspace
	     * window.
	     */

	    Do_Focus_Key ((ClientData *) NULL, CurrentTime, 
	    		ALWAYS_SET_FOCUS);
	}

        if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	{
	    SetColormapFocus (ACTIVE_PSD, (ClientData *) NULL);
	}
    }

} /* END OF FUNCTION HandleWsEnterNotify */



/*************************************<->*************************************
 *
 *  HandleWsLeaveNotify (leaveEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes LeaveNotify events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  enterEvent = pointer to an leave notify event on the root window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsLeaveNotify (leaveEvent)
    XLeaveWindowEvent *leaveEvent;

#else /* _NO_PROTO */
void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent)
#endif /* _NO_PROTO */
{
    WmScreenData *pSD;

    /*
     * The root window was exited; do focus processing
     * if necessary:
     */

    if (!wmGD.menuActive &&
	((leaveEvent->detail == NotifyNonlinear) ||
	(leaveEvent->detail == NotifyNonlinearVirtual)))
    {
	/*
	 * The pointer has moved to another screen.  Fix the
	 * focus on the screen controlled by the window manager.
	 */

        if ((wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER) ||
            (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER))
	{
	    if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_POINTER)
	    {
	        Do_Focus_Key ((ClientData *) NULL, leaveEvent->time,
		    (SCREEN_SWITCH_FOCUS | ALWAYS_SET_FOCUS));
	    }
	    if (wmGD.colormapFocusPolicy == CMAP_FOCUS_POINTER)
	    {
	        SetColormapFocus (ACTIVE_PSD, (ClientData *) NULL);
	    }
	}

	/*  Set new active screen */

	if (!XFindContext (DISPLAY, leaveEvent->root, wmGD.screenContextType,
	    (XPointer *)&pSD))
	{
	    /* moved to another screen we manage! */
	    SetActiveScreen (pSD);
	}
	else
	{
	    /* off onto an unmanaged screen */
	    wmGD.queryScreen = True;

	    /* set input focus to pointer root */
	    XSetInputFocus (DISPLAY, PointerRoot, 
		RevertToPointerRoot, leaveEvent->time);
	}
    }
} /* END OF FUNCTION HandleWsLeaveNotify */



/*************************************<->*************************************
 *
 *  HandleWsConfigureRequest (focusEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes ConfigureRequest events that are reported to
 *  the root window.
 *
 *
 *  Inputs:
 *  ------
 *  focusEvent = pointer to a configure request event on the root window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsConfigureRequest (configureEvent)
    XConfigureRequestEvent *configureEvent;

#else /* _NO_PROTO */
void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent)
#endif /* _NO_PROTO */
{
    ClientData *pCD;
    XConfigureEvent notifyEvent;
    Boolean configChanged;
    XWindowChanges values;


    /*
     * A window that is a child of the root window is being
     * configured.  Either it is an un-managed window or it is a
     * managed window that did the configuration before it was
     * reparented.
     */

    if (XFindContext (DISPLAY, configureEvent->window, wmGD.windowContextType,
	    (XPointer *)&pCD))
    {
	/*
	 * Get window attribute information; this is used later on
	 * to decide if a synthetic ConfigureNotify event should
	 * be send to the client.
	 */

	if (WmGetWindowAttributes (configureEvent->window))
	{
	    configChanged =
		(wmGD.windowAttributes.x != configureEvent->x) ||
		(wmGD.windowAttributes.y != configureEvent->y) ||
		(wmGD.windowAttributes.width != configureEvent->width) ||
		(wmGD.windowAttributes.height != configureEvent->height) ||
		(wmGD.windowAttributes.border_width !=
				       configureEvent->border_width) ||
		(configureEvent->value_mask & (CWSibling|CWStackMode));

            /*
             * The window is not (yet) managed.  Do the window
	     * configuration.
             */

	    if (configChanged)
	    {
	        values.x = configureEvent->x;
	        values.y = configureEvent->y;
	        values.width = configureEvent->width;
	        values.height = configureEvent->height;
	        values.border_width = configureEvent->border_width;
	        values.sibling = configureEvent->above;
	        values.stack_mode = configureEvent->detail;
	        XConfigureWindow (DISPLAY, configureEvent->window,
	            (unsigned int) (configureEvent->value_mask), &values);
	    }

	    /*
	     * Some clients expect a ConfigureNotify event even if the
	     * XConfigureWindow call has NO effect.  Send a synthetic
	     * ConfigureNotify event just to be sure.
	     */

	    if (!configChanged)
	    {
	        notifyEvent.type = ConfigureNotify;
	        notifyEvent.display = DISPLAY;
	        notifyEvent.event = configureEvent->window;
	        notifyEvent.window = configureEvent->window;
	        notifyEvent.x = configureEvent->x;
	        notifyEvent.y = configureEvent->y;
	        notifyEvent.width = configureEvent->width;
	        notifyEvent.height = configureEvent->height;
	        notifyEvent.border_width = configureEvent->border_width;
	        notifyEvent.above = None;
	        notifyEvent.override_redirect = False;

	        XSendEvent (DISPLAY, configureEvent->window, False,
	            StructureNotifyMask, (XEvent *)&notifyEvent);
            }
        }
    }
    else
    {
        /*
         * The context information on the window WAS found.
         * The window is already managed by the window manager
         * so this is a configuration request that was made before
         * the window was reparented.
         */

	HandleCConfigureRequest (pCD, configureEvent);
    }

} /* END OF FUNCTION HandleWsConfigureRequest */



/*************************************<->*************************************
 *
 *  HandleWsFocusIn (focusEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function processes FocusIn events that are reported to the root
 *  window.
 *
 *
 *  Inputs:
 *  ------
 *  focusEvent = pointer to a focus in event on the root window.
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void HandleWsFocusIn (focusEvent)
    XFocusInEvent *focusEvent;

#else /* _NO_PROTO */
void HandleWsFocusIn (XFocusInEvent *focusEvent)
#endif /* _NO_PROTO */
{
    ClientData *pCD;
    Boolean sameScreen;

    /*
     * This code is used to handle the case of the focus being
     * set to pointer root (either explicitly by some client, by the window
     * manager or as a result of a "revert to" action).
     * It also handles the case where the focus is manipulated by a window
     * manager on another screen (in this case let the other window manager
     * control the focus). Reset the focus to a client window if appropriate.
     */

    if (((focusEvent->mode == NotifyNormal) ||
	 (focusEvent->mode == NotifyUngrab)) &&
	((focusEvent->detail == NotifyPointerRoot) ||
	 (focusEvent->detail == NotifyDetailNone) ||
	 (focusEvent->detail == NotifyInferior)))
    {
	/*
	 * Fix the keyboard focus if it should be set to a particular client.
	 */

        pCD = GetClientUnderPointer (&sameScreen);
	if (wmGD.keyboardFocus && (focusEvent->detail != NotifyInferior))
	{
	    if (sameScreen)
	    {
		/*
		 * Assume that the focus still belongs to the screen
		 * controlled by mwm.  Repair the focus if the client
		 * is still active.
		 */

		if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
		{
		    Do_Focus_Key (wmGD.keyboardFocus, GetTimestamp (),
			ALWAYS_SET_FOCUS);
		}
		else
		{
		    if (pCD || (focusEvent->detail == NotifyDetailNone))
		    {
			/* !!! check for redundant focus setting !!! */
	        	Do_Focus_Key (pCD, GetTimestamp (), ALWAYS_SET_FOCUS);
		    }
		}
		SetKeyboardFocus ((ClientData *) NULL, REFRESH_LAST_FOCUS);
	    }
	    else
	    {
	        /*
		 * Assume that the focus is now controlled by a
		 * window manager on another screen.  Clear the
		 * focus locally.
	         */

		SetKeyboardFocus ((ClientData *) NULL, REFRESH_LAST_FOCUS);
	    }
        }
        else
        {
	    /*
	     * No client window currently has the focus.  If the pointer
	     * is on the mwm-controlled screen set the focus to
	     * the window management window if the focus is explicit.
	     */

	    if (sameScreen)
	    {
		if (wmGD.keyboardFocusPolicy == KEYBOARD_FOCUS_EXPLICIT)
		{
		    if ((focusEvent->detail == NotifyInferior) &&
			(wmGD.keyboardFocus != wmGD.nextKeyboardFocus))
		    {
			/*
			 * Window that had the focus went away.  Try to
			 * reset the window to the next keyboard focus
			 * client window if there is one.
			 */

		        Do_Focus_Key (wmGD.nextKeyboardFocus, GetTimestamp (),
			    ALWAYS_SET_FOCUS);
		    }
		    else
		    {
		        /* Re: CR 4896                                          */
                        /* The previous version would pass NULL widget to this  */
                        /* this routine.  This doesn't seem to make sense. NULL */
                        /* has been replaced by pCD which seems to fix the icon */
                        /* focus problem.                                       */
                        /* Another related patch is made in WmCEvent.c.         */
		        Do_Focus_Key ((ClientData *) pCD, CurrentTime, 
					ALWAYS_SET_FOCUS);
		    }
		}
		else /*KEYBOARD_FOCUS_POINTER*/
		{
		    if (pCD || focusEvent->detail != NotifyPointerRoot)
		    {
		        Do_Focus_Key (pCD, GetTimestamp (), ALWAYS_SET_FOCUS);
		    }
		}
	    }
        }
    }

} /* END OF FUNCTION HandleWsFocusIn */



/*************************************<->*************************************
 *
 *  GetTimestamp ()
 *
 *
 *  Description:
 *  -----------
 *  This function is used to provide a timestamp for use with X calls that
 *  require a timestamp (and a timestamp is not available from a prior
 *  X event).
 *
 *
 *  Outputs:
 *  -------
 *  Return = a timestamp value
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
Time GetTimestamp ()

#else /* _NO_PROTO */
Time GetTimestamp (void)
#endif /* _NO_PROTO */
{
    /* 
     * !!! get a timestamp ...						!!!
     * !!! do a 0-len append to some wm property and get the event from !!!
     * !!! the property notify						!!!
     */

    return (CurrentTime);

} /* END OF FUNCTION GetTimestamp */



/*************************************<->*************************************
 *
 *  PullExposureEvents ()
 *
 *
 *  Description:
 *  -----------
 *  Pull in and process all outstanding exposure events 
 *
 *
 *  Inputs:
 *  ------
 * 
 *  Outputs:
 *  -------
 *
 *  Comments:
 *  --------
 *  Useful for cleaning up display after menu popdown
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
void PullExposureEvents ()
#else /* _NO_PROTO */
void PullExposureEvents (void)
#endif /* _NO_PROTO */
{
    XEvent	event;
    Boolean	dispatchEvent;

    /* 
     * Force the exposure events into the queue
     */
    XSync (DISPLAY, False);

    /*
     * Selectively extract the exposure events
     */
    while (XCheckMaskEvent (DISPLAY, ExposureMask, &event))

    {
        /*
	 * Check for, and process non-widget events.  The events may be
	 * reported to the root window, to some client frame window,
	 * to an icon window, or to a "special" window management window.
	 */

	if (event.xany.window == ACTIVE_ROOT)
	{
	    dispatchEvent = WmDispatchWsEvent (&event);
	}
	else
	{
	    dispatchEvent = WmDispatchClientEvent (&event);
	}

	if (dispatchEvent)
	{
	    /*
	     * Dispatch widget related event:
	     */

	    XtDispatchEvent (&event);
	}
    }

#ifdef DEC_MOTIF_EXTENSION
    /* Check the mwm dialog boxes as well. */
    if ( wmGD.dialog_display != NULL )
        while ( XCheckMaskEvent( wmGD.dialog_display, ExposureMask, &event ))
          {  
            if (event.xany.window == ACTIVE_ROOT)
                dispatchEvent = WmDispatchWsEvent (&event);
            else dispatchEvent = WmDispatchClientEvent (&event);
	    if (dispatchEvent)
                XtDispatchEvent (&event);
          }      
#endif /* DEC_MOTIF_EXTENSION */

} /* END OF FUNCTION PullExposureEvents */

