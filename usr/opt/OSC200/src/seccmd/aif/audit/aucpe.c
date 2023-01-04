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
static char	*sccsid = "@(#)$RCSfile: aucpe.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:54 $";
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
 * Routines to implement the selection of audit event types for collection.
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfAudit.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int aucpe_auth();
static int aucpe_bfill();
static int aucpe_valid();
static int aucpe_exit();

/* structures defined in au_scrns.c */

extern Scrn_parms	aucpe_scrn;
extern Scrn_desc	aucpe_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aucpe_scrn
#define STRUCTTEMPLATE	aucpe_struct
#define DESCTEMPLATE	aucpe_desc
#define FILLINSTRUCT	audit_mask_struct
#define FILLIN		aucpe_fill
#define TRAVERSERW	TRAV_RW

#define EVENT_TITLE_DESC_COL0	0
#define EVENT_TITLE_DESC_COL1	1
#define EVENT_UNDER_DESC_COL0	2
#define EVENT_UNDER_DESC_COL1	3
#define EVENT_SCRTOG_DESC	4
#define EVENT_CURRENT_DESC	5
#define EVENT_FUTURE_DESC	6

#define EVENT_SCRTOG_STRUCT	0
#define EVENT_CURRENT_STRUCT	1
#define EVENT_FUTURE_STRUCT	2
#define NSCRNSTRUCT		3

#define FIRSTDESC	EVENT_SCRTOG_DESC

static Scrn_struct	*aucpe_struct;
static AUDIT_MASK_STRUCT au_buf, *aucpe_fill = &au_buf;

static int IsISSO;
static int NumberEvents = 0;
static char **EventTable = (char **) 0;
static char *EventState = NULL;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aucpe_auth(argv, aufill)
	char **argv;
	audmask_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aucpe_auth");
	if (first_time) {
		AuditEvMaskStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aucpe_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Builds the fillin structure by retrieving the mask from the
 * audit_parms file.  Returns 0 on success.
 */

aucpe_bfill(aufill)
	audmask_fillin *aufill;
{
	int ret;

	ENTERFUNC("aucpe_bfill");
	ret = AuditGetMaskStructure(aufill);
	switch (CheckAuditEnabled()) {
	case -1:	/* audit is running but there is no daemon */
	case 0:		/* audit is not enabled */
		aucpe_desc[EVENT_CURRENT_DESC].inout = FLD_OUTPUT;
		break;
	case 1:		/* audit is enabled and daemon is running */
		aucpe_desc[EVENT_CURRENT_DESC].inout = FLD_BOTH;
		break;
	}
	EXITFUNC("aucpe_bfill");
	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aucpe_bstruct(aufill, sptemplate)
	audmask_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	int i;
	struct scrn_struct *sp;

	ENTERFUNC("aucpe_bstruct");

	if (EventTable == (char **) 0) {

		AuditAllocEventMaskTable(&NumberEvents, &EventTable);

		/*
		 * Allocate the state structure for the values
		 */

		EventState = Calloc(NumberEvents, 1);

		if (EventState == NULL)
			MemoryError();
	}

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[EVENT_SCRTOG_STRUCT].pointer = (char *) EventTable;
	sp[EVENT_SCRTOG_STRUCT].state = EventState;
	sp[EVENT_SCRTOG_STRUCT].filled = NumberEvents;
	sp[EVENT_SCRTOG_STRUCT].validate = NULL;

	sp[EVENT_CURRENT_STRUCT].pointer = &aufill->this;
	sp[EVENT_CURRENT_STRUCT].filled = 1;
	sp[EVENT_CURRENT_STRUCT].validate = NULL;
	sp[EVENT_FUTURE_STRUCT].pointer = &aufill->future;
	sp[EVENT_FUTURE_STRUCT].filled = 1;
	sp[EVENT_FUTURE_STRUCT].validate = NULL;

	/*
	 * Fill in the state of the event mask
	 */

	AuditGetMask(aufill);

	for (i = 0; i < NumberEvents; i++)
		EventState[i] = (aufill->mask[i][0] == YESCHAR);

	EXITFUNC("aucpe_bstruct");
	return 0;
}

/*
 * action routine.
 * Unload the state fields to the mask structure and update audit state
 */

static int
aucpe_action(aufill)
	audmask_fillin *aufill;
{
	int i;

	ENTERFUNC("aucpe_action");
	for (i = 0; i < NumberEvents; i++)
		if (EventState[i])
			aufill->mask[i][0] = YESCHAR;
		else
			aufill->mask[i][0] = NOCHAR;

	(void) AuditUpdateMask(aufill);
	EXITFUNC("aucpe_action");
	return 1;
}


static void
cpe_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	audmask_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("cpe_free");
	AuditFreeMaskTable(aufill);
	EXITFUNC("cpe_free");
	return;
}

/*
 * validate the structure -- make sure at least one of this/future set
 */

static int
aucpe_valid(argv, aufill)
	char **argv;
	audmask_fillin *aufill;
{
	int ret = 0;

	ENTERFUNC("aucpe_valid");
	if (aufill->this == (char) 0 && aufill->future == (char) 0) {
                pop_msg(
                  "Neither 'This Session' nor 'Future Sessions' is selected.",
                  "Please select at least one of these items on the screen.");
		ERRFUNC("aucpe_valid", "neither this nor future selected");
		ret = 1;
	}
	EXITFUNC("aucpe_valid");
	return ret;
}

#define SETUPFUNC	aucpe_setup	/* defined by stemplate.c */
#define AUTHFUNC	aucpe_auth
#define BUILDFILLIN	aucpe_bfill

#define INITFUNC	aucpe_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aucpe_bstruct

#define ROUTFUNC	aucpe_exit		/* defined by stemplate.c */
#define VALIDATE	aucpe_valid
#define SCREENACTION	aucpe_action

#define FREEFUNC	aucpe_free		/* defined by stemplate.c */
#define FREESTRUCT	cpe_free

#include "stemplate.c"

#endif /* SEC_BASE */
