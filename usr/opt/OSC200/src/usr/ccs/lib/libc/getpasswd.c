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
static char *rcsid = "@(#)$RCSfile: getpasswd.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/06/07 23:00:54 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak endpwent = __endpwent
#pragma weak getpwent = __getpwent
#pragma weak getpwnam = __getpwnam
#pragma weak getpwuid = __getpwuid
#pragma weak setpwent = __setpwent
#endif
#endif
#include "ts_supp.h"
#include "SIA/siad.h"

int
  setpwent (void)
{
  union sia_get_params params;

  sia_getpasswd (P_SET, NON_REENTRANT, &params);
	return TS_SUCCESS;
}

void
  endpwent (void)
{
  union sia_get_params params;

  sia_getpasswd (P_END, NON_REENTRANT, &params);
}

/* Old global that some utilities still expect to get in libc */

struct passwd *
  getpwent (void)
{
  union sia_get_params params;

  if (sia_getpasswd (P_ENT, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct passwd *) NULL);

  return (params.passwd.result);
}

struct passwd *
  getpwnam (char *name)
{
  union sia_get_params params;
  
  params.passwd.name = name;
  if (sia_getpasswd (P_NAM, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct passwd *) NULL);

  return (params.passwd.result);
}

struct passwd *
  getpwuid (uid_t uid)
{
  union sia_get_params params;
  
  params.passwd.uid = uid;
  if (sia_getpasswd (P_UID, NON_REENTRANT, &params) == SIAFAIL)
    return ((struct passwd *) NULL);

  return (params.passwd.result);
}
	
