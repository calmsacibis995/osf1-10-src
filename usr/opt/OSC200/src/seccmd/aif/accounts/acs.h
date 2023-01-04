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
 *	@(#)$RCSfile: acs.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:39 $
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
/* Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*  User account (short form) utility header
 */

#include	<sys/stat.h>
#include	<sys/param.h>
#include	<stdio.h>
#include	<pwd.h>
#include	<grp.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/security.h>
#include	<sys/audit.h>
#include	<prot.h>
#include	"userif.h"
#include	"UIMain.h"
#include	"Accounts.h"
#include	"valid.h"
#include	"logging.h"
#include	"Messages.h"
#include	"Utils.h"


#ifdef ACS_ALLOCATE
#define GL_ALLOCATE
#endif

#include	"gl_defs.h"

int
	acs_auth(),
	acs_canaccesspw(),
	raccess_pw(),
	acs_save(),
	acs_free();

int acs_radio();	/* radio button action */


/* Template for fill-in screen */

static struct	pwd_fillin {
	char	**users;
	int	nusers;
	char	user[UNAMELEN + 1];
	char	*state;
	char	*old_state;
	int	nstructs;
} Scr_pwd;

static struct pwd_fillin *scr_pwd = &Scr_pwd;


/* offsets for scrn_struct */

#define	USERNAMES 	0
#define NUSERS		1
#define USERNAME	2
#define PWNSCRNSTRUCT	5

#define FILLINSTRUCT	pwd_fillin
#define FILLIN		scr_pwd

#define FIRSTDESC	USERNAMES
#define NSCRNSTRUCT	1
#define TRAVERSERW	TRAV_RW

#define AUTHFUNC	acs_auth
#define	SCREENACTION	acs_save
#define FREESTRUCT	acs_free

