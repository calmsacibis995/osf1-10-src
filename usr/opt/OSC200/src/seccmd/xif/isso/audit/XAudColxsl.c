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
static char	*sccsid = "@(#)$RCSfile: XAudColxsl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:56 $";
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
		XAudColxsl.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for AudColxsl function in role programs.
		
	entry points:
		AudColxslStart()
		AudColxslOpen()
		AudColxslClose()
		AudColxslStop()
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

extern int
	XGetAuditSL(),
	XWriteAuditSL();

extern void
	CreateAudColxsl(),
	SetAudColxsl();

AUDIT_SENSITIVITY_STRUCT audit_fillin;

static char
	*msg_error_1,	
	**msg_error_text_1;

static char
	slabel[BUFSIZ];

/* Definitions */
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (AudColxslStart, AudColxslOpen, AudColxslClose,
	AudColxslStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "AudColxsl");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "audit,AudColxsl");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_audit_collection_max_sl", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);

	CreateAudColxsl(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], AudColxslClose) 

static int
LoadVariables ()
{
	int ret;
	char	*sl;
	char	*dflt_sl;

	ret = XGetAuditSL(&audit_fillin);
	if (ret)
		return (ret);

	/* XGetAuditSL sets max ir ptr. If none in audit file then syshi is
	 * used.  */
	sl      = mand_ir_to_er(audit_fillin.max_ir_ptr);
	dflt_sl = (char *) Malloc(strlen(sl) + 1);
	strcpy (dflt_sl, sl);

	SetAudColxsl(dflt_sl, mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	return (SUCCESS);
}

/* Called from SL code */
void
AudColxslOK(w, ptr, info, sl, this, future)
	Widget 		w;
	caddr_t		ptr;
	caddr_t 	info;
	char		*sl;
	Boolean		this;
	Boolean		future;
{
	/* Save parameters in static storage and then invoke confirmation */
	strcpy (slabel, sl);
	audit_fillin.this_session = (char) this;
	audit_fillin.future_sessions = (char) future;
	OKCallback(w, ptr, info);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	mand_ir_t	*m;

	/* Must select either future or current sessions */
	if (! audit_fillin.future_sessions && ! audit_fillin.this_session) {
		if (! msg_error_1)
			LoadMessage ("msg_audit_current_or_future_error",
				&msg_error_1, &msg_error_text_1);
		ErrorMessageOpen(-1, msg_error_1, 0, NULL);
		return (FAILURE);
	}

	m = mand_er_to_ir (slabel);
	if (! m)
		return (FAILURE);
	mand_copy_ir (m, audit_fillin.max_ir_ptr);
	mand_free_ir (m);
	return (SUCCESS);
}

static int
WriteInformation ()
{
	return (XWriteAuditSL(&audit_fillin));
}

#endif /* SEC_MAC */
#endif /* SEC_BASE */
