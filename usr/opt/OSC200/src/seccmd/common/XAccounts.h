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
 *	@(#)$RCSfile: XAccounts.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:01:38 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifdef SEC_BASE

#ifndef __XAccounts__
#define __XAccounts__

/*
	filename:
		XAccounts.h
	
	copyright:
		Copyright (c) 1989, 1990 SecureWare, Inc.
		ALL RIGHTS RESERVED
	
	function:
		include file for auth part of ISSO role program
*/

#include "Accounts.h"

/* 
 * Macro to define general information for the accounts screens.
 * This defines things particular to accounts which are not in XMacros.h
 */

#define CREATE_ACCOUNTS_HEADER                                		\
									\
/* External routines */							\
extern void   SetDeviceName();						\
extern void   SetUserName();						\
extern void   SetWidgetWidth();						\
extern void   DebugUser();						\
extern void   CreateItemsTextD();					\
extern void   CreateItemsYND();						\
extern void   SetToggle();						\
extern void   SetTextD();						\
extern void   SetYND();							\
extern void   LoadMessage();						\
extern int    XGetTerminalInfo();					\
extern int    XGetDeviceInfo();						\
extern int    XGetUserInfo();						\
extern int    XGetSystemInfo();						\
extern int    XWriteTerminalInfo();					\
extern int    XWriteDeviceInfo();					\
extern int    XWriteUserInfo();						\
extern int    XWriteSystemInfo();					\
extern int    *MallocInt();						\
extern char   *MallocChar();						\
extern Widget *MallocWidget();						\
extern Widget CreateHeaderRight();					\
extern Widget CreateSecondaryForm();					\
extern Widget CreateDeviceName();					\
extern Widget CreateUserName();						\
extern Dimension GetWidth();						\
									\
/* Local routines */							\
static void								\
	TextCallback(),							\
	TextDefaultCallback(),						\
	OnCallback(),							\
	DefaultCallback();						\
									\
/* Local variables */							\
static Boolean								\
	in_change_mode;                /* Need this for text input */	\
									\
static struct    prpw_if    pr;		/* Protected password */	\
static struct    dev_if     dv;		/* Device assignment */		\
static struct    sdef_if    sd;		/* System default */		\
									\
static int								\
	*value,								\
	*def_value;							\
									\
static char								\
	**msg_header,							\
	*msg_header_text,						\
	*value_state,							\
	*toggle_state,							\
	*def_toggle_state;						\
									\
static Widget								\
	user_name_widget,						\
	device_name_widget,						\
	*text_widget,							\
	*text_default_widget,						\
	*yes_widget,							\
	*default_widget;


/* Macros for callbacks */
#define CREATE_TEXT_CALLBACKS(ARG1)                                   	\
/* If start typing then turn default button off */			\
static void 								\
TextCallback(w, i, info) 						\
	Widget                  w; 					\
	int                        i;					\
	XmAnyCallbackStruct    *info;					\
{									\
	/* If changing the value then switch the default toggle off */  \
	if (in_change_mode) {						\
		SetToggle(text_default_widget[i], False);		\
	}								\
}									\
 									\
static void 								\
TextDefaultCallback(w, i, info) 					\
	Widget                  w; 					\
	int                        i;					\
	XmAnyCallbackStruct    *info;					\
{									\
	char buffer[20];						\
	sprintf (buffer, "%d", ARG1 [i]);				\
	XtRemoveCallback (text_widget[i], XmNvalueChangedCallback, 	\
			TextCallback ,i); 				\
	XmTextSetString (text_widget[i], buffer);			\
	XtAddCallback (text_widget[i], XmNvalueChangedCallback, 	\
			TextCallback ,i); 				\
}


#define CREATE_ON_CALLBACKS(ARG1, ALL_BUTTONS, NUM_BUTTONS)             \
static void 								\
OnCallback(w, i, info) 							\
	Widget                  w; 					\
	int                     i;					\
	XmAnyCallbackStruct    *info;					\
{									\
	int j;								\
									\
	/* Clear the default toggle */					\
	if (ALL_BUTTONS) {						\
		for (j=0; j<NUM_BUTTONS; j++)				\
			SetToggle(default_widget[j], False);		\
	}								\
	else 								\
		SetToggle(default_widget[i], False);			\
}									\
									\
static void 								\
DefaultCallback(w, i, info) 						\
	Widget                  w; 					\
	int                     i;					\
	XmAnyCallbackStruct    *info;					\
{									\
	/* Set the current user value to system default */		\
	SetToggle(yes_widget[i], (ARG1 [i] == YES_CHAR) );		\
}


#endif /* __XAccounts__ */
#endif /* SEC_BASE */
