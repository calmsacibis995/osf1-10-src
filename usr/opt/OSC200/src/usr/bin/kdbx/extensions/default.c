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
static char *rcsid = "@(#)$RCSfile: default.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 09:23:36 $";
#endif
#include "krash.h"

static char *format(char *command, char *arg)
{
  int count, buf_len;
  char *ptr, *ret, *start, *end, *buf_ptr;
  
  count = 0;
  ptr = command;
  while((ptr = strstr(ptr, "%s")) != NULL){
    count++;
    ptr += 2;
  }
  buf_len = strlen(command) + count * (strlen(arg) - 2);
  NEW_TYPE(ret, buf_len + 1, char, char *, "format");
  start = command;
  end = &command[strlen(command)];
  buf_ptr = ret;
  while(1){
    ptr = strstr(start, "%c");
    if(!ptr){
      strncpy(buf_ptr, start, end - start);
      ret[buf_len] = '\0';
      break;
    }
    else {
      strncpy(buf_ptr, start, ptr - start);
      buf_ptr += ptr - start;
      start = ptr + 2;
      sprintf(buf_ptr, "%s", arg);
      buf_ptr += strlen(buf_ptr);
    }
  }
  return(ret);
}

static void Usage(void)
{
  fprintf(stderr, "Usage : default default_val command args...\n");
  quit(1);
}

int main(int argc, char **argv)
{
  char *val, *command, **args, *buf, *line;
  Status status;

  if(argc < 3) Usage();
  val = argv[1];
  command = argv[2];
  args = &argv[3];
  context(True);
  if(*args){
    while(*args){
      buf = format(command, *args);
      krash(buf, False, False);
      free(buf);
      args++;
    }
  }
  else {
    buf = format(command, val);
    krash(buf, False, False);
  }
  quit(0);
}
