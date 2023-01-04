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
static char *rcsid = "@(#)$RCSfile: acc_callbacks.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:10:59 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	acc_callbacks.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:20  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:22  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:14  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:34  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * @(#)acc_callbacks.c	1.1 11:15:37 11/8/91 SecureWare
 * 
 * Copyright (c) 1991, SecureWare, Inc.  All rights reserved.
 */


/* #ident "@(#)acc_callbacks.c	1.1 11:15:37 11/8/91 SecureWare" */

/* 
 * This module contains the callbacks for the Access Test screen.
 */

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <If.h>
#include "paclif.h"

/* 
 * Set the permissions on the Test Access screen according to the 
 * ACL entries the user has selected.
 *
 */

determine_access (ids, pacl)
struct acc_ids	*ids;
struct paclif	*pacl;
{
	int	i, j, mask, allowed, mask_set, found;
	char	id [NUSERNAME];

	found = 0;
	allowed = 0;
	strcpy (id, trim_name (ids->username));

	/* If the username is valid, check for a match */

	if (id [0] && strcmp (id, trim_name (pacl->owner_name)) == 0) {
		perms_to_mode (pacl->user_perms, &allowed);
		mode_to_perms (ids->perms, allowed);
		return 0;
	}
	
	/* Check for the creator name (used by IPC objects) */

	if (id [0] && !isblank (pacl->creator_name) &&
	    strcmp (id, trim_name (pacl->creator_name)) == 0) {
		perms_to_mode (pacl->user_perms, &allowed);
		mode_to_perms (ids->perms, allowed);
		return 0;
	}
	
	mask = 0;
	perms_to_mode (pacl->mask_perms, &mask);
	mask_set = !isblank (pacl->mask_perms);

	/* if mask is 0, we could skip much of this, but, why bother? */

	/* 
	 * Check to see if the username matches anything in the list
	 * of users.
	 */

	if (id [0]) {
		for (i = 0; i < pacl->number_users; i++)
			if (strcmp (id, trim_name (pacl->user [i])) == 0) {
				perms_to_mode (pacl->user [i] + 9, &allowed);
				mode_to_perms (ids->perms, allowed & mask);
				return 0;
			}
	}

	/* 
	 * Check to see if any of the groups match the owner group
	 * or creator group (for IPC objects), and find the
	 * union of *all* of the permissions granted.
	 */

	strcpy (id, trim_name (pacl->owner_group));

	if (id [0]) {
		if (check_list_access (id, ids->group, ids->number_groups) 
		    != -1) {
			perms_to_mode (pacl->group_perms, &allowed);
			found = 1;
		} else {
			strcpy (id, trim_name (pacl->creator_group));
			if (!isblank (id))
				if (check_list_access (id, ids->group, 
						       ids->number_groups)) {
					perms_to_mode (pacl->group_perms, 
						       &allowed);
					found = 1;
				}
		}
	}

	/* Be sure to find the union of the access permissions allowed! */

	for (i = 0; i < pacl->number_groups; i++) {
		strcpy (id, trim_name (pacl->group [i]));
		for (j = 0; id [0] && j < ids->number_groups; j++) {
			if (strcmp (id, trim_name (ids->group [j])) == 0) {
				perms_to_mode (pacl->group [i] + 9, 
					       &allowed);
				found = 1;
			}
		}
	}

	if (found) {
		mode_to_perms (ids->perms, allowed & mask);
		return 0;
	}

	/* All else has failed.  We just provide the Other entry values. */

	perms_to_mode (pacl->other_perms, &allowed);
	mode_to_perms (ids->perms, allowed);
	return 0;
}

check_list_access (id, list, length)
char	*id;
char	**list;
int	length;
{
	int i;

	for (i = 0; i < length; i++)
		if (strcmp (id, trim_name (list [i])) == 0) 
			return i;
	
	return -1;
}


/*
 * Fill in the id for the subject access test structure.
 */

reset_ids (id)
struct acc_ids	*id;
{
	char	*name;
	static char	**msg_reset_ids, *msg_reset_ids_text;

	if (name = pw_idtoname (getluid()))
		strcpy (id->username, name);
	else 
		strcpy (id->username, "Unknown");
}

/*
 * Fill in the list of groups for the subject access test structure.
 * If no valid username is specified, then display a message.
 */

load_groups (id)
struct acc_ids	*id;
{
	int	i, gid;
	char	*name, *group;
	struct group	*grp;
	struct passwd	*pw;
	static char	**msg_load_groups, *msg_load_groups_text;

	name = trim_name (id->username);

	if (*name == 0) {
		if (!msg_load_groups)
			LoadMessage ("msg_load_groups", 
				     &msg_load_groups, 
				     &msg_load_groups_text);
		ErrorMessageOpen (-1, msg_load_groups, 0, NULL);
		return -1;
	}

	setpwent();
	if (!(pw = getpwnam (name))) {
		if (!msg_load_groups)
			LoadMessage ("msg_load_groups", 
				     &msg_load_groups, 
				     &msg_load_groups_text);
		ErrorMessageOpen (-1, msg_load_groups, 2, NULL);
		return -1;
	}

	/* 
	 * Find the first group from the /etc/passwd entry;  pick it out,
	 * then find the rest of the groups.  What a pain!
	 */
	
	group = gr_idtoname (gid = pw->pw_gid);

	free_cw_table (id->group);
	id->group = alloc_cw_table (2, NGROUPNAME);

	if (group)
		strcpy (id->group [0], group);
	else
		sprintf (id->group [0], "%d", gid);

	id->number_groups = 2;

	setgrent();
	while (grp = getgrent ()) {
		for (i = 0; grp->gr_mem [i]; i++) {
			if ((strcmp (grp->gr_mem [i], name) == 0) &&
			    (strcmp (grp->gr_name, id->group [0]) != 0)) {
				id->group = 
					expand_cw_table (id->group,
							 id->number_groups,
							 id->number_groups + 1,
							 NGROUPNAME);
				strcpy (id->group [id->number_groups - 1],
					grp->gr_name);
				id->number_groups++;
				break;
			}
		}
	}
	return 0;
}
