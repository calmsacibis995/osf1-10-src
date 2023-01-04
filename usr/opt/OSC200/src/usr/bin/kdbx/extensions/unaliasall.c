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
static char *rcsid = "@(#)$RCSfile: unaliasall.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/08/02 16:41:06 $";
#endif
#include <stdio.h>
#include <ctype.h>
#include "krash.h"

static char *help_string =
"unaliasall - remove all aliases                                          \\\n\
    Usage : unaliasall                                                    \\\n\
";

main(argc, argv)
int argc;
char **argv;
{
  char *buf, *cmd, *ptr;
  Status status;

  check_args(argc, argv, help_string);
  print(" "); 
  krash("context user", False, False);
  krash("alias", False, True);
  while(buf = read_line(NULL)){
    ptr = buf;
    while(!isspace(*ptr)) ptr++;
    *ptr = '\0';
    NEW_TYPE(cmd, strlen("unalias ") + strlen(buf) + 1, char, char *, "main");
    sprintf(cmd, "unalias %s", buf);
    krash(cmd, False, False);
    free(cmd);
    free(buf);
  }
  quit(0);
}
