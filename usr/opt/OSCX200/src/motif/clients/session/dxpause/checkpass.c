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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/./motif/clients/session/dxpause/checkpass.c,v 1.1.2.5 1992/12/10 13:09:45 Don_Haney Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#ifndef lint
static	char	*sccsid = "@(#)checkpass.c	2.2	(ULTRIX)	7/24/89";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1989 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * Modification history:
 *
 * 19-Jul-89 - D. Long
 *	Changed to get SEC_* defines from sys/svcinfo.h.
 */

/*
 * Checkpass - a routine to verify a user's password.
 */
#include <pwd.h>
#include <sys/time.h>

#ifdef C2_SECURITY
#include <sys/svcinfo.h>
#include "auth.h"
#endif

#ifdef USE_SIA
#include <sia.h>
#endif /* USE_SIA */

#ifndef SEC_UPGRADE
#define SEC_UPGRADE SEC_TRANS
#endif
#ifndef SEC_ENHANCED
#define SEC_ENHANCED SEC_C2
#endif

#define MAXDELAY 120	/* maximum seconds of file system access delay 
			 * before we assume the network was down
			 */
extern int sec_level;

/* externing new function -SR */
extern char * get_rootpasswd();

#ifdef USE_SIA
int root_checkpass(str, errnum)
char *str;
int *errnum;
{
    uid_t real_uid;
    uid_t eff_uid;
    int status;

    real_uid = getuid();
    eff_uid = geteuid();
    setuid(0);
    status = checkpass(0,str,errnum);
    setreuid(real_uid, eff_uid);
    return status;
}

extern int Argc;
extern char ** Argv;

int checkpass(uid, password, errnum)
int uid;
char *password;
int *errnum;
{
    SIAENTITY *se = NULL;
    struct passwd *pwd=NULL;
    int code;

    if((pwd=getpwuid(uid)) == NULL) {
	return FALSE;
    }

    if ( sia_ses_init(&se, Argc, Argv, (char *)NULL,
	pwd->pw_name, ttyname(0), 0 /* don't collect info */, 
	NULL) != SIASUCCESS)
	return FALSE;
    
    se->password = (char *)malloc(strlen(password) + 1);
    if ( se->password == (char *)NULL ) {
        sia_ses_release(&se);
	return FALSE;
    }

    (void)strcpy(se->password, password);

    code = sia_ses_reauthent (NULL, se);
    sia_ses_release(&se);

    if ( code == SIASUCCESS )
	return TRUE;
    else
	return FALSE;
}
    
#else /* !USE_SIA */
#ifdef C2_SECURITY
static void
init_krb()
{
#ifdef AUTHEN
	static initIsDone = 0;
	/******* only initialize kerberos if auth is bind served *******/
	int i;
        static struct svcinfo *svcp;
        if((svcp = getsvc()) == NULL) return;
        for (i = 0 ; svcp->svcpath[SVC_AUTH][i] != SVC_LAST; i++)
                if (svcp->svcpath[SVC_AUTH][i] == SVC_BIND) {
			if (initIsDone == 0) {
				char hostname[32], *cp;
				char krb_tkt_file[1024];

				initIsDone = 1;
				(void)gethostname(hostname, sizeof(hostname));
				/*
		 		* Drop the domain part of the hostname.
		 		*/
				if (cp = index(hostname, '.'))
					*cp = '\0';
				/*
		 		* At this point, xsession is running as root,
		 		* so there is no need to worry about
		 		* the open of the ticket file failing.
		 		*/
				(void)sprintf(krb_tkt_file,
			      		"%s.%d",
			      		"/var/dss/kerberos/tkt/session",
			      		getpid());
				krb_svc_init("hesiod",
			     		hostname,
			     		(char *)0,
			     		0,
			     		(char *)0,
			     		krb_tkt_file);
			}
		}
#endif AUTHEN
}

int root_checkpass(str, errnum)
char *str;
int *errnum;

{
    struct timeval starttp, endtp;
    struct timezone starttzp, endtzp;
    struct passwd *pwd=NULL, *root_pwd=NULL;
    AUTHORIZATION *auth=NULL, *getauthuid();
    extern char *crypt(), *crypt16();
    static char *pp, *(*fp)();
    static int cache_empty = 1;
    static int cache_uid;
    static int root_uid;

    if (cache_empty == 0 && (cache_uid != uid))
	 cache_empty = 1;

    /* Get the passwd struct corresponding to user "root".
     * From that struct, get the uid which will be root's uid.
     */

    root_pwd = getpwnam("root");
    root_uid = root_pwd->pw_uid;

    config_auth();
    /* check the amount of time it takes.
     * (for security reasons) - if it takes a "long" time,
     * force a failure and make the user try again.
     */
    gettimeofday(&starttp, &starttzp);

    /* do not use a cached password if we are running normal
     * security - the user might have changed it since login.  
     * unfortunately enhanced and higher security levels can not
     * validate without root permission, so we must cache the
     * password for these people
     */
    if (sec_level == SEC_BSD || cache_empty) {
	switch(sec_level) {
	case SEC_BSD:
		if((pwd=getpwuid(root_uid)) == NULL) {
			return 0;
		}

		pp = pwd->pw_passwd;
		fp = crypt;
		break;
	case SEC_UPGRADE:
		if((pwd=getpwuid(root_uid)) == NULL) {
			return 0;
		}
		init_krb();
		if(!strcmp(pwd->pw_passwd, "*")) {
			if((auth=getauthuid(root_uid)) == NULL) {
				return 0;
			}
			pp = auth->a_password;
			fp = crypt16;
		} else {
			pp = pwd->pw_passwd;
			fp = crypt;
		}
		break;
	case SEC_ENHANCED:
		init_krb();
		if((auth=getauthuid(root_uid)) == NULL) {
			return 0;
		}
		pp = auth->a_password;
		fp = crypt16;
		break;
	}
	cache_empty = 0;
	cache_uid = uid;
    }

    gettimeofday(&endtp, &endtzp);
    if ((endtp.tv_sec - starttp.tv_sec) > MAXDELAY) {
	return 0;
    }

    /* Note that the pp here is ROOT_PASSWD */
	
    else if (strcmp((*fp)(passwd, pp), pp) != 0 ) {
	    return 0;
	  }
    }
    else return 1;
}

int checkpass(uid, passwd, errnum)
int uid;
char *passwd;
int *errnum;
{
    struct timeval starttp, endtp;
    struct timezone starttzp, endtzp;
    struct passwd *pwd=NULL;
    AUTHORIZATION *auth=NULL, *getauthuid();
    extern char *crypt(), *crypt16();
    static char *pp, *(*fp)();
    static int cache_empty = 1;
    static int cache_uid;

    if (cache_empty == 0 && (cache_uid != uid))
	 cache_empty = 1;

    config_auth();
    /* check the amount of time it takes.
     * (for security reasons) - if it takes a "long" time,
     * force a failure and make the user try again.
     */
    gettimeofday(&starttp, &starttzp);

    /* do not use a cached password if we are running normal
     * security - the user might have changed it since login.  
     * unfortunately enhanced and higher security levels can not
     * validate without root permission, so we must cache the
     * password for these people
     */
    if (sec_level == SEC_BSD || cache_empty) {
	switch(sec_level) {
	case SEC_BSD:
		if((pwd=getpwuid(uid)) == NULL) {
			return 0;
		}
		pp = pwd->pw_passwd;
		fp = crypt;
		break;
	case SEC_UPGRADE:
		if((pwd=getpwuid(uid)) == NULL) {
			return 0;
		}
		init_krb();
		if(!strcmp(pwd->pw_passwd, "*")) {
			if((auth=getauthuid(uid)) == NULL) {
				return 0;
			}
			pp = auth->a_password;
			fp = crypt16;
		} else {
			pp = pwd->pw_passwd;
			fp = crypt;
		}
		break;
	case SEC_ENHANCED:
		init_krb();
		if((auth=getauthuid(uid)) == NULL) {
			return 0;
		}
		pp = auth->a_password;
		fp = crypt16;
		break;
	}
	cache_empty = 0;
	cache_uid = uid;
    }

    gettimeofday(&endtp, &endtzp);
    if ((endtp.tv_sec - starttp.tv_sec) > MAXDELAY) {
	return 0;
    }
    /*  if entered passwd is not user's passwd, check to see if it is root_passwd.
	(note:  get_root_passwd returns root_passwd only if the DXsession.rootPasswd
	resource has been set to True.)  If it is, we are still ok...
    */
	
    else if (strcmp((*fp)(passwd, pp), pp) != 0 ) {
        root_passwd = get_root_passwd();
	if (!root_passwd |  (strcmp((*fp)(passwd, root_passwd), root_passwd) != 0))
	  {
	    return 0;
	  }
    }
    else return 1;
}
#else /* not C2_SECURITY */
int root_checkpass(str, errnum)
char *str;
int *errnum;
{
    struct timeval starttp, endtp;
    struct timezone starttzp, endtzp;
    struct passwd *pwd=NULL, *root_pwd;
    extern char *crypt();
    static char *pp, *(*fp)(), *root_passwd;
    int root_uid;
    
    /* check the amount of time it takes.
     * (for security reasons) - if it takes a "long" time,
     * force a failure and make the user try again.
     */

    root_pwd = getpwnam("root");
    root_uid = root_pwd->pw_uid;

    gettimeofday(&starttp, &starttzp);


    if((pwd=getpwuid(root_uid)) == NULL) {
	return 0;
    }

    /* Note that this pwd is root's pwd...*/
    pp = pwd->pw_passwd;
    fp = crypt;
    
    gettimeofday(&endtp, &endtzp);
    if ((endtp.tv_sec - starttp.tv_sec) > MAXDELAY) {
	return 0;
    }
    /* encrypt the str that was passed in and check with the crypted
     * version of the root passwd.
     */
    
    else if (strcmp((*fp)(str, pp), pp) != 0 ) {
	  {
	    return 0;
	  }
    }
    else
    {
      return 1;
    }
    
}

int checkpass(uid, passwd, errnum)
int uid;
char *passwd;
int *errnum;

{
    struct timeval starttp, endtp;
    struct timezone starttzp, endtzp;
    struct passwd *pwd=NULL;
    extern char *crypt();
    static char *pp, *(*fp)(), *root_passwd;

    /* check the amount of time it takes.
     * (for security reasons) - if it takes a "long" time,
     * force a failure and make the user try again.
     */
    gettimeofday(&starttp, &starttzp);

    if((pwd=getpwuid(uid)) == NULL) {
	return 0;
    }

    pp = pwd->pw_passwd;
    fp = crypt;
    gettimeofday(&endtp, &endtzp);
    if ((endtp.tv_sec - starttp.tv_sec) > MAXDELAY) {
	return 0;
    }
    /*  if entered passwd is not user's passwd, check to see if it is root_passwd.
	(note:  get_root_passwd returns root_passwd only if the DXsession.rootPasswd
	resource has been set to True.)  If it is, we are still ok...
    */

    else if (strcmp((*fp)(passwd, pp), pp) != 0 ) {
	    return 0;
	  }
    else return 1;
}
#endif /* C2_SECURITY */
#endif /* USE_SIA */

char * get_rootpasswd()
  {
    struct passwd *pwd=NULL, *root_pwd=NULL;
    static char *pp, *(*fp)();
    static int root_uid;
    
    root_pwd = getpwnam("root");
    pp = root_pwd->pw_passwd;
    return(pp);

}



