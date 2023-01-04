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
static char	*sccsid = "@(#)$RCSfile: acuu.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:04 $";
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
 *  Account Unlock User routines
 */

#include	"acs.h"


/* routine definitions for multiply used routines */

static int valid_it();


extern Scrn_parms	acuu_scrn;
extern Scrn_desc	acuu_desc[];
Scrn_struct		*acuu_struct;

#define PARMTEMPLATE	acuu_scrn
#define STRUCTTEMPLATE	acuu_struct
#define DESCTEMPLATE	acuu_desc


/*
 *  designate a user - argv[1] could be the user name or id
 */



/* fill in a new pwfill structure with empty entries */
static
int
acuu_bfill (pwfill)
register struct	pwd_fillin	*pwfill;
{
	register int i;

	ENTERLFUNC("acs_bfill");
	pwfill->users = NULL;
	pwfill->nusers = 0L;
	memset(pwfill->user, '\0', sizeof(pwfill->user));
	GetAllLockedUsers (&pwfill->nusers, &pwfill->users);
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
acuu_bstruct (pwfill, sptemplate)
struct	pwd_fillin	*pwfill;
struct	scrn_struct	**sptemplate;
{
	static struct	scrn_struct	*sp;
	int	i;

	ENTERFUNC("acuu_bstruct");
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
	EXITFUNC("acuu_bstruct");
	return (!sp);
}




static
int
valid_it (argv, pwfill)
char **argv;
struct	pwd_fillin	*pwfill;
{
	char *s;
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
		pr.prpw.uflg.fg_lock = 0;
		pr.prpw.ufld.fd_lock = 0;
		ret = WriteUserInfo (&pr);
	} else {
		CantUpdateUserMsg ();
		ERRFUNC ("acuu_valid", "can't update user");
	}
	EXITFUNC("valid_it");
	return 0;
}



#define	SETUPFUNC	acuu_setup
#define	BUILDFILLIN	acuu_bfill

#define	INITFUNC	acuu_init
#define BUILDSTRUCT	acuu_bstruct

#define	ROUTFUNC	acuu_exit
#define VALIDATE	valid_it

#define FREEFUNC	acuu_free


#include "stemplate.c"
