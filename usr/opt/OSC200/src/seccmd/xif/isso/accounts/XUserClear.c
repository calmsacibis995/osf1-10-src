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
static char	*sccsid = "@(#)$RCSfile: XUserClear.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:05 $";
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
#if SEC_MAC


/*
	filename:
		XUserClear.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting user clearance 
		
	entry points:
		UserClearStart()
		UserClearOpen()
		UserClearClose()
		UserClearStop()
*/

/* Common C include files */
#include <sys/types.h>
#include <sys/security.h>
#include <stdio.h>

/* X Include files */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>

#include <mandatory.h>

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"
#include "XMacros.h"

extern void
	CreateUserClearance(),
	SetUserClearance();

static Boolean
	set_slabel;

static char
	slabel[BUFSIZ];

/* Definitions */
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (UserClearStart, UserClearOpen, UserClearClose,
	UserClearStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;
	Dimension   max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "UserClear");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "accounts,UserClear");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_user_clearance", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* User name                                                          */
	/**********************************************************************/
	user_name_widget = CreateUserName (form_widget, w);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, user_name_widget, True, 
			NULL);

	CreateUserClearance(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], UserClearClose) 

static int
LoadVariables ()
{
	int i;
	int ret;
	char	*sl;
	char	*dflt_sl;
	char	*user_sl;

	ret = XGetUserInfo(chosen_user_name, &pr);
	if (ret)
		return (ret);

	/* If user's clearance set use that. If not then use the system default.
	 * If no system default then use syslo
	 */
	if (pr.prpw.uflg.fg_clearance) {
		sl      = mand_ir_to_er(&pr.prpw.ufld.fd_clearance);
		user_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (user_sl, sl);
	}
	else {
		user_sl = (char *) Malloc(2);
		strcpy (user_sl, " ");
	}

	if (pr.prpw.sflg.fg_clearance) {
		sl      = mand_ir_to_er(&pr.prpw.sfld.fd_clearance);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}
	else {
#if SEC_ENCODINGS
		sl      = mand_ir_to_er(mand_minclrnce);
#else
		sl      = mand_ir_to_er(mand_syslo);
#endif
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}

	/* Set the user name */
	SetUserName(user_name_widget, pr.prpw.ufld.fd_name);

	SetUserClearance(pr.prpw.uflg.fg_clearance, dflt_sl, user_sl,
		mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	free(user_sl);
	return (ret);
}

/* Called from SL code */
void
UserClearOK(w, ptr, info, sl, sl_given)
	Widget 		w;
	caddr_t		ptr;
	caddr_t 	info;
	char		*sl;
	Boolean		sl_given;
{
	/* Save parameters in static storage and then invoke confirmation */
	strcpy (slabel, sl);
	set_slabel = sl_given;
	OKCallback(w, ptr, info);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	struct pr_passwd *prpwd = &pr.prpw;
	mand_ir_t	*m;

	if (set_slabel) {
		m = mand_er_to_ir (slabel);
		if (! m)
		return (FAILURE);
		prpwd->uflg.fg_clearance = 1;
		mand_copy_ir (m, &prpwd->ufld.fd_clearance);
		mand_free_ir (m);
	}
	else
		prpwd->uflg.fg_clearance = 0;

	return (SUCCESS);
}

static int
WriteInformation ()
{
	return (XWriteUserInfo (&pr));
}

#endif /* SEC_MAC */
#endif /* SEC_BASE */
