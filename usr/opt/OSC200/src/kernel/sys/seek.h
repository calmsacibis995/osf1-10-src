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
 * @(#)$RCSfile: seek.h,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:14:39 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */

#ifndef	_SYS_SEEK_H_
#define	_SYS_SEEK_H_

/* Symbolic constants for the "lseek" function: */
#define SEEK_SET 0	/* Set file pointer to "offset" */
#define SEEK_CUR 1	/* Set file pointer to current plus "offset" */
#define SEEK_END 2	/* Set file pointer to EOF plus "offset" */

#ifdef _BSD
#define L_SET		0	/* absolute offset */
#define L_INCR		1	/* relative to current offset */
#define L_XTND		2	/* relative to end of file */
#endif

#endif	/* _SYS_SEEK_H_ */
