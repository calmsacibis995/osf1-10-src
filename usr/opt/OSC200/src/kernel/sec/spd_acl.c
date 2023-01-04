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
static char	*sccsid = "@(#)$RCSfile: spd_acl.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/06/03 15:00:58 $";
#endif 
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
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

/* #ident "@(#)spd_acl.c	4.1 21:44:58 7/9/90 SecureWare"*/
/* Based on:
 *	"@(#)spd_acl.c	2.10.1.2 12:46:33 2/2/90 SecureWare, Inc."
 */

#include <sec/include_sec>

#if SEC_ACL_SWARE /*{*/

extern int sware_debug;

static int acl_getdecision();

char	*spd_allocate(), *spd_send_message();

/*
 * aclinit()-Discretionary access initialization routine
 * This is the SP_INIT() routine for the acl policy.
 * This routine returns a 1 when the policy is configured and
 * the stub routine configured when the policy is not in use
 * returns a 0 from the routine of the same name.
 */

aclinit(policy)
int policy;
{
	/* ACLs are configured */

	aclpolicy = policy;
	aclobjtag = OBJECT_TAG(policy,ACL_OBJ_TAG);
	aclsubjtag = SUBJECT_TAG(policy,ACL_SUBJ_TAG);
	return(1);
}

/*
 * aclcreate()-This routines implements the object inheritance of
 * ACL labels when a new process or object is created. The only
 * tag that is currently inherited is the subject ACL tag which
 * is a composite of the process user and group ids. For Multiple Group
 * systems, this is a database mapping. For System V, the tag
 * can be set using the id values or inherited from parent.
 * aclcreate is called by SP_OBJECT_CREATE().
 */

aclcreate(process,object,parent,attrtype)
tag_t *process, *object, *parent;
attrtype_t attrtype;
{

	/* Only subject labels are inherited currently */

#ifdef DEBUG
	if(attrtype != SEC_SUBJECT)
	   if(sware_debug)
		printf("aclcreate: parent %x child %x for type %d\n",
			parent,object,attrtype);
#endif
	switch(attrtype) {

	   case SEC_SUBJECT:

		*object = *process;
		return(1);

	   case SEC_OBJECT:

	/* Object labels are set for access control by Unix DAC */

		*object = SEC_WILDCARD_TAG_VALUE;
		return(1);

	}
}

/*
 * acldelete()-used to maintain reference counts for ACL object
 * tags when an object is deleted. Reference count maintenance is
 * not included in the first implementation of the security policy
 * architecture.  acldelete is the sw_obj_delete function for the acl policy.
 */

acldelete(parent,child,attrtype)
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
 * aclaccess()-This is the access control routine for the ACL
 * policy, and is called by SP_ACCESS().
 * The subject tag points to the composite tag for the
 * subject requesting object access. For System V.3, the tag is
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

aclaccess(subject, object, mode, daemon_check, udac)
	tag_t		*subject;
	tag_t		*object;
	int		mode;
	int		daemon_check;
	register udac_t	*udac;
{
	register int	acl_decision;
	register int	unix_mode;

#ifdef DEBUG
	if(sware_debug)
		printf("aclaccess: subject %x object %x mode %x\n",
			subject,object,mode);
#endif

	/*
	 * If the access mode contains no bits of interest to
	 * this policy, grant access.
	 */
	if ((mode & ACL_MODES) == 0)
		return 0;

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
	 * Make UNIX DAC decision
	 * Incorporate Mandatory Locking checks if the system implements them.
	 * Also systems that check this routine for SUID/SGID.
	 */

	if (sp_udac_decision(udac, unix_mode))
		return(1);
		
	/* If the object tag is a wildcard then the access succeeds */

	if (*object == SEC_WILDCARD_TAG_VALUE)
		return(0);
	
	if (aclpolicy == -1) {
		return 1;
	}

#if SEC_GROUPS
	/*
	 * A wildcard subject tag and no ALLOWDACACCESS privilege
	 * means the process must establish an identity by having
	 * the daemon map a tag for it.
	 */

	if (*subject == SEC_WILDCARD_TAG_VALUE)
		if (acl_mapsubj_tag(subject)) /* MP NOTE: PENDABLE */
			return(1);
#endif

	/* Using the subject tag (uid/gid) and the object tag, request
	 * an access decision from the cache or from the daemon.
	 * MP NOTE: PENDABLE
	 */

	acl_decision = acl_getdecision(*subject,*object,daemon_check);

	if(acl_decision == -1) {
		if(daemon_check == 0)
			return(-1);
		AIP->si_error = ESEC_NO_DECISION;
		return(1);
	}

	if ((unix_mode & acl_decision) != unix_mode) {
		return(1);
	}
	return(0);
}

/*
 * aclgetattr()-used by high level interface routines to get the
 * ACL entries from an object. The object tag identifies the ACL
 * entries stored in the database. The ACL header (uid/gid values)
 * is stripped from the actual ACL IR before returning to the
 * caller.  Note that Multiple Groups systems do not support the retrieval
 * of subject tags through this interface.
 * This is the SP_GETATTR() routine for the acl policy.
 */

aclgetattr(attrtype, tag, attr)
	attrtype_t	attrtype;
	tag_t		tag;
	attr_t		*attr;
{
	register struct spd_get_attribute	*mp;
	register struct spd_internal_rep	*rp;
	int	buff_size = 0;
	attr_t	kattr;

	/* Copyin from user space the IR pointer and length */

	if (copyin(attr, &kattr, sizeof kattr)) {
		return(EFAULT);	/* XXX caller doesn't check return value! */
	}

	/* Do not attempt to get IR for Wildcard tags */

	if (tag == SEC_WILDCARD_TAG_VALUE) {
		kattr.ir_length = 0;
		kattr.code = SEC_WILDCARD_TAG;
#ifdef DEBUG
		if (sware_debug)
			printf("aclgetattr: WILDCARD tag\n");
#endif

		if (copyout(&kattr, attr, sizeof kattr)) {
			return(EFAULT);
		}
		return 0;
	}

	/* Retrieve and return the IR for the label */

	mp = (struct spd_get_attribute *) spd_allocate(sizeof *mp);
	if (mp == NULL) {
		return(EIO);
	}

	mp->mhdr.msg_type = SPD_GET_ATTRIBUTE;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;
	mp->tag = tag;
	mp->tag_type = attrtype;

#ifdef DEBUG
	if (sware_debug)
		printf("aclgetattr: for tag %x length %d buffer %x\n",
			mp->tag, kattr.ir_length, kattr.ir);
#endif

	/* Send the message to the ACL daemon */

	buff_size = sizeof *mp;
	rp = (struct spd_internal_rep *)
			spd_send_message(mp, &buff_size, aclpolicy);

	/* Check for error return else copy the label to user space */

	if (rp == (struct spd_internal_rep *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		AIP->si_error = ESEC_NO_DECISION;
		return(EACCES);
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
		return(EFAULT);
	}

	spd_deallocate(rp);

	/* Copyout the attr structure to indicate to caller the label size */

	if (copyout(&kattr, attr, sizeof kattr)) {
		return(EFAULT);
	}

	return 0;
}

/*
 * aclsetattr()-this is the ACL policy specific portion of the
 * setlabel(2) system call. All of the object dependent checks
 * have been made in the aclsetattr_check() routine. This routine
 * therefore need only set the new tag returned from the map tag
 * call into the object.  Subject tag changes are made in the
 * dac_newsubtag() routine, in spd_misc.c, which is called from
 * each location where there is a potential change in subject
 * identity.  This is the SP_SETATTR() routine for the ACL policy.
 */

aclsetattr(type, tags, tagnum, new)
	attrtype_t	type;
	tag_t		*tags;
	int		tagnum;
	tag_t		new;
{
	tag_t	*target;

	if (type == SEC_SUBJECT)
		target = &tags[SUBJECT_TAG(aclpolicy,tagnum)];
	else
		target = &tags[OBJECT_TAG(aclpolicy,tagnum)];
#ifdef DEBUG
	if(sware_debug)
		printf("aclsetattr: new tag value %x for object %x\n",
			new, *target);
#endif
	/* Audit support for the setlabel(2) system call */

	AIP->si_event_type = ET_DAC_CHANGE;
	AIP->si_label = AUDIT_ACL_OBJ;
	AIP->si_cur_label = *target;
	AIP->si_new_label = new;

	/* Set the new label tag */

	*target = new;
	return(0);	/* XXX caller doesn't expect a return value! */
}

/*
 * aclsetattr_check()-this routine performs low-level system
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

aclsetattr_check(objtype, object, parent, tags, tagnum, new)
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
		return(EACCES);

	   case OT_REGULAR:
	   case OT_DEVICE:
	   case OT_DIRECTORY:
	   case OT_PIPE:

	   {
#if defined(_OSF_SOURCE)
		struct vnode	*vp = (struct vnode *) object;
		struct vattr	vattr;

                VOP_GETATTR(vp, &vattr, u.u_cred, error);
		if ((error) ||
		    !sec_owner(vattr.va_uid, vattr.va_uid)) {
			AIP->si_error = ESEC_NOT_OWNER;
			return(EPERM);
		}
#else /* !_OSF_SOURCE */
		register struct inode *ip = (struct inode *) object;
#ifdef DEBUG
		if(sware_debug)
		   printf("aclsetattr_check: file access ip %x uid %d gid %d\n",
			ip,ip->i_uid,ip->i_gid);
#endif

		if(!sec_owner(ip->i_uid, ip->i_uid)) {
			AIP->si_error = ESEC_NOT_OWNER;
			return(EPERM);
		}
#endif /* !_OSF_SOURCE */

		break;
	   }

	   case OT_MESSAGE_QUEUE:

		perm = &((struct msqid_ds *)object)->msg_perm;
		goto ipc_owner_check;

	   case OT_SEMAPHORE:

		perm = &((struct semid_ds *)object)->sem_perm;
		goto ipc_owner_check;

	   case OT_SHARED_MEMORY:

#ifdef _OSF_SOURCE
                perm = &((struct shmid_internal *)object)->s.shm_perm;
#else
                perm = &((struct shmid_ds *)object)->shm_perm;
#endif

ipc_owner_check:

#ifdef DEBUG
		if(sware_debug)
		   printf("aclsetattr_check: ipc %x uid %d gid %d\n",
			object, perm->uid, perm->gid);
#endif

		if(!sec_owner(perm->uid, perm->cuid)) {
			AIP->si_error = ESEC_NOT_OWNER;
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
 */

aclmaptag(attrtype, tagnum, attr, udac, new)
	attrtype_t	attrtype;
	int		tagnum;
	attr_t		*attr;
	udac_t		*udac;
	tag_t		*new;
{
	return sp_maptag(aclpolicy, attrtype, tagnum, attr, udac, new,
				SEC_IR_USER);
}

#if SEC_GROUPS /*{*/
/*
 * map the process identity to a tag.  For multiple group systems, this must
 * incorporate the effective user and group of the process, as well as the
 * supplementary groups.  This mapping is avoided until the process makes
 * its first access, to avoid unnecessary tag mappings of intermediate
 * identities.  NOTE: the IR for subject identities is fixed length.
 * the setgroups() system call fills unused groups with NOGROUP.
 * This is called only from aclaccess().
 */
acl_mapsubj_tag(subject)
tag_t *subject;
{
	register struct	spd_map_tag	*mp;
	register struct	spd_set_tag	*rp;
	register dacid_t		*dac_ir;
	int	buff_size;
	int	dacir_size;
	int	i, ngroups;

	/* Build the message for the daemon */

#ifdef _OSF_SOURCE
	ngroups = u.u_ngroups;
#else
	for (i = 0; u.u_groups[i] != NOGROUP && i < NGROUPS_MAX; i++)
		;
	ngroups = i;
#endif
	dacir_size = sizeof(dac_ir->uid) +
	   sizeof(dac_ir->gid) + (ngroups * sizeof(dac_ir->groups[0]));
	buff_size = sizeof *mp + dacir_size;
	mp = (struct spd_map_tag *) spd_allocate(buff_size);
	if (mp == NULL) {
		return 1;
	}
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

	rp = (struct spd_set_tag *) spd_send_message(mp, &buff_size, aclpolicy);

	/* if tag couldn't be mapped, operation fails. */

	if (rp == (struct spd_set_tag *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		AIP->si_error = ESEC_NO_DECISION;
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
 * acl_getdecision()-make a decision between the subject and object
 * based on presented tags. Checks the cache first and then prepares
 * a message for the daemon if not in the cache. The cache is loaded
 * with the decision upon return unless an error occurred.
 * Returns a decision word composed of logical OR of SP_READACC,
 * SP_WRITEACC, and SP_EXECACC.  This is called only by aclaccess().
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
acl_getdecision(subject, object, daemon_check)
	tag_t	subject, object;
	int	daemon_check;
{
	acache_dec_t			dec;
	int				buff_size;
	struct spd_make_decision	*mp;
	struct spd_decision		*rp;
	int				ret = 0;

	/* Check for subject and object cache entry */

	if (dcache_lookup(aclpolicy, subject, object, &dec, sizeof dec) == 0) {

		/* For quick checking, do not request decision from daemon */

		if(daemon_check == 0)
			return(-1);

		/* Request the access decision from the ACL daemon */

		buff_size = sizeof *mp + sizeof dec;
		mp = (struct spd_make_decision *) spd_allocate(buff_size);

		mp->mhdr.msg_type = SPD_MAKE_DECISION;
		mp->mhdr.mh_flags = 0;
		mp->mhdr.error_code = 0;

		mp->subject = subject;
		mp->object = object;

		rp = (struct spd_decision *)
				spd_send_message(mp, &buff_size, aclpolicy);

		/* Check the return status from the daemon */

		if (rp == (struct spd_decision *) 0 || rp->mhdr.error_code) {
			spd_deallocate(rp);
			return(-1);
		}

		/* save the decision and release the reply message */

		dec = *(acache_dec_t *) &rp[1];
		spd_deallocate(rp);

		/* Load the decision into the cache */

		dcache_insert(aclpolicy, subject, object, &dec, sizeof dec);
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
 * aclchange_obj()-this routine is called from the security functions
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
 */

aclchange_obj(tags,new_dac,flags)
	tag_t	*tags;		/* object's tag pool */
	dac_t	*new_dac;	/* new attributes for object */
	int	flags;		/* which fields of new_dac are changed */
{
	struct spd_change_discr	*mp;
	struct spd_new_discr	*rp;
	tag_t			*target;
	int			msgsize;
	int			ret;

	if (aclpolicy == -1)
		return 0;

	/* If a wildcard ACL, there is no internal rep */

	target = &tags[OBJECT_TAG(aclpolicy, 0)];
	if (*target == SEC_WILDCARD_TAG_VALUE)
		return(0);

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
	rp = (struct spd_new_discr *) spd_send_message(mp, &msgsize, aclpolicy);

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

	spd_deallocate(rp);
	return ret;
}

/*
 * aclchange_subj() - the SP_CHANGE_SUBJECT() routine for the acl policy
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
aclchange_subj()
{
	register dacid_t *dac;

	if (aclpolicy != -1) {

		/* Set the tag pool pointer to the first ACL subject tag */

		dac = (dacid_t *)
			&SIP->si_tag[SUBJECT_TAG(aclpolicy,ACL_SUBJ_TAG)];

		/* Set the uid/gid of the process into the tag */
		/* MP NOTE: tag change */

		dac->uid = u.u_uid;
		dac->gid = u.u_gid;
	}
	return(0);
}
#else
aclchange_subj()
{
	/* Set the ACL subject tag to wildcard */

	if (aclpolicy != -1)	/* see aclaccess() */
		SIP->si_tag[SUBJECT_TAG(aclpolicy,ACL_SUBJ_TAG)] =
				SEC_WILDCARD_TAG_VALUE;
	return(0);
}
#endif

#endif /*} SEC_ACL_SWARE */
