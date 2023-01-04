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
static char	*sccsid = "@(#)$RCSfile: XDevDefsil.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:09:44 $";
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
#if SEC_ILB


/*
	filename:
		XDevDefsil.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevDefsil function in role programs.
		
	entry points:
		DevDefsilStart()
		DevDefsilOpen()
		DevDefsilClose()
		DevDefsilStop()
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
	CreateDevDefsil(),
	SetDevDefsil();

static int
	save_mode;

static char
	ilabel[BUFSIZ],
	slabel[BUFSIZ];

/* Definitions */

CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevDefsilStart, DevDefsilOpen, DevDefsilClose,
	DevDefsilStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevDefsil");
	XtAddCallback(form_widget, XmNhelpCallback,
					 HelpDisplayOpen, "devices,DevDefsil");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_default_single_level_il", 
			&msg_header, &msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, w, True, NULL);

	CreateDevDefsil(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], DevDefsilClose) 

static int
LoadVariables ()
{
	int i;
	int ret;
	char	*il;
	char	*dflt_il;
	char	*dev_il;

	ret = XGetSystemInfo(&sd);
	if (ret)
		return (ret);

	if (sd.df.devg.fg_cur_il) {
		il      = ilb_ir_to_er(sd.df.devd.fd_cur_il);
		dflt_il = (char *) Malloc(strlen(il) + 1);
		strcpy (dflt_il, il);
	}
	else {
		il      = ilb_ir_to_er(mand_syslo);
		dflt_il = (char *) Malloc(strlen(il) + 1);
		strcpy (dflt_il, il);
	}

	SetDevDefsil(dflt_il, dflt_il, mand_syshi->class, mand_syshi->cat);
	free(dflt_il);
	return (ret);
}

/* Called from SL code */
#define LABEL_IL	1
#define LABEL_SL	2
#define LABEL_BOTH	3
void
DevDefsilOK(w, ptr, info, set_mode, il, sl)
	Widget 		w;
	caddr_t		ptr;
	caddr_t 	info;
	int		set_mode;
	char		*il;
	char		*sl;
{
	/* Save parameters in static storage and then invoke confirmation */
	switch (set_mode) {
		case LABEL_BOTH:
			strcpy (ilabel, il);
			strcpy (slabel, sl);
			break;

		case LABEL_IL:
			strcpy (ilabel, il);
			break;

		case LABEL_SL:
			strcpy (slabel, sl);
			break;
	}

	save_mode = set_mode;
	OKCallback(w, ptr, info);
}

/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	struct pr_default *sdef = &sd.df;
	ilb_ir_t	*m;

#ifdef DEBUG
	if (ilabel)
		printf ("ilabel %s\n", ilabel);
	if (slabel)
		printf ("slabel %s\n", slabel);
#endif
	m = ilb_er_to_ir (ilabel);
	if (! m)
		return (FAILURE);
	/* Make sure we have space to store the ir */
	if (! sdef->devg.fg_cur_il)
		sdef->devd.fd_cur_il = ilb_alloc_ir();
	sdef->devg.fg_cur_il = 1;
	ilb_copy_ir (m, sdef->devd.fd_cur_il);
	ilb_free_ir (m);

	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Write information */
	return (XWriteSystemInfo (&sd));
}

#endif /* ILB */
#endif /* SEC_BASE **/
