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
 *	@(#)$RCSfile: lockf.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/11 16:53:03 $
 */
/*	@(#)lockf.h	1.2 90/11/09 4.1NFSSRC Copyr 1990 SMI */

#ifndef _ufs_lockf_h
#define	_ufs_lockf_h

/* constants and structures for the locking code ... */

/*
 * Sized to be MAXPROCESSes which actually gives us some spare slots
 * because as many processes (system types) won't ask for file locks
 */

#ifdef KERNEL
#ifndef MAXPROCESS
#define	MAXPROCESS 135
#endif
#else
#define	MAXPROCESS 135
#endif

#ifndef NULL
#define	NULL	0
#endif
struct lock_ref {
	struct lock_ref  *Next,		/* Link list pointers		   */
			 *Prev;
	struct data_lock *ThisLock,	/* Lock this reference refers too  */
			 *Conflict;	/* Lock in conflict with this one  */
	int		 Type;		/* BLOCKING, BLOCKED, or FREE	   */
	};

struct data_lock {
	struct data_lock *Next,		/* Next lock in the list	   */
			*Prev,		/* Previous Lock in the list	   */
			*NextProc;	/* Link list of process' locks	   */
	struct process_locks *MyProc;	/* Points to process lock list	   */
	struct lock_ref  *Is_Blocking,	/* Is Blocking list		   */
			*Blocked_By;	/* Blocked by list NULL == ACTIVE  */
	struct reclock	*req;		/* A back pointer to the reclock   */
	int		base,		/* Region description.		   */
			length,		/* Length of lock		   */
			type,		/* EXCLUSIVE or SHARED		   */
			granted,	/* The granted flag		   */
			color,		/* Used during deadlock search	   */
			LockID,		/* ID of this lock		   */
			system,		/* System process 'pid' is on..    */
			pid,		/* "Owner" of lock.		   */
			class;		/* Class of lock (FILE, IO, NET)   */
	int		rsys,		/* System process 'rpid' is on..   */
			rpid;		/* Remote process identifier	   */
	};

/* process lock structure holds locks owned by a given process */
struct process_locks {
	int		pid;
	struct process_locks *next;
	struct data_lock	*lox;
	};


/* pointed to by the vnode */
struct lock_list {
	struct data_lock *exclusive,	/* The exclusive list		   */
			 *shared,	/* The shared list		   */
			 *pending;	/* Those that want to get on	   */
	};

/* structure that contains list of locks to be granted */

#define GRANT_LOCK_FLAG		0xf     /* flag in eld.l_xxx to signify   */
					/* grant locks need to be granted */
					/* by lockmgr			  */

#ifdef KERNEL
#define MAX_GRANT_LOCKS         52

struct grant_lock {
	struct data_lock *grant_lock_list[MAX_GRANT_LOCKS];
	struct grant_lock *next;
	};
#endif

/* Reference 'types' also BLOCKING below is used */
#define	FREE	0
#define	BLOCKED 1

#define	EXCLUSIVE	1	/* Lock is an Exclusive lock		*/
#define	BLOCKING	2	/* Block process for this lock. 	*/

/* Lock classes, determines how they are treated when they are unlocked. */
#define	FILE_LOCK	0	/* Generic class no special treatment	*/
#define	IO_LOCK		1	/* Indicates I/O waiting to complete 	*/
#define	LOCKMGR		2	/* Indicates lock manager lock 		*/

/* These defines are used with lock structures to determine various things */
#define	LOCK_TO_EOF	-1
#define	END(l)	(((l)->length == LOCK_TO_EOF || ((l)->length == 0)) ? \
	LOCK_TO_EOF : (l)->base + (l)->length - 1)
#define	ACTIVE(l)	((l)->Blocked_By == NULL)

/*
 * Determine if locks intersect.  Lock intersection is computed
 * by determining:
 *
 *	If			Then they intersect if
 *	--			----------------------
 * 	Both not to-EOF locks	bases and ends are mutually out of range
 *	One only is EOF lock	If the EOF-lock base is within end of
 *				non-EOF lock
 *	Both are EOF locks	Always
 */
#define	TOUCHING(a, b) \
	((END(a) >= 0 && END(b) >= 0 && \
	(!((END(a) < (b)->base) || ((a)->base > END(b))))) || \
	(END(a) < 0 && END(b) >= 0 && END(b) >= (a)->base) || \
	(END(a) >= 0 && END(b) < 0 && END(a) >= (b)->base) || \
	(END(a) < 0 && END(b) < 0))

/* Is TRUE if A and B are adjacent */
#define	ADJACENT(a, b) ((END(a)+1 == (b)->base) || ((a)->base == END(b)+1))

/* Is TRUE if a is completely contained within b */
#define	WITHIN(a, b) \
	(((a)->base >= (b)->base) && \
	((END(a) >= 0 && END(b) >= 0 && END(a) <= END(b)) || (END(b) < 0) || \
	((b)->length == 0x7fffffff)))

/* Is TRUE if a and b are owned by the same process... (even remotely) */
#ifdef KERNEL
#define	SAMEOWNER(a, b)  (((a)->pid == (b)->pid) && \
			((a)->rpid == (b)->rpid) && \
			((a)->rsys == (b)->rsys))
#else
#define	SAMEOWNER(a, b)	(((a)->pid == (b)->pid) && \
			((a)->rpid == (b)->rpid))
#endif

#ifdef KERNEL
#define	LOCKS(v)	((struct lock_list *)((v)->v_filocks))
#define	Exclusive	((LOCKS(vp)) ? LOCKS(vp)->exclusive : NULL)
#define	Shared		((LOCKS(vp)) ? LOCKS(vp)->shared : NULL)
#define	Pending		((LOCKS(vp)) ? LOCKS(vp)->pending : NULL)

#endif

#ifndef KERNEL
struct	data_lock	*free_locks;

#define	FREELOCK(l)	{ if ((l)->LockID < 0) { (l)->pid = 0; \
			    (l)->Next = free_locks; \
			    (l)->NextProc = 0; \
			    free_locks = (l); \
			  }\
 			}
#endif

#endif /*!_ufs_lockf_h*/
