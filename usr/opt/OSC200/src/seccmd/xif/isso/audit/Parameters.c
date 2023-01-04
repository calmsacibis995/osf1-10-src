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
static char	*sccsid = "@(#)$RCSfile: Parameters.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:41 $";
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

#ifdef SEC_BASE

/*
	filename:
		Parameters.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED
	
	function:
		
	entry points:
		AuditParametersGet()
		AuditParametersCheck()
		AuditParametersPut()
*/

#include "XAudit.h"
#include <sys/secioctl.h>

#include <protcmd.h>

static char 
	**msg_aparameters,
	 *msg_aparameters_text;
	
static int
	ChangeAuditOnStartup();

void 
AuditParametersStart() {
	LoadMessage("msg_isso_audit_aparameters", &msg_aparameters, &msg_aparameters_text);
}

void 
AuditParametersStop() {
/* nothing here */
}

/* 
	Get audit parameters file data and fill in superstructure data elements
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

	/* SW_3000 audit no longer uses a multi word mask but a single
	 * word mask. The bit handling routines are therefore redefined
	 */
#undef ISBITSET
#undef ADDBIT
#undef RMBIT
	/* add bit to single-word mask */
#define ADDBIT(mask,bit)  \
	mask |= bit

	/* remove bit from single-word mask */
#define RMBIT(mask,bit)  \
	mask &= ~bit

	/* test bit in single-word mask */
#define ISBITSET(mask,bit)  \
	((mask & bit) != 0)

	if (ISBITSET(aufill->au.audit_flags, AUDIT_COMPACT_FLAG))
		aufill->compacted = TRUE;
	else
		aufill->compacted = FALSE;
	if (ISBITSET(aufill->au.audit_flags, AUDIT_PANIC_FLAG))
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
		fp = new_audit_parms_file();
		if (! fp)
			return 1;
		ofp = old_audit_parms_file();
		if (! ofp)  {
			fclose(fp);
			unlink_new_parms_file();
			return 1;
		}
		if (aufill->compacted)
			ADDBIT(aufill->au.audit_flags, AUDIT_COMPACT_FLAG);
		else    
			RMBIT(aufill->au.audit_flags, AUDIT_COMPACT_FLAG);
		/* sense of comparison is for shutdown on screen */
		if (aufill->shut_or_panic)
			RMBIT(aufill->au.audit_flags, AUDIT_PANIC_FLAG);
		else    
			ADDBIT(aufill->au.audit_flags, AUDIT_PANIC_FLAG);
		if (fwrite(&aufill->au, sizeof(aufill->au), 1, fp) != 1) {
			ErrorMessageOpen(3230, msg_aparameters, 6, NULL);
			goto bad;
		}
		fseek(ofp, aufill->au.uid_offset, 0);
		for (i = 0; i < aufill->au.uid_count * sizeof(uid_t); i++)
			if ((c = getc(ofp)) == EOF ||
			    putc(c, fp) == EOF)
			    goto bad1;
		fseek(ofp, aufill->au.gid_offset, 0);
		for (i = 0; i < aufill->au.gid_count * sizeof(gid_t); i++)
			if ((c = getc(ofp)) == EOF || putc(c, fp) == EOF)
			    goto bad1;
		fseek(ofp, aufill->au.dir_offset, 0);
		for (i = 0; i < aufill->au.dir_count; i++)
			do {
			    if ((c = getc(ofp)) == EOF || putc(c, fp) == EOF)
			        goto bad1;
			} while (c != '\0');
		fclose(fp);
		fclose(ofp);
		link_new_parms_file();

		/* change the audit on startup indicator, if it has changed */
		if (aufill->initially_audit_on_startup != aufill->audit_on_startup)
			ChangeAuditOnStartup(aufill->audit_on_startup);

		/* AUDIT successful change of audit parameter file */
		sa_audit_audit(ES_AUD_MODIFY,
			           "Successful change of audit parameter file");
	}
	return 0;
	
bad1:
	ErrorMessageOpen(3240, msg_aparameters, 9, NULL);
			
bad:
	fclose(fp);
	fclose(ofp);
	unlink_new_parms_file();
	/* AUDIT unsuccessful change of audit parameter file */
	sa_audit_audit(ES_AUD_ERROR,
			       "Unsuccessful change of audit parameter file");
	return 1;
}

/* 
*   IsAuditOnStartup() is supposed to report on whether audit
*   is enabled on system startup
*   For SMP+3.0 this is handled in the defaults file
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
