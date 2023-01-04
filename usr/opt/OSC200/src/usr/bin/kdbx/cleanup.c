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
static char *rcsid = "@(#)$RCSfile: cleanup.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:57:46 $";
#endif
#include "krash_p.h"

static CleanupArray *cleanups = NULL;

void new_cleanup_level(void)
{
  LevelArray *level;

  if(!cleanups) cleanups = CleanupArrayNew(NULL);
  level = LevelArrayNew(NULL);
  CleanupArrayAppend(cleanups, &level);
}

int add_cleanup(void (*proc)(void *), void *arg)
{
  CleanupRec rec;
  LevelArray *level;

  if(CleanupArraySize(cleanups) == 0){
    fprintf(stderr, "add_cleanup - size = 0\n");
    return(-1);
  }
  rec.proc = proc;
  rec.arg = arg;
  level = *CleanupArrayLastElement(cleanups);
  LevelArrayAppend(level, &rec);
  return(LevelArraySize(level) - 1);
}

void delete_cleanup(int cleanup, Boolean do_cleanup)
{
  LevelArray *level;
  CleanupRec *rec;

  if(CleanupArraySize(cleanups) == 0){
    fprintf(stderr, "delete_cleanup - No levels\n");
    return;
  }
  level = *CleanupArrayLastElement(cleanups);
  if((0 > cleanup) || (LevelArraySize(level) <= cleanup)){
    fprintf(stderr, "delete_cleanup - cleanup out of range\n");
    return;
  }
  if(do_cleanup){
    rec = LevelArrayElement(level, cleanup);
    (*rec->proc)(rec->arg);
  }
  LevelArrayDelete(level, cleanup);
}

void do_cleanups(void)
{
  LevelArray *level;
  CleanupRec *rec;

  if(CleanupArraySize(cleanups) == 0) return;
  level = *CleanupArrayLastElement(cleanups);
  LevelArrayIterPtr(level, rec) (*rec->proc)(rec->arg);
  CleanupArrayDelete(cleanups, CleanupArraySize(cleanups) - 1);
}

