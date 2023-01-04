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
static char	*sccsid = "@(#)$RCSfile: sec_mld.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/06/03 14:48:26 $";
#endif 

/*
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc. and
 * should be treated as Confidential.
 */

/* #ident "@(#)sec_mld.c	3.1 10:00:54 6/7/90 SecureWare" */
/*
 * Based on:	@(#)sec_mld.c	2.10.2.3 12:15:30 12/22/89
 */

#include <sec/include_sec>

#if SEC_MAC

#ifdef _OSF_SOURCE
/*
 * mld_traverse
 */

mld_traverse(ndp, errp)
	register struct nameidata	*ndp;
	int				*errp;
{
	register struct vnode	*dp = ndp->ni_vp;
	register int		error;
	register int		saveop;
	register char		*savepn;

	/*
	 * If the found vnode is not a multilevel directory or if the
	 * caller is privileged, there is nothing to do.
	 */
	if ((!IS_DIRECTORY(dp)) || (dp->v_flag & VMLD) == 0 ||
	    privileged(SEC_MULTILEVELDIR, 0))
		return SEC_MLD_PASS;

	/*
	 * The found vnode is a multilevel directory and the bypass
	 * privilege is not in effect.  If the found component name
	 * is "..", return a special code to get our caller to repeat
	 * the lookup in the mld itself.
	 */
	if (ndp->ni_isdotdot)
		return SEC_MLD_DOTDOT;
	
	/*
	 * Look for the appropriate subdirectory, creating it if
	 * necessary.  We first save the current namei operation
	 * and pathname tail pointer so that our caller can translate
	 * any components that remain after the mld.  We then dispose
	 * of the vnode for the mld's parent.
	 * If we are at the end of the
	 * pathname and a delete operation is in progress (meaning that
	 * an attempt is being made to remove the mld itself) set the
	 * operation for the subdirectory lookup to DELETE so that the
	 * information needed to delete the subdirectory will be saved.
	 * Otherwise we set up for a create operation in case the
	 * subdirectory does not exist.
	 */
	saveop = ndp->ni_nameiop;
	savepn = ndp->ni_next;
	vrele(ndp->ni_dvp);

	if (SIP->si_diversion[0] == '\0') {
		/*
		 * Don't try to divert a process with a wildcard MAC label.
		 */
		ndp->ni_dvp = dp;
		ndp->ni_vp = NULL;
		*errp = ENOENT;
		return SEC_MLD_ERROR;
	}

	if ((saveop & OPFLAG) == DELETE && *ndp->ni_next == '\0')
		ndp->ni_nameiop = DELETE | WANTPARENT;
	else
		ndp->ni_nameiop = CREATE | MLDCREATE | WANTPARENT;
	ndp->ni_next = "";
	/* XXX Check locking of si_diversion and ni_ptr */
	ndp->ni_ptr = SIP->si_diversion;
	ndp->ni_dent.d_namlen = ndp->ni_namelen = sizeof SIP->si_diversion - 1;
	SIP_ATTR_LOCK();
	bcopy(SIP->si_diversion, ndp->ni_dent.d_name,
	      sizeof SIP->si_diversion);
	SIP_ATTR_UNLOCK();
	VOP_LOOKUP(dp, ndp, error);
	if (error == 0) {
		/*
		 * The subdirectory already exists.
		 */
	} else if (error == ENOENT && (ndp->ni_nameiop & OPFLAG) == CREATE) {
		/*
		 * The subdirectory does not exist, and the operation
		 * calls for us to create it. Make sure the filesystem
		 * is writable.
		 */
		if (is_read_only_filesystem(dp)) {
			error = EROFS;
		} else {
			/*
			 * VOP_MKDIR does a vput on ndp->ni_dvp (which now
			 * contains a copy of dp, courtesy of VOP_LOOKUP)
			 * regardless of the outcome.  But our caller expects
			 * it to at least be referenced and possibly locked.
			 * We add a reference here to prevent the vnode from
			 * disappearing out from under us.
			 * XXX -- I believe that this assumption is still true,
			 * although the vnode is no longer locked.
			 */
			VREF(dp);
			VOP_MKDIR(ndp, (struct vattr *) 0, error);

			/*
			 * XXX MP Note: Race with VOP_MKDIR by another
			 * thread here ? Should be able to deal with
			 * EEXIST error...
			 */
		}
	}
	ndp->ni_nameiop = saveop;
	ndp->ni_next = savepn;
	return (*errp = error) ? SEC_MLD_ERROR : SEC_MLD_DIVERT;
}

#endif /* _OSF_SOURCE */

/*
 * mld_dominate
 *
 * Determine if the calling process's sensitivity level dominates the
 * sensitivity level of the multilevel directory whose tag pool is
 * passed as an argument.  This is used to enforce increasing tree
 * during the creation of a mld subdirectory.  It is called only from
 * ufs_mkdir (or equivalent).
 */

mld_dominate(tags)
	tag_t	*tags;
{
	int		macdec;
	extern int	macpolicy, macsubjtag, macobjtag, mac_getdecision();

	if (macpolicy == -1)
		return(0);

	/*
	 * Get the access decision between the process and the directory.
	 * The subject must dominate or the levels must be the same.
	 * XXX MP Note: possible race with process slevel change
	 */

	macdec = mac_getdecision(SIP->si_tag[macsubjtag], tags[macobjtag], 1);

	return (macdec & (MAC_SDOM | MAC_SAME)) != 0;
}

#endif /* SEC_MAC */
