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
static char *rcsid = "@(#)$RCSfile: dis.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:25:57 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"dis - disassemble some instructions                                      \\\n\
    Usage : dis start-address [num-instructions]                          \\\n\
    num-instructions instructions starting at start-address are printed.  \\\n\
    If num-instructions is not given, 1 is assumed.                       \\\n\
";

static void Usage(){
  fprintf(stderr, "Usage : dis start-address [num-instructions]\n");
  quit(1);  
}

main(argc, argv)
int argc;
char **argv;
{
  char buf[256], *line, *ptr;
  Status status;
  int n;

  check_args(argc, argv, help_string);
  if(argc == 2) n = 1;
  else if(argc == 3){
    n = strtoul(argv[2], &ptr, 0);
    if(*ptr != '\0') Usage();
  }
  else Usage();
  sprintf(buf, "%s/%di", argv[1], n);
  dbx(buf, True);
  status.type = OK;
  while((line = read_line(&status)) != NULL){
    if(*line != '\0') print(line);
    free(line);
  }
  if(status.type != OK){
    print_status("read_line failed", &status);
    quit(1);
  }
  else quit(0);
}
