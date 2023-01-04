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
 *	@(#)$RCSfile: ftw.h,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/08/12 21:12:26 $
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 *	Codes for the third argument to the user-supplied function
 *	which is passed as the second argument to ftw
 */
#ifndef _FTW_H_
#define _FTW_H_

#include <standards.h>
#include <sys/stat.h>

#ifdef _XOPEN_SOURCE

#define	FTW_F	0	/* file */
#define	FTW_D	1	/* directory */
#define	FTW_DNR	2	/* directory without read permission */
#define	FTW_NS	3	/* unknown type, stat failed */


#ifdef _AES_SOURCE
#define FTW_SL	4	/* symbolic link */
#endif

#ifdef _OSF_SOURCE

#define FTW_DP	5	/* directory was previously visited */
#define FTW_PHYS	0x00000001
#define FTW_MOUNT	0x00000002
#define FTW_DEPTH	0x00000004
#define FTW_CHDIR	0x00000008

struct FTW {
	int base;
	int level;
}; 

#endif /* _OSF_SOURCE */


struct stat;

#ifdef _NO_PROTO
extern int ftw();
#ifdef _OSF_SOURCE
extern int nftw();
#endif /* _OSF_SOURCE */
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern int ftw(const char *, int (*)(const char *, 
		       const struct stat *, int), int );
#ifdef _OSF_SOURCE
extern int nftw(const char *, int (*)(char *, const struct stat *, 
		       int, struct FTW), int, int);
#endif /* _OSF_SOURCE */
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

#endif /* _XOPEN_SOURCE */

#endif /* _FTW_H_ */

