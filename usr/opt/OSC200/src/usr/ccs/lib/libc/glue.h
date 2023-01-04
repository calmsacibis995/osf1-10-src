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
 * @(#)$RCSfile: glue.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 23:08:20 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#ifdef _THREAD_SAFE
#define inuse(iop)	((iop)->_flag & (_IOINUSE|_IOREAD|_IOWRT|_IORW))
#else
#define inuse(iop)	((iop)->_flag & (_IOREAD|_IOWRT|_IORW))
#endif /* _THREAD_SAFE */

#define _NIOBRW		16
#define _NROWSIZE	(_NIOBRW * sizeof(FILE))
#define _NROWSTART	4
#define _NROWEXTEND	(_NROWSTART << 2)

struct glued {
        unsigned   lastfile;
        unsigned   freefile;
        int     nfiles;
        int     nrows;
        int     crow;
        FILE    **iobptr;
        };

extern struct glued _glued;
