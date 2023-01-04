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
 *	@(#)$RCSfile: mp.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/12/15 22:13:43 $
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */


#ifndef _MP_H_
#define _MP_H_

#ifndef BYTE_ORDER
#include <sys/types.h>
#include <machine/endian.h>
#endif

#define MINT struct mint
MINT
{	int len;
	short *val;
};

#define FREE(x) {if(x.len!=0) {free((void *)x.val); x.len=0;}}

#ifndef DBG
#define shfree(u) free((void *)u)
#else
#include <stdio.h>
#define shfree(u) { if(dbg) fprintf(stderr, "free %o\n", u); free((void *)u);}
extern int dbg;
#endif /* DBG */

#if     BYTE_ORDER == BIG_ENDIAN
struct half
{       short high;
        short low;
};
#else
struct half
{       short low;
        short high;
};
#endif /* BYTE_ORDER */

#ifdef __cplusplus
extern "C" {
#endif
extern MINT *itom();
extern short *xalloc();
#ifdef __cplusplus
}
#endif

#ifdef lint
extern xv_oid;
#define VOID xv_oid =
#else
#define VOID
#endif

#endif /* _MP_H_ */
