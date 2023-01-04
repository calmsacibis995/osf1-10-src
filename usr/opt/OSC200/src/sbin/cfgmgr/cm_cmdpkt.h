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
 *	@(#)$RCSfile: cm_cmdpkt.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:32:31 $
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

#define LOCALSOCKNAME   "/dev/cfgmgr"		/* Local Unix socket name */
#define SERVICENAME     "cfgmgr"		/* Remote Inet service name */

/*
 *      Communication command structure
 */

typedef struct {
	int	cmdpkt_op;				/* Subsys command */
	int	cmdpkt_loglvl;				/* Subsys log level */
	char	cmdpkt_name[128];			/* Subsys name */
	char	cmdpkt_opts[128];			/* Pass thru options */
} cmgr_cmdpkt_t;

#define PKTSZ   sizeof(cmgr_cmdpkt_t)

