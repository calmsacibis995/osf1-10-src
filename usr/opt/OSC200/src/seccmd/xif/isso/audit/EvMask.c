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
static char	*sccsid = "@(#)$RCSfile: EvMask.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:29 $";
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

#if SEC_BASE


/* 
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

 */

/* routines to display and update event mask in the audit parameters file */

#include "XAudit.h"
#include <sys/secioctl.h>

static char 
	**msg,
	 *msg_text;
	
void 
AuditEvMaskStart() {
	LoadMessage("msg_isso_audit_aevmask", &msg, &msg_text);
}

void 
AuditEvMaskStop() {
}

int 
AuditGetMaskStructure(aufill)
	audmask_fillin  *aufill;
{
	FILE    *fp;

	fp = open_aud_parms(&aufill->au);
	if (! fp)
		return 1;
	fclose(fp);
	return 0;
}

int 
AuditGetMask(aufill)
	audmask_fillin  *aufill;
{
	int     i;

	aufill->mask = alloc_cw_table(AUDIT_MAX_EVENT, 2);
	if (! aufill->mask)
		MemoryError();
		/* Dies */
	for (i = 1; i <= AUDIT_MAX_EVENT; i++)  {
		if (ISBITSET(aufill->au.event_mask, i))
			aufill->mask[i-1][0] = YESCHAR;
		else 
			aufill->mask[i-1][0] = NOCHAR;
	}
	aufill->this = FALSE;
	aufill->future = FALSE;
	return 0;
}

void 
AuditFreeMaskTable(aufill) 
	audmask_fillin  *aufill;
{
	if (aufill->mask) {
		free_cw_table(aufill->mask);
		aufill->mask = NULL;
	}
	return;
}

/* Check that each of the event mask characters is either upper or lower
 * case YESCHAR or NOCHAR.
 */

int 
AuditCheckMask(aufill)
	audmask_fillin  *aufill;
{
	register int i;
	char c;

	for (i = 0; i < AUDIT_MAX_EVENT; i++)  {
		c = aufill->mask[i][0];
		if (islower(c))
			c = aufill->mask[i][0] = toupper(c);
		if (c == YESCHAR || c == NOCHAR)
			continue;
		ErrorMessageOpen(2610, msg, 0, NULL);
		return 1;
	}
	return 0;
}

int 
AuditUpdateMask(aufill)
	audmask_fillin  *aufill;
{
	int     fd,
			i,
			ret;
	FILE    *fp;
	mask_t  privmask[SEC_SPRIVVEC_SIZE];
	struct  audit_ioctl au_ioctl;

	/* unload screen version of mask to audit_init structure */
	for (i = 1; i <= AUDIT_MAX_EVENT; i++) {
		if (aufill->mask[i-1][0] == YESCHAR)
			ADDBIT( aufill->au.event_mask, i );
		else
			RMBIT( aufill->au.event_mask, i );
	}

	/* update file if future session specified */
	if (aufill->future)  {
		fp = fopen (AUDIT_PARMSFILE, "r+");
		if (! fp) {
			ErrorMessageOpen(2620, msg, 3, NULL);
			/* AUDIT failure to open AUDIT_PARMSFILE */
			audit_security_failure (OT_SUBSYS, AUDIT_PARMSFILE, 
		"Failure to open for update", ET_SYS_ADMIN);
		} else {
			if (fwrite (&aufill->au, sizeof (aufill->au), 
					1, fp) != 1) {
				ErrorMessageOpen(2630, msg, 6, NULL);
				/* AUDIT failure to write parameter file */
				audit_security_failure (OT_SUBSYS, 
					AUDIT_PARMSFILE, "Failure to write", 
					ET_SYS_ADMIN);
			} else {
				/* AUDIT successful change of mask in file */
				sa_audit_audit (ES_AUD_MODIFY,
				  "Change of event mask in parameter file");
			}
			fclose(fp);
		}
	}

	/* do the audit ioctl if this session specified */
	if (aufill->this)  {
		fd = open(AUDIT_WDEVICE, O_WRONLY);
		if (fd < 0) {
			ErrorMessageOpen(2650, msg, 12, NULL);
			/* can't AUDIT audit device open failure!! */
		}
		else {
			for (i = 0; i < AUDIT_MASK_SIZE; i++)
				au_ioctl.system_mask[i] =
				  aufill->au.event_mask[i];
			ret = ioctl (fd, AUDIOC_SYSMASK, &au_ioctl);
			if (ret < 0) {
				ErrorMessageOpen(2660, msg, 15, NULL);
				/* AUDIT failure to change mask */
				sa_audit_audit (ES_AUD_ERROR,
			"Failure to set event mask for current audit session");
			}
			else {
				/* AUDIT successful mask change */
				sa_audit_audit (ES_AUD_MODIFY,
			"Successful event mask change for current session");
			}
			close(fd);
		}
	}
	return 1;
}
#endif /* SEC_BASE */
