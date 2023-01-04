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
static char *rcsid = "@(#)$RCSfile: siad_c_pass.c,v $ $Revision: 1.1.14.4 $ (DEC) $Date: 1993/08/04 21:22:07 $";
#endif
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

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
/*
 * Modify a field in the password file (either password, login shell, or
 * gecos field).  This program should be suid with an owner with write
 * permission on /etc/passwd.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak bsd_chg_it = __bsd_chg_it
#pragma weak bsd_replace = __bsd_replace
#pragma weak bsd_ulimit = __bsd_ulimit
#pragma weak siad_chg_password = __siad_chg_password
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

siad_chg_password(collect,username,argc,argv)
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
	uid=getuid(); /* what uid is asking to make the change? */
        pwd=NULL;
        if(bsd_siad_getpwnam(username,&siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADSUCCESS)
                pwd = &siad_bsd_passwd;
	if(pwd == NULL) /* could not find user in local BSD database */
		{
		sia_warning(collect, MSGSTR(UNKUID, "%s: %u: unknown user uid.\n"), progname, uid);
                return(SIADFAIL);
		}
	if((uid != 0) && (uid != pwd->pw_uid))
		{
		sia_warning(collect, MSGSTR(PERMDEN1, "Permission denied.\n"));
		return(SIADFAIL);
		}
	sia_warning(collect, MSGSTR(CHANG, "Changing %s for %s.\n"),"password",username);
	/********************************************/
	/* collecting and checking the new password */
	/********************************************/
	if((cp = getnewpasswd((collect),pwd, uid)) == NULL)
		return(SIADFAIL);
	if(bsd_chg_it((collect),pwd,uid,cp,CHGPASSWD) == SIADFAIL)
		return(SIADFAIL);
	else	return(SIADSUCCESS);
}


bsd_chg_it(collect,pwd,uid,cp,it)
int (*collect)();
register struct passwd *pwd;
uid_t   uid;
char *cp;
int it;
	{
        FILE    *tf;
        FILE    *passfp;
        DBM     *dp;
        uid_t   euid, geteuid();
        int     i, acctlen, ch, fd;
	int 	didchg=0;
        static  char pwstr[1024];
        char    *p, *str, *getpwline();
	char warnstr[80];
	int     local=FALSE, hash=FALSE;
	datum   key;

	if((euid=geteuid()) != 0)
		return(SIADFAIL);
	/********************************************/
	/* Time to change whatever needs changing   */
	/* in either the /etc/passwd or the database*/
	/********************************************/
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
	(void) signal(SIGTSTP, SIG_IGN);
	(void) umask(0);
	if ((fd = open(PTEMP, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
		if (errno == EEXIST)
			sia_warning(collect, MSGSTR(PWDFILEBUSY, "%s: password file busy - try again.\n"), progname);
		else {
			perror((char *)NULL);
			sia_warning(collect, "change password: %s: ", PTEMP);
		}
		return(SIADFAIL);
	}
	if ((tf = fdopen(fd, "w")) == NULL) {
		sia_warning(collect, MSGSTR(FDOPNFAIL, "%s: fdopen failed.\n"), progname);
		return(SIADFAIL);
	}
	if ((dp = dbm_open(PASSWD, O_RDWR, 0644)) == NULL) {
		if(errno == ENOENT)
			sprintf(warnstr,MSGSTR(NODB, "Hashed database not in use, only %s text file updated.\n"), PASSWD);
		else {
			sprintf(warnstr,MSGSTR(WARN1, "Warning: dbm_open failed: %s: "), PASSWD);
		}
	SIALOG((SIA_MSG_LOGERROR,"ERROR"),warnstr);
	sia_warning((collect),warnstr);
	}
	else	if (flock(dp->dbm_dirf, LOCK_SH) < 0) {
		SIALOG((SIA_MSG_LOGALERT,"ALERT"),(MSGSTR(WARN2, "Warning: lock failed")));
		dbm_close (dp);
		dp = NULL; 
	}
	bsd_ulimit(RLIMIT_CPU);
	bsd_ulimit(RLIMIT_FSIZE);
	/*****************Ultrix 4.2 /etc/passwd update***************
	 *
   	 * Fix from V4.2 Ultrix to maintain the (+,-,+:) YP syntax in the
	 * original password file. The code from OSF assumed that getpwents
	 * would provide all passwd file strings. Not true for YP syntax.
	 *
	 * Copy the original PASSWD "/etc/passwd" file substituting only
	 * the password to be modified
	 *************************************************************/
	 if ((passfp = fopen("/etc/passwd", "r")) == NULL) {
		strcpy(warnstr, MSGSTR(WARN1, "Warning: can not open /etc/passwd:n"));
		SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"), warnstr);
		sia_warning(collect, warnstr);
	}
	acctlen = strlen(pwd->pw_name);

	/**************************************************************************/
	/* Verify that the passwd entry exists in the local /etc/passwd file AND  */
	/* in the hashed passwd database file (if one exists).                    */
	/*                                                                        */
	/* If the hashed database exists:                                         */
	/*         Return SIADFAIL if the entry not found in BOTH the /etc/passwd */
	/*         file and the hashed database.                                  */
	/*                                                                        */
	/* If the hashed database doesn't exist:                                  */
	/*         Return SIADFAIL if the entry is not found in the /etc/passwd   */
	/*         file.                                                          */
	/**************************************************************************/
	
	/* Set local=TRUE if the user is found in the local /etc/passwd file */
	while (str=getpwline(pwstr, sizeof pwstr, passfp)) {
	         i = strcspn(pwstr, ":\n");
	         if (i == acctlen && !strncmp(pwd->pw_name, pwstr, i)) {
	                 local = TRUE;
	         }
	}

	rewind(passfp);

	/* Set hash=TRUE if the user is found in the hashed passwd database file */
	if (dp != NULL) {
	         for (key = dbm_firstkey(dp); key.dptr != NULL; key = dbm_nextkey(dp)) { 
	                 if (strncmp(pwd->pw_name, key.dptr,key.dsize) == 0) {
	                          hash = TRUE;
	                          break;
	                 }
	         }	
	}

	/* If the user is found in the hashed file, but not in the local file, or vice versa, */
	/* report the mismatch.                                                               */
	if (((dp != NULL) && (local == TRUE) && (hash == FALSE)) 
	    || ((dp != NULL) && (local == FALSE) && (hash == TRUE))) {
	         sprintf (warnstr, MSGSTR(SIA_MSG_MISMATCH, 
	          "Hashed passwd database does not match the %s file.  Rerun 'mkpasswd %s'.\n"),
			  PASSWD, PASSWD);
	         sia_warning((collect),warnstr);
	         goto out;
	}
	
	/* If the user is in neither the local file nor the hashed file (if it exists),   */
	/* report the error and remind the user to use 'yppasswd' for NIS entries.        */
	if (((dp == NULL) && (local == FALSE))
	    || ((dp != NULL) && (local == FALSE) && (hash == FALSE))) {
	         sprintf(warnstr, MSGSTR(SIA_MSG_NIS, 
        "\nUser %s not found in the local %s file.  Use 'yppasswd' to change an NIS passwd entry.\n"),
			 pwd->pw_name, PASSWD);
	         sia_warning((collect),warnstr);
	         goto out;
	}

	while(str=getpwline(pwstr, sizeof pwstr, passfp)) {
                i = strcspn(pwstr, ":\n");
                if(i == acctlen && !strncmp(pwd->pw_name, pwstr, i)) {
	/* with the new Ultrix scheme this should never happen */
			if (uid && uid != pwd->pw_uid) {
				
				sia_warning(collect, MSGSTR(PERMDEN2, "%s: permission denied.\n"), progname);
				goto out;
			}
			switch(it)
				{
				case CHGFINGER: pwd->pw_gecos = cp;
					break;
				case CHGSHELL: pwd->pw_shell = cp;
					break;
				case CHGPASSWD: pwd->pw_passwd = cp;
					break;
				}
			if (pwd->pw_gecos[0] == '*')	/* ??? */
				pwd->pw_gecos++;
			bsd_replace(dp, pwd);
			fprintf(tf, "%s:%s:%d:%d:%s:%s:%s\n",
			pwd->pw_name,
			pwd->pw_passwd,
			pwd->pw_uid,
			pwd->pw_gid,
			pwd->pw_gecos,
			pwd->pw_dir,
			pwd->pw_shell);
			didchg=1;
			}
		else
			fputs(pwstr, tf);
	}
	endpwent();
	(void)fclose(passfp);
	if(!didchg)     /* no change has occured */
                {
                sia_warning(collect, MSGSTR(UNKUID, "%s: %u: unknown user uid.\n"),
progname, uid);
                goto out;
                }
	if (dp && dbm_error(dp))
		{
		SIALOG(MSGSTR(SIA_MSG_LOGERROR, "ERROR"), MSGSTR(WARN3, "Warning: dbm_store failed.\n"));
		}
	(void) fflush(tf);
	if (ferror(tf)) {
		sprintf(warnstr, MSGSTR(WARN4, "Warning: %s write error, %s not u pdated.\n"), PTEMP, PASSWD);
		SIALOG(MSGSTR(SIA_MSG_LOGERROR, "ERROR"), warnstr);
		sia_warning(collect, warnstr);
		goto out;
	}
	(void)fclose(tf);
	if (dp != NULL) {
	        if (flock(dp->dbm_dirf,LOCK_UN) != 0){
		       SIALOG((SIA_MSG_LOGALERT,"ALERT"),(MSGSTR(WARN2, "Warning: lock failed")));
	        }
		dbm_close (dp);
		dp = NULL;
	}
	if (rename(PTEMP, PASSWD) < 0) {
		perror(progname);
	out:
		(void)unlink(PTEMP);
		return(SIADFAIL);
	}
	return(SIADSUCCESS);
}
bsd_ulimit(lim)
	int	lim;
{
	struct rlimit rlim;

	rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
	(void)setrlimit(lim, &rlim);
}

/*
 * Replace the password entry in the dbm data base with pwd.
 */
bsd_replace(dp, pwd)
	DBM *dp;
	struct passwd *pwd;
{
	datum key, content;
	register char *cp, *tp;
	char buf[SIABUFSIZ];

	if (dp == NULL)
		return;

	cp = buf;
#define	COMPACT(e)	tp = pwd->e; while (*cp++ = *tp++);
	COMPACT(pw_name);
	COMPACT(pw_passwd);
	bcopy((char *)&pwd->pw_uid, cp, sizeof (int));
	cp += sizeof (int);
	bcopy((char *)&pwd->pw_gid, cp, sizeof (int));
	cp += sizeof (int);
	bcopy((char *)&pwd->pw_quota, cp, sizeof (int));
	cp += sizeof (int);
	COMPACT(pw_comment);
	COMPACT(pw_gecos);
	COMPACT(pw_dir);
	COMPACT(pw_shell);
	content.dptr = buf;
	content.dsize = cp - buf;
	key.dptr = pwd->pw_name;
	key.dsize = strlen(pwd->pw_name);
	dbm_store(dp, key, content, DBM_REPLACE);
	key.dptr = (char *)&pwd->pw_uid;
	key.dsize = sizeof (int);
	dbm_store(dp, key, content, DBM_REPLACE);
}

static char *
getnewpasswd(collect,pwd, u)
	int (*collect)();
	register struct passwd *pwd;
	uid_t u;
{
	prompt_t askforpass; /* 0 prompt for collecting password */
	time_t	salt, time();
	int	c, i, insist, len;
	char	*pw;	
	char	 pwbuf[PASS_MAX+1], pwcopy[PASS_MAX+1], saltc[2],
		*crypt(), *getpass();
	int		timeout=0;	/* 20 seconds */
	int		rendition=SIAONELINER;
	unsigned char   *title=NULL;
	int             num_prompts=1;
	pw=pwbuf;
	askforpass.result=(uchar *)pwbuf;
	askforpass.max_result_length=MAX_PWD_LENGTH;
	askforpass.min_result_length=0;
	askforpass.control_flags = SIARESINVIS;
	if (pwd->pw_passwd[0] && u != 0) {
		askforpass.prompt= (unsigned char *) MSGSTR(OLDPWD, "Old password:");
		if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS)	/*DAL002*/
			{
			sia_warning(collect, MSGSTR(BADTTY, "No controlling tty."));
			return(NULL);
			}
		pw = crypt(pw, pwd->pw_passwd);
		if (strcmp(pw, pwd->pw_passwd) != 0) {
			sia_warning(collect, MSGSTR(SORRY, "Sorry."));
			return(NULL);
		}
	}
for(;;) {
	askforpass.prompt= (unsigned char *) MSGSTR(NEWPWD, "New password:");
	if((*collect)(timeout,rendition,title,num_prompts,&askforpass) != SIACOLSUCCESS ||
	    !*pwbuf)		/*DAL001*/
		{
		sia_warning(collect, MSGSTR(PWDUNCH, "Password unchanged."));
		return(NULL);
		}
	if (strcmp(pwbuf, pwcopy)) {
		insist = 1;
		(void)strcpy(pwcopy, pwbuf);
	}
	else if (++insist == 4)
		break;
	if ((len = strlen(pwbuf)) <= 4)
		{
		sia_warning(collect, MSGSTR(LONGER, "Please enter a longer password."));
		}
	else {
		for (pw = pwbuf; *pw && islower(*pw); ++pw);
		if (*pw)
			break;
		sia_warning(collect, MSGSTR(SUGGEST, "Please don't use an all-lower case password.\nUnusual capitalization, control characters or digits are suggested."));
		}
	}
	askforpass.prompt= (unsigned char *) MSGSTR(RETYPR, "Retype new password:");
	bzero(pwbuf,sizeof(pwbuf));
	if((*collect)(timeout,rendition,title,num_prompts,&askforpass)  != SIACOLSUCCESS)
		return(NULL);
	if((strcmp(pwbuf, pwcopy)) || (strlen(pwbuf) != len)) {
		sia_warning(collect, MSGSTR(MISMATCH, "Mismatch - password unchanged."));
		return(NULL);
		}
	(void)time(&salt);
	salt = 9 * getpid();
	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for (i = 0; i < 2; i++) {
		c = saltc[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		saltc[i] = c;
	}
	return(crypt(pwbuf, saltc));
}
/*
 * Internal function to safely get a line from the named stream.  If
 * the line is too long to fit into the buffer it is thrown away and a
 * new line is fetched until successful or end-of-file.
 */
static char *
getpwline(strng, len, file)
char *strng;
int len;
FILE *file;
{
        register char *s;
        int c;

        while(s=fgets(strng, len, file))
                if(strchr(strng, '\n'))
                        break;
                else
                        while((c=getc(file)) != EOF && c != '\n') ;
        return s;
}
