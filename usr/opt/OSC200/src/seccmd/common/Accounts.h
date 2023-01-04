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
 *	@(#)$RCSfile: Accounts.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:59 $
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
#ifdef SEC_BASE

#ifndef _Accounts_h_
#define _Accounts_h_

/*
	filename:
		Accounts.h
	
	copyright:
		Copyright (c) 1989, 1990 SecureWare, Inc.
		ALL RIGHTS RESERVED
	
	function:
		common include file for account maintenance facilities
*/

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#include "gl_defs.h"
#include "UIstrlen.h"

/* Load in main variables that are accessed by all files */
GLOBAL
char 	
	*chosen_user_name,
	*chosen_device_name;

/*
 * status codes, errors, etc
 */

#define SUCCESS			0
#define FAILURE			1

#define NOT_USER		1
#define NO_PRDB_ENTRY		2
#define ACCT_RETIRED		3
#define IS_ISSO			4
#define NOT_ISSO		5
#define NULL_USER		6
#define CANT_UPDATE		7
#define CANT_READ_AUDIT		8

#define NO_DEVICE_ENTRY		9
#define NULL_DEVICE		10
#define NO_TERMINAL_ENTRY	11
#define CANT_UPDATE_DEVICES	12
#define CANT_UPDATE_TERMINAL	13
#define NULL_TERMINAL		14

#define IS_SYSADMIN		15
#define IS_ISSO_ADMIN		16	/* ISSO && SYS_ADMIN */

#define NULL_PTR		17

/* General values */
#define NO_CHAR		'N'
#define YES_CHAR	'Y'
#define DEFAULT_CHAR	'D'


/* Protected password structure */

struct prpw_if {
	struct pr_passwd	prpw;
};

/* Device assignment field */

struct dev_if {
	struct	dev_asg		dev;
	struct	pr_term		tm;	/* use if dev is terminal or host */
};

/* system defaults */

struct	sdef_if  {
	struct	pr_default	df;
};

#define ROUND2(i)	( (i + 1) / 2)
#define ROUND13(i)	( (i + 2) / 3)
#define ROUND23(i)	( (2 * (i + 1) ) / 3)

#endif /* _Accounts_h_ */
#endif /* SEC_BASE */
