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
#ifdef MODE_SWITCH
/************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts.
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

************************************************************************/

#include <stdio.h>
#include <sys/types.h>

#define NEED_EVENTS
#include "misc.h"
#include "X.h"
#include "Xlib.h"
#include "Xproto.h"
#include "keysym.h"
#include "inputstr.h"
#include "keybind.h"
#include "xkmeproto_include.h"

extern InputInfo inputInfo;

unsigned short   DriverKeyBindSupport  = DRVKB_SUPPORT_INIT_VALUE;
short            KeyBindDefined = FALSE;
KeyBindLis       *pCurrentKeyBind = NULL;
KBModKey_roadmap *pKBModKey_roadmap = NULL;

static unsigned int KeymapModePropertyCreated = 0;

#ifndef VMS
#define SUCCESS 1
#define FAILURE 0

#define ONE_KEYCODE 1    /* One keycode in the key combination */
#define TWO_KEYCODE 2    /* Two keycode in the key combination */

/* internal state variables */
 
/* Key on off state of each modifier keys in the key combination 
 * [0] : 1st keycode of 1st key combinaton
 * [1] : 2nd keycode of 1st key combination (optional) 
 * [2] : 1st keycode of 2nd key combination
 * [3] : 2nd keycode of 2nd key combination (optional)  
 */ 
static Bool	 lckdownStat[4];
static Bool	 oneshotStat[4];
static Bool	 normalStat[4];

static int	 LckdownMask; 
static int	 OneshotMask;
static int       NormalMask;
static int	 activeMask;

typedef struct KeyBindDef *KeyBindDefPtr;
static KeyBindDefPtr pLckdownMod = NULL;
static KeyBindDefPtr pOneshotMod = NULL;
static KeyBindDefPtr pNormalMod  = NULL;


/* 
 * convenient macros used by ProcessKeyboardInput()
 *
 * EQU_MOD_KEY      : Equivalent modifier keycode. 
 * KEY_COMB_DEF_NUM : number of key combination definition.
 * KEY_NUM(i)       : number of keycode in the combination.
 * KEY(i,j)         : keycode value in the combination.
 */ 
#define EQU_MOD_KEY      kbdef$b_modkey
#define KEY_COMB_DEF_NUM kbdef$b_num_comb
#define KEY_NUM(i)       kbdef$r_keyComb[i].kbdef$b_combCharLength
#define KEY(i,j)	 kbdef$r_keyComb[i].kbdef$B_KC[j]

#define KEY_PRESS(k)	{ xevent->u.u.type = KeyPress; \
			  xevent->u.u.detail = k; \
			  (*pdev->processInputProc)(xevent, dev, 1); \
			}

#define KEY_RELEASE(k)	{ xevent->u.u.type = KeyRelease; \
			  xevent->u.u.detail = k; \
			  (*pdev->processInputProc)(xevent, dev, 1); \
			}

#define STATE_CLEAR	{ int i; \
			  for (i = 0; i < 4; i++) { \
			      lckdownStat[i] = FALSE; \
			      oneshotStat[i] = FALSE; \
			      normalStat[i]  = FALSE; \
			  } \
			}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**   
**	ProcessKeyboardInput -	UWS specific function covering some functions
**				of the VMS keyboard driver. Receive X events 
**				and send them to dispatcher.
**
**
**  FORMAL PARAMETERS:
**
**	xevent - X event
**	dev    - keyboard device
**
**  IMPLICIT INPUTS:
**
**      internal states
**
**  IMPLICIT OUTPUTS:
**
**      internal states
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**	This function controls the following three sets of internal states:
**
**		lckdownStat[]	- lock down key states
**		oneshotStat[]	- one shot key states
**		normalStat[]	- normal bind key states
**
**--
**/
void ProcessKeyboardInput(xevent, pdev)
xEvent *xevent;
DevicePtr pdev;
{
    KeyCode key;
    int i, j, type;
    DeviceIntPtr    dev   = (DeviceIntPtr)pdev;

    /* Key Bind Defined ? If no keybind definition , do nothing */ 
    if (!KeyBindDefined) { 
       (*pdev->processInputProc)(xevent, dev, 1);
       return;
    }

    /* This function is re-entrant, but does not send lock-down/one-shot/
     * normal binding keycodes twice.
     *
     * Valid sequences of lock-down/one-shot/normal binding keycodes are 
     * transformed to specified key press or release events.
     */
    key = xevent->u.u.detail;

     /* Is Lock Down Mofifier defined ? */
    if (pLckdownMod && (!activeMask || (activeMask & LckdownMask))){

	/* Ignore the equivalent modifier keycode receving the event.
	 * Because this event not receiving from keyboard driver was
         * created by this routine to support the keybinding definition 
         * for mode switch.
         */
	if (key == pLckdownMod->EQU_MOD_KEY) return;

	if (LckdownModKeyInput(xevent,pdev)) return;
    } 

    /* Is One Shot Modifier defined ? */
    if (pOneshotMod && (!activeMask || (activeMask & OneshotMask))){

        /* Ignore the equivalent modifier keycode receiving the evnet.*/
        if (key == pOneshotMod->EQU_MOD_KEY) return;

	if (OneshotModKeyInput(xevent,pdev)) return; 
    }

    /* Is Normal Modifier defined ? */
    if (pNormalMod && (!activeMask || (activeMask & NormalMask))){

	/* Ignore the equivalent modifier keycode receiving the evnet.*/
	if (key == pNormalMod->EQU_MOD_KEY) return; 

	if (NormalModKeyInput(xevent,pdev)) return; 
    }

    (*pdev->processInputProc)(xevent, dev, 1);
}

/*
**++
**
**  FUNCTIONAL DESCRIPTION: LckdownModKeyInput
**
**	    Lock-down modifier
**
**	If a key or key combination is bound as a "lock-down" modifier,
**	it serves as a "toggled" key. The Locking Shift key is an example
**	of the lock-down modifier. Press it once to lock the keyboard into
**	the Shift mode, and press it the second time to unlock the keyboard.
**	When the lock-down cycle is active, the modifer state for KeyPress
**	events will have the lock-down modifier bit marked.
**
**      For example if the key combination of ALT/Space is bound to the
**	mode-switch keycode 7A(hex) then the following input events can
**	be expected.
**
**	if the mode-switch state was OFF.
**
**			Physical Keybound State   Input Event Generated
**			-----------------------	  ---------------------
**		    		ALT key dwon	   KeyPress ALT keycode
**			       Space Key down     KeyRelease Alt keycode
**						   KeyPress 7A
**				ALT Key up
**				Space Key up
**
**	if the mode-switch state was already ON.
**
**			Physical Keybound State   Input Event Generated
**			-----------------------	  ---------------------
**		    		ALT key dwon	   KeyPress ALT keycode	
**			       Space Key down     KeyRelease Alt keycode
**					           KeyRelease 7A
**				ALR Key up
**				Space Key up
**
**  FORMAL PARAMETERS:
**
**	xevent - X event
**	dev    - keyboard device
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      internal states
**
**  FUNCTION VALUE:
**
**	0 : This routine do nothing. 
**	1 : transform the key event.
**
**
**  SIDE EFFECTS:
**
**	none
**
**--
*/
int LckdownModKeyInput(xevent,pdev)
xEvent *xevent;
DevicePtr pdev;
{
    int i,j;
    int type;
    KeyCode key;
    DeviceIntPtr    dev   = (DeviceIntPtr)pdev;

    key = xevent->u.u.detail;
    type = xevent->u.u.type;

    if(type == KeyPress){
        for (i = 0; i < pLckdownMod->KEY_COMB_DEF_NUM; i++) {

            switch (pLckdownMod->KEY_NUM(i)) {
	        case ONE_KEYCODE:
	            if (key != pLckdownMod->KEY(i,0))
		        break;

                    /* key is same as keycode of lock-down modifier
                     * key combination. Do the trasforming key
		     * event.
                     */
		     if (activeMask & LckdownMask) {
		         KEY_RELEASE(pLckdownMod->EQU_MOD_KEY);
		         activeMask &= ~LckdownMask;

			 /* Notice the Lock-down key release to
			  * the interest clients by Window Property.
			  */
                         HandleKeyboardModeChange(pLckdownMod->EQU_MOD_KEY,
						  0,
						  KeyRelease);
		     } else {
		         KEY_PRESS(pLckdownMod->EQU_MOD_KEY);
		         activeMask |= LckdownMask;
		     } 
		     return 1;

	        case TWO_KEYCODE:
	            for (j = 0; j < TWO_KEYCODE; j++) {
		        if (key != pLckdownMod->KEY(i,j))
		            continue;

                        /* The key is same as the one of keycodes of  
                         * lock-down modifer key combination.
                         * This keycode state already on ?
                         */
		        if (lckdownStat[i*2+j]) continue;

			lckdownStat[i*2+j] = TRUE;

		        if (lckdownStat[i*2-j+1]){
		        /* Two keycodes in the key combination 
		         * are already state ON.
		         */
		            KEY_RELEASE(pLckdownMod->KEY(i,1-j));
		            if (activeMask & LckdownMask) {
		                KEY_RELEASE(pLckdownMod->EQU_MOD_KEY);
			        activeMask &= ~LckdownMask;

				/* Notice the Lock-down key release to
				 * the interest clients by Window
				 * Property.
				 */
                                HandleKeyboardModeChange(
						    pLckdownMod->EQU_MOD_KEY,
						    0,
			            		    KeyRelease);
		            } else {
		                KEY_PRESS(pLckdownMod->EQU_MOD_KEY);
			        activeMask |= LckdownMask;
			    }
			    STATE_CLEAR;
			    return 1; 
			}
                    }
    		    break;

		default  :
	    	    break;
	    }
	}
    }

    if(type == KeyRelease){
	for (i = 0; i < pLckdownMod->KEY_COMB_DEF_NUM; i++) {
	    switch (pLckdownMod->KEY_NUM(i)) {
		case ONE_KEYCODE:
		    if (key == pLckdownMod->KEY(i,0)) return 1;
		    break;

		case TWO_KEYCODE:
		    for (j = 0; j < TWO_KEYCODE; j++) {
			if (key == pLckdownMod->KEY(i,j)) 
			    if (lckdownStat[i*2+j]) {
				lckdownStat[i*2+j] = FALSE;
				break;
			    } else return 1;
		    }
		    break;
		default:
		    break;
	    }
	}
    }
    return 0;
}

/*
**++
**
**  FUNCTIONAL DESCRIPTION : OneshotModKeyInput
**
**	    One-shot modifier 
**
**	If a key or key combination is bound as a "one-shot" modifier,
**	the modifer state only affects the next key press immediately 
**	following this pressed key or key combinaton. This means, only
**	the KeyPrss event sent for the very next key will have the modifier
**	state marked with the one-shot modifier as active. You can treat
**	one-shot modifier as a lock key that unlock automatically after
**	typing in one more key.
**
**      For example if the key combination of ALT/Space is bound to the
**	mode-switch keycode 7A(hex) then the user pressed keycode for
**	PF1, the following events can be expected.
**
**
**			Physical Keybound State   Input Event Generated
**			-----------------------	  ---------------------
**		    		ALT key dwon	   KeyPress ALT keycode	
**			       Space key down	   KeyRelease ALT keycode
**				ALT key up
**				Space key up
**						   KeyPress 7A
**				PF1 key down	   KeyPress PF1
**				PF1 key up	   KeyRelease PF1
**						   KeyRelease 7A
**
**  FORMAL PARAMETERS:
**
**	xevent - X event
**	pdev    - keyboard device
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      internal states
**
**  FUNCTION VALUE:
**
**	0 : This routine do nothing. 
**	1 : transform the key event.
**
**  SIDE EFFECTS:
**
**	none
**
**--
*/
int OneshotModKeyInput(xevent,pdev)
xEvent *xevent;
DevicePtr pdev;
{
    int     i,j;
    int     type;
    KeyCode key;
    DeviceIntPtr    dev   = (DeviceIntPtr)pdev;

    key = xevent->u.u.detail;
    type = xevent->u.u.type;

    if( type == KeyPress ){

	for (i = 0; i < pOneshotMod->KEY_COMB_DEF_NUM; i++) {

	    switch (pOneshotMod->KEY_NUM(i)) {
	        case ONE_KEYCODE:
		    if (key == pOneshotMod->KEY(i,0)) {
			KEY_PRESS(pOneshotMod->EQU_MOD_KEY);
			    activeMask |= OneshotMask;
			    return 1;
		    }
		    break;

		case TWO_KEYCODE:
		    for (j = 0; j < TWO_KEYCODE; j++) {

			if (key != pOneshotMod->KEY(i,j))
			    continue;

			if (!oneshotStat[i*2+j]) {
			    oneshotStat[i*2+j] = TRUE;
			    if (oneshotStat[i*2-j+1]) {
			        KEY_RELEASE(pOneshotMod->KEY(i,1-j));
				KEY_PRESS(pOneshotMod->EQU_MOD_KEY);
			        activeMask |= OneshotMask;
			        STATE_CLEAR;
			        return 1;
			    }
			}
		    }
		    break;

		default:
		    break;
	    }
	}	
    }

    if(type == KeyRelease){

	for (i = 0; i < pOneshotMod->KEY_COMB_DEF_NUM; i++) {

	    switch (pOneshotMod->KEY_NUM(i)) {

		case ONE_KEYCODE:
		    if (key == pOneshotMod->KEY(i,0))
			return 1; 
		    break;

		case TWO_KEYCODE:
		    for (j = 0; j < TWO_KEYCODE; j++) {
			if (key == pOneshotMod->KEY(i,j)) 
			    if (oneshotStat[i*2+j]) {
				oneshotStat[i*2+j] = FALSE;
				break;
			    } else 
				return 1;
		    }
		    break;

		default:
		    return 0;
	    }
	}

        if (activeMask & OneshotMask) {
	    KEY_RELEASE(key);
	    KEY_RELEASE(pOneshotMod->EQU_MOD_KEY);
	    activeMask &= ~OneshotMask;
            return 1;
        }

    }
    return 0;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	NormalModKeyInput - Normal Modifier
**
**	If a key or key combination is bound as a "normal" modifier,
**	the modifer state must be set while this key or key combination  
**	is down. So the user can phiysically hold down this key or key
**	combination while entering other key inputs. 
**
**	For example if key combination of Control/Space is bound to
**	keycode 7B, and 7B is specified as MOD3 modifier.
**
**	Physical input		events
**	--------------		----------------
**	Control down 		KeyPress Control key
**	Space down		KeyRelease Control key
**				KeyPress 7B
**
**	(the user enter the key sequence)
**
**	Control up
**	Space	up
**				KeyRelease 7B
**
**
**  FORMAL PARAMETERS:
**
**	xevent - X event
**	pdev    - keyboard device
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      internal states
**
**  FUNCTION VALUE:
**
**	0 : This routine do nothing. 
**	1 : transform the key event.
**
**  SIDE EFFECTS:
**
**	none
**
**--
*/
int NormalModKeyInput(xevent,pdev) 
xEvent *xevent;
DevicePtr pdev;
{
    int i,j,type;
    KeyCode key;
    DeviceIntPtr    dev   = (DeviceIntPtr)pdev;

    key = xevent->u.u.detail;
    type = xevent->u.u.type;

    if(type == KeyPress){
		
	for (i = 0; i < pNormalMod->KEY_COMB_DEF_NUM; i++) {
	    switch (pNormalMod->KEY_NUM(i)) {

	        case ONE_KEYCODE:
		    if (key == pNormalMod->KEY(i,0)){ 
		        KEY_PRESS(pNormalMod->EQU_MOD_KEY);
			return 1; 
		    }
		    break;

    	        case TWO_KEYCODE:
		    for (j = 0; j < TWO_KEYCODE; j++) {

	                if (key != pNormalMod->KEY(i,j))
			    continue;

			/* When the Normal modifier is active, the
			 * combination key's Press event must be ignored.
			 */
			if (activeMask & NormalMask) return 1;

			/* The first key of key combination */
			if (j == 0) normalStat[i*2] = TRUE;

			/* The second key of keycombination */
		        if (j == 1 && normalStat[i*2]) {
			    activeMask |= NormalMask;
		            KEY_RELEASE(pNormalMod->KEY(i,1-j));
			    KEY_PRESS(pNormalMod->EQU_MOD_KEY);
			    STATE_CLEAR;
			    return 1;
			}
		    }
		    break;

	        default:
	            break;
            }
        }
    }

    if(type == KeyRelease){
		
	for (i = 0; i < pNormalMod->KEY_COMB_DEF_NUM; i++) {

	    switch (pNormalMod->KEY_NUM(i)) {

		case ONE_KEYCODE:
		    if (key == pNormalMod->KEY(i,0)) {
			KEY_RELEASE(pNormalMod->EQU_MOD_KEY);
			return 1;
		    }
		    break;


		case TWO_KEYCODE:
		    for (j = 0; j < TWO_KEYCODE; j++){ 

			if (key == pNormalMod->KEY(i,j)){
			    /* When the Normal modifier state is active,
                             * the combination key's release event must
                             * be ignored.
			     */
 		            if(activeMask & NormalMask) {
				/* The key is the first key of key
				 * combination.
				 */
			        if(j == 0){
		            	    KEY_RELEASE(pNormalMod->EQU_MOD_KEY);
			            activeMask &= ~NormalMask;
				}
				return 1;
			    }else{
			        /* When the key combination key is
				 * gotten, Normal state change to OFF.
				 */
				 normalStat[i*2+j] = FALSE;
			    }
			}
		    }
		    break;

	        default:
		    break;
	    }
	}
    }
    return 0;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**   
**	DoKeyboardKeybindOps -	to map or unmap the special key binding. This 
**				function replaces the VMS version interacting
**				the VMS keyboard driver directly.
**				Each modifier mask and key binding infomation
** 				is set to refer in ProcessKeyboardInput routin.
**
**  FORMAL PARAMETERS:
**
**	commandMode - to map or unmap 
**			0 : Unmap 
**			1 : setting keybinding
**      commandMask - what to map or unmap
**			bit 1 on    : turning lock-down modifier setting
**			bit 2 on    : turning one-shot modifier setting
**			bit 3 on    : turning normal modifier setting
**			bit 4-8     : ignorence
**                      all bit off : all modifier unmap when commandMode = 0. 
**      pKeyBindingPkt - ptr to the keybinding information
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      internal states
**
**  FUNCTION VALUE:
**
**      DRVKEYBIND_SUCCESS
**
**  SIDE EFFECTS:
**
**	Reset all internal state variables used by ProcessKeyboardInput().
**
**--
**/
int
DoKeyboardKeybindOps(commandMode,commandMask,pKeyBindingPkt)
char commandMode,commandMask;
KeyBindLis *pKeyBindingPkt;
{
    int i;

    if (commandMode) {

	/* Map specified key bindings.
	 */
	if (commandMask & kbdef$m_lockdown_mod) {
	    pLckdownMod = NULL;
	    for (i = 0; i < pKeyBindingPkt->itemCount; i++) {
		if (pKeyBindingPkt->KBDef[i].kbdef$r_kb_type.kbdef$B_MASK 
				                      & kbdef$m_lockdown_mod) {
		    pLckdownMod = (KeyBindDefPtr)&(pKeyBindingPkt->KBDef[i]);
                    LckdownMask = kbdef$m_lockdown_mod;
	        }
	    }
	}
	if (commandMask & kbdef$m_oneshot_mod) {
	    pOneshotMod = NULL;
	    for (i = 0; i < pKeyBindingPkt->itemCount; i++) {
		if (pKeyBindingPkt->KBDef[i].kbdef$r_kb_type.kbdef$B_MASK 
						       & kbdef$m_oneshot_mod) {
		    pOneshotMod = (KeyBindDefPtr )&(pKeyBindingPkt->KBDef[i]);
                    OneshotMask = kbdef$m_oneshot_mod;
		}
	    }
	}
	if (commandMask & kbdef$m_normal_mod) {
	    pNormalMod = NULL;
	    for (i = 0; i < pKeyBindingPkt->itemCount; i++) {
		if (pKeyBindingPkt->KBDef[i].kbdef$r_kb_type.kbdef$B_MASK 
				          	        & kbdef$m_normal_mod) {
		    pNormalMod = (KeyBindDefPtr )&(pKeyBindingPkt->KBDef[i]);
                    NormalMask = kbdef$m_normal_mod;

		}
	    }
	}
    } else {

	/* Unmap specified key bindings.
	 */
	if (commandMask & kbdef$m_lockdown_mod) {
	    pLckdownMod = NULL;
            LckdownMask = NULL;
	}
	if (commandMask & kbdef$m_oneshot_mod) {
	    pOneshotMod = NULL;
            OneshotMask = NULL;
	}
	if (commandMask & kbdef$m_normal_mod) {
	    pNormalMod  = NULL;
	    NormalMask  = NULL;
	}

        /* Unmap all specified key bindings */
	if(!commandMask){
	    pLckdownMod = NULL;
            pOneshotMod = NULL;
            pNormalMod  = NULL;
            LckdownMask = NULL;
            OneshotMask = NULL;
	    NormalMask  = NULL;
 	}
    }

    /* Reset all internal state variables except for LckdownMask and
     * OneshotMask.
     */
    for (i = 0; i < 4; i ++) {
	lckdownStat[i] = FALSE;
	oneshotStat[i] = FALSE;
  	normalStat[i]  = FALSE;
    }

    activeMask = 0;

    /* UWS version always returns DRVKEYBIND_SUCCESS.
     */
    return(DRVKEYBIND_SUCCESS);
}
#endif VMS

/*
**++
**  FUNCTIONAL DESCRIPTION:
**   
**	HandleKeyBinding  - For a new keysym table, determine if special
**                          key binding is defined.  If so, then processing
**			    the key binding information.
**
**  This routine first needs to check to see whether the special key binding
**  is defined in the specified new keyboard mapping.  if index entry FF of
**  the keyboard mapping table is defined , then it, should follow the chain
**  to gather all necessary information on the special key binding from  the 
**  table.If the existing key binding and the currently intended binding are
**  the same,then no action is needed. Otherwise it should place the inform-
**  ation in a global data structure,and send the information to the driver.
**  If there are modifiers to bind,this routine should also allocate the ap-
**  propriate modifier bits and fix up the modifier map to reflect the defi-
**  ned key binding.
**
**  FORMAL PARAMETERS:
**
**    pKeyc     - ptr to the key class device structure 
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      If the new key binding information is defined, then both the 
**      modifierKeyMap and modifierMap that are in the keyclass devie
**      structure will be modified to reflect the new binding.
**
**  FUNCTION VALUE:
**
**      return status of number of mapping changed
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int  HandleKeyBinding(pKeyc)
KeyClassPtr pKeyc;
{
    int      i;                                   
    KeyCode  minkc, maxkc;
    int      mapwidth;
    KeySym   *mapFF;
    int      dokeybind = TRUE;
    int      status;
    char     KBcommandMode = (char)NULL;
    char     KBcommandMask = (char)NULL;
    CARD8    mask;
    KeyBindLis *pKBTemp = NULL;

    /* If DriverKeyBindSupport = 0  , it indicates that the server failed
     * to interface with the driver on defining the key binding.
     */
    if(DriverKeyBindSupport == DRVKEYBIND_FAILURE) return(0);

    /* The mapFF is the pointer of keycode FF in new keysym table.
     */
    minkc    = pKeyc->curKeySyms.minKeyCode;
    maxkc    = pKeyc->curKeySyms.maxKeyCode;
    mapwidth = pKeyc->curKeySyms.mapWidth;
    mapFF = pKeyc->curKeySyms.map + (KEYBIND_PTR_ENTRY_INDEX - minkc) * mapwidth;

    /* If the index entry FF of the specified new keysym table contains 
     * no key binding definition and there is an existing binding defined,  
     * we need to cause the existing keybinding definition to be unmapped.
     */
    if( (maxkc < KEYBIND_PTR_ENTRY_INDEX) || 
	((*mapFF) == KEYBIND_PTR_ENTRY_IS_EMPTY) ||
        (mapwidth != FOUR_COLUMN_KEYMAP )){
        if(KeyBindDefined){
            UnmapAndCleanup(pKeyc);
            return(MOD_MAPPING_CHANGED);
        } else
           return (0);  /* no need to do anything */
     }

   /* There is new key binding information. we need to gather the new key 
    * binding info in a temporary work space.
    * If no information is collected then return.
    */
   if(CollectKeybindInfo(&pKBTemp, pKeyc) == FAILURE){
        UnmapAndCleanup(pKeyc);
        return(MOD_MAPPING_CHANGED);
   }

   /* See if the existing binding is the same as the intended new binding.
    * If there are the same, then noop and return.
    */ 
   if(KeyBindDefined){
       if(CheckKeyBind(pKBTemp, pCurrentKeyBind) == SAME_AS_CURRENT_KB){
           Xfree(pKBTemp);      	
	   return(0);
       }else
         UnmapAndCleanup(pKeyc);
       
   }

   /*== dokeybind ==*/
   /* Form the keybinding command and mask values, and the io request 
    * packet, so that we can instruct the driver to set up the binding.
    *
    * KBcommandMode - 1          : for setting key binding
    * KBcommandMask - bit 0   on : for turning lock-down modifier setting 
    *	              bit 1   on : for turning one-shot modifier setting 
    *		      bit 2   on : for turning normal modifier setting 
    *		      bit 4-7    : ignored for now
    */
   KBcommandMode = IO$K_DECW_KEYBIND_DEF;

   for(i = 0; i < ENTRYINDEX_MAX_NUM; i++)
       KBcommandMask |= 
         pKBTemp->KBDef[i].kbdef$r_kb_type.kbdef$B_MASK;

   if( DoKeyboardKeybindOps(KBcommandMode, KBcommandMask, pKBTemp)==
       DRVKEYBIND_SUCCESS){
       /* We have successfully tell the driver to define the 
        * key binding. So, set the appropriate type bit in the 
        * flag - KeyBindDefined.
        */
        KeyBindDefined = KBcommandMask;

        /* Load the new key binding info to the global key binding
	 * structure of CurrentKeyBind.
         */
	if(pCurrentKeyBind)
	    Xfree(pCurrentKeyBind);
        pCurrentKeyBind = pKBTemp;

        if(FixupKBModifierMap(pCurrentKeyBind, pKeyc)){
            /* The server interfaces succcessfully with the driver on setting
             * up the key binding definition. Then check to see whether we need
	     * to create the property for controlling the keyboard mode-switch.
             * This property needs to be associated with the root window.
             * When server startup, the default keymap will be loaded prior to 
	     * the creation of the root window.  And we will be called if the
             * default keymap contains keybinding information.  Thus we need to
	     * guard aginst this timing hole by skipping the property until
	     * next time.
             */ 
	    if(KeymapModePropertyCreated == PASS_INIT){
	      extern WindowPtr    *WindowTable;
	      SetKeymapModeProperty(WindowTable[0],0,0,0);	/* create it */   
	      KeymapModePropertyCreated++;
            } else if(KeymapModePropertyCreated ==KEYMAPPROPERTY_CREATED )
	       SetKeymapModeProperty(0,0,0,0);                  /* reset it */
	    else 
               KeymapModePropertyCreated++;	    
	     
            DriverKeyBindSupport = DRVKEYBIND_SUCCESS;         
	} else
            UnmapAndCleanup(pKeyc);
	return(MOD_MAPPING_CHANGED);
   } else {
       /* The driver fails to define the key binding. */
       DriverKeyBindSupport = DRVKEYBIND_FAILURE;
       Xfree(pKBTemp);         	
       return(0);
   }  
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**   UnmapAndCleanup 
**      If there is anything wrong during the process of 
**	defining the new binding, we must perform unmap
**      and cleanup ops to keep what's known in the keyboard 
**      driver and the server consistent.  Also free  the
**	temporary work area and set the value in the global
**	flag.
**
**  FORMAL PARAMETERS:
**
**    pKeyc     - ptr to the key class device structure 
**
**  IMPLICIT INPUTS:
**      none
**
**  IMPLICIT OUTPUTS:
**      none
**
**  FUNCTION VALUE:
**      none
**
**  SIDE EFFECTS:
**	none
**--
**/
UnmapAndCleanup(pKeyc)
KeyClassPtr pKeyc;
{
    if(KeyBindDefined){

       UnmapKeyBind(UNMAP_ALL);
   
       /* clean up the modifierkey map using the current 
        * pKBModKey_roadmap structure.  
        * And then pKBMopkey_roadmap should be clean up as well.
        */
       CleanUpModifierMap(pKBModKey_roadmap, pKeyc);

       KeyBindDefined = 0;
   
       /*
        * Free the work area of pKBModKey_roadmap and pCurrentKeyBind.
        */
       Xfree(pKBModKey_roadmap);
       pKBModKey_roadmap = (KBModKey_roadmap *)NULL;
    }
    Xfree(pCurrentKeyBind);
    pCurrentKeyBind = (KeyBindLis *)NULL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	CheckKeyEntryIndex - Check the value of key entry index to key 
**			     binding definition for duplicates and valid
**			     range.  The invalid key entry are ignored
**			     and the entry index array are rearranged to 
**			     hold only the valid entry index.
**
**  FORMAL PARAMETERS:
**
**	entry_index - pointers to the keycombination
**      minkc       - minimum keycode value
**      maxkc       - maximum keycode value
**
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int CheckKeyEntryIndex(pKBTemp, entry_index, minkc, maxkc)
KeyBindLis *pKBTemp;
KeyCode    *entry_index;
KeyCode    minkc;
KeyCode    maxkc;
{
    int i,j;

    /* check the repetition */
    for(i = 0; i < ENTRYINDEX_MAX_NUM; i++){
        for(j = i+1; j < ENTRYINDEX_MAX_NUM; j++){
            if(entry_index[i] == entry_index[j])
		entry_index[j] = (KeyCode)NULL;
        }
    }

    /* Check the value of entry index */
    for(i = 0, j = 0; i < ENTRYINDEX_MAX_NUM; i++){
        if(entry_index[i] < minkc || entry_index[i] > maxkc){ 		
            entry_index[i] = (KeyCode)NULL; 
        }else{
	    j++;                        /* count the number of itemCount */
        }
     }
    if(pKBTemp->itemCount > j) pKBTemp->itemCount = j;

    /* set the correct position of entry index*/
    for(i = 0; i < ENTRYINDEX_MAX_NUM; i++){
        if(entry_index[i] == NULL){
            for(j = i + 1; j < ENTRYINDEX_MAX_NUM; j++){
	        if(entry_index[j]){
	            entry_index[i] = entry_index[j];
                    entry_index[j] = (KeyCode)NULL;
                    break;
                }
            }
	}
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	CheckKeyBindEntry  - data is ignored if invalid data is found 
**
**		1. Valid keybinding type
**		2. Valid  equivalent keycode - falls in the range between min
**                  			       keycode and max keycode.
**        	3. valid number of key combination
**   
**
**  FORMAL PARAMETERS:
**
**	pKBDef   - pointers to the keycombination
**      pKeyc     - ptr to the key class device structure 
**
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS
**	FAILURE
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int CheckKeyBindEntry(pKBDef, pKeyc)
KeyBindEntry *pKBDef;
KeyClassPtr pKeyc;
{
    KeyCode  minkc;
    KeyCode  maxkc;
    int	     mapwidth;
    KeySym   *mod_sym;

    minkc    = pKeyc->curKeySyms.minKeyCode;
    maxkc    = pKeyc->curKeySyms.maxKeyCode;
    mapwidth = pKeyc->curKeySyms.mapWidth;

    /* Valid  equivalent keycode ? */
    if(pKBDef->EquivKeyCode < minkc || pKBDef->EquivKeyCode > maxkc)
	return(FAILURE); 

    /* Valid keybinding type ?
     * 1 = lock-down (KBTYPE_LOCKDOWN)
     * 2 = one-shot  (KBTYPE_ONESHOT)
     * 3 = normal    (KBTYPE_NORMAL)
     */
    if(pKBDef->KBType < KBTYPE_LOCKDOWN || pKBDef->KBType > KBTYPE_NORMAL)
	return(FAILURE); 

    /* Check for valid keysym assigned to Equivkeycode here.
     * Specifically, the XK_MODE_SWITCH keysym should be checked to see 
     * if lock-down or one-shot has this keysym in the equiv keycode slot
     * in the keysym table.
     */
    mod_sym = (pKeyc->curKeySyms.map) + (pKBDef->EquivKeyCode - minkc) * mapwidth;

    /* one_shot or lock_down  */  
    if(pKBDef->KBType == KBTYPE_LOCKDOWN ||pKBDef->KBType == KBTYPE_ONESHOT)
	if(*mod_sym != XK_Mode_switch)
            return(FAILURE); 

    /* valid number of key combination */
    if(pKBDef->NumKeyComb == 0)
	return(FAILURE); 
    
    if(pKBDef->NumKeyComb > 2) pKBDef->NumKeyComb = 2;

    return(SUCCESS);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	CheckKeyComb - Check keycodes in a key combination to see 
**		       whether they are valid keycodes. 
**		       The second key code may be 0, so that it 
**		       is possible to bind a one-key to a modifier
**		       keycode.  All specified keycode must fall 
**		       within the range of the min keycode and max
**		       keycode of the specified keysym map.
**
**  FORMAL PARAMETERS:
**
**	pKeyCode - pointers to the keycombination
**      minkc - minimum keycode value
**      maxkc - maximum keycode value
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS - if both keycodes are valid
**	FAILURE - at least one keycode is invalid
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int
CheckKeyComb(pKeycode, minkc, maxkc)
KeyCode *pKeycode;
KeyCode minkc,maxkc;
{
   KeyCode kc2 = (*pKeycode);
   KeyCode kc1 = (*(pKeycode+1));

   if(!kc1) return (FAILURE);		/* NULL  keycode */
       
   if(kc1 < minkc || kc1 > maxkc)       /* kc1 is invalid */
       return (FAILURE);

   if(kc2)
     if(kc2 < minkc || kc2 > maxkc)     /* kc2 is invalid */
       return (FAILURE);
     else return(SUCCESS);		/* both keycodes are ok */
   else return (SUCCESS);               /* one-key sequence is ok */

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	CollectKeybindInfo - To gather the new key binding info from the keysym 
**			     map table to a temporary work space.
**
**  FORMAL PARAMETERS:
**
**	pKBTemp   - ptr to the new key binding definition temporary structure
**      pKeyc     - ptr to the key class device structure 
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS - collect the new key binding definition
**	FAILURE - no information is collected
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int CollectKeybindInfo (ppKBTemp ,pKeyc)
KeyBindLis  **ppKBTemp;
KeyClassPtr pKeyc;
{
    int      i,j,k; 
    KeyCode  minkc;
    KeyCode  maxkc;
    int      mapwidth;
    int      data_counter;
    int	     free_modifier_counter;
    KeyCode  entry_index[3];
    unsigned char type;
    unsigned char equivkeycode;
    unsigned char numkeycomb;
    KeyCode  codebind[2][2];
    unsigned char combcharlen;
    KeyBindPtrEntry *PtrEntry;
    KeyBindLis *pKBTemp;
    KeyCombDef *pKeycomb;

    /* Let pKBTemp points to an allocated working space */
    if(!(pKBTemp = (KeyBindLis *) Xalloc(sizeof(KeyBindLis)) ) )
	return(FAILURE); 
    bzero(pKBTemp, sizeof(KeyBindLis));

    /* Pick up entry index for FF from the specified keyboard mapping table */
    minkc    = pKeyc->curKeySyms.minKeyCode;
    maxkc    = pKeyc->curKeySyms.maxKeyCode;
    mapwidth = pKeyc->curKeySyms.mapWidth;

    PtrEntry = (KeyBindPtrEntry *)(pKeyc->curKeySyms.map + (KEYBIND_PTR_ENTRY_INDEX - minkc) * mapwidth);

    pKBTemp->itemCount = (char)(PtrEntry->NumKeyBind);

    entry_index[0] = PtrEntry->KBEntryIndex1;   
    entry_index[1] = PtrEntry->KBEntryIndex2; 
    entry_index[2] = PtrEntry->KBEntryIndex3; 

    /* Check the value of key entry index to key binding definition.
     * Any invalid entry index is dropped and the itmeCount value is
     * reset.
     */
    CheckKeyEntryIndex(pKBTemp, entry_index, minkc, maxkc);

    data_counter = 0;		/* Initialize data counter to  0 */

    free_modifier_counter = KBModifierMaskFreeBits(pKeyc);

    for(i = 0; entry_index[i] && i < pKBTemp->itemCount; i++){
        KeyBindEntry *pKBDef =  (KeyBindEntry *)( 
				((int *)(pKeyc->curKeySyms.map)) + 
				((entry_index[i]-minkc)*mapwidth) );

        if(CheckKeyBindEntry(pKBDef, pKeyc)==FAILURE) continue;

        /* Make sure we still have modifier to use.*/
         if(free_modifier_counter < 0) break;

	/* All valid, then place this binding info into the table */

        pKeycomb = (KeyCombDef*)&(pKBTemp->KBDef[data_counter].kbdef$r_keyComb[0]);
     
	if(CheckKeyComb(&(pKBDef->KeyComb1[0]),minkc,maxkc) == SUCCESS){

            pKeycomb->KC[0] = pKBDef->KeyComb1[1];
            pKeycomb->KC[1] = pKBDef->KeyComb1[0];

            if(pKeycomb->KC[0] == pKeycomb->KC[1])
		pKeycomb->KC[1] = 0;

            if(pKeycomb->KC[0] && pKeycomb->KC[1])
	        pKeycomb->combCharLength = 2;
	    else
                pKeycomb->combCharLength = 1;

	    *pKeycomb++;
	    pKBTemp->KBDef[data_counter].kbdef$b_num_comb++;
	}

	if(CheckKeyComb(&(pKBDef->KeyComb2[0]),minkc,maxkc) == SUCCESS){

            pKeycomb->KC[0] = pKBDef->KeyComb2[1];
            pKeycomb->KC[1] = pKBDef->KeyComb2[0];

            if(pKeycomb->KC[0] == pKeycomb->KC[1])
		pKeycomb->KC[1] = 0;

            if(pKeycomb->KC[0] && pKeycomb->KC[1])
	        pKeycomb->combCharLength = 2;
	    else
                pKeycomb->combCharLength = 1;

	    pKBTemp->KBDef[data_counter].kbdef$b_num_comb++;
	}
        if (pKBTemp->KBDef[data_counter].kbdef$b_num_comb){
    
            pKBTemp->KBDef[data_counter].kbdef$r_kb_type.kbdef$B_MASK = 
                1 << (pKBDef->KBType-1); 
	    pKBTemp->KBDef[data_counter].kbdef$b_modkey =
                pKBDef->EquivKeyCode;

            data_counter++;
	    free_modifier_counter--;
        }
    }

    if( data_counter){
       pKBTemp->itemCount = data_counter;
       *ppKBTemp = pKBTemp;
       return(SUCCESS);
    }else{
       Xfree( pKBTemp );         	  /* Free the allocated work space */
       ppKBTemp = (KeyBindLis **)NULL;   /* Let pKBTemp points to NULL */
       return(FAILURE);
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	FixupKBModifierMap - fix up the modifierKeyMap to reflect the new
**			     key binding definition.
**	  
**	  This routine is called by HandleKeyBinding, after the specified
**	  new key biniding information is validated.  
**
**  FORMAL PARAMETERS:
**
**	pCurrentKeyBind - ptr to the data structure that holds the key 
**			  binding definition that is presently known to
**			  driver and server.
**      pKeyc           - ptr to the key class device structure 
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS 
**	FAILURE 
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int FixupKBModifierMap(pCurrentKeyBind, pKeyc)
KeyBindLis  *pCurrentKeyBind;
KeyClassPtr pKeyc;
{
    int     i,j;
    int     count;
    int     itemcnt;
    int     size;
    KeyCode equiv;
    KeyCode *pMap = pKeyc->modifierKeyMap;
    int     maxKeysPerMod = pKeyc->maxKeysPerModifier;
    unsigned char    defined[255];

    size = (sizeof(KBModKey_roadmap)) * MAX_NUM_KEYBIND_ALLOWED;

    if ( !pKBModKey_roadmap )
        pKBModKey_roadmap = (KBModKey_roadmap *)Xalloc(size);
    else 
        /* Free the define keybinding modifier keys from the keyModifierMap 
	 * by cleaning up the keysym map and modifierkey map.
         * The current content of pKBModKey_roadmap contains the necessary
         * information to help to do the clean up.
         */
	CleanUpModifierMap(pKBModKey_roadmap, pKeyc);

    bzero(pKBModKey_roadmap, size);
    bzero(&defined[0], 255);

    for(i = ROW_POS_MOD1, itemcnt = 0; 
        (i < ROW_SIZE_MODKEYMAP) && 
	(pCurrentKeyBind->itemCount > itemcnt); i++){

	unsigned char *pMapStartPos = (unsigned char *)pMap + (i * maxKeysPerMod);
	unsigned char modifierKey = pCurrentKeyBind->KBDef[itemcnt].kbdef$b_modkey;

        for(j = 0, count = 0; j < maxKeysPerMod; j++){
	    short index = 0;
	    if( index = (short)*((unsigned char *)(pMapStartPos + j)) ){
		count++;
	        defined[index] = *(pMapStartPos +j);
            }
        }

        if(!count && !(defined[modifierKey]) ){
            /* There is free modifier bits left to use in the modifier mask */
            /* Make allocation of modifier key map in the key class 
             * device data struct.  Then make appropriate changes to 
             * the modifier map. 
             */
            *(pMap + ( i * maxKeysPerMod) ) = modifierKey;

            pKeyc->modifierMap[modifierKey] = 1 << i;

            pKBModKey_roadmap[itemcnt].pKBDef = 
               &(pCurrentKeyBind->KBDef[itemcnt]);
	    pKBModKey_roadmap[itemcnt].ModKeyMap_row_index = i+1;
	    pKBModKey_roadmap[itemcnt].ModKeyMap_col_index = 1;

	    itemcnt++;
	    defined[modifierKey] = modifierKey;
        }
    }
    if(itemcnt)
      return (SUCCESS);
    else
      return (FAILURE);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	UnmapKeyBind  -  Unmap the key binding definition that is presently
**			 known to the driver.
**   
**
**  FORMAL PARAMETERS:
**
** 	settingType - bit mask defined as KeyBindDefined 
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS 
**	FAILURE
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int UnmapKeyBind(settingType)
char settingType;
{
    register int status;
    char     KBcommandMode;
    char     KBcommandMask;

    /* specified setting Type is currently active */
/*    if(settingType != KeyBindDefined) return;
*/
    KBcommandMode = IO$K_DECW_KEYBIND_UNMAP;

    KBcommandMask = settingType;

    status = DoKeyboardKeybindOps(KBcommandMode, KBcommandMask, 0);

    if (status == DRVKEYBIND_SUCCESS)
      return (TRUE);
    else
      return (FALSE);

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	KBModifierMaskFreeBits  - Look in the keyclass device structure and 
**                                count number of free slots in the modifier 
**				  bit mask.
**
**  FORMAL PARAMETERS:
**
**	pKeyc : pointer to keyclass device structure
**
**  IMPLICIT INPUTS:
**
**      pKBModKey_roadmap :
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**	total number of free bits as status
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int KBModifierMaskFreeBits(pKeyc)
 KeyClassPtr pKeyc;	
{ 
   int i,j;
   int count,total_number;
   CARD8 maxcol;
   char *pTemp = (char *)(pKeyc->modifierKeyMap);

    total_number = 0;
    maxcol = pKeyc->maxKeysPerModifier;
    

    /* Scan modifierKeyMap array to find the number of the rows that
     * contains all zeros for the specific modifiers.
     */	             
                                           /* loop of the row direction */
    for(i = ROW_POS_MOD1; i < ROW_SIZE_MODKEYMAP; i++){
    
	char  *pMapEntry = pTemp + i * maxcol;

        count = 0;

        for(j = 0; j < maxcol; j++)         /* loop of the column direction */
	    if(*(pMapEntry + j)) 
               count++;
        
        if(!count) total_number++;
    }

    /* Also count the modifiers that are currently being used 
     * for the existing key bindings. 
     */
    if(pKBModKey_roadmap){
        for(i = 0; i < ENTRYINDEX_MAX_NUM; i++)
            if(pKBModKey_roadmap[i].ModKeyMap_row_index) total_number++;  
    }

    return total_number;
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	CheckKeyBind  -  Check if the existing key binding and the currently
**			 intended binding are the same.
**			    
**  FORMAL PARAMETERS:
**
**	pKBTemp	  	- ptr to the new key binding definition 
**	pCurrentKeyBind - existing key bind .
**			  ptr to data structure that holds the key binding
**			  definition that is presently known to driver and
**			  server.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**	SAME_AS_CURRENT_KB        - if the existing key binding and the
**                                  currently intended binding are the same
**
**      DIFFERENT_FROM_CURRENT_KB - if the existing key binding and the 
**                                  intended binding are different.
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
int CheckKeyBind(pKBTemp, pCurrentKeyBind)
KeyBindLis  *pKBTemp;
KeyBindLis  *pCurrentKeyBind;
{
    /* ???rewrite this */
    return(DIFFERENT_FROM_CURRENT_KB);
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      CleanUpModifierMap 
**         Free the defined keybinding modifier keys from
**	   the modifierkeyMap and then clean up modifierMap.
**         The current content of pKBModKey_roadmap contains
**         the necessary information to help to do the clean 
**         up. And then pKBMopkey_roadmap should be cleaned up
**         as well.
**
**  FORMAL PARAMETERS:
**
**	pKeyc              - ptr to keyclass device structure.
**      pKBModKey_roadmap  - ptr to the data structure that save the 
**			     information of the define key binding modifier 
**			     keys.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**      
**	none
**
**--
**/
CleanUpModifierMap(pKBModKey_roadmap, pKeyc)
KBModKey_roadmap *pKBModKey_roadmap;
KeyClassPtr pKeyc;
{
    int     i,j;
    int	    count,max_count;
    int     row_index;
    int     col_index;
    char    equiv;
    KeyCode *pMap = pKeyc->modifierKeyMap;
    int     maxKeysPerMod = pKeyc->maxKeysPerModifier;

    /* clean up the modifierKeyMap and modifierMap.
     */
    for(i = 0; i < ENTRYINDEX_MAX_NUM; i++){

        row_index = pKBModKey_roadmap[i].ModKeyMap_row_index;
        col_index = pKBModKey_roadmap[i].ModKeyMap_col_index;

        if(row_index){
            row_index--;
            col_index--;
            equiv     = pKBModKey_roadmap[i].pKBDef->kbdef$b_modkey;
            if(equiv)
                *(pMap + row_index * maxKeysPerMod + col_index) = 0;

            if(!col_index && pKeyc->modifierMap[equiv]){
		int mod_bit = ffs((int)(pKeyc->modifierMap[equiv])) - 1;
		unsigned short pTemp = (unsigned short)(pKeyc->modifierMap[equiv]);
		if(pKeyc->modifierKeyCount[mod_bit])
		  pKeyc->modifierKeyCount[mod_bit]--;
		pKeyc->state &= ~pTemp;
                pKeyc->modifierMap[equiv] = 0;
            }

            /* ##### Bug fix for OSF/1 , Jan-8-1993 K.Ichimaru #####
             * When the keymap file was changed under the lock-down modifier
             * state on, the server leaves the server inside key down state of
             * lock-down modifier key alone.It cause the problem of mode_switch
             * work. When the lock-down mode_switch is defined next time, the
             * first lock-down modifier KeyPress events will be ignored.
             * Therefore it has to be clear.
             */
            if(equiv){
                int bit = 1 << (equiv & 7);
                if( pKeyc->down[equiv >> 3 ] & bit)
                    pKeyc->down[equiv >> 3 ] &= ~bit;
            }

        }
    }

    /* Calculate the maxKeysPerModifier
     */
    max_count = 0;
    for(i = 0; i < ROW_SIZE_MODKEYMAP; i++){
	count = 0;
        for(j = 0; j < maxKeysPerMod; j++){
            if(*(pMap + i * maxKeysPerMod + j))
                count++;
        }
        if(max_count < count) max_count = count;
    }
    if(maxKeysPerMod < max_count) 
        pKeyc->maxKeysPerModifier = max_count;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**   SetModMapKeyBindCheck   
**
**	 This routine is called by ProcSetModifierMapping()
**       in devices.c. If the newly specified modifier map 
**       invalidates the existing key binding, then the driver 
**       needs to be notified to unmap the existing key binding. 
**
**  FORMAL PARAMETERS:
**
**	pKeyc              - ptr to keyclass device structure.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**	none
**--
**/
SetModMapKeyBindCheck(pKeyc)
KeyClassPtr pKeyc;
{
    int     i;
    int     invalidate_flag = 0;
    int     row_index;
    int     col_index;
    char    equiv;
    KeyCode *pMap = pKeyc->modifierKeyMap;
    int     maxKeysPerMod = pKeyc->maxKeysPerModifier;

    /* Is the specific type of keybinding defined ?
     */
    if(KeyBindDefined){

	/* Does the specified modifier map invalidated ? */
	for(i = 0; i < ENTRYINDEX_MAX_NUM; i++){
            row_index = pKBModKey_roadmap[i].ModKeyMap_row_index;
            col_index = pKBModKey_roadmap[i].ModKeyMap_col_index;

            if(row_index){
                row_index--;
                col_index--;
                equiv  = pKBModKey_roadmap[i].pKBDef->kbdef$b_modkey;

                if(equiv != *(pMap + row_index * maxKeysPerMod + col_index))
		    invalidate_flag = 1;
	    }
	}                                       

        if(invalidate_flag){
            KeyBindDefined = 0;

           /*
            * Free the work area of pKBModKey_roadmap and pCurrentKeyBind.
            */
            Xfree(pKBModKey_roadmap[0].pKBDef);
            Xfree(pKBModKey_roadmap[1].pKBDef);
            Xfree(pKBModKey_roadmap[2].pKBDef);
            Xfree(pKBModKey_roadmap);
            pKBModKey_roadmap = (KBModKey_roadmap *)NULL;
            Xfree(pCurrentKeyBind);
            pCurrentKeyBind = (KeyBindLis *)NULL;
        }
    }
}


/* This is called by ProcessInputEvent to process a 
 * keymapNotify event.  This is being used to 
 * update a property named _DEC_KEYMAP_MODE, so that 
 * interested clients would promptly know the change
 * of the keyboard state(LDM on to LDMoff)
 * We need to remap the key binding type that's been
 * passed to the binding type constants that are 
 * known to clients. 
 */ 
HandleKeyboardModeChange(keycode,type,state) 
short keycode, type, state;
{
    static short type_remap[3] = {KBTYPE_LOCKDOWN,KBTYPE_ONESHOT,KBTYPE_NORMAL};

    type = type_remap[type];

    /* Make sure if we have this property already created before updating
     * it.
     */
    if(!KeymapModePropertyCreated || KeymapModePropertyCreated == PASS_INIT ){
      extern WindowPtr    *WindowTable;
      SetKeymapModeProperty(WindowTable[0],0,0,0);	/* create it */   
      KeymapModePropertyCreated = KEYMAPPROPERTY_CREATED;
    }
    /* pass 0 as pWin, so it would be an update to the
     * the existing property.
     */
    if(KeymapModePropertyCreated ==  KEYMAPPROPERTY_CREATED)
      SetKeymapModeProperty(0,keycode,type,state);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**    DoKeyboardModeSwitch
**  
**	  Send the driver the correct modifier keycode to emulate 
**	  the keyboard lock-down mode-switching function or to unlcok
**	  the keyboard from mode-switching.
**
**  FORMAL PARAMETERS:
**
**       CommandOption - Lock down or unlock keyboard mode switching
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**     
**      none
**
**  FUNCTION VALUE:
**
**      none
**
**  SIDE EFFECTS:
**
**	none
**--
**/
int
DoKeyboardModeSwitch(commandOption)
int commandOption;
{
    register KeyClassPtr pKeyc = inputInfo.keyboard->key;
    int i = 0, found = FALSE;
    struct KeyBindDef *pTemp;

    if(!KeyBindDefined || !pKBModKey_roadmap)
      return(KBModeSwitchFailure);

    /* Scan through the key binding definition area to find
     * the binding definition for the lock-down modifier.
     */
    while(!found & i < MAX_NUM_KEYBIND_ALLOWED ){
       pTemp = pKBModKey_roadmap[i].pKBDef;

       if(pTemp->kbdef$r_kb_type.kbdef$r_mask_bits.kbdef$v_lockdown_mod)
         found = TRUE;
       else
         i++;
    }

    if(!found)
      return(KBModeSwitchInvalidCmd);
    else{
      /* 
       * Double check the modifier key state before sending 
       * this software-emulated keycode down to the keyboard 
       * device driver.  If the modifier is already in the 
       * state it is requested, no action is done, and a
       * no-op status is returned.  Otherwise the keycode is 
       * sent to the driver to cause the keyboard to either 
       * locking down or unlock the mode-switch function.
       */
      char modkey_mask;
      char modifierKey;
      int  modkey_down;

      modkey_mask = 
        pKeyc->modifierMap[(int)(modifierKey = pTemp->kbdef$b_modkey)];
      modkey_down = (int)modkey_mask & (int)(pKeyc->state);

      if(commandOption == LockDownModeSwitch && modkey_down)
        return(KBModeSwitchNoop);
      
      if(commandOption == UnlockModeSwitch && !modkey_down)
        return(KBModeSwitchNoop);
      
      if(SendFakeKeybindModKey(modifierKey,commandOption) != DRVKEYBIND_SUCCESS)
        return(KBModeSwitchFailure);
      else
        return(KBModeSwitchSuccess);
    } 
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**   
**	SendFakeKeyBindModKey
**
**	  UWS specific function convering some functions of the VMS
**	  keyboard driver function.
**
**	  This function generate the fake Key event, as if 
**	  the user has typed in such a keycode on the keyboard.
**	  Currently, this function is to be used to send the modifier
**	  keycode for locking down or unlocking the keyboard mode-switch.
** 
**  FORMAL PARAMETERS:
**
**      Keycode to send 
**      CommandOption : 1	 Lock down keyboard mode switching
**			2	 Unlock keyboard mode switching
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      DRVKEYBIND_SUCCESS or DRVKEYBIND_FAILURE
**
**  SIDE EFFECTS:
**
**	none
**
**--
**/
int
SendFakeKeybindModKey(keycode,CommandOption)
int keycode;
int CommandOption;
{
    xEvent *xevent;
    DevicePtr pdev;
    DeviceIntPtr dev;

    pdev = LookupKeyboardDevice();
    dev   = (DeviceIntPtr)pdev;
    xevent =(xEvent *) xalloc ( sizeof(xEvent));

    if( CommandOption == 1 ){
	KEY_PRESS(keycode);
        activeMask |= LckdownMask;
    }

    if( CommandOption == 2 ){
	KEY_RELEASE(keycode);
	activeMask &= ~LckdownMask;
        HandleKeyboardModeChange(keycode, 0, KeyRelease);
    }

    xfree(xevent);

    /* There is no this case. */
    if( CommandOption != 1 && CommandOption != 2 )  
      return(DRVKEYBIND_FAILURE);

    return(DRVKEYBIND_SUCCESS);
}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      InitHandleKeybindParameter:
**
**  Initial the parameters for the Mode Switch mechanism of the i18n
**  keyboard support.
**
**  This routine was added for bug fix of server crash while using the
**  Mode Switch mechanism because of the internal data inconsistency.
**  2-Feb-1993 K.Ichimaru
**
**/
void
InitHandleKeybindingParameter()
{
    int i;

    DriverKeyBindSupport  = DRVKB_SUPPORT_INIT_VALUE;
    KeyBindDefined = FALSE;
    pCurrentKeyBind = NULL;
    pKBModKey_roadmap = NULL;
    KeymapModePropertyCreated = 0;

    pLckdownMod = NULL;
    pOneshotMod = NULL;
    pNormalMod  = NULL;

    LckdownMask = NULL;
    OneshotMask = NULL;
    NormalMask  = NULL;
    activeMask =  NULL;

    for (i = 0; i < 4; i ++) {
        lckdownStat[i] = FALSE;
        oneshotStat[i] = FALSE;
        normalStat[i]  = FALSE;
    }
}

#endif MODE_SWITCH
