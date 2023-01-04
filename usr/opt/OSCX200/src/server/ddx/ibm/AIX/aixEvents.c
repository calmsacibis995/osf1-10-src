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
 * $XConsortium: aixEvents.c,v 1.3 91/07/16 12:59:36 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#include <sys/hft.h>

#define NEED_EVENTS
#define NEED_REPLIES
#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "aixInput.h"
#include "input.h"
#include "inputstr.h"

#include "compiler.h"

#include "ibmIO.h"
#include "ibmMouse.h"
#include "ibmKeybd.h"
#include "ibmScreen.h"
#include "ibmTrace.h"

#include "OSio.h"
#include "hftQueue.h"

#ifdef AIXEXTENSIONS

#include "AIX.h"
#include "AIXproto.h"
#include "AIXext.h"

#ifdef CFBSERVER
extern DevicePtr rtDial ;
extern DevicePtr rtLpfk ;
#endif

#ifndef CFBSERVER /* come frome aixExtHook */
AIXInfoRec      aixInfo  = {0,                          /* kbdid */
	                    0,                          /* display vrmid */
	                    0,                          /* display model */
	                    0,                          /* autoloadmode */
	                   -1,                          /* fd  */
	                    0,                          /* kbd iodn */
	                    0,                          /* loc iodn */
#ifdef AIXTABLET
	                    0,                          /* mouse iodn */
	                    0,                          /* tablet iodn */
#endif AIXTABLET
	                    0,                          /* dial iodn */
	                    0,                          /* lpfk iodn */
	                    0,                          /* loctype */
	                   };
#endif

#ifdef AIXTABLET
extern DevicePtr rtTablet ;
#endif AIXTABLET

#endif

#ifdef  XTESTEXT1
/*
 * device ID defines
 */
#define XE_MOUSE 1 /* mouse */
#define XE_DKB 2 /* main keyboard */

extern KeyCode xtest_command_key;

#define  XTestSERVER_SIDE
#include "xtestext1.h"
extern Bool     XTestStealKeyData();
/*
 * defined in xtestext1di.c
 */
extern int      on_steal_input;         /* steal input mode is on.      */
extern int      exclusive_steal;

/*
 * defined in xtestext1di.c
 */
extern short    xtest_mousex;
/*
 * defined in xtestext1di.c
 */
extern short    xtest_mousey;

#endif  /* XTESTEXT1 */

extern  int      screenIsSaved;
extern  int      kbdType ;

static  hftEvent delayedEvent;
static  int      delayed_left;
static  int      delayed_right;
static  int      delayed_middle;
static  unsigned char lastButtons = 0x00 ;
static  short    lastModKeys;
static  int      pendingX;
static  int      pendingY;

static int  kanjiCapsLockOn = 0;

#define GET_OS_TIME() (GetTimeInMillis())

extern void aixFlushMouse();
extern int aixPtrEvent() ;
extern int aixKbdEvent() ;
extern int aixTabletEvent();
extern int aixDialEvent();
extern int aixLpfkEvent();
extern void     ibmReactivateScreens(), ibmDeactivateScreens();

void
AIXInitEventHandlers()
{
    TRACE(("InitEventHandlers()\n"));

#ifdef XTESTEXT1
    xtest_command_key = 0x78;   /* F1 key */
#endif /* XTESTEXT1 */

    if (hftInstallHandler(HFT_LOCATOR,aixPtrEvent)==HFT_ERROR) {
	ErrorF("Couldn't install mouse handler\n");
    }
    if (hftInstallHandler(HFT_KEYBOARD,aixKbdEvent)==HFT_ERROR) {
	ErrorF("Couldn't install keyboard handler\n");
    }
    hftInstallHandler(HFT_EVENT_ARRIVED,HFT_IGNORE);
    if (hftInstallHandler(HFT_GRANTED,ibmReactivateScreens)==HFT_ERROR) {
	ErrorF("Couldn't install grant routine\n");
    }
    if (hftInstallHandler(HFT_RETRACTED,ibmDeactivateScreens)==HFT_ERROR) {
	ErrorF("Couldn't install retract handler\n");
    }
#ifdef AIXEXTENSIONS
    if (hftInstallHandler(HFT_TABLET,aixTabletEvent)==HFT_ERROR) {
	ErrorF("Couldn't install tablet handler\n");
    }
    if (hftInstallHandler(HFT_DIAL,aixDialEvent)==HFT_ERROR) {
	ErrorF("Couldn't install dial handler\n");
    }
    if (hftInstallHandler(HFT_LPFK,aixLpfkEvent)==HFT_ERROR) {
	ErrorF("Couldn't install lpfk handler\n");
    }
#endif
}

void
ProcessInputEvents()
{

#ifdef IBM_SPECIAL_MALLOC
extern int ibmShouldDumpArena;

    if (ibmShouldDumpArena) {
	ibmDumpArena();
    }
#endif /* IBM_SPECIAL_MALLOC */

    TRACE(("ProcessInputEvents (pending=%d)\n",hftPending));

    if ( screenIsSaved == SCREEN_SAVER_ON ) {
	SaveScreens( SCREEN_SAVER_OFF, ScreenSaverReset );
    }
    hftDispatchEvents();
    if (pendingX||pendingY)
	aixFlushMouse();
}

static void
aixFlushMouse()
{
register        int     x,y;
register        ibmPerScreenInfo        *screenInfo;
	        int     oldScr, newScr, v;
	        int     setCursor;
	        xEvent  e;

    TRACE(("aixFlushMouse\n"));

    if ((pendingX)||(pendingY)) {
	ibmAccelerate(pendingX,pendingY);
	x= AIXCurrentX+pendingX;
	y= AIXCurrentY-pendingY;

	pendingX= 0;
	pendingY= 0;

	oldScr = ibmCurrentScreen;
	newScr = ibmCurrentScreen;
	screenInfo = ibmScreens[ oldScr ];
	setCursor = FALSE;

	while( y<0 || x<0 || y>screenInfo->ibm_ScreenBounds.y2 || x>screenInfo->ibm_ScreenBounds.x2 ) {
	    if( y < 0 ){
	        if( (v=aixWrapUp(newScr)) < 0 ){
	                /* if no wrap this direction */
	            y= 0;
	            setCursor= TRUE;
	        }
	        else{
	            newScr = v;
	            screenInfo = ibmScreens[newScr];
	            y+= screenInfo->ibm_ScreenBounds.y2;
	            setCursor= TRUE;
	        }
	    }
	    else if( y>screenInfo->ibm_ScreenBounds.y2 ) {
	        if( (v=aixWrapDown(newScr)) < 0 ){
	                /* if no wrap this direction */
	            y= screenInfo->ibm_ScreenBounds.y2;
	            setCursor= TRUE;
	        }
	        else{
	            newScr = v;
	            screenInfo = ibmScreens[newScr];
	            y-= screenInfo->ibm_ScreenBounds.y2;
	            setCursor= TRUE;
	        }
	    }

	    if( x < 0 ){
	        if( (v=aixWrapLeft(newScr)) < 0 ){
	                /* if no wrap this direction */
	            x= 0;
	            setCursor= TRUE;
	        }
	        else{
	            newScr = v;
	            screenInfo = ibmScreens[newScr];
	            x+= screenInfo->ibm_ScreenBounds.x2;
	            setCursor= TRUE;
	        }
	    }
	    else if( x>screenInfo->ibm_ScreenBounds.x2 ) {
	        if( (v=aixWrapRight(newScr)) < 0 ){
	                /* if no wrap this direction */
	            x= screenInfo->ibm_ScreenBounds.x2;
	            setCursor= TRUE;
	        }
	        else{
	            newScr = v;
	            screenInfo = ibmScreens[newScr];
	            x-= screenInfo->ibm_ScreenBounds.x2;
	            setCursor= TRUE;
	        }
	    }
	}

	if (setCursor)
	    setCursorPosition(x,y);

	if (oldScr != newScr ) {
	TRACE(("switching screens in ProcessInputEvents\n"));
	NewCurrentScreen( screenInfo->ibm_Screen, x, y ) ;
	}

#ifdef XTESTEXT1
	if (on_steal_input)
	{
	        /*
	         * only call if the mouse position has actually moved
	         */
	        if ((x != xtest_mousex) || (y != xtest_mousey))
	        {
	                XTestStealMotionData((x - xtest_mousex),
	                                     (y - xtest_mousey),
	                                     XE_MOUSE,
	                                     xtest_mousex,
	                                     xtest_mousey);
	        }
	}
#endif /* XTESTEXT1 */

#ifdef SOFTWARE_CURSOR
	miPointerMoveCursor(screenInfo->ibm_Screen, x, y, TRUE) ;
#else
	(* screenInfo->ibm_CursorShow )( x, y ) ;
#endif

	e.u.u.type=                     MotionNotify;
	e.u.keyButtonPointer.rootX=     AIXCurrentX= x;
	e.u.keyButtonPointer.rootY=     AIXCurrentY= y;
	e.u.keyButtonPointer.time=      lastEventTime = GET_OS_TIME();
	pendingX= pendingY= 0;
	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
    }
}

/***====================================================================***/

	/*
	 * All of this keyboard stuff needs to
	 * be reorganized anyway (into osKeymap.h and a single keyboard.c)
	 */


#define LSHIFT  (HFUXLSH)
#define RSHIFT  (HFUXRSH)
#define RALT    (HFUXRALT)
#define LALT    (HFUXLALT)

#define CTRL    0

#define MODKEYMASK      (LSHIFT|RSHIFT|RALT|LALT|CTRL)
#define setModKeyState(ms,pe)   \
	        ((ms)=((((pe)->keStatus[0]<<8)|(pe)->keStatus[1])&MODKEYMASK))

#ifdef XTESTEXT1
#define FAKEEVENT(ev,key,up)    {\
	(ev)->u.u.detail= (key);\
	(ev)->u.u.type= ((up)?KeyRelease:KeyPress);\
	if (!on_steal_input || \
	    XTestStealKeyData((ev)->u.u.detail, (ev)->u.u.type, XE_DKB, \
	                      AIXCurrentX, AIXCurrentY)) \
	(*ibmKeybd->processInputProc)((ev),ibmKeybd,1);\
	}
#elif defined(SOFTWARE_CURSOR)
#define FAKEEVENT(ev,key,up)    {\
	(ev)->u.u.detail= (key);\
	(ev)->u.u.type= ((up)?KeyRelease:KeyPress);\
	miPointerPosition(ibmScreens[ibmCurrentScreen]->ibm_Screen,\
	                  &((ev)->u.keyButtonPointer.rootX),       \
	                  &((ev)->u.keyButtonPointer.rootY)         \
	                  ) ;                                       \
	(*ibmKeybd->processInputProc)((ev),ibmKeybd,1);\
	}
#else /* HARDWARE_CURSOR */
#define FAKEEVENT(ev,key,up)    {\
	(ev)->u.u.detail= (key);\
	(ev)->u.u.type= ((up)?KeyRelease:KeyPress);\
	(*ibmKeybd->processInputProc)((ev),ibmKeybd,1);\
	}
#endif /* XTESTEXT1 */

void
aixFakeModKeyEvent(changedModKeys)
unsigned short  changedModKeys;
{
xEvent          e;

    TRACE(("aixFakeModKeyEvent(%d)\n"));

    e.u.keyButtonPointer.rootX= AIXCurrentX;
    e.u.keyButtonPointer.rootY= AIXCurrentY;
    e.u.keyButtonPointer.time=  lastEventTime= GET_OS_TIME();

    if (changedModKeys&LSHIFT) {
	FAKEEVENT(&e,Aix_Shift_L,(lastModKeys&LSHIFT));
    }
    if (changedModKeys&RSHIFT) {
	FAKEEVENT(&e,Aix_Shift_R,(lastModKeys&RSHIFT));
    }
    if (changedModKeys&LALT) {
	FAKEEVENT(&e,Aix_Alt_L,(lastModKeys&LALT));
    }
    if (changedModKeys&RALT) {
	FAKEEVENT(&e,Aix_Alt_R,(lastModKeys&RALT));
    }

    return;

}

/***====================================================================***/

static int
aixKbdEvent(pEv)
hftEvent        *pEv;
{
hftKeyEv        *pKey= &pEv->hftEv.hftKey;
unsigned short  theseModKeys;
DeviceIntPtr    KeyboardPtr;
xEvent          e;
int             key;

    TRACE(("aixKbdEvent (%d)\n",pKey->kePos));

    key = pKey->kePos + AIX_MIN_KEY ;

    if ((pendingX)||(pendingY))
	aixFlushMouse();

    setModKeyState(theseModKeys,pKey);
    if (theseModKeys!=lastModKeys) {
	aixFakeModKeyEvent(theseModKeys^lastModKeys);
    }

    lastModKeys= theseModKeys;

    switch (key) {
	case Aix_Alt_L:
	        {
	        if (kbdType == HF106KBD)        /* special case */
	                break ;
	        else
	                return(1);
	        }
	case Aix_Shift_L:
	case Aix_Shift_R:
	case Aix_Alt_R:
	                return(1);
	case Aix_Backspace:
	        if ((!ibmDontZap)&&(pKey->keStatus[0]&HFUXCTRL)&&
	                                (pKey->keStatus[0]&HFUXALT)) {
	            GiveUp();
	        }
	        break;
	default:
	        break;
    }

    if ((!ibmKeyRepeat)&&pKey->keStatus[1]&HFUXRPT)
	return(1);

    e.u.keyButtonPointer.rootX= AIXCurrentX;
    e.u.keyButtonPointer.rootY= AIXCurrentY;
    e.u.keyButtonPointer.time=  lastEventTime= GET_OS_TIME();
    e.u.u.detail= key ;
    if (pKey->keStatus[1]&HFUXRPT)      {
	e.u.u.type= KeyRelease;
#ifdef XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_DKB,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
#ifdef SOFTWARE_CURSOR
	miPointerPosition(ibmScreens[ibmCurrentScreen]->ibm_Screen,
	                  &e.u.keyButtonPointer.rootX,
	                  &e.u.keyButtonPointer.rootY
	                  ) ;
#endif
	(*ibmKeybd->processInputProc)(&e,ibmKeybd,1);
	e.u.u.type= KeyPress;
    }
    else if (pKey->keStatus[0]&HFUXMAKE)        e.u.u.type= KeyPress;
    else                                        e.u.u.type= KeyRelease;

    if (kbdType == HF106KBD) {
	/*
	        On 106 keyboard the Caps Lock key is in the same
	        place as the Left Alt key and is only active when
	        the Right Alt key is down.  To toggle Caps Lock type
	        the sequence Right_Alt down, Left_Alt down, then let
	        the keys up in any order.  The Left_Alt acts like a
	        normal key when Right_Alt is up.
	*/

    KeyboardPtr = (DeviceIntPtr)ibmKeybd;

    if( (key == Aix_Alt_L) && (e.u.u.type == KeyPress) )
    {
	/* if left alt key was just pressed */
	/* if right alt key is down */
	if( theseModKeys & RALT ){
	        if( kanjiCapsLockOn ){
	                /* turn OFF caps lock */
#ifdef CFBSERVER
/* where is is defined ??? */
	            kanjiClearCapsLock();
	            kanjiCapsLockOn = 0;
#endif
	            SetCapsLockLED(0);
	        }
	        else{
	                /* turn ON caps lock */
#ifdef CFBSERVER
	            kanjiSetCapsLock();
	            kanjiCapsLockOn = 1;
#endif
	            SetCapsLockLED(1);
	        }
	}
    }

    /*
     *  toggle num lock key:
     *  ignore releases, toggle on & off on presses
     */

    if( (KeyboardPtr)->key->modifierMap[key] & NumLockMask )
    {
       if( e.u.u.type == KeyRelease )
	        return (1);
       if( KeyboardPtr->key->down[key >> 3] & (1 << (key & 7)) )
       {
	    e.u.u.type = KeyRelease;
	    SetNumLockLED(0);
       } else
	    SetNumLockLED(1);
    }

    } /* end if (kbdType == HF106KBD) */

    if (kbdType == HF101KBD || kbdType == HF102KBD) {

    /*
     *  toggle lock shift keys:
     *  ignore releases, toggle on & off on presses
     */

    KeyboardPtr = (DeviceIntPtr)ibmKeybd;

    if( KeyboardPtr->key->modifierMap[key] & LockMask )
    {
	if( e.u.u.type == KeyRelease )
	     return (1);
	if( KeyboardPtr->key->down[key >> 3] & (1 << (key & 7)) )
	{
	     e.u.u.type = KeyRelease;
	     SetCapsLockLED(0);
	} else
	     SetCapsLockLED(1);
    }

    /*
     *  toggle num lock key:
     *  ignore releases, toggle on & off on presses
     */

    if( KeyboardPtr->key->modifierMap[key] & NumLockMask )
    {
       if( e.u.u.type == KeyRelease )
	        return (1);
       if( KeyboardPtr->key->down[key >> 3] & (1 << (key & 7)) )
       {
	    e.u.u.type = KeyRelease;
	    SetNumLockLED(0);
       } else
	    SetNumLockLED(1);
    }

    }

#ifdef XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_DKB,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */

#ifdef SOFTWARE_CURSOR
	miPointerPosition(ibmScreens[ibmCurrentScreen]->ibm_Screen,
	                  &(e.u.keyButtonPointer.rootX),
	                  &(e.u.keyButtonPointer.rootY)
	                  ) ;
#endif
    (*ibmKeybd->processInputProc)(&e,ibmKeybd,1);

    return(1);
}

/***====================================================================***/

#define NONE    0x00
#define LEFT    Button1
#define MIDDLE  Button2
#define RIGHT   Button3

#define UP      ButtonRelease
#define DOWN    ButtonPress

static int aix3ButtonPtrEvent();


static void
locatorTimeout()
{
aixPtrEvent(&delayedEvent);
return;
}


static int
aixPtrEvent(pIn)
hftEvent        *pIn;
{
extern  int     AIXMouseChordDelay;
hftLocEv        *pEv= &pIn->hftEv.hftLoc;
unsigned char   buttons= pEv->leButtons&(HFT_BUTTONS|HFT_MBUTTON);
xEvent          e;


    TRACE(("aixPtrEvent (%d,%d)\n",pEv->leDeltaX,pEv->leDeltaY));
    pendingX+= pEv->leDeltaX;
    pendingY+= pEv->leDeltaY;

    if (pendingX||pendingY)
	aixFlushMouse();
    e.u.keyButtonPointer.rootX=     AIXCurrentX;
    e.u.keyButtonPointer.rootY=     AIXCurrentY;
    e.u.keyButtonPointer.time=      GET_OS_TIME();

    if (buttons&HFT_MBUTTON) {
    /* We don't care about all the 2 button stuff anymore because
       we know there is a three button mouse.  Therefore, LEFT&RIGHT
       buttons down don't mean middle button( like they had to for a two
       button mouse), they mean left and right button down consecutively.*/
	if (delayed_right || (buttons&HFT_RBUTTON) ) {
	    hftAddTimeout(NULL,0);
	    e.u.u.detail= RIGHT;
	    e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	    (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	}
	if (delayed_left || (buttons&HFT_LBUTTON) ) {
	    hftAddTimeout(NULL,0);
	    e.u.u.detail= LEFT;
	    e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	    (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	}
	/* if we had previously generated a middle event from a LEFT&RIGHT
	   and we are waiting for it to go away */
	if (delayed_middle) {
	    hftAddTimeout(NULL,0);
	    e.u.u.detail= MIDDLE;
	    e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	    (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	}
	e.u.u.detail= MIDDLE;
	e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
	lastButtons = buttons;
	if (hftInstallHandler(HFT_LOCATOR,aix3ButtonPtrEvent)==HFT_ERROR)
	    ErrorF("Couldn't install three button mouse handler\n");
	return(1);
    }


    switch(lastButtons)
    {
	case NONE  :
	     switch(buttons)
	     {
	         case NONE  :
	              break;
	         case HFT_LBUTTON  :
	              delayed_left = TRUE;
	              break;
	         case HFT_RBUTTON  :
	              delayed_right = TRUE;
	              break;
	         case HFT_BUTTONS  :
	              e.u.u.detail= MIDDLE;
	              e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	              (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              break;
	      }
	     break;
	case HFT_LBUTTON  :
	     switch(buttons)
	     {
	         case NONE  :
	              if (delayed_left) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_left = FALSE;
	              }
	              if (delayed_middle) {
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_middle = FALSE;
	              } else {
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              }
	              break;
	         case HFT_LBUTTON  :
	              if (delayed_left) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_left = FALSE;
	              }
	              break;
	         case HFT_RBUTTON  :
	              if (delayed_left) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_left = FALSE;
	              }
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              delayed_right = TRUE;
	              break;
	         case HFT_BUTTONS  :
	              if (delayed_left) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_left = FALSE;
	              } else if (!delayed_middle) {
	                  e.u.u.detail= LEFT;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              }
	              break;
	      }
	     break;
	case HFT_RBUTTON  :
	     switch(buttons)
	     {
	         case NONE  :
	              if (delayed_right) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= RIGHT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_right = FALSE;
	              }
	              if (delayed_middle) {
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_middle = FALSE;
	              } else {
	                  e.u.u.detail= RIGHT;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              }
	              break;
	         case HFT_LBUTTON  :
	              if (delayed_right) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= RIGHT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_right = FALSE;
	              }
	              e.u.u.detail= RIGHT;
	              e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	              (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              delayed_left = TRUE;
	              break;
	         case HFT_RBUTTON  :
	              if (delayed_right) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= RIGHT;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_right = FALSE;
	              }
	              break;
	         case HFT_BUTTONS  :
	              if (delayed_right) {
	                  hftAddTimeout(NULL,0);
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  delayed_right = FALSE;
	              } else if (!delayed_middle) {
	                  e.u.u.detail= RIGHT;
	                  e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	                  e.u.u.detail= MIDDLE;
	                  e.u.u.type=   DOWN;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	                  (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              }
	              break;
	      }
	     break;
	case HFT_BUTTONS  :
	     switch(buttons)
	     {
	         case NONE  :
	              e.u.u.detail= MIDDLE;
	              e.u.u.type=   UP;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	              (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	              break;
	         case HFT_LBUTTON  :
	         case HFT_RBUTTON  :
	              delayed_middle = TRUE;
	              break;
	         case HFT_BUTTONS  :
	              break;
	      }
	      break;
    }

    lastButtons = buttons;

    if (delayed_left || delayed_right) {
	delayedEvent = *pIn;
	delayedEvent.hftEv.hftLoc.leDeltaX = 0;
	delayedEvent.hftEv.hftLoc.leDeltaY = 0;
	hftAddTimeout(locatorTimeout,AIXMouseChordDelay);
    }
    return(1);
}



/***====================================================================***/

static int
aix3ButtonPtrEvent(pIn)
hftEvent        *pIn;
{
extern  int     AIXMouseChordDelay;
hftLocEv        *pEv= &pIn->hftEv.hftLoc;
unsigned char   buttons= pEv->leButtons&(HFT_BUTTONS|HFT_MBUTTON);
unsigned char   changed;
xEvent          e;

    TRACE(("aix3ButtonPtrEvent (%d,%d)\n",pEv->leDeltaX,pEv->leDeltaY));
    pendingX+= pEv->leDeltaX;
    pendingY+= pEv->leDeltaY;

    if (pendingX||pendingY)
	aixFlushMouse();

    e.u.keyButtonPointer.rootX=     AIXCurrentX;
    e.u.keyButtonPointer.rootY=     AIXCurrentY;
    e.u.keyButtonPointer.time=      GET_OS_TIME();

    changed = buttons ^ lastButtons;

    if (changed & HFT_BUTTON1) {
	if (buttons & HFT_BUTTON1)      e.u.u.type = DOWN;
	    else                        e.u.u.type = UP;
	e.u.u.detail= LEFT;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
    }
    if (changed & HFT_BUTTON3) {
	if (buttons & HFT_BUTTON3)      e.u.u.type = DOWN;
	    else                        e.u.u.type = UP;
	e.u.u.detail= RIGHT;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
    }
    if (changed & HFT_BUTTON2) {
	if (buttons & HFT_BUTTON2)      e.u.u.type = DOWN;
	    else                        e.u.u.type = UP;
	e.u.u.detail= MIDDLE;
#ifdef  XTESTEXT1
	if (!on_steal_input ||
	    XTestStealKeyData(e.u.u.detail, e.u.u.type, XE_MOUSE,
	                      AIXCurrentX, AIXCurrentY))
#endif /* XTESTEXT1 */
	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
    }

    lastButtons = buttons;
    return(1);
}


/*
 * Additional test extension functions.
 */
#ifdef XTESTEXT1
void
XTestGenerateEvent(dev_type, keycode, keystate, mousex, mousey)
	int     dev_type;
	int     keycode;
	int     keystate;
	int     mousex;
	int     mousey;
{
	xEvent  e;
	TRACE(("XTestGenerateEvent#1\n"));

	if (pendingX||pendingY)
	    aixFlushMouse();
	/*
	 * the server expects to have the x and y position of the locator
	 * when the action happened placed in other_p[XPOINTER]
	 */
	if (dev_type == XE_MOUSE)
	{
	        if (keystate == XTestKEY_UP)
	                keystate = ButtonRelease;
	        else
	                keystate = ButtonPress;
	        e.u.keyButtonPointer.rootX= AIXCurrentX = mousex;
	        e.u.keyButtonPointer.rootY= AIXCurrentY = mousey;
	        e.u.keyButtonPointer.time= lastEventTime = GET_OS_TIME();
	        e.u.u.detail=                   keycode;
	        e.u.u.type=                     keystate;
	        (*ibmPtr->processInputProc)(&e,ibmPtr,1);
	}
	else  /* keyboard events */
	{
	        if (keystate == XTestKEY_UP)
	                keystate = KeyRelease;
	        else
	                keystate = KeyPress;
	        e.u.keyButtonPointer.rootX= AIXCurrentX = mousex;
	        e.u.keyButtonPointer.rootY= AIXCurrentY = mousey;
	        e.u.keyButtonPointer.time= lastEventTime = GET_OS_TIME();
	        e.u.u.detail=                   keycode;
	        e.u.u.type=                     keystate;
	        (*ibmKeybd->processInputProc)(&e,ibmKeybd,1);
	}

#ifdef notdef
	/*
	   ##### Here is the original HP code, the generated event is put in
	   ##### event queue.  Similar implementation might be needed
	   ##### in our AIX hft layer, but until we see XTM code, we'll
	   ##### have better idea.   -glee
	 */
	/*
	 * set the last event time so that the screen saver code will
	 * think that a key has been pressed
	 */
	lastEventTime = GetTimeInMillis();

	put_keyevent(keycode,
	             keystate,
	             0,
	             dev_type,
	             tmp_ptr,
	             &hil_info);

	ProcessInputEvents();
#endif

}


void
XTestGetPointerPos(fmousex, fmousey)
	short *fmousex, *fmousey;
{
	*fmousex = AIXCurrentX;
	*fmousey = AIXCurrentY;
}

/******************************************************************************
 *
 *      Tell the server to move the mouse.
 *
 */
void
XTestJumpPointer(jx, jy, dev_type)
/*
 * the x and y position to move the mouse to
 */
int     jx;
int     jy;
/*
 * which device is supposed to move (ignored)
 */
int     dev_type;
{
	xEvent e;
	TRACE(("XTestJumpPointer#1\n"));

	ProcessInputEvents();
	/*
	 * set the last event time so that the screen saver code will
	 * think that the mouse has been moved
	 */
	e.u.keyButtonPointer.time = lastEventTime = GET_OS_TIME();
	/*
	 * tell the server where the mouse is being moved to
	 */
	e.u.keyButtonPointer.rootX = AIXCurrentX = jx;
	e.u.keyButtonPointer.rootY = AIXCurrentY = jy;
	e.u.u.type = MotionNotify;

	(*ibmPtr->processInputProc)(&e,ibmPtr,1);
	(*(ibmCursorShow(ibmCurrentScreen)))(jx, jy);
}
#endif /* XTESTEXT1 */

#ifdef AIXEXTENSIONS

static int aixTabletSaveX = 1 ;
static int aixTabletSaveY = 1;
int aixTabletScaleX ;
int aixTabletScaleY ;

static void
aixFlushTablet(deltax,deltay)
int deltax,deltay;
{
register        int     x,y;
register        ibmPerScreenInfo        *screenInfo;
	        int     setCursor;
	        xEvent  e;

    TRACE(("aixFlushTablet(%d, %d)\n", deltax, deltay));

    screenInfo = ibmScreens[ ibmCurrentScreen ];
    setCursor = TRUE ;

#ifdef AIXV3
    /* save current values */
    aixTabletSaveX = deltax;
    aixTabletSaveY = deltay;

    /* deltax and deltay are really absolute tablet coordinates */
    x = deltax * screenInfo->ibm_ScreenBounds.x2 / aixTabletScaleX;
    y = deltay * screenInfo->ibm_ScreenBounds.y2 / aixTabletScaleY;
#else
    x = AIXCurrentX ;
    y = screenInfo->ibm_ScreenBounds.y2 - AIXCurrentY ;

    /*
     * Analize horizontal movement for releative coordinates
     */

    if ( deltax <= aixTabletSaveX )
	x =  deltax * x / aixTabletSaveX ;
    else if ((screenInfo->ibm_ScreenBounds.x2 * aixTabletScaleX) != aixTabletSaveX)
	x += (deltax - aixTabletSaveX) * (screenInfo->ibm_ScreenBounds.x2 - x) /
	     (screenInfo->ibm_ScreenBounds.x2 * aixTabletScaleX - aixTabletSaveX);

    if ( !deltax )
	aixTabletSaveX = 1 ;
    else
	aixTabletSaveX = deltax;

    /*
     * Analize vertical movement for releative coordinates
     */

    if ( deltay <= aixTabletSaveY )
       y = deltay * y / aixTabletSaveY ;
    else if ((screenInfo->ibm_ScreenBounds.y2 * aixTabletScaleY) != aixTabletSaveY)
       y += (deltay - aixTabletSaveY) * (screenInfo->ibm_ScreenBounds.y2 - y) /
	    (screenInfo->ibm_ScreenBounds.y2 * aixTabletScaleY - aixTabletSaveY);

    if ( !deltay)
	aixTabletSaveY = 1 ;
    else
	aixTabletSaveY = deltay ;
#endif

    y = screenInfo->ibm_ScreenBounds.y2 - y ;

    if ( y > screenInfo->ibm_ScreenBounds.y2)
	y = screenInfo->ibm_ScreenBounds.y2;

    if ( x > screenInfo->ibm_ScreenBounds.x2)
	x = screenInfo->ibm_ScreenBounds.x2;

    if ( y <= 0)
	y = 0;

    if ( x <= 0)
	x = 0;

    if (setCursor) {
	/* OS-DEPENDENT MACRO GOES HERE!!
	 * MACRO DEFINED IN FILE ibmos.h
	 * TELL OS THAT CURSOR HAS MOVED
	 * TO A NEW POSITION
	 */
	setCursorPosition(x,y);
    }

    (* screenInfo->ibm_CursorShow )( x, y ) ;

    AIXCurrentX= x ;
    AIXCurrentY= y ;
    pendingX= pendingY= 0;

}

/***====================================================================***/

static int
aixTabletEvent(pIn)
hftEvent        *pIn;
{
hftLocEv        *pEv= &pIn->hftEv.hftLoc;
unsigned char   tmpbuttons,buttons ;
xEvent          e;

#ifdef AIXTABLET

xExtEvent       xe;

	TRACE(("aixTabletEvent()\n"));
	TRACE(("aixTabletEvent: Event type = %d\n", pIn->hftEvType));
	TRACE(("aixTabletEvent: Buttons    = %d\n", pEv->leButtons));
	TRACE(("aixTabletEvent: DeltaX     = %d\n", pEv->leDeltaX));
	TRACE(("aixTabletEvent: DeltaY     = %d\n", pEv->leDeltaY));

	xe.u.Tablet.rootX = AIXCurrentX;
	xe.u.Tablet.rootY = AIXCurrentY;
	xe.u.Tablet.deviceid = DEVTABLET;
	xe.u.Tablet.axes_count = 2;             /* always 2 for tablet */
	xe.u.Tablet.first_axes = 0;             /* 0 - x  1 - y */
	xe.u.Tablet.axes_data[0] = pEv->leDeltaX;
	xe.u.Tablet.axes_data[1] = pEv->leDeltaY;
	lastEventTime = xe.u.Tablet.time = GET_OS_TIME();

	xe.u.u.type = DeviceMotion;
	xe.u.u.detail = pEv->leButtons & HFT_TABLET_BUTTONS;

	if (rtTablet->on)
	    (*(rtTablet->processInputProc))((xEvent *)&xe, rtTablet,1);

#else AIXTABLET

#ifdef AIXV3
	/* check if cursor has moved or different button is pressed */
	if ((aixTabletSaveX == pEv->leDeltaX) &&
	    (aixTabletSaveY == pEv->leDeltaY) &&
	    ((pEv->leButtons & HFT_TABLET_BUTTONS) == lastButtons))
	        return (1) ;

	buttons = pEv->leButtons & HFT_TABLET_BUTTONS ;
#else

	/* bug compatible */

	if (pEv->leButtons == 3 )
	        return ;

	buttons = pEv->leButtons >> 3 ;

	switch (buttons)
	{
	case  1 :
	        buttons = 0x80 ;
	        break ;
	case  2 :
	        buttons = 0x40 ;
	        break ;
	case  3 :
	        buttons = 0x20 ;
	        break ;
	case  4 :
	        buttons = 0x10 ;
	        break ;
	}
#endif

	if (lastButtons != (buttons & HFT_TABLET_BUTTONS)) {
	    if (lastButtons) {
	        e.u.u.type = ButtonRelease;
	        buttons = 0x00 ;
	        tmpbuttons = lastButtons;
	    } else {
	        e.u.u.type = ButtonPress;
	        tmpbuttons = buttons;
	    }

	    lastButtons = buttons & HFT_TABLET_BUTTONS ;

	    switch (tmpbuttons) {
	        case  HFT_BUTTON1 : e.u.u.detail = Button1;  break;
	        case  HFT_BUTTON2 : e.u.u.detail = Button2;  break;
	        case  HFT_BUTTON3 : e.u.u.detail = Button3;  break;
	        case  HFT_BUTTON4 : e.u.u.detail = Button4;  break;
	    }
	}
	else
	{
	    e.u.u.detail= 0 ;
	    e.u.u.type=  MotionNotify;
	}

	aixFlushTablet(pEv->leDeltaX,pEv->leDeltaY);

	e.u.keyButtonPointer.rootX=     AIXCurrentX;
	e.u.keyButtonPointer.rootY=     AIXCurrentY;
	e.u.keyButtonPointer.time=      GET_OS_TIME();

	(*(ibmPtr->processInputProc))(&e,ibmPtr,1);
#endif AIXTABLET

	return (1) ;
}

#endif AIXEXTENSIONS


#ifdef AIXEXTENSIONS
static int
aixDialEvent (pIn)
hftEvent        *pIn;
{
    xExtEvent   xe;
    hftDialEv   *pEv = &pIn->hftEv.hftDial;

    xe.u.DialLpfk.rootX =  AIXCurrentX;
    xe.u.DialLpfk.rootY =  AIXCurrentY;
    lastEventTime = xe.u.DialLpfk.time = GET_OS_TIME();

    xe.u.u.type = DialRotate;
    xe.u.u.detail = (char) pEv->deDialNo;
    xe.u.DialLpfk.value = (char) pEv->deDelta;

#ifdef CFBSERVER
    if (rtDial->on)
	(*(rtDial->processInputProc))((xEvent *)&xe, rtDial,1);
#endif

    return (1);
}


static int
aixLpfkEvent (pIn)
hftEvent        *pIn;
{
    xExtEvent   xe;
    hftLPFKEv   *pEv = &pIn->hftEv.hftLpfk;

    xe.u.DialLpfk.rootX =  AIXCurrentX;
    xe.u.DialLpfk.rootY =  AIXCurrentY;
    lastEventTime = xe.u.DialLpfk.time = GET_OS_TIME();

    xe.u.u.type = LPFKeyPress ;
    xe.u.u.detail = pEv->lpfkeKeyNo ;

#ifdef CFBSERVER
    if (rtLpfk->on)
	(*(rtLpfk->processInputProc))((xEvent *)&xe, rtLpfk,1);
#endif

    return (1);
}
#endif

