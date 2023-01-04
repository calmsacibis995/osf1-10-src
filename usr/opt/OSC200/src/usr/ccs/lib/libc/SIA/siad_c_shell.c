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
static char *rcsid = "@(#)$RCSfile: siad_c_shell.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/08/04 21:22:12 $";
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
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_chg_shell = __siad_chg_shell
#endif
#endif
#include <sys/secdefines.h>
#include "siad.h"
#include "siad_bsd.h"
extern struct passwd siad_bsd_passwd;
extern char siad_bsd_getpwbuf[];

#include <stdio.h>
#include <ndbm.h>
#include <ctype.h>
#include <paths.h>
#include <limits.h>

#ifdef NLS
#include <locale.h>
#endif

#undef	MSGSTR
#define	MSGSTR(n,s)	GETMSGSTR(MS_BSDPASSWD,(n),(s))

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif


/*
 * This should be the first thing returned from a getloginshells()
 * but too many programs know that it is /sbin/sh.
 *
 * Note:  The default shell as determined by <paths.h>, and as used in
 *        login(1) is /bin/sh, not /sbin/sh.  /sbin/sh might not be in
 *        /etc/shells.  -dlong 91/10/8
 */

#define	DEFSHELL	_PATH_BSHELL

#define	PASSWD		"/etc/passwd"
#define	PTEMP		"/etc/ptmp"
#define	EOS		'\0';
#define MAX_PWD_LENGTH  16
#define progname	"passwd"

siad_chg_shell(collect,username,argc,argv)
int (*collect)();
char *username;
int     argc;
char    *argv[];
{
	struct passwd   *pwd;
	extern	struct passwd   *getpwnam();
        FILE    *tf;
        FILE    *passfp;
        DBM     *dp;
        uid_t   uid, getuid();
        uid_t   euid, geteuid();
        int     i, acctlen, ch, fd, dochfn, dochsh;
        char    *p, *str, *cp, *umsg,
                *getfingerinfo(), *getloginshell(), *getnewpasswd();
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif
	/****** Check if authorized calling program *******/
	if((euid=geteuid()) != 0)
		return(SIADFAIL);
	/****** Check if user is registered with proper uid *****/
	uid=getuid();
        pwd=NULL;
        if(username == NULL)
		username=getlogin();
        if(bsd_siad_getpwnam(username,&siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADSUCCESS)
                pwd = &siad_bsd_passwd;
	if(pwd == NULL) /* should be siad_getpwnam call */
		{
		sia_warning(collect, MSGSTR(UNKUID, "%s: %u: unknown user uid.\n"), progname, uid);
                return(SIADFAIL);
		}
	if((uid != 0) && (uid != pwd->pw_uid))
		{
		sia_warning(collect, MSGSTR(PERMDEN1, "Permission denied.\n"));
		return(SIADFAIL);
		}
	/*printf(MSGSTR(CHANG, "Changing %s for %s.\n"));*/
	/********************************************/
	/* collecting and checking the new password */
	/********************************************/
	if((cp = getloginshell((collect),pwd, uid)) == NULL)
		return(SIADFAIL);
	if(bsd_chg_it((collect),pwd,uid,cp,CHGSHELL) == SIADFAIL)
		return(SIADFAIL);
	else	return(SIADSUCCESS);
}

#define	SHELLSIZ	100

static char *
getloginshell(collect,pwd, u)
	int (*collect)();
	struct passwd *pwd;
	uid_t u;
{
	static char newshell[SHELLSIZ+1];
	static char pmptstr[SHELLSIZ+1];
	char *cp, *valid, *getusershell();
	prompt_t askforpass; /* 0 prompt for collecting shell */
	int             timeout=0;      
        int             rendition=SIAONELINER;
        unsigned char   *title=NULL;
        int             num_prompts=1;
        askforpass.result=(uchar *)newshell;
        askforpass.max_result_length=SHELLSIZ;
        askforpass.min_result_length=0;
        askforpass.control_flags = SIARESANY;
	askforpass.prompt= (uchar *)&pmptstr;

	if (pwd->pw_shell == 0 || *pwd->pw_shell == '\0')
		pwd->pw_shell = DEFSHELL;
	if (u != 0) {
		setusershell();
		do {
			valid = getusershell();
			if (valid == NULL) {
				sia_warning(collect, MSGSTR(RESTRICT, "Cannot change from restricted shell %s\n"),pwd->pw_shell);
				exit(1);
			}
		} while (strcmp(pwd->pw_shell, valid) != 0);
		endusershell();
	}
	sprintf(askforpass.prompt, MSGSTR(OLDSH, "Old shell: %s\nNew shell: "), pwd->pw_shell);
	if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)
                        {
                        return(NULL);
                        }
	cp = index(newshell, '\n');
	if (cp)
		*cp = '\0';
	if (newshell[0] == 0) {
		sia_warning((collect),MSGSTR(LOGINSH, "Login shell unchanged."));
		return(NULL);
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
				sia_warning(collect, MSGSTR(UNACCSH, "%s is unacceptable as a new shell.\n"),newshell);
				exit(1);
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
			strncpy(newshell, valid, sizeof newshell);
			valid = newshell;
		}
		endusershell();
	}
	else
		valid = newshell;
	if (strcmp(valid, pwd->pw_shell) == 0) {
		puts(MSGSTR(LOGINSH, "Login shell unchanged."));
		exit(1);
	}
	if (access(valid, X_OK) < 0) {
		sia_warning(collect, MSGSTR(UNAVAILB, "%s is unavailable.\n"), valid);
		exit(1);
	}
	if (strcmp(valid, DEFSHELL) == 0)
		valid[0] = '\0';
	return (valid);
}
