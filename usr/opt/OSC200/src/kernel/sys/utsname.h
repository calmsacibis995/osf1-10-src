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
 *	@(#)$RCSfile: utsname.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/14 13:52:52 $
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


#ifndef	_SYS_UTSNAME_H_
#define	_SYS_UTSNAME_H_
#include <standards.h>

/*
 * POSIX requires that certain values be included in utsname.h.  It also
 * requires that when _POSIX_SOURCE is defined only those standard
 * specific values are present.  This header includes all the POSIX
 * required entries.
 */
#ifdef _POSIX_SOURCE

#define  _SYS_NMLN           32     /* Important: do not change this value ! */

struct utsname {
	char    sysname[_SYS_NMLN];
	char    nodename[_SYS_NMLN];
	char    release[_SYS_NMLN];
	char    version[_SYS_NMLN];
	char    machine[_SYS_NMLN];
};

#ifndef _KERNEL
#ifdef _NO_PROTO	
extern int uname();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern int uname(struct utsname *);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* _KERNEL */

#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE

#define SYS_NMLN	_SYS_NMLN

extern struct utsname utsname;

#endif /* _OSF_SOURCE */

#endif /* _SYS_UTSNAME_H_ */
