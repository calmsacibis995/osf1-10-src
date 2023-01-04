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
static char *rcsid = "@(#)$RCSfile: getgroup_r.c,v $ $Revision: 1.1.9.5 $ (DEC) $Date: 1993/12/17 17:10:56 $";
#endif

 
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endgrent_r = __endgrent_r
#pragma weak getgrent_r = __getgrent_r
#pragma weak getgrgid_r = __getgrgid_r
#pragma weak getgrnam_r = __getgrnam_r
#pragma weak setgrent_r = __setgrent_r
#endif
#endif
#include "SIA/siad.h"
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include "ts_supp.h"

int setgrent_r(FILE **gr_fp)
{
	union sia_get_params params;

	TS_EINVAL(!gr_fp);
	memset(&params, 0, sizeof params);
	params.group.name = (char *) gr_fp;
	if (sia_getgroup (G_SET, REENTRANT, &params) == SIAFAIL)
		return TS_FAILURE;
	else
		return TS_SUCCESS;
}

void endgrent_r(FILE **gr_fp)
{
	union sia_get_params params;

	if(!gr_fp) {
		TS_SETERR(EINVAL);
		return;
	}
	memset(&params, 0, sizeof params);
	params.group.name = (char *) gr_fp;
	sia_getgroup (G_END, REENTRANT, &params);
}

int
  getgrent_r (struct group *result, char *buffer, int len, FILE **gr_fp)
{
  union sia_get_params params;

  /* Check parameters */
  if ((result == (struct group *) NULL) ||
      (buffer == (char *) NULL) ||
	(gr_fp == (FILE **) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
      
  params.group.result = result;
  params.group.buffer = buffer;
  params.group.len = len;
	params.group.name = (char *) gr_fp;

  if (sia_getgroup (G_ENT, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}

int
  getgrnam_r (char *name, struct group *result, char *buffer, int len)
{
  union sia_get_params params;
  
  /* Check parameters */
  if ((result == (struct group *) NULL) ||
      (buffer == (char *) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
  
  params.group.name = name;
  params.group.result = result;
  params.group.buffer = buffer;
  params.group.len = len;

  if (sia_getgroup (G_NAM, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}

int
  getgrgid_r (gid_t gid, struct group *result, char *buffer, int len)
{
  union sia_get_params params;
  
  /* Check parameters */
  if ((result == (struct group *) NULL) ||
      (buffer == (char *) NULL) ||
      (len < 1)) {
    TS_SETERR(EINVAL);
    return (-1);
  }
   
  params.group.gid = gid;
  params.group.result = result;
  params.group.buffer = buffer;
  params.group.len = len;

  if (sia_getgroup (G_GID, REENTRANT, &params) == SIAFAIL)
    return (-1);  /* entry not found */

  return (0);
}
	
