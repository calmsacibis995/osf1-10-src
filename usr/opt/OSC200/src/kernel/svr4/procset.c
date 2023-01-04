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
static char *rcsid = "@(#)$RCSfile: procset.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/08/03 18:42:35 $";
#endif

#include <sys/user.h>
#include <sys/proc.h>
#include <sys/procset.h>
#include <sys/priocntl.h>

/*
 * look in newproc(), where the pid is selected, and where this
 * max is checked for -- pid is always wrapped to < 30000 by
 * newproc().
 *
 * Could also use the #define PID_MAX, which is set to SHRT_MAX.
 */
#define REAL_PIDMAX 30000

/*
 * validated_selection_criterion
 *	Make sure the procset has valid values for right, left
 *	set and operation.
 *
 * Returns:
 *	KERN_SUCCESS - if validation passes.
 *	EINVAL - if validation fails.
 */
int
validated_selection_criterion(struct proc *p, procset_t *psp)
{
	idtype_t idtype;
	id_t *idp;

	/*
	 * For each set, idtype and id are valid.
	 * First do left set, then right set.
	 */
	idtype = psp->p_lidtype;
	idp = &psp->p_lid;
	while (1) {
		if (*idp == P_MYID) {
			switch (idtype)	{
			default:
				return (EINVAL);
			case P_PID:
				*idp = (id_t)(p->p_pid);
				break;
			case P_PPID:
				*idp = (id_t)(p->p_ppid);
				break;
			case P_PGID:
				*idp = (id_t)(p->p_pgid);
				break;
			case P_SID:
				*idp = (id_t)(p->p_session->s_leader->p_pid);
				break;
			case P_CID:
				*idp = class_id(p);
				if (*idp == -1)
					return(EINVAL);
				break;
			case P_UID:
				*idp = (id_t)(p->p_svuid);
				break;
			case P_GID:
				*idp = (id_t)(p->p_svgid);
				break;
			case P_ALL:
				break;
			}
		} else {
			/*
			 * We only need to validate if we have set the
			 * parameters ourself.
			 */
			switch (idtype) {
			default:
				return EINVAL;
				break;
			case P_PID:
			case P_PPID:
			case P_PGID:
			case P_SID:
				/*
				 * In truth, the selected() function
				 * below will detect this case just
				 * fine. But we can safely return
				 * ESRCH here, because this cannot
				 * be a real PID. (This code used to
				 * return EINVAL, which is clearly
				 * non-SVID3.
				 */
				if ((*idp < 0) || (*idp > REAL_PIDMAX))
					return ESRCH;
				break;
			case P_CID:
				if (!class_valid( *idp ))
					return EINVAL;
				break;
			case P_UID:
			case P_GID:
				if (((int)*idp < 0) || (*idp > (uid_t)UID_MAX))
					return EINVAL;
				break;
			case P_ALL:
				break;
			}

			if (idp == &psp->p_rid)
				break;

		} /* end if-else */

		idtype = psp->p_ridtype;
		idp = &psp->p_rid;
	}

	/* check validity of procset operation type. */
	switch (psp->p_op) {
	default:
		return EINVAL;
	case POP_DIFF:
	case POP_AND:
	case POP_OR:
	case POP_XOR:
		/* ok */
		break;
	}
	return KERN_SUCCESS;
}

#define SYSCLASS(p) (((p)->thread)->vm_privilege && ((p)->p_pid != 1))

/*
 * selected
 *	Determine whether the process p is a member of the
 *	complex set defined by procset (psp).  If the process
 *	is a system process, this test always fails.
 *
 * Note: it is assumed that the process passed to this routine is 
 * a valid, running process.  idtype being valid is a precondition 
 * for this routine.  p_op being valid is a precondition for this
 * routine.  All procset values must be valid or this routine may
 * panic().
 *
 * Returns:
 *	1 - (non-zero) - if process is in set.
 *	0 - if process is not in set.
 */
int
selected(p, psp)
    struct proc *p;
    procset_t *psp;
{
	boolean_t *in_set, inleftset, inrightset;
	idtype_t idtype;
	id_t id;

	/*
	 * If this is a system process, return FALSE.
	 *
	 * SSYS means swapper or pager process.  What else do
	 * we need to look for?
	 */
	if (p->p_pid == 0 || p->p_flag&SSYS || SYSCLASS(p))
		return 0;

	/*
	 * For each set, determine if process is in set.
	 * First do left set, then right set
	 */
	in_set = &inleftset;
	idtype=psp->p_lidtype;
	id=psp->p_lid;

	while(1) {
		switch (idtype) {
		default:
			/*
			 * idtype being valid is a precondition for this
			 * routine.  Panic.
			 */
			panic("selected");
			break;
		case P_PID:
			*in_set = ((pid_t)id == p->p_pid);
			break;
		case P_PPID:
			*in_set = ((pid_t)id == p->p_ppid);
			break;
		case P_PGID:
			*in_set = ((pid_t)id == p->p_pgid);
			break;
		case P_SID:
			/* some processes may not be in a session */
    			*in_set = p->p_session->s_leader ?
				((pid_t)id == p->p_session->s_leader->p_pid) :
					0;
			break;
		case P_CID:
			*in_set = is_class_member(p, id);
			break;
		case P_UID:
			*in_set = ((uid_t)id == p->p_svuid);
			break;
		case P_GID:
			*in_set = ((gid_t)id == p->p_svgid);
			break;

		case P_ALL:
			*in_set = 1;
			break;
		}

		if (in_set == &inrightset)
			break;
		in_set = &inrightset;
		idtype = psp->p_ridtype;
		id=psp->p_rid;
	}


	/* apply operation */
	switch (psp->p_op) {
	case POP_DIFF:
		return(inleftset & ~inrightset);
	case POP_AND:
		return(inleftset & inrightset);
	case POP_OR:
		return(inleftset | inrightset);
	case POP_XOR:
		return(inleftset ^ inrightset);
	}

	/*
	 * p_op being valid is a precondition for this
	 * routine.  Panic.
	 */
	panic("selected");
	return(0);
}
