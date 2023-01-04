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


#ifndef _SVR4_SYS_PROCSET_H
#define _SVR4_SYS_PROCSET_H

#include <sys/types.h>


/*
 * This is the set of operators which can be applied to two sets of
 * processes to create a new set of processes.
 */
typedef enum idop {
	POP_DIFF,	/* Set difference.  The processes which are in
			 * the left set and not in the right set.
			 */

	POP_AND,	/* Set intersection.  The processes which are
			 * in both the left and right sets.
			 */

	POP_OR,		/* Set union.  The processes which are in
			 * either the left or the right sets (or
			 * both).
			 */

	POP_XOR		/* Set exclusive or.  The processes which are
			 * in either the left or right sets but not in
			 * both.
			 */
} idop_t;



/*
 * This is the set of legal values for an identifier type.  An idtype
 * and id together define a set of processes.
 */
typedef enum idtype {
	P_PID,	/* process id type */
	P_PPID,	/* parent process id type */
	P_PGID,	/* process group (job control group) id type */
	P_SID,	/* session id type */
	P_CID,	/* scheduling class id type */
	P_UID,	/* user id type */
	P_GID,	/* group id type */
	P_ALL	/* all processes */
} idtype_t;



/*
 * This structure is used to create a set of processes.  The set is
 * created by applying the operator to the two sets of processes
 * defined by this structure
 */
typedef struct procset {
	idop_t p_op;		/* The operator to apply to the
				 * two sets (left and right) described
				 * below.
				 */

	idtype_t p_lidtype;	/* The id type for the left set. */
	id_t p_lid;		/* The id for the left set. */

	idtype_t p_ridtype;	/* The id type of for right set. */
	id_t p_rid;		/* The id of the right set. */
} procset_t;



/*
 * Use this macro to initialize a procset_t structure.
 */
#define	setprocset(psp, op, ltype, lid, rtype, rid) \
			(psp)->p_op		= (op); \
			(psp)->p_lidtype	= (ltype); \
			(psp)->p_lid		= (lid); \
			(psp)->p_ridtype	= (rtype); \
			(psp)->p_rid		= (rid);


#define P_INITPID  1
#define P_INITUID  0
#define P_INITPGID 0






#endif	/* _SVR4_SYS_PROCSET_H */
