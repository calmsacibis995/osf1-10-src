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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: pwd.h,v $ $Revision: 4.2.7.4 $ (OSF) $Date: 1993/08/04 21:23:39 $ */
/* pwd.h	1.11  com/inc,3.1,8943 7/14/89 08:36:10 */
/*
 * COMPONENT_NAME: pwd.h
 *                                                                    
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 */                                                                   

#ifndef _PWD_H_
#define _PWD_H_

#include <standards.h>
#include <sys/types.h>

/* The POSIX standard requires that certain elements be included in pwd.h. 
 * It also requires that when _POSIX_SOURCE is defined, only those standard
 * specific elements are made available.
 * This header includes all the POSIX required entries.
 */

#ifdef _POSIX_SOURCE

#if defined(__cplusplus)
extern "C"
{
#endif
extern struct passwd *getpwuid __((uid_t));
extern struct passwd *getpwnam __((const char *));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
extern int getpwuid_r __((uid_t, struct passwd *, char *, int));
extern int getpwnam_r __((const char *, struct passwd *, char *, int));
#endif	/* _REENTRANT || _THREAD_SAFE */
#if defined(__cplusplus)
}
#endif

struct passwd {
	char	*pw_name;
	char	*pw_passwd;
	uid_t	pw_uid;
	gid_t	pw_gid;
        int     pw_quota;
        char    *pw_comment;
	char    *pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};

#endif /* _POSIX_SOURCE */

#ifdef _OSF_SOURCE 

#define pw_etc pw_gecos

#if defined(__cplusplus)
extern "C"
{
#endif
extern struct passwd *getpwent __((void));
extern int setpwent __((void));
extern void endpwent __((void));
extern void setpwfile __((const char *));
#if defined(__cplusplus)
}
#endif

#include <stdio.h>
#if defined(__cplusplus)
extern "C"
{
#endif
extern struct passwd *fgetpwent __((FILE *));
extern int putpwent __((struct passwd *, FILE *));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)

extern int fgetpwent_r __((FILE *, struct passwd *, char *, int));
extern int getpwent_r __((struct passwd *, char *, int, FILE **));
extern int setpwent_r __((FILE **));
extern void endpwent_r __((FILE **));
#endif	/* _REENTRANT || _THREAD_SAFE */
#if defined(__cplusplus)
}
#endif

#endif /* _OSF_SOURCE */ 

#endif /* _PWD_H_ */
