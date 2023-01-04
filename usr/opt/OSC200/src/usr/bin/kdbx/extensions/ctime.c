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
static char *rcsid = "@(#)$RCSfile: ctime.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:50:30 $";
#endif
#include <time.h>
#include <krash.h>

static char *help_string =
"ctime - convert from seconds to a string                                 \\\n\
    Usage : ctime seconds                                                 \\\n\
";

Usage(void){
  print("Usage : ctime seconds");
  quit(1);
}

main(int argc, char **argv)
{
  char *ret, *ptr;
  unsigned int secs, i;

  check_args(argc, argv, help_string);
  if(argc != 2) Usage();
  secs = strtoul(argv[1], &ptr, 0);
  if(*ptr != '\0'){
    fprintf(stderr, "Couldn't parse \"%s\" as a number\n", argv[1]);
    Usage();
  }
  ret = ctime(&secs);
  ret[strlen(ret) - 1] = '\0';
  print(ret);
  quit(0);
}
