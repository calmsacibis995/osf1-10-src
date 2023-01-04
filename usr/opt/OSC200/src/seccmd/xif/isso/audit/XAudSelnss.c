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
static char	*sccsid = "@(#)$RCSfile: XAudSelnss.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:03 $";
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

#include <sys/secdefines.h>
#if SEC_BASE
#if SEC_MAC && ! SEC_SHW


/*
	filename:
		XAudSelnss.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for AudSelnss function in role programs.
		
	entry points:
		AudSelnssStart()
		AudSelnssOpen()
		AudSelnssClose()
		AudSelnssStop()
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
#include <stdio.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include <mandatory.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XAudit.h"
#include "XMacros.h"

extern void
	SelectionReset(),
	CreateAudSelnss(),
	SetAudSelnss();

extern 	AUDIT_SELECTION_STRUCT	selection_fillin;

static char
	slabel[BUFSIZ];

/* Definitions */
CREATE_ACCOUNTS_HEADER

/* N.B. The Close routine is defined separately. AudSelnssJunk is not
 * used anywhere */
CREATE_SCREEN_ROUTINES (AudSelnssStart, AudSelnssOpen, AudSelnssJunk,
	AudSelnssStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "AudSelnss");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "audit,AudSelnss");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_audit_selection_min_subject_sl", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);

	CreateAudSelnss(work_area1_frame);
}

/* Clean up after finished with the screen.
 * frees transient memory, gets rid of screen and returns control
 * to the menubar.
 */
void
AudSelnssClose()
{
	/* Note we do not sensitize the menu bar */
	/*    XtSetSensitive(main_menubar, True); */

	XtUnmanageChild(form_widget);

	/* Free memory */
	if (save_memory) {
		if (form_open) {
			XtDestroyWidget(form_widget);
			form_open = FALSE;
		}
		if (confirmation_open) {
			XtDestroyWidget(confirmation_widget);
			confirmation_open = FALSE;
		}
	}
	/* Redisplay the main Audit Selection window */
	SelectionReset();
}

CREATE_CALLBACKS (msg_header[1], AudSelnssClose) 

static int
LoadVariables ()
{
	char	*sl;
	char	*dflt_sl;

	sl      = mand_ir_to_er(selection_fillin.slevel_min);
	dflt_sl = (char *) Malloc(strlen(sl) + 1);
	strcpy (dflt_sl, sl);

	SetAudSelnss(dflt_sl, mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	return (SUCCESS);
}

/* Called from SL code */
void
AudSelnssOK(w, ptr, info, sl)
	Widget 		w;
	caddr_t		ptr;
	caddr_t 	info;
	char		*sl;
{
	/* Save parameters in static storage and then invoke confirmation */
	strcpy (slabel, sl);
	OKCallback(w, ptr, info);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	mand_ir_t	*m;

	m = mand_er_to_ir (slabel);
	if (! m)
		return (FAILURE);
	mand_copy_ir (m, selection_fillin.slevel_min);
	mand_free_ir (m);
	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Nothing to do here. We have saved the pointer */
	return (SUCCESS);
}

#endif /* SEC_MAC */
#endif /* SEC_BASE */
