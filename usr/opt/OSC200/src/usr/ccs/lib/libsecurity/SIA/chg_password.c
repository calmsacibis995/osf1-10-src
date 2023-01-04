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
static char *rcsid = "@(#)$RCSfile: chg_password.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/04/01 20:20:09 $";
#endif

/*****************************************************************************
*
*	int	siad_chg_password((*sia_collect),username)
*
* Description: The purpose of this routine is to perform a change password
* function.
*
* Returns: 
*		SAIDSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/

#include <sia.h>
#include <siad.h>
#include <sia_mech.h>

#include <sys/secdefines.h>

#include <sys/security.h>
#include <prot.h>

#include <stdio.h>
#include <pwd.h>
#include <strings.h>

siad_chg_password(collect,username,argc,argv)
int (*collect)();
const char *username;
int argc;
char *argv[];
{
        struct passwd *pwd;
	extern int (*_c2_collect)();
	extern int _c2_collinput;

	_c2_collect = collect;
	_c2_collinput = 1;

        set_auth_parameters(argc, argv);
	initprivs();

	if (c2AuthorizePwdChange(username, &pwd) == SIADFAIL)
	  return(SIADFAIL);

	if (passwd_change(collect) == SIADFAIL)
	  return(SIADFAIL);

	return(SIADSUCCESS);
	
} /* siad_chg_password */

/*
 * c2AuthorizePwdChange...
 *
 * This routine authorizes a change to the a password record.  To do
 * this it verifies that the caller is changing his own password info,
 * or that he has command authorization for changing passwords in general.
 *
 * By convention, if the username passed in is NULL, then the owner of
 * the password record being changed is assumed to be the current
 * process.
 *
 * The password pointer passed in gets filled in with the password
 * structure to be changed and is returned.
 */

c2AuthorizePwdChange(username, pwd)
     const char *username;
     struct passwd **pwd;
{
	uid_t uid, getluid();
	extern char *command_name;
  
	/*
	 * this call to passwd_getlname() sets up globals in passwd_sec
	 * for later calls we'll make (eeeuuuuch!).
	 */
	
	(void) passwd_getlname();

        /*
	 * get my own login UID and get the password structure for
	 * the user whose info is changing (which might not be me).
	 */
	
	uid = getluid();

	if (!username) {
	  *pwd = getpwuid(uid);
	  if (!*pwd)
	    fprintf(stderr, GETMSG(MS_SIA_PWD,UNKUID,"%s: %u: unknown uid\n"),
		    command_name, uid);
	}
	else {
	  *pwd = getpwnam(username); 
	  if (!*pwd)
	    fprintf(stderr, GETMSG(MS_SIA_PWD,UNKUSER,"%s: %s: unknown user\n"),
		    command_name, username);
	}
 
	if (!*pwd) {
		return SIADFAIL;
	}

	/*
	 * verify that we have the command authorization to change
	 * the passwd (in case we're changing someone else's passwd).
	 *
	 * args: pwd - passwd we're changing
	 * 	 uid - our uid
	 */
	
	return(passwd_auth(*pwd, uid));
	
} /* c2AuthorizePwdChange */
