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
static char	*sccsid = "@(#)$RCSfile: XDefClear.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:06:08 $";
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
		XDefClear.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		front end for setting Default User Clearance
		
	entry points:
		DefClearStart()
		DefClearOpen()
		DefClearClose()
		DefClearStop()
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
	CreateDefClearance(),
	SetDefClearance();

static char
	slabel[BUFSIZ];

/* Definitions */

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DefClearStart, DefClearOpen, DefClearClose,
	DefClearStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DefClear");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "accounts,DefClear");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_accounts_default_clearance", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);
	CreateDefClearance(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], DefClearClose) 

static int
LoadVariables ()
{
	int ret;
	char	*sl;
	char	*dflt_sl;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	/* If user's clearance set use that. If not then use the system default.
	 * If no system default then use syslo
	 */
	if (sd.df.prg.fg_clearance) {
		sl      = mand_ir_to_er(&sd.df.prd.fd_clearance);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}
	else {
		/* Choose a default of lowest clearance allowed */
#if SEC_ENCODINGS
		sl      = mand_ir_to_er(mand_minclrnce);
#else
		sl      = mand_ir_to_er(mand_syslo);
#endif
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}

	/* Set the default clearance */
	SetDefClearance(dflt_sl, mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	return (SUCCESS);
}

/* Called from SL code */
void
DefClearOK(w, ptr, info, sl)
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
/* Check the clearance is valid. Return FAILURE if not, store it if OK  */
/************************************************************************/
static int
ValidateEntries ()
{
	struct pr_default *sdef = &sd.df;
	mand_ir_t	*m;

	m = mand_er_to_ir (slabel);
	if (! m)
		return (FAILURE);
	sdef->prg.fg_clearance = 1;
	mand_copy_ir (m, &sdef->prd.fd_clearance);
	mand_free_ir (m);

	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Write that sucker back to the database */
	return (XWriteSystemInfo (&sd));
}

#endif /* SEC_MAC */
#endif /* SEC_BASE */
