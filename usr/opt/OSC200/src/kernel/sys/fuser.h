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
 * @(#)$RCSfile: fuser.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/05/27 21:37:12 $
 */
#ifndef	_SYS_FUSER_H
#define _SYS_FUSER_H

/*
 * fuser() flags
 */
#define F_FILE_ONLY	0x1
#define F_CONTAINED	0x2


/*
 * Data structure filled in by fuser().
 */
typedef struct f_user {
	pid_t	fu_pid;
	int	fu_flags;
	uid_t	fu_uid;	
} f_user_t;


/*
 * fu_flags values
 */
#define F_CDIR		0x0001
#define F_RDIR		0x0002
#define F_PDIR		0x0004
#define F_OPEN		0x0008
#define F_TTY		0x0010
#define F_TRACE		0x0020

#if	!defined(_KERNEL)

#if	defined(__STDC__)
extern int	fuser(const char *, int, struct f_user *, int);
#else	/* !__STDC__ */
extern int	fuser();
#endif	/* !__STDC__ */

#endif	/* !_KERNEL */

#endif	/* !_SYS_FUSER_H */
