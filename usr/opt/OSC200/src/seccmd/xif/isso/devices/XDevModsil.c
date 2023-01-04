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
static char	*sccsid = "@(#)$RCSfile: XDevModsil.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:37 $";
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
		XDevModsil.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevModsil function in role programs.
		
	entry points:
		DevModsilStart()
		DevModsilOpen()
		DevModsilClose()
		DevModsilStop()
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
	CreateDevModsil(),
	SetDevModsil();

static Boolean
	set_ilabel;

static int
	save_mode;

static char
	ilabel[BUFSIZ],
	slabel[BUFSIZ];

/* Definitions */
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevModsilStart, DevModsilOpen, DevModsilClose,
	DevModsilStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w,w1;
	int         i;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevModsil");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevModsil");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_modify_single_level_il", &msg_header, 
			&msg_header_text);
	w = CreateTitle (form_widget, msg_header[0]);

	/**********************************************************************/
	/* Device name                                                        */
	/**********************************************************************/
	device_name_widget = CreateDeviceName (form_widget, w);

	/**********************************************************************/
	/* Create a form to hold the on-off boxes                             */
	/**********************************************************************/
	work_area1_frame  = CreateFrame(form_widget, device_name_widget, True, 
			NULL);

	CreateDevModsil(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], DevModsilClose) 

static int
LoadVariables ()
{
	int i;
	int ret;
	char	*il;
	char	*dflt_il;
	char	*device_il;

	ret = XGetDeviceInfo(chosen_device_name, &dv);
	if (ret)
		return (ret);

	/* If device's cur il set use that. If not then use the system default.
	 * If no system default then use syslo
	 */
	if (dv.dev.uflg.fg_cur_il) {
		il      = ilb_ir_to_er(dv.dev.ufld.fd_cur_il);
		device_il = (char *) Malloc(strlen(il) + 1);
		strcpy (device_il, il);
	}
	else {
		device_il = (char *) Malloc(2);
		strcpy (device_il, " ");
	}

	if (dv.dev.sflg.fg_cur_il) {
		il      = ilb_ir_to_er(dv.dev.sfld.fd_cur_il);
		dflt_il = (char *) Malloc(strlen(il) + 1);
		strcpy (dflt_il, il);
	}
	else {
		il      = ilb_ir_to_er(mand_syslo);
		dflt_il = (char *) Malloc(strlen(il) + 1);
		strcpy (dflt_il, il);
	}

	/* Set the device name */
	SetDeviceName(device_name_widget, dv.dev.ufld.fd_name);

	SetDevModsil((Boolean) dv.dev.uflg.fg_cur_il, dflt_il, device_il,
		mand_syshi->class, mand_syshi->cat);
	free(dflt_il);
	free(device_il);
	return (ret);
}

/* Called from SL code */
#define LABEL_IL	1
#define LABEL_SL	2
#define LABEL_BOTH	3
void
DevModsilOK(w, ptr, info, set_mode, il, sl, il_given)
	Widget 		w;
	caddr_t		ptr;
	caddr_t 	info;
	int		set_mode;
	char		*il;
	char		*sl;
	Boolean		il_given;
{
	/* Save parameters in static storage and then invoke confirmation */
	/* As currently implemented only LABEL_IL should be called. Others
	 * left in for future development ... */
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
	set_ilabel = il_given;
	OKCallback(w, ptr, info);
}


/************************************************************************/
/* Validates data from text widget                                      */
/************************************************************************/
static int
ValidateEntries ()
{
	struct dev_asg	*dev = &dv.dev;
	ilb_ir_t	*m;

	if (set_ilabel) {
		m = ilb_er_to_ir (ilabel);
		if (! m)
			return (FAILURE);
		/* Make sure we have space to store the ir */
		if (! dev->uflg.fg_cur_il)
			dev->ufld.fd_cur_il = mand_alloc_ir();
		dev->uflg.fg_cur_il = 1;
		ilb_copy_ir (m, dev->ufld.fd_cur_il);
	}
	else
		dev->uflg.fg_cur_il = 0;

	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Write information */
	return (XWriteDeviceInfo (&dv));
}

#endif /* SEC_ILB */
#endif /* SEC_BASE */
