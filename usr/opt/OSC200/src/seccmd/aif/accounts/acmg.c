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
static char	*sccsid = "@(#)$RCSfile: acmg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:25 $";
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



/*  Group account maintenance routines.
 *  Modify a group in /etc/group.
 *  argv is ignored
 */

#include	"ag.h"

/* routine definitions for multiply used routines */

static int acmg_auth();
static int checkgroupname();
static int checkgroupid();
static int acmg_new();
static int acmg_free();


extern Scrn_parms	acmg_scrn;
extern Scrn_desc	acag_desc[];
Scrn_struct		*acmg_struct;



#define PARMTEMPLATE	acmg_scrn
#define STRUCTTEMPLATE	acmg_struct
#define DESCTEMPLATE	acag_desc

struct group grp;

/* see if authorized to do this
 * returns 0 if group can be added, 1 if not
 */

static
int
acmg_auth (argv, grfill)
char	**argv;
struct  grp_fillin	*grfill;
{
	grfill->new = acmg_new;
	return (ag_canaddgroup (argv, grfill));
}


static
int
acmg_new (group, groupid)
char *group;
long groupid;
{
	struct	group	up;

	ENTERFUNC("acmg_new");
	if (group && group[0] && gr_nametoid (group) == (gid_t) -1) {
		nosuchgroup (group);
		ERRFUNC ("acmg_new", "group exists");
		return (1);
	}

	if (groupid != BOGUS_ID && gr_idtoname (groupid) == NULL) {
		nosuchgid (groupid);
		ERRFUNC ("acmg_new", "gid exists");
		return (1);

	if (group && group[0])
		GetGRGroupByName (group, &grp);
	else if (groupid != BOGUS_ID)
		GetGRGroupByGID (groupid, &grp);
	}
	EXITFUNC("acmg_new");
	return 0;
}




/*
 * fill in scrn_struct structure for the screen
 */

static
int
acmg_bstruct (grfill, sptemplate)
struct	grp_fillin	*grfill;
struct	scrn_struct	**sptemplate;
{
	register struct	scrn_struct	*sp;

	ENTERFUNC("acmg_bstruct");
	grfill->nstructs = GRNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		if (!grfill->groupname[0] && grfill->groupid == BOGUS_ID &&
		    !acmg_new (gl_group, gl_gid))
			if (gl_group[0])
				strcpy (grfill->groupname, (char *)gl_group);
			if (gl_gid)
				grfill->groupid = gl_gid;

		DUMPDECP (" sp being filled in", 0, 0, 0);
		DUMPDETI (" name=<%s> id=<%dl>",
			&(grfill->groupname[0]), grfill->groupid, NULL);

		/* set up state of group membership */
		if (grp.gr_mem != (char **) 0) {
			int i, j;

			for (i = 0; grp.gr_mem[i]; i++)
				for (j = 0; j < grfill->nusermem; j++)
					if (strcmp(grp.gr_mem[i],
						   grfill->usermem[j]) == 0) {
						sec_gus_mask[j] = 1;
						break;
					}
		}

		sp[GROUPNAME].pointer = grfill->groupname;
		sp[GROUPNAME].validate = checkgroupname;
		sp[GROUPNAME].filled = 1;
		sp[GROUPID].pointer = (char *) &grfill->groupid;
		sp[GROUPID].validate = checkgroupid;
		sp[GROUPID].filled = grfill->groupid ? 1 : 0;
		sp[USERFNMEM].pointer = (char *) grfill->usermem;
		sp[USERFNMEM].state = sec_gus_mask;
		sp[USERFNMEM].validate = NULL;
		sp[USERFNMEM].filled = grfill->nusermem;
		DUMPDETI (" name=<%s> id=<%dl>",
		    sp[GROUPNAME].pointer, (long) sp[GROUPID].pointer, NULL);
	}
	EXITFUNC("acmg_bstruct");
	return (!sp);
}



static
int
checkgroupname (grfill)
struct	grp_fillin	*grfill;
{

	ENTERFUNC("checkgroupname");
	if (!grfill->groupname) {
		pop_msg ("No group name entered", "Please enter a name");
		ERRFUNC ("checkgroupname", "no group name");
		return (1);
	}
	if (InvalidGroup (grfill->groupname)) {
		InvGroupMsg (grfill->groupname);
		ERRFUNC ("checkgroupname", "bad group name");
		return (1);
	}
	if (gr_nametoid (grfill->groupname) == (gid_t) -1) {
		GroupExistsMsg (grfill->groupname);
		ERRFUNC ("checkgroupname", "group name exists");
		return (1);
	}
	EXITFUNC("checkgroupname");
	return (0);
}




/* check uniqueness of group id.  If not assigned, don't allow.
 * return 0 if 0, assigned, or entered value is unique or 0, 1 otherwise.
 */

static
int
checkgroupid (grfill)
struct grp_fillin	*grfill;
{
	ENTERFUNC("checkgroupid");
	if (grfill->groupid == 0 || grfill->groupid == BOGUS_ID) {
		ERRFUNC ("checkgroupid", "no GID");
		return (1);
	}
	if (gr_idtoname (grfill->groupid) == NULL) {
		GroupExistsMsg (grfill->groupid);
		ERRFUNC ("checkgroupid", "GID exists");
		return (1);
	}
	EXITFUNC("checkgroupid");
	return (0);
}

/* is it a new group? If so, return 0, else 1 */

static
int
valid_group (argv, grfill)
char **argv;
struct	grp_fillin	*grfill;
{

	char	*group;
	long	groupid;

	ENTERFUNC("valid_group");

	if (*grfill->groupname)
		group = grfill->groupname;
	else
		group = NULL;
	groupid = grfill->groupid;

	if (grfill->groupid == 0 || groupid == BOGUS_ID) {
		ERRFUNC ("valid_group", "no group or id");
		return (1);
	}

	DUMPVARS ("valid_group: group='%s' gid=<%d>", (group?group:"NO NAME"),
		groupid, NULL);
	/* check syntax of group name, assure that there are no ':'.
	 * getscreen checks for unprintable characters.
	 */

	if (InvalidGroup (group)) {
		InvGroupMsg (group);
		ERRFUNC ("valid_group", "bad group name");
		return (1);
	}

	if (acmg_new (group, groupid))
		return (1);

	if (group) {
		strcpy (grfill->groupname, group);
		strcpy ((char *)gl_group, group);
	}
	gl_gid = groupid;
	EXITFUNC("valid_group");
	return 0;
}


/* does the screen action once everything is validated */
 
static
int
do_addgroup (grfill)
register struct	grp_fillin	*grfill;
{
	int ret;
	struct group grp;
	int i, j;

	ENTERFUNC("do_addgroup");

	memset((char *) &grp, '\0', sizeof grp);
	grp.gr_name = grfill->groupname;
	grp.gr_passwd = "";
	grp.gr_gid = grfill->groupid;

	/* pick up group state */
	for (i = 0, j = 0; i < grfill->nusermem; i++)
		if (sec_gus_mask[i])
			j++;

	/* if the group has members, set up membership array */
	if (j > 0) {
		grp.gr_mem = (char **) Calloc(j+1, sizeof(char *));
		for (i = 0, j = 0; i < grfill->nusermem; i++)
			if (sec_gus_mask[i])
				grp.gr_mem[j++] = grfill->usermem[i];
	}

	ret = XCreateGroup (&grp);
	
	if (grp.gr_mem != (char **) 0)
		Free((char *) grp.gr_mem);
	grfill->groupid = BOGUS_ID;
	EXITFUNC("do_addgroup");
	return (CONTINUE);

}



/* free malloc'd memory */

static
int
do_free (argv, grfill, nstructs, pp, dp, sp)
char	**argv;
struct	grp_fillin	*grfill;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	if (grfill->usermem) {
		free_cw_table (grfill->usermem);
		grfill->usermem = NULL;
	}
	if (sec_gusers) {
		Free (sec_gusers);
		sec_gusers = NULL;
	}
	if (sec_gus_mask) {
		Free (sec_gus_mask);
		sec_gus_mask = NULL;
	}

	return (1);
}

#define	SETUPFUNC	acmg_setup
#define	AUTHFUNC	acmg_auth
#define BUILDFILLIN	ag_bfill

#define	INITFUNC	acmg_init
#define BUILDSTRUCT	acmg_bstruct

#define	ROUTFUNC	acmg_exit
#define VALIDATE	valid_group
#define	SCREENACTION	do_addgroup

#define FREEFUNC	acmg_free
#define FREESTRUCT	do_free



#include "stemplate.c"

