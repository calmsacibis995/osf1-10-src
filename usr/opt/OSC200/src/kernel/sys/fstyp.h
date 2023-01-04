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
 * @(#)$RCSfile: fstyp.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/03 22:59:37 $
 */

#ifndef _SYS_FSTYP_H
#define _SYS_FSTYP_H


#define FSTYPSZ		16	/* max size of fs identifier */

/*
 * Opcodes for the sysfs() system call.
 */
#define GETFSIND	1	/* translate fs identifier to fstype index */
#define GETFSTYP	2	/* translate fstype index to fs identifier */
#define GETNFSTYP	3	/* return the number of fstypes */

#if defined(__STDC__) && !defined(_KERNEL)
int sysfs(int, ...);
#endif

#endif	/* _SYS_FSTYP_H */
