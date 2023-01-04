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
static char	*sccsid = "@(#)$RCSfile: acru.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:35 $";
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



/*
 *  Account Retire User routines
 */

#include	"acs.h"


/* routine definitions for multiply used routines */

static int valid_it();


extern Scrn_parms	acru_scrn;
extern Scrn_desc	acru_desc[];
Scrn_struct		*acru_struct;

#define PARMTEMPLATE	acru_scrn
#define STRUCTTEMPLATE	acru_struct
#define DESCTEMPLATE	acru_desc


/*
 *  designate a user - argv[1] could be the user name or id
 */





/* fill in a new pwfill structure with empty entries */
static
int
acru_bfill (pwfill)
register struct	pwd_fillin	*pwfill;
{
	register int i;

	ENTERLFUNC("acs_bfill");
	pwfill->users = NULL;
	pwfill->nusers = 0L;
	memset(pwfill->user, '\0', sizeof(pwfill->user));
	GetAllNonretiredUsers (&pwfill->nusers, &pwfill->users);
	if (!(pwfill->state = Calloc (pwfill->nusers, sizeof (char *))))
		MemoryError (); /* Dies */
	if (!(pwfill->old_state = Calloc (pwfill->nusers, sizeof (char *))))
		MemoryError (); /* Dies */
	EXITLFUNC("acs_bfill");
	return (0);
}


/*
 * fill in scrn_struct structure for the screen
 */

static
int
acru_bstruct (pwfill, sptemplate)
struct	pwd_fillin	*pwfill;
struct	scrn_struct	**sptemplate;
{
	static struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acru_bstruct");
	pwfill->nstructs = PWNSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp != (struct scrn_struct *) 0) {
		DUMPDECP (" sp being filled in", 0, 0, 0);
		sp[USERNAMES].pointer = (char *) pwfill->users;
		sp[USERNAMES].filled = pwfill->nusers;
		sp[USERNAMES].state = pwfill->state;
		sp[USERNAMES].val_act = acs_radio;
	}
	EXITFUNC("acru_bstruct");
	return (!sp);
}




static
int
valid_it (argv, pwfill)
char **argv;
struct	pwd_fillin	*pwfill;
{
	struct prpw_if pr;
	int ret;
	register char *first, *cp, *last;

	ENTERFUNC("valid_it");
	first = pwfill->state;
	last = first + pwfill->nusers;
	for (cp = first; cp < last; cp++)
		if (*cp) {
			strcpy (pwfill->user, pwfill->users[cp - first]);
			break;
	}
	ret = GetUserInfo (pwfill->user, &pr);
	if (ret == SUCCESS) {
		pr.prpw.uflg.fg_retired = 1;
		pr.prpw.ufld.fd_retired = 1;
		ret = WriteUserInfo (&pr);
	} else {
		CantUpdateUserMsg ();
		ERRFUNC ("acru_valid", "can't update user");
	}
	EXITFUNC("valid_it");
	return 0;
}



#define	SETUPFUNC	acru_setup
#define	BUILDFILLIN	acru_bfill

#define	INITFUNC	acru_init
#define BUILDSTRUCT	acru_bstruct

#define	ROUTFUNC	acru_exit
#define VALIDATE	valid_it

#define FREEFUNC	acru_free


#include "stemplate.c"
