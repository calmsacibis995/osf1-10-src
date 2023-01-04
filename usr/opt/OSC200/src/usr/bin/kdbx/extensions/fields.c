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
static char *rcsid = "@(#)$RCSfile: fields.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:50:44 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"fields - print out some fields of a structure                            \\\n\
    Usage : fields type-name address [field...]                           \\\n\
    type-name is the type of a pointer to the structure                   \\\n\
    address is the numeric address of the structure                       \\\n\
    field is zero or more field names                                     \\\n\
";

static void Usage(void){
  fprintf(stderr, "Usage : fields type-name address [field...]\n");
  quit(1);
}

main(int argc, char **argv)
{
  char *ptr, buf[256], *line, *type;
  Status status;
  long addr;

  check_args(argc, argv, help_string);
  argv++;
  argc--;
  if(argc < 2) Usage();
  type = argv[0];
  addr = strtoul(argv[1], &ptr, 16);
  if(*ptr != '\0') Usage();
  argv += 2;
  argc -= 2;
  while(*argv){
    sprintf(buf, "dbx print ((%s) 0x%p).%s\n", type, addr, *argv);
    printf(buf);
    fflush(stdout);
    status.type = OK;
    while((line = read_line(&status)) != NULL){
      print(line);
      free(line);
    }
    if(status.type != OK){
      print_status("read_line failed", &status);
      quit(1);
    }  
    argv++;
  }
  quit(0);
}
