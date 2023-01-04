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
 * @(#)$RCSfile: cred.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 19:01:55 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990  Mentat Inc.
 ** cred.h 2.1, last change 11/14/90
 **/

#ifndef _SYS_CRED_H
#define _SYS_CRED_H

/*
 * Define "struct cred" for V.4 Streams drivers.
 * The Streams spec calls out only cr_uid and cr_gid.
 */

#include <sys/ucred.h>

typedef struct ucred	cred_t;

/* TEMPORARY (also questionable) */
#define	cr_ruid	cr_uid
#define	cr_rgid	cr_gid
#define	cr_suid	cr_uid
#define	cr_sgid	cr_gid

#endif
