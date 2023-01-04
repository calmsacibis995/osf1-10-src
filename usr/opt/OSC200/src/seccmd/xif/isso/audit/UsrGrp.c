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
static char	*sccsid = "@(#)$RCSfile: UsrGrp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:07:49 $";
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
		UsrGrp.c
		
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	entry points:
		AuditUsersGroupsGet(aufill)
		AuditUsersValidate(aufill)
		AuditGroupsValidate(aufill)
		AuditUsersGroupsPut(aufill)
		AuditUsersGroupsFree(aufill)
*/

#include "XAudit.h"
#include <sys/secioctl.h>


void AuditUsersGroupsFree();

static char 
	**msg_ausrgrp,
	 *msg_ausrgrp_text;
	
void 
AuditUsersGroupsStart() 
{
	LoadMessage("msg_isso_audit_ausrgrp", &msg_ausrgrp, &msg_ausrgrp_text);
}

void 
AuditUsersGroupsStop() 
{
}

int 
AuditUsersGroupsGet(aufill)
	AUDIT_USERS_GROUPS_STRUCT *aufill;
{
	FILE    *fp;
	uid_t  id;
	struct  passwd  *pwd, *getpwuid();
	struct  group   *grp, *getgrgid();
	int     i,
		width = 9;  /* PORTABILITY we need user name len #define */
	char    buf[80],
		*cp;
	int     return_value;
	
	return_value = 1;
	fp = open_aud_parms(&aufill->au);
	if (! fp)
		return 1;

	aufill->users = alloc_cw_table(aufill->au.uid_count, width);
	aufill->groups = alloc_cw_table(aufill->au.gid_count, width);
	fseek(fp, aufill->au.uid_offset, 0);
	for (i = 0; i < aufill->au.uid_count; i++)  {
		if (fread(&id, sizeof(id), 1, fp) != 1) {
			/* AUDIT audit parameter file inconsistency */
			audit_no_resource(AUDIT_PARMSFILE, OT_SUBSYS,
			  "Audit parameter file inconsistency", ET_SYS_ADMIN);
			goto bad;
		}
		pwd = getpwuid(id);
		if (! pwd) {
			sprintf(buf, "%s", id);
			ErrorMessageOpen(2810, msg_ausrgrp, 0, buf);
			return_value = 2;
			goto bad;
		} 
		strcpy(aufill->users[i], pwd->pw_name);
	}
	aufill->nusers = aufill->au.uid_count;
	fseek(fp, aufill->au.gid_offset, 0);
	for (i = 0; i < aufill->au.gid_count; i++)  {
		if (fread(&id, sizeof(id), 1, fp) != 1) {
			/* AUDIT audit parameter file inconsistency */
			audit_no_resource(AUDIT_PARMSFILE, OT_SUBSYS,
			"Audit parameter file inconsistency", ET_SYS_ADMIN);
			goto bad;
		}
		grp = getgrgid(id);
		if (! grp) {
			sprintf(buf, "%d", id);
			ErrorMessageOpen(2820, msg_ausrgrp, 3, buf);
			return_value = 2;
			goto bad;
		} 
		strcpy(aufill->groups[i], grp->gr_name);
	}
	aufill->ngroups = aufill->au.gid_count;
	fclose(fp);
	return 0;
	
bad:
	AuditUsersGroupsFree(aufill);
	fclose(fp);
	return return_value;
}

void 
AuditUsersGroupsFree(aufill)
	AUDIT_USERS_GROUPS_STRUCT *aufill;
{
	if (aufill->groups) {
		free_cw_table(aufill->groups);
		aufill->groups =(char **) 0;
	}
	if (aufill->users) {
		free_cw_table(aufill->users);
		aufill->users =(char **) 0;
	}
	return;
}

int 
AuditUsersValidate(aufill)
	AUDIT_USERS_GROUPS_STRUCT *aufill;
{
	int i;
	struct  passwd  *getpwnam();
	char    buf[80];

	for (i = 0; i < aufill->nusers; i++)
		if (aufill->users[i][0] && ! getpwnam(aufill->users[i])) {
		      ErrorMessageOpen(2830, msg_ausrgrp, 6, aufill->users[i]);
			return 1;
		}
	return 0;
}

int 
AuditGroupsValidate(aufill)
	AUDIT_USERS_GROUPS_STRUCT *aufill;
{
	int i;
	struct  group   *getgrnam();
	char    buf[80];

	for (i = 0; i < aufill->ngroups; i++)
		if (aufill->groups[i][0] != '\0')
			if (! getgrnam(aufill->groups[i])) {
			 	ErrorMessageOpen(2840, msg_ausrgrp, 
					9, aufill->groups[i]);
				return 1;
			}
	return 0;
}

int 
AuditUsersGroupsPut(aufill)
	AUDIT_USERS_GROUPS_STRUCT *aufill;
{
	int     c,
		dcount,
		gcount,
		i,
		ucount;
	struct  passwd  *pwd, *getpwnam();
	struct  group   *grp, *getgrnam();
	FILE    *fp,
		*ofp;
	long    save_diroffset;
	uid_t   id,
		*uptr;
	struct audit_select *au;
	int     ausize,
		count,
		fd;

	/* Initialize pointer to audit_select */
	au = NULL;
	
	/* If don't change this or future parameters, just return */
	if (! aufill->this && ! aufill->future)
		return 1;

	/* Count up user id's and group id's and set offsets in init struct */
	for (i = 0, ucount = 0; i < aufill->nusers; i++)
		if (aufill->users[i][0])
			ucount++;
	for (i = 0, gcount = 0; i < aufill->ngroups; i++)
		if (aufill->groups[i][0])
			gcount++;

	/* 
		If changing for this session, need to allocate a structure
		large enough to hold users and groups.
	*/
	if (aufill->this) {
		ausize = AUDITSELECT_SIZE +
			 ((ucount + gcount) * sizeof(uid_t));
		au = (struct audit_select *) Calloc(ausize, 1);
		if (! au)
			MemoryError();
			/* Dies */
		au->sel_size = ausize;
		au->uid_count = ucount;
		if (ucount > 0)
			au->uid_offset = AUDITSELECT_SIZE;
		au->gid_count = gcount;
		if (gcount > 0)
			au->gid_offset = AUDITSELECT_SIZE + 
				(ucount * sizeof(gid_t));
	}

	/* need to preserve old information if re-writing file */
	if (aufill->future) {
		dcount = aufill->au.buf_length - 
		 (AUDITINIT_SIZE +
		   ((aufill->au.uid_count + aufill->au.gid_count)
			  * sizeof(uid_t)));
		aufill->au.uid_count = ucount;
		aufill->au.gid_count = gcount;
		aufill->au.uid_offset = AUDITINIT_SIZE;
		aufill->au.gid_offset = aufill->au.uid_offset 
		              + ucount * sizeof(uid_t);

		/* Compute other offsets */
		save_diroffset = aufill->au.dir_offset;
		aufill->au.dir_offset = aufill->au.gid_offset + 
			gcount * sizeof(gid_t);
		aufill->au.buf_length = aufill->au.dir_offset + dcount;

		fp = new_audit_parms_file();
		if (! fp) {
			if (au)
				free(au);
			return 1;
		}

		ofp = old_audit_parms_file();
		if (! ofp) {
			fclose(fp);
			unlink_new_parms_file();
			if (au)
				free(au);
			return 1;
		}

		if (fwrite(&aufill->au, sizeof(aufill->au), 1, fp) != 1)
			goto bad;
	}

	/* now write users and groups to file and/or to ioctl structure */
	if (aufill->this)
		/* point at the first byte after the au structure */
		uptr = (uid_t *)(au + 1);

	/* Pick up userid and write or load into data structure */
	for (i = 0; i < ucount; i++) {
		pwd = getpwnam(aufill->users[i]);
		id = pwd->pw_uid;
		if (aufill->future && fwrite(&id, sizeof(uid_t), 1, fp) != 1)
			goto bad;
		if (aufill->this)
			*uptr++ = id;
	}

	/* Pick up groupid and write or load into data structure */
	for (i = 0; i < gcount; i++) {
		grp = getgrnam(aufill->groups[i]);
		id = grp->gr_gid;
		if (aufill->future && fwrite(&id, sizeof(gid_t), 1, fp) != 1)
			goto bad;
		if (aufill->this)
			*uptr++ = id;
	}

	/* put other file information if writing to file */
	if (aufill->future) {
		fseek(ofp, save_diroffset, 0);
		for (i = 0; i < dcount; i++)
			if ((c = getc(ofp)) == EOF || putc(c, fp) == EOF)
				goto bad;
		fclose(fp);
		fclose(ofp);
		link_new_parms_file();
		/* AUDIT successful change of user and group list */
		sa_audit_audit(ES_AUD_MODIFY,
		  "Successful change of user and groups to audit");
	}

	/* do audit ioctl if changing for this session */
	if (aufill->this) {
		fd = open(AUDIT_WDEVICE, O_RDWR);
		count = 0;
		if (fd >= 0) {
			count = ioctl(fd, AUDIOC_IDS, au);
			close(fd);
		}
		if (fd < 0 || count < 0) {
			sa_audit_audit(ES_AUD_MODIFY,
		"Unsuccessful change of current session users and groups\n");
			ErrorMessageOpen(2850, msg_ausrgrp, 12, NULL);
		} else
			sa_audit_audit(ES_AUD_MODIFY,
		"Successful change of current session users and groups\n");
	}

	if (au)
		free(au);
	return 0;
	
bad:
	if (aufill->future) {
		ErrorMessageOpen(2860, msg_ausrgrp, 15, NULL);
		/* AUDIT failed write to audit parameter file */
		audit_no_resource(AUDIT_PARMSFILE, OT_SUBSYS,
			"Write to new parameter file failed", ET_SYS_ADMIN);
		fclose(fp);
		fclose(ofp);
		unlink_new_parms_file();
	}
	if (au)
		free(au);
	return 1;
}

#endif /* SEC_BASE */
