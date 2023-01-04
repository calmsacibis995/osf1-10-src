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
static char *rcsid = "@(#)$RCSfile: spd_misc.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/04/01 20:06:35 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* @(#)spd_misc.c	6.2 18:09:36 1/31/91 SecureWare */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 *
 * Security Policy Miscellaneous Routines
 *
 * This module contains miscellaneous security policy routines that
 * are used by all policies.
 */

#include <sec/include_sec>

#if SEC_ARCH /*{*/

char	*spd_allocate(), *spd_send_message();

#ifdef DEBUG
extern int sware_debug;
#endif

/*
 * SP_OBJECT_CREATE()-this routine implements the object label
 * inheritance routine which calls all security policies in the
 * switch to perform the policy specific inheritance rules. Some
 * policies may choose to do nothing. The process, object, and 
 * parent pointers point to the beginning of each object's tag pool.
 *
 * POSIX ACLS -- add new arguments (dac,umask) to function
 * 
 */

sp_object_create(process,object,parent,attrtype,dac,umask)
tag_t *process, *object, *parent;
attrtype_t attrtype;
dac_t *dac;
mode_t umask;
{
	tag_t *proctag, *objtag, *partag;
	int i;
	int ret_flags = 0;
        int flags;


	/* Call each security policy for label inheritance */

	for(i=0; i < spolicy_cnt; i++) {
		proctag = &process[sp_switch[i].sw_first_subj_tag];
 
	/*
	 * If the attribute type is a subject, then the target object
	 * tag pointer is assumed to be a process. The tag pointer
	 * must be calculated using the subject tag offset instead of
	 * the object tag offset as is the case for an object.
	 */
 
		if(attrtype == SEC_SUBJECT)
			objtag =  &object[sp_switch[i].sw_first_subj_tag];
		else
			objtag =  &object[sp_switch[i].sw_first_obj_tag];

	/* The parent object pointer may be Null */

		if(parent != (tag_t *) 0)
			partag =  &parent[sp_switch[i].sw_first_obj_tag];
		else partag = (tag_t *) 0;

		/*
		 *  POSIX ACLS -- new arguments to function
		 *                and checking of return codes
 		 */
                flags = (*sp_switch[i].sw_obj_create)
                                (proctag,objtag,partag,attrtype,dac,umask);
                if (flags)
                        ret_flags = flags;

	}

	return(ret_flags);
}

/*
 * SP_DELETE()-this routine implements the interface to the
 * security policies to handle any tag manipulation as part of
 * object deletion. This is intended for use with tag reference
 * count maintenance (subsequent releases).
 */

SP_DELETE(object,objtype)
tag_t *object;
int objtype;
{
	return(0);
}

/*
 * SP_ACCESS()-this routine is used to make access decisions
 * to test for access permission to an object. This routine is
 * provided to cycle through the security policy switch entries for
 * each policy configured, prepare the arguments for the access check
 * call to the policy, and react to the return value. If any of the
 * policies returns an indicator of denied access, this return may
 * return immediately indicating no access since SP_ACCESS() is defined
 * to return success (a 0 value) only when all policies do so.
 *
 * Returns:	0      if access is permitted
 * 		EACCES if access from any policy is denied
 */

SP_ACCESS(subject,object,mode,udac)
tag_t *subject, *object;
int mode;
udac_t *udac;
{
	tag_t *first_sub, *first_obj;
	int ret, i, access_okay = 1;
	char check_policy[SEC_TAG_COUNT];

	/* For each policy, set the tag pointers for the subject and
	 * object to the appropriate entry in the tag pool whose ptr
	 * is passed based on the switch definition of tag locations.
	 * Each policy is called first without daemon checking to see
	 * if a negative decision can be located quickly.	   */

	for(i=0; i < spolicy_cnt; i++) {
		first_sub = &subject[sp_switch[i].sw_first_subj_tag];
		first_obj = &object[sp_switch[i].sw_first_obj_tag];

		ret = (*sp_switch[i].sw_access)(first_sub,first_obj,
						  mode,0,udac);

		/* If the return is 1, access is denied. A 0 return means
		   access is granted, while  -1 indicates a decision 
                   from the corresponding policy daemon is needed. */

		if(ret == 1) {			/* Access denied, bye */
			return EACCES;
		}
		else if(ret == -1) {
			check_policy[i] = 1;
			access_okay = 0;
		     }
		     else {		/* Access granted, don't recheck */
			check_policy[i] = 0;
			continue;
		     }
	}

	/* In spinning through the cache-check-only loop, any denied
	 * access decisions caused an immediate return. Thus, to get this
	 * far, all policies were unable to determine that access was
	 * denied since the cache did not have a decision or it had an
	 * affirmative decision. The latter condition can be detected by
	 * all policies returning 0 meaning the cache has a grant decision. */

	if(access_okay == 1)	/* All policies grant access from cache */
		return(0);

	/* Perform the same access checks with daemon checking enabled
	 * this time since no negative decisions could be found without
	 * daemon consultation.					    */

	for(i=0; i < spolicy_cnt; i++) {

	/* check the policy flag to see if we need to re-call it */

		if(check_policy[i] == 0)
			continue;

		first_sub = &subject[sp_switch[i].sw_first_subj_tag];
		first_obj = &object[sp_switch[i].sw_first_obj_tag];

		if((*sp_switch[i].sw_access)(first_sub,first_obj,mode,1,udac)) {
			return EACCES;
		}
	}

	/* All policies grant access */

	return(0);
}

/*
 * sec_ipcaccess
 *
 * Front end to SP_ACCESS for System V IPC object access checks.
 */

sec_ipcaccess(perm, tags, mode)
	register struct ipc_perm	*perm;
	tag_t				*tags;
	int				mode;
{
	udac_t	udac;

	if (security_is_on) {
		udac.uid = perm->uid;
		udac.cuid = perm->cuid;
		udac.gid = perm->gid;
		udac.cgid = perm->cgid;
		udac.mode = perm->mode & 0777;

		return SP_ACCESS(SIP->si_tag, tags, mode, &udac);
	}

	/* code taken from ipcaccess */
	if (u.u_uid == 0)
		return (0);
	/* ignore all modes except read, write and exec */
	if (! (mode & (SP_READACC|SP_WRITEACC|SP_EXECACC))) {
		return (0);
	}

	if (u.u_uid != perm->uid && u.u_uid != perm->cuid) {
		mode >>= 3;
		if (u.u_gid != perm->gid && u.u_gid != perm->cgid)
			mode >>= 3;
	}

	if (mode & perm->mode)
		return (0);

	return (1);
}

/*
 * sp_clear_tags()-called from exit() to clear the process tags.
 * Give back the ILB bitmap we got at birth from secinfo_dup().
 * MP note:
 * Must SIP_ILB_LOCK here to avoid possible races with other code
 * paths that may try to access si_subj_bits while we're freeing it.
 */

sp_clear_tags()
{
	bzero(SIP->si_tag, sizeof(tag_t) * SEC_TAG_COUNT);
}

/*
 * sp_maptag()-map the specified internal representation to a tag
 * and return that tag to the caller.
 * This version of the function is plugged into all security policy
 * switch entries.  In the future, policies can implement their own
 * maptag functions that may maintain a kernel cache of attributes.
 * Returns 0 on success or errno on failure.
 */

sp_maptag(policy,attrtype,tagnum,attr,udac,new_tag,ir_space)
	int		policy;		/* switch offset */
	attrtype_t	attrtype;	/* SEC_SUBJECT or SEC_OBJECT */
	int		tagnum;		/* policy-relative tag offset */
	attr_t		*attr;		/* pointer to IR and IR type */
	udac_t		*udac;		/* owner/creator uid/gid, and mode */
	tag_t		*new_tag;	/* mapped tag is placed here */
	int		ir_space;	/* SEC_IR_KERNEL or SEC_IR_USER */
{
	register struct	spd_map_tag	*map_tag;
	register struct	spd_set_tag	*set_tag;
	int				buff_size = 0;

	/* Validity check on the label buffer size-0 length is OK */

	if (attr->ir_length > SEC_MAX_IR_SIZE)
		return EINVAL;

	/* Map the new attribute to a tag using the daemon */

	buff_size = SPD_MAP_TAG_SIZE + attr->ir_length;
	map_tag = (struct spd_map_tag *) spd_allocate(buff_size);

	map_tag->mhdr.msg_type = SPD_MAP_TAG;
	map_tag->mhdr.error_code = 0;
	map_tag->mhdr.mh_flags = 0;

	map_tag->which_tag = tagnum;
	map_tag->ir_type = attrtype;
	if (udac != (udac_t *) 0)
		map_tag->unixdac = *udac;
	else
		bzero(&map_tag->unixdac, sizeof(udac_t));

	map_tag->ir.ir_length = attr->ir_length;

	/*
	 * Put IR into the message from user/kernel space
	 * if length greater than 0
	 */

	if (attr->ir_length) {
		if (ir_space == SEC_IR_KERNEL)
			bcopy(attr->ir, (char *) map_tag + SPD_MAP_TAG_SIZE,
				attr->ir_length);
		else if (copyin(attr->ir, (char *) map_tag + SPD_MAP_TAG_SIZE,
				attr->ir_length)) {
			spd_deallocate(map_tag);
			return EFAULT; 
		}
	}

	set_tag = (struct spd_set_tag *)
			spd_send_message(map_tag, &buff_size, policy);

	/* Return error if the mapping request did not succeed */

	if (set_tag == (struct spd_set_tag *) 0 || 
	    set_tag->mhdr.error_code != 0) {
		if (set_tag)
			spd_deallocate(set_tag);
		return EIO;
	}

	*new_tag = set_tag->tag;
	spd_deallocate(set_tag);
	return(0);
}

/*
 * Common code for System V IPC code (semget(), shmget(), msgget())
 */

sec_svipc_object_create(tags)
register tag_t *tags;
{
	
	/* POSIX ACLS --  new arguments to function and 
         * ignore return value since no back pressure is possible 
	 */

        SP_OBJECT_CREATE(SIP->si_tag, tags, (tag_t *) 0, SEC_OBJECT,
                                (dac_t *) 0, (mode_t) 0);
#if SEC_MAC || SEC_NCAV
	AUDSTUB_LEVELS(tags);
#endif
}

#endif /*} SEC_ARCH */
