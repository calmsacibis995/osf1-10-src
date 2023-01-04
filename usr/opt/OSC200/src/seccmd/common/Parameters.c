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
static char	*sccsid = "@(#)$RCSfile: Parameters.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:00:07 $";
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



/* Routines that implement update of audit parameters */

#include <sys/secdefines.h>

#ifdef SEC_BASE

#include "gl_defs.h"
#include "IfAudit.h"
#include <sys/secioctl.h>

#include <protcmd.h>

static char 
	**msg_aparameters,
	 *msg_aparameters_text;
	
static int
	ChangeAuditOnStartup();

void 
AuditParametersStart() {
	LoadMessage("msg_isso_audit_aparameters",
			&msg_aparameters, &msg_aparameters_text);
}

void 
AuditParametersStop() {
/* nothing here */
}

/* 
 *	Get audit parameters file data and fill in superstructure data elements
 */

int 
AuditParametersGet(aufill)
	audparm_fillin  *aufill;
{
	FILE    *fp;

	fp = open_aud_parms(&aufill->au);
	if (! fp)
		return 1;
	fclose(fp);

	/* Add in default values */
	aufill->audit_on_startup = IsAuditOnStartup();
	aufill->initially_audit_on_startup = aufill->audit_on_startup;

	if (aufill->au.audit_flags & AUDIT_COMPACT_FLAG)
		aufill->compacted = TRUE;
	else
		aufill->compacted = FALSE;
	if (aufill->au.audit_flags & AUDIT_PANIC_FLAG)
		aufill->shut_or_panic = FALSE;
	else
		aufill->shut_or_panic = TRUE;
	aufill->this = FALSE;
	aufill->future = FALSE;
	return 0;
}

int 
AuditParametersCheck(aufill)
	audparm_fillin *aufill;
{
	extern char *malloc();

	if (aufill->au.read_count < AUDIT_MINIMUM_READ_COUNT) {
		char *msg =
		  "minimum specifiable read count, currently set to %d.";
		char *msgalloc = malloc(strlen(msg) + 10);

		if (msgalloc == NULL)
			MemoryError();
		sprintf(msgalloc, msg, AUDIT_MINIMUM_READ_COUNT);
		pop_msg("The read count specified is smaller than the",
		  msgalloc);
		free(msgalloc);
		return 1;
	}

	if (aufill->au.max_memory < AUDIT_MINIMUM_BUFFER_SIZE) {
		char *msg =
		  "minimum kernel buffer size, currently set to %d.";
		char *msgalloc = malloc(strlen(msg) + 10);

		if (msgalloc = NULL)
			MemoryError();
		sprintf(msgalloc, msg, AUDIT_MINIMUM_BUFFER_SIZE);
		pop_msg("The buffer size specified is smaller than the",
		  msgalloc);
		free(msgalloc);
		return 1;
	}

	if (aufill->au.max_memory < 4 * aufill->au.read_count) {
		pop_msg("The buffer size specified must be at least 4",
		  "times the size of the read count.");
		return 1;
	}

	if (aufill->this == (char) 0 && aufill->future == (char) 0) {
		pop_msg("You must specify at least one of 'This Session'",
		  "or 'Future Sessions.");
		return 1;
	}

	return 0;
}

int 
AuditParametersPut(aufill)
	audparm_fillin  *aufill;
{
	FILE    *fp,
			*ofp;
	struct  audit_ioctl auioctl;
	int     c,
			fd,
			i,
			j;

	if (aufill->this) {
		auioctl.read_count = aufill->au.read_count;
		fd = open(AUDIT_WDEVICE, O_WRONLY);
		if (fd < 0) {
			ErrorMessageOpen(3220, msg_aparameters, 3, NULL);
			/* can't AUDIT failure to open audit device! */
		} else {

		if (ioctl(fd, AUDIOC_DAEMON, &auioctl) < 0) {
			    /* AUDIT AUDIT_DAEMON ioctl failure */
			    sa_audit_audit(ES_AUD_ERROR,
			        "AUDIT_DAEMON ioctl failure");
			} 
			else {
			    /* AUDIT successful parameter change */
			    sa_audit_audit(ES_AUD_MODIFY,
			        "AUDIT_DAEMON ioctls successful");
			}
			close(fd);
		}
	}
		
	if (aufill->future) {
		if (aufill->compacted)
			aufill->au.audit_flags |= AUDIT_COMPACT_FLAG;
		else    
			aufill->au.audit_flags &= ~AUDIT_COMPACT_FLAG;

		/* sense of comparison is for shutdown on screen */

		if (aufill->shut_or_panic)
			aufill->au.audit_flags &= ~AUDIT_PANIC_FLAG;
		else    
			aufill->au.audit_flags |= AUDIT_PANIC_FLAG;

		update_audit_parms(&aufill->au);

		/* change the audit on startup indicator, if it has changed */

		if (aufill->initially_audit_on_startup !=
				aufill->audit_on_startup)
			ChangeAuditOnStartup(aufill->audit_on_startup);

		/* AUDIT successful change of audit parameter file */
		sa_audit_audit(ES_AUD_MODIFY,
			           "Successful change of audit parameter file");
	}
	return 0;
	
error:
	sa_audit_audit(ES_AUD_ERROR,
			       "Unsuccessful change of audit parameter file");
	return 1;
}

/* 
*   IsAuditOnStartup() is supposed to report on whether audit
*   is enabled on system startup
*/

IsAuditOnStartup()
{
	int     retval = FALSE;
	struct pr_default *df;


	df = getprdfnam("default");
	if (df && df->sflg.fg_audit_enable && df->sfld.fd_audit_enable)
		retval = TRUE;
	endprdfent();
	return (retval);
}

/*
*   ChangeAuditOnStartup() executes a utility program to enable/disable
*   audit on system boot
*   
*   Note that output from audit change program is not collected by the
*   parent program
*/

static int 
ChangeAuditOnStartup(audit_on_startup)
	char    audit_on_startup;
{
	int 	retval = FALSE;
	struct pr_default	*df_old;
	struct pr_default	df;

	/* Only get called if definitely going to change */
	df_old = getprdfnam ("default");
	if (df_old) {
		df = *df_old;

		df.sflg.fg_audit_enable = 1;
		if (audit_on_startup) 
			df.sfld.fd_audit_enable = 1;
		else 
			df.sfld.fd_audit_enable = 0;
		retval = putprdfnam ("default", &df);
	}
	return (retval);
}
#endif /* SEC_BASE */
