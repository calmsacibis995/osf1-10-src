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
static char *rcsid = "@(#)$RCSfile: task.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/24 15:30:27 $";
#endif

#include <stdio.h>
#include "sys/types.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "krash.h"

static char *help_string =
"task - print the task table                                              \\\n\
    Usage : task [proc address...]                                             \\\n\
    If addresses are present, the task structures named by the addresses  \\\n\
    are printed.  Otherwise, all tasks are printed.                       \\\n\
";
   
FieldRec fields[] = {
  { ".task", NUMBER, NULL, NULL },
  { ".task->ref_count", NUMBER, NULL, NULL },
  { ".task->thread_count", NUMBER, NULL, NULL },
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".task->u_address", NUMBER, NULL, NULL },
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

static Boolean prtask(long addr, long *next_ret)
{
  DataStruct proc;
  char *error, line[256], address[12], task[12], utask[12];

  if(!cast(addr, "struct proc", &proc, &error)){
    fprintf(stderr, "Couldn't cast addr to a proc:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  if(!read_field_vals(proc, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    return(False);
  }
  sprintf(line, "%s  %s %5u %5u    %s ",
          format_addr((long) fields[0].data, task),
          format_addr((long) struct_addr(proc), address),
          fields[1].data,
          fields[2].data,
          format_addr((long) fields[4].data, utask));
  print(line);
  if(next_ret) *next_ret = (long) fields[3].data;
  return(True);
}

Usage(){
  fprintf(stderr, "Usage : task [proc address...]\n");
  quit(1);
}

main(argc, argv)
int argc;
char **argv;
{
  long procaddr;
  char *error, *ptr;

  check_args(argc, argv, help_string);
  argv++;
  argc--;
  if(!check_fields("struct proc", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  print("Task Addr    Proc Addr    Ref  Threads  Utask Addr ");
  print("===========  =========== ===== =======  ===========");
  if(argc == 0){
    if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read allproc:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    do {
      if(!prtask(procaddr, &procaddr)) quit(1);
    } while(procaddr != 0);
  }
  else {
    while(*argv){
      procaddr = strtoul(*argv, &ptr, 16);
      if(*ptr != '\0') Usage();
      if(!prtask(procaddr, NULL)) quit(1);
      argv++;
    }
  }
  quit(0);
}
