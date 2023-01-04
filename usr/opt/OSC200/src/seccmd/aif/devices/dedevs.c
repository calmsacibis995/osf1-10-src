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
static char	*sccsid = "@(#)$RCSfile: dedevs.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:10 $";
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
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*
 * Routines to implement printer parameters screen
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfDevices.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"
#include "dedevs.h"	/* screen layout */

/* static routine definitions */

static int SLSingleToggle();
static int SLMultiToggle();
static int DeviceExpand();
static void init_device_table();
static void init_sp_name_list();
static int TermUlogToggle();
static int TermLogTimeoutToggle();
static int TermLogDelayToggle();
static int TermUlogType();
static int TermLogTimeoutType();
static int TermLogDelayType();

/* structures defined in de_scrns.c */

extern Scrn_parms	deptr_scrn;
extern Scrn_desc	deptr_desc[];
extern Scrn_parms	dermv_scrn;
extern Scrn_desc	dermv_desc[];
extern Scrn_parms	detrm_scrn;
extern Scrn_desc	detrm_desc[];

/* screen description common to all screens */

#define TITLE_DESC		0
#define NAME_DESC		1
#define DEVLIST_TITLE_DESC	2
#define DEVLIST_DESC		3

#define NAME_STRUCT		0
#define DEVLIST_STRUCT		1

/* The rest of the screen depends on the device type.  For a printer: */

/* screen titles for printers in de_scrns.c */

extern uchar deadp_title[];
extern uchar demdp_title[];
extern uchar dermp_title[];
extern uchar dedsp_title[];

/* screen titles for removables in de_scrns.c */

extern uchar deadr_title[];
extern uchar demdr_title[];
extern uchar dedsr_title[];
extern uchar dermr_title[];

/* screen titles for terminals in de_scrns.c */

extern uchar deadt_title[];
extern uchar demdt_title[];
extern uchar dermt_title[];
extern uchar dedst_title[];

/* set to the current elements used for this screen */

static DeviceFillin *device_fill;
static Scrn_parms *device_scrn;

static enum device_scrn_mode	DeviceMode;
static char **DeviceTable;
static int NDevices;
static int IsISSO;

/********************************************************
 *							*
 *   The following routines are specific to printers:	* 
 *							*
 ********************************************************/

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
deptr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deptr_bfill");

	if (DeviceMode == DeviceCreate) {
		ret = DevCreateFillin(PrinterDevice, defill);
		deptr_desc[NAME_DESC].inout = FLD_BOTH;
	} else {
		ret = DevGetFillin(DevSelected, defill);
		deptr_desc[NAME_DESC].inout = FLD_OUTPUT;
	}

	init_device_table(defill);

	/* set screen title */

	switch (DeviceMode) {
	case DeviceCreate:
		deptr_scrn.sh->title = deadp_title;
		break;
	case DeviceUpdate:
		deptr_scrn.sh->title = demdp_title;
		break;
	case DeviceDisplay:
		deptr_scrn.sh->title = dedsp_title;
		break;
	case DeviceRemove:
		deptr_scrn.sh->title = dermp_title;
		break;
	}

	/* set edit-ability depending on mode */

	switch (DeviceMode) {
	case DeviceCreate:
	case DeviceUpdate:
		deptr_scrn.scrntype = SCR_FILLIN;
		break;
	case DeviceDisplay:
	case DeviceRemove:
		deptr_scrn.scrntype = SCR_NOCHANGE;
		break;
	}

	device_scrn = &deptr_scrn;

	EXITFUNC("deptr_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
deptr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("deptr_bstruct");

	sp = deptr_scrn.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	init_sp_name_list(sp, defill);

#if SEC_MAC
	/*
	 * set changed on both of these so toggling either will update
	 * both fields.
	 */
	sp[PTR_SINGLE_SL_STRUCT].pointer = &defill->asg_sl_sl;
	sp[PTR_SINGLE_SL_STRUCT].filled = 1;
	sp[PTR_SINGLE_SL_STRUCT].val_act = SLSingleToggle;
	sp[PTR_SINGLE_SL_STRUCT].changed = 1;

	sp[PTR_MULTI_SL_STRUCT].pointer = &defill->asg_ml_sl;
	sp[PTR_MULTI_SL_STRUCT].filled = 1;
	sp[PTR_MULTI_SL_STRUCT].val_act = SLMultiToggle;
	sp[PTR_MULTI_SL_STRUCT].changed = 1;
#endif /* SEC_MAC */

	EXITFUNC("deptr_bstruct");
	return 0;
}

/********************************************************
 *							*
 *   The following routines are specific to removables:	* 
 *							*
 ********************************************************/

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dermv_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermv_bfill");


	if (DeviceMode == DeviceCreate) {
		ret = DevCreateFillin(RemovableDevice, defill);
		dermv_desc[NAME_DESC].inout = FLD_BOTH;
	} else {
		ret = DevGetFillin(DevSelected, defill);
		dermv_desc[NAME_DESC].inout = FLD_OUTPUT;
	}

	init_device_table(defill);

	/* set screen title */

	switch (DeviceMode) {
	case DeviceCreate:
		dermv_scrn.sh->title = deadr_title;
		break;
	case DeviceUpdate:
		dermv_scrn.sh->title = demdr_title;
		break;
	case DeviceDisplay:
		dermv_scrn.sh->title = dedsr_title;
		break;
	case DeviceRemove:
		dermv_scrn.sh->title = dermr_title;
		break;
	}

	/* set edit-ability depending on mode */

	switch (DeviceMode) {
	case DeviceCreate:
	case DeviceUpdate:
		dermv_scrn.scrntype = SCR_FILLIN;
		break;
	case DeviceDisplay:
	case DeviceRemove:
		dermv_scrn.scrntype = SCR_NOCHANGE;
		break;
	}

	device_scrn = &dermv_scrn;

	EXITFUNC("dermv_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dermv_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("dermv_bstruct");

	sp = dermv_scrn.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	init_sp_name_list(sp, defill);

#if SEC_MAC
	sp[RMV_SINGLE_SL_STRUCT].pointer = &defill->asg_sl_sl;
	sp[RMV_SINGLE_SL_STRUCT].filled = 1;
	sp[RMV_SINGLE_SL_STRUCT].val_act = SLSingleToggle;
	sp[RMV_SINGLE_SL_STRUCT].changed = 1;

	sp[RMV_MULTI_SL_STRUCT].pointer = &defill->asg_ml_sl;
	sp[RMV_MULTI_SL_STRUCT].filled = 1;
	sp[RMV_MULTI_SL_STRUCT].val_act = SLMultiToggle;
	sp[RMV_MULTI_SL_STRUCT].changed = 1;
#endif /* SEC_MAC */
#if SEC_ARCH
	sp[RMV_CTL_IMP_TOG_STRUCT].pointer = &defill->import_enable;
	sp[RMV_CTL_IMP_TOG_STRUCT].filled = 1;

	sp[RMV_CTL_EXP_TOG_STRUCT].pointer = &defill->export_enable;
	sp[RMV_CTL_EXP_TOG_STRUCT].filled = 1;
#endif /* SEC_ARCH */

	EXITFUNC("dermv_bstruct");
	return 0;
}

/********************************************************
 *							*
 *   The following routines are specific to terminals:	* 
 *							*
 ********************************************************/

/*
 * Called when user selects the default Unsuccessful login toggle
 */

static int
TermUlogToggle()
{
	int ret;

	if (device_fill->def_max_ulogins) {

		/* turned on default -- disable typing */

		device_fill->max_ulogins =
			device_fill->def_max_ulogins_value;
		detrm_desc[TRM_ULOG_VALUE_DESC].inout = FLD_OUTPUT;

		ret = 1;	/* redraw */

	} else {
		
		/* turned off default -- re-enable typing */

		detrm_desc[TRM_ULOG_VALUE_DESC].inout = FLD_BOTH;

		ret = 0;
	}
	return ret;
}

/*
 * Called when the user starts typing a value in the Unsuccessful login field
 */

static int
TermUlogType()
{
	int ret;

	if (device_fill->def_max_ulogins) {
		device_fill->def_max_ulogins = 0;
		ret = 1;
	} else
		ret = 0;
	return ret;
}

/*
 * Called when the user selectes the default login timeout toggle
 */

static int
TermLogTimeoutToggle()
{
	int ret;

	if (device_fill->def_login_timeout) {
		device_fill->login_timeout =
			device_fill->def_login_timeout_value;
		detrm_desc[TRM_LOGTIME_VALUE_DESC].inout = FLD_OUTPUT;
		ret = 1;
	} else {
		detrm_desc[TRM_LOGTIME_VALUE_DESC].inout = FLD_BOTH;
		ret = 0;
	}
	return ret;
}

/*
 * Called when the user starts typing in the login timeout field
 */

static int
TermLogTimeoutType()
{
	int ret;

	if (device_fill->def_login_timeout) {
		device_fill->def_login_timeout = 0;
		ret = 1;
	} else
		ret = 0;

	return ret;
}

/*
 * Called when the user selects the default login delay toggle
 */

static int
TermLogDelayToggle()
{
	int ret;

	if (device_fill->def_login_delay) {
		device_fill->login_delay = 
			device_fill->def_login_delay_value;
		detrm_desc[TRM_LOGDEL_VALUE_DESC].inout = FLD_OUTPUT;
		ret = 1;
	} else {
		detrm_desc[TRM_LOGDEL_VALUE_DESC].inout = FLD_BOTH;
		ret = 0;
	}
	return ret;
}

/*
 * Called when the user starts typing in the login delay field
 */

static int
TermLogDelayType()
{
	int ret;

	if (device_fill->def_login_delay) {
		device_fill->def_login_delay = 0;
		ret = 1;
	} else
		ret = 0;

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
detrm_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("detrm_bfill");


	if (DeviceMode == DeviceCreate) {
		ret = DevCreateFillin(TerminalDevice, defill);
		detrm_desc[NAME_DESC].inout = FLD_BOTH;
	} else {
		ret = DevGetFillin(DevSelected, defill);
		detrm_desc[NAME_DESC].inout = FLD_OUTPUT;
	}

	init_device_table(defill);

	/* set screen title */

	switch (DeviceMode) {
	case DeviceCreate:
		detrm_scrn.sh->title = deadt_title;
		break;
	case DeviceUpdate:
		detrm_scrn.sh->title = demdt_title;
		break;
	case DeviceDisplay:
		detrm_scrn.sh->title = dedst_title;
		break;
	case DeviceRemove:
		detrm_scrn.sh->title = dermt_title;
		break;
	}

	/* set edit-ability depending on mode */

	switch (DeviceMode) {
	case DeviceCreate:
	case DeviceUpdate:
		detrm_scrn.scrntype = SCR_FILLIN;
		break;
	case DeviceDisplay:
	case DeviceRemove:
		detrm_scrn.scrntype = SCR_NOCHANGE;
		break;
	}

	device_scrn = &detrm_scrn;

	EXITFUNC("detrm_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
detrm_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("dermv_bstruct");

	sp = detrm_scrn.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	init_sp_name_list(sp, defill);

	/* set changed on many so they will update when toggling "default" */

	sp[TRM_ULOG_VALUE_STRUCT].pointer = (char *) &defill->max_ulogins;
	sp[TRM_ULOG_VALUE_STRUCT].filled = 1;
	sp[TRM_ULOG_VALUE_STRUCT].changed= 1;
	sp[TRM_ULOG_VALUE_STRUCT].val_act= TermUlogType;

	sp[TRM_ULOG_DEFTOG_STRUCT].pointer = &defill->def_max_ulogins;
	sp[TRM_ULOG_DEFTOG_STRUCT].filled = 1;
	sp[TRM_ULOG_DEFTOG_STRUCT].changed = 1;
	sp[TRM_ULOG_DEFTOG_STRUCT].val_act = TermUlogToggle;

	sp[TRM_ULOG_DEFVAL_STRUCT].pointer =
	  (char *) &defill->def_max_ulogins_value;
	sp[TRM_ULOG_DEFVAL_STRUCT].filled = 1;

	sp[TRM_LOGTIME_VALUE_STRUCT].pointer = (char *) &defill->login_timeout;
	sp[TRM_LOGTIME_VALUE_STRUCT].filled = 1;
	sp[TRM_LOGTIME_VALUE_STRUCT].changed = 1;
	sp[TRM_LOGTIME_VALUE_STRUCT].val_act = TermLogTimeoutType;

	sp[TRM_LOGTIME_DEFTOG_STRUCT].pointer = &defill->def_login_timeout;
	sp[TRM_LOGTIME_DEFTOG_STRUCT].filled = 1;
	sp[TRM_LOGTIME_DEFTOG_STRUCT].changed = 1;
	sp[TRM_LOGTIME_DEFTOG_STRUCT].val_act = TermLogTimeoutToggle;

	sp[TRM_LOGTIME_DEFVAL_STRUCT].pointer =
	  (char *) &defill->def_login_timeout_value;
	sp[TRM_LOGTIME_DEFVAL_STRUCT].filled = 1;

	sp[TRM_LOGDEL_VALUE_STRUCT].pointer = (char *) &defill->login_delay;
	sp[TRM_LOGDEL_VALUE_STRUCT].filled = 1;
	sp[TRM_LOGDEL_VALUE_STRUCT].changed = 1;
	sp[TRM_LOGDEL_VALUE_STRUCT].val_act = TermLogDelayType;

	sp[TRM_LOGDEL_DEFTOG_STRUCT].pointer = &defill->def_login_delay;
	sp[TRM_LOGDEL_DEFTOG_STRUCT].filled = 1;
	sp[TRM_LOGDEL_DEFTOG_STRUCT].changed = 1;
	sp[TRM_LOGDEL_DEFTOG_STRUCT].val_act = TermLogDelayToggle;

	sp[TRM_LOGDEL_DEFVAL_STRUCT].pointer =
	  (char *) &defill->def_login_delay_value;
	sp[TRM_LOGDEL_DEFVAL_STRUCT].filled = 1;

#if SEC_MAC
	sp[TRM_SINGLE_SL_STRUCT].pointer = &defill->asg_sl_sl;
	sp[TRM_SINGLE_SL_STRUCT].filled = 1;
	sp[TRM_SINGLE_SL_STRUCT].val_act = SLSingleToggle;
	sp[TRM_SINGLE_SL_STRUCT].changed = 1;

	sp[TRM_MULTI_SL_STRUCT].pointer = &defill->asg_ml_sl;
	sp[TRM_MULTI_SL_STRUCT].filled = 1;
	sp[TRM_MULTI_SL_STRUCT].val_act = SLMultiToggle;
	sp[TRM_MULTI_SL_STRUCT].changed = 1;
#endif /* SEC_MAC */

	EXITFUNC("detrm_bstruct");
	return 0;
}

/************************************************************************
 *									*
 * Everything below this line is common to all device types		*
 *									*
 ************************************************************************/

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

int
device_auth(defill, device_mode)
	struct device_fillin *defill;
	enum device_scrn_mode	device_mode;
{
	static int first_time = 1;

	ENTERFUNC("device_auth");

	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}

	DeviceMode = device_mode;

	/* if creating a device, need to remove any current selection */

	if (DeviceMode == DeviceCreate)
		DeselectDevice();

	/* save the fillin for future manipulation */

	device_fill = defill;

	EXITFUNC("device_auth");

	if (IsISSO)
		return 0;
	else
		return 1;
}

#if SEC_MAC
/* Toggle functions for the single/multi SL items */

static int
SLSingleToggle()
{
	device_fill->asg_sl_sl = 1;
	device_fill->asg_ml_sl = 0;
	return 1;
}

static int
SLMultiToggle()
{
	device_fill->asg_sl_sl = 0;
	device_fill->asg_ml_sl = 1;
	return 1;
}
#endif

/* Expand the Devices table, if necessary */

static int
DeviceExpand()
{
	char **device_table;
	struct scrn_struct *sp = device_scrn->ss;

	/* only expand if the user just typed in the last blank */

	if (DeviceTable[NDevices - 1][0] != '\0') {
		device_table = expand_cw_table(DeviceTable,
		  NDevices, NDevices + 1, DEVICEWIDTH+1);

		if (device_table == (char **) 0)
			MemoryError();
		DeviceTable = device_table;
		NDevices++;
		sp[DEVLIST_STRUCT].pointer = (char *) device_table;
		sp[DEVLIST_STRUCT].filled = NDevices;
		return 1;
	}
	return 0;
}

/*
 * Initialize the device table from the initial "devices" field in the fillin
 */

static void
init_device_table(defill)
	DeviceFillin *defill;
{
	int i;

	/* defill->ndevices is set to the actual number of devices */

	NDevices = defill->ndevices + 1;
	DeviceTable = alloc_cw_table(NDevices, DEVICEWIDTH + 1);
	if (DeviceTable == (char **) 0)
		MemoryError();

	for (i = 0; i < defill->ndevices; i++)
		strcpy(DeviceTable[i], defill->devices[i]);
}

/*
 * Initialize the name and device list field in the scrn_struct table
 */

static void
init_sp_name_list(sp, defill)
	Scrn_struct *sp;
	DeviceFillin *defill;
{
	sp[NAME_STRUCT].pointer = defill->device_name;
	sp[NAME_STRUCT].filled = 1;

	sp[DEVLIST_STRUCT].pointer = (char *) DeviceTable;
	sp[DEVLIST_STRUCT].filled = NDevices;
	if (DeviceMode == DeviceCreate || DeviceMode == DeviceUpdate)
		sp[DEVLIST_STRUCT].val_act = DeviceExpand;
	else
		sp[DEVLIST_STRUCT].val_act = NULL;
}

/*
 * action routine.
 * Rewrite necessary databases
 */

int
device_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("device_action");

	defill->devices = DeviceTable;
	defill->ndevices = NDevices - 1;

	ret = DevChangeControlParams(defill);

	EXITFUNC("device_action");
	return INTERRUPT;
}


void
device_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("device_free");

	if (DeviceTable != (char **) 0) {
		free((char *) DeviceTable);
		DeviceTable = (char **) 0;
	}

	DevFreeFillin(defill);

	EXITFUNC("device_free");
	return;
}

/*
 * validate the structure
 */

int
device_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;
	int i;
	char **device_table, ndevices;

	ENTERFUNC("device_valid");

	/* if asking to create, make sure the device doesn't already exist */

	if (DeviceMode == DeviceCreate) {
		if (defill->device_name[0] == '\0') {
			pop_msg("You must specify a device name to create",
			  "a new entry in the Device Assignment Database.");
			ret = 1;
		} else {
			GetAllDevices(&ndevices, &device_table);
			for (i = 0; i < ndevices; i++) {
				if (strcmp(device_table[i],
					   defill->device_name) == 0) {
					pop_msg(
			"The device name you selected already already exists.",
			"Please use 'Modify' to update that device's entry.");
					ret = 1;
					break;
				}
			}
		}
	}

	if (ret == 0)
		ret = DevValidateDeviceParams(defill, DeviceMode);

	EXITFUNC("device_valid");
	return ret;
}

/*
 * Routines that are called from the top level screen that
 * work on the selected device
 */

static int
NoDevSelected()
{
	pop_msg("You must select a device to use this option.",
		"Please return to the 'Select Device:' menu option.");
}

/* called when the user specifies 'Modify Device' */

int
demd_doit()
{
	if (!IsDeviceSelected) {
		NoDevSelected();
		return INTERRUPT;
	}
	switch(DeviceType) {
	case PrinterDevice:
		return demdp_doit();
	case TerminalDevice:
		return demdt_doit();
	case RemovableDevice:
		return demdr_doit();
#if SEC_NET_TTY
	case HostDevice:
		return demdh_doit();
#endif
	}
}

/* called when the user specifies 'Remove Device' */

int
derd_doit()
{
	if (!IsDeviceSelected) {
		NoDevSelected();
		return INTERRUPT;
	}
	switch(DeviceType) {
	case PrinterDevice:
		return dermp_doit();
	case TerminalDevice:
		return dermt_doit();
	case RemovableDevice:
		return dermr_doit();
#if SEC_NET_TTY
	case HostDevice:
		return dermh_doit();
#endif
	}
}

/* called when the user specifies 'Display Device' */

int
dedd_doit()
{
	if (!IsDeviceSelected) {
		NoDevSelected();
		return INTERRUPT;
	}
	switch(DeviceType) {
	case PrinterDevice:
		return dedsp_doit();
	case TerminalDevice:
		return dedst_doit();
	case RemovableDevice:
		return dedsr_doit();
#if SEC_NET_TTY
	case HostDevice:
		return dedsh_doit();
#endif
	}
}
#endif /* SEC_BASE */
