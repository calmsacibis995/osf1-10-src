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
static char *rcsid = "@(#)$RCSfile: pacllib.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/04/01 20:24:37 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	pacllib.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.1.2.5  1992/04/30  17:27:30  build
 * 	Add acl_tag_to_ir for use by reduce to supprot POSIX acls.
 * 	[1992/04/30  13:47:37  valin]
 *
 * Revision 1.1.2.4  1992/04/24  17:45:49  valin
 * 	Fix documentation to reflect changes to code for POSIX acls.
 * 	[1992/04/24  17:38:51  valin]
 * 
 * 	fix the acl_er_to_ir routine to allow POSIX ACL entries to be separated
 * 	by commas as well as a new line character (e.g., ACL's entered from a
 * 	command line).
 * 	[1992/04/24  17:16:57  valin]
 * 
 * Revision 1.1.2.3  1992/04/23  18:12:53  marquard
 * 	Fixed pacl_init_ioctl() to call close() whether ioctl() fails or not.
 * 	[1992/04/09  20:58:07  marquard]
 * 
 * Revision 1.1.2.2  1992/04/05  19:50:46  marquard
 * 	POSIX ACL security library functions.
 * 	[1992/04/05  13:52:58  marquard]
 * 
 * $OSF_EndLog$
 */

/*
 * Copyright (c) 1988-1991 SecureWare, Inc.  All rights reserved.
 *
 *	@(#)pacllib.c	1.9 09:49:52 10/16/91 SecureWare
 *
 * This is unpublished proprietary source code of SecureWare, Inc. and
 * should be treated as confidential.
 */

#include <sys/secdefines.h>

#if SEC_ACL_POSIX /*{*/

#include <stdio.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/secioctl.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#include <sys/sec_objects.h>
#include <acl.h>
#include "libsecurity.h"


static acl_data_t	acl_da;		  /* data format ACL */
static acle_t		*acl_ir = NULL;	  /* working ACL ir buffer */
static int		acl_ir_num = 0;	  /* number of data entries */
static int		acl_ir_sz = 0;    /* number of possible entries */

static char		*acl_er = NULL;	  /* working ACL er buffer */
static int		acl_ersz = 0;	  /* number of entries in it */

static int pacl_init_ioctl();
static int get_perm ();
static int get_name ();
static int check_ir_space ();
static int check_er_space ();
static int is_qualifier_unique ();

#define	INIT_BUFSIZE	256
#define MAX_ENTRY_SIZE	35

#define MAX_IR_ENTRIES	SEC_MAX_IR_SIZE / ACL_IR_SIZE

#define	CLIENT		1

static int 		paclinit = 0;
struct acl_config	acl_config;	/* policy configuration parameters */
static struct sp_init	sp_init;	/* initialization structure */

/*
 * FUNCTION:
 *	acl_add_perm()
 *
 * ARGUMENTS:
 *	acl_entry_t entry_d;	- ACL entry to be changed.
 *	permset_t   perms;      - permissions to be added.
 *
 * DESCRIPTION:
 *	The acl_add_perm() function adds the set of 
 *	permissions contained in the argument perms
 *	to the acl entry referred to by entry_d.
 *	Any previous permissions will be left
 *	unchanged.  Any attempt to add permissions
 *	that are already present shall not be 
 *	considered an error.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 *			 if perms does not contain valid file permissions.
 */

int
acl_add_perm (entry_d, perms)
acl_entry_t	entry_d;
acl_permset_t	perms;
{

	/*
	 * Check for bad pointer to entry
	 * out of bounds perms, or bad entry.
	 */
	if ((entry_d == (acl_entry_t)0) ||
	    (perms & ~ACL_PRDWREX) ||
	    (entry_d->acl_magic != ACL_MAGIC)) {
		errno = EINVAL;
		return -1;
	}

	entry_d->acl_perm |= perms;

	return 0;
}


/*
 * FUNCTION:
 *	acl_alloc()
 *
 * ARGUMENTS:
 *	acl_t *acl;	- the acl entry;
 *
 * DESCRIPTION:
 *	The acl_alloc() routine allocates and initializes
 *	working storage area where an ACL may be manipulated.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		ENOMEM - if unable to allocate the required memory.
 *	
 */

int
acl_alloc (acl)
acl_t	*acl;
{
	acl_entry_t	ent;
	acl_t		newacl;

	/* check argument */
	if (acl == (acl_t *)NULL) {
		errno = EINVAL;
		return -1;
	}

	/*  Initialize ACL struct. */

	newacl = (acl_t)calloc (1,ACL_SIZE);
	if (!newacl) {
		errno = ENOMEM;
		return -1;
	}

	/* mark working storage as valid */
	newacl->acl_magic = ACL_MAGIC;

	/* Add initial entries */
	if (check_wk_space (newacl, INIT_ACL_SIZE))
		return -1;

	*acl = newacl;

	return 0;
}

/*
 * FUNCTION:
 *	acl_calc_mask()
 *
 * ARGUMENTS:
 *	acl_t acl; - working space previously allocated by acl_alloc().
 *
 * DESCRIPTION:
 *	The acl_calc_mask() function calculates the MASK_OBJ entry of the ACL
 *	indicated by the acl descriptor.
 *	 - The value of MASK_OBJ shall be the union of the permissions
 *		of all ACL entries that refer to members of the
 *		File Group Class. The file group class is all
 *		extended entries plus the GROUP_OBJ entry.
 *	 - The value of MASK_OBJ is identical to the file group
 *		class permission bits.
 *	 - If the ACL contains a MASK_OBJ already,
 *	 	its permissions are overwritten; if it does not contain a
 *		MASK_OBJ, one is added.
 *	 - No assumption is made that the acl passed has been validated
 *		by acl_valid ().  Therefore, no checks are performed.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if argument acl is not valid.
 *		ENOMEM - if acl_calc mask is unable to 
 *			allocate enough memory for a MASK_OBJ entry.
 */

int
acl_calc_mask (acl)
acl_t	acl;
{
	acl_permset_t		newperm = 0;
	acl_entry_t		maskent = (acl_entry_t)NULL;
	register acl_entry_t	ent;

	/* check arguments */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
		!acl->acl_first) {
		errno = EINVAL;
		return -1;
	}

	ent = acl->acl_first;

	/*
	 * Increment through entries and
	 * calculate the mask from GROUP_OBJ
	 * and all USER and GROUP entries.
	 */
	while (ent) {

		switch (ent->acl_tag) {
		
			case USER :
			case GROUP :
			case GROUP_OBJ :
				newperm |= ent->acl_perm;
				continue;

			case MASK_OBJ :
				maskent = ent;
				continue;
		}
		ent = (acl_entry_t) ent->acl_next;
	}

	/*
	 * If we haven't got the mask,
	 * create an entry in the ACL.
	 * We will get a pointer to the
	 * new entry in the return argument.
	 */
	if (maskent == (acl_entry_t)NULL) {

		if (acl_create_entry(acl,&maskent))
			return -1;
		maskent->acl_tag = MASK_OBJ;
	}

	maskent->acl_perm = newperm;

	return 0;
}


/*
 * FUNCTION:
 *	acl_copy_entry()
 *
 * ARGUMENTS:
 *	acl_entry_t	src_d;	- the source entry.
 *	acl_entry_t	dest_d;	- the destination entry.
 *
 * DESCRIPTION:
 *	The acl_copy_entry() function copies the contents of the
 *	entry indicated by the src_d descriptor to the ACL entry
 *	indicated by the dest_d descriptor.  The previous
 *	contents will be overwritten.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - Argument src_d or dest_d does not
 *			refer to a valid ACL entry.
 */


int
acl_copy_entry (src_d, dest_d)
acl_entry_t src_d;
acl_entry_t dest_d;
{
	/*
	 * Check for valid entries
	 */
	if (!dest_d || dest_d->acl_magic != ACL_MAGIC ||
	    !src_d || src_d->acl_magic != ACL_MAGIC) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * In order to preserve the head pointer
	 * of the destination entry, this is an
	 * assignment instead of a memcpy.
	 */
	
	dest_d->acl_tag = src_d->acl_tag;
	dest_d->acl_perm = src_d->acl_perm;
	dest_d->acl_id = src_d->acl_id;

	return 0;
}



/*
 * FUNCTION:
 *	acl_create_entry()
 *
 * ARGUMENTS:
 *	acl_t acl;		- pointer to acl to be added to.
 *	acl_entry_t *entry_dp;	- return descriptor for new entry.
 *
 * DESCRIPTION:
 *	The acl_create_entry() function creates a new ACL entry in
 *	the ACL indicated by the descriptor acl.  If there is
 *	insufficient space in the ACL for creating a new entry,
 *	then additional memory is allocated.  The descriptor for this
 *	new ACL entry shall be returned in the location referred to
 *	by argument entry_dp.
 *	The components of the new ACL entry are initialized
 *	in the following ways.
 *		- The ACL tag type component shall not
 *		  refer to any possible ACL tag type.
 *		- The qualifier shall not refer to any possible
 *		  user id or group id.
 *		- The set of permissions shall have no permissions
 *		  denoted.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - acl does not refer to a ACL allocated by acl_alloc().
 *		ENOMEM - The ACL working storage requires more memory than the
 *			system is able to provide.
 */

int
acl_create_entry (acl, entry_dp)
acl_t		acl;
acl_entry_t	*entry_dp;
{
	register acl_entry_t	ent;
	register int		i;

	/* check arguments */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
		entry_dp == (acl_entry_t *)NULL) {
		errno = EINVAL;
		return -1;
	}

	/* Make sure there is room in the acl. */
	if (check_wk_space (acl, acl->acl_num + 1))
		return -1;

	ent = acl->acl_first;
	for (i=0; i < acl->acl_num; i++)
		ent = (acl_entry_t) ent->acl_next;

	/* initialize the entry members */
	ent->acl_tag = BOGUS_OBJ;
	ent->acl_uid = BOGUS_UID;
	ent->acl_perm = ACL_NOPERM;

	/* set return pointer to next acl entry */
	*entry_dp = ent;

	/* increment number of entries */
	acl->acl_num++;

	return 0;
}

/*
 * FUNCTION:
 *	acl_delete_entry()
 *
 * ARGUMENTS:
 *	acl_entry_t entry_d;	- entry to be removed.
 *
 * DESCRIPTION:
 *	The acl_delete_entry() function shall remove the ACL
 *	entry indicated by entry_d descriptor from the ACL
 *	that contains it.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - entry_d does not refer to a valid ACL entry.
 */

int
acl_delete_entry (entry_d)
acl_entry_t	entry_d;
{
	acl_t		acl;
	acl_entry_t	curr_ent = (acl_entry_t)NULL;
	acl_entry_t	prev_ent = (acl_entry_t)NULL;

	/* 
	 * Check argument.
	 * Check that entry_d is in range and valid.
	 * Check that ACL is valid and has at least one entry.
	 */
	if (!(entry_d) || (entry_d->acl_magic != ACL_MAGIC) ||
	    !(acl = entry_d->acl_head) || (acl->acl_magic != ACL_MAGIC) ||
	    !(acl->acl_num)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Increment through entries and
	 * look for the one we want.
	 * When found, remove it from the list.
	 */
	curr_ent = acl->acl_first;
	while (curr_ent) {

		if (curr_ent == entry_d) {

			/* If previous entry is NULL we have first entry. */
			if (!prev_ent)
				acl->acl_first=(acl_entry_t)curr_ent->acl_next;
			else
				prev_ent->acl_next = curr_ent->acl_next;

			entry_d = (acl_entry_t)NULL;
			break;
		}
		prev_ent = curr_ent;
		curr_ent = (acl_entry_t) curr_ent->acl_next;
	}

	/* if entry_d is not NULL we didn't find it */
	if (entry_d != (acl_entry_t)NULL) {
		errno = EINVAL;
		return -1;
	}

	acl->acl_num--;
	acl->acl_size--;

	return 0;
}
		
/*
 * FUNCTION:
 *	acl_delete_perm()
 *
 * ARGUMENTS:
 *	acl_entry_t entry_d;	- ACL entry to be changed.
 *	permset_t perms;	- permissions to be deleted.
 *
 * DESCRIPTION:
 *	The acl_delete_perm() function deletes the set of 
 *	permissions contained in the argument perms
 *	to the acl entry referred to by entry_d.
 *	Any previous permissions will be left
 *	unchanged.  Any attempt to delete permissions
 *	that are are not present shall not be 
 *	considered an error.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 *			 if perms does not contain valid file permissions.
 */


int
acl_delete_perm (entry_d, perms)
acl_entry_t	entry_d;
acl_permset_t	perms;
{

	/*
	 * check for bad pointer to entry
	 * or out of bounds perms
	 */
	if ((entry_d == (acl_entry_t)0) ||
	    (entry_d->acl_magic != ACL_MAGIC) ||
	    (perms & ~ACL_PRDWREX)) {
		errno = EINVAL;
		return -1;
	}

	/* delete permission in the acl entry */

	entry_d->acl_perm &= ~perms;

	return 0;
}



/*
 * FUNCTION:
 *	acl_free()
 *
 * ARGUMENTS:
 *	acl_t acl;	- pointer to the acl entry;
 *
 * DESCRIPTION:
 *	The acl_free() routine deallocates space allocated
 *	by acl_alloc().
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - pointer to acl_t is invalid.
 *	
 */


int
acl_free (acl)
acl_t	acl;
{
	register acl_entry_t	ent;
	register acl_entry_t	next;

	/*
	 * Check the acl argument.
	 */
	if ((acl == (acl_t)NULL) ||
	    (acl->acl_magic != ACL_MAGIC)) {
		errno = EINVAL;
		return -1;
	}

	ent = acl->acl_first;

	while (ent) {

		if (ent->acl_magic == ACL_MAGIC) {
			next = (acl_entry_t)ent->acl_next;
			free (ent);
			ent = next;
		}

		else
			break;

	}

	free (acl);

	return 0;
}


/*
 * FUNCTION:
 *	acl_get_entry()
 *
 * ARGUMENTS:
 *	acl_t 	acl;		- working space allocated by acl_alloc().
 *	acl_entry_t *entry_dp;	- pointer to ACL entry returned.
 *
 * DESCRIPTION:
 *	The acl_get_entry() function shall get a descriptor
 *	to the next ACL entry of the ACL indicated by argument
 *	acl.  The descriptor shall be returned in the 
 *	location referred to by the argument entry_dp.
 *	 - acl refers to an ACL that shall have been previously
 *		allocated by acl_alloc().
 *	 - The first call to acl_get_entry() following a call to
 *		acl_read() shall obtain the first entry in the ACL,
 *		as ordered by the system.
 *
 *	Subsequent calls to acl_get_entry()
 *	shall obtain successive ACL entries, until the last
 *	entry is obtained.  After the last entry has been obtained
 *	the value 0 shall be returned.  Calls to acl_get_entry()
 *	shall not modify an ACL entry or the ACL on a file.
 *
 * RETURNS:
 *	NOERROR - (1) if there are ACLs on the file.
 *		  (0) if there are no more ACLs on the file.
 *	ERROR   - (-1) and errno set.
 *		EINVAL - acl doesn't refer to an ACL allocated by acl_alloc().
 */

int
acl_get_entry (acl,entry_dp)
acl_t		acl;
acl_entry_t	*entry_dp;
{
	register int		i;
	register acl_entry_t	ent;

	/*
	 * Check the acl argument.
	 */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
		entry_dp == (acl_entry_t *)NULL) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * if there aren't any entries
	 * or if we have reached the last one
	 */
	if (acl->acl_num == 0 || acl->acl_counter >= acl->acl_num)
		return 0;

	/* get to the next entry */
	ent = acl->acl_first;
	for (i=0; i < acl->acl_num; i++) {

		if ( i == acl->acl_counter )
			break;

		ent = ent->acl_next;
	}

	*entry_dp = ent;

	acl->acl_counter++;

	return 1;
}

/*
 * FUNCTION:
 *	acl_rewind()
 *
 * ARGUMENTS:
 *	acl_t acl;	- working space previously allocated by acl_alloc()
 *
 * DESCRIPTION:
 *	The acl_rewind() function rewinds the internal descriptor
 *	for the acl argument such that a subsequent call to 
 *	acl_get_entry() using the same acl argument shall obtain
 *	the first entry in the ACL.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - acl does not refer to a valid ACL entry.
 */


int
acl_rewind(acl)
acl_t	acl;
{
	
	/* Check the acl argument.  */

	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC) {
		errno = EINVAL;
		return -1;
	}

	acl->acl_counter = 0;

	return 0;

}

/*
 * FUNCTION:
 *	acl_get_perm()
 *
 * ARGUMENTS:
 *	acl_entry_t	entry_d; - an ACL entry.
 *	acl_permset_t	*perms;  - place to return the permissions;
 *
 * DESCRIPTION:
 *	The acl_get_perm() function retrieves the permissions
 *	member from the acl entry passed.  Any permissions
 *	currently on the ACL will be left unchanged.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 */

int
acl_get_perm (entry_d, perms)
acl_entry_t	entry_d;
acl_permset_t	*perms;
{

	/* check arguments */
	if (entry_d == (acl_entry_t)NULL ||
		entry_d->acl_magic != ACL_MAGIC ||
		perms == (acl_permset_t *)NULL) {
		errno = EINVAL;
		return -1;
	}

	/* return the perms member */
	*perms = entry_d->acl_perm;

	return 0;
}


/*
 * FUNCTION:
 *	acl_get_tag()
 *
 * ARGUMENTS:
 *	acl_entry_t entry_d;	- the ACL entry.
 *	acl_tag_t acl_tag_t;	- place to return the tag.
 *	void	*tag_qualifier;	- the tag qualifier.
 *
 * DESCRIPTION:
 *	The acl_get_tag() function returns the tag_type and
 *	qualifier of the tag for the ACL entry indicated by the
 *	the argument entry_d in the tag_type argument.
 *	The location referred to by the tag_qualifier shall be
 *	set to the qualifier data contained within the ACL entry.
 *
 *	The returned value of the tag_qualifier is determined
 *	as follows:
 *		USER - uid_t;
 *		GROUP - gid_t;
 *		USER_OBJ,GROUP_OBJ,OTHER_OBJ,MASK_OBJ - undefined.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 */


int
acl_get_tag (entry_d, tag_type, tag_qualifier)
acl_entry_t	entry_d;
acl_tag_t	*tag_type;
void		*tag_qualifier;
{

	/* check arguments */
	if (entry_d == (acl_entry_t)0 || entry_d->acl_magic != ACL_MAGIC) {
		errno = EINVAL;
		return -1;
	}


	/* get qualifier */
	switch (entry_d->acl_tag) {

		case (USER)	 :

			*(void **)tag_qualifier = (void *)entry_d->acl_uid;
			break;
				
		case (GROUP)	 :

			*(void **)tag_qualifier = (void *)entry_d->acl_gid;
			break;
				
		case (USER_OBJ)  :
		case (GROUP_OBJ) :
		case (MASK_OBJ)	 :
		case (OTHER_OBJ) :

			*(void **)tag_qualifier = (void *)0;
			break;
	
		case (BOGUS_OBJ) : 
		default :

			errno = EINVAL;
			return -1;
	}

	/* get tag type */
	*tag_type = entry_d->acl_tag;

	return 0;
}


/*
 * FUNCTION:
 *	acl_package_size()
 *
 * ARGUMENTS:
 *	acl_t 		   acl;     - space allocated by acl_alloc().
 *	acl_package_type_t pack_type; - ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 *
 * DESCRIPTION:
 *	The acl_package_size () function determines the number of bytes
 *	necessary to represent the data or text package as specified
 *	by acl.
 *	
 * NOTES:
 * 	- The MAX_ENTRY_SIZE entry character limit is broken down as follows:
 *		\n tag : id : perm \n \t #effective \n \0
 *		 1  5  1 10 1  3    1  1     10      1  1 = 35
 *
 *	- A carriage return will be appended to
 *		the head of the first group
 *		and other entry.
 *	- The largest tag is other which has five characters.
 *	- The largest id will be -1 as a string (4294967295).
 *
 * RETURNS:
 *	NOERROR - The length needed in bytes.
 *	ERROR   - (-1) and errno set.
 *		EINVAL - argument acl is not valid.
 *			 pack type is not ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 */

ssize_t
acl_package_size (acl, pack_type)
acl_t		acl;
acl_package_type_t	pack_type;
{
	ssize_t	size;

	if ((acl == (acl_t)NULL) || acl->acl_magic != ACL_MAGIC ||
	    (pack_type != ACL_DATA_PACKAGE && pack_type != ACL_TEXT_PACKAGE)) {
		errno = EINVAL;
		return -1;
	}

	if (pack_type == ACL_TEXT_PACKAGE)
		size = acl->acl_num * MAX_ENTRY_SIZE;
	else
		size = (acl->acl_num * ACL_IR_SIZE) + ACL_DATA_SIZE;

	return (size);

}

/*
 * FUNCTION:
 *	acl_unpack()
 *
 * ARGUMENTS:
 *	char 		   *buf;      - data to unpack.
 *	acl_package_type_t pack_type; - ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 *	acl_t 		   acl;       - space allocated by acl_alloc().
 *
 * DESCRIPTION:
 *	The acl_unpack() function translates the text or data package
 *	into an ACL.  Additional memory is allocated if needed.
 *
 * RETURNS:
 *	NOERROR - The length of the translated acl in bytes.
 *	ERROR   - (-1) and errno set.
 *		ENOMEM - insufficient memory available to store acl.
 *		EINVAL - argument acl is not valid.
 *			 pack type is not ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 */

int
acl_unpack (buf, pack_type, acl)
char 		   *buf;
acl_package_type_t pack_type;
acl_t 		   acl;
{
	acl_data_t	data;
	acle_t		*ent;
	acl_entry_t	went;
	int		numents, len;
	register int	i;

	/* Check arguments */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
	    (pack_type != ACL_DATA_PACKAGE && pack_type != ACL_TEXT_PACKAGE)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Translate from data package representation
	 * to working storage (acl_entry_t format).
	 */ 
	if (pack_type == ACL_DATA_PACKAGE) {

		data = (acl_data_t)buf;
		if (!data || data->acl_hdr.acl_magic != ACL_MAGIC) {
			errno = EINVAL;
			return -1;
		}

		numents = data->acl_hdr.acl_num;
		ent = (acle_t *) (data + 1);
	}

	/*
	 * Translate from external representation (lsacl format)
	 * to working storage (acl_entry_t format).
	 */ 
	if (pack_type == ACL_TEXT_PACKAGE) {

		/* Translate the text package to an ACL.  */
		ent = (acle_t *)acl_er_to_ir (buf, &numents);
		if (ent == (acle_t *)NULL)
			return -1;
	}

	/* get enough space for numents entries */
	if (check_wk_space (acl, numents))
		return -1;

	went = acl->acl_first;
	acl->acl_num = numents;

	for (i=0; i < numents; i++, ent++) {

		went->acl_head = (void *)acl;
		went->acl_tag  = ent->acl_tag;
		went->acl_perm = ent->acl_perm;

		/*
		 * Qualifiers for USER_OBJ, GROUP_OBJ, OTHER_OBJ, and
		 * MASK_OBJ entries must be 0, regardless of how they
		 * are stored in the policy database.
		 */

		switch (went->acl_tag) {
		case USER_OBJ:
		case GROUP_OBJ:
		case OTHER_OBJ:
		case MASK_OBJ:
			went->acl_uid = (uid_t) 0;
			break;
		default:
			went->acl_id = ent->acl_id;
			break;
		}

		went = (acl_entry_t) went->acl_next;
	}

	return (numents * ACL_ENT_SIZE);
}

/*
 * FUNCTION:
 *	acl_pack()
 *
 * ARGUMENTS:
 *	acl_t 		   acl;       - space allocated by acl_alloc().
 *	char 		   *buf;      - place to put the result.
 *	ssize_t		   buf_len;   - maximum bytes to store.
 *	acl_package_type_t pack_type; - ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 *
 * DESCRIPTION:
 *	The acl_pack() function translates the ACL in working storage
 *	referred to by the argument acl into memory referred to 
 *	by buf in a data or text package as specified by pack_type.
 *	The acl_pack() function will only package up to buf_len bytes.
 *
 * RETURNS:
 *	NOERROR - The length of the translated acl in bytes.
 *	ERROR   - (-1) and errno set.
 *		ENOMEM - buf space insufficient to store acl.
 *		EINVAL - argument acl is not valid.
 *			 pack type is not ACL_DATA_PACKAGE or ACL_TEXT_PACKAGE.
 */

ssize_t
acl_pack (acl, buf, buf_len, pack_type)
acl_t 		   acl;
char 		   *buf;
ssize_t		   buf_len;
acl_package_type_t pack_type;
{
	char			*erep;
	ssize_t			len;
	register acl_entry_t	ent;
	acl_data_t		data;
	acle_t			*eptr;
	register int 		i;

	/* Check arguments */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
	   (pack_type != ACL_DATA_PACKAGE && pack_type != ACL_TEXT_PACKAGE) ||
	   (!buf)) {
		errno = EINVAL;
		return -1;
	}

	/* If no entries. */
	if (!acl->acl_first)
		return 0;

	/*
	 * Translate from working storage (acl_entry_t format)
	 * to external representation (lsacl format).
	 */ 
	if (pack_type == ACL_TEXT_PACKAGE) {

		/*
		 * First convert from working storage to
		 * data package format.  This is the easiest
		 * way to do this because all the translation
		 * code is in the acl_ir_to_er() routine already.
		 */

		/* set up the data package pointers and allocate space */
		if (check_ir_space (acl->acl_num))
			return -1;

		eptr = acl_ir;
		ent = acl->acl_first;

		for (i=0; i < acl->acl_num; i++, eptr++) {

			eptr->acl_tag  = ent->acl_tag;
			eptr->acl_perm = ent->acl_perm;
			eptr->acl_id   = ent->acl_id;
			ent = (acl_entry_t) ent->acl_next;
		}

		erep = (char *)acl_ir_to_er (acl_ir, acl->acl_num);
		if (erep == (char *)NULL)
			return -1;

		if ((len = strlen (erep) + 1) > buf_len) {
			errno = ENOMEM;
			return -1;
		}

		strncpy (buf,erep,buf_len);

		return len;
	}

	/*
	 * Translate from working storage (acl_entry_t format)
	 * to data package representation.
	 */ 

	if (pack_type == ACL_DATA_PACKAGE) {

		/* check buffer */
		len = (acl->acl_num * ACL_IR_SIZE) + ACL_DATA_SIZE;
		if (len > buf_len) {
			errno = ENOMEM;
			return -1;
		}

		data = (acl_data_t) buf;
		if (data->acl_hdr.acl_magic != ACL_MAGIC) {
			errno = EINVAL;
			return -1;
		}

		data->acl_hdr.acl_num = acl->acl_num;
		eptr = (acle_t *) (data + 1);
		ent = acl->acl_first;

		/* copy into internal rep structure */
		for (i=0; i < acl->acl_num; i++, eptr++) {

			eptr->acl_tag  = ent->acl_tag;
			eptr->acl_perm = ent->acl_perm;
			eptr->acl_id   = ent->acl_id;

			ent = (acl_entry_t) ent->acl_next;
		}

		return len;
	}
}

/*
 * FUNCTION:
 *	acl_read()
 *
 * ARGUMENTS:
 *	char 		*path;	- the file name.
 *	acl_type_t 	type;	- one of ACL_TYPE_ACCESS or ACL_TYPE_DEFAULT.
 *	acl_t 		acl;	- working space allocated by acl_alloc().
 *
 * DESCRIPTION:
 *	The acl_read() function reads a file's ACL, a directory's ACL,
 * 	or the default ACL associated with a directory.
 * 	The resultant ACL is stored into acl which is a working
 * 	storage area previously allocated by acl_alloc().
 * 	Anything in the working area will be overwritten.
 * 	If additional space is needed to store the ACL it is allocated.
 * 	The type argument is used to indicate that the access ACL or
 * 	the default ACL is returned.
 *
 * RETURNS:
 *	NOERROR - The number of ACL entries.
 *	ERROR   - (-1) and errno set.
 */

int
acl_read (path, type, acl)
char		*path;
acl_type_t	type;
acl_t		acl;
{
	obj_t		obj;
	attr_t		attr;
	u_int		tagnum;
	acl_entry_t	entry_d;
	int		numents,ret;
	ssize_t		buf_len;
	struct stat	sb;
	
	/* check arguments */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
		(type != ACL_TYPE_DEFAULT && type != ACL_TYPE_ACCESS)) {
		errno = EINVAL;
		return -1;
	}

	/* initialize acl policy */
	if (acl_init()) {
		errno = EINVAL;
		return -1;
	}

	/* check for retrieval of default ACL on non-directory type */

	if (stat(path, &sb) < 0)
		return -1;

	if (type == ACL_TYPE_DEFAULT && !S_ISDIR(sb.st_mode)) {
		errno = ENOTDIR;
		return -1;
	}

	/* set ACL tag type to actual or default */
	tagnum = type==ACL_TYPE_ACCESS?PACL_OBJ_ACCESS_TAG:PACL_OBJ_DFLT_TAG;

	/* Allocate space for internal representation. */
	if (check_ir_space (acl->acl_size))
		return -1;

	/* get the acl from the file */
	attr.ir_length = acl_ir_sz * ACL_IR_SIZE;
	attr.ir = (char *) acl_ir;
	obj.o_file = path;

	while (1) {

		ret = getlabel(acl_config.policy,tagnum,&attr,OT_REGULAR,&obj);
		if (ret == -1)
			return -1;

		else {

			acl->acl_counter = 0;

			switch (attr.code) {

			   /*
		 	    * In this release the USER_OBJ, GROUP_OBJ,
			    * and OTHER_OBJ entries are separate from
			    * the file permission bits, so if return
			    * is WILD_CARD make up an acl entry
		 	    * from the file permissions.
		 	    */
			   case SEC_WILDCARD_TAG:

				/*
	 			 * If trying to get default acl
	 			 * it is ok to have wildcard.
	 			 */
				if (type == ACL_TYPE_DEFAULT)
					acl->acl_num = 0;

				else
					if (get_wildcard_acl (acl, path))
						return -1;

				return acl->acl_num;

			   case SEC_ACTUAL_TAG:

				if (attr.ir_length == 0) {
					errno = EIO;
					return -1;
				}

				/*
			 	 * See if we had enough space.
				 * If we did, return the acl.
				 * If we didn't, get some more space.
			 	 */
				numents = attr.ir_length / ACL_IR_SIZE;

				if (acl_ir_sz >= numents) {

					acl->acl_num = numents;
					acl_da->acl_hdr.acl_num = numents;

					/* unpack internal to working storage */
					if (acl_unpack (acl_da,
						ACL_DATA_PACKAGE,acl) < 0)
						return -1;

					return numents;
				}

				if (check_ir_space (numents))
					return -1;

				attr.ir_length = acl_ir_sz * ACL_IR_SIZE;
				attr.ir = (char *) acl_ir;

				continue;

			   default :
				/* should never happen */
				return -1;
			}
		}
	}
	/* not reached */
}

/*
 * FUNCTION:
 *	get_wildcard_acl()
 *
 * ARGUMENTS:
 *	acl_t	acl;    - ACL entry.
 *	char	*path;  - the file name.
 *
 * DESCRIPTION:
 *	The get_wildcard_acl() function sets up a wildcard
 *	acl when there is no tag associated with the object.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		If stat fails.
 *		If cannot get more entries.
 */

int
get_wildcard_acl (acl, path)
acl_t		acl;
char		*path;
{
	acl_entry_t	ent;
	struct stat 	st;

	if (stat (path,&st))
		return -1;

	/* Make sure there is enough room. */
	if (check_wk_space (acl, 3))
		return -1;

	acl->acl_num = 3;
	ent = acl->acl_first;

	ent->acl_head = (void *)acl;
	ent->acl_magic = ACL_MAGIC;
	ent->acl_tag = USER_OBJ;
	ent->acl_uid = st.st_uid;
	ent->acl_perm = (st.st_mode & 0700) >> 6;

	ent = (acl_entry_t) ent->acl_next;

	ent->acl_head = (void *)acl;
	ent->acl_magic = ACL_MAGIC;
	ent->acl_tag = GROUP_OBJ;
	ent->acl_gid = st.st_gid;
	ent->acl_perm = (st.st_mode & 0070) >> 3;

	ent = (acl_entry_t) ent->acl_next;

	ent->acl_head = (void *)acl;
	ent->acl_magic = ACL_MAGIC;
	ent->acl_tag = OTHER_OBJ;
	ent->acl_uid = BOGUS_UID;
	ent->acl_perm = (st.st_mode & 0007);

	return 0;
}

/*
 * FUNCTION:
 *	check_wk_space()
 *
 * ARGUMENTS:
 *	acl_t	acl;       - ACL entry.
 *	int	numneed;   - The number of entries needed.
 *
 * DESCRIPTION:
 *	The check_wk_space() function checks the acl for enough
 *	room to store numneed entries.  If there is not enough,
 *	it allocates some more space.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		ENOMEM - if error getting more memory.
 */

int
check_wk_space (acl, numneed)
acl_t	acl;
int	numneed;
{
	acl_entry_t	newent;
	int		size = INIT_ACL_SIZE;
	acl_entry_t	ent;
	register int	i;

	/* If we already have enough space, return. */
	if (acl->acl_size >= numneed)
		return 0;

	if ((acl->acl_size + numneed) > MAX_IR_ENTRIES) {
		errno = ENOMEM;
		return -1;
	}

	while (size < numneed)
		size += INIT_ACL_SIZE;

	newent = (acl_entry_t) calloc (size, ACL_ENT_SIZE);
	if (newent == (acl_entry_t)NULL) {
		errno = ENOMEM;
		return -1;
	}

	/* Initialize the new space into a linked list. */

	ent = newent;
	for (i=0; i < size; i++, ent++) {

		ent->acl_head = (void *)acl;
		ent->acl_magic = ACL_MAGIC;
		if (i != size - 1)
			ent->acl_next = (void *)(ent + 1);
	}

	/* If this is an initial allocation, set first pointer. */
	if (!acl->acl_first)
		acl->acl_first = newent;

	/* Append the new linked list to the old. */
	else {
		ent = acl->acl_first;

		for (i=0; i < acl->acl_size; i++) {
			if (i != acl->acl_size - 1)
				ent = (acl_entry_t) ent->acl_next;
		}

		ent->acl_next = (void *) newent;
	}

	/* Set the new size */
	acl->acl_size += size;

	return 0;
}

/*
 * FUNCTION:
 *	acl_set_perm()
 *
 * ARGUMENTS:
 *	acl_entry_t	entry_d; - ACL entry.
 *	acl_permset_t	perms;   - ACL permissions;
 *
 * DESCRIPTION:
 *	The acl_set_perm() function shall set the permissions of the
 *	ACL entry indicated by argument entry_d to the permissions contained
 *	in the argument perms.  Any previous permissions shall be replaced.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 *		       - if perms are not valid permission bits.
 */

int
acl_set_perm (entry_d, perms)
acl_entry_t	entry_d;
acl_permset_t	perms;
{

	/* check arguments */
	if (entry_d == (acl_entry_t)0 || entry_d->acl_magic != ACL_MAGIC ||
		perms & ~ACL_PRDWREX) {
		errno = EINVAL;
		return -1;
	}

	/* set permissions */
	entry_d->acl_perm = perms;

	return 0;
}

/*
 * FUNCTION:
 *	acl_set_tag()
 *
 * ARGUMENTS:
 *	acl_entry_t entry_d;	- the ACL entry.
 *	acl_tag_t tag_type;	- place to return the tag.
 *	void	*tag_qualifier;	- the tag qualifier.
 *
 * DESCRIPTION:
 *	The acl_set_tag() function shall set the tag type of an
 *	ACL entry to tag_type with a qualifier in the location
 *	referred to by tag_qualfier.
 *	For the tag types USER_OBJ,GROUP_OBJ,MASK_OBJ, or OTHER_OBJ the
 *	the tag_qualifier may be passed as a NULL.
 *	
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if entry_d does not refer to a valid ACL entry.
 *		       - if tag_type is not a valid tag type;
 */

int
acl_set_tag (entry_d, tag_type, tag_qualifier)
acl_entry_t	entry_d;
acl_tag_t	tag_type;
void		*tag_qualifier;
{

	/* check arguments */
	if (entry_d == (acl_entry_t)NULL || entry_d->acl_magic != ACL_MAGIC) {
		errno = EINVAL;
		return -1;
	}

	/* set tags in entry */
	switch (tag_type) {

		case USER	 :
			entry_d->acl_uid = (uid_t)tag_qualifier;
			break;
		case GROUP	 :
			entry_d->acl_gid = (gid_t)tag_qualifier;
			break;

		case GROUP_OBJ   :
			entry_d->acl_gid = (uid_t)NULL;
			break;

		case USER_OBJ    :
			entry_d->acl_uid = (gid_t)NULL;
			break;

		case MASK_OBJ	 :
		case OTHER_OBJ   :
			break;

		default :
			errno = EINVAL;
			return -1;
	}

	entry_d->acl_tag = tag_type;

	return 0;
}

/*
 * FUNCTION:
 *	acl_valid ()
 *
 * ARGUMENTS:
 *	acl_t		acl	  - working space allocated by acl_alloc()
 *	acl_type_t	type	  - one of ACL_TYPE_ACCESS or ACL_TYPE_DEFAULT
 *	acl_entry_d	*entry_dp - a place to put non-conforming ACL entry.
 *
 * DESCRIPTION:
 *	The acl_valid () function checks the access or default ACL for validity.
 *	- The three required entries: USER_OBJ, GROUP_OBJ,
 *		and OTHER_OBJ shall exist exactly once.
 *	- If the ACL contains four or more entries then the MASK_OBJ
 *		entry shall exist exactly once.
 *	- The ordering may be changed in some implementation defined manner.
 *	- The position of the internal descriptor within the ACL referred to
 *		by acl is undefined.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - the ACL contains entries that are not unique.
 *		       - entry_dp will be set to the failing entry.
 *		EINVAL - acl doesn't refer to an ACL allocated by acl_alloc().
 *		       - there is a missing entry.
 *		       - entry_dp will be set to NULL.
 *			 
 */

int
acl_valid (acl, type, entry_dp)
acl_t		acl;
acl_type_t	type;
acl_entry_t	*entry_dp;
{
	acl_entry_t	ent = (acl_entry_t)NULL;

	/* check args */
	if (acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC ||
		(type != ACL_TYPE_DEFAULT && type != ACL_TYPE_ACCESS))
		goto out;

	/* If empty it must be default type */
	if (acl->acl_num == 0) {
		/* Make sure acl entries pointer is clear */
		acl->acl_first = (acl_entry_t)NULL;
		*entry_dp = (acl_entry_t)NULL;
		if (type != ACL_TYPE_DEFAULT)
			goto out;
		else
			return 0;
	}

	/*
	 * Must be at least USER_OBJ,
	 * GROUP_OBJ, OTHER_OBJ and
	 * not greater than MAX_IR_ENTRIES.
	 */
	if (acl->acl_num < 3 || acl->acl_num > MAX_IR_ENTRIES)
		goto out;

	/* Sort them into evaluating order. */
	if (sort_acl (acl)) {
		errno = ENOMEM;
		*entry_dp = ent;
		return -1;
	}

	/*
	 * Check that all necessary entries are present.
	 * The USER_OBJ, GROUP_OBJ, MASK_OBJ, and OTHER_OBJ
	 * should appear only once.
	 */

	ent = acl->acl_first;

	if (ent->acl_tag != USER_OBJ)
		goto out;

	ent = (acl_entry_t) ent->acl_next;

	if (acl->acl_num > 3) {
		if (ent->acl_tag != MASK_OBJ)
			goto out;
		else
			ent = (acl_entry_t) ent->acl_next;
	}

	while (ent->acl_tag == USER) {
		if (is_qualifier_unique(acl, ent))
			goto out;
		ent = (acl_entry_t) ent->acl_next;
	}

	if (ent->acl_tag != GROUP_OBJ)
		goto out;

	ent = (acl_entry_t) ent->acl_next;

	while (ent->acl_tag == GROUP) {
		if (is_qualifier_unique(acl, ent))
			goto out;
		ent = (acl_entry_t) ent->acl_next;
	}

	if (ent->acl_tag != OTHER_OBJ)
		goto out;

	return 0;

out:
	errno = EINVAL;
	*entry_dp = ent;
	return -1;

}

/*
 * FUNCTION:
 *	sort_acl ()
 *
 * ARGUMENTS:
 *	acl_t		acl;  - acl to be sorted.
 *
 * DESCRIPTION:
 *	The sort_acl () function puts the acl passed into
 *		evaluating order.
 * 	The tag values correspond to the order
 *		in which they are evaluated.
 * 
 *		USER_OBJ entry
 *		MASK_OBJ entry
 *		USER entries
 *		GROUP_OBJ entry
 *		GROUP entries
 *		OTHER entry
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		ENOMEM - If more entries than system can handle.
 */

int
sort_acl (acl)
acl_t	acl;
{
struct sort {
	acl_tag_t	tag;
	acl_entry_t	ent;
};
struct sort buffer [MAX_IR_ENTRIES];
register acl_entry_t	ent;
acl_entry_t		lastptr;
register int		i = 0,j;

	if (acl->acl_num > MAX_IR_ENTRIES)
		return -1;

	/* Load entries into the sort buffer */
	ent = acl->acl_first;
	for (i=0; i < acl->acl_num; i++ ) {

		buffer [i].tag = ent->acl_tag;
		buffer [i].ent = ent;

		ent = (acl_entry_t) ent->acl_next;
	}

	lastptr = ent;

	/* Sort the buffer according to tag number. */
	for (i=0; i < acl->acl_num - 1; i++) {

		struct sort temp;

		for (j = i + 1; j < acl->acl_num; j++) {
			if (buffer[j].tag < buffer[i].tag) {
				temp = buffer[i];
				buffer[i] = buffer[j];
				buffer[j] = temp;
			}
		}
	}

	/* Link the buffer together by setting the next pointers. */
	acl->acl_first = buffer[0].ent;
	for (i=0; i < acl->acl_num - 1; i++)
		buffer[i].ent->acl_next = buffer[ i + 1 ].ent;

	buffer[i].ent->acl_next = lastptr;

	return 0;
}

/*
 * FUNCTION:
 *	is_qualifier_unique()
 *
 * ARGUMENTS:
 *	acl_t acl;	  - working space previously allocated by acl_alloc()
 *	acl_entry_t type; - entry to check.
 *
 * DESCRIPTION:
 *	The is_qualifier_unique() function
 *	checks for duplicate tag values.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If duplicate entry is found.
 */

static int
is_qualifier_unique (acl, ent)
acl_t		acl;
acl_entry_t	ent;
{
register int	i;
acl_entry_t	entp = acl->acl_first;

	for (i=0; i < acl->acl_num; i++) {

		/*
		 * Skip if not the same tag type
		 * or if we are looking at the same entry.
		 */
		if ((entp->acl_tag != ent->acl_tag) || (entp == ent))
			continue;

		switch (ent->acl_tag) {

			case USER :
 				if (entp->acl_uid == ent->acl_uid)
					return -1;
				continue;

			case GROUP :
 				if (entp->acl_gid == ent->acl_gid)
					return -1;
				continue;

		}
		entp = (acl_entry_t) entp->acl_next;
	}

	return 0;
}

/*
 * FUNCTION:
 *	acl_write()
 *
 * ARGUMENTS:
 *	char		*path; - the file name.
 *	acl_type_t	type;  - one of ACL_TYPE_ACCESS or ACL_TYPE_DEFAULT.
 *	acl_t		acl; - working space allocated by acl_alloc().
 *
 * DESCRIPTION:
 *	The acl_write() function:
 *		- associates an access ACL with a file or directory,
 *		- associates a default ACL with a directory,
 *		- or deletes a default ACL from a directory. 
 *
 *	The ACL may have been obtained from a call from acl_read() or
 *	newly created through successive calls to acl_create_entry().
 *
 *	A default ACL may be removed from a directory by writing an
 *	empty default ACL to that directory.
 *
 *	The acl_write() function will succeed only if acl is validated
 *	using acl_valid(), or if acl has no entries and path refers
 *	to a directory.  In all cases, no entries may have the same ACL
 *	entry tag type and ACL entry qualifier specified.  ACL entries that
 *	are implementation defined may define their own restrictions.
 *
 *	After completion the new ACL will be set and the previous ACL
 *	shall no longer be in effect.
 *
 *	No intermediate state may exist where the previous default and the
 *	new ACL, may exist in whole or in part.  This call may result
 *	in changes to the file's permission bits.
 *	
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		EINVAL - if acl does not refer to a valid ACL entry.
 */

int
acl_write (path, type, acl)
char		*path;
acl_type_t	type;
acl_t		acl;
{
	obj_t		obj;
	attr_t		attr;
	u_int		tagnum;
	acl_entry_t	ent;
	ssize_t		buf_len;

	/* check arguments */
	if (!path || acl == (acl_t)NULL || acl->acl_magic != ACL_MAGIC) {
		errno = EINVAL;
		return -1;
	}

	/* validate the acl */
	if (acl_valid (acl,type,&ent))
		return -1;

	/* Initialize acl_config struct */
	if (acl_init()) {
		errno = EINVAL;
		return -1;
	}

	/* set ACL tag type to actual or default */
	tagnum = type==ACL_TYPE_ACCESS?PACL_OBJ_ACCESS_TAG:PACL_OBJ_DFLT_TAG;

	/*
	 * If empty set the entry to wildcard.  It has already
	 * been verified by acl_valid(), so no check is needed here.
	 */
	if (acl->acl_num == 0) {
		if (type == ACL_TYPE_DEFAULT) {
			attr.code = SEC_WILDCARD_TAG;
			attr.ir = (char *)0;
			attr.ir_length = 0;
		}
		else {
			errno = EINVAL;
			return -1;
		}
	}
	else {

		/* Convert from working storage to internal representation. */
		
		if (check_ir_space (acl->acl_size))
			return -1;

		buf_len = (acl_ir_sz * ACL_IR_SIZE) + ACL_DATA_SIZE;

		if (acl_pack (acl,(char *)acl_da, buf_len,ACL_DATA_PACKAGE) < 0)
			return -1;

		attr.code = SEC_ACTUAL_TAG;
		attr.ir = (char *) (acl_da + 1);
		attr.ir_length = acl->acl_num * ACL_IR_SIZE;
	}

	obj.o_file = path;

	if (setlabel(acl_config.policy, tagnum, &attr, OT_REGULAR, &obj))
		return -1;

	return 0;
}

/************************************************************************/
/*	Below are SecureWare functions and are not part of the standard.*/
/************************************************************************/


static int clear_white_space ();
static int set_perm ();
static int get_id ();
static int set_entry ();
static int set_user_entry ();
static int set_group_entry ();
static int getptoken ();
static int scan_literal ();
static int pacl_1ir_to_er ();


struct lexstate {
	char		*string;	/* string being parsed */
	char		*pos;		/* current position in string */
	char		strval[32];	/* value of STRING token */
};


/*
 * FUNCTION:
 *	acl_init()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The acl_init() function reads the ACL_CONFIG_FILE and
 *	initializes the acl_config structure.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		If the call to pacl_init_ioctl fails.
 *		If the parameter file cannot be opened.
 *		If the parameter file cannot be scanned.
 */

int
acl_init()
{
FILE	*fp;
int	ret, fields;
char	buffer[100];
long	minor_device;

	/* only do this once */
	if (paclinit)
		return 0;

	/*
	 * Retrieve values from kernel
	 */

	if (pacl_init_ioctl())
		return -1;

	/* read in the parameters file */
	fp = fopen(PACL_PARAM_FILE, "r");
	if (!fp)
		return -1;

	/* get parameter line */
	do {
		if (fgets(buffer, sizeof buffer, fp) == NULL) {
			fclose(fp);
			return -1;
		}
	} while (buffer[0] == '\n' || buffer[0] == '#');

	/* parse it into fields */
	fields = sscanf(buffer, "%s %ld %d",
			acl_config.dbase, &acl_config.cache_size,
			&acl_config.buffers);

	if (fields != 3) {	/* format error */
		fprintf(stderr, MSGSTR(ACLLIB_1, "Error reading ACL configuration file\n"));
		ret = -1;
	} else			/* success */
		ret = 0;

	fclose(fp);

	paclinit = 1;
	return ret;
}


/*
 * FUNCTION:
 *	pacl_init_ioctl()
 *
 * ARGUMENTS:
 *	none.
 *
 * DESCRIPTION:
 *	The pacl_init_ioctl() function opens the
 *	policy control device and makes the call
 *	to ioctl to initialize the sp_init structure.
 *	It then uses the values returned to update
 *	the acl_config structure.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		If failure to open control device.
 *		If ioctl call fails.
 */

static int
pacl_init_ioctl()
{
	int	pacl_device;
	int	result;

	/* open the policy control device */

	if ((pacl_device = open(SPD_CONTROL_DEVICE, O_RDWR)) == -1) {
		perror(MSGSTR(ACLLIB_2, "Error opening policy control device"));
		return -1;
	}

	/*
	 * Initialize the magic number to 
	 * POSIX Access Control Lists.
	 */
	sp_init.magic = SEC_PACL_MAGIC;

	/* perform the ioctl to get parms */

	result = ioctl(pacl_device, SPIOC_GETCONF, &sp_init);
	close(pacl_device);
	if (result == -1) {
		return -1;
	}

	acl_config.subj_tags =      sp_init.subj_tag_count;
	acl_config.obj_tags =       sp_init.obj_tag_count;
	acl_config.first_subj_tag = sp_init.first_subj_tag;
	acl_config.first_obj_tag =  sp_init.first_obj_tag;
	acl_config.minor_device =   sp_init.spminor;
	acl_config.policy =         sp_init.policy;

	return 0;
}

/*
 * FUNCTION:
 *	acldaemon_init()
 *
 * ARGUMENTS:
	struct	sp_init	*sp; - place to put the policy info.
 *
 * DESCRIPTION:
 *	The acldaemon_init() function is called from
 *	from the daemon to initialize the internal
 *	acl_config structure and to retrieve the
 *	policy configuration parameters.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If initialization fails.
 */

int
acldaemon_init(sp)
	struct sp_init	*sp;
{
	if (acl_init() != 0)
		return -1;

	*sp = sp_init;

	return 0;
}

/*
 * FUNCTION:
 *	acl_ir_to_er()
 *
 * ARGUMENTS:
 *	acle_t	*ent;     - ACL entry.
 *	int	numents; - the number of entries.
 *
 * DESCRIPTION:
 *	The acl_ir_to_er() function converts an internal representation
 *	to an external character string representation.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		- If not enough internal space.
 *	        - If the conversion fails.
 */

char *
acl_ir_to_er (ent, numents)
register acle_t	*ent;	 /* pointer to ACL entries */
register int	numents; /* number of ACL entries */
{
	register int cc = 0;
	register int i,ret;

	/* If buffer already contains data clear it. */
	if (acl_er)
		*acl_er = '\0';

	/* Allocate an er buffer on first entry */
	if (check_er_space (INIT_BUFSIZE))
		return (char *)NULL;

	for (i=0; i < numents; i++, ent++) {

		/*
		 * Expand er buffer if necessary
		 * 35 character limit is broken down as follows:
		 *	\n tag : id : perm \n \t #effective \n \0
		 *	 1  5  1 10 1  3    1  1     10      1  1 = 35
		 *
		 * NOTES:
		 *	A carriage return will be appended to
		 *		the head of the first group
		 *		and other entry.
		 *	The largest tag is other which is five chars.
		 *	The largest id will be -1 as a string (4294967295).
		 */

		if (check_er_space (cc + MAX_ENTRY_SIZE))
			return (char *)NULL;

		if ((ret = pacl_1ir_to_er (ent)) == -1)
			return (char *)NULL;

		cc += ret;
	}

	return acl_er;
}

/*
 * FUNCTION:
 *	pacl_1r_to_er()
 *
 * ARGUMENTS:
 *	acle_t	*ent; - ACL entry.
 *
 * DESCRIPTION:
 *	The pacl_1r_to_er() function converts one entry into a
 *	string representation.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1) and errno set.
 *		- If MAX_ENTRY_SIZE has been reached.
 *	        - If entry is not correctly formatted.
 */

int
pacl_1ir_to_er (ent)
acle_t	*ent;
{
	static acl_permset_t	mask;
	static int	gotmask, gotgroup;
	uid_t		uid;
	gid_t		gid;
	char		perm [4], mask_perm [4], effperm [4];
	char		mybuf [INIT_BUFSIZE], *name;
	int		size = 0;


	switch (ent->acl_tag) {

	   case USER_OBJ :

		gotmask = 0; gotgroup = 0;
		get_perm (ent,perm,0,0,effperm);
		sprintf (mybuf,"user::%s\n",perm);
		break;

	   case MASK_OBJ :

		gotmask++;
		mask = ent->acl_perm;
		get_perm (ent,mask_perm,0,0,effperm);
		sprintf (mybuf,"mask::%s\n",mask_perm);
		break;

	   case USER :

		if (get_name(ent,&name)) {
			if (get_perm (ent,perm,mask,gotmask,effperm) == 1)

				sprintf (mybuf,"user:%d:%s\t#effective:%s\n",
					ent->acl_uid,perm,effperm);
			else
				sprintf (mybuf,"user:%d:%s\n",
					ent->acl_uid,perm);
		}
		else {
			if (get_perm (ent,perm,mask,gotmask,effperm) == 1)

				sprintf (mybuf,"user:%s:%s\t#effective:%s\n",
					name,perm,effperm);
			else
				sprintf (mybuf,"user:%s:%s\n", name,perm);

		}
		break;

	   case GROUP_OBJ :

		if (!gotgroup) {
			strcat (acl_er,"\n");
			size++;
		}
		gotgroup++;

		if (get_perm (ent,perm,mask,gotmask,effperm) == 1)
			sprintf (mybuf,"group::%s\t#effective:%s\n",
				perm, effperm);
		else
			sprintf (mybuf,"group::%s\n",perm);

		break;

	   case GROUP :

		if (!gotgroup) {
			strcat (acl_er,"\n");
			size++;
		}
		gotgroup++;

		if (get_name(ent,&name)) {
			if (get_perm (ent,perm,mask,gotmask,effperm) == 1)

				sprintf (mybuf,"group:%d:%s\t#effective:%s\n",
					ent->acl_gid,perm,effperm);
			else
				sprintf (mybuf,"group:%d:%s\n",
					ent->acl_gid,perm);
		}
		else {
			if (get_perm (ent,perm,mask,gotmask,effperm) == 1)

				sprintf (mybuf,"group:%s:%s\t#effective:%s\n",
					name,perm,effperm);
			else
				sprintf (mybuf,"group:%s:%s\n", name,perm);
		}

		break;

	   case OTHER_OBJ :

		strcat (acl_er,"\n");

		if (get_perm (ent,perm,mask,0,effperm) == 1)
			sprintf (mybuf, "other::%s\t#effective:%s\n",
				perm,effperm);
		else
			sprintf (mybuf, "other::%s\n",perm);

		break;

	   default :

		errno = EINVAL;
		return -1;
	}

	size += strlen (mybuf);

	/* Make sure entry was within specs. */
	if ((size + 1) > MAX_ENTRY_SIZE) {
		errno = EINVAL;
		return -1;
	}

	strcat (acl_er,mybuf);

	return size;
}


/*
 * FUNCTION:
 *	get_perm()
 *
 * ARGUMENTS:
 *	acle_t 		*ent;	  - pointer to acl entry.
 *	char		*retperm; - place to put result.
 *	acl_permset_t 	mask;	  - the permissions of MASK_OBJ.
 *	int		gotmask;  - flag indicating receipt of mask.
 *	char		*effperm; - the resultant effective permissions.
 *
 * DESCRIPTION:
 *	The get_perm() function returns the permissions string
 *	corresponding to the acl_perm member of the acl entry.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If the id cannot be mapped to a user or group name.
 */

static int
get_perm (ent, retperm, mask, gotmask, effperm)
acle_t		*ent;
char		*retperm;
acl_permset_t	mask;
int		gotmask;
char		*effperm;
{ 
	acl_permset_t	perm = ent->acl_perm;

	/*
	 * Convert the hex representation
	 * of the permissions to a
	 * string representation.
	 */
	strcpy (retperm,"---");

	if (perm & ACL_PREAD)
		retperm[0] = 'r';
	if (perm & ACL_PWRITE)
		retperm[1] = 'w';
	if (perm & ACL_PEXECUTE)
		retperm[2] = 'x';
	
	/*
	 * We don't need to flag this entry for #effective
	 * string inclusion if we haven't got a mask entry
	 * or this is a USER_OBJ or OTHER_OBJ entry.
	 */
	if(!gotmask || (ent->acl_tag == USER_OBJ)||(ent->acl_tag == OTHER_OBJ))
		return 0;

	/*
	 * If the permissions of this entry are more permissive
	 * than the mask's permissions, flag this entry to
	 * include the #effective string.
	 */
	if ((perm & mask) != perm) {

		strcpy (effperm,"---");

		if ((perm & mask) & ACL_PREAD)
			effperm[0] = 'r';
		if ((perm & mask) & ACL_PWRITE)
			effperm[1] = 'w';
		if ((perm & mask) & ACL_PEXECUTE)
			effperm[2] = 'x';

		return 1;
	}

	return 0;
}

/*
 * FUNCTION:
 *      get_name()
 *
 * ARGUMENTS:
 *      acle_t	*ent;   - pointer to acl entry.
 *      char	**name; - place to return name.
 *
 * DESCRIPTION:
 *      The get_name() function returns a user or group name
 *      associated with the qualifier member of the acl entry.
 *
 * RETURNS:
 *      NOERROR - (0).
 *      ERROR   - (-1).
 *              If the id cannot be mapped to a user or group name.
 */

static int
get_name (ent, name)
acle_t	*ent;
char	**name;
{

        if (ent->acl_tag == USER)
                *name = (char *)pw_idtoname (ent->acl_uid);
        else
                *name = (char *)gr_idtoname (ent->acl_gid);
        if (!*name)
                return -1;
        return 0;
}

/*
 * FUNCTION:
 *	check_er_space ()
 *
 * ARGUMENTS:
 *	int	need; - the amount of space needed.
 *
 * DESCRIPTION:
 *	The check_er_space() function checks for 
 *	sufficient room in the ACL buffer.  If not
 *	enough space, it will reallocate.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If memory allocation fails.
 */

static int
check_er_space (need)
int	need;
{
	register char	*new_er;

	/*
	 * If the need is more than
	 * the current size, get some more.
	 */
	if (need > acl_ersz) {

		if (acl_ersz == 0)
			new_er = (char *)malloc(INIT_BUFSIZE);
		else
			new_er = (char *)realloc((char *)acl_er,
				(acl_ersz + INIT_BUFSIZE));

		if (new_er == NULL)
			return -1;

		acl_ersz += INIT_BUFSIZE;
		acl_er = new_er;
	}

	return 0;
}

/*
 * FUNCTION:
 *	acl_er_to_ir ()
 *
 * ARGUMENTS:
 *	char	*string; - the string to convert.
 *	int	*size;	 - the number of acl entries.
 *
 * DESCRIPTION:
 *	The acl_er_to_ir() function converts an ACL
 *	external representation to internal representation.
 *
 *	The space allocated will have to be freed by the
 *	calling party.
 *
 * RETURNS:
 *	NOERROR - pointer to the acl.
 *	ERROR   - NULL.
 *		If memory allocation fails.
 *		If the parse fails.
 */

acle_t *
acl_er_to_ir (string, size)
char	*string;
int	*size;
{
	struct lexstate	ls;
	acle_t		*ep;
	register int	count = 0;
	int		ret,num,len;

	if (!(len = clear_white_space (string)))
		return (acle_t *)NULL;

	num = (len / ACL_IR_SIZE) + 1;

	/* allocate an ir buffer on first call */
	if (check_ir_space(num))
		return (acle_t *)NULL;

	/* reset number in global acl */
	acl_ir_num = 0;

	/* set acl entry pointer to allocated space */
	ep = acl_ir;

	ls.string = ls.pos = string;

	while (ret = getptoken(&ls)) {

		if (ret == -1)
			return (acle_t *)NULL;

		if (check_ir_space(acl_ir_num + 1))
			return (acle_t *)NULL;

		if (scan_literal(&ls,ep))
			return (acle_t *)NULL;

		ep++; acl_ir_num++;

	}

	*size = acl_ir_num;

	return acl_ir;
}

/*
 * FUNCTION:
 *	check_ir_space ()
 *
 * ARGUMENTS:
 *	int	need; - the number of acl entries.
 *
 * DESCRIPTION:
 *	The check_ir_space() function checks for 
 *	sufficient room in the ACL buffer.  If not
 *	enough space, it will reallocate.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If memory allocation fails.
 */

static int
check_ir_space (need)
int	need;
{
	acl_data_t	new_da;
	int		size = INIT_ACL_SIZE,siz;

	/*
	 * If the need is more than
	 * the current size, get some more.
	 */
	if (need > acl_ir_sz) {

		while (size < need)
			size += INIT_ACL_SIZE;

		if (acl_ir_sz == 0) {
		   siz = (size * ACL_IR_SIZE) + ACL_DATA_SIZE;
		   new_da = (acl_data_t)malloc(siz);
		   new_da->acl_hdr.acl_num = 0;
		}
		else {
		   siz = ((acl_ir_sz + size) * ACL_IR_SIZE) + ACL_DATA_SIZE;
		   new_da = (acl_data_t)realloc((char *)acl_da,siz);
		}

		if (new_da == NULL) {
			errno = ENOMEM;
			return -1;
		}

		acl_ir_sz += size;
		acl_da = new_da;
		new_da->acl_hdr.acl_magic = ACL_MAGIC;
		acl_ir = (acle_t *) (acl_da + 1);
	}

	return 0;
}

/*
 * FUNCTION:
 *	getptoken ()
 *
 * ARGUMENTS:
 *	register struct lexstate *ls; - pointer to
 *			structure holding lex information.
 *
 * DESCRIPTION:
 *	The getptoken() function scans the file buffer
 *	for tokens of the type user, mask, group, other.
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If memory allocation fails.
 */

static int
getptoken (ls)
register struct lexstate	*ls;
{
	register char		*cp = ls->pos;
	register unsigned	num, r;

	/* end of string */
	if (*cp == '\0') {
		ls->pos = cp;
		return 0;
	}

	/*
	 * Parse the buffer.
	 * Entries should look like:
	 *
	 *   type:qualifier:perms\n
	 *
 	 *	user::rwx\n
 	 *	mask::r-x\n
 	 *	user:adm:r-x\n
 	 *	user:tomg:rwx	#effective:r-x\n
 	 *
 	 *	group::r--\n
 	 *	group:daemon:r-x\n
 	 *
 	 *	other::r--\n
	 */
	
	/* get past type   user: */
	while (cp && *cp && (*cp != ':')) cp++;

	/* get past colon */
	if (cp && *cp) cp++;

	/* get past the qualifier */
	if (cp && *cp && *cp != ':')
		while (cp && *cp && (*cp != ':')) cp++;

	/* get past colon */
	if (cp && *cp) cp++;

	/* get past perms, perms might end in '#' */
	while (cp && *cp && (*cp != '\n') && 
		(*cp != ',') && (*cp != '#')) cp++;

	/* subtract out the '\n' ',' or '#' */
	num = cp - ls->pos;
	if (num >= sizeof ls->strval)
		num = sizeof ls->strval - 1;

	/* copy result into new buffer */
	strncpy(ls->strval, ls->pos, num);

	/* terminate new buffer */
	ls->strval[num] = '\0';

	/*
	 * If the current pointer isn't a '\n',
	 * increment to the end of the line to
	 * shore up for the next time in.
	 * This is necessary when the
	 * #effective:rwx comment is present.
	 */

	if (cp && *cp == '#') {
		while (cp && (*cp != '\n') && (*cp != ',')) cp++;
		if (cp) cp++;
	}
	else if (cp && *cp) cp++;

	/* skip past any stray '\n' or ',' and set current position */
	while (cp && *cp && ((*cp == '\n') || (*cp == ','))) cp++;

	/* set current position */
	ls->pos = cp;

	return 1;
}


/*
 * FUNCTION:
 *	scan_literal ()
 *
 * ARGUMENTS:
 *	register struct lexstate *ls; - pointer to
 *			structure holding lex information.
 *	register acle_t	*ep; - pointer to acl entry;
 *
 * DESCRIPTION:
 *	The scan_literal() function scans the acl entry
 *	string and converts it to an acl entry structure. 
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If parse of string fails.
 */

static int
scan_literal (ls, ep)
register struct lexstate	*ls;
register acle_t			*ep;
{
register int	err = 0;
char		*literal;
	
	/* Interpret the acl string */

	if ((strncmp (ls->strval, "user:", 5) == 0) ||
	    (strncmp (ls->strval, "u:", 2) == 0)) {

		/* get past the ':' */
		literal = (char *)strchr (ls->strval,':') + 1;

		/* get the user entry from the string */
		if (set_user_entry (literal,ep))
			err++;
	}

	else if ((strncmp (ls->strval, "mask:", 5) == 0) ||
	    (strncmp (ls->strval, "m:", 2) == 0)) {

		/* get past the ':' */
		literal = (char *)strchr (ls->strval,':') + 1;

		/* get the mask entry from the string */
		if (set_entry (literal,ep,MASK_OBJ))
			err++;
	}

	else if ((strncmp (ls->strval, "group:", 6) == 0) ||
	    (strncmp (ls->strval,"g:",2) == 0)) {

		/* get past the ':' */
		literal = (char *)strchr (ls->strval,':') + 1;

		/* get the group entry from the string */
		if (set_group_entry (literal,ep))
			err++;
	}

	else if ((strncmp (ls->strval, "other:", 6) == 0) ||
	    (strncmp (ls->strval,"o:",2) == 0)) {

		/* get past the ':' */
		literal = (char *)strchr (ls->strval,':') + 1;

		/* get the other entry from the string */
		if (set_entry (literal,ep,OTHER_OBJ))
			err++;
	}

	else
		err++;

	if (err) {
		errno = EINVAL;
		return -1;
	}

	return 0;

}


/*
 * FUNCTION:
 *	set_user_entry ()
 *
 * ARGUMENTS:
 *	char	*cp; - the string to scan.
 *	acle_t	*ep; - the acl entry to return.
 *
 * DESCRIPTION:
 *	The set_user_entry() parses the literal string
 *	into an acl_entry structure.
 *
 *	The string is passed as a USER entry:
 *		:qualifier:rwx
 *
 *	or as a USER_OBJ entry:
 *		::rwx
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If parse of string fails.
 */


static int
set_user_entry (cp, ep)
register char	*cp;
register acle_t	*ep;
{
	uid_t	id;
	char	*p;	/* pointer to permissions */

	/*
	 * If next char is ':' then the qualifier
	 * field is NULL.  We have a USER_OBJ entry.
	 * 	:rwx
	 */
	if (*cp == ':') {
		ep->acl_tag = USER_OBJ;
		ep->acl_uid = (uid_t)NULL;

		/* set pointer to permissions */
		p = ++cp;
	}

	/*
	 * We have a USER entry.
	 * Translate the qualifier.
	 *	qualifier:rwx
	 */
	else {

		/*
		 * Save pointer to beginning of
		 * qualifier.  Increment to next ':'
		 */
		p = cp;
		while (p && *p && *p != ':') p++;

		/*
		 * If we get to end before we get to
		 * a ':' then syntax error.
		 */
		if (*p == ':')
			*p = '\0';
		else
			return -1;
		/*
		 * Increment past ':'.
		 */
		p++;

		/*
		 * If not an id, get id from name,
		 * else, check if id is valid.
		 */
		if (get_id (cp, &id) == 0) {

			if ((id = (uid_t)pw_nametoid(cp)) == (uid_t)-1)
				return -1;
		}

		/* Set information in acl entry */

		ep->acl_tag = USER;
		ep->acl_uid = id;
	}

	if (set_perm (p,ep))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	set_group_entry ()
 *
 * ARGUMENTS:
 *	char	*cp; - the string to scan.
 *	acle_t	*ep;  - the acl entry to return.
 *
 * DESCRIPTION:
 *	The get_pacl_entry() parses the literal string
 *	into an acl_entry structure.
 *
 *	The string is passed as a GROUP entry:
 *		:qualifier:rwx
 *
 *	or as a GROUP_OBJ entry:
 *		::rwx
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If parse of string fails.
 */

static int
set_group_entry (cp, ep)
register char	*cp;
register acle_t	*ep;
{
	gid_t	id;
	char	*p;	/* pointer to permissions */

	/*
	 * If next char is ':' then the qualifier
	 * field is NULL.  We have a GROUP_OBJ entry.
	 * 	:rwx
	 */
	if (*cp == ':') {
		ep->acl_tag = GROUP_OBJ;
		ep->acl_gid = (gid_t)NULL;

		/* set pointer to permissions */
		p = ++cp;
	}

	/*
	 * We have a GROUP entry.
	 * Translate the qualifier.
	 *	qualifier:rwx
	 */
	else {

		/*
		 * Save pointer to beginning of
		 * qualifier.  Increment to next ':'
		 */
		p = cp;
		while (*p && *p != ':') p++;

		/*
		 * If we get to end before we get to
		 * a ':' then syntax error.
		 */
		if (*p == ':')
			*p = '\0';
		else
			return -1;

		/* increment past ':' */
		p++;

		/*
		 * If not an id, get id from name,
		 * else, check if id is valid.
		 */
		if (get_id (cp, &id) == 0) {

			if ((id = (gid_t)gr_nametoid(cp)) == (gid_t)-1)
				return -1;
		}

		/* Set information in acl entry */

		ep->acl_tag = GROUP;
		ep->acl_gid = id;
	}

	if (set_perm (p,ep))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	set_entry ()
 *
 * ARGUMENTS:
 *	register char	*cp; - the string to scan.
 *	register acle_t	*ep; - the acl entry to return.
 *	acl_tag_t	tag; - the acl_tag type;
 *
 * DESCRIPTION:
 *	The set_entry() parses the literal string
 *	into an acl_entry structure.
 *
 *	The string is passed as:
 *		::rwx
 *
 * RETURNS:
 *	NOERROR - (0).
 *	ERROR   - (-1).
 *		If parse of string fails.
 */

static int
set_entry (cp, ep, tag)
register char	*cp;
register acle_t	*ep;
acl_tag_t	tag;
{

	/*
	 * We should be at the qualifier entry.
	 * If we don't have a ':' ,
	 * we have a syntax error.
	 * 	::rwx
	 */
	if (*cp != ':')
		return -1;

	/* Increment past the ':' */
	cp++;

	ep->acl_tag = tag;
	ep->acl_uid = (uid_t)NULL;

	if (set_perm (cp,ep))
		return -1;

	return 0;
}

/*
 * FUNCTION:
 *	get_id ()
 *
 * ARGUMENTS:
 *	char	*cp; - the id string.
 *	uid_t	*id; - the id integer returned.
 *
 * DESCRIPTION:
 *	The get_id() function interprets the
 *	qualifier field.  If it is a number,
 *	return it as a uid_t.  If it is a character
 *	string, return 0.
 *
 * RETURNS:
 *	 0 - If the qualifier is a string.
 *	 1 - If the qualifier is a number.
 */

static int
get_id (cp,id)
char	*cp;
uid_t	*id;
{
	register uid_t 		num;
	register unsigned	r;

	switch (*cp) {

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':

			num = *cp++ - '0';
			/*
		 	* Interpret numbers with leading 0 or 0x
		 	* according to C conventions.
		 	*/
			if (num == 0)
				if (*cp == 'x' || *cp == 'X') {
					r = 16;
					++cp;
				} else
					r = 8;
			else
				r = 10;
			while (isdigit(*cp) || r == 16 && isxdigit(*cp)) {
				num *= r;
				num += *cp - (isdigit(*cp)?'0':(islower(*cp)?'a':'A'));
				++cp;
			}
			*id = num;

			break;

		default :
			return 0;
	}

	return 1;
}

/*
 * FUNCTION:
 *	set_perm ()
 *
 * ARGUMENTS:
 *	char	*cp; - the id string.
 *	acle_t	*ep;  - the acl entry.
 *
 * DESCRIPTION:
 *	The set_perm() function parses the
 *	permissions field. 
 *
 *	- If string is an absolute value, then it shall contain
 *		exactly one of each of the following characters
 *		in the following order: r, w, and x.  Or the
 *		place holder: '-' to indicate no access.
 *
 *	- If the string is a relative value, then it shall be proceded
 *		by a '+' to indicate additional access or a '^' to 
 *		indicate that access is to be removed.  The relative
 *		string shall be at least one character and at most
 *		three characters.  The symbolic string shall contain
 *		at most one of each of the following characters in
 *		and order: r, w, x.
 *
 *	- The access permissions may also be presented in octal.  If
 *		the octal representations are used, then exactly one
 *		octal digit is required.
 *
 * RETURNS:
 *	 0 - If ok.
 *	 -1 - If error.
 */

static int
set_perm (cp,ep)
register char	*cp;
register acle_t	*ep;
{
	register int		i;
	register acl_permset_t	perm = 0, noperm = 0;
	int			orep;		/* octal representation */

	if (cp[0] == '\0')
		return -1;

	if (cp[0] != '^' && cp[0] != '+') {

		/*
		 * Check for octal value.
		 */
		if (isdigit (cp[0])) {

			/* If digit, only one allowed */
			if (cp[1])
				return -1;

			orep = atoi (cp);
			if (orep > 7 || orep < 0)
				return -1;

			perm = orep;
		}

		/*
		 * Must be some combination of
		 * rwx or with '-' as a format
		 * character.
		 */
		else {

			/*
			 * Mark all format characters first to
			 * make sure user knows what he is flagging.
			 */
			if (strchr(cp,'-') && strlen (cp) != 3)
				return -1;

			for (i=0; (i < 3 && cp[i]); i++) {
				if (cp[i] == '-') {
					if (i == 0)
						noperm |= ACL_PREAD;
					if (i == 1)
						noperm |= ACL_PWRITE;
					if (i == 2)
						noperm |= ACL_PEXECUTE;
				}
			}

  			/* Maximum 3 characters.  */
			if (cp[i])
				return -1;

			for (i=0; (i < 3 && cp[i]); i++) {

				if (cp[i] == 'r') {
					if (perm & ACL_PREAD ||
						noperm & ACL_PREAD)
						return -1;
					else
						perm |= ACL_PREAD;
				}

				else if (cp[i] == 'w') {
					if (perm & ACL_PWRITE ||
						noperm & ACL_PWRITE)
						return -1;
					else
						perm |= ACL_PWRITE;
				}

				else if (cp[i] == 'x') {
					if (perm & ACL_PEXECUTE ||
						noperm & ACL_PEXECUTE)
						return -1;
					else
						perm |= ACL_PEXECUTE;
				}

				else if (cp[i] == '-')
					;

				else
					return -1;
			}
		}
	}

	else {
		/*
		 * At most '^' or '+' plus three characters
		 * or at least '^' or '+' plus one character.
		 */
		if (strlen (cp) > 4 || strlen (cp) < 2)
			return -1;

		if (cp [0] == '+')
			perm |= ACL_PADD;

		if (cp [0] == '^')
			perm |= ACL_PREMOVE;

		cp++;

		/* Set value if octal representation */
		if (isdigit (cp[0])) {

			/* If digit, only one allowed */
			if (cp[1])
				return -1;

			orep = atoi (cp);
			if (orep > 7 || orep < 0)
				return -1;

			perm |= orep;
		}

		else {
			for (i=0; cp[i]; i++) {

				switch (cp[i]) {

					case 'r' :  perm |= ACL_PREAD;
							break;
					case 'w' :  perm |= ACL_PWRITE;
							break;
					case 'x' :  perm |= ACL_PEXECUTE;
							break;
					default  :  return -1;
				}
			}
		}
	}

	/* set permissions */
	ep->acl_perm = perm;

	return 0;
}

/*
 * FUNCTION:
 *	clear_white_space ()
 *
 * ARGUMENTS:
 *	char	*p; - the buffer string.
 *
 * DESCRIPTION:
 *	The clear_white_space() function increments
 *	through the buffer and clears out all the
 *	white space (spaces, tabs, etc.).
 *	It will not clear the '\n' because they are
 *	needed to parse the buffer.
 *
 * RETURNS:
 *	 0 - always
 */

static int
clear_white_space (p)
char	*p;
{
register int	i = 0,j;

	while (p[i]) {

		while (p[i] && isspace (p[i])) {

			/* we need to keep '\n' */
			if (p[i] == '\n')
				break;
			j = i;
			while (p[j]) {
				p [j] = p [j + 1];
				j++;
			}
		}
		i++;
	}

	return i;
}

#ifndef SEC_STANDALONE /*{*/
/*
 * This routine is given an IR and entry count for an ACL and fills in
 * the tag for that IR.  It also returns 1 to note that the operation
 * succeeded or 0 to signify that the operation failed and the tag
 * is left unchanged.
 */

int
acl_ir_to_tag(ir, count, tag)
	acle_t *ir;
	int count;
	tag_t *tag;
{
	register int spdfd;
	register int ret;
	register int msg_size;
	register struct spd_map_tag *query;
	int got_tag = 0;
	int acl_ir_size;
	struct spd_set_tag response;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + 32 + 2];

	if (acl_init() != 0)
		return got_tag;

	acl_ir_size = count * sizeof(acle_t);
	msg_size = sizeof(*query) + acl_ir_size;
	query = (struct spd_map_tag *) calloc(msg_size, 1);
	if (query != (struct spd_map_tag *) 0)  {
	    sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		    acl_config.minor_device | CLIENT);
	    spdfd = open(spd_dev, O_RDWR);

	    if (spdfd >= 0)  {
		query->mhdr.msg_type = SPD_MAP_TAG;
		query->ir.ir_length = acl_ir_size;
		memcpy(query + 1, ir, acl_ir_size);
		ret = write(spdfd, query, msg_size);

		if (ret == msg_size)  {
			msg_size = sizeof(response);
			response.mhdr.msg_type = SPD_SET_TAG;
			ret = read(spdfd, &response, msg_size);

			if (ret == msg_size)  {
				*tag = response.tag;
				got_tag = 1;
			}
		}
		(void) close(spdfd);
	    }
	    free(query);
	}

	return got_tag;
}
#endif /*} SEC_STANDALONE */

#ifndef SEC_STANDALONE /*{*/
/*
 * This routine is given a tag for an ACL and fills in the IR for the ACL.
 * A return value >= 0 indicates the number of entries in the ACL.
 * A return of -1 indicates an error.
 */

int
acl_tag_to_ir(tag, ir)
	tag_t tag;
	char *ir;
{
	register int spdfd;
	register int ret;
	register int acl_size, acl_entries;
	register struct spd_internal_rep *response;
	struct spd_get_attribute query;
	char spd_dev[sizeof(SP_DAEMON_DEVICE) + 32];

	acl_init();

	response = (struct spd_internal_rep *) calloc(1024, 1);
	if (response == (struct spd_internal_rep *) 0)
		return -1;

	sprintf(spd_dev, "%s%d", SP_DAEMON_DEVICE,
		acl_config.minor_device | 1);
	spdfd = open(spd_dev, O_RDWR);
	if (spdfd < 0) {
		free(response);
		return -1;
	}

	query.mhdr.msg_type = SPD_GET_ATTRIBUTE;
	query.tag = tag;
	ret = write(spdfd, &query, sizeof query);

	if (ret == sizeof query) {
		response->mhdr.msg_type = SPD_INTERNAL_REP;
		response->ir.ir_length = 1024 - SPD_INTERNAL_REP_SIZE;
		ret = read(spdfd, response, 1024);

		/* If the internal rep size is 0 it is a Null ACL */

		if(response->mhdr.error_code != 0)
			acl_entries = -1;
		else if (response->ir.ir_length == 0)
			acl_entries = 0;
		else if (ret <= 1024)  {
			memcpy(ir, response + 1, response->ir.ir_length);
			acl_entries = response->ir.ir_length / sizeof(acle_t);
		}
	} else
		acl_entries = -1;

	(void) close(spdfd);
	free(response);

	return(acl_entries);
}
#endif /*} SEC_STANDALONE */

#endif /*} SEC_ACL_POSIX */
