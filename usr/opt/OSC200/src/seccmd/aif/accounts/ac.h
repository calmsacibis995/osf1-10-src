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
 *	@(#)$RCSfile: ac.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:16 $
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



/*  User account utility header
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


#ifdef AC_ALLOCATE
#define GL_ALLOCATE
#endif

#include	"gl_defs.h"

int
	checkgecos(),
	checkgroup(),
	checkhomedir(),
	checkshellname();

void
	getgroups();

int
	do_adduser(),
	do_freeuser();

void
	pwd2pwf(),
	pwf2pwd();

int
	ac_auth(),
	ac_canadduser(),
	ac_isolduser(),
	ac_bfill();


/* Template for fill-in screen */

static struct	pwd_fillin {
	char	username[UNAMELEN + 1];
	long	userid;
	char	groupname[GNAMELEN + 1];
	char	homedir[NHOMEDIR+1];
	char	shellname[NSHELLNAME+1];
	char	gecos[NGECOS+1];
	char	**groupmem;		/* for scrolling region */
	int	ngroupmem;		/* number of pointers in groupmem */
	long	groupid;
	int	ndescs;
	int	nstructs;
	int	(*new) ();		/* new_user func for isolduser() */
} Scr_pwd;

static struct pwd_fillin *scr_pwd = &Scr_pwd;


/* offsets for scrn_struct */

#define	USERNAME 	0
#define USERID		1
#define GROUPFNNAME	2
#define HOMEDIR		3
#define SHELLNAME	4
#define GECOS		5
#define GROUPFNMEM	6
#define PWNSCRNSTRUCT	7

#define FILLINSTRUCT	pwd_fillin
#define FILLIN		scr_pwd

#define FIRSTDESC	USERNAME
#define NSCRNSTRUCT	PWNSCRNSTRUCT
#define TRAVERSERW	TRAV_RW

#define PWCHUNK		50

extern int
	chooseuserid(),
	CheckPasswordAccess(),
	CheckValidName(),
	UserNameExists(),
	CheckValidUid(),
	UserUidExists(),
	CheckValidHomeDir(),
	CheckValidShell(),
	CheckValidComment(),
	XModifyUserGroup(),

	InvalidUser();

GLOBAL char **sec_groups INIT1 (NULL);	/* ptrs to sec. group memberships */
GLOBAL char *sec_gr_mask INIT1 (NULL);	/* whether sec. groups are selected */

GLOBAL char **group_list,
	**groups;

/* global routines used everywhere */

struct	passwd	*getpwnam(), *getpwent(), *getpwuid();

