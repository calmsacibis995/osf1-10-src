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
 * @(#)$RCSfile: memdebug.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/19 12:25:07 $
 */
/*
 *  STREAMS_MEMDBG can be used to keep track of mblk's that have been allocated
 *  but not freed.  To use it, supply the line "option STREAM_MEMDBG" in 
 *  the BINARY config file and rebuild
 *  the kernel.  Note: you must have source code to use this option.
 *  A linked list, pointed to by the global str_memused, 
 *  is kept giving information about these mblk's.
 *  A user's STREAMS module may want to include <streams/memdebug.h>
 *  to provide more detailed information in the linked list.
 */
#include <streams_memdbg.h>
#if STREAMS_MEMDBG
extern  struct msgb *debug_allocb(int size, int pric, char *str, int line);
#define allocb(size, pri)  debug_allocb(size, pri, __FILE__, __LINE__)
#define freeb(msg)  debug_freeb(msg, __FILE__, __LINE__)
#define puthere(q, mp)  debug_puthere((q), (mp), __FILE__, __LINE__)
#endif /* STREAMS_MEMDBG */

