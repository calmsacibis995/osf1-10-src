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
static char *rcsid = "@(#)$RCSfile: proc.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/06/11 15:31:46 $";
#endif
#include <stdio.h>
#include "sys/types.h"
#include "sys/time.h"
#include "sys/proc.h"
#include "krash.h"

static char *help_string =
"proc - print the proc table                                              \\\n\
    Usage : proc [address...]                                             \\\n\
    If addresses are present, the proc structures named by the addresses  \\\n\
    are printed.  Otherwise, all procs are printed.                       \\\n\
";

FieldRec fields[] = {
  { ".p_pid", NUMBER, NULL, NULL },
  { ".p_ppid", NUMBER, NULL, NULL },
  { ".p_pgrp->pg_id", NUMBER, NULL, NULL },
  { ".p_ruid", NUMBER, NULL, NULL },
  { ".p_pri", NUMBER, NULL, NULL },
  { ".p_cpu", NUMBER, NULL, NULL },
  { ".p_sig", NUMBER, NULL, NULL },
  { ".p_flag", NUMBER, NULL, NULL },
  { ".p_nxt", NUMBER, NULL, NULL },
  { ".thread->wait_event", NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

static char state(c)
char c;
{
  switch(c){
  case NULL: return(' ');
  case SSLEEP: return('s');
  case SWAIT: return('w');
  case SRUN: return('r');
  case SIDL: return('z');
  case SSTOP: return('t');
  default: return('?');
  }
}

static Boolean prproc(long addr, long *next_ret)
{
  DataStruct proc;
  unsigned int flag;
  char *error, line[256], address[12], event[12];

  if(!cast(addr, "struct proc", &proc, &error)){
    fprintf(stderr, "Couldn't cast addr to a proc:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  if(!read_field_vals(proc, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    return(False);
  }
  flag = (unsigned int) fields[7].data;
  sprintf(line, "%s %5u %5u %5u %4u %3u %3u %08p %s"
	  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	  format_addr((long) struct_addr(proc), address),
	  fields[0].data,
	  fields[1].data, fields[2].data, fields[3].data,
	  (unsigned int) fields[4].data & 0377,
	  (unsigned int) fields[5].data & 0377, fields[6].data,
	  format_addr((long) fields[9].data, event),
	  flag & SLOAD ? " in" : "",
	  flag & SSYS ? " sys" : "",
	  flag & STRC ? " trace" : "",
	  flag & SOMASK ? " omask" : "",
	  flag & SWEXIT ? " exit" : "",
	  flag & SPHYSIO ? " physio" : "",
	  flag & SVFORK ? " vfork" : "",
	  flag & SSEQL ? " seq" : "",
	  flag & SUANOM ? " rand" : "",
	  flag & STIMO ? " timo" : "",
	  flag & SOWEUPC ? " owe" : "",
	  flag & SNOCLDSTOP ? " chldstop" : "",
	  flag & SPAGV ? " pagv" : "",
	  flag & SCTTY ? " ctty" : "",
	  flag & SXONLY ? " xonly" : "",
	  flag & SEXEC ? " exec" : "");
  print(line);
  if(next_ret) *next_ret = (long) fields[8].data;
  return(True);
}

Usage(){
  fprintf(stderr, "Usage : proc [address...]\n");
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
  print("Addr        PID   PPID  PGRP  UID   PY CPU SIGS     Event       Flags");
  if(argc == 0){
    if(!read_sym_val("allproc", NUMBER, &procaddr, &error)){
      fprintf(stderr, "Couldn't read allproc:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    do {
      if(!prproc(procaddr, &procaddr)) quit(1);
    } while(procaddr != 0);
  }
  else {
    while(*argv){
      procaddr = strtoul(*argv, &ptr, 16);
      if(*ptr != '\0') Usage();
      if(!prproc(procaddr, NULL)) quit(1);
      argv++;
    }
  }
  quit(0);
}
