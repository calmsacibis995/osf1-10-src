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
static char *rcsid = "@(#)$RCSfile: spd_pacl.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:06:41 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	spd_pacl.c,v $
 * Revision 1.1.1.2  92/06/23  01:44:00  devrcs
 *  *** OSF1_1B30 version ***
 * 
 * Revision 1.1.4.5  1992/04/30  17:26:28  build
 * 	Add logic to distinguish default ACL changes.
 * 	[1992/04/29  20:46:06  valin]
 *
 * Revision 1.1.4.4  1992/04/27  20:04:01  valin
 * 	Changed one more use of the udac pointer (where the pointer
 * 	value is NULL). Caused a system crash in the paclmaptag
 * 	routine.
 * 	[1992/04/27  20:00:29  valin]
 * 
 * 	Changed the paclmaptag routine to check for a null pointer to the
 * 	udac structure before it tries to copy the structure. Also, changed
 * 	the set attr routine to return ENOTDIR instead of EPERM when attempting
 * 	to set a default ACL on a non-directory object.
 * 	[1992/04/24  22:03:13  valin]
 * 
 * Revision 1.1.4.3  1992/04/21  13:17:06  marquard
 * 	Added function declaration for pacl_getobjtag().
 * 	[1992/04/20  21:40:46  marquard]
 * 
 * Revision 1.1.4.2  1992/04/05  18:53:22  marquard
 * 	POSIX ACL security policy daemon kernel code.
 * 	[1992/04/05  18:52:07  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988-91 SecureWare, Inc.  All rights reserved.
 *
 * @(#)spd_pacl.c	1.7 16:32:35 10/9/91 SecureWare
 *
 * Discretionary Access Control Module
 * Access Control List Kernel Module
 *
 * This kernel module defines the security switch entry points
 * for the portion of Discretionary Access Control implemented
 * by Access Control Lists (ACLs). Objects that have ACLs on
 * them require access decisions to be made by the ACL policy.
 * This module serves as the focal point for those access
 * decisions and the functions required to perform those access
 * decisions. This module interacts with the decision cache for
 * this policy to determine if ACL access decisions are already
 * available. The setting, deleting, and changing of object ACLs
 * is also handled through interfaces to this module.
 */

#include <sec/include_sec>
#include <sys/sec_objects.h>

#if SEC_ACL_POSIX /*{*/

extern int sware_debug;

static int pacl_getdecision();

char	*spd_allocate(), *spd_send_message();
mode_t	pacl_getobjtag();

/*
 * paclinit()-Discretionary access initialization routine
 * This is the SP_INIT() routine for the acl policy.
 * This routine returns a 1 when the policy is configured and
 * the stub routine configured when the policy is not in use
 * returns a 0 from the routine of the same name.
 */

paclinit(policy)
int policy;
{
	/* ACLs are configured */

	paclpolicy = policy;
	paclobjtag = OBJECT_TAG(policy,PACL_OBJ_ACCESS_TAG);
	paclsubjtag = SUBJECT_TAG(policy,PACL_SUBJ_TAG);
	return(1);
}

/*
 * paclcreate()
 *
 * -This routine implements the object inheritance of
 *	ACL labels when a new process or object is created.
 *
 * - The subject tag that is inherited is a direct assignment
 *	of the parent process's tag.  This tag is a composite of the
 *	process user and group ids. For Multiple Group systems,
 *	this is a database mapping.  For systems with 16 bit
 *	user and group ids, and that do not support supplementary
 *	groups, the tag is inherited from parent.
 *
 * - The object tag is based on the mode requested, the umask, and
 *	the parent's default acl.  The mapping logic is in the daemon.
 *
 * - paclcreate is called by SP_OBJECT_CREATE().
 *
 * - returns a mask of SEC_NEW_UID, SEC_NEW_GID, and SEC_NEW_MODE.
 *   only SEC_NEW_MODE is used here.
 *
 */

paclcreate(process, object, parent, attrtype, dac, umask)
tag_t *process, *object, *parent;
attrtype_t attrtype;
dac_t *dac;
mode_t umask;
{

#ifdef DEBUG
	if(attrtype != SEC_SUBJECT)
	   if(sware_debug)
		printf("paclcreate: parent %x child %x for type %d\n",
			parent,object,attrtype);
#endif

	/* Subject labels are inherited. */
	switch(attrtype) {

	   case SEC_SUBJECT:

		*object = *process;
		return(0);

	   case SEC_OBJECT:

		/*
		 * If there's a parent directory and it has a default ACL,
		 * we need to consult the daemon to tell us what the new
		 * access and default ACLs will be.  This decision depends
		 * on the specified mode, not the umask.
		 */

		if (parent != (tag_t *) 0 &&
		    dac != (dac_t *) 0 &&
		    parent[PACL_OBJ_DFLT_TAG] != SEC_WILDCARD_TAG_VALUE) {

			/*
			 * On a successful object tag retrieval,
			 * the tags will be set.  Otherwise, we need
			 * to set according to the old UNIX rules.  We
			 * can't really do anything different if the
			 * policy daemon is broken.
			 */

			dac->mode = pacl_getobjtag(dac,
					 parent[PACL_OBJ_ACCESS_TAG],
					 parent[PACL_OBJ_DFLT_TAG],
					&object[PACL_OBJ_ACCESS_TAG],
					&object[PACL_OBJ_DFLT_TAG]);

			if (dac->mode != 0)
				return SEC_NEW_MODE;
		}

		/*
		 * If there's no parent or the parent doesn't have a
		 * default ACL, the object's permissions are according
		 * to the traditional UNIX rules.
		 */

		if (dac != (dac_t *) 0) {
			dac->mode &= ~umask;	/* mask out umask */
		}

		object[PACL_OBJ_ACCESS_TAG] = SEC_WILDCARD_TAG_VALUE;
		object[PACL_OBJ_DFLT_TAG] = SEC_WILDCARD_TAG_VALUE;

		if (dac != (dac_t *) 0)
			return SEC_NEW_MODE;
		else
			return 0;
	}
}

/*
 * pacldelete()-used to maintain reference counts for ACL object
 * tags when an object is deleted. Reference count maintenance is
 * not included in the first implementation of the security policy
 * architecture.  pacldelete is the sw_obj_delete function for the acl policy.
 */

pacldelete(parent,child,attrtype)
tag_t *parent, *child;
attrtype_t attrtype;
{
	/* Not used in this release */

	return(1);
}

/*
 * Define the access mode bits that are relevant to the ACL policy.
 */

#define	ACL_MODES	(SP_READACC|SP_WRITEACC|SP_CREATEACC|SP_EXECACC)

/*
 * paclaccess()-This is the access control routine for the ACL
 * policy, and is called by SP_ACCESS().
 * The subject tag points to the composite tag for the
 * subject requesting object access. For 16 bit user and group ids
 * and no supplementary groups, the tag is
 * not a database tag but simply a 32-bit entity including the
 * uid and gid (both ushorts) of the subject.  For Multiple Group
 * systems, the subject tag is a mapping from the
 * effective user and group IDs and supplementary groups to a tag.
 * The object tag is a database tag that has a mapping to the IR
 * for the ACL.  * A Wildcard ACL, an object tag of 0, indicates
 * that ACL access * is granted (the decision is a Unix
 * discretionary one).
 *
 * Function returns:	0	= access allowed
 * 			1	= access denied
 * 	       	       -1	= no cache entry
 * MP NOTE:
 * modifies subject tag, so tag pool copying can't be done
 */

paclaccess(subject, object, mode, daemon_check, udac)
	tag_t		*subject;
	tag_t		*object;
	int		mode;
	int		daemon_check;
	register udac_t	*udac;
{
	register int	acl_decision;
	register int	unix_mode;

#ifdef DEBUG
	if(sware_debug & 2)
		printf("paclaccess: subject %x object %x mode %x\n",
			subject,object,mode);
#endif

	/*
	 * If the access mode contains no bits of interest to
	 * this policy, grant access.
	 */
	if ((mode & ACL_MODES) == 0) {
#ifdef DEBUG
		if (sware_debug & 2)
			printf("uninteresting mode %x\n", mode);
#endif
		return 0;
	}
		
	/*
	 * If no udac pointer was passed, there is no discretionary
	 * check on this object.
	 */

	if (udac == (udac_t *) 0)
		return 0;

	/*
	 * A process with allowdacaccess overrides the permission bit
	 * and access control list DAC checks
	 */

	if(privileged(SEC_ALLOWDACACCESS,0))
		return(0);

	/*
	 * A create check is made for write access to a directory.
	 */

	unix_mode = 0;
	if (mode & (SP_CREATEACC | SP_WRITEACC))
		unix_mode |= SP_WRITEACC;
	unix_mode |= mode & (SP_READACC | SP_EXECACC);


	/*
	 * If wildcard tag on object make decision based on dac.
	 * Incorporate Mandatory Locking checks if the system implements them.
	 * Also systems that check this routine for SUID/SGID.
	 */

	if (*object == SEC_WILDCARD_TAG_VALUE) {
#ifdef DEBUG
		if (sware_debug)
			printf("wildcard tag on object\n");
#endif
		if (sp_udac_decision(udac, unix_mode)) {
#ifdef DEBUG
			if (sware_debug)
			  printf("paclaccess: Failing access based on dac.\n");
#endif

			return 1;
		}
		else
			return 0;
	}
		
	if (paclpolicy == -1) 
		return 1;

#if SEC_GROUPS
	/*
	 * A wildcard subject tag and no ALLOWDACACCESS privilege
	 * means the process must establish an identity by having
	 * the daemon map a tag for it.
	 */

	if (*subject == SEC_WILDCARD_TAG_VALUE)
		if (pacl_mapsubj_tag(subject)) /* MP NOTE: PENDABLE */
			return(1);
#endif

	/* Using the subject tag (uid/gid) and the object tag, request
	 * an access decision from the cache or from the daemon.
	 * MP NOTE: PENDABLE
	 */

	acl_decision = pacl_getdecision(*subject,*object,unix_mode,daemon_check);

	if(acl_decision == -1) {
		if(daemon_check == 0) {
			return(-1);
		}
		return(1);
	}

	if ((unix_mode & acl_decision) != unix_mode) 
		return(1);

	return(0);
}

/*
 * paclgetattr()-used by high level interface routines to get the
 * ACL entries from an object. The object tag identifies the ACL
 * entries stored in the database. The ACL header (cuid/cgid values)
 * is stripped from the actual ACL IR before returning to the
 * caller.  Note that Multiple Groups systems do not support the retrieval
 * of subject tags through this interface.
 * This is the SP_GETATTR() routine for the acl policy.
 */

paclgetattr(attrtype, tag, attr)
	attrtype_t	attrtype;
	tag_t		tag;
	attr_t		*attr;
{
	register struct spd_get_attribute	*mp;
	register struct spd_internal_rep	*rp;
	int	buff_size = 0;
	attr_t	kattr;


	/* Copyin from user space the IR pointer and length */

	if (copyin(attr, &kattr, sizeof kattr)) 
		return -1;	/* XXX caller doesn't check return value! */

	/* Do not attempt to get IR for Wildcard tags */

	if (tag == SEC_WILDCARD_TAG_VALUE) {
		kattr.ir_length = 0;
		kattr.code = SEC_WILDCARD_TAG;
#ifdef DEBUG
		if (sware_debug)
			printf("paclgetattr: WILDCARD tag\n");
#endif

		if (copyout(&kattr, attr, sizeof kattr)) 
			return -1;
		return 0;
	}

	/* Retrieve and return the IR for the label */

	mp = (struct spd_get_attribute *) spd_allocate(sizeof *mp);
	if (mp == NULL) 
		return -1;

	mp->mhdr.msg_type = SPD_GET_ATTRIBUTE;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;
	mp->tag = tag;
	mp->tag_type = attrtype;

	/* Send the message to the ACL daemon */

	buff_size = sizeof *mp;
	rp = (struct spd_internal_rep *)
			spd_send_message(mp, &buff_size, paclpolicy);

	/* Check for error return else copy the label to user space */

	if (rp == (struct spd_internal_rep *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return -1;
	}

	/* If the buffer is not large enough truncate the return copy
	 * but indicate in the kattr copyout the actual label size. */

	if (rp->ir.ir_length > kattr.ir_length)
		buff_size = kattr.ir_length;
	else
		buff_size = rp->ir.ir_length;

	kattr.code = SEC_ACTUAL_TAG;
	kattr.ir_length = rp->ir.ir_length;

	/* Do not attempt to copyout() if the buffer size is 0 */

	if (buff_size > 0 && copyout(&rp[1], kattr.ir, buff_size)) {
		spd_deallocate(rp);
		return -1;
	}

	spd_deallocate(rp);

	/* Copyout the attr structure to indicate to caller the label size */

	if (copyout(&kattr, attr, sizeof kattr)) 
		return -1;

	return 0;
}

/*
 * paclsetattr()-this is the ACL policy specific portion of the
 * setlabel(2) system call. All of the object dependent checks
 * have been made in the paclsetattr_check() routine. This routine
 * therefore need only set the new tag returned from the map tag
 * call into the object.  Subject tag changes are made in the
 * dac_newsubtag() routine, in spd_misc.c, which is called from
 * each location where there is a potential change in subject
 * identity.  This is the SP_SETATTR() routine for the ACL policy.
 */

paclsetattr(type, tags, tagnum, new)
	attrtype_t	type;
	tag_t		*tags;
	int		tagnum;
	tag_t		new;
{
	tag_t	*target;

	if (type == SEC_SUBJECT)
		target = &tags[SUBJECT_TAG(paclpolicy,tagnum)];
	else
		target = &tags[OBJECT_TAG(paclpolicy,tagnum)];
#ifdef DEBUG
	if(sware_debug)
		printf("paclsetattr: new tag value %x for object %x\n",
			new, *target);
#endif
	/* Audit support for the setlabel(2) system call */

	/* Set the new label tag */

	*target = new;
	return(0);	/* XXX caller doesn't expect a return value! */
}

/*
 * paclsetattr_check()-this routine performs low-level system
 * dependent checks on the objects involved in the setattr
 * system call. This is used to maintain the relationship
 * between the objects and to isolate any system dependencies
 * from the actual label setting code.  Programs may not set
 * the subject identity with this call.  That is only accomplished
 * using the UNIX subject identity changing calls (setuid, etc.).
 * This is the SP_SETATTR_CHECK routine for the acl policy.
 * XXX MP note: If vnode not locked between setattr_check and setattr,
 * this check may be invalid.
 */

paclsetattr_check(objtype, object, parent, tags, tagnum, new)
	int	objtype;
	caddr_t	object,
		parent;
	tag_t	*tags;
	int	tagnum;
	tag_t	new;
{
	struct ipc_perm *perm;
        int     error;

	/* Perform target-specific owner checks */

	switch(objtype) {

	   case OT_PROCESS:	/* not allowed */
		return(1);

	   case OT_REGULAR:
	   case OT_DEVICE:
	   case OT_DIRECTORY:
	   case OT_PIPE:

	   {
		struct vnode	*vp = (struct vnode *) object;
		struct vattr	vattr;

		/* if setting default ACL, must be on a directory */

		if (tagnum == PACL_OBJ_DFLT_TAG && vp->v_type != VDIR) 
			return(ENOTDIR);

                VOP_GETATTR(vp, &vattr, u.u_cred, error);
		if ((error) ||
		    !sec_owner(vattr.va_uid, vattr.va_uid)) {
			return(1);
		}
		break;
	   }

	   case OT_MESSAGE_QUEUE:

		perm = &((struct msqid_ds *)object)->msg_perm;
		goto ipc_owner_check;

	   case OT_SEMAPHORE:

		perm = &((struct semid_ds *)object)->sem_perm;
		goto ipc_owner_check;

	   case OT_SHARED_MEMORY:

                perm = &((struct shmid_internal *)object)->s.shm_perm;

ipc_owner_check:

#ifdef DEBUG
		if(sware_debug)
		   printf("paclsetattr_check: ipc %x uid %d gid %d\n",
			object, perm->uid, perm->gid);
#endif

		if(!sec_owner(perm->uid, perm->cuid)) {
			return(1);
		}

		break;

	   default:

		break;	/* XXX something more fatal ? */

	}

	return(0);
}

/*
 * Map an ACL internal representation to a tag
 * This is the SP_MAPTAG() routine for the acl policy.
 * If the caller is requesting an access tag and there are
 * only three entries and they are the required three entries,
 * we need to return a WILDCARD tag and reset the udac structure
 * to reflect the new mode.  Otherwise, we go to the daemon to
 * map a tag.
 */

paclmaptag(attrtype, tagnum, attr, udac, new_tag)
	attrtype_t	attrtype;
	int		tagnum;
	attr_t		*attr;
	udac_t		*udac;
	tag_t		*new_tag;
{
	int	buff_size = 0;
	register struct	spd_map_tag	*map_tag;
	register struct	spd_set_tag	*set_tag;
	udac_t				newudac;
	int				entries;

	/* Maximum size check on the label buffer size */

	if (attr->ir_length > SEC_MAX_IR_SIZE)
		return EINVAL;

	/* if there are no entries, we must be mapping a default tag */

	if (attr->ir_length == 0)
		if (tagnum == PACL_OBJ_DFLT_TAG) {
			*new_tag = SEC_WILDCARD_TAG_VALUE;
			return 0;
		}
		else
			return EINVAL;

	/* Allocate a message buffer in preparation for sending to daemon */

	buff_size = SPD_MAP_TAG_SIZE + attr->ir_length;
	map_tag = (struct spd_map_tag *) spd_allocate(buff_size);

	/* Retrieve the ACL from user space */

	if (copyin(attr->ir, (char *) map_tag + SPD_MAP_TAG_SIZE,
			attr->ir_length)) {
		spd_deallocate(map_tag);
		return EFAULT; 
	}

	/* map a tag through the daemon */

	map_tag->mhdr.msg_type = SPD_MAP_TAG;
	map_tag->mhdr.error_code = 0;
	map_tag->mhdr.mh_flags = 0;

	map_tag->which_tag = tagnum;
	map_tag->ir_type = attrtype;
	if (udac != (udac_t *)0)
		map_tag->unixdac = *udac;
	else
		bzero(&map_tag->unixdac, sizeof(udac_t));

	map_tag->ir.ir_length = attr->ir_length;


	set_tag = (struct spd_set_tag *)
			spd_send_message(map_tag, &buff_size, paclpolicy);

	/* Return error if the mapping request did not succeed */

	if (set_tag == (struct spd_set_tag *) 0 || 
	    set_tag->mhdr.error_code != 0) {

		int code = set_tag->mhdr.error_code;

		if (set_tag)
			spd_deallocate(set_tag);
		if (code == SPD_EBADIREP)
			return EINVAL;
		else
			return EIO;
	}

	*new_tag = set_tag->tag;

#ifdef DEBUG
	if (sware_debug)
		printf
		  ("map_tag dmn rsp: tag:%x flags:%s uid:%d gid:%d mode:0%o\n",
		set_tag->tag,
		set_tag->obj_flags == SEC_NEW_MODE ? "new_mode" : "null",
		set_tag->obj_discr.uid,
		set_tag->obj_discr.gid,
		set_tag->obj_discr.mode);
#endif

	/* For this policy, only the mode can change due to a mapping */

	if ((set_tag->obj_flags == SEC_NEW_MODE) && (udac != (udac_t *)0))
		udac->mode = set_tag->obj_discr.mode;

	spd_deallocate(set_tag);
	return(0);

}

#if SEC_GROUPS /*{*/
/*
 * map the process identity to a tag.  For multiple group systems, this must
 * incorporate the effective user and group of the process, as well as the
 * supplementary groups.  This mapping is avoided until the process makes
 * its first access, to avoid unnecessary tag mappings of intermediate
 * identities.  NOTE: the IR for subject identities is fixed length.
 * the setgroups() system call fills unused groups with NOGROUP.
 * This is called only from paclaccess().
 */
pacl_mapsubj_tag(subject)
tag_t *subject;
{
	register struct	spd_map_tag	*mp;
	register struct	spd_set_tag	*rp;
	register dacid_t		*dac_ir;
	int	buff_size;
	int	dacir_size;
	int	i, ngroups;

	/* Build the message for the daemon */

	ngroups = u.u_ngroups;
	dacir_size = sizeof(dac_ir->uid) +
	   sizeof(dac_ir->gid) + (ngroups * sizeof(dac_ir->groups[0]));
	buff_size = sizeof *mp + dacir_size;
	mp = (struct spd_map_tag *) spd_allocate(buff_size);
	if (mp == NULL) 
		return 1;
	mp->mhdr.msg_type = SPD_MAP_TAG;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;
	mp->ir_type = SEC_SUBJECT;
	mp->ir.ir_length = dacir_size;

	/* point to ir body, and fill with contents of identity */

	dac_ir = (dacid_t *) &mp[1];
	dac_ir->uid = u.u_uid;
	dac_ir->gid = u.u_gid;
	for (i = 0; i < ngroups; ++i)
		dac_ir->groups[i] = u.u_groups[i];

	/* send message to daemon */

	rp = (struct spd_set_tag *) spd_send_message(mp, &buff_size, paclpolicy);

	/* if tag couldn't be mapped, operation fails. */

	if (rp == (struct spd_set_tag *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return(1);
	}
		
	/* Copy tag into subject identity */

	*subject = rp->tag;	/* MP Note:  Tag modification */
	spd_deallocate(rp);
	return(0);
}
#endif /*} SEC_GROUPS */


/*-----------------------------------------------------------------------*/
/*	Routines not part of the switch called entries follow this	 */
/*-----------------------------------------------------------------------*/


/*
 * pacl_getdecision()-make a decision between the subject and object
 * based on presented tags. Checks the cache first and then prepares
 * a message for the daemon if not in the cache. The cache is loaded
 * with the decision upon return unless an error occurred.
 * Returns a decision word composed of logical OR of SP_READACC,
 * SP_WRITEACC, and SP_EXECACC.  This is called only by paclaccess().
 *
 * Daemon check = 0	Only consult decision cache
 * 	     = 1	Consult policy daemon if needed
 * MP note:
 * 
 * Even though there are several pendable paths here, we don't need to
 * worry about attribute changes invalidating the access decision.
 * This is because the cache lookups are based on tag values.  If the
 * attributes change, the tags will change, and thus prevent the use
 * of stale decisions.
 */

static
pacl_getdecision(subject, object, unix_mode, daemon_check)
	tag_t	subject, object;
	int	unix_mode;
	int	daemon_check;
{
	acache_dec_t			dec;
	int				buff_size;
	struct spd_make_decision	*mp;
	struct spd_decision		*rp;
	int				ret = 0;

	/* Check for subject and object cache entry */

	if (dcache_lookup(paclpolicy, subject, object, &dec, sizeof dec) == 0) {

		/* For quick checking, do not request decision from daemon */

		if(daemon_check == 0)
			return(-1);

		/* Request the access decision from the ACL daemon */

		buff_size = sizeof *mp + sizeof dec;
		mp = (struct spd_make_decision *) spd_allocate(buff_size);

		mp->mhdr.msg_type = SPD_MAKE_DECISION;

		/* Need to send the mode requested to the daemon.  */
		mp->mhdr.mh_flags = (unix_mode >> 6) & 0007;
		mp->mhdr.error_code = 0;

		mp->subject = subject;
		mp->object = object;

		rp = (struct spd_decision *)
				spd_send_message(mp, &buff_size, paclpolicy);

		/* Check the return status from the daemon */

		if (rp == (struct spd_decision *) 0 || rp->mhdr.error_code) {
			spd_deallocate(rp);
			return(-1);
		}

		/* save the decision and release the reply message */

		dec = *(acache_dec_t *) &rp[1];
		spd_deallocate(rp);

		/* Load the decision into the cache */

		dcache_insert(paclpolicy, subject, object, &dec, sizeof dec);
	}

	/*
	 * Compute return value from decision structure
	 */

	if (dec.decision.read)
		ret |= SP_READACC;
	if (dec.decision.write)
		ret |= SP_WRITEACC;
	if (dec.decision.exec)
		ret |= SP_EXECACC;

	return(ret);
}

/*
 * pacl_getobjtag() makes a request to the daemon to
 *	implement the POSIX object inheritance
 *	method of acl assignment.  This routine is only called
 *	if the parent directory of the file to be created has a
 *	default ACL.  Returns the new mode of the object, or 0.
 */

mode_t
pacl_getobjtag(dac, access_tag, default_tag, new_access_tag, new_default_tag)
dac_t	*dac;			/* pointer to the new file's attributes */
tag_t	access_tag;		/* parent directory's access ACL tag */
tag_t	default_tag;		/* parent directory's default ACL tag */
tag_t	*new_access_tag;	/* return - new file's access ACL */
tag_t	*new_default_tag;	/* return - new file's default ACL */
{

	int				buff_size;
	struct spd_object_create	*mp;
	struct spd_new_objtag		*rp;
	mode_t				mode;

	/* Request the new tag from the ACL daemon */

	buff_size = sizeof *mp;
	mp = (struct spd_object_create *) spd_allocate(buff_size);

	mp->mhdr.msg_type = SPD_GET_NEW_OBJTAG;

	mp->mhdr.mh_flags = 0;
	mp->mhdr.error_code = 0;

	mp->access_tag = access_tag;
	mp->default_tag = default_tag;

	mp->edac.euid = dac->uid;
	mp->edac.egid = dac->gid;

	/* Need to send the mode requested to the daemon.  */
	mp->mode = dac->mode;

#ifdef DEBUG
	if (sware_debug)
	  printf("pacl_getobjtag: effective uid:%d effective gid:%d\n",
		mp->edac.euid, mp->edac.euid);
#endif

	rp = (struct spd_new_objtag *)
			spd_send_message(mp, &buff_size, paclpolicy);

	/* Check the return status from the daemon */

	if (rp == (struct spd_new_objtag *) 0 || rp->mhdr.error_code) {

		if (rp)
			spd_deallocate(rp);
		return -1;	/* Return not checked. */
	}

	/* set the tag and mode, then release the reply message */

	*new_access_tag = rp->access_tag;
	*new_default_tag = rp->default_tag;

	mode = rp->mode;

	spd_deallocate(rp);

	return mode;
}

/*
 * paclchange_obj()-this routine is called from the security functions
 * switch when a discretionary attribute change takes place for an
 * object. It is the SP_CHANGE_OBJECT() routine.
 * This is required since coordination with certain security
 * policies may be necessary (such as ACLs to support ACL_OWNER entries).
 * The return value of the function specifies whether any components of
 * the object's discretionary attributes (owner, group, or mode) changed
 * as a result of the change to the access control list.
 * This version of the function always returns 0 (no need to change the
 * owner/group/mode when the ACL is changed due to a change in the owner,
 * group, or mode).
 *
 */

paclchange_obj(tags,new_dac,flags)
	tag_t	*tags;		/* object's tag pool */
	dac_t	*new_dac;	/* new attributes for object */
	int	flags;		/* which fields of new_dac are changed */
{
	struct spd_change_discr	*mp;
	struct spd_new_discr	*rp;
	tag_t			*target;
	int			msgsize;
	int			ret;

	if (paclpolicy == -1)
		return 0;

	/* If tag is wildcard, no need to interact with daemon. */
	target = &tags[OBJECT_TAG(paclpolicy, 0)];
	if (*target == SEC_WILDCARD_TAG_VALUE)
		return(0);

#ifdef DEBUG
	printf ("paclchange_obj: tag:%x uid:%d gid:%d mode:%o\n",
		*target,new_dac->uid,new_dac->gid,new_dac->mode);
#endif

	/* Fetch the internal rep to rewrite with new header */

	mp = (struct spd_change_discr *) spd_allocate(sizeof *mp);
	if (mp == NULL)
		return 0;
	mp->mhdr.msg_type = SPD_CHANGE_DISCR;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;
	mp->object_dac = *new_dac;
	mp->object_tag = *target;
	mp->object_flags = flags;

	msgsize = sizeof *mp;
	rp = (struct spd_new_discr *)spd_send_message(mp, &msgsize, paclpolicy);

	if (rp == (struct spd_new_discr *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return 0;
	}

	/* Assign the tag returned by the policy daemon */

	*target = rp->object_tag; /* MP NOTE: tag change */

	/* No changes to ACL reflect back onto the owner/group/mode */

	if (ret = rp->object_flags)
		*new_dac = rp->object_discr;

#ifdef DEBUG
	if (sware_debug)
	  printf("paclchange_obj: tag:%x uid:%d gid:%d mode:%o ret: 0%o\n",
		*target,rp->object_discr.uid,rp->object_discr.gid,
		rp->object_discr.mode, ret);
#endif

	spd_deallocate(rp);
	return ret;
}

/*
 * paclchange_subj() - the SP_CHANGE_SUBJECT() routine for the acl policy
 * This routine is called from various locations
 * in the kernel where the effective user id and group id values
 * of the process change. This affects the access decisions for
 * ACLs concerning the ACL_OWNER entries. The subject tag that is
 * used to determine access for those ACL entries is recomputed.
 * For systems with supplementary groups, the tag cannot
 * be built directly but must be mapped by the database.
 * This evaluation is saved until the first access check without
 * ALLOWDACCESS, to avoid unnecessary tag allocations during process
 * identity changes through a sequence of setuid, setgid, and setgroups
 * calls.
 */

#if !SEC_GROUPS
paclchange_subj()
{
	register dacid_t *dac;

	if (paclpolicy != -1) {

		/* Set the tag pool pointer to the first ACL subject tag */

		dac = (dacid_t *)
			&SIP->si_tag[SUBJECT_TAG(paclpolicy,PACL_SUBJ_TAG)];

		/* Set the uid/gid of the process into the tag */
		/* MP NOTE: tag change */

		dac->uid = u.u_uid;
		dac->gid = u.u_gid;
	}
	return(0);
}
#else
paclchange_subj()
{
	/* Set the ACL subject tag to wildcard */

	if (paclpolicy != -1)	/* see paclaccess() */
		SIP->si_tag[SUBJECT_TAG(paclpolicy,PACL_SUBJ_TAG)] =
				SEC_WILDCARD_TAG_VALUE;
	return(0);
}
#endif

#endif /*} SEC_ACL_POSIX */
