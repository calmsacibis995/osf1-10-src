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
static char *rcsid = "@(#)$RCSfile: sia_s_init.c,v $ $Revision: 1.1.11.4 $ (DEC) $Date: 1993/08/04 21:21:18 $";
#endif
/*****************************************************************************
* Usage:  int *sia_ses_init( 
*				SIAENTITY *(*entityhdl);
*				int	argc;
*				char	*argv[];
*				char    *hostname;
*				char    *username;
*				char 	*tty;
*				int	colinput;
*				char	*gssapi; )
*
* Description: The purpose of this routine is to do initialization of the
* mechanisms per session. This routine will be called at the beginning of
* each authentication process. Consequently the security mechanisms can use
* the siad_ses_init routine to do general setup for the session.
*
*	Internal return codes: 	
*	(SUCCESS | CONTINUE) - used to allow other authentication mechanisms
*				the ability to authenticate. This case might
*				happen when a user is registered both locally
*				and distributed.
*
*	(SUCCESS | STOP) - Used when no other authentication mechanisms are 
*				either necessary and/or allowed. This case
*				might happen when a user like root is only
*				registered locally.
*
*	(FAILURE | CONTINUE) - Used to allow other authentication mechanisms
*				 the ability to authenticate. This case would
*				be used when a user has no local registry but 
*				does have a distributed registry.
*
*	(FAILURE | STOP) - Absolute denial of authentication. This case is used
*				when a local ACL refuses authentication of a 
*				particular entity.
*
* Parameter Descriptions:  none
*
*
* Assumed Inputs: sia_matrix is properly configured with installed security
*			mechanisms.
*
* Success return: SIASUCCESS .
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
#pragma weak sia_ses_init = __sia_ses_init
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int 
sia_ses_init (SIAENTITY *(*entityhdl), int argc, char *argv[], 
	      char *hostname, char *username, char *tty, int colinput, 
	      char *gssapi)
{
	
/*****************************************************************************************/
        int siaderr=(SIADFAIL | SIADSTOP);   /* SIAD security mechanism returned value */
        int siastat=0;                  /* sia status between mechanism calls */
        int pkgind=0;                   /* package index */
	int i,j;		
        SIAENTITY *entity=NULL;     /* For storing this procs entity */
	uchar_t amask[AUDIT_MASK_LEN];	/* for setting up audit information */
/*****************************************************************************************/
	SIATHREADLOCK(SIA_SES_LOCK); 
	if(sia_init() == SIAFAIL)	/* do sia system initialization */
		{
		SIATHREADREL(SIA_SES_LOCK);
		return(SIAFAIL);
		}
	if(argc == 0)		/* minimally identify calling command */
		{
                SIATHREADREL(SIA_SES_LOCK);
                return(SIAFAIL);
                }
	/* set up for trusted audit events only */
	bzero((void *)amask, sizeof amask);
	for (i = MIN_TRUSTED_EVENT;  i <= MAX_TRUSTED_EVENT;  i++) {
		A_PROCMASK_SET(amask, i, 1, 1);
	}
	/* add error checking here? */
	(void) audcntl(SET_PROC_AMASK, (char *)amask, sizeof amask, 0L, 0L, 0L);
	(void) audcntl(SET_PROC_ACNTL, (char *)0, 0L, AUDIT_AND, 0L, 0L);

        if(*entityhdl == NULL)
                {                       /* setup an entity if none exists */
                if(sia_alloc_entity(entityhdl)==SIAFAIL)
                        {
                        SIATHREADREL(SIA_SES_LOCK);
                        return(SIAFAIL);
                        }
		}
	entity= *entityhdl;
	if(hostname != NULL)
		{
		entity->hostname = malloc(strlen(hostname) + 1);
		if(entity->hostname == NULL)
			{
			sia_free_entity(entity,SIAFREEALL);
			SIATHREADREL(SIA_SES_LOCK);
			return(SIAFAIL);
			}
		strcpy(entity->hostname,hostname);
		}
	if(username != NULL)
                {
                entity->name = malloc(strlen(username) + 1);
                if(entity->name == NULL)
                        {
                        sia_free_entity(entity,SIAFREEALL);
                        SIATHREADREL(SIA_SES_LOCK);
                        return(SIAFAIL);
                        }
		strcpy(entity->name,username);
                }
	if(tty != NULL)
                {
                entity->tty = malloc(strlen(tty) + 1);
                if(entity->tty == NULL)
                        {
                        sia_free_entity(entity,SIAFREEALL);
                        SIATHREADREL(SIA_SES_LOCK);
                        return(SIAFAIL);
                        }
		strcpy(entity->tty,tty);
                }
	if(gssapi != NULL)
		{
		entity->gssapi = gssapi;
		}
	entity->argc = argc;
	if(argc)
		{
		if((entity->argv =  (char **) calloc(argc, sizeof (char *))) == NULL)
                        {
			entity->argc=0;
			sia_free_entity(entity,SIAFREEALL);
                        SIATHREADREL(SIA_SES_LOCK);
                        return(SIAFAIL);
                        }
		for(i=0; i < argc; i++)
			{
			entity->argv[i] = malloc(strlen(argv[i]) + 1);
			if(entity->argv[i] == NULL)
				{
				for(j=0; j<i; j++)
					free(entity->argv[j]);
				entity->argc=0;
				sia_free_entity(entity,SIAFREEALL);
                        	SIATHREADREL(SIA_SES_LOCK);
                        	return(SIAFAIL);
				}
			strcpy(entity->argv[i], argv[i]);
			}
		}
	entity->colinput=colinput;
        while(pkgind < SIASWMAX && sia_matrix[SIA_SES_INIT][pkgind].pkgnam != NULL)
                {
		if(sia_matrix[SIA_SES_INIT][pkgind].fp == NULL)
			{
			(void) sia_setupfp(SIA_SES_INIT, pkgind);
			}
		if(sia_matrix[SIA_SES_INIT][pkgind].fp!= NULL)
			{
			siaderr = (*sia_matrix[SIA_SES_INIT][pkgind].fp)
					(entity, pkgind);
			if(siaderr == SIADFAIL)
				{
				SIALOG(MSGSTR(SIA_MSG_LOGERROR, "ERROR"),MSGSTR(SIA_MSG_SESINITERR,"Failure to initialize  session\n"));
				SIATHREADREL(SIA_SES_LOCK);
				return(SIAFAIL);
				}
			}
		pkgind++;
		}
	SIATHREADREL(SIA_SES_LOCK);
	return(SIASUCCESS);
}
