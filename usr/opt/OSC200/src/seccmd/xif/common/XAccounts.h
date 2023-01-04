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
 *	@(#)$RCSfile: XAccounts.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:04:39 $
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

/* The following are required */
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

/* Load in main variables that are accessed by all files */
#ifndef MAIN
extern
#endif
char 	
	*chosen_user_name,
	*chosen_device_name;

/* Errors */
#define SUCCESS			0
#define FAILURE			1

#define NOT_USER		1
#define NO_PRDB_ENTRY		2
#define ACCT_RETIRED		3
#define IS_ISSO			4
#define NOT_ISSO		5
#define NULL_USER		6
#define CANT_UPDATE		7
#define CANT_READ_AUDIT		8		/* Not used */

#define NO_DEVICE_ENTRY		9
#define NULL_DEVICE		10
#define NO_TERMINAL_ENTRY	11
#define CANT_UPDATE_DEVICES	12
#define CANT_UPDATE_TERMINAL	13
#define NULL_TERMINAL		14

/* General values */
#define NO_CHAR		'N'
#define YES_CHAR	'Y'
#define DEFAULT_CHAR	'D'

#define SECINWEEK	(60 * 60 * 24 * 7)
#define SECINDAY	(60 * 60 * 24)


/* Structure used for X interface */
/* UNAMELEN is also defined in XMain.h because it is used everywhere */
#ifndef UNAMELEN
#define UNAMELEN	8
#endif

#ifndef NGROUPNAME
#define NGROUPNAME	9
#endif

struct prpw_if {
	struct pr_passwd	prpw;	/* keep a copy of the entry so
					 * we don't have to keep looking
					 * it up.
					 */
};

/* Device assignment field */

struct dev_if {
	struct	dev_asg		dev;	/* keep a copy of the entry so we
					 * don't have to keep looking it up.
					 */
	struct	pr_term		tm;	/* keep a copy of the entry so we don't
					 * have to keep looking it up. Only set
					 * if device is a terminal or host
					 */
};

/*
 *	Fillin structure used for system default processing.
 *
 * O.K. Pretty stupid not defining anything here I know, but this allows
 * for expansion if necessary 
 */

struct	sdef_if  {
	struct	pr_default	df;	/* keep a copy of the entry so we
					 * don't have to keep looking it up.
					 */
};

/* MACROS */
#define ROUND2(i)	( (i + 1) / 2)
#define ROUND13(i)	( (i + 2) / 3)
#define ROUND23(i)	( (2 * (i + 1) ) / 3)

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
