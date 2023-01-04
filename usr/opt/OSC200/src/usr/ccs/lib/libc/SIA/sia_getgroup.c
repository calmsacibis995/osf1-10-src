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
static char *rcsid = "@(#)$RCSfile: sia_getgroup.c,v $ $Revision: 1.1.13.5 $ (DEC) $Date: 1993/12/17 17:10:46 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak sia_getgroup = __sia_getgroup
#endif
#include "siad.h"
#include "siad_bsd.h"

/* This module consists of one routine which multiplexes between the three  */
/* routines siad_getgrnam, siad_getgrgid, and siad_getgrent.  This routine  */
/* is called by getgrnam, getgrnam_r, getgrgid, getgrgid_r, getgrent, and   */
/* getgrent_r.                                                              */

/*      sia_getgroup():
	Provide thread locking for libc_r routines (-D_THREAD_SAFE),
	provide static storage for non reentrant getgr* routines,
	and call the appropriate siad_getgr* routine
 */	

  
int
  sia_getgroup (int function, int reentrant, union sia_get_params *params)
{
  int cap_index;
  static struct group group;
  static char buffer[BUFSIZ + 1];
  static FILE *gr_fp=NULL;

  /* Provide thread locking for ALL routines in libc_r */
  SIATHREADLOCK (SIA_GROUP_LOCK);

  /* Check parameters */
  if ((function < G_ENT || function > G_END) ||
      (reentrant < NON_REENTRANT || reentrant > REENTRANT) ||
      (params == (union sia_get_params *) NULL) ) 
    { 
      SIATHREADREL (SIA_GROUP_LOCK);

      return (SIAFAIL);
    }	

  /* Initialize SIA */
  if (sia_init() == SIAFAIL) {

    SIATHREADREL (SIA_GROUP_LOCK);

    return (SIAFAIL);
  }

  /* Provide static storage for non-reentrant routines */
  if (reentrant == NON_REENTRANT) {
    params -> group.buffer = buffer;
    params -> group.result = &group;
    params -> group.len    = sizeof(buffer) - 1;

    if(function != G_END) {
      bzero(buffer+8, sizeof(buffer)-8);
      bzero(&group, sizeof(struct group));
    }
    if(function != G_NAM && function != G_GID)
	params->group.name = (char *) &gr_fp;
  }

  /* Set capability index according to function.  The capability  */
  /* index is an index into the SIA matrix.                       */
  cap_index = SIA_GETGRENT + function;
  
  /* Call the appropriate siad_getgr* routine(s) */
  if (sia_switch (cap_index, params) == SIAFAIL) {
    SIATHREADREL (SIA_GROUP_LOCK);
	
    return (SIAFAIL);
  }
	
  SIATHREADREL (SIA_GROUP_LOCK);
	
  return (SIASUCCESS);
}
