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
 *	prof.h
 */
#include "cpus.h"

#define MAX_CPU         NCPUS	/* see DEFAULT/GENERIC config files */

#define PRF_RESET       0	/* initialize/reset all profiler flags */

#define PRF_OFF         0	/* profiler collecting samples */
#define PRF_ON          1	/* profiler collecting samples */
#define PRF_VAL         2	/* profiler contains valid text symbols */

#define PRF_STATUS      1	/* profiling on or off */
#define PRF_MAX         2	/* total # of text addresses */
#define PRF_MODE        3	/* set profiling mode */
#define PRF_DEBUG_SET   4	/* set debuging mode */
#define PRF_FREE	5	/* reset and free up kernel memory allocated
				   for profiling */

#ifdef PRF_IOCTLS
#define PRF_SET_MODE 	_IOW(0,PRF_MODE,int)
#define PRF_GET_STATUS	_IOR(0,PRF_STATUS,int)
#define PRF_GET_MAX	_IOR(0,PRF_MAX,int)
#define PRF_PROF_RESET	_IOW(0,PRF_FREE,int)
#endif

