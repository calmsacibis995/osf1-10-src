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
static char	*sccsid = "@(#)$RCSfile: XDevDefssl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:47 $";
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
		XDevDefssl.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevDefssl function in role programs.
		
	entry points:
		DevDefsslStart()
		DevDefsslOpen()
		DevDefsslClose()
		DevDefsslStop()
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
	CreateDevDefssl(),
	SetDevDefssl();

static char
	slabel[BUFSIZ];

/* Definitions */

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevDefsslStart, DevDefsslOpen, DevDefsslClose,
	DevDefsslStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevDefssl");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "devices,DevDefssl");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_default_single_level_sl", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);

	CreateDevDefssl(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], DevDefsslClose) 

static int
LoadVariables ()
{
	int i;
	int ret;
	char	*sl;
	char	*dflt_sl;
	char	*user_sl;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);


	if (sd.df.devg.fg_cur_sl) {
		sl      = mand_ir_to_er(sd.df.devd.fd_cur_sl);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}
	else {
		sl      = mand_ir_to_er(mand_syslo);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}

	/* Set the user name */
	SetDevDefssl(dflt_sl, mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	return (ret);
}

/* Called from SL code */
void
DevDefsslOK(w, ptr, info, sl)
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
	struct pr_default *sdef = &sd.df;
	mand_ir_t	*m;

	m = mand_er_to_ir (slabel);
	if (! m)
		return (FAILURE);
	/* Make sure we have space to store the ir */
	if (! sdef->devg.fg_cur_sl)
		sdef->devd.fd_cur_sl = mand_alloc_ir();
	sdef->devg.fg_cur_sl = 1;
	mand_copy_ir (m, sdef->devd.fd_cur_sl);
	mand_free_ir (m);

	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Write information */
	return (XWriteSystemInfo (&sd));
}

#endif /* SEC_MAC */
#endif /* SEC_BASE **/
