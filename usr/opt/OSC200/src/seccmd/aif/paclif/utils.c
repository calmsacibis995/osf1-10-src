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
static char *rcsid = "@(#)$RCSfile: utils.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:26 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	utils.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:53:01  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:54  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:54  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:28  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * @(#)utils.c	1.1 11:19:19 11/8/91 SecureWare
 * 
 * Copyright (c) 1991, SecureWare, Inc.  All rights reserved.
 */


/* #ident "@(#)utils.c	1.1 11:19:19 11/8/91 SecureWare" */

/*
 * This module contains the miscellaneous routines to handle 
 * the manipulate of permissions fields, including mask recalculation.
 *
 *  isblank () -- is the string empty?
 *  trim_name () -- return a string with trailing blanks removed
 *  mode_to_perms () -- convert mode word to a permissions string
 *  perms_to_mode () -- convert a permissions string to a mode word
 *  correct_permissions () -- clean up a perms field as it is entered
 *  check_position () -- check the user and group lines
 *  check_mask () -- mask auto calculate routine
 *  compute_mask () -- set mask to the union of all permissions
 *  purge_all_ineffectives () -- remove ineffective permissions
 *  set_all_effectives () -- set effective permissions for all fields
 *  set_effective () -- set effective permissions for one field
 *  perform_set_base () -- set the ACL back to just base permissions
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <acl.h>
#include <If.h>
#include "paclif.h"

/*
 * Determine if the string has only white space in it.
 */

isblank (s)
char	*s;
{
	while (*s && isspace(*s)) s++;

	return (*s == 0);
}

/*
 * Return a name with all of the trailing spaces lopped off.
 * This assumes that the name will be a username or groupname, 
 * which will fit into a space of a previously determined size.
 */

char *
trim_name (name)
char	*name;
{
	static char	t [NUSERNAME];
	char	*s = (char *) &t;

	while (*name && !isspace (*name))
		*s++ = *name++;

	*s = 0;
	return ((char *) &t);
}


/*
 * Get the permissions from the permissions vector, and set a string.
 */

mode_to_perms (field, value)
char		*field;
acl_permset_t	value;
{
	if (value & ACL_PREAD)
		field [0] = 'r';
	else 
		field [0] = '-';
	if (value & ACL_PWRITE)
		 field [1] = 'w';
	else 
		 field [1] = '-';
	if (value & ACL_PEXECUTE)
		 field [2] = 'x';
	else 
		  field [2] = '-';
}

/*
 * Set the permissions in the permissions vector, from a string.
 */

perms_to_mode (field, value)
char		*field;
acl_permset_t	*value;
{
	int	i;

	for (i = 0; i < 3 && field [i]; i++) {
		switch (field [i]) {
		case 'r':
		case 'R':
			*value |= ACL_PREAD;
			break;
		case 'w':
		case 'W':
			*value |= ACL_PWRITE;
			break;
		case 'x':
		case 'X':
			*value |= ACL_PEXECUTE;
			break;
		default:
			break;
		}
	}
}


/*
 * This routine will examine the permission field and the backing field
 * to determine what the user has done.  It will remove duplications,
 * and clear out unwanted characters.
 */

correct_permissions (new, old)
char	*new, *old;
{
	int i, j, l, c;

	c = 0;
	while (new [c] && old [c] && new [c] == old [c])
		c++;

	if (c == 3)
		return 0;

	/*
	 * If the string is not 3 characters long, fill it out with
	 * dashes.  This prevents us from seeing old permissions popping
	 * up into previously blank fields.
	 */

	if ((l = strlen (new)) < 3)
		for (i = l; i < 3; i++)
			new [i] = '-';

	for (i = c; i < 3; i++)
		switch (new [i]) {
		case 'R':
		case 'W':
		case 'X':
			new [i] = tolower(new [i]);
		case 'r':
		case 'w':
		case 'x':
			for (j = 0; j < 3; j++)
				if (i != j && new [i] == new [j])
					new [j] = '-';
			break;
		default:
			new [i] = '-';
			break;
		}
}


/*
 * Check to make sure that the entries in the lists are valid.
 * That is, they only display values in the appropriate fields.
 * The username should be confined to the first eight characters.
 * Positions 10-12 should contain the permissions set by the user
 * and positions 14-16 should contain the effective permissions.
 */

check_position (pacl, list, backing, size, mask)
struct paclif	*pacl;
char	**list, **backing;
int	size;
char	*mask;
{
	int	i, j, k, mode = 0, oops = 0;
	char	*s, *t;

	for (i = 0; i < size - 1 && !oops; i++) {
		s = list [i];
		t = backing [i];

		/* 
		 * Find the first character that differs on this line.
		 * If nothing differs, go on to the next line.
		 */

		j = 0;
		while (s [j] && t [j] && s [j] == t [j])
			j++;

		if (j == ENTRY_WIDTH)
			continue;

		/*
		 * First we calculate j, the position at which the first 
		 * differnt character was found?
		 *
		 * Here we have several cases:
		 *
		 * 1.  The user tried to change the effective permissions.
		 *     We do not allow this.
		 *     (j > 12)
		 * 2.  The user changed permissions.  We recalculate 
		 *     effectives, and blank out 9 and 13.
		 *     ( j > 8)
		 * 3.  The user has changed something in the username field.
		 *     We copy in the old permissions and blank 9 and 13.
		 *     (j < 9)
		 */

		if (j > 12) { /* case 1 */
			strncpy (s + 13, t + 13, 3);
		} else if (j > 8) { /* case 2 */
			s [8] = s [12] = ' ';
			correct_permissions (s + 9, t + 9);
			if (pacl->mask_recalculate)
				if (check_mask (pacl, s + 9, t + 9) == 0)
					set_effective (s + 9, s + 13, mask);
				else 
					strcpy (s, t);
			else
				set_effective (s + 9, s + 13, mask);
		} else 	if (j < 9) { /* case 3 */
			if (s [0] == 0) {  /* shouldn't happen! */
				strcpy (s, INITIAL_LINE);
			} else {
				strncpy (s + 8, t + 8, 8);
				if ((j = strlen (s)) < ENTRY_WIDTH && j < 9)
					strncpy (s, INITIAL_LINE, 8);
			}
		}
		
		strcpy (t, s);
		oops = 1;
	}

	return oops;
}


/*
 * Calculate the new mask value.   If the new value would change the
 * permissions on the other values, signal an error, and suggest that the
 * user set some of the toggles.  If the user has removed a permission
 * so that the mask no longer need display that permission, remove it.
 *
 * Otherwise, we find the union for the old values of the USER, GROUP, and 
 * GROUP_OBJ permissions, both actual and effective.  We then determine 
 * which permissions differ between the actual and effective rights.  
 * 
 * The new candidate mask value is the union of the newly granted
 * permissions and the old effective permissions.
 * 
 * If there are any permissions in the cadidate new mask that are also in
 * the permissions that differ betweeen the original actual and effective
 * rights, applying the candidate new mask would unexpectedly grant some new
 * right that the user did not intend;  Thus we protest.  Otherwise, we
 * set the new mask.
 */

#define XOR(a, b)	(~(((a) & (b)) | (~(a) & ~(b))))

check_mask (pacl, new, old)
struct paclif	*pacl;
char		*new, *old;
{
	int	i, old_actual, old_effective, old_mask, old_permission,
		old_difference, new_difference, new_mask, new_permission,
		new_actual, difference;
	char	temp_perms [4];

	static char	**msg_acl_set_mask, *msg_acl_set_mask_text;

	new_permission = old_permission = 0;

	perms_to_mode (new, &new_permission);
	perms_to_mode (old, &old_permission);
	
	difference = XOR (new_permission, old_permission);

	bcopy (old, temp_perms, 4);

	/* Remove any bits that are to be removed.... */

	if (difference & old_permission)
		mode_to_perms (old, ~(difference & old_permission) & 
			       old_permission);

	old_mask = old_actual = new_actual = 0;
	perms_to_mode (pacl->mask_perms, &old_mask);
	perms_to_mode (pacl->backing_group_perms, &old_actual);

	for (i = 0; i < pacl->number_users - 1; i++) {
		perms_to_mode (pacl->backing_user [i] + 9, &old_actual);
	}

	for (i = 0; i < pacl->number_groups - 1; i++) {
		perms_to_mode (pacl->backing_group [i] + 9, &old_actual);
	}

	old_effective = old_actual & old_mask;
	old_difference = XOR (old_effective, old_actual);
	
	new_difference = difference & new_permission;
	new_mask = new_difference | old_effective;

	bcopy (temp_perms, old, 4);

	if (new_mask & old_difference) {
		beep ();
		return -1;
	}

#if I_CHANGE_MY_MIND
	old_mask = old_actual = new_actual = 0;
	perms_to_mode (pacl->mask_perms, &old_mask);
	perms_to_mode (pacl->backing_group_perms, &old_actual);
	perms_to_mode (pacl->group_perms, &new_actual);

	for (i = 0; i < pacl->number_users; i++) {
		perms_to_mode (pacl->backing_user [i] + 9, &old_actual);
		perms_to_mode (pacl->user [i] + 9, &new_actual);
	}

	for (i = 0; i < pacl->number_groups; i++) {
		perms_to_mode (pacl->backing_group [i] + 9, &old_actual);
		perms_to_mode (pacl->group [i] + 9, &new_actual);
	}

	old_effective = old_actual & old_mask;
	old_difference = XOR (old_effective, old_actual);
	new_difference = XOR (old_actual, new_actual);

	new_mask = old_mask;

	if (new_difference & new_actual) {  /* we've added some perms */
		new_mask = new_mask | (new_difference & new_actual);
	}

	if (new_difference & old_actual) { /* we've removed some perms */
		new_mask = new_mask & (~(new_difference & old_actual));
	}

#if I_REALLY_MUCKED_UP
	new_difference = XOR (new_permission, old_permission);

	
	
	new_mask = new_difference | old_effective;

	if (new_mask & old_difference) {
#if I_FIGURE_OUT_WHY

		/*
		 * This *probably* doesn't work because it is getting
		 * called from a val_act routine.  (As it is, it doesn't
		 * bother to erase the screen before displaying the message.
		 */

		if (!msg_acl_set_mask)
			LoadMessage ("msg_acl_set_mask", 
				     &msg_acl_set_mask, 
				     &msg_acl_set_mask_text);
		ErrorMessageOpen (-1, msg_acl_set_mask, 0, NULL);
#endif
		beep ();
		return -1;
	}
#endif
#endif

	mode_to_perms (pacl->mask_perms, new_mask);

	set_all_effectives (pacl);

	return 0;
}

/*
 * Compute the mask by OR'ing together all of the selected values.
 */

compute_mask (pacl)
struct paclif	*pacl;
{
	int	i, permissions;

	permissions = 0;
	perms_to_mode (pacl->group_perms, &permissions);

	for (i = 0; i < pacl->number_users; i++) 
		perms_to_mode (pacl->user [i] + 9, &permissions);

	for (i = 0; i < pacl->number_groups; i++) 
		perms_to_mode (pacl->group [i] + 9, &permissions);

	mode_to_perms (pacl->mask_perms, permissions);

	set_all_effectives (pacl);
}

/*
 * Purge all of the selected privileges which are ineffective because
 * of the mask value.
 */

purge_all_ineffectives (pacl)
struct paclif	*pacl;
{
	int	i, mask, permissions;

	perms_to_mode (pacl->mask_perms, &mask);

	permissions = 0;
	perms_to_mode (pacl->group_perms, &permissions);
	permissions &= mask;
	mode_to_perms (pacl->group_perms, permissions);
	strcpy (pacl->backing_group_perms, pacl->group_perms);

	for (i = 0; i < pacl->number_users; i++) {
		permissions = 0;
		perms_to_mode (pacl->user [i] + 9, &permissions);
		permissions &= mask;
		mode_to_perms (pacl->user [i] + 9, permissions);
	}

	for (i = 0; i < pacl->number_groups; i++) {
		permissions = 0;
		perms_to_mode (pacl->group [i] + 9, &permissions);
		permissions &= mask;
		mode_to_perms (pacl->group [i] + 9, permissions);
	}

	set_all_effectives (pacl);
}

set_all_effectives (pacl)
struct paclif	*pacl;
{
	int	i;

	if (pacl->number_users == 1 && pacl->number_groups == 1 &&
	    isblank (pacl->mask_perms))
		return;

	for (i = 0; i < pacl->number_users; i++) {
		set_effective (pacl->user [i] + 9, pacl->user [i] + 13,
			       pacl->mask_perms);
		strcpy (pacl->backing_user [i], pacl->user [i]);
	}

	for (i = 0; i < pacl->number_groups; i++) {
		set_effective (pacl->group [i] + 9, pacl->group [i] + 13,
			       pacl->mask_perms);
		strcpy (pacl->backing_group [i], pacl->group [i]);
	}

	/* 
	 * Don't compute the group effective if the mask is not set
	 * and no other effectives are displayed.
	 */

	set_effective (pacl->group_perms, pacl->group_eff, pacl->mask_perms);
	strcpy (pacl->backing_group_perms, pacl->group_perms);
}

/*
 * Set the effective permissions for individual entries from the 
 * users and groups lists.
 */

set_effective (perms, eff, mask)
char	*perms, *eff, *mask;
{
	int	value = 0, mask_value = 0;

	perms_to_mode (perms, &value);
	perms_to_mode (mask, &mask_value);

	value &= mask_value;

	mode_to_perms (eff, value);
}


/*
 * Remove all non-base entries from the ACL (clearing the 'Mask Perms',
 * 'User', and 'Group' fields.  Set the group permissions to the
 * intersection of the group permissions and the mask permissions.
 */

perform_set_base (pacl)
struct paclif	*pacl;
{
	if (!isblank (pacl->mask_perms)) {
		strcpy (pacl->group_perms, pacl->group_eff);
		strcpy (pacl->backing_group_perms, pacl->group_eff);
	}

	pacl->group_eff [0] = 
	pacl->mask_perms [0] = 
        pacl->backing_mask_perms [0] = 0;
	
	free_cw_table (pacl->user);
	free_cw_table (pacl->backing_user);
	free_cw_table (pacl->group);
	free_cw_table (pacl->backing_group);

	pacl->user = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->group = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->backing_user = alloc_cw_table (1, ENTRY_WIDTH + 1);
	pacl->backing_group = alloc_cw_table (1, ENTRY_WIDTH + 1);

	pacl->number_users = pacl->number_groups = 1;
}
