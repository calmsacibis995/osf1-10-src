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
static char *rcsid = "@(#)$RCSfile: cast.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:49:20 $";
#endif
#include <stdio.h>
#include <krash.h>

static char *help_string =
"cast - have dbx print a piece of memory as a given type                  \\\n\
    Usage : cast address type                                             \\\n\
    This is equivalent to the dbx command \"print *((type) address)\"     \\\n\
";

static void Usage(){
  fprintf(stderr, "Usage - cast address type\n");
  quit(1);
}

main(int argc, char **argv)
{
  Status status;
  char buf[256], *ptr, *resp, *line;
  int i;
  long addr;
  Boolean is_addr;

  check_args(argc, argv, help_string);
  if(argc < 3) Usage();
  addr = strtoul(argv[1], &ptr, 16);
  if(*ptr != '\0') is_addr = False;
  else is_addr = True;
  strcpy(buf, "print *((");
  for(i=2;i<argc;i++){
    strcat(buf, argv[i]);
    strcat(buf, " ");
  }
  if(is_addr) sprintf(&buf[strlen(buf)], "*) 0x%p)", addr);
  else sprintf(&buf[strlen(buf)], "*) %s)", argv[1]);
  dbx(buf, True);
  status.type = OK;
  while((line = read_line(&status)) != NULL){
    print(line);
    free(line);
  }
  if(status.type != OK){
    print_status("read_line failed", &status);
    quit(1);
  }
  quit(0);
}
