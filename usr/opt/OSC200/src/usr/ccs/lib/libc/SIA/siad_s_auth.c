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
static char *rcsid = "@(#)$RCSfile: siad_s_auth.c,v $ $Revision: 1.1.15.7 $ (DEC) $Date: 1993/12/16 23:55:38 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_authent((*sia_collect),
*				SIAENTITY *entityhdl)
*
* Description: The purpose of this routine is to do session authentication.
* If the collect function pointer is NULL this is a non-interactive request.
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
#pragma weak siad_ses_authent = __siad_ses_authent
#endif
#endif
#include <ttyent.h>
#include <stdio.h>
#include "pathnames.h"
#include "siad.h"
#include "siad_bsd.h"

#undef	MSGSTR
#define	MSGSTR(n,s)	GETMSGSTR(MS_BSD,(n),(s))

#define	MAX_RETRIES	10
#define	START_BACKOFF	3
#define	BACKOFF_TIME	5
#define	MAX_PWD_LENGTH	16
#define	MAX_LOGIN_LENGTH	8
#define	PTYS	"ptys"

static struct passwd nobody =
	{"nobody", "nologin", 100, 100, 0, 0, "nobody", "/tmp", "/bin/false"};


int
  siad_ses_authent (int (*collect)(), SIAENTITY *entity, int siastat, 
		    int pkgind)
{
	prompt_t prompts[2];
	char *logintitle;
	char *tty, *s;
	struct ttyent *ts;
	int i, c;
	struct passwd *pwd = NULL;
	FILE *fp;

/*
 * Return immediately if already successful.
 * We will check out the account and pwd information in
 * siad_ses_estab()
 */
	if((siastat==SIADSUCCESS) && (geteuid() == 0))/* BSD accepts vouchers*/
		return SIADSUCCESS;
	if(entity==NULL)	     /* no entity no processing */
		return (SIADFAIL | SIADSTOP);
	if((entity->acctname != NULL) || (entity->pwd != NULL)) 
		return (SIADFAIL | SIADSTOP); /* not determined yet */
/*
 * The layered/switched processing of SIA allows previous configured 
 * security mechanism the option of collecting both the loginname and
 * the password. To avoid duplication of prompting to the user this 
 * authentication routine first checks for the existence of either or 
 * both the loginname or password. If both do not exist use form collection
 * to collect both. If no collect routine is available the
 * loginname and password must have been precollected.
 */
/* RETRYING IS DONE BY THE COMMAND CALLING THE SIA ROUTINES NOT AT THIS LEVEL */

/* Can we collect anything */


	if((collect != NULL) && entity->colinput) {	/* can we collect anything? */
		if (entity->name == NULL && collect == sia_collect_trm)
			{		  /* no name on terminal--get name */
			entity->name = malloc(SIANAMEMIN+1);
			if(entity->name == NULL)
				return SIADFAIL;
			prompts[0].prompt= (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
			prompts[0].result = (unsigned char *) entity->name;
			prompts[0].min_result_length = 1;
			prompts[0].max_result_length = MAX_LOGIN_LENGTH;
			prompts[0].control_flags = 0;
			if((*collect)(240, SIAONELINER, "", 1, &prompts) != SIACOLSUCCESS)
				return(SIADFAIL | SIADSTOP); /* could not collect name */
			}
		if(entity->name == NULL)  /* name collected? */
			{		  /* must first allocate a name big */
					  /* enough for all machanisms      */
			entity->name = malloc(SIANAMEMIN+1);
			if(entity->name == NULL)
				return SIADFAIL;
			if(entity->password == NULL) {  /* no password collected */
				/* use SIA form collection to collect both */
				entity->password = malloc(SIAMXPASSWORD+1);
				if(entity->password == NULL)
					return SIADFAIL;
				bzero(entity->password, SIAMXPASSWORD+1);
				logintitle = NULL;
				prompts[0].prompt = (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
                                prompts[0].result = (unsigned char *) entity->name;
                                prompts[0].min_result_length = 1;
                                prompts[0].max_result_length = MAX_LOGIN_LENGTH;
                                prompts[0].control_flags = 0;
				prompts[1].prompt= (unsigned char *) MSGSTR(ENTRY_PWD, "Password:");
                                prompts[1].result = (uchar *) entity->password;
                                prompts[1].min_result_length = 0;
                                prompts[1].max_result_length = MAX_PWD_LENGTH;
                                prompts[1].control_flags = SIARESINVIS;
				if((*collect)(0, SIAFORM, logintitle, 2, &prompts) != SIACOLSUCCESS)
					return(SIADFAIL | SIADSTOP); /* could not collect name and password */
				}
			else { /* need to collect only the name STRANGE CASE */
				prompts[0].prompt= (unsigned char *) MSGSTR(ENTRY_LOGIN, "login: ");
                        	prompts[0].result = (unsigned char *) entity->name;
                        	prompts[0].min_result_length = 1;
                        	prompts[0].max_result_length = MAX_LOGIN_LENGTH;
                        	prompts[0].control_flags = 0;
                        	if((*collect)(240, SIAONELINER, "", 1, &prompts) != SIACOLSUCCESS)
					return(SIADFAIL | SIADSTOP); /* could not collect name and password */
				}
			}
		else 	if(entity->password == NULL) 
			{ /* need to collect only the password */
			entity->password = malloc(SIAMXPASSWORD+1);
			if(entity->password == NULL)
				return SIADFAIL;
			bzero(entity->password, SIAMXPASSWORD+1);
			if(entity->name[0] == NULL) /* we must have a name */
				return SIADFAIL;
			if(bsd_siad_getpwnam(entity->name,&siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL)
				pwd =  &nobody;
			else    pwd = &siad_bsd_passwd;
			prompts[0].prompt = (unsigned char *) MSGSTR(ENTRY_PWD, "Password:");
                        prompts[0].result = (uchar *) entity->password;
                        prompts[0].min_result_length = 0;
                        prompts[0].max_result_length = MAX_PWD_LENGTH;
                        prompts[0].control_flags = SIARESINVIS;
			if (pwd->pw_passwd && pwd->pw_passwd[0]) /* only ask if we must */
                        if ((*collect)(240, SIAONELINER, "", 1, &prompts) != SIACOLSUCCESS)
					return(SIADFAIL | SIADSTOP); /* could not collect name and password */
                	}
		}
/* At this point we must have a name and may have a password if the account
 * requires a password  */
	if(entity->password != NULL)
		{
		if(strlen(entity->password) > MAX_PWD_LENGTH)
			return SIADFAIL;
		}
	if(entity->name[0] == NULL) /* we must have a name */
		return SIADFAIL;
	if (pwd == NULL)
		{
		if(bsd_siad_getpwnam(entity->name,&siad_bsd_passwd, &siad_bsd_getpwbuf,SIABUFSIZ) == SIADFAIL)
			pwd =  &nobody;
		else    pwd = &siad_bsd_passwd;
		}
/*
 * Authenticate.
 */
		if(pwd->pw_uid == 0 && entity->tty)
/*
 * Check secure terminal.
 */
			{
/*
 * Special case for pseudo-tty's.
 */
			tty = entity->tty;
			if(strlen(tty) == 10 && !strncmp(tty, "/dev/tty", 8) &&
			   ((tty[8] >= 'a' && tty[8] <= 'z' && tty[8] != 'd') ||
			   (tty[8] >= 'A' && tty[8] <= 'Z')))
				ts = getttynam(PTYS);
			else
				ts = getttynam(tty);
			if(!ts || !(ts->ty_status&TTY_SECURE))
				{
				syslog(LOG_WARNING|LOG_AUTH,
				    MSGSTR(REFUSEDLOG, "ROOT LOGIN REFUSED %s"),
					tty);
				if(collect != NULL)
					{
					prompts[0].prompt= (unsigned char *) MSGSTR(REFUSEDON,
				    	"root login refused on this terminal.");
					(void) (*collect)(0, SIAWARNING, "", 1, &prompts);
					}
				return SIADFAIL;
				}
			}
/*
 * No password required, make sure supplied password is empty.
 */
		if(!pwd->pw_passwd || !*pwd->pw_passwd) /* if no password required */
			{
			if(!entity->password || !*entity->password) /* then no password should be supplied */
				{
				if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
					return SIADFAIL;
				else	return SIADSUCCESS;
				}
			else	return SIADFAIL;
			}
		if(!entity->password || !*entity->password)
			 return SIADFAIL;
/*
 * Password required and non-empty password supplied. Check for correct password.
 */		
                if(entity->password && pwd->pw_passwd && strlen(pwd->pw_passwd) >= 2 &&         /*DAL001*/
		    !strcmp(pwd->pw_passwd, crypt(entity->password, pwd->pw_passwd)))
			{
			if(sia_make_entity_pwd(pwd,entity) == SIAFAIL)
				return SIADFAIL;
			else	{
				return SIADSUCCESS;
				}
			}
		else
			{
			return SIADFAIL;
			}
return SIADFAIL;
}
