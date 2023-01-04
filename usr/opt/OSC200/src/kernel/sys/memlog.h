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
 * @(#)$RCSfile: memlog.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/01 20:04:20 $
 */

#ifndef	_SYS_MEMLOG_H
#define _SYS_MEMLOG_H

#include <sys/ioctl.h>

#define MALLOC_LOG      1
#define FREE_LOG        2
#define KALLOC_LOG      3
#define KFREE_LOG       4
#define KGET_LOG        5
#define KMEM_ALLOC_LOG  6
#define KMEM_FREE_LOG   7
#define ZALLOC_LOG      8
#define ZGET_LOG        9
#define ZFREE_LOG       10
#define H_KMEM_ALLOC_LOG        11
#define H_KMEM_ZALLOC_LOG       12
#define H_KMEM_FREE_LOG         13
#define H_KMEM_FAST_ALLOC_LOG   14
#define H_KMEM_FAST_ZALLOC_LOG  15
#define H_KMEM_FAST_FREE_LOG    16

#define MEM_RECORD_SIZE 50

/* memory logger - group M */
#define MEM_LOG_ENABLE           _IOW('M', 1, long)
#define MEM_LOG_DISABLE          _IO('M', 2)

#ifdef	_KERNEL
long   asm( const char *,...);
#pragma intrinsic(asm)

#define GET_CALLER(caller) \
        void *caller = (void *)asm("lda %v0,-4(%ra)")

extern void memory_log(int type, void *caller, int size);
extern long memlog;

#endif  /* _KERNEL */

#endif	/* _SYS_MEMLOG_H */
