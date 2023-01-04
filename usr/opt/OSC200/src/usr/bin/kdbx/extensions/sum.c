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
static char *rcsid = "@(#)$RCSfile: sum.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:54:45 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"sum - print a summary of the system                                      \\\n\
    Usage : sum                                                           \\\n\
";

static void read_var(name, type, val)
char *name;
int type;
long *val;
{
  char *error;
  long n;

  if(!read_sym_val(name, type, &n, &error)){
    fprintf(stderr, "Reading %s:\n", name);
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  *val = n;
}

main(argc, argv)
int argc;
char **argv;
{
  DataStruct utsname, cpup, time;
  char buf[256], *error, *resp, *sysname, *release, *version, *machine;
  long avail, secs;

  check_args(argc, argv, help_string);
  read_var("utsname.nodename", STRING, &resp);
  sprintf(buf, "Hostname : %s", resp);
  print(buf);
  free(resp);
  read_var("cpu_avail", NUMBER, &avail);
  read_var("cpup.system_string", STRING, &resp);
  sprintf(buf, "cpu: %s\tavail: %d", resp, avail);
  print(buf);
  free(resp);
  read_var("boottime.tv_sec", NUMBER, &secs);
  sprintf(buf, "Boot-time:\t%s", ctime(&secs));
  buf[strlen(buf) - 1] = '\0';
  print(buf);
  read_var("time.tv_sec", NUMBER, &secs);
  sprintf(buf, "Time:\t%s", ctime(&secs));
  buf[strlen(buf) - 1] = '\0';
  print(buf);
  read_var("utsname.sysname", STRING, &sysname);
  read_var("utsname.release", STRING, &release);
  read_var("utsname.version", STRING, &version);
  read_var("utsname.machine", STRING, &machine);
  sprintf(buf, "Kernel : %s release %s version %s (%s)", sysname, release,
	  version, machine);
  print(buf);
  quit(0);
}	
