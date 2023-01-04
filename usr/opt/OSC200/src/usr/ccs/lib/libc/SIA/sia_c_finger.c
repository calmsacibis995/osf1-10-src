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
static char *rcsid = "@(#)$RCSfile: sia_c_finger.c,v $ $Revision: 1.1.13.3 $ (DEC) $Date: 1993/08/04 21:19:42 $";
#endif
/*****************************************************************************
* Usage:  int * sia_chg_finger( (*sia_collect),username)
*
* Description: The purpose of this routine is to implement a finger changing
* process which can handle one or more different security mechanisms. This 
* routine first checks the available mechanisms to determine which have
* the calling entity registered, via getpwbynam. Once this has been determined 
* the user will be given a choice of changing the various fingers. 
* The common case is that a user will only be registered with one security
* mechanism. In this case the one mechanism routine will be called.
*
* Parameter Descriptions: 
*
*	Param1: sia_collect
*       Usage: Collection of  parameters from the calling program/user.
*	Syntax: n.a.
*
* Assumed Inputs: None
*
* Success return: SIASUCCESS accompanied with a non_NULL passport handle.
*
* Error Conditions and returns otherthan parameter syntax checks listed above:
*
* 	Condition1: loader failure to load libsia or a package within libsia
*	Return: SIALDRERR
*
* 	Condition2: failure to properly authenticate
*	Return: SIAFAIL
*****************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_chg_finger = __sia_chg_finger
#endif
#endif
#ifdef uname
#undef uname
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_chg_finger (int (*collect)(), char *username, int argc, char *argv[])
{
        int siaderr= (SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=SIAFAIL;                  /* sia status between mechanism calls */
	int pkgind=0;                   /* package index */
	int siacollerr=0;
	int i=0;
	struct passwd	*pwd;
	int sia_chg_finger_array[8];
	int collindex=0;
	int (*pfp)();
/******* collection parameter initialization ******/
	struct prompt_t  prompts[8];
	int timeout=0;
	int rendition=0;
	int num_prompts=0;
	char uname[100];
	char menutitle[100];
	char promptstrs[8][100];

	SIATHREADLOCK(SIA_CHANGE_LOCK)
	if(collect == NULL)
                return(SIAFAIL);
	bzero(promptstrs,sizeof(promptstrs));
	bzero(prompts,sizeof(prompts));
	bzero(menutitle,sizeof(menutitle));
/*****************************************************************************************/
        if(sia_init() == SIAFAIL)
                {
                SIATHREADREL(SIA_CHANGE_LOCK)
                return(SIAFAIL);
                } 
	if(username == NULL)
		{
		pwd=getpwuid(getuid());
		if(pwd == NULL)
			{
			SIATHREADREL(SIA_CHANGE_LOCK)
                	return(SIAFAIL);
                	}
		else	{
			username=uname;
			strcpy(username,pwd->pw_name);
			}
		}
        while(pkgind < SIASWMAX && sia_matrix[SIA_CHG_FINGER][pkgind].pkgnam != NULL)	/* first check if user is registered */
                {
                if(sia_matrix[SIA_CHK_USER][pkgind].fp == NULL)
                        {
			(void) sia_setupfp(SIA_CHK_USER, pkgind);
                        }
                pfp=sia_matrix[SIA_CHK_USER][pkgind].fp;
                if(pfp != NULL)
                        {
                        if ((siaderr = (*pfp)(username,CHGFINGER)) == SIADSUCCESS)
                                {
                                /* save package index mapping */
                                /* and creat the prompt string */
                                sia_chg_finger_array[collindex]=pkgind;
                                sprintf(promptstrs[collindex],"%s", sia_matrix[SIA_CHG_FINGER][pkgind].pkgnam);
                                collindex++;
                                }
                        }
		pkgind++;
		}
	if(collindex == 0)	/* no change finger configuration */
		{
/* Changed to report error to user. DAL001
		SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_NOCHGFINGER,"No selection configured for changing finger "));
*/
		sia_warning(collect, GETMSGSTR(MS_BSDPASSWD, UNKUSER, "%s: %s: unknown user.\n"), "Change finger", username);
		SIATHREADREL(SIA_CHANGE_LOCK)
		return(SIAFAIL);
		}
	if(collindex > 1)      /* more than one option */
		{
		pkgind=-1;
		sprintf(menutitle, MSGSTR(SIAMULTMECHMENU, "You are registered with the following security mechanisms\n"));
		rendition = SIAMENUONE;
		timeout = SIAONEMIN;
		num_prompts=collindex;
		collindex--;
		for(i=0;i<=collindex;i++)
			{
			prompts[i].control_flags = 0; /* non-NULL result pointer => selected */
			prompts[i].prompt=(unsigned char *) promptstrs[i];
			}
		if(((*collect)(timeout,rendition,menutitle,num_prompts,prompts)) == SIACOLSUCCESS)
			{
			for(i=0;i<=collindex;i++)
				{
				if(prompts[i].result != NULL)
					{
					pkgind=sia_chg_finger_array[i];
					}
				}
			}	
		if(pkgind == -1)
			{
			SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_BADCOLLERR,"Error in parameter collection"));
                        SIATHREADREL(SIA_CHANGE_LOCK)
                        return(SIAFAIL);
			}
		}
	else	{	/* only one choice */
		pkgind=sia_chg_finger_array[0];
		}
	if(sia_matrix[SIA_CHG_FINGER][pkgind].fp == NULL)
		{	/* load was successful once already for siad_chk_user */
		if (sia_setupfp(SIA_CHG_FINGER, pkgind) == SIAFAIL)
			{
			SIATHREADREL(SIA_CHANGE_LOCK);
			return SIAFAIL;
			}
                }
        siaderr = (*sia_matrix[SIA_CHG_FINGER][pkgind].fp)((collect),username,argc,argv);
        if(siaderr == SIADFAIL)
                {
                SIALOG(MSGSTR(SIA_MSG_LOGERROR,"ERROR"),MSGSTR(SIA_MSG_CHGFINGERR,"Failure to change finger for %s"),username);
                SIATHREADREL(SIA_CHANGE_LOCK)
                return(SIAFAIL);
                }
        else    {
                SIALOG(MSGSTR(SIA_MSG_LOGEVENT,"EVENT"),MSGSTR(SIA_MSG_CHGFINGOOD,"Successful finger change for %s"),username);
                SIATHREADREL(SIA_CHANGE_LOCK)
                return(SIASUCCESS);
                }
}
