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
static char	*sccsid = "@(#)$RCSfile: acsutils.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:54 $";
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
 *  User account (short form) utility routines
 */

#include	"acs.h"




/* see if authorized to do this
 * returns 0 if user can be added, 1 if not
 */

int
acs_auth (argv, pwfill)
char	**argv;
struct  pwd_fillin	*pwfill;
{
	ENTERLFUNC("acs_auth");
	return (acs_canaccesspw (argv, pwfill));
}



/* returns 0 if passwd readable, 1 if not */

int
acs_canaccesspw (argv, pwfill)
char	**argv;
struct	pwd_fillin	*pwfill;
{
	ENTERLFUNC("acs_canaccesspw");
	if (raccess_pw ()) {
		ERRLFUNC ("acs_canaccesspw", "passwd access");
		return (1);
	}
	EXITLFUNC("acs_canaccesspw");
	return (0);
}


/* returns 0 if passwd file readable, else 1 */

int
raccess_pw ()
{
	extern	char	passwd[];		/* password file name */
	int	status = 0;

	ENTERLFUNC("access_pw");
	if (eaccess (passwd, 1) != 0) {  /* need read */
		if (! msg_error_cant_access_passwd)
			LoadMessage("msg_accounts_make_user_cant_access_passwd",
				&msg_error_cant_access_passwd, 
				&msg_error_cant_access_passwd_text);
		ErrorMessageOpen(-1, msg_error_cant_access_passwd, 0, NULL);
		ERRLFUNC ("access_pw", "passwd access");
		status = 1;
	}
	EXITLFUNC("access_pw");
	return (status);
}





/*
	executed each selection - emulates radio button action
*/


int
acs_radio ()
{
	register char *cp, *last, *op;

	cp = scr_pwd->state;
	last = cp + scr_pwd->nusers;
	for (op = scr_pwd->old_state; cp < last; cp++, op++)
		if (*cp)
			if (*op)
				*cp = *op = 0;
			else 
				*op = 1;
		else
			if (*op)
				*op = 0;
	return 1;
}



/*
 * save the chosen username, and it's UID, in the
 * global vars. don't bother if it's the same as the
 * current ones.
 */

int
acs_save (pwfill)
struct	pwd_fillin	*pwfill;
{
	struct passwd user;

	ENTERLFUNC("acs_save");
	if (strcmp ((char *)gl_user, pwfill->user)) {
		strcpy ((char *)gl_user, pwfill->user);
		if (GetPWUserByName((char *)gl_user, &user))
			gl_uid = user.pw_uid;
		else
			gl_uid = BOGUS_ID;
	}
	EXITLFUNC("acs_save");
	return 0;
}



/* free malloc'd memory */

int
acs_free (argv, pwfill, nstructs, pp, dp, sp)
char	**argv;
struct	pwd_fillin	*pwfill;
int	nstructs;
Scrn_parms	*pp;
Scrn_desc	*dp;
Scrn_struct	*sp;
{
	ENTERLFUNC("acs_free");
	if (pwfill->users)
		free_cw_table (pwfill->users);
	Free (pwfill->state);
	Free (pwfill->old_state);
	EXITLFUNC("acs_free");
	return (1);
}

