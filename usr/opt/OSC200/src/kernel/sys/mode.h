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
 *	@(#)$RCSfile: mode.h,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/06/08 01:14:36 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: mode.h
 *                                                                    
 * ORIGIN: IBM
 *
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#ifndef _SYS_MODE_H_
#define _SYS_MODE_H_


/*
 * POSIX requires that certain values effectively be included in stat.h.
 * It also requires that when _POSIX_SOURCE is defined, only those standard
 * specific values are present.  Since mode.h defines values on behalf of
 * stat.h and since stat.h includes mode.h, this header adheres to the
 * POSIX requirements.
 */

#include <standards.h>
#ifdef _POSIX_SOURCE 

/*
 *	(stat) st_mode bit values
 */

#define S_ISUID		0004000		/* set user id on execution */
#define S_ISGID		0002000		/* set group id on execution */

					/* ->>> /usr/group definitions <<<- */
#define S_IRWXU		0000700		/* read,write,execute perm: owner */
#define S_IRUSR		0000400		/* read permission: owner */
#define S_IWUSR		0000200		/* write permission: owner */
#define S_IXUSR		0000100		/* execute/search permission: owner */
#define S_IRWXG		0000070		/* read,write,execute perm: group */
#define S_IRGRP		0000040		/* read permission: group */
#define S_IWGRP		0000020		/* write permission: group */
#define S_IXGRP		0000010		/* execute/search permission: group */
#define S_IRWXO		0000007		/* read,write,execute perm: other */
#define S_IROTH		0000004		/* read permission: other */
#define S_IWOTH		0000002		/* write permission: other */
#define S_IXOTH		0000001		/* execute/search permission: other */

/*
 *	File type macros
 */

#define S_ISFIFO(m)	(((m)&(0170000)) == (0010000))
#define S_ISDIR(m)	(((m)&(0170000)) == (0040000))
#define S_ISCHR(m)	(((m)&(0170000)) == (0020000))
#define S_ISBLK(m)	(((m)&(0170000)) == (0060000))
#define S_ISREG(m)	(((m)&(0170000)) == (0100000))

#endif /* _POSIX_SOURCE */

#ifdef _XOPEN_SOURCE

/*
 *	Additional mode bit values
 *	
 */

#define S_IFMT		0170000		/* type of file */
#define   S_IFREG	0100000		/*   regular */
#define   S_IFDIR	0040000		/*   directory */
#define   S_IFBLK	0060000		/*   block special */
#define   S_IFCHR	0020000		/*   character special */
#define   S_IFIFO	0010000		/*   fifo */

#define S_ISVTX		0001000		/* save text even after use */
#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE

/*
 *	More mode bit values
 *	(Macros are separated because they're not strictly part of the standard)
 */


#define S_IFSOCK	0140000		/* socket */
#define S_IFLNK		0120000		/* symbolic link */

#define S_ENFMT		S_ISGID		/* record locking enforcement flag */

/*
 * These next three have been WITHDRAWN from the X/Open standard.
 * Use S_IRUSR, S_IWUSR, and S_XUSR instead.
 */

#define S_IREAD		S_IRUSR		/* read permission, owner */
#define S_IWRITE	S_IWUSR		/* write permission, owner */
#define S_IEXEC		S_IXUSR		/* execute/search permission, owner */

#define S_ISLNK(m)	(((m)&(S_IFMT)) == (S_IFLNK))
#define S_ISSOCK(m)	(((m)&(S_IFMT)) == (S_IFSOCK))


/*
 *	Equivalent mode macros (from ufs/inode.h)
 *	Removed -- use S_* instead of *
 *	eg. S_IFMT instead of IFMT.
 */

/*
 *	Default file mode.
 */

#define S_DEFFILEMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

#endif /* _OSF_SOURCE */
#endif /* _SYS_MODE_H_ */
