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
static char *rcsid = "@(#)$RCSfile: thread.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/24 15:31:08 $";
#endif

#include <stdio.h>
#include "krash.h"
/* due to a conflict with timer_t, we will #define thread states instead 
 * of including thread.h - Satish Damle  05/19/93
*/
#define TH_WAIT                 0x01    /* thread is queued for waiting */
#define TH_SUSP                 0x02    /* thread has been asked to stop */
#define TH_RUN                  0x04    /* thread is running or on runq */
#define TH_SWAPPED              0x08    /* thread is swapped out */
#define TH_IDLE                 0x10    /* thread is an idle thread */

static char *help_string =
"thread - print the thread table                                              \\\n\
    Usage : thread [proc address...]                                             \\\n\
    If addresses are present, the thread structures named by the addresses  \\\n\
    are printed.  Otherwise, all threads are printed.                       \\\n\
";
   
FieldRec proc_fields[] = {
  { ".task", NUMBER, NULL, NULL },
  { ".task->thread_count", NUMBER, NULL, NULL },
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".task->thread_list.next", NUMBER, NULL, NULL }
};
 
FieldRec thread_fields[] = {
  { ".wait_event", NUMBER, NULL, NULL },
  { ".thread_list.next", NUMBER, NULL, NULL },
  { ".state", NUMBER, NULL, NULL },
  { ".pcb", NUMBER, NULL, NULL }
};

#define PROC_FIELDS (sizeof(proc_fields)/sizeof(proc_fields[0]))
#define THREAD_FIELDS (sizeof(thread_fields)/sizeof(thread_fields[0]))

static Boolean prthread(long addr, long *next_ret)
{
  DataStruct proc, thread;
  char *error, line[256], address[12], task[12], t_addr[12], pcb[12], event[12];
  int i, state;
  long nextthread;

  if(!cast(addr, "struct proc", &proc, &error)){
    fprintf(stderr, "Couldn't cast addr to a proc:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  if(!read_field_vals(proc, proc_fields, PROC_FIELDS)){
    field_errors(proc_fields, PROC_FIELDS);
    return(False);
  }
  nextthread = (long) proc_fields[3].data;
  for(i=0; i<(long)proc_fields[1].data; i++) {
    if(!cast(nextthread, "struct thread", &thread, &error)){
      fprintf(stderr, "Couldn't cast nextthread to a thread:\n");
      fprintf(stderr, "%s\n", error);
      return(False);
    }
    if(!read_field_vals(thread, thread_fields, THREAD_FIELDS)){
      field_errors(thread_fields, THREAD_FIELDS);
      return(False);
    }
    state = (int) thread_fields[2].data;
    sprintf(line, "%s %13s %13s %13s %13s"
            " %s%s%s%s%s",
            format_addr((long) nextthread, t_addr),
            format_addr((long) proc_fields[0].data, task),
            format_addr((long) struct_addr(proc), address),
            format_addr((long) thread_fields[0].data, event),
            format_addr((long) thread_fields[3].data, pcb),
            state & TH_WAIT ? " wait" : "",
            state & TH_SUSP ? " susp" : "",
            state & TH_RUN ? " run" : "",
            state & TH_SWAPPED ? " swap" : "",
            state & TH_IDLE ? " idle" : "");
    print(line);
  nextthread = (long) thread_fields[1].data;
  }
  if(next_ret) *next_ret = (long) proc_fields[2].data;
  return(True);
}

Usage(){
  fprintf(stderr, "Usage : thread [proc address...]\n");
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
  if(!check_fields("struct proc", proc_fields, PROC_FIELDS, NULL)){
    field_errors(proc_fields, PROC_FIELDS);
    quit(1);
  }
  if(!check_fields("struct thread", thread_fields, THREAD_FIELDS, NULL)){
    field_errors(thread_fields, THREAD_FIELDS);
    quit(1);
  }
  print("Thread Addr   Task Addr     Proc Addr     Event         pcb          state");
  print("===========   ===========   ===========   ===========   ===========  =====");
  if(argc == 0){
    if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read allproc:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    do {
      if(!prthread(procaddr, &procaddr)) quit(1);
    } while(procaddr != 0);
  }
  else {
    while(*argv){
      procaddr = strtoul(*argv, &ptr, 16);
      if(*ptr != '\0') Usage();
      if(!prthread(procaddr, NULL)) quit(1);
      argv++;
    }
  }
  quit(0);
}
