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
 *	@(#)$RCSfile: dk.h,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/05/27 19:06:31 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_SYS_DK_H_
#define _SYS_DK_H_

/* dk.h
 *
 * Modification History
 *
 * 20-Nov-91	Tom Tierney
 *	Modified DK_NDRIVE to support up to 256 disk units.
 *
 */ 
 
/*
 * Instrumentation
 */

#define CPUSTATES	5

#define CP_USER		0
#define CP_NICE		1
#define CP_SYS		2
#define CP_IDLE		3
#define CP_WAIT		4

#define DK_NDRIVE	256	

#ifdef	KERNEL
extern long	cp_time[CPUSTATES];

extern int	dk_ndrive;
extern int	dk_busy;
extern long	dk_time[DK_NDRIVE];
extern long	dk_seek[DK_NDRIVE];
extern long	dk_xfer[DK_NDRIVE];
extern long	dk_wds[DK_NDRIVE];
extern long	dk_wpms[DK_NDRIVE];

extern long	tk_nin;
extern long	tk_nout;
       int      tk_cancc;
       int      tk_rawcc;

/* for sar -a support */
extern long tf_iget;
extern long tf_namei;
extern long tf_dirblk;

/* for sar -b support */
extern long ts_bread;
extern long ts_bwrite;
extern long ts_lread;
extern long ts_lwrite;
extern long ts_phread;
extern long ts_phwrite;

/* for sar -c support */
extern long ts_sysread;
extern long ts_syswrite;
extern long ts_sysexec;
extern long ts_readch;
extern long ts_writech;

/* for sar -k support */
extern long sar_kmem_fail;

/* for sar -q support */
extern long sar_runocc;

/* for sar -v support */
extern long tbl_proc_ov;
extern long tbl_inod_ov;
extern long tbl_file_ov;
extern long numvnodes;

extern long pg_v_pgpgin;
extern long pg_v_sftlock;
extern long pg_v_pgout;
extern long pg_v_dfree;
extern long pg_v_scan;
extern long pg_v_s5ifp;

#endif	/* KERNEL */
#endif	/* _SYS_DK_H_ */
