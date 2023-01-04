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
 * @(#)$RCSfile: pathnames.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 22:12:32 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */

#define CCPATH	"/usr/ccs/bin/cc"
#define LDPATH	"/usr/ccs/bin/ld"

#ifdef SIDECAR
#define CCFLAGS " -Wf,-wchar32 -Wf,-unsigned_wchar -std1 -I/usr/i18n/include -c"
#else
#define CCFLAGS " -std1 -c"
#endif /* SIDECAR */

/*
 * Must have TWO %s's here.  First is additional loader options (-L flag)
 * and second is the resulting locale object name
 */
#ifdef SIDECAR
#define LDFLAGS_FMT " -shared -set_version i18n -soname %s %s -o %s"
#define LDLIBRARY   " -lwc -lc"
#else
#define  LDFLAGS_FMT " -shared -soname %s %s -o %s"
#define LDLIBRARY   " -lc"
#endif

#ifdef SIDECAR
#define CHARPATH "/usr/i18n/lib/nls/charmap/"
#else
#define CHARPATH "/usr/lib/nls/charmap/"
#endif

