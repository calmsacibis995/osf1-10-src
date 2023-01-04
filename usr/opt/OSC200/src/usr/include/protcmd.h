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
 *	@(#)$RCSfile: protcmd.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:18:29 $
 */ 
/*
 */
#if SEC_BASE
#ifndef __PROTCMD__
#define __PROTCMD__

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* Copyright (c) 1988, SecureWare, Inc.
 *   All rights reserved
 *

 * Based on:

 */

/*
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */

/*
 * Definitions for security-relevant routines of enhanced UNIX commands
 * that must be protected.
 */

#define AT_SEC_WRITEMODE	0220	/* Secure mode for creating files */
#define AT_SEC_ATMODE		06040	/* Secure mode for using files */
#define CRONTAB_SEC_CRMODE	0440	/* Secure mode for creating crontabs. */
#define	INIT_DATA_FILE_MODE	0600
#define LOGIN_PROGRAM		"/tcb/lib/login"
#define PASSWD_PROGRAM		"/tcb/bin/passwd"
#define INITCOND_PROGRAM	"/tcb/lib/initcond"
#define	DEV_LEADER		"/dev/"
#define	INIT_INITTAB_LOCATION	"/tcb/files/inittab"
#define	INIT_SU_LOCATION	"/tcb/bin/su"

#if SEC_MAC || SEC_ILB
#define	LP_LABEL_PREFIX		'L'
#define	LP_FILTER_PREFIX	'f'
#endif

#endif /* __PROTCMD__ */
#endif /* SEC_BASE */
