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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: XMenu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:12 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if SEC_BASE


/*
	filename:
		XMenu.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		ISSO Menu bar definition for Devices
		
	entry points:
		DevicesMenuBarInitialize()
*/


/* Common C include files */
#include <sys/types.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>

/* ISSO include files */
#include "XMain.h"
#include "XDevices.h"

/* Definitions */
#define NUM_DEFAULT_OPTIONS     5
#define NUM_ADD_OPTIONS    	4
#define NUM_MODIFY_OPTIONS	6

/* External routines */
extern void
	LoadMessage(),

	DevDefLogStart(),   DevDefLogOpen(),   DevDefLogClose(),
#if defined(SEC_MAC) && ! defined(SEC_SHW)
	DevDefnslStart(),   DevDefnslOpen(),   DevDefnslClose(),
	DevDefxslStart(),   DevDefxslOpen(),   DevDefxslClose(),
	DevDefsslStart(),   DevDefsslOpen(),   DevDefsslClose(),
#endif
#ifdef ILB
	DevDefsilStart(),   DevDefsilOpen(),   DevDefsilClose(),
#endif

#if SEC_NET_TTY
	DevAddHosStart(),   DevAddHosOpen(),   DevAddHosClose(),
#endif
	DevAddPriStart(),   DevAddPriOpen(),   DevAddPriClose(),
	DevAddRemStart(),   DevAddRemOpen(),   DevAddRemClose(),
	DevAddTerStart(),   DevAddTerOpen(),   DevAddTerClose(),

	DevModSelStart(),   DevModSelOpen(),   DevModSelClose(),
	DevModLogStart(),   DevModLogOpen(),   DevModLogClose(),
#if defined(SEC_MAC) && ! defined(SEC_SHW)
	DevModnslStart(),   DevModnslOpen(),   DevModnslClose(),
	DevModxslStart(),   DevModxslOpen(),   DevModxslClose(),
	DevModsslStart(),   DevModsslOpen(),   DevModsslClose(),
#endif
#ifdef ILB
	DevModsilStart(),   DevModsilOpen(),   DevModsilClose(),
#endif

	DevLockStart()  ,   DevLockOpen()  ,   DevLockClose()  ,
	DevUnlockStart(),   DevUnlockOpen(),   DevUnlockClose(),

#if SEC_NET_TTY
	DevHosLckStart(),   DevHosLckOpen(),   DevHosLckClose(),
	DevHosUlkStart(),   DevHosUlkOpen(),   DevHosUlkClose(),
	DevRemoveStart(),   DevRemoveOpen(),   DevRemoveClose(),
	DevHosRemStart(),   DevHosRemOpen(),   DevHosRemClose();
#else
	DevRemoveStart(),   DevRemoveOpen(),   DevRemoveClose();
#endif

/* Local routines */
static void
	MapPulldown();

/* Local variables */
static XtCallbackProc Defaults[] = {
	DevDefLogOpen,
#if defined(SEC_MAC) && ! defined(SEC_SHW)
	DevDefnslOpen,
	DevDefxslOpen,
	DevDefsslOpen,
#else
	DevDefLogOpen,	/* Give dummy values */
	DevDefLogOpen,
	DevDefLogOpen,
#endif
#ifdef ILB
	DevDefsilOpen,
#else
	DevDefLogOpen,
#endif
	};

static XtCallbackProc Add[] = {
#if SEC_NET_TTY
	DevAddHosOpen,
#else
	DevAddPriOpen,
#endif
	DevAddPriOpen,
	DevAddRemOpen,
	DevAddTerOpen,
	};

static XtCallbackProc Modify[] = {
	DevModSelOpen,
	DevModLogOpen,
#if defined(SEC_MAC) && ! defined(SEC_SHW)
	DevModnslOpen,
	DevModxslOpen,
	DevModsslOpen,
#else
	/* Put in dummy values if not a SEC_MAC non SEC_SHW system */
	DevModSelOpen,
	DevModSelOpen,
	DevModSelOpen,
#endif
#ifdef ILB
	DevModsilOpen,
#else
	/* Put in dummy values if not an ILB system */
	DevModSelOpen,
#endif
	};

static char
	**msg,
	*msg_text;

static Widget
	ModifyButtons[NUM_MODIFY_OPTIONS];

void 
DevicesMenuBarInitialize()
{
	Cardinal    n;
	Arg         args[20];
	int         i, j;
	char        m;
	Widget      DevicesPulldown,
		    pulldown,
		    w;
	XmString    xmstring;
				
	/* Read the menu options from the external file */
	LoadMessage ("msg_devices_options", &msg, &msg_text);

	/******************************************************************/
	/* Devices Pulldown                                               */
	/******************************************************************/
	
	n = 0;
	DevicesPulldown = XmCreatePulldownMenu(main_menubar, 
			"PulldownMenu", args, n);
	
	i = 0;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,DevicesPulldown); n++;
	w = XmCreateCascadeButton(main_menubar, "MenubarButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* System defaults                                                */
	/******************************************************************/
	
	n = 0;
	pulldown = XmCreatePulldownMenu(DevicesPulldown, "PulldownMenu", 
			args, n);

	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,pulldown); n++;
	w = XmCreateCascadeButton(DevicesPulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/* Cascading Menu Options */
	for (j=0; j<NUM_DEFAULT_OPTIONS; j++) {
		i ++;
		xmstring = XmStringCreate(msg[i], charset);
		if (! xmstring)
	   	    MemoryError();
		m = msg[++i][0];
		n = 0;
		XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
		XtSetArg(args[n], XmNmnemonic       ,m); n++;

	if (
#ifdef ILB
	 (True) &&
#else
	(j != 4) &&
#endif

#if ! defined (SEC_MAC) || defined (SEC_SHW)
	(j < 1 || j > 3) 
#else
	(True)
#endif
		) {
		w = XmCreatePushButtonGadget(pulldown, "PulldownButton", 
			args, n);
		XtManageChild(w);
		XtAddCallback (w, XmNactivateCallback, Defaults[j], NULL);
		XmStringFree(xmstring);
		}
	}

	/******************************************************************/
	/* Add Devices                                                    */
	/******************************************************************/
	n = 0;
	pulldown = XmCreatePulldownMenu(DevicesPulldown, "PulldownMenu", 
		args, n);

	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,pulldown); n++;
	w = XmCreateCascadeButton(DevicesPulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/* Cascading Menu Options */
	for (j=0; j<NUM_ADD_OPTIONS; j++) {
		i++;
		xmstring = XmStringCreate(msg[i], charset);
		if (! xmstring)
			MemoryError();
		m = msg[++i][0];
		n = 0;
		XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
		XtSetArg(args[n], XmNmnemonic       ,m); n++;
		/* No Host until 2.0 */
#ifndef SEC_NET_TTY
		if (j != 0) {
#endif
		w = XmCreatePushButtonGadget(pulldown, "PulldownButton", 
			args, n);
		XtManageChild(w);
		XtAddCallback (w, XmNactivateCallback, Add[j], NULL);
		XmStringFree(xmstring);
#ifndef SEC_NET_TTY
		}
#endif

	}

	/******************************************************************/
	/* Modify defaults                                                */
	/******************************************************************/
	n = 0;
	pulldown = XmCreatePulldownMenu(DevicesPulldown, "PulldownMenu", 
			args, n);
	XtAddCallback(pulldown, XmNmapCallback, MapPulldown, NULL);

	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];
	n = 0;
	XtSetArg(args[n], XmNlabelString  ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic     ,m); n++;
	XtSetArg(args[n], XmNsubMenuId    ,pulldown); n++;
	w = XmCreateCascadeButton(DevicesPulldown, "CascadeButton", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	/* Cascading Menu Options */
	for (j=0; j<NUM_MODIFY_OPTIONS; j++) {
		i ++;
		xmstring = XmStringCreate(msg[i], charset);
		if (! xmstring)
	   	    MemoryError();
		m = msg[++i][0];
		n = 0;
		XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
		XtSetArg(args[n], XmNmnemonic       ,m); n++;

	if (
#ifdef ILB
	 (True) &&
#else
	(j != 5) &&
#endif

#if ! defined (SEC_MAC) || defined (SEC_SHW)
	(j < 2 || j > 4) 
#else
	(True)
#endif
		) {
		w = XmCreatePushButtonGadget(pulldown, "PulldownButton", 
			args, n);
		XtManageChild(w);
		XtAddCallback (w, XmNactivateCallback, Modify[j], NULL);
		XmStringFree(xmstring);
		ModifyButtons[j] = w;
		}

		/* Add a separator */
		if (j == 0) {
			n = 0;
			XtSetArg(args[n], XmNseparatorType ,XmDOUBLE_LINE); n++;
			w = XmCreateSeparatorGadget(pulldown, "Separator", 
				args, n);
			XtManageChild(w);
		}
	}

	/******************************************************************/
	/* Lock Terminal                                                  */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevLockOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* Unlock Terminal                                                */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevUnlockOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);


#if SEC_NET_TTY
	/******************************************************************/
	/* Lock Host                                                      */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevHosLckOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);

	/******************************************************************/
	/* Unlock Host                                                    */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevHosUlkOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);
#else
	/* If not SEC_NET_TTY then we need to increment the pointer */
	i ++;
	i ++;
	i ++;
	i ++;
#endif

	/* Create a separator for Remove Devices */
	n = 0;
	XtSetArg(args[n], XmNseparatorType ,XmDOUBLE_LINE); n++;
	w = XmCreateSeparatorGadget(DevicesPulldown, "Separator", args, n);
	XtManageChild(w);

	/******************************************************************/
	/* Remove Devices                                                 */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevRemoveOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);

#if SEC_NET_TTY
	/******************************************************************/
	/* Remove Host                                                    */
	/******************************************************************/
	i ++;
	xmstring = XmStringCreate(msg[i], charset);
	if (! xmstring)
		MemoryError();
	m = msg[++i][0];

	n = 0;
	XtSetArg(args[n], XmNlabelString    ,xmstring); n++;
	XtSetArg(args[n], XmNmnemonic       ,m); n++;
	w = XmCreatePushButtonGadget(DevicesPulldown, "CascadeButton", args, n);
	XtAddCallback (w, XmNactivateCallback, DevHosRemOpen, NULL);
	XtManageChild(w);
	XmStringFree(xmstring);
#endif
}

void 
StartDevicesFunctionalUnits()
{
	chosen_device_name = NULL;
	/* The order does not really matter so all non SEC_SHW stuff put at end */
	DevDefLogStart();

#if SEC_NET_TTY
	DevAddHosStart();
	DevHosLckStart();
	DevHosUlkStart();
	DevHosRemStart();
#endif
	DevAddPriStart();
	DevAddRemStart();
	DevAddTerStart();

	DevModSelStart();
	DevModLogStart();

	DevLockStart()  ;
	DevUnlockStart();
	DevRemoveStart();

#if defined (SEC_MAC) && ! defined (SEC_SHW)
	DevDefnslStart();
	DevDefxslStart();
	DevDefsslStart();

	DevModnslStart();
	DevModxslStart();
	DevModsslStart();
#endif
#ifdef ILB
	DevDefsilStart();
	DevModsilStart();
#endif
}

void 
StopDevicesFunctionalUnits()
{
	/* The order does not really matter so all non SEC_SHW stuff put at end */
	DevDefLogStop();

#if SEC_NET_TTY
	DevAddHosStop();
	DevHosLckStart();
	DevHosUlkStart();
	DevHosRemStart();
#endif
	DevAddPriStop();
	DevAddRemStop();
	DevAddTerStop();

	DevModSelStop();
	DevModLogStop();

	DevLockStop()  ;
	DevUnlockStop();

	DevRemoveStop();

#if defined (SEC_MAC) && ! defined (SEC_SHW)
	DevDefnslStop();
	DevDefxslStop();
	DevDefsslStop();

	DevModnslStop();
	DevModxslStop();
	DevModsslStop();
#endif
#ifdef ILB
	DevDefsilStop();
	DevModsilStop();
#endif
}

/* This callback invoked when the pulldown is mapped; it checks whether */
/* all the modify options are available for this user                  */
static void 
MapPulldown(w, ptr, info)
	Widget    w;
	char     *ptr;
	XmAnyCallbackStruct *info ;
{
	struct dev_asg *dev;
	int	j;

	/* If no device selected then only enable Select */
	if (! chosen_device_name) {
		XtSetSensitive (ModifyButtons[1], False);
#if SEC_MAC
#ifndef SEC_SHW
		XtSetSensitive (ModifyButtons[2], False);
		XtSetSensitive (ModifyButtons[3], False);
		XtSetSensitive (ModifyButtons[4], False);
#endif
#ifdef ILB
		XtSetSensitive (ModifyButtons[5], False);
#endif
#endif
		return;
	}

	dev = getdvagnam(chosen_device_name);

	/* If we can't read it display all options. User will get error later */
	if (dev == (struct dev_asg *) 0) {
		XtSetSensitive (ModifyButtons[1], True);
#if SEC_MAC
#ifndef SEC_SHW
		XtSetSensitive (ModifyButtons[2], True);
		XtSetSensitive (ModifyButtons[3], True);
		XtSetSensitive (ModifyButtons[4], True);
#endif
#ifdef ILB
		XtSetSensitive (ModifyButtons[5], True);
#endif
#endif
		return;
	}

	XtSetSensitive (ModifyButtons[1], True);

	/* Check to see if multi or single */
#if SEC_MAC
#ifndef SEC_SHW
	if (ISBITSET(dev->ufld.fd_assign, AUTH_DEV_SINGLE)) {
		XtSetSensitive (ModifyButtons[2], False);
		XtSetSensitive (ModifyButtons[3], False);
		XtSetSensitive (ModifyButtons[4], True);
	}
	else {
		XtSetSensitive (ModifyButtons[2], True);
		XtSetSensitive (ModifyButtons[3], True);
		XtSetSensitive (ModifyButtons[4], False);
	}
#endif

#ifdef ILB
	if (ISBITSET(dev->ufld.fd_assign, AUTH_DEV_ILSINGLE)) {
		XtSetSensitive (ModifyButtons[5], True);
	}
	else {
		XtSetSensitive (ModifyButtons[5], False);
	}
#endif

#endif /* SEC_MAC */

}    
#endif /* SEC_BASE **/
