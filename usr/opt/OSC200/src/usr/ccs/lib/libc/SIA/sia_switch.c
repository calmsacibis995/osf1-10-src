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
static char *rcsid = "@(#)$RCSfile: sia_switch.c,v $ $Revision: 1.1.14.5 $ (DEC) $Date: 1993/11/19 21:44:12 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak sia_setupfp = __sia_setupfp
#pragma weak sia_switch = __sia_switch
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  sia_switch (int cap, union sia_get_params *params)
{
  	int	pkgind = 0; 
  	int	siaderr = (SIADFAIL | SIADSTOP);
	long    tempint;
	int	have_tempint = 0;
	FILE **context=NULL;

	if(params->group.buffer != NULL)
		{
		if (params->group.len <= 8)
			return SIAFAIL;
		bcopy(params->group.buffer, &tempint, sizeof (long));
		if (tempint != (int) tempint)
			tempint = -1;
		have_tempint = 1;
		params->group.buffer += 8;
		params->group.len -= 8;
		params->group.pkgind = tempint;
		}
	if((cap == SIA_GETGRENT) || (cap == SIA_GETPWENT))
		{
		if((params->group.pkgind > -1 ) && (params->group.pkgind < SIASWMAX))
			{ /* a legal package index */
			if((sia_matrix[cap][params -> group.pkgind].pkgnam == NULL ) || (sia_matrix[cap][params -> group.pkgind].fp == NULL))
				{
				params -> group.pkgind = 0;
				pkgind = -1;
				}
			else pkgind = params -> group.pkgind;
			}
		else	{
			params -> group.pkgind = 0;
			pkgind = -1;
			}
		if(pkgind == -1) /* must be the beginning of a get*ent */
			{
			pkgind=0;
			switch(cap) {	/*  first getgrent or getpwent call */
	  			case  SIA_GETGRENT:	
					{
					context = (FILE **) params->group.name;
					sia_setupfp(SIA_SETGRENT,pkgind);
					siaderr = (*sia_matrix[SIA_SETGRENT][pkgind].fp) (context);
					break;
					}
	  			case 	SIA_GETPWENT:
					{
					context = (FILE **) params->passwd.name;
                                	sia_setupfp(SIA_SETPWENT,pkgind);
                                	siaderr = (*sia_matrix[SIA_SETPWENT][pkgind].fp) (context);
					break;
					}
	   			}
			}
		
		}
	else	{
		params -> group.pkgind = 0;
		pkgind = 0;
		}
while(pkgind < SIASWMAX && sia_matrix[cap][pkgind].pkgnam != NULL)
	{
	if (sia_matrix[cap][pkgind].fp == NULL)
		sia_setupfp(cap,pkgind);
	if (sia_matrix[cap][pkgind].fp != NULL)
		{
		switch (cap) 
			{
	  		case SIA_GETGRENT:
				context = (FILE **) params->group.name;
				if(pkgind != params -> group.pkgind)	/* done processing one mechanism */
					{
					if(pkgind != 0)	/* guard against first one */
						{
						sia_setupfp(SIA_ENDGRENT,params -> group.pkgind);
						siaderr = (*sia_matrix[SIA_ENDGRENT][params -> group.pkgind].fp) (context);
						sia_setupfp(SIA_SETGRENT,pkgind);
						siaderr = (*sia_matrix[SIA_SETGRENT][pkgind].fp) (context);
						}
					params -> group.pkgind = pkgind;
					}
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> group.result, params -> group.buffer, params -> group.len, context);
	    			break;
	  		case SIA_GETGRGID:
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> group.gid, params -> group.result, params -> group.buffer, params -> group.len);
	   			break;
	  		case SIA_GETGRNAM:
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> group.name, params -> group.result, params -> group.buffer, params -> group.len);
	    			break;
	  		case SIA_SETGRENT:
			case SIA_ENDGRENT:
				context = (FILE **) params->group.name;
				siaderr = (*sia_matrix[cap][pkgind].fp) (context);
				break;
			case SIA_SETPWENT:
			case SIA_ENDPWENT:
				context = (FILE **) params->passwd.name;
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (context);
	    			break;
	  		case SIA_GETPWENT:
				context = (FILE **) params->passwd.name;
                        	if(pkgind != params -> passwd.pkgind)    /* done processing one mechanism */
                                	{
                                	if(pkgind != 0) /* guard against first one */
                                        	{
                                        	sia_setupfp(SIA_ENDPWENT,params -> passwd.pkgind);
                                        	siaderr = (*sia_matrix[SIA_ENDPWENT][params -> passwd.pkgind].fp) (context);
                                        	sia_setupfp(SIA_SETPWENT,pkgind);
                                        	siaderr = (*sia_matrix[SIA_SETPWENT][pkgind].fp) (context);
                                        	}
                                	params -> passwd.pkgind = pkgind;
                                	}
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> passwd.result, params -> passwd.buffer, params -> passwd.len, context);
	    			break;
	  		case SIA_GETPWUID:
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> passwd.uid, params -> passwd.result, params -> passwd.buffer, params -> passwd.len);
	    			break;
	  		case SIA_GETPWNAM:
	    			siaderr = (*sia_matrix[cap][pkgind].fp) (params -> passwd.name, params -> passwd.result, params -> passwd.buffer, params -> passwd.len);
	    			break;
	  		default:
				if (have_tempint) {
					params->group.buffer -= 8;
					params->group.len += 8;
					tempint = params->group.pkgind;
					bcopy(&tempint,
					      params->group.buffer,
					      sizeof (long));
				}
	    			return (SIAFAIL);
	  		}
	  	if (siaderr == SIADSUCCESS)
			{
			if (have_tempint) {
				params->group.buffer -= 8;
				params->group.len += 8;
				tempint = params->group.pkgind;
				bcopy(&tempint,
				      params->group.buffer,
				      sizeof (long));
			}
	    		return (SIASUCCESS);
			}
		}
	pkgind++;
    	}
params -> passwd.pkgind = -1; /* reset package index */
if (have_tempint) {
	params->group.buffer -= 8;
	params->group.len += 8;
	tempint = params->group.pkgind;
	bcopy(&tempint,
	      params->group.buffer,
	      sizeof (long));
	}
return (SIAFAIL);
}

int
sia_setupfp(cap,pkgind)
int cap;
int pkgind;
{
#ifndef SV_SHLIBS
  ldr_module_t    lib_handle = NULL;
  int             (*fp) ();
#else 
void *lib_handle;
void *fp;
#endif
if (cap < 0 || cap >= SIADCAPMAX || pkgind < 0 || pkgind >= SIASWMAX)
	{
	return SIAFAIL;
	}
if (sia_matrix[cap][pkgind].fp == NULL)
	{       
	  /* if libc routine then setup fp and call it */
	if(strcmp(sia_matrix[cap][pkgind].libnam,SIALIBCSO) == 0)
		{
		sia_matrix[cap][pkgind].fp = sia_cap_fps[cap];
		}	
	  else    { /* we are looking this up in a shared library */
#ifndef SV_SHLIBS 	/*********** OSF SHARED LIBRARIES *************/
	    	/* check to be sure the required shared library is loaded */
	    	if ((lib_handle = load (sia_matrix[cap][pkgind].libnam, LDR_NOFLAGS)) >= 0)
	      		{
			/* set fp from the shared library lookup table */
			fp = ldr_lookup_package (sia_matrix[cap][pkgind].pkgnam, sia_caps[cap]);
			if (( fp == NULL) | (fp && fp == (void *)-1))
		    		SIALOG (MSGSTR (SIA_MSG_LOGERROR, "ERROR"), MSGSTR (SIA_MSG_CONFERR, "SIA configuration error pertaining to %s\n"), sia_caps[cap]);
			else    sia_matrix[cap][pkgind].fp = fp;
	      		}
	    	else    {
	      		SIALOG (MSGSTR (SIA_MSG_LOGERROR, "ERROR"), MSGSTR (SIA_MSG_CONFERR, "SIA configuration error pertaining to %s\n"), sia_caps[cap]);
	    		}
#else		/*************** SVR4 SHARED LIBRARIES ****************/
	 	lib_handle = dlopen(sia_matrix[cap][pkgind].libnam, RTLD_LAZY);
	 	if (lib_handle != NULL) {
                	fp = dlsym(lib_handle,sia_caps[cap]);
                	if (fp == NULL)
		    		SIALOG (MSGSTR (SIA_MSG_LOGERROR, "ERROR"), MSGSTR (SIA_MSG_CONFERR, "SIA configuration error pertaining to %s\n"), sia_caps[cap]);
			else sia_matrix[cap][pkgind].fp = fp;
        		}
		else    {
			SIALOG (MSGSTR (SIA_MSG_LOGERROR, "ERROR"), MSGSTR (SIA_MSG_CONFERR, "SIA configuration error pertaining to %s\n"), sia_caps[cap]);
			}
#endif
		}
	}
if (sia_matrix[cap][pkgind].fp == NULL)
	return SIAFAIL;
return SIASUCCESS;
}
