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
static char *rcsid = "@(#)$RCSfile: getpasswd_r.c,v $ $Revision: 1.1.9.4 $ (DEC) $Date: 1993/10/05 20:58:41 $";
#endif
       
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endpwent_r = __endpwent_r
#pragma weak getpwent_r = __getpwent_r
#pragma weak getpwnam_r = __getpwnam_r
#pragma weak getpwuid_r = __getpwuid_r
#pragma weak setpwent_r = __setpwent_r
#endif
#endif
#include "SIA/siad.h"
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include "ts_supp.h"

int setpwent_r(FILE **pw_fp)
{
	union sia_get_params params;

	TS_EINVAL(!pw_fp);
	memset(&params, 0, sizeof params);
	params.passwd.name = (char *) pw_fp;
	if(sia_getpasswd(P_SET, REENTRANT, &params) == SIAFAIL)
		return TS_FAILURE;
	else
		return TS_SUCCESS;
}

void endpwent_r(FILE **pw_fp)
{
	union sia_get_params params;

	if(!pw_fp) {
		TS_SETERR(EINVAL);
		return;
	}
	memset(&params, 0, sizeof params);
	params.passwd.name = (char *) pw_fp;
	sia_getpasswd(P_END, REENTRANT, &params);
}

int
  getpwent_r (struct passwd *result, char *buffer, int len, FILE **pw_fp)
{
  union sia_get_params params;

  /* Check parameters */
  if ((result == (struct passwd *) NULL) ||
      (buffer == (char *) NULL) ||
	(pw_fp == (FILE **) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
      
  params.passwd.result = result;
  params.passwd.buffer = buffer;
  params.passwd.len = len;
	params.passwd.name = (char *) pw_fp;

  if (sia_getpasswd (P_ENT, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}

int
  getpwnam_r (char *name, struct passwd *result, char *buffer, int len)
{
  union sia_get_params params;
  
  /* Check parameters */
  if ((result == (struct passwd *) NULL) ||
      (buffer == (char *) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
  
  params.passwd.name = name;
  params.passwd.result = result;
  params.passwd.buffer = buffer;
  params.passwd.len = len;

  if (sia_getpasswd (P_NAM, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}

int
  getpwuid_r (uid_t uid, struct passwd *result, char *buffer, int len)
{
  union sia_get_params params;
  
  /* Check parameters */
  if ((result == (struct passwd *) NULL) ||
      (buffer == (char *) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
   
  params.passwd.uid = uid;
  params.passwd.result = result;
  params.passwd.buffer = buffer;
  params.passwd.len = len;

  if (sia_getpasswd (P_UID, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}
	
