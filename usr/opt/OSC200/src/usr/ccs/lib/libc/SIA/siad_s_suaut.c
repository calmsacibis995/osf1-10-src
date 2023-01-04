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
static char *rcsid = "@(#)$RCSfile: siad_s_suaut.c,v $ $Revision: 1.1.16.4 $ (DEC) $Date: 1993/12/16 23:55:40 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_suauthent((*sia_collect),
*				SIAENTITY *entity,
*				char *password,
*				int siastat,
*				int pkgind)
*
* Description: The purpose of this routine is to do session authentication
* for the su command.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_ses_suauthent = __siad_ses_suauthent
#endif
#endif
#include <varargs.h>
#include "siad.h"
#include "siad_bsd.h"


#undef	MSGSTR
#define	MSGSTR(n,s)	GETMSGSTR(MS_BSD,(n),(s))

#define	MAX_PWD_LENGTH	16
#define	MAX_LOGIN_LENGTH	8



static int 
  fail(va_alist)
va_dcl
{
	va_list ap;
	unsigned char *fmt, buff[1025];
	prompt_t prompt;
	int (*collect)();

	va_start(ap);
	collect = va_arg(ap, void *);
	fmt = va_arg(ap, unsigned char *);
	vsprintf(buff, fmt, ap);
	prompt.prompt = buff;
	prompt.control_flags = 0;
	if(collect)
		(*collect)(0, SIAWARNING, "", 1, &prompt);
	va_end(ap);
	return SIADFAIL;
}

int
  siad_ses_suauthent (int (*collect)(), SIAENTITY *entity, int siastat,
		      int pkgind)
{
	prompt_t prompt;
	int i;
	uid_t uid;
	struct passwd *pwd;
	struct passwd *mypwd;
	char myname[SIAMXACCTNAME];

	/* ??? */
	if(geteuid() != 0)
		return SIADFAIL;
	if (siastat==SIADSUCCESS)	/* BSD accepts vouchers*/
		return SIADSUCCESS;
	uid=getuid();
	mypwd=getpwuid(uid); /* this can come from any security mechanism */
	if(mypwd == NULL)
		 return SIADFAIL;
	strcpy(myname,mypwd->pw_name);
	if(!*entity->name) /* no name assume suing to root */
		strcpy(entity->name, "root");
	if(bsd_siad_getpwnam(entity->name, &siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL)
		return fail(collect, MSGSTR(UNKNOWN, "Unknown login: %s"), entity->name);
	else	pwd = &siad_bsd_passwd;
/*
 * Verify superuser access using "system" group.
 */
	if(pwd->pw_uid == 0)
		{
		gid_t gidset[NGROUPS_MAX];
		i = getgroups(NGROUPS_MAX, gidset);
		if (i < 0)
			{
			syslog(LOG_ERR|LOG_AUTH, "getgroups: %m");
			return SIADFAIL;
			}
		while (i--)
			{
			if (gidset[i] == 0)
				goto userok;
			}
		return fail(collect, MSGSTR(NO_PERM, "You do not have permission to su %s\n"), entity->name);
		}
/*
 * Authenticate.
 */

userok:
	if(entity->password != NULL)
		{
		if(strlen(entity->password) > MAX_PWD_LENGTH)
			 return SIADFAIL;
		}
/*
 * No password supplied, prompt interactively.
 */
	else	if(uid != 0)	/* is caller root? */    
			{
			if(collect == NULL)
                        	return SIADFAIL;
			if(pwd->pw_passwd && *pwd->pw_passwd) {
                		entity->password = malloc(SIAMXPASSWORD+1);
                		if(entity->password == NULL)
                        		return SIADFAIL;
				bzero(entity->password, SIAMXPASSWORD+1);
				prompt.prompt = (unsigned char *) MSGSTR(ENTRY_PWD, "Password:");
				prompt.result = (uchar *) entity->password;
				prompt.min_result_length = 0;
				prompt.max_result_length = MAX_PWD_LENGTH;
				prompt.control_flags = SIARESINVIS;
				if ((*collect)(0, SIAONELINER, "", 1, &prompt) != SIACOLSUCCESS)
					return SIADFAIL;
				} 
			}
	if(uid == 0)	{	/* let root su to anyone */
			if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
				return fail(collect, MSGSTR(SORRY, "Sorry\n"));
			return SIADSUCCESS;
			}
/*
 * No password required, make sure supplied password is empty.
 */
        if((entity->password == NULL) && !(pwd->pw_passwd && *pwd->pw_passwd))
                {
                if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
			return fail(collect, MSGSTR(SORRY, "Sorry\n"));
                return SIADSUCCESS;
                }
/*
 * Password required and non-empty password supplied. Check for correct password.
 */
        if(entity->password && pwd->pw_passwd && strlen(pwd->pw_passwd) >= 2 && /*DAL001*/
            !strcmp(pwd->pw_passwd, crypt(entity->password, pwd->pw_passwd)))
                {
                if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
			return fail(collect, MSGSTR(SORRY, "Sorry\n"));
                return SIADSUCCESS;
                }
        else	{
		return fail(collect, MSGSTR(SORRY, "Sorry\n"));
		}
return fail(collect, MSGSTR(SORRY, "Sorry\n"));
}
