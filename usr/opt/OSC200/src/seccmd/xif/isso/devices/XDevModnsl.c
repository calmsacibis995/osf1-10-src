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
static char	*sccsid = "@(#)$RCSfile: XDevModnsl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:10:34 $";
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
		XDevModnsl.c
		
	copyright:
		Copyright (c) 1989-1990 SKM, L.P.
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for DevModnsl function in role programs.
		
	entry points:
		DevModnslStart()
		DevModnslOpen()
		DevModnslClose()
		DevModnslStop()
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
	CreateDevModnsl(),
	SetDevModnsl();

static Boolean
	set_slabel;

static char
	slabel[BUFSIZ];

/* Definitions */
CREATE_ACCOUNTS_HEADER

CREATE_SCREEN_ROUTINES (DevModnslStart, DevModnslOpen, DevModnslClose,
	DevModnslStop)

static void 
MakeWidgets() 
{
	Widget      work_area1_frame,
    		    w,w1;
	int         i;
	Dimension   max_label_width;

	/**********************************************************************/
	/* Bulletin widget to hold everything                                 */
	/**********************************************************************/
	form_widget = CreateForm(mother_form, "DevModnsl");
	XtAddCallback(form_widget, XmNhelpCallback,
				 HelpDisplayOpen, "devices,DevModnsl");
	
	/**********************************************************************/
	/* Title label                                                        */
	/**********************************************************************/
	if (! msg_header)
		LoadMessage ("msg_devices_modify_multi_level_min", &msg_header, 
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

	CreateDevModnsl(work_area1_frame);
}

CREATE_CALLBACKS (msg_header[1], DevModnslClose) 

static int
LoadVariables ()
{
	int i;
	int ret;
	char	*sl;
	char	*dflt_sl;
	char	*device_sl;

	ret = XGetDeviceInfo(chosen_device_name, &dv);
	if (ret)
		return (ret);

	/* If device's min sl set use that. If not then use the system default.
	 * If no system default then use syslo
	 */
	if (dv.dev.uflg.fg_min_sl) {
		sl      = mand_ir_to_er(dv.dev.ufld.fd_min_sl);
		device_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (device_sl, sl);
	}
	else {
		device_sl = (char *) Malloc(2);
		strcpy (device_sl, " ");
	}

	if (dv.dev.sflg.fg_min_sl) {
		sl      = mand_ir_to_er(dv.dev.sfld.fd_min_sl);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}
	else {
		sl      = mand_ir_to_er(mand_syslo);
		dflt_sl = (char *) Malloc(strlen(sl) + 1);
		strcpy (dflt_sl, sl);
	}

	/* Set the device name */
	SetDeviceName(device_name_widget, dv.dev.ufld.fd_name);

	SetDevModnsl(dv.dev.uflg.fg_min_sl, dflt_sl, device_sl,
		mand_syshi->class, mand_syshi->cat);
	free(dflt_sl);
	free(device_sl);
	return (ret);
}

/* Called from SL code */
void
DevModnslOK(w, ptr, info, sl, sl_given)
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
	struct dev_asg	*dev = &dv.dev;
	mand_ir_t	*m;

	if (set_slabel) {
		m = mand_er_to_ir (slabel);
		if (! m)
			return (FAILURE);
		/* Make sure we have space to store the ir */
		if (! dev->uflg.fg_min_sl)
			dev->ufld.fd_min_sl = mand_alloc_ir();
		dev->uflg.fg_min_sl = 1;
		mand_copy_ir (m, dev->ufld.fd_min_sl);
	}
	else
		dev->uflg.fg_min_sl = 0;

	return (SUCCESS);
}

static int
WriteInformation ()
{
	/* Write information */
	return (XWriteDeviceInfo (&dv));
}

#endif /* SEC_MAC && ! SEC_SHW */
#endif /* SEC_BASE **/
