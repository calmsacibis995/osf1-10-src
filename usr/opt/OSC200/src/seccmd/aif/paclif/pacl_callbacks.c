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
static char *rcsid = "@(#)$RCSfile: pacl_callbacks.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:38 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	pacl_callbacks.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.1.2.5  1992/04/28  18:12:35  valin
 * 	Fix chmod and chown running on a C2 system.  The problem involves that the
 * 	user dose not have the required priv. and can not pick it up by
 * 	file base as this is a B1 feature.  Solution is to call the commands
 * 	chown and chgrp to pick up a setuid of 0.
 * 	[1992/04/28  18:03:22  valin]
 *
 * Revision 1.1.2.4  1992/04/21  13:32:51  marquard
 * 	Removed #ident line, which contained 8 bit characters.  These
 * 	characters caused RCS to believe that this file is a binary file.
 * 	[1992/04/21  13:10:15  marquard]
 * 
 * 	Fixed #ident problem.
 * 	[1992/04/20  21:39:49  marquard]
 * 
 * 	Replaced improper use of NULL with 0.
 * 	[1992/04/20  21:13:37  marquard]
 * 
 * Revision 1.1.2.3  1992/04/05  18:20:01  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:56  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1991, SecureWare, Inc.  All rights reserved.
 */




/*
 * This module contains the callbacks for the paclif which handle the
 * reading, writing, and interpretting of the ACL entries.
 *
 * The routines in this module will perform the backend work which involves
 * anything more complex than a line or two of code.  These routines 
 * will not set any of the screen structure variables, as that is
 * all handled in the pacl_scrn.c routines.
 */

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/secdefines.h>
#include <sys/security.h>
#include <acl.h>
#include <If.h>
#include "paclif.h"
#include <sys/param.h>

extern int	sec_errno;

/* 
 * Determine if the filename refers to an IPC object rather than a file.
 */

is_ipc_object (filename)
char *filename;
{
	return (isspace (filename [1]) && 
		(filename [0] == 'm' || filename [0] == 'q' 
		 || filename [0] == 's'));
}

/*
 * Read the ACLs for a filename.  Return 0 on success, errno on failure.
 * If the specified filename is blank, clear all of the fields.
 */

init_paclif (pacl)
struct paclif	*pacl;
{
	int		i, entries;
	char   		*s;

	pacl->creator_name [0] = 
	pacl->creator_group [0] = 
	pacl->owner_name [0] = 
	pacl->owner_group [0] = 
	pacl->user_perms [0] = 
	pacl->backing_user_perms [0] = 
	pacl->group_perms [0] = 
	pacl->backing_group_perms [0] = 
	pacl->group_eff [0] = 
	pacl->other_perms [0] = 
	pacl->backing_other_perms [0] = 
	pacl->backing_mask_perms [0] = 
	pacl->mask_perms [0] = 0;

	pacl->is_ipc_object = 0;

	if (pacl->user) {
		free_cw_table (pacl->user);
		free_cw_table (pacl->backing_user);
	}
	if (pacl->group) {
		free_cw_table (pacl->group);
		free_cw_table (pacl->backing_group);
	}

	pacl->user = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->group = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->backing_user = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->backing_group = alloc_cw_table (1, ENTRY_WIDTH + 1);

	pacl->number_users = pacl->number_groups = 1;

#if I_MESSED_UP
	strcpy (pacl->user [pacl->number_users - 1], INITIAL_LINE);
	strcpy (pacl->backing_user [pacl->number_users - 1], INITIAL_LINE);
	strcpy (pacl->group [pacl->number_groups - 1], INITIAL_LINE);
	strcpy (pacl->backing_group [pacl->number_groups - 1], INITIAL_LINE);
#endif
}

/*
 * Read the ACLs for a filename.  Return 0 on success, errno on failure.
 * If the specified filename is blank, clear all of the fields.
 */

static char	**msg_acl_read, *msg_acl_read_text;

read_paclif (pacl, filename, flags)
struct paclif	*pacl;
char		*filename;
int		flags;
{
	int	i, entries, status;
	char   	*s;
	acl_t	*acl;

	init_paclif (pacl);

	if (isblank (filename)) {/* blank filename; don't give 'em anything! */
		if (!msg_acl_read)
			LoadMessage ("msg_acl_read", &msg_acl_read, 
				     &msg_acl_read_text);
		ErrorMessageOpen (-1, msg_acl_read, 8, NULL);
		return -1;
	}

	pacl->is_ipc_object = is_ipc_object (filename);

	if (acl_alloc (&acl) != 0) {
		if (!msg_acl_read)
			LoadMessage ("msg_acl_read", &msg_acl_read, 
				     &msg_acl_read_text);
		ErrorMessageOpen (-1, msg_acl_read, 0, NULL);
		return -1;
	}

	if (pacl->is_ipc_object) {
		entries = ipc_read_acl (filename, acl);
	} else {
		entries = file_read_acl (filename, flags, acl);
	}
		
	if (entries == -1) {
		acl_free (acl);
		return -1;
	}

	for (i = 0; i < entries; i++) {
		acl_entry_t	entry;

		if (acl_get_entry (acl, &entry) <= 0 || 
		    get_entry (pacl, entry) != 0) {
			acl_free (acl);
			if (!msg_acl_read)
				LoadMessage ("msg_acl_read", &msg_acl_read, 
					     &msg_acl_read_text);
			ErrorMessageOpen (-1, msg_acl_read, 4, NULL);
			return -1;
		}
	}
	acl_free (acl);

	strcpy (pacl->backing_mask_perms, pacl->mask_perms);
	strcpy (pacl->backing_user_perms, pacl->user_perms);
	strcpy (pacl->backing_other_perms, pacl->other_perms);

	set_all_effectives (pacl);


	if (flags & PACLIF_DEFAULT_ENTRY) 
		return 0;
	
	strcpy (pacl->filename, filename);

	if (pacl->is_ipc_object) {
		return (ipc_stat (pacl));
	} else {
		return (file_stat (pacl));
	}
}


file_read_acl (filename, flags, acl)
char		*filename;
int		flags;
acl_type_t	*acl;
{
	int		i, entries;

	entries = acl_read (filename, 
			    (flags & PACLIF_DEFAULT_ENTRY)  ? 
			    ACL_TYPE_DEFAULT : ACL_TYPE_ACCESS,
			    acl);
	if (entries == -1) {
		if (!msg_acl_read)
			LoadMessage ("msg_acl_read", &msg_acl_read, 
				     &msg_acl_read_text);
		if (flags & PACLIF_DEFAULT_ENTRY && errno == ENOTDIR)
			ErrorMessageOpen (-1, msg_acl_read, 6, NULL);
		else if (errno == ENOENT)
			ErrorMessageOpen (-1, msg_acl_read, 10, NULL);
		else
			ErrorMessageOpen (-1, msg_acl_read, 2, NULL);
	}
		
	return entries;
}


file_stat (pacl)
struct paclif	*pacl;
{
	int		uid, gid;
	char		*s;
	struct stat	statbuf;

	static char	**msg_stat, *msg_stat_text;
	
	if (stat (pacl->filename, &statbuf) != 0) {
		if (!msg_stat)
			LoadMessage ("msg_stat", &msg_stat, 
				     &msg_stat_text);
		ErrorMessageOpen (-1, msg_stat, 10, NULL);
		return -1;
	}
	uid = statbuf.st_uid;
	gid = statbuf.st_gid;
	
	if (s = pw_idtoname (uid))
		strcpy (pacl->owner_name, s);
	else
		sprintf (pacl->owner_name, "%d", uid);
	if (s = gr_idtoname (gid))
		strcpy (pacl->owner_group, s);
	else
		sprintf (pacl->owner_group, "%d", gid);
	
	return 0;
}

/* 
 * Write contents of the paclif structure to the right object.
 * The flag field can be used to force the ACL to be written as the
 * default ACL of a directory, or to set the ACL for a IPC object.
 */

write_paclif (pacl, flags)
struct paclif	*pacl;
int		flags;
{
	int		i, id, status, old_errno, mask_present = 0;
	char		*s;
	acl_t		acl;
	acl_entry_t	entry;
	acl_permset_t	perms;

	static char	**msg_acl_write, *msg_acl_write_text;

	if (isblank (pacl->filename)) {
		if (!msg_acl_write)
			LoadMessage ("msg_acl_write", &msg_acl_write, 
				     &msg_acl_write_text);
		ErrorMessageOpen (-1, msg_acl_write, 11, NULL);
		return -1;
	}

	pacl->is_ipc_object = is_ipc_object (pacl->filename);

	if (acl_alloc (&acl) != 0) {
		if (!msg_acl_write)
			LoadMessage ("msg_acl_write", 
				     &msg_acl_write, 
				     &msg_acl_write_text);
		ErrorMessageOpen (-1, msg_acl_write, 13, NULL);
		return -1;
	}

	perms = 0;
	perms_to_mode (pacl->user_perms, &perms);
	set_entry (acl, USER_OBJ, NULL, perms);

	perms = 0;
	perms_to_mode (pacl->group_perms, &perms);
	set_entry (acl, GROUP_OBJ, NULL, perms);

	perms = 0;
	perms_to_mode (pacl->other_perms, &perms);
	set_entry (acl, OTHER_OBJ, NULL, perms);

	if (!isblank (pacl->mask_perms)) {
		mask_present = 1;
		perms = 0;
		perms_to_mode (pacl->mask_perms, &perms);
		set_entry (acl, MASK_OBJ, NULL, perms);
	}

	for (i = 0; i < pacl->number_users - 1; i++) {
		perms = 0;
		perms_to_mode (pacl->user [i] + 9, &perms);

		if (!isblank (s = trim_name (pacl->user [i]))) {
			if (!mask_present) {
				if (!msg_acl_write)
					LoadMessage ("msg_acl_write", 
						     &msg_acl_write, 
						     &msg_acl_write_text);
				ErrorMessageOpen (-1, msg_acl_write, 15, NULL);
				return -1;
			}

			if ((id = pw_nametoid (s)) == -1) {
				acl_free (acl);
				if (!msg_acl_write)
					LoadMessage ("msg_acl_write", 
						     &msg_acl_write, 
						     &msg_acl_write_text);
				ErrorMessageOpen (-1, msg_acl_write, 0, s);
				return -1;
			}
			set_entry (acl, USER, id, perms);
		}
	}

	for (i = 0; i < pacl->number_groups - 1; i++) {
		perms = 0;
		perms_to_mode (pacl->group [i] + 9, &perms);

		if (!isblank (s = trim_name (pacl->group [i]))) {
			if (!mask_present) {
				if (!msg_acl_write)
					LoadMessage ("msg_acl_write", 
						     &msg_acl_write, 
						     &msg_acl_write_text);
				ErrorMessageOpen (-1, msg_acl_write, 15, NULL);
				return -1;
			}

			if ((id = gr_nametoid (s)) == -1) {
				acl_free (acl);
				if (!msg_acl_write)
					LoadMessage ("msg_acl_write", 
						     &msg_acl_write, 
						     &msg_acl_write_text);
				ErrorMessageOpen (-1, msg_acl_write, 2, s);
				return -1;
			}
			set_entry (acl, GROUP, id, perms);
		}
	}

	if (pacl->is_ipc_object) {
		return (ipc_write_acl (pacl->filename, acl));
	} 

	status = acl_write (pacl->filename, 
			    (flags & PACLIF_DEFAULT_ENTRY)  ? 
			    ACL_TYPE_DEFAULT : ACL_TYPE_ACCESS,
			    acl);

	if (status == 0)
		return 0;

	old_errno = errno;

	/* Determine which part of the ACL was the problem. */

	if (acl_valid (acl, 
		       (flags & PACLIF_DEFAULT_ENTRY)  ? 
		       ACL_TYPE_DEFAULT : ACL_TYPE_ACCESS,
		       &entry) == 0) {
		if (!msg_acl_write)
			LoadMessage ("msg_acl_write", 
				     &msg_acl_write, 
				     &msg_acl_write_text);
		ErrorMessageOpen (-1, msg_acl_write, 4, 
				  sys_errlist [old_errno]);
		return -1;
	}

	if (entry) {
		if (!msg_acl_write)
			LoadMessage ("msg_acl_write", 
				     &msg_acl_write, 
				     &msg_acl_write_text);
		ErrorMessageOpen (-1, msg_acl_write, 7, NULL);
		return -1;
	} else {
		if (!msg_acl_write)
			LoadMessage ("msg_acl_write", 
				     &msg_acl_write, 
				     &msg_acl_write_text);
		ErrorMessageOpen (-1, msg_acl_write, 9, NULL);
		return -1;
	}
}

delete_paclif (pacl, flags)
struct paclif	*pacl;
int		flags;
{
	acl_t		acl;
	static char	**msg_acl_delete, *msg_acl_delete_text;

	if (isblank (pacl->filename)) {
		if (!msg_acl_delete)
			LoadMessage ("msg_acl_delete", 
				     &msg_acl_delete, 
				     &msg_acl_delete_text);
		ErrorMessageOpen (-1, msg_acl_delete, 5, NULL);
		return -1;
	}

	if (acl_alloc (&acl) != 0) {
		if (!msg_acl_delete)
			LoadMessage ("msg_acl_delete", 
				     &msg_acl_delete, 
				     &msg_acl_delete_text);
		ErrorMessageOpen (-1, msg_acl_delete, 0, NULL);
		return -1;
	}

	if (acl_write (pacl->filename, 
		       (flags & PACLIF_DEFAULT_ENTRY)  ? 
		       ACL_TYPE_DEFAULT : ACL_TYPE_ACCESS,
		       acl) == -1) {
		if (!msg_acl_delete)
			LoadMessage ("msg_acl_delete", 
				     &msg_acl_delete, 
				     &msg_acl_delete_text);
		ErrorMessageOpen (-1, msg_acl_delete, 2, sys_errlist [errno]);
		return -1;
	}

	return 0;
}



set_entry (acl, tag, id, perms)
acl_t		acl;
acl_tag_t	tag;
acl_permset_t	perms;
int		id;
{
	acl_entry_t	entry;
	static char	**msg_acl_set_entry, *msg_acl_set_entry_text;

	if ((acl_create_entry (acl, &entry) != 0) ||
	    (acl_set_perm (entry, perms) != 0) ||
	    (acl_set_tag (entry, tag, 
			  (tag == USER || tag == GROUP) ? id : 0) != 0)) {
		if (!msg_acl_set_entry)
			LoadMessage ("msg_acl_set_entry", 
				     &msg_acl_set_entry, 
				     &msg_acl_set_entry_text);
		ErrorMessageOpen (-1, msg_acl_set_entry, 0, NULL);
		return -1;
	}

	return 0;
}

/*
 * Rip out the information from the ACL entry, and put it into the
 * paclif structure.
 */

get_entry (pacl, entry)
struct paclif	*pacl;
acl_entry_t	entry;
{
	char		*s;
	int		id;
	acl_permset_t	perms;
	acl_tag_t	tag;

	if (acl_get_tag (entry, &tag, &id) != 0 || 
	    acl_get_perm (entry, &perms) != 0)
		return -1;

	switch (tag) {
	case USER_OBJ:
	       mode_to_perms (pacl->user_perms, perms);
	       break;
	case MASK_OBJ:
	       mode_to_perms (pacl->mask_perms, perms);
	       break;
	case GROUP_OBJ:
	       mode_to_perms (pacl->group_perms, perms);
	       break;
	case OTHER_OBJ:
	       mode_to_perms (pacl->other_perms, perms);
	       break;
	case USER:
	       if ((s = pw_idtoname ((uid_t) id)) != 0)
		       sprintf (pacl->user [pacl->number_users - 1], 
				"%-16s", s);
	       else
		       sprintf (pacl->user [pacl->number_users - 1], 
				"%-16d", (uid_t) id);

	       mode_to_perms (pacl->user [pacl->number_users - 1] + 9, 
				perms);
	       pacl->user = expand_cw_table (pacl->user, 
					     pacl->number_users,
					     pacl->number_users + 1, 
					     ENTRY_WIDTH + 1);
	       pacl->backing_user = expand_cw_table (pacl->backing_user, 
						     pacl->number_users,
						     pacl->number_users + 1, 
						     ENTRY_WIDTH + 1);
	       strcpy (pacl->backing_user [pacl->number_users],
		       pacl->user [pacl->number_users]);
	       pacl->number_users++;
	       break;
	case GROUP:
	       if ((s = gr_idtoname ((gid_t) id)) != 0)
		       sprintf (pacl->group [pacl->number_groups - 1], 
				"%-16s", s);
	       else
		       sprintf (pacl->group [pacl->number_groups - 1], 
				"%-16d", (uid_t) id);

	       mode_to_perms (pacl->group [pacl->number_groups - 1] + 9, 
				perms);
	       pacl->group = expand_cw_table (pacl->group, 
					     pacl->number_groups,
					     pacl->number_groups + 1, 
					     ENTRY_WIDTH + 1);
	       pacl->backing_group = expand_cw_table (pacl->backing_group, 
						      pacl->number_groups,
						      pacl->number_groups + 1,
						      ENTRY_WIDTH + 1);
	       strcpy (pacl->backing_group [pacl->number_groups],
		       pacl->group [pacl->number_groups]);
	       pacl->number_groups++;
	       break;
       default:
	       return -1;
       }
	return 0;
}

/*
 * Change the owner and group of a file.  If the owner or group is specified
 * as a NULL, then don't change that particular parameter.
 */

perform_chown (filename, owner, group)
char	*filename, *owner, *group;
{
	int	uid = -1;
	int	gid = -1;
	static char	**msg_chown, *msg_chown_text;
	char	*command;
	int 	commad_results = 0;

	if (owner != NULL && (uid = pw_nametoid (owner)) == -1) {
		if (!msg_chown)
			LoadMessage ("msg_chown", &msg_chown, &msg_chown_text);
		ErrorMessageOpen (-1, msg_chown, 0, NULL);
		return -1;
	}

	if (group != NULL && (gid = gr_nametoid (group)) == -1) {
		if (!msg_chown)
			LoadMessage ("msg_chown", &msg_chown, &msg_chown_text);
		ErrorMessageOpen (-1, msg_chown, 2, NULL);
		return -1;
	}

	/* IPC object */

	if (is_ipc_object (filename))
		return (ipc_set_attrs (filename, NULL, uid, gid));

	/* Object is file */

#if SEC_PRIV

	if (chown (filename, (uid_t) uid, (gid_t) gid) != 0) {
#else
	if (!(command = Calloc (1, MAXPATHLEN+100))) {
		MemoryError ();
		return -1;
	}
	if(group != NULL && owner != NULL) {
		sprintf(command,"/usr/bin/chown %s:%s %s",owner,group,filename);
	}
	else {
		if(group != NULL) {
			sprintf(command,"/usr/bin/chgrp %s %s",group,filename);
		}
		else {
			if(owner != NULL) {
				sprintf(command,"/usr/bin/chown %s %s",owner,filename);
			}
		}
	}
	if(system(command)) {
#endif
		if (!msg_chown)
			LoadMessage ("msg_chown", &msg_chown, &msg_chown_text);
		ErrorMessageOpen (-1, msg_chown, 4, NULL);
		return -1;
	}
#ifdef SEC_PRIV
	return 0;
#else
	free(command);
	return 0;
#endif
}


set_mode (pacl)
struct paclif	*pacl;
{
	static char	**msg_chmod, *msg_chmod_text;

	int	new_mode = 0;
	struct stat	statbuf;

	if (isblank (pacl->filename)) {
		if (!msg_chmod)
			LoadMessage ("msg_chmod", &msg_chmod, &msg_chmod_text);
		ErrorMessageOpen (-1, msg_chmod, 0, NULL);
		return -1;
	}

	perms_to_mode (pacl->user_perms, &new_mode);
	new_mode <<= 3;
	
	if (!isblank (pacl->mask_perms))
		perms_to_mode (pacl->mask_perms, &new_mode);
	else
		perms_to_mode (pacl->group_perms, &new_mode);
	new_mode <<= 3;
	
	perms_to_mode (pacl->other_perms, &new_mode);

	/* For an IPC object */

	if (is_ipc_object (pacl->filename))
		return (ipc_set_attrs (pacl->filename, &new_mode, -1, -1));

	/* For a file */

	if (stat (pacl->filename, &statbuf) != 0) {
		if (!msg_chmod)
			LoadMessage ("msg_chmod", &msg_chmod, &msg_chmod_text);
		ErrorMessageOpen (-1, msg_chmod, 4, NULL);
		return -1;
	}

	/* Make sure that the suid, etc. bits are preserved. */

	new_mode = (statbuf.st_mode & 0777) | new_mode;

	if (chmod (pacl->filename, new_mode) != 0) {
		if (!msg_chmod)
			LoadMessage ("msg_chmod", &msg_chmod, &msg_chmod_text);
		ErrorMessageOpen (-1, msg_chmod, 2, NULL);
		return -1;
	}
	return 0;
}

/*
 * IPC routines
 *
 * These handle the manipulation of the various IPC objects, including
 * setting permissions and owner ids.
 */

static char	**msg_ipc_open, *msg_ipc_open_text;

/* 
 * Open the IPC device, returning a file descriptor.
 */

int
ipc_open (filename)
char	*filename;
{
	int	fd, key;
	char	*s;

	key = atoi (filename + 1);

	s = filename + 1;

	while (isspace (*s)) 
		s++;

	if (key == 0 && !isdigit (*s)) {
		if (!msg_ipc_open)
			LoadMessage ("msg_ipc_open", 
				     &msg_ipc_open, 
				     &msg_ipc_open_text);
		ErrorMessageOpen (-1, msg_ipc_open, 0, NULL);
		return -1;
	}

	fd = key;

#if I_CHANGE_MY_MIND
        switch (filename [0]) {
        case 'm':
                fd = shmget (key, 0, 0);
                break;
        case 'q':
                fd = msgget (key, 0);
                break;
        case 's':
                fd = semget (key, 0, 0);
                break;
        }

        if (fd == -1) {
                if (!msg_ipc_open)
                        LoadMessage ("msg_ipc_open",
                                     &msg_ipc_open,
                                     &msg_ipc_open_text);
                ErrorMessageOpen (-1, msg_ipc_open, 4, NULL);
                return -1;
        }
#endif
	return fd;
}

/* 
 * Figure out the owner, creator, and all that jazz.
 * If the IPC read failed because of a wildcard ACL,
 * the entries will be empty.  We should fill 'em.  (Ugh.)
 */

int
ipc_stat (pacl)
struct paclif	*pacl;
{
	int		fd, status;
	char		*s;
	struct ipc_perm	*perms;
	struct msqid_ds	msg_buf;
	struct shmid_ds	mem_buf;
	struct semid_ds	sem_buf;

	static char	**msg_ipc_stat, *msg_ipc_stat_text;

	if ((fd = ipc_open (pacl->filename)) == -1)
		return -1;
	
	switch (pacl->filename [0]) {
	case 'm':
		status = shmctl (fd, IPC_STAT, &mem_buf);
		perms = &mem_buf.shm_perm;
		break;
	case 'q':
		status = msgctl (fd, IPC_STAT, &msg_buf);
		perms = &msg_buf.msg_perm;
		break;
	case 's':
		status = semctl (fd, 0, IPC_STAT, (int) &sem_buf);
		perms = &sem_buf.sem_perm;
		break;
	}

	if (status != 0) {
		if (!msg_ipc_stat)
			LoadMessage ("msg_ipc_stat", 
				     &msg_ipc_stat, 
				     &msg_ipc_stat_text);
		if (errno == EINVAL)
			ErrorMessageOpen (-1, msg_ipc_stat, 0, NULL);
		else
			ErrorMessageOpen (-1, msg_ipc_stat, 3, NULL);
		return -1;
	}

	if (s = pw_idtoname (perms->uid))
		strcpy (pacl->owner_name, s);
	else
		sprintf (pacl->owner_name, "%d", perms->uid);
	if (s = gr_idtoname (perms->gid))
		strcpy (pacl->owner_group, s);
	else
		sprintf (pacl->owner_group, "%d", perms->gid);
	if (s = pw_idtoname (perms->cuid))
		strcpy (pacl->creator_name, s);
	else
		sprintf (pacl->creator_name, "%d", perms->cuid);
	if (s = gr_idtoname (perms->cgid))
		strcpy (pacl->creator_group, s);
	else
		sprintf (pacl->creator_group, "%d", perms->cgid);

	if (isblank (pacl->user_perms)) {
		mode_to_perms (pacl->other_perms, perms->mode & 007);
		perms->mode >>= 3;
		mode_to_perms (pacl->group_perms, perms->mode & 007);
		perms->mode >>= 3;
		mode_to_perms (pacl->user_perms, perms->mode & 007);
		
		strcpy (pacl->backing_group_perms, pacl->group_perms);
		strcpy (pacl->backing_user_perms, pacl->user_perms);
		strcpy (pacl->backing_other_perms, pacl->other_perms);
	}
		
	return 0;
}

/*
 * Retrieve the ACL entry.  Return the count of entries read.
 */

ipc_read_acl (filename, acl)
char	*filename;
acl_t	acl;
{
	int	fd, i, count, status;
	char	*er;
	acle_t	*ir;

	static char	**msg_ipc_read, *msg_ipc_read_text;

	if ((fd = ipc_open (filename)) == -1)
		return -1;


	switch (filename [0]) {
	case 'm':
		count = shm_statacl (fd, NULL, 0);
		break;
	case 'q':
		count = msg_statacl (fd, NULL, 0);
		break;
	case 's':
		count = sem_statacl (fd, NULL, 0);
		break;
	}

	if ((count < 0) || 
	    !(ir = (acle_t *) Calloc (sizeof (acle_t), count))) {
		if (!msg_ipc_read)
			LoadMessage ("msg_ipc_read", 
				     &msg_ipc_read, 
				     &msg_ipc_read_text);
		if (sec_errno == ESEC_WILDCARD_TAG) {
#if STEVE_HAS_HIS_WAY
			ErrorMessageOpen (-1, msg_ipc_read, 0, NULL);
#endif
			return 0;
		} else {
			ErrorMessageOpen (-1, msg_ipc_read, 6, NULL);
			return -1;
		}
	}

	switch (filename [0]) {
	case 'm':
		status = shm_statacl (fd, ir, count);
		break;
	case 'q':
		status = msg_statacl (fd, ir, count);
		break;
	case 's':
		status = sem_statacl (fd, ir, count);
		break;
	}

	if (status < 0) {
		if (!msg_ipc_read)
			LoadMessage ("msg_ipc_read", 
				     &msg_ipc_read, 
				     &msg_ipc_read_text);
		ErrorMessageOpen (-1, msg_ipc_read, 2, NULL);
		return -1;
	}

	if ((er = acl_ir_to_er (ir, count)) == NULL) {
		Free (ir);
		acl_free (acl);
		if (!msg_ipc_read)
			LoadMessage ("msg_ipc_read", 
				     &msg_ipc_read, 
				     &msg_ipc_read_text);
		ErrorMessageOpen (-1, msg_ipc_read, 4, NULL);
		return -1;
	}

	Free (ir);

	if (acl_unpack (er, ACL_TEXT_PACKAGE, acl) == -1) {
		acl_free (acl);
		if (!msg_ipc_read)
			LoadMessage ("msg_ipc_read", 
				     &msg_ipc_read, 
				     &msg_ipc_read_text);
		ErrorMessageOpen (-1, msg_ipc_read, 4, NULL);
		return -1;
	}

	return count;
}

/*
 * Set the ACL on an IPC object.
 */

ipc_write_acl (filename, acl)
char	*filename;
acl_t	*acl;
{
	int	fd, count, size, status;
	char	*er;
	acle_t	*ir;

	static char	**msg_ipc_write, *msg_ipc_write_text;

	if ((fd = ipc_open (filename)) == -1)
		return -1;

	size = acl_package_size (acl, ACL_TEXT_PACKAGE);

	if (!(er = Calloc (1, size + 1))) {
		MemoryError ();
		return -1;
	}

	acl_pack (acl, er, size + 1, ACL_TEXT_PACKAGE);

	if (!(ir = acl_er_to_ir (er, &count))) {
		if (!msg_ipc_write)
			LoadMessage ("msg_ipc_write", 
				     &msg_ipc_write, 
				     &msg_ipc_write_text);
		ErrorMessageOpen (-1, msg_ipc_write, 0, NULL);
		return -1;
	}

	switch (filename [0]) {
	case 'm':
		status = shm_chacl (fd, ir, count);
		break;
	case 'q':
		status = msg_chacl (fd, ir, count);
		break;
	case 's':
		status = sem_chacl (fd, ir, count);
		break;
	}
	
	free (ir);
	Free (er);

	if (status != 0) {
		if (!msg_ipc_write)
			LoadMessage ("msg_ipc_write", 
				     &msg_ipc_write, 
				     &msg_ipc_write_text);
		if (errno != EINVAL)
			ErrorMessageOpen (-1, msg_ipc_write, 2, NULL);
		else
			ErrorMessageOpen (-1, msg_ipc_write, 2, NULL);

		return -1;
	}

	return 0;
}


ipc_set_attrs (filename, mode, uid, gid)
char	*filename;
int	*mode, uid, gid;
{
	int		fd, status;
	char		*s;
	struct ipc_perm	*perms;
	struct msqid_ds	msg_buf;
	struct shmid_ds	mem_buf;
	struct semid_ds	sem_buf;

	static char	**msg_ipc_set_attrs, *msg_ipc_set_attrs_text;
	
	if ((fd = ipc_open (filename)) == -1)
		return -1;
	
	switch (filename [0]) {
	case 'm':
		status = shmctl (fd, IPC_STAT, &mem_buf);
		perms = &mem_buf.shm_perm;
		break;
	case 'q':
		status = msgctl (fd, IPC_STAT, &msg_buf);
		perms = &msg_buf.msg_perm;
		break;
	case 's':
		status = semctl (fd, 0, IPC_STAT, (int) &sem_buf);
		perms = &sem_buf.sem_perm;
		break;
	}

	if (status != 0) {
		if (!msg_ipc_set_attrs)
			LoadMessage ("msg_ipc_set_attrs", 
				     &msg_ipc_set_attrs, 
				     &msg_ipc_set_attrs_text);
		if (errno != EINVAL)
			ErrorMessageOpen (-1, msg_ipc_set_attrs, 0, NULL);
		else
			ErrorMessageOpen (-1, msg_ipc_set_attrs, 5, NULL);
		return -1;
	}

	if (uid != -1)
		perms->uid = uid;
	
	if (gid != -1)
		perms->gid = gid;

	if (mode)
		perms->mode = *mode;

	switch (filename [0]) {
	case 'm':
		status = shmctl (fd, IPC_SET, &mem_buf);
		break;
	case 'q':
		status = msgctl (fd, IPC_SET, &msg_buf);
		break;
	case 's':
		status = semctl (fd, 0, IPC_SET, (int) &sem_buf);
		break;
	}

	if (status != 0) {
		if (!msg_ipc_set_attrs)
			LoadMessage ("msg_ipc_set_attrs", 
				     &msg_ipc_set_attrs, 
				     &msg_ipc_set_attrs_text);
		ErrorMessageOpen (-1, msg_ipc_set_attrs, 3, NULL);

		return -1;
	} else {
		return 0;
	}

}


