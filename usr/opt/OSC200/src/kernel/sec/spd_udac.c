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
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved.
 *
 * UNIX discretionary access control module.
 *
 * This module includes code that emulates the UNIX discretionary
 * policy model when there is no other discretionary access control
 * policy.  It also includes the function that implements the UNIX
 * discretionary access check.
 * The former is protected with !SEC_ACL.  The UNIX discretionary
 * check may be called from any of the discretionary access control
 * policies.
 */

/* #ident "@(#)spd_udac.c	4.1 21:45:10 7/9/90 SecureWare"*/

#include <sec/include_sec>

#if SEC_ARCH

#if !SEC_ACL

/* If no ACL policy defined, most entry points are no-ops */

udacinit(policy)
	int policy;
{
	return 1;
}

/* 
 * POSIX ACLS -- new arguments to function
 *               and return codes
 */
udaccreate(process, object, parent, attrtype, dac, umask)
	tag_t *process, *object, *parent;
	attrtype_t attrtype;
        dac_t *dac;
        mode_t umask;
{
       /* umask and mode are sent separately */

        dac->mode &= ~umask;
        return SEC_NEW_MODE;

}

/*
 * Define the access mode bits that are significant to the normal
 * UNIX discretionary access control mechanism.
 */
#define	UDAC_MODES	(SP_READACC | SP_WRITEACC | SP_CREATEACC | SP_EXECACC)

udacaccess(subject, object, mode, daemon_check, udac)
	tag_t	*subject;
	tag_t	*object;
	int	mode;
	int	daemon_check;
	register udac_t	*udac;
{
#ifdef DEBUG
	if(sware_debug)
		printf("udacaccess: mode %o\n", mode);
#endif

	/*
	 * If the access mode doesn't contain any bits we care
	 * about, or if the caller hasn't passed the object's
	 * UNIX discretionary attributes, grant access.
	 */

	if ((mode & UDAC_MODES) == 0 || udac == (udac_t *) 0)
		return 0;

	/*
	 * The access check on a directory at file create time is
	 * for SP_CREATEACC.  From our point of view, this must be
	 * considered as a SP_WRITEACC check.
	 */
	
	if (mode & SP_CREATEACC)
		mode = (mode & ~SP_CREATEACC) | SP_WRITEACC;

	/*
	 * Make the UNIX DAC decision
	 */

	if (sp_udac_decision(udac, mode & UDAC_MODES)) {
		if (!privileged(SEC_ALLOWDACACCESS, 0))
			return 1;
	}

	return 0;
}


udacgetattr(attrtype, tag, attr)
	attrtype_t	attrtype;
	tag_t		tag;
	attr_t		*attr;
{
	return(EINVAL);
}


udacsetattr(type, tags, tag_offset, new)
	attrtype_t	type;
	tag_t		*tags;
	int		tag_offset;
	tag_t		new;
{
	return(EINVAL);
}

udacsetattr_check(objtype, object, parent, tags, tag_offset, new)
	int	objtype;
	caddr_t	object, parent;
	tag_t	*tags;
	int	tag_offset;
	tag_t	new;
{
	return(EINVAL);
}

udacmaptag(attrtype, tagnum, attr, udac, new)
	attrtype_t	attrtype;
	int		tagnum;
	attr_t		*attr;
	udac_t		*udac;
	tag_t		*new;
{
	return(EINVAL);
}

/*
 * This is the SP_CHANGE_SUBJECT routine.
 */

udacchange_subj()
{
	return 0;
}

/*
 * This is the SP_CHANGE_OBJECT routine.
 */

udacchange_obj(tags, new_dac, flags)
	tag_t *tags;
	dac_t *new_dac;
	int flags;
{
	return 0;
}
#endif /* !SEC_ACL */


/*
 * Make the UNIX discretionary access control decision between the
 * current process and the object's protection attributes as stored
 * in the udac argument.  
 * This check differs between different vendor choices for whether
 * or not to support supplementary groups and on the data structures
 * used to store them.
 * This is called only by paclaccess(), aclaccess() and udacaccess().
 * Returns 0 if the access succeeds, 1 on failure.
 * PORT NOTE: Check the vendor's access routine to make sure it
 * is consistent with the checks made here.
 * MP note: Potential race here with uid/gid change
 */

sp_udac_decision(udac, mode)
	register udac_t	*udac;
	register int	mode;
{

	/*
	 * Check access to an object based on its owner, group and
	 * UNIX permission bits.
	 * NOTE the dependency on SP_READACC being 0400!!!!
	 * Recode if the OS base defines it differently
	 */

	if (u.u_uid != udac->cuid && u.u_uid != udac->uid) {
		mode >>= 3;
		/*
		 * OSF (BSD 4.4) puts the effective group in the
		 * first element of the group array
		 */

		if (u.u_gid == udac->cgid || u.u_gid == udac->gid)
			goto found;
#if SEC_GROUPS
		/*
		 * Check owning group membership.
		 * Check creating group membership only if creating group
		 * is different than the owning group.
		 */

		if (GROUPMEMBER(udac->gid) ||
		    udac->cgid != udac->gid && GROUPMEMBER(udac->cgid))
			goto found;
#endif /* SEC_GROUPS */
		mode >>= 3;
	}
found:
	if ((mode & udac->mode) != mode) {
		return 1;
	}
	return 0;
}

/*
 * dacowner()-this routine is the entry point called from the
 * security functions switch whenever a routine or security policy
 * needs to check ownership for an object. The object's owner id
 * and, for IPC objects, creator id are passed as arguments.  For
 * non-IPC objects, the owner id is supplied for both arguments.
 *
 * Returns:	0 if not owner (with EPERM)
 * 		1 if object owner
 */

dacowner(oid, cid)
	uid_t oid;	/* owner uid */
	uid_t cid;	/* creator uid */
{
	/* Ownership exists if the effective uid matches either of
	 * the object's ids (passed as arguments) or if the OWNER
	 * privilege is in effect.  The routine returns EPERM as a
	 * side effect if not the owner.  */

	return u.u_uid == oid || u.u_uid == cid || privileged(SEC_OWNER,EPERM);
}


#endif /* SEC_ARCH */
