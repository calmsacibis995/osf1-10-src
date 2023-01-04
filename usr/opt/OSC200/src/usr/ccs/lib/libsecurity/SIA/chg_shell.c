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
static char *rcsid = "@(#)$RCSfile: chg_shell.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/04/01 20:20:16 $";
#endif
     
/*****************************************************************************
*
*	int	siad_chg_shell((*sia_collect),username)
*
* Description: The purpose of this routine is to perform a change shell
* function.
*
* Returns: 
*		SAIDSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
******************************************************************************/

#include <sia.h>
#include <siad.h>
#include <sia_mech.h>

#include <sys/secdefines.h>
#include <sys/security.h>
#include <prot.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <ndbm.h>
#include <errno.h>
#include <strings.h>
#include <ctype.h>
#include <paths.h>
#include <limits.h>

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];

extern char *putGetLine();
extern int (*_c2_collect)();

static char *getLoginShell();
static int shell_change();

#define	DEFSHELL	_PATH_BSHELL
#define SHELL_SIZE	256

siad_chg_shell(collect,username,argc,argv)
int (*collect)();
const char *username;
int argc;
char *argv[];
{
        struct passwd *pwd;

	_c2_collect = collect;  

	set_auth_parameters(argc, argv);
	initprivs();
	
	if (c2AuthorizePwdChange(username, &pwd) == SIADFAIL)
	  return(SIADFAIL);

	if (shell_change(pwd) == SIADFAIL)
	  return(SIADFAIL);

	return(SIADSUCCESS);
	
} /* siad_chg_shell */

static int
shell_change(pwd)
     struct passwd *pwd;
{
    char *pC;

    if ((pC = getLoginShell(pwd)) == (char *)SIADFAIL)
      return(SIADFAIL);

    pwd->pw_shell = pC;

    if (updatePasswordFile(pwd) == SIADFAIL)
      return(SIADFAIL);

    return(SIADSUCCESS);
    
} /* shell_change */

static char *
getLoginShell(pwd)
     struct passwd *pwd;
{
        char *newshell;
	char *cp, *valid, *getusershell();
	char buf[1024];
	uid_t u, getluid();
  
	u = getluid();	/* get my own login ID */

	if (pwd->pw_shell == 0 || *pwd->pw_shell == '\0')
		pwd->pw_shell = DEFSHELL;
	if (u != 0) {
		setusershell();
		do {
			valid = getusershell();
			if (valid == NULL) {
				show_error(GETMSG(MS_SIA_PWD, RESTRICT, "Cannot change from restricted shell %s"),
					pwd->pw_shell);
				return(SIADFAIL);
			}
		} while (strcmp(pwd->pw_shell, valid) != 0);
		endusershell();
	}

	sprintf(buf, GETMSG(MS_SIA_PWD, OLDSH, "Old shell: %s\nNew shell: "), pwd->pw_shell);
	newshell = putGetLine(buf, SHELL_SIZE);
	cp = index(newshell, '\n');
    	if (cp)
		*cp = '\0';
	if (newshell[0] == 0) {
		show_error(GETMSG(MS_SIA_PWD, LOGINSH, "Login shell unchanged."));
		return(SIAFAIL);
	}
	/*
	 * Allow user to give shell name w/o preceding pathname.
	 */
	if (u != 0 || newshell[0] != '/') {
	        setusershell();
		do {
			valid = getusershell();
			if (valid == 0) {
				if (u == 0) {
					valid = newshell;
					break;
				}
				show_error(GETMSG(MS_SIA_PWD, UNACCSH, "%s is unacceptable as a new shell."),
					newshell);
				return(SIADFAIL);
			}
			if (newshell[0] == '/') {
				cp = valid;
			} else {
				cp = rindex(valid, '/');
				if (cp == 0)
					cp = valid;
				else
					cp++;
			}
		} while (strcmp(newshell, cp) != 0);
/*
 * Don't use pointer value returned from getusershell().  This will be an
 * invalid pointer after the call to endusershell().  Make a local copy to
 * a static variable before then.  -dlong 91/10/8
 */
		if(valid && valid != newshell) {
			strncpy(newshell, valid, SHELL_SIZE);
			valid = newshell;
		}
		endusershell();
	}
	else
		valid = newshell;

	if (strcmp(valid, pwd->pw_shell) == 0) {
		show_mesg(GETMSG(MS_SIA_PWD, LOGINSH, "Login shell unchanged."));
		return(SIADFAIL);
	}
	if (access(valid, X_OK) < 0) {
		show_error(GETMSG(MS_SIA_PWD, UNAVAILB, "%s is unavailable."), valid);
		return(SIADFAIL);
	}
	if (strcmp(valid, DEFSHELL) == 0)
		valid[0] = '\0';
	return (valid);
	
} /* getloginshell */
