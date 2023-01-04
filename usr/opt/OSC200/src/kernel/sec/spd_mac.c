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
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * Mandatory Access Control Security Module
 *
 * This kernel module defines the security switch entry points
 * for Mandatory Access Control. The module determines what
 * actions are necessary to determine mandatory access between
 * two labels. The module makes use of the Mandatory Access
 * decision cache for performance enhancement but also interfaces
 * to the Mandatory Access Security Policy Daemon when necessary.
 */

/* @(#)spd_mac.c	6.1 17:31:25 1/31/91 SecureWare */

#include <sec/include_sec>

#if SEC_MAC /*{*/

extern int sware_debug;

/* Declarations for Multilevel Directory Name Deflection */

#define MAC_DIGITS	11
#define MAC_STRING	"mac"

char	*spd_allocate(), *spd_send_message();

/*
 * Mandatory Access Initialization Routine - SP_INIT() routine
 */

macinit(policy)
	int	policy;
{
	/* MAC is configured */

	macpolicy = policy;
	macobjtag = OBJECT_TAG(macpolicy,MAND_OBJ_SL_TAG);
	macsubjtag = SUBJECT_TAG(macpolicy,MAND_SUBJ_SL_TAG);
	macclrnce = SUBJECT_TAG(macpolicy,MAND_SUBJ_CL_TAG);

	return(1);
}

/*
 * maccreate()-this ruotine implements the inheritance rules for
 * mandatory labels. MAC labels are inherited by both new subjects
 * at fork() time and for newly created objects. In both cases,
 * the child assumes the Current level of the process as the new
 * MAC label. The Current level of the process is the first parent
 * tag by convention and is thus at *parent. The second tag, the
 * MAC clearance, has no bearing on inheritance.
 * This is the SP_OBJECT_CREATE() routine for MAC.
 *
 * POSIX ACLS -- added new arguments (dac,umask) to function
 */

maccreate(process,object,parent,attrtype,dac,umask)
	tag_t		*process, *object, *parent;
	attrtype_t	attrtype;
        dac_t           *dac;
        mode_t          umask;

{

	/* Mandatory labels are inherited by processes and objects */

#ifdef DEBUG
	if(attrtype != SEC_SUBJECT)
	   if(sware_debug)
		printf("maccreate: parent %x child %x for type %d\n",
			process,object,attrtype);
#endif
	switch(attrtype) {

	/* There are two sensitivity labels on processes: the sensitivity
	 * level and the clearance.  Both are inherited at process create.  */

	   case SEC_SUBJECT:

		object[MAND_SUBJ_SL_TAG] = process[MAND_SUBJ_SL_TAG];
		object[MAND_SUBJ_CL_TAG] = process[MAND_SUBJ_CL_TAG];
		break;

	   case SEC_OBJECT:

	/* A new object normally inherits the sensitivity label of the process
	 * that created it, unless the process has the ALLOWMACACCESS privilege
	 * and its label has not yet been set.  In this case, the new object
	 * inherits its parent directory's label.  */

		if(*process == SEC_WILDCARD_TAG_VALUE
		   && (parent != (tag_t *) 0))
			object[MAND_OBJ_SL_TAG] = parent[MAND_OBJ_SL_TAG];
		else
			object[MAND_OBJ_SL_TAG] = process[MAND_OBJ_SL_TAG];
	
		break;

	   default:

		break;
	}

	/* 
	 * POSIX ACL -- return code changed to 0
	 */
	return(0);
}

/*
 * Reference count maintenance for Object Deletion
 * This is the sw_obj_delete routine for the MAC policy.
 */

macdelete(subject,object,attr_type)
tag_t *subject, *object;
attrtype_t attr_type;
{
	/* Not used in this release */

	return(1);
}


/*
 * Define the access mode bits that are relevant to the MAC policy
 */

#define	NEED_SAME(m) ((m)&(SP_MOUNTACC|SP_IOCTLACC))
#define	NEED_SDOM(m) ((m)&(SP_READACC|SP_EXECACC|SP_STATACC))
#define	NEED_ODOM(m) ((m)&(SP_WRITEACC|SP_DELETEACC|SP_KILLACC|SP_SETATTRACC))
#define	NEED_SAME_OR_PRIV(m)	((m)&(SP_CREATEACC|SP_LINKACC))

/* Mandatory Access Control Decision (SP_ACCESS()) Routine
 *
 * The subject tag pointer points to the first tag for the policy. For
 * mandatory, there are two tags: the current level and the clearance.
 * The mode determines which type of access check is made by the module.
 * If daemon check is specified, then only the cache is checked. If the
 * decision is not present a -1 is returned.
 *
 * Function returns:	0	= access allowed
 *			1	= access denied
 *		       -1	= no cache entry
 */

macaccess(subject,object,mode,daemon_check,udac)
	tag_t	*subject, *object;
	int	mode;
	int	daemon_check;
	udac_t	*udac;		/* unused by this policy */
{
	int	macdec;
	tag_t	clearance = subject[MAND_SUBJ_CL_TAG];

#ifdef DEBUG
	if(sware_debug)
		printf("macaccess: subject %x object %x mode %x\n",
			subject,object,mode);
#endif

	/*
	 * Check the access mode for relevance to the MAC policy.
	 */
	if ((NEED_SAME(mode) | NEED_SAME_OR_PRIV(mode) |
	     NEED_SDOM(mode) | NEED_ODOM(mode)) == 0)
		return 0;

	/*
	 * A process whose sensitivity level has not yet been set is
	 * denied access unless it has the ALLOWMACACCESS privilege.
	 */
	if (*subject == SEC_WILDCARD_TAG_VALUE && mode != SP_LINKACC &&
	    !privileged(SEC_ALLOWMACACCESS, EACCES)) {
		return(1);
	}

	/*
	 * If the object has a wildcard tag or the process is privileged
	 * to bypass MAC checks, then grant access unless the function
	 * is a Link, Create or Mount. These are necessary to maintain
	 * increasing tree regardless of privilege.
         */

	if ((mode & (SP_LINKACC | SP_CREATEACC | SP_MOUNTACC)) == 0) {
		if (*object == SEC_WILDCARD_TAG_VALUE ||
		    privileged(SEC_ALLOWMACACCESS, 0))
			return(0);
	} else if (*object == SEC_WILDCARD_TAG_VALUE) {
		/*
		 * We never allow mounting on or creating/linking files in
		 * directories with wildcard tags.
		 */
		return(1);
	}

	/*
	 * Handle those decisions that can be made based on the tags
	 * themselves without having to consult the decision cache or
	 * the daemon.
	 */
	if (*subject == *object)
		return 0;

	if (NEED_SAME(mode) || NEED_SDOM(mode) && NEED_ODOM(mode)) {
		/*
		 * The mode requires subject and object to
		 * be at the same level.
		 */
		return(1);
	}

	if (NEED_SAME_OR_PRIV(mode)) {
		/*
		 * The mode normally requires subject and object
		 * to be at the same level, but a privileged subject
		 * that dominates the object is allowed.  We verify
		 * the privilege here and perform the dominance check
		 * below.
		 */
		if(!privileged(SEC_ALLOWMACACCESS,EACCES)) {
			return(1);
		}

		/*
		 * If the MAC level of the process is not set, allow
		 * the action to proceed.   In the create case the new
		 * object will inherit its MAC tag from the parent
		 * directory instead of from the process.
		 */
		if(*subject == SEC_WILDCARD_TAG_VALUE)
			return(0);
	}

	/*
	 * All other cases require that we know the dominance
	 * relationship between the two tags.  Get a decision
	 * from the cache or the daemon.
	 */
	macdec = mac_getdecision(*subject,*object,daemon_check);
	if(macdec == 0) {
		if(daemon_check == 0)
			return(-1);
		else {
			return(1);
		}
	}

	/*
	 * Perform the access check based on the mode.
	 */
	if (NEED_SDOM(mode) | NEED_SAME_OR_PRIV(mode)) {
		/*
		 * The subject must dominate the object.
		 */
		if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
			return(1);
		}
		return 0;
	}
	if (NEED_ODOM(mode)) {
		/*
		 * The object must dominate the subject.
		 */
		if ((macdec & (MAC_ODOM|MAC_SAME)) == 0) {
			return(1);
		}

		/* If levels are equal, access is granted */

		if (macdec & MAC_SAME)
			return(0);

		/* Object dominates.  Check for WriteUp Privileges */

		if (privileged(SEC_WRITEUPSYSHI,0))
			return(0);

		if (privileged(SEC_WRITEUPCLEARANCE,0)) {

			if(clearance == *object)
				return(0);

			/* Now make a decision between the process's
			 * clearance and the object.        */

			macdec = mac_getdecision(clearance,*object,daemon_check);

			if(macdec == 0) {
				if(daemon_check == 0)
				    return(-1);
				else {
				    return(1);
				}
			}

			if (macdec & (MAC_SDOM|MAC_SAME))
				return(0);

		}

		return(1);
	}

	/*
	 * We should never get here since we bailed out at the top
	 * if the access mode contained no MAC-relevant bits.
	 */
	printf("macaccess: illegal mode %x\n", mode);
	panic("macaccess");
}

/*
 * MAC specific portion of the getlabel(2) system call
 * This is the SP_GETATTR() routine for MAC.
 */

macgetattr(attrtype, tag, attr)
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
		return(EFAULT);
	}

	/* Do not attempt to get IR for Wildcard tags */

	if (tag == SEC_WILDCARD_TAG_VALUE) {
		kattr.ir_length = 0;
		kattr.code = SEC_WILDCARD_TAG;
#ifdef DEBUG
		if (sware_debug)
			printf("macgetattr: WILDCARD tag\n");
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
		printf("macgetattr: for tag %x length %d buffer %x\n",
			mp->tag, kattr.ir_length, kattr.ir);
#endif

	/* Send the message to the MAC daemon */

	buff_size = sizeof *mp;
	rp = (struct spd_internal_rep *)
			spd_send_message(mp, &buff_size, macpolicy);

	/* Check for error return else copy the label to user space */

	if (rp == (struct spd_internal_rep *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
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
 * MAC specific portion of the setlabel(2) system call
 * This is the SP_SETATTR() routine for MAC.
 */

macsetattr(type, tags, tagnum, new)
	attrtype_t	type;	/* SEC_SUBJECT or SEC_OBJECT */
	tag_t		*tags;	/* tag pool of entity being regraded */
	int		tagnum;	/* policy-relative tag offset */
	register tag_t	new;	/* new tag value */
{
	register tag_t	*target;
	register tag_t	clearance = SIP->si_tag[macclrnce];
	register int	macdec;

#ifdef DEBUG
	if (sware_debug)
		printf("macsetattr: tags %x tagnum %d new value %x\n",
			tags, tagnum, new);
#endif

	if (type == SEC_SUBJECT)
		target = &tags[SUBJECT_TAG(macpolicy,tagnum)];
	else
		target = &tags[OBJECT_TAG(macpolicy,tagnum)];


	/*
	 * Wildcard tags can only be set on objects and then only
	 * by a process with the appropriate privilege.
	 */
	if (new == SEC_WILDCARD_TAG_VALUE) {
		if (type == SEC_SUBJECT) {
			return(EINVAL);
		}
		{
			if (!privileged(SEC_ALLOWMACACCESS, EPERM)) {
				return(EPERM);
			}
		}

		*target = new;
		return 0;
	}

	/* Are we setting a subject tag? */

	if (type == SEC_SUBJECT) {
		tag_t	process_sl = tags[macsubjtag];

#ifdef DEBUG
		if (sware_debug)
			printf("macsetattr: setting label on subject\n");
#endif
		switch (tagnum) {

		  case MAND_SUBJ_SL_TAG:	/* process sensitivity label */


			/* Can only do this if clearance has been set. */

			if (clearance == SEC_WILDCARD_TAG_VALUE) {
				return(EPERM);
			}

			/* Subject can change sensitivity level if it
			 * has allowmacaccess				*/

			if (*target != SEC_WILDCARD_TAG_VALUE &&
			    !privileged(SEC_ALLOWMACACCESS, EPERM)) {
				return(EPERM);
			}

			/* Quick check of process level equal to clearance */

			if (clearance == new) {
				mac_setdeflect(target, new);
				return 0;
			}

#ifdef DEBUG
			if (sware_debug)
				printf("macsetattr: clrnce/level decision\n");
#endif
			/* Clearance must dominate new process level */

			macdec = mac_getdecision(clearance, new, 1);
			if (macdec == 0) {
				return(EINVAL);
			}

			if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
				return(EPERM);
			}

			/* Set the new tag for the process and for audit */
			/* Set the MAC multilevel directory deflection name */

			mac_setdeflect(target, new);
			return 0;

		  case MAND_SUBJ_CL_TAG:	/* process clearance */


			if (clearance == SEC_WILDCARD_TAG_VALUE) {

				/* Setting the clearance for the first time */

				*target = new;
				return 0;
			}

			/* Resetting the clearance. It must be dominated by
			 * the old clearance and must dominate the current
			 * level of the process. The current level of the
			 * process must have already been set before changing
			 * clearance. */

			if (process_sl == SEC_WILDCARD_TAG_VALUE) {
				return(EINVAL);
			}

			/* If the new clearance is the same as the current
			 * clearance or is the same as the current level then
			 * set the new label without needing a decision.   */

			if (clearance == new)
				return 0;
			if (new == process_sl) {
				*target = new;
				return 0;
			}

			/* Make the necessary decision to insure that the new
			 * clearance is dominated by old clearance.	   */

			macdec = mac_getdecision(clearance, new, 1);
			if (macdec == 0) {
				return(EINVAL);
			}

			if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
				return(EPERM);
			}

			/* Check relationship between new clearance and
			 * current sensitivity level */

			macdec = mac_getdecision(new, process_sl, 1);
			if (macdec == 0) {
				return(EINVAL);
			}

			if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
				return(EPERM);
			}

			/* Whew! Set the new clearance. */

			*target = new;
			return 0;

		}
	}


	/*
	 * We are setting an object tag.
	 */

#ifdef DEBUG
	if(sware_debug)
		printf("macsetattr: setting label on object\n");
#endif

	switch (tagnum) {

	  case MAND_OBJ_SL_TAG:	/* object sensitivity label */



		/* If the tag to be modified is currently the Wildcard tag,
		 * ALLOWMACACCESS privilege is required.  Ownership of the
		 * object has already been verified by macsetattr_check(). */

		if (*target == SEC_WILDCARD_TAG_VALUE) {

			if (!privileged(SEC_ALLOWMACACCESS, EPERM)) {
				return(EPERM);
			}

			*target = new;
			return 0;
		}

		/* Enforce the relationship between old and new labels, subject
		 * to the caller's privileges. */

		macdec = mac_getdecision(new, *target, 1);
		if (macdec == 0) {
			return(EINVAL);
		}

		if (macdec & MAC_SAME)
			return 0;

		if (macdec & MAC_INCOMP) {
			if (!privileged(SEC_ALLOWMACACCESS, EPERM)) {
				return(EPERM);
			}
			*target = new;
			return 0;
		}

		if (macdec & MAC_ODOM) {

			/* If the old label dominates the new label,
			 * must have DOWNGRADE privilege.  */
#ifdef DEBUG
			if (sware_debug)
				printf("macsetattr: object dominates check\n");
#endif
			if (!privileged(SEC_ALLOWMACACCESS, 0) &&
			    !privileged(SEC_DOWNGRADE, EPERM)) {
				return(EPERM);
			}
			*target = new;
			return 0;
		}

		if (macdec & MAC_SDOM) {

			/* If the new label dominates the old label, must have
			 * either WRITEUPCLEARANCE or WRITEUPSYSHI.  */
#ifdef DEBUG
			if (sware_debug)
				printf("macsetattr: subject dominates check\n");
#endif
			if (privileged(SEC_ALLOWMACACCESS, 0) ||
			    privileged(SEC_WRITEUPSYSHI, 0)) {
				*target = new;
				return 0;
			}

			if (!privileged(SEC_WRITEUPCLEARANCE, EPERM)) {
				return(EPERM);
			}

			/* Clearance must dominate new label */

#ifdef DEBUG
			if(sware_debug)
				printf("macsetattr: clearance/tag decision\n");
#endif
			macdec = mac_getdecision(clearance, new, 1);
			if (macdec == 0) {
				return(EINVAL);
			}

			if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
				return(EPERM);
			}

			*target = new;
			return 0;
		}

		/* Should never get here */
		return(EINVAL);
	

	  default:	/* should never get here */

		return(EINVAL);
	}
}

/*
 * mac_setattr_check()-this routine performs low-level system
 * dependent checks on the objects involved in the setattr
 * system call. This is used to maintain the relationship
 * between the objects and to isolate any system dependencies
 * from the actual label setting code.
 * This is the SP_SETATTR_CHECK() routine for MAC.
 */

macsetattr_check(objtype, obj, parent, tags, tagnum, new)
	int	objtype;	/* object type of target object */
	caddr_t	obj,		/* target object */
		parent;		/* target object's parent */
	tag_t	*tags;		/* target object's tag pool */
	int	tagnum;		/* policy-relative tag number */
	tag_t	new;		/* new tag value */
{
	register struct ipc_perm	*perm;
	tag_t	*target;
	int	macdec;

#ifdef DEBUG
	if (sware_debug)
		printf("macsetattr_check: tags %x tagnum %d new value %x\n",
			tags, tagnum, new);
#endif

	/* Perform target type dependent access checks */

	switch (objtype) {

	    case OT_PROCESS:
		return 0;

	    case OT_REGULAR:
		return mac_filesetattr_check(obj, parent, tags, tagnum, new);

	    case OT_MESSAGE_QUEUE:
		perm = &((struct msqid_ds *) obj)->msg_perm;
		goto ipc_owner_check;

	    case OT_SEMAPHORE:
		perm = &((struct semid_ds *) obj)->sem_perm;
		goto ipc_owner_check;

	    case OT_SHARED_MEMORY:
		/*
		 * Changing the sensitivity label of a shared memory segment
                 * that is in use is not permitted since access mediation
                 * only takes place at allocation time.  Consequently, once
                 * attached to any process, the level must remain unchanged.
                 * Technically, the level could be changed if the reference
                 * count is one and the changer is the same process that has
                 * the segment attached.
                 * PORT NOTE:  Some ports artificially bump the reference
                 * count for convenience of VM code.  Beware if you change
                 * EBUSY code.
		 */
#ifdef DEBUG
		if (sware_debug)
		    printf("macsetattr_check: shared mem ref count check\n");
#endif
		if (shm_active(obj)) { /* see include_sec */
			return(EBUSY);
		}
		perm = &((struct shmid_internal *)obj)->s.shm_perm;

ipc_owner_check:

#ifdef DEBUG
		if (sware_debug)
			printf("macsetattr_check: ipc %x uid %d gid %d\n",
				obj, perm->uid, perm->gid);
#endif

		if (!sec_owner(perm->uid, perm->cuid)) {
			return(EPERM);
		}
		return 0;

	    default:
		return 0;
	}
}

/*
 * Map a MAC internal representation to a tag
 * This is the SP_MAPTAG() routine for MAC.
 */

macmaptag(attrtype, tagnum, attr, udac, new)
	attrtype_t	attrtype;
	int		tagnum;
	attr_t		*attr;
	udac_t		*udac;
	tag_t		*new;
{
	return sp_maptag(macpolicy, attrtype, tagnum, attr, udac, new,
				SEC_IR_USER);
}



/*-----------------------------------------------------------------------*/
/*	Routines not part of the switch called entries follow this	 */
/*-----------------------------------------------------------------------*/


/*
 * mac_getdecision()-make a decision between the two labels based on
 * the presented tags. Checks the cache first and then prepares a
 * message for the daemon if not in the cache. The cache is loaded
 * with the decision upon return unless an error occurred.
 *
 * Daemon check = 0	Only consult decision cache
 * 	     = 1	Consult policy daemon if needed
 */

mac_getdecision(subject, object, daemon_check)
	register tag_t	subject, object;
	int	daemon_check;
{
	struct spd_make_decision	*mp;
	struct spd_decision		*rp;
	int		bufsize, ret;
	mcache_dec_t	dec;

	/* Handle those decisions that can be made based on the
	 * tag values themselves.  */

	if (subject == object)
		return MAC_SAME;

	/*
	 * These tests are invalid when information labels are involved
	 * since a non-syshi information label may actually contain a
	 * syshi sensitivity label, and similarly for syslo.
	 */
	if (subject == SEC_MAC_SYSHI_TAG || object == SEC_MAC_SYSLO_TAG)
		return MAC_SDOM;
	
	if (subject == SEC_MAC_SYSLO_TAG || object == SEC_MAC_SYSHI_TAG)
		return MAC_ODOM;

	/* Check for subject and object cache entry */

	if (dcache_lookup(macpolicy, subject, object, &dec, sizeof dec)) {
		if (dec.decision.subj_dom)
			ret = MAC_SDOM;
		else if (dec.decision.obj_dom)
			ret = MAC_ODOM;
		else if (dec.decision.same)
			ret = MAC_SAME;
		else if (dec.decision.incomp)
			ret = MAC_INCOMP;
		else
			ret = 0;
		return ret;
	}

	/* No cache entry found. Check for entry with tags in reverse order. */

	if (dcache_lookup(macpolicy, object, subject, &dec, sizeof dec)) {
		if (dec.decision.subj_dom)
			ret = MAC_ODOM;		/* invert relationship */
		else if (dec.decision.obj_dom)
			ret = MAC_SDOM;		/* invert relationship */
		else if (dec.decision.incomp)
			ret = MAC_INCOMP;
		else if (dec.decision.same)
			ret = MAC_SAME;
		else
			ret = 0;
		return ret;
	}

	/* No cache entry for tags in either order.  Stop here unless
	 * caller wants us to consult the daemon.  */

	if (daemon_check == 0)
		return 0;

	/* Request the decision from the MAC daemon */

	bufsize = sizeof *mp + sizeof dec;
	mp = (struct spd_make_decision *) spd_allocate(bufsize);
	if (mp == NULL)
		return 0;

	mp->mhdr.msg_type = SPD_MAKE_DECISION;
	mp->mhdr.mh_flags = 0;
	mp->mhdr.error_code = 0;

	mp->subject = subject;
	mp->object = object;

	rp = (struct spd_decision *) spd_send_message(mp, &bufsize, macpolicy);
	if (rp == (struct spd_decision *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return 0;
	}

	/* Make a local copy of the decision and release the reply message */

	dec = *(mcache_dec_t *) &rp[1];
	spd_deallocate(rp);

	/* Load the decision into the cache */

	dcache_insert(macpolicy, subject, object, (char *) &dec, sizeof dec);

	/* Construct the return value before releasing the buffer */

	if (dec.decision.subj_dom)
		ret = MAC_SDOM;
	else if (dec.decision.obj_dom)
		ret = MAC_ODOM;
	else if (dec.decision.incomp)
		ret = MAC_INCOMP;
	else if (dec.decision.same)
		ret = MAC_SAME;
	else
		ret = 0;

	return ret;
}

/*
 * Map an internal representation of an SL or IL to a tag.
 * The ir is in kernel space.
 */
macilb_irtotag(ir, ir_length, tag)
	caddr_t	ir;
	int	ir_length;
	tag_t	*tag;
{
	register struct spd_map_tag *mp;
	register struct spd_set_tag *rp;
	int size;

	if (ir_length < 0 || ir_length > SEC_MAX_IR_SIZE)
		return EINVAL;

	size = sizeof *mp + ir_length;
	mp = (struct spd_map_tag *) spd_allocate(size);
	if (mp == NULL)
		return EIO;

	mp->mhdr.msg_type = SPD_MAP_TAG;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;

	mp->ir_type = SEC_OBJECT;
	bzero(&mp->unixdac, sizeof(udac_t));

	mp->ir.ir_length = ir_length;

	if (ir_length)
		bcopy(ir, (caddr_t) &mp[1], ir_length);

	rp = (struct spd_set_tag *) spd_send_message(mp, &size, macpolicy);

	/* Return error if the mapping request did not succeed */

	if (rp == (struct spd_set_tag *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return EIO;
	}

	*tag = rp->tag;
	spd_deallocate(rp);
	return 0;
}

/*
 * Convert a tag to an internal representation and store it in the
 * supplied ir buffer.  The amount of space in the buffer is given
 * by the initial value of *ir_size, which is modified upon successful
 * return to indicate the actual size of the stored ir.  If the ir
 * is longer than the supplied buffer, an error is returned.
 */
macilb_tagtoir(tag, ir, ir_size)
	register tag_t *tag;
	caddr_t ir;
	register int *ir_size;
{
	register struct spd_get_attribute *mp;
	register struct spd_internal_rep *rp;
	int length;

	if (*tag == SEC_WILDCARD_TAG_VALUE) {
		*ir_size = 0;
		return 0;
	}

	mp = (struct spd_get_attribute *) spd_allocate(sizeof *mp);
	if (mp == NULL)
		return EIO;

	mp->mhdr.msg_type = SPD_GET_ATTRIBUTE;
	mp->mhdr.error_code = 0;
	mp->mhdr.mh_flags = 0;

	mp->tag = *tag;

	length = sizeof *mp;
	rp = (struct spd_internal_rep *)
		spd_send_message(mp, &length, macpolicy);
	if (rp == (struct spd_internal_rep *) 0 || rp->mhdr.error_code) {
		if (rp)
			spd_deallocate(rp);
		return EIO;
	}

	if (*ir_size < rp->ir.ir_length) {
		spd_deallocate(rp);
		return EMSGSIZE;
	}

	if ((*ir_size = rp->ir.ir_length) > 0)
		bcopy(&rp[1], ir, rp->ir.ir_length);

	spd_deallocate(rp);
	return 0;
}


/*
 * mac_setdeflect()-convert the specified tag into a directory name
 * and store it into the u area for use by the multilevel directory
 * code.  This is called only from macsetattr(), and is the only place
 * where si_pslevel is changed.
 * ASSERT: Changing tags, etc. only of calling process SIP.
 * MP Note:
 * Changes to si_diversion, si_pslevel, and SIP tag
 * MUST be atomic with respect to each other.
 */

mac_setdeflect(target, tag)
tag_t *target;
register tag_t tag;
{
	register char *cp;

	cp = SIP->si_diversion + sizeof SIP->si_diversion;
	*--cp = '\0';
	SIP_ATTR_LOCK();
	*target = tag;

	SIP->si_pslevel = tag;

	do {
		*--cp = (tag % 10) + '0';
		tag /= 10;
	} while (cp > SIP->si_diversion);

	bcopy(MAC_STRING, SIP->si_diversion, sizeof MAC_STRING - 1);
	SIP_ATTR_UNLOCK();
}

/*
 * mac_audittag()-audit the Mandatory tag from the specified object
 * tag pool since the operation is security relevant and the security
 * level must be recorded. This is called from an audit stub to
 * isolate the call permitting stub replacement and also serves to
 * isolate the MAC policy from use outside the policy switch.
 */

mac_audittag(tags)
tag_t *tags;
{

	return(0);
}

/*
 * mac_filesetattr_check()-file system architecture-specific routine
 * that makes the appropriate owner, reference count, and empty
 * directory checks:
 *   1.  can't change the sensitivity level of "."
 *   2a. On IL systems, must have mand and discr write access to change label.
 *       and, if the new label does not dominate the old one, the process
 *       must own the object
 *   2b. On non-IL systems, process must own the object to change its label.
 *   3.  Can't change the label on an object if its reference count > 1.
 *       Exception are sockets and char specials, which may be 2.
 *   4.  Can't set SL of children of multilevel directory.
 *   5.  Can't set wildcard SL tag on a directory.
 *   6.  Can't change SL of non-empty directory.
 *   7.  Can't downgrade or set incomparable a file that has more than one link.
 *   8.  Must maintain the increasing tree.
 *
 * Following are the three reference implementations.
 */


mac_filesetattr_check(vp, dvp, tags, tagnum, new)
	register struct vnode	*vp;	/* vnode being regraded */
	register struct vnode	*dvp;	/* parent of vnode being regraded */
	tag_t			*tags;	/* tag pool of vnode being regraded */
	int			tagnum;	/* policy-relative tag offset */
	tag_t			new;	/* new tag value */
{
	struct vattr	vattr;
	struct vsecattr	vsattr;
	tag_t		*target;
	int		macdec;
	int		error;

#ifdef DEBUG
	if (sware_debug)
		printf("mac_filesetattr_check: vp %x\n", vp);
#endif

	target = &tags[OBJECT_TAG(macpolicy, tagnum)];

	/*
	 * A chlevel "." is not permitted because ip and dp are
	 * the same inode. A relationship between the two to check
	 * for increasing tree is not possible.
	 */

	if (tagnum == MAND_OBJ_SL_TAG && vp == dvp) {
		return(EINVAL);
	}

	/*
	 * XXX Is this really needed ?  Current callers seem to
	 * check for this, but it makes sense to do it here in case
	 * we're called from other places some day.
	 */

	if (!is_secure_filesystem(vp))
		return(EINVAL);

	/* Retrieve the attributes for ownership checks below */

	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error) {
		return(error);
	}

	/* Must be the owner of the object to change label */

	if (!sec_owner(vattr.va_uid, vattr.va_uid)) {
		return(EPERM);
	}

	/*
	 * If the reference count on the object is greater than 1
	 * then the object is in use by someone else.  Access to
	 * a file for read/write is controlled at open time and
	 * not on each operation.  Consequently, once it is open,
	 * the level must remain unchanged.  Exceptions are made
	 * for terminal regrade for the benefit of getty and
	 * initcond, and for sockets for the benefit of multilevel
	 * servers.  If the following conditions hold, the regrade
	 * is allowed:
	 *
	 *	1. ALLOWMACACCESS is in effect
	 *	2. Reference count = 2 (one for getty, one for chslabel)
	 *	3. The inode is a character special device or a socket
	 */

#ifdef DEBUG
	if (sware_debug)
		printf("mac_filesetattr_check: reference count check\n");
#endif

	/*
	 * For BSD 4.4 vnode-based kernels, there are two reference counts
	 * in the vnode.
	 */
 
	if (vp->v_usecount > 1)
		if (vp->v_usecount != 2 ||
		    ((!IS_CHAR_DEV(vp)) && (!IS_SOCKET(vp))) ||
		    !privileged(SEC_ALLOWMACACCESS, 0)) {
			return(EBUSY);
		}
 
	/*
	 * The sensitivity level of a diversion (MLD) directory
         * cannot be changed since the tag/SL relationship must
         * be maintained. Do not use the parent inode to detect
         * an MLD since the parent points to the process' notion
         * of the parent directory, namely the parent of the
         * actual MLD itself. This inode will not have the MLD
         * flag set. Thus, check only the current ip as the child.
	 * XXX Possible race here in MP world
	 */

	if (dvp->v_flag & VMLD) {
		return(EINVAL);
	}

	/* If target is directory, disallow a wildcard tag and
	 * ensure that the directory is empty.   */

	if (IS_DIRECTORY(vp)) {
		if (new == SEC_WILDCARD_TAG_VALUE) {
			return(EINVAL);
		}
#ifdef DEBUG
		if (sware_debug)
		    printf("mac_filesetattr_check: empty directory check\n");
#endif
		if (!(VOP_DIREMPTY(vp, dvp, u.u_cred, error))) {
			return(EINVAL);
		}
	}

	/*
	 * If the target is not a directory and has more than 1 link,
	 * the operation must be an upgrade, since we have no way to
	 * check the relationship between the new tag and those of
	 * the directories containing the additional links.
	 */

	else {
		if (new == SEC_WILDCARD_TAG_VALUE)
			return 0;

		if (vattr.va_nlink > 1) {
#ifdef DEBUG
			if (sware_debug)
			    printf("macsetattr_check: file link count check\n");
#endif
			macdec = mac_getdecision(new, *target, 1);
			if (macdec == 0) {
				return(EINVAL);
			}

			if (macdec & (MAC_ODOM|MAC_INCOMP)) {
				return(EINVAL);
			}
		}
	}

	/*
	 * get sensitivity level tag from the parent directory
	 */

	vsattr.vsa_valid = VSA_TAG;
	vsattr.vsa_policy = macpolicy;
	vsattr.vsa_tagnum = MAND_OBJ_SL_TAG;
	if (VOP_GETSECATTR(dvp, &vsattr, u.u_cred, error)) {
		return(error);
	}

#ifdef DEBUG
	if (sware_debug)
		printf("mac_filesetattr_check: parent dvp %x tag %x\n",
			dvp, vsattr.vsa_tag);
#endif

	if (vsattr.vsa_tag == SEC_WILDCARD_TAG_VALUE) {
		return(EINVAL);
	}

	/* The new label must dominate the parent directory */
#ifdef DEBUG
	if (sware_debug)
		printf("macsetattr_check: getting tag/dir decision\n");
#endif
	macdec = mac_getdecision(new, vsattr.vsa_tag, 1);
	if (macdec == 0) {
		return(EINVAL);
	}

	if ((macdec & (MAC_SDOM|MAC_SAME)) == 0) {
		return(EINVAL);
	}

	return 0;
}

 /*
  * POSIX ACLS
  *
  * Initialize the MAC tag associated with a multilevel child directory
  * to the sensitivity label of the invoking process.
  */

sp_init_mld_child(tags)
tag_t   *tags;
{
        tags[macobjtag] = SIP->si_tag[macsubjtag];
}
#endif /* SEC_MAC */
