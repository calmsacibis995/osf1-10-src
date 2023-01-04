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
static char	*sccsid = "@(#)$RCSfile: desd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:49 $";
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
 * Routines to implement device selection screen
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

/* static routine definitions */

static int desd_auth();
static int desd_bfill();
static int desd_valid();
int desd_exit();
void DeselectDevice();
static int DeviceToggle();

/* structures defined in de_scrns.c */

extern Scrn_parms	desd_scrn;
extern Scrn_desc	desd_desc[];

/* definitions for the screen infrastructure */

#define SELECT_TITLE_DESC	0
#define SELECT_DESC		1

#define SELECT_STRUCT		0
#define NSCRNSTRUCT		1

#define FIRSTDESC	SELECT_DESC

Scrn_struct	*desd_struct;

/* static storage for local usage */

static char **DeviceTable;			/* list of devices to display */
static int NDevices;				/* length of DeviceTable */
static char *DeviceState;			/* toggle button state */
static int DeviceIndex;				/* index of highlighted item */
char DevSelected[TERMINAL_NAME_LEN + 1];	/* which device selected */
char IsDeviceSelected;				/* has device been selected */
static enum device_type	DeviceSelectedType = UnknownDevice;
				/* type of last selected device */
enum device_type	DeviceType;		/* which type is selected */

static int IsISSO;

/*
 * Initialization for this screen.
 * Remembers the device type that was requested.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

int
desd_auth(device_type)
	enum device_type device_type;
{
	static int first_time = 1;

	ENTERFUNC("desd_auth");

	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}

	DeviceType = device_type;

	EXITFUNC("desd_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * If the user requested a new type, remove the old selection.
 * Build a table of the type of device the user requested.
 */

int
desd_bfill(defill)
	struct device_fillin *defill;
{
	ENTERFUNC("desd_bfill");

	/*
	 * if user requested to select different device type, 
	 * remove old selection.
	 */

	if (!IsDeviceSelected || DeviceSelectedType != DeviceType)
		DeselectDevice();

	EXITFUNC("desd_bfill");

	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
desd_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;
	int ret = 0;

	ENTERFUNC("desd_bstruct");

	sp = desd_scrn.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* Free old memory, if it exists */

	if (DeviceTable != (char **) 0) {
		free_cw_table(DeviceTable);
		DeviceTable = (char **) 0;
		free(DeviceState);
		DeviceState = NULL;
	}

	switch (DeviceType) {
	case PrinterDevice:
		GetAllPrinters(&NDevices, &DeviceTable);
		break;
	case TerminalDevice:
		GetAllTerminals(&NDevices, &DeviceTable);
		break;
	case RemovableDevice:
		GetAllRemovables(&NDevices, &DeviceTable);
		break;
#if SEC_NET_TTY
	case HostDevice:
		GetAllHosts(&NDevices, &DeviceTable);
		break;
#endif
	}

	if (NDevices == 0) {
		pop_msg("There are no devices of that type.",
		  "You must create devices before selecting them.");
		ret = 1;
		goto out;
	}

	DeviceState = (char *) Calloc(NDevices, 1);
	if (DeviceState == NULL)
		MemoryError();

	sp[SELECT_STRUCT].pointer = (char *) DeviceTable;
	sp[SELECT_STRUCT].filled = NDevices;
	sp[SELECT_STRUCT].val_act = DeviceToggle;
	sp[SELECT_STRUCT].state = DeviceState;

	/* make sure that when user types execute, something is done */

	sp[SELECT_STRUCT].changed = 1;

	/*
	 * if a device has been selected, highlight that one.
	 * Otherwise, just highlight the first one in the list
	 */

	if (IsDeviceSelected) {
		int i;

		for (i = 0; i < NDevices; i++)
			if (strcmp(DevSelected, DeviceTable[i]) == 0) {
				DeviceState[i] = 1;
				DeviceIndex = i;
				break;
			}
		if (i == NDevices) {
			IsDeviceSelected = 0;
		}
	}

	if (!IsDeviceSelected) {
		DeviceState[0] = 1;
		DeviceIndex = 0;
		gl_device = NULL;
	}
		
out:
	EXITFUNC("desd_bstruct");
	return ret;
}

/*
 * Routine that de-selects the selected device
 */

void
DeselectDevice()
{
	DeviceSelectedType = UnknownDevice;
	IsDeviceSelected = 0;
	gl_device = NULL;
	memset(DevSelected, '\0', sizeof DevSelected);
}

/*
 * Toggle routine to implement radio buttons on the screen.
 */

static int
DeviceToggle()
{
	int i, j;
	int ret = 0;

	ENTERFUNC("DeviceToggle");
	for (i = 0; i < NDevices && !ret; i++) {

		/*
		 * check for a different device selected; turn off previous
		 * selection
		 */

		if (DeviceState[i] && i != DeviceIndex) {
			for (j = 0; j < NDevices; j++)
				if (j != i && DeviceState[j])
					DeviceState[j] = 0;
			DeviceIndex = i;
			ret = 1;
		}

		/*
		 * If item was de-selected and it was the one selected,
		 * turn it back on (one must always be selected!)
		 */

		if (!DeviceState[i] && i == DeviceIndex) {
			DeviceState[i] = 1;
			ret = 1;
		}
	}

	EXITFUNC("DeviceToggle");

	return 1;	/* redraw screen */
			
}

/*
 * action routine.
 * Store the device that was selected
 */

int
desd_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desd_action");

	DeviceSelectedType = DeviceType;
	IsDeviceSelected = 1;
	strcpy(DevSelected, DeviceTable[DeviceIndex]);

	/*  Assign selected device for the screens */

	gl_device = (uchar *) DevSelected;

	EXITFUNC("desd_action");

	return INTERRUPT;
}


void
sd_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("sd_free");

	if (DeviceTable != (char **) 0) {
		free_cw_table(DeviceTable);
		free(DeviceState);
		DeviceTable = (char **) 0;
	}

	EXITFUNC("sd_free");
	return;
}

/*
 * There is no validation routine.
 */

int
desd_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	ENTERFUNC("desd_valid");

	EXITFUNC("desd_valid");
	return 0;
}

#endif /* SEC_BASE */
