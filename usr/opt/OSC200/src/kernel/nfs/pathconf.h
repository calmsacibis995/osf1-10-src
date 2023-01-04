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
 * @(#)$RCSfile: pathconf.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 17:55:54 $
 */
/*	@(#)pathconf.h	1.1 90/07/18 4.1NFSSRC 1.9 89/06/26 SMI;*/	

#ifndef	__sys_pathconf_h
#define	__sys_pathconf_h

/*
 * POSIX pathconf information
 *
 * static pathconf kludge notes:
 *	For NFSv2 servers, we've added a vop (vop_cntl) to dig out pathconf
 *	information.  The mount program asked for the information from
 *	a remote mountd daemon.  If it gets it, it passes the info
 *	down in a new args field.  The info is passed in the struct below
 *	in nfsargs.pathconf.  There's a new NFS mount flag so that you know
 *	this is happening.  NFS stores the information locally; when a
 *	pathconf request is made, the request is intercepted at the client
 *	and the information is retrieved from the struct passed down by
 *	mount. It's a kludge that will go away as soon
 *	as we can ask the nfs protocol these sorts of questions (NFSr3).
 *	All code is noted by "static pathconf kludge" comments and is
 *	restricted to nfs code in the kernel.
 */

#define	_BITS		(8 * sizeof(short))
#define _PC_LAST	9
#define	_PC_N		((_PC_LAST + _BITS - 1) / _BITS)
#define	_PC_ISSET(n, a)	(a[(n) / _BITS] & (1 << ((n) % _BITS)))
#define	_PC_SET(n, a)	(a[(n) / _BITS] |= (1 << ((n) % _BITS)))
#define	_PC_ERROR	0

struct	pathcnf {
	/*
	 * pathconf() information
	 */
	int		pc_link_max;	/* max links allowed */
	short		pc_max_canon;	/* max line len for a tty */
	short		pc_max_input;	/* input a tty can eat all once */
	short		pc_name_max;	/* max file name length (dir entry) */
	short		pc_path_max;	/* path name len (/x/y/z/...) */
	short		pc_pipe_buf;	/* size of a pipe (bytes) */
	u_char		pc_vdisable;	/* safe char to turn off c_cc[i] */
	char		pc_xxx;		/* alignment padding; cc_t == char */
	short		pc_mask[_PC_N];	/* see below */
#ifdef	KERNEL
	short		pc_refcnt;	/* number of mounts that use this */
	struct pathcnf	*pc_next;	/* linked list */
#endif
};

/*
 * pc_mask is used to encode either
 *	a) boolean values (for chown_restricted and no_trunc)
 *	b) errno on/off (for link, canon, input, name, path, and pipe)
 * The _PC_XXX values are defined in unistd.h; they start at 1 and go up
 * sequentially.
 * _PC_ERROR is used as the first bit to indicate total failure
 * (all info invalid).
 * To check for an error something like
 * 	_PC_ISSET(_PC_PATHMAX, foo.pc_mask) != 0
 * is used.
 */

/*
 * The size of the non-kernel part of the struct.
 */
#ifdef	KERNEL
#define	PCSIZ		(int)(&(((struct pathcnf*)0)->pc_refcnt))
#define	PCCMP(p1, p2)	bcmp((char*)p1, (char*)p2, PCSIZ)
#endif

#endif	/* !__sys_pathconf_h */
