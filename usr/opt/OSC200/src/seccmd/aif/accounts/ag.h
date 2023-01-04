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
 *	@(#)$RCSfile: ag.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:07 $
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



/*  Group account utility header
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


#ifdef AG_ALLOCATE
#define GL_ALLOCATE
#endif

#include        "gl_defs.h"

void
	checkmemgroup(),
	getusers();

int
	do_addgroup(),
	do_freegroup();

void
	grp2grf(),
	grf2grp();

int
	ag_auth(),
	ag_canaddgroup(),
	ag_isoldgroup(),
	ag_bfill();


/* Template for fill-in screen */

static struct	grp_fillin {
	char	groupname[GNAMELEN + 1];
	long	groupid;
	char	**usermem;		/* for scrolling region */
	int	nusermem;		/* number of pointers in usermem */
	int	ndescs;
	int	nstructs;
	int	(*new) ();		/* new_group func for isoldgroup() */
} Scr_grp;

static struct grp_fillin *scr_grp = &Scr_grp;


/* offsets for scrn_struct */

#define	GROUPNAME 	0
#define GROUPID		1
#define USERFNMEM	2
#define GRNSCRNSTRUCT	3

#define FILLINSTRUCT	grp_fillin
#define FILLIN		scr_grp

#define FIRSTDESC	1
#define NSCRNSTRUCT	GRNSCRNSTRUCT
#define TRAVERSERW	TRAV_RW

extern int
	CheckGroupAccess(),
	CheckValidGroup(),
	GroupNameExists(),
	CheckValidGid(),
	GroupGidExists(),
	XModifyUserGroup(),

	InvalidGroup();

GLOBAL char **sec_gusers INIT1 (NULL);	/* ptrs to actual group members */
GLOBAL char *sec_gus_mask INIT1 (NULL);	/* whether users are selected */

/* global routines used everywhere */

struct	group	*getgrnam(), *getgrent(), *getgrgid();

extern char *Calloc(), *Malloc(), *Realloc();
extern void Free();
