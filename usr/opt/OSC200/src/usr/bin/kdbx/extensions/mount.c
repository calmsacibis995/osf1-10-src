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
static char *rcsid = "@(#)$RCSfile: mount.c,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/07/30 19:32:29 $";
#endif
#include <stdio.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include "krash.h"

static char *help_string =
"mount - print the mount table                                            \\\n\
    Usage : mount [-s] [address...]                                       \\\n\
    -s outputs a short form of the table.                                 \\\n\
    If addresses are present, the mount entries named by them are printed.\\\n\
";

FieldRec short_mount_fields[] = {
  { ".m_next", NUMBER, NULL, NULL },
  { ".m_stat.f_type", NUMBER, NULL },
  { ".m_stat.f_mntonname", STRING, NULL, NULL },
  { ".m_stat.f_mntfromname", STRING, NULL, NULL }
};

#define NUM_SHORT_MOUNT_FIELDS \
(sizeof(short_mount_fields)/sizeof(short_mount_fields[0]))

FieldRec long_mount_fields[] = {
  { ".m_next", NUMBER, NULL, NULL },
  { ".m_stat.f_type", NUMBER, NULL },
  { ".m_stat.f_mntonname", STRING, NULL, NULL },
  { ".m_stat.f_mntfromname", STRING, NULL, NULL },
  { ".m_vnodecovered", NUMBER, NULL, NULL },
  { ".m_mounth", NUMBER, NULL, NULL },
  { ".m_flag", NUMBER, NULL, NULL }
};

#define NUM_LONG_MOUNT_FIELDS \
(sizeof(long_mount_fields)/sizeof(long_mount_fields[0]))

static char *fs_name(int type)
{
  switch(type){
  case MOUNT_NONE: return("unknown");
  case MOUNT_UFS: return("ufs");
  case MOUNT_NFS: return("nfs");
  case MOUNT_MFS: return("mfs");
  case MOUNT_PC: return("pc");
  case MOUNT_S5FS: return("s5fs");
  case MOUNT_CDFS: return("cdfs");
  case MOUNT_DFS: return("dfs");
  case MOUNT_EFS: return("efs");
  case MOUNT_PROCFS: return("procfs");
  case MOUNT_MSFS: return("msfs");
  case MOUNT_FFM: return("ffm");
  case MOUNT_FDFS: return("fdfs");
  default:
    fprintf(stderr, "fs_name : Bad type - %d\n", type);
    return("???");
  }
}

static Boolean getdev(char *where, char *maj, char *min)
{
  struct stat buf;
  if(stat(where, &buf) == -1){
    strcpy(maj, "   ");
    strcpy(min, "   ");
  }
  else {
    sprintf(maj, "%d", major(buf.st_rdev));
    sprintf(min, "%d", minor(buf.st_rdev));
  }
  return(True);
}

static Boolean prmount(Boolean do_short, int n, long addr, long *next_ret)
{
  DataStruct mount;
  char *error, buf[256];
  char maj[4], min[9];
  char mountaddr_f[12], vnodeaddr_f[12], rootvpaddr_f[12];

  if(!cast(addr, "struct mount", &mount, &error)){
    fprintf(stderr, "Couldn't cast to mount:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
    format_addr(addr, mountaddr_f);
  if(do_short){
    if(!read_field_vals(mount, short_mount_fields, NUM_SHORT_MOUNT_FIELDS)){
      field_errors(short_mount_fields, NUM_SHORT_MOUNT_FIELDS);
      return(False);
    }
    if(!getdev(short_mount_fields[3].data, maj, min)) return(False);
    sprintf(buf, "%s  %3s  %7s  %7s %-35s  %-25s", mountaddr_f,
	    maj, min, fs_name((int) short_mount_fields[1].data),
	    short_mount_fields[3].data, short_mount_fields[2].data);
    print(buf);
    if(next_ret) *next_ret = (long) short_mount_fields[0].data;
  }
  else {
    if(!read_field_vals(mount, long_mount_fields, NUM_LONG_MOUNT_FIELDS)){
      field_errors(long_mount_fields, NUM_LONG_MOUNT_FIELDS);
      return(False);
    }
    if(!getdev(long_mount_fields[3].data, maj, min)) return(False);
    format_addr((long)long_mount_fields[4].data, vnodeaddr_f);
    format_addr((long)long_mount_fields[5].data, rootvpaddr_f);
    sprintf(buf, "%s  %3s  %7s  %12s %12s %7s %-25s %s%s%s%s%s%s%s", mountaddr_f,
	    maj, min, vnodeaddr_f, rootvpaddr_f,
	    fs_name((int) long_mount_fields[1].data),
	    long_mount_fields[2].data,
	    ((long) long_mount_fields[6].data) & M_RDONLY ? " ro" : "",
	    ((long) long_mount_fields[6].data) & M_SYNCHRONOUS ? " sync" : "",
	    ((long) long_mount_fields[6].data) & M_NODEV ? " ndev" : "",
	    ((long) long_mount_fields[6].data) & M_QUOTA ? " q" : "",
	    ((long) long_mount_fields[6].data) & M_LOCAL ? " loc" : "",
	    ((long) long_mount_fields[6].data) & M_NOEXEC ? " nex" : "",
	    ((long) long_mount_fields[6].data) & M_NOSUID ? " nsu" : "");
    print(buf);
    if(next_ret) *next_ret = (long) long_mount_fields[0].data;
  }
  return(True);
}

main(int argc, char **argv)
{
  Boolean do_short;
  long root_addr, addr;
  int i;
  char *ptr, *error;

  check_args(argc, argv, help_string);
  argv++;
  argc--;
  do_short = False;
  if((argc > 0) && !strcmp(argv[0], "-s")){
    do_short = True;
    argv++;
    argc--;
  }
  if(do_short){
    if(!check_fields("struct mount", short_mount_fields,
		     NUM_SHORT_MOUNT_FIELDS, NULL)){
      field_errors(short_mount_fields, NUM_SHORT_MOUNT_FIELDS);
      quit(1);
    }
    print(" MOUNT        MAJ    MIN    TYPE                DEVICE                      MOUNT POINT");
    print("===========  =====  ===== ======== ===================================  ========================");
  }
  else {
    if(!check_fields("struct mount", long_mount_fields,
		     NUM_LONG_MOUNT_FIELDS, NULL)){
      field_errors(long_mount_fields, NUM_LONG_MOUNT_FIELDS);
      quit(1);
    }
    print(" MOUNT        MAJ    MIN     VNODE        ROOTVP      TYPE       PATH                 FLAGS");
    print("===========  =====  =====   ===========  =========== ======= =======================  =====");
  }
  if(!read_sym_val("rootfs", NUMBER, &addr, &error)){
    fprintf(stderr, "Couldn't read rootfs:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  root_addr = addr;
  if(argc == 0){
    i = 0;
    do {
      if(!prmount(do_short, i, addr, &addr)) quit(1);
      i++;
    } while(addr != root_addr);
  }
  else {
    while(*argv){
      i = strtoul(*argv, &ptr, 0);
      if(*ptr != '\0'){
	fprintf(stderr, "Couldn't parse %s to a number\n", *argv);
	quit(1);
      }
      if(!list_nth_cell(root_addr, "struct mount", i, "m_next", True, &addr,
			&error)){
	fprintf(stderr, "Couldn't get %d'th element of mount table\n", i);
	fprintf(stderr, "%s\n", error);
	quit(1);
      }
      if(!prmount(do_short, i, addr, NULL)) quit(1);
      argv++;
    }
  }
  quit(0);
}

