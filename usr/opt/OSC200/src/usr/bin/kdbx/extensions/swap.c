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
static char *rcsid = "@(#)$RCSfile: swap.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/11/05 17:55:03 $";
#endif
# include <sys/types.h>
# include <dirent.h>
# include <sys/mode.h>
# include <sys/stat.h>
#include "krash.h"

static char *help_string =
"swap - print a summary of swap space                                     \\\n\
    Usage : swap                                                          \\\n\
";

FieldRec sum_fields[] = {
  { ".vs_fl",            NUMBER,  NULL, NULL},
  { ".vs_swapsize",      NUMBER,  NULL, NULL},
  { ".vs_freespace",     NUMBER,  NULL, NULL},
  { ".vs_vinfo.vps_dev", NUMBER,  NULL, NULL}
};

#define NUM_SUM_FIELDS (sizeof(sum_fields)/sizeof(sum_fields[0]))

static int tot_alloc = 0;
static int tot_free = 0;
static int tot_partitions = 0;
static dev_t dumpdev = 0;
static dev_t swap_default = 0;
static int multiplier;

static char *partition_name(dev_t dev)
{
  static char device_name[256+4+1];	/* `/dev/file-name-up-to-255-characters'<NUL> */
  DIR *dir;
  struct dirent *dirent;

  /*
  ** Find the device in the /tmp directory.
  */
  if (!(dir = opendir("/dev")))
    return NULL;

  /*
  ** Directory is accessed, read through each name in `/dev', and
  ** stat the file to obtain the (possible) major/minor numbers.
  ** When we have a block device which matches, return its name.
  */
  while (dirent = readdir(dir)) {
    /*
    ** Got some directory information.  Check for a block special
    ** with matching major and minor numbers.  If (when) found,
    ** return a pointer to the generated name.
    */
    struct stat local_stat;

    sprintf(device_name, "/dev/%s", dirent->d_name);
    if (!stat(device_name, &local_stat)) {
      if (S_ISBLK(local_stat.st_mode) &&
          major(local_stat.st_rdev) == major(dev) &&
          minor(local_stat.st_rdev) == minor(dev)) {
	closedir(dir);
	return device_name;
	}
      }
    }
  closedir(dir);
  return NULL;
  }

static void sum_one(void)
{
  char buf[256];

  print("");
  if (!tot_partitions) {
    print("       Swap device name              Size       In Use       Free");
    print("--------------------------------  ----------  ----------  ----------");
    multiplier = getpagesize()/1024;
    }
  tot_alloc      += (int) sum_fields[1].data;
  tot_free       += (int) sum_fields[2].data;
  tot_partitions += 1;
  sprintf(buf, "%-32s%11dk%11dk%11dk",
	  partition_name((int) sum_fields[3].data),
	  multiplier * (int) sum_fields[1].data,
	  multiplier * ((int) sum_fields[1].data - (int) sum_fields[2].data),
	  multiplier * (int) sum_fields[2].data);
  if ((dev_t) sum_fields[3].data == dumpdev)
    strcat(buf, "  Dumpdev");
  if ((dev_t) sum_fields[3].data == swap_default)
    strcat(buf, "Defsw");
  print(buf);
  sprintf(buf, "                                %11dp%11dp%11dp",
	  (int) sum_fields[1].data,
	  ((int) sum_fields[1].data - (int) sum_fields[2].data),
	  (int) sum_fields[2].data);
  if ((dev_t) sum_fields[3].data == swap_default)
    strcat(buf, "  DefSwap");
  print(buf);
}

static void sum_end(void)
{
  char buf[256];

  print("--------------------------------  ----------  ----------  ----------");
  sprintf(buf, "Total swap partitions: %4d     %11dk%11dk%11dk",
		tot_partitions,
		multiplier * tot_alloc,
		multiplier * (tot_alloc - tot_free),
		multiplier * tot_free);
  print(buf);
  sprintf(buf, "                                %11dp%11dp%11dp",
		tot_alloc, (tot_alloc - tot_free), tot_free);
  print(buf);
}

main(int argc, char **argv)
{
  long addr, end, val;
  int nfields;
  DataStruct pager;
  char *error;
  FieldRec *fields;
  void (*one_proc)(void), (*end_proc)(void);

  check_args(argc, argv, help_string);
  if(argc == 1){
    argc++;
    argv[1] = "-sum";
  }
  argc--;
  argv++;
  if (!read_sym_val("vm_swap_head", NUMBER, &end, &error)) {
    fprintf(stderr, "Couldn't read vm_swap_head:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }  
  while(argc){
    fields = sum_fields;
    nfields = NUM_SUM_FIELDS;
    one_proc = sum_one;
    end_proc = sum_end;
    if(!check_fields("struct vm_swap", fields, nfields, NULL)){
      field_errors(fields, nfields);
      quit(1);
    }

    /*
    ** Obtain information about the dump and default swap devices.
    */
    if(!read_sym_val("vm_swap_lazy", NUMBER, &val, &error)){
      fprintf(stderr, "Couldn't read vm_swap_lazy:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    swap_default = val;
    if (!read_sym_val("dumpdev", NUMBER, &val, &error)) {
      fprintf(stderr, "Couldn't read dumpdev:\n");
      fprintf(stderr, "%s\n", error);
      quit(1);
    }
    dumpdev = val;

    /*
    ** Process the list.
    */
    addr = end;
    do {
      if(!cast(addr, "struct vm_swap", &pager, &error)){
	fprintf(stderr, "Couldn't cast addr to a vm_swap:\n");
	fprintf(stderr, "%s\n", error);
	return(False);
      }
      if(!read_field_vals(pager, fields, nfields)){
	field_errors(fields, nfields);
	return(False);
      }
      if(one_proc) (*one_proc)();
      addr = (long) fields[0].data;
    } while(addr != end);
    if(end_proc) (*end_proc)();
    argc--;
    argv++;
  }  
  quit(0);
}
