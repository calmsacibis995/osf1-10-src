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
 *	@(#)$RCSfile: cfgmgr.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/05/05 13:48:58 $
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

 */

#define	CMGR_PID_PATHNAME	"/var/run/cfgmgr.pid"
/*
 *      CMGR:	Common global data
 */
typedef struct {
	char *		progname;       /* Program name */
	char *		database;       /* Database file name */
	int		pidfd;		/* Lock file id */
	int		maxrec;		/* Max size of record */
	int		maxatr;		/* Max number of attributes */
	int    		fflg;           /* True if option -f present */
	int    		lflg;           /* True if option -l present */
	int    		dflg;           /* True if option -d present */
	int    		vflg;           /* True if option -v present */
	int		reset;		/* Reset cfgmgr */
	int		client;		/* Client is alive */
	int		exitval;	/* Exit value */
} cmgr_common_t;

extern cmgr_common_t	CMGR;

