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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: agutils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:10 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*  Group account utility routines.
 */

#include	"ag.h"


int
ag_canaddgroup (argv, grfill)
char	**argv;
struct	grp_fillin	*grfill;
{
	char	*group;
	long	groupid;
	char	*group_copy;
	char	*strrchr();

	ENTERFUNC("ag_canaddgroup");
	/* check that the program can write in the parent directory of
	 * the group file.
	 */
	
	group_copy = (char *)malloc (strlen(groupfn)+1);
	strcpy (group_copy, groupfn);
	group = strrchr (group_copy, '/');
	*group = '\0';
	if (eaccess (group_copy, 3) != 0) {  /* need write and search */
		if (! msg_cant_access_group)
			LoadMessage("msg_accounts_make_group_cant_access_group",
				&msg_cant_access_group, 
				&msg_cant_access_group_text);
		ErrorMessageOpen(-1, msg_cant_access_group, 0, NULL);
		ERRFUNC ("ag_canaddgroup", "group access");
		return (1);
	}
	return (ag_isoldgroup (argv, grfill));
}


/* is it a new/old group? If so, return 0, else 1 */

int
ag_isoldgroup (argv, grfill)
char **argv;
struct	grp_fillin	*grfill;
{

	char	*group;
	long	groupid;

	ENTERFUNC("ag_isoldgroup");

	if (*gl_group)
		group = (char *)gl_group;
	else
		group = NULL;
	groupid = gl_gid;
	if ((!group || !*group) && groupid == BOGUS_ID) {
		ERRFUNC ("ag_isoldgroup", "no group or id");
		return 0;
	}

	DUMPVARS ("ag_isoldgroup: group='%s' gid=<%d>", (group?group:"NO NAME"),
		groupid, NULL);
	if (grfill->new && (*grfill->new)(group, groupid))
		return (1);

	if (group)
		strcpy (grfill->groupname, group);
	grfill->groupid = groupid;
	grfill->groupid = 0;
	EXITFUNC("ag_isoldgroup");
	return 0;
}





/* fill in a new grfill structure with empty entries */
int
ag_bfill (grfill)
register struct	grp_fillin	*grfill;
{
	int i;

	ENTERFUNC("acag_bfill");
	grfill->groupname[0] = '\0';
	grfill->groupid = BOGUS_ID;
	GetAllUsers (&grfill->nusermem, &grfill->usermem);
	if (!grfill->usermem ||
		!(sec_gus_mask = Calloc (grfill->nusermem, sizeof (char))))
		MemoryError ();		/* dies */
	EXITFUNC("acag_bfill");
	return (0);
}


