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
 *	@(#)$RCSfile: XMacros.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:07 $
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

#ifndef __XMacros__
#define __XMacros__

/*
	filename:
		XMacros.h
	
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		Defines macros for use by all the role programs
*/

/* Requires that Widget be defined */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

/* Macro to define the start, open, close and stop routines */
#define CREATE_SCREEN_ROUTINES(START, OPEN, CLOSE, STOP)		\
/* External routines */							\
extern void   WorkingOpen();						\
extern void   WorkingClose();						\
extern void   CenterForm();						\
extern void   ErrorMessageOpen();					\
extern void   HelpDisplayOpen();					\
extern void   CreateThreeButtons();					\
extern Widget CreateForm();						\
extern Widget CreateTitle();						\
extern Widget CreateHeader();						\
extern Widget CreateFrame();						\
extern Widget CreateConfirmationBox();					\
									\
/* Local routines */							\
static int								\
	LoadVariables(),						\
	ValidateEntries(),						\
	WriteInformation();						\
									\
static void								\
	MakeWidgets(),							\
	OKCallback(), 							\
	CancelCallback(),						\
	ConfirmOKCallback(),						\
	ConfirmCancelCallback();					\
									\
/* Local variables */							\
static int 								\
	confirmation_open,						\
	form_open;							\
									\
static Widget								\
	form_widget,							\
	confirmation_widget;						\
									\
/* Initialize variables that need to be initialized;			\
 * called once at startup time						\
 */									\
void 									\
START ()                    		 				\
{									\
	confirmation_open = FALSE;					\
	form_open = FALSE;						\
}									\
									\
/* Main entry point for the screen					\
 */									\
 									\
void 									\
OPEN ()                 						\
{									\
	XtSetSensitive(main_menubar, False);				\
	WorkingOpen(main_shell);        				\
									\
	/* First time -- create the window */				\
	if (! form_open) {						\
		MakeWidgets();						\
		form_open = TRUE;					\
	}								\
									\
	/* Load the screen specific values - if error then return */    \
	if (LoadVariables() )          					\
		return;							\
									\
	CenterForm(form_widget);					\
	WorkingClose();							\
}									\
									\
/* Clean up after finished with the screen.       			\
 * frees transient memory, gets rid of screen and returns control 	\
 * to the menubar.							\
 */									\
void 									\
CLOSE()									\
{									\
	/* Sensitize the menu bar */					\
	XtSetSensitive(main_menubar, True);				\
	XtUnmanageChild(form_widget);					\
									\
	/* Free memory */						\
	if (save_memory) {						\
		if (form_open) {					\
			XtDestroyWidget(form_widget);			\
			form_open = FALSE;				\
		}							\
		if (confirmation_open) {				\
			XtDestroyWidget(confirmation_widget);		\
			confirmation_open = FALSE;			\
		}							\
	}								\
}									\
									\
/* Called at exit for final clean up */					\
void 									\
STOP() 									\
{									\
	/* Null operation. Program is about to halt - nothing to do 	\
	*/								\
}


/* 
 * Create standard OK, Cancel, Help callbacks
 */
#define CREATE_CALLBACKS(CONFIRM_MESSAGE,CLOSE_ROUTINE)          	\
static void 								\
OKCallback(w, ptr, info) 						\
	Widget		w; 						\
	caddr_t		ptr;						\
	caddr_t		info;						\
{									\
	if (ValidateEntries() != SUCCESS)      				\
		return;							\
									\
	/* Create the confirmation box */				\
	XtSetSensitive (form_widget, False);				\
	if (! confirmation_open) {					\
		confirmation_open = TRUE;				\
		confirmation_widget = CreateConfirmationBox(mother_form,\
			        CONFIRM_MESSAGE);			\
		XtAddCallback(confirmation_widget, XmNokCallback, 	\
			                ConfirmOKCallback,NULL);	\
		XtAddCallback(confirmation_widget, XmNcancelCallback,	\
			                ConfirmCancelCallback,NULL);	\
	}								\
	else  								\
		XtManageChild(confirmation_widget);			\
}									\
									\
static void 								\
ConfirmOKCallback(w, ptr, info) 					\
	Widget		w; 						\
	caddr_t		ptr;						\
	caddr_t		info;						\
{									\
	XtSetSensitive(form_widget, True);	  			\
	XtUnmanageChild(confirmation_widget);				\
	if (! WriteInformation() )					\
		CLOSE_ROUTINE();  					\
}									\
									\
static void 								\
ConfirmCancelCallback(w, ptr, info) 					\
	Widget		w; 						\
	caddr_t		ptr;						\
	caddr_t		info;						\
{									\
	XtSetSensitive(form_widget, True);				\
	XtUnmanageChild(confirmation_widget);				\
}									\
									\
static void CancelCallback(w, ptr, info) 				\
	Widget		w; 						\
	caddr_t		ptr;						\
	caddr_t		info;						\
{									\
	CLOSE_ROUTINE();  						\
}

#endif /* __XMacros__ */
#endif /* SEC_BASE */
