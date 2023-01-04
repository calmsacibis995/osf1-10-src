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
static char *rcsid = "@(#)$RCSfile: file.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/06/25 15:06:00 $";
#endif
#include <stdio.h>
#include <sys/fcntl.h>
#include "krash.h"

static char *help_string =
"file - print out the file table                                          \\\n\
    Usage : file [addresses...]                                           \\\n\
    If no arguments are present, all file entries with non-zero reference \\\n\
    counts are printed. Otherwise, the file entries named by the addresses\\\n\
    are printed.                                                          \\\n\
";

/* *** Implement addresses *** */

FieldRec fields[] = {
  { ".f_type", NUMBER, NULL, NULL },
  { ".f_count", NUMBER, NULL, NULL },
  { ".f_msgcount", NUMBER, NULL, NULL },
  { ".f_cred", NUMBER, NULL, NULL },
  { ".f_data", NUMBER, NULL, NULL },
  { ".f_ops", NUMBER, NULL, NULL },
  { ".f_u.fu_offset", NUMBER, NULL, NULL },
  { ".f_flag", NUMBER, NULL, NULL }
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

static char *get_type(int type)
{
  static char buf[5];

  switch(type){
  case 1: return("file");
  case 2: return("sock");
  case 3: return("npip");
  case 4: return("pipe");
  default:
    sprintf(buf, "*%3d", type);
    return(buf);
  }
}

static Boolean prfile(DataStruct ele, long vn_addr, long socket_addr)
{
  char *error, op_buf[12], *ops, buf[256], address[12], cred[12], data[12];

  if(!read_field_vals(ele, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    return(False);
  }
  if((long) fields[1].data == 0) return(True);
  if((long) (fields[5].data) == 0) ops = " *Null*";
  else if((long) (fields[5].data) == vn_addr) ops = "  vnops";
  else if((long) (fields[5].data) == socket_addr) ops = "sockops";
  else format_addr((long) fields[5].data, op_buf);
  format_addr((long) struct_addr(ele), address);
  format_addr((long) fields[3].data, cred);
  format_addr((long) fields[4].data, data);
  sprintf(buf, "%s %s %4d %4d %s %11s %11s %6d%s%s%s%s%s%s%s%s%s",
	  address, get_type((int) fields[0].data), fields[1].data,
	  fields[2].data, ops, data, cred, fields[6].data,
	  ((long) fields[7].data) & FREAD ? " r" : "",
	  ((long) fields[7].data) & FWRITE ? " w" : "",
	  ((long) fields[7].data) & FAPPEND ? " a" : "",
	  ((long) fields[7].data) & FNDELAY ? " nd" : "",
	  ((long) fields[7].data) & FMARK ? " m" : "",
	  ((long) fields[7].data) & FDEFER ? " d" : "",
	  ((long) fields[7].data) & FASYNC ? " as" : "",
	  ((long) fields[7].data) & FSHLOCK ? " sh" : "",
	  ((long) fields[7].data) & FEXLOCK ? " ex" : "");
  print(buf);
  return(True);
}

static Boolean prfiles(DataStruct fil, int n, long vn_addr, long socket_addr)
{
  DataStruct ele;
  char *error;

  if((ele = array_element(fil, n, &error)) == NULL){
    fprintf(stderr, "Couldn't get array element\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }
  return(prfile(ele, vn_addr, socket_addr));
}

static void Usage(void){
  fprintf(stderr, "Usage : file [addresses...]\n");
  quit(1);
}

main(int argc, char **argv)
{
  int i;
  long nfile, vn_addr, socket_addr, addr;
  char *error, *ptr;
  DataStruct fil;

  check_args(argc, argv, help_string);
  argv++;
  argc--;
  if(!check_fields("struct file", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  if(argc == 0) fil = read_sym("file");
  if(!read_sym_val("nfile", NUMBER, &nfile, &error) ||
     !read_sym_addr("vnops", &vn_addr, &error) ||
     !read_sym_addr("socketops", &socket_addr, &error)){
    fprintf(stderr, "Couldn't read nfile:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  print("Addr        Type  Ref  Msg Fileops      f_data        Cred Offset Flags");
  print("=========== ====  ===  === ======= =========== =========== ====== =====");
  if(argc == 0){
    for(i=0;i<nfile;i++){
      if(!prfiles(fil, i, vn_addr, socket_addr)) quit(1);
    }
  }
  else {
    while(*argv){
      addr = strtoul(*argv, &ptr, 16);
      if(*ptr != '\0'){
	fprintf(stderr, "Couldn't parse %s to a number\n", *argv);
	quit(1);
      }
      if(!cast(addr, "struct file", &fil, &error)){
	fprintf(stderr, "Couldn't cast address to a file:\n");
	fprintf(stderr, "%s\n", error);
	quit(1);
      }
      if(!prfile(fil, vn_addr, socket_addr)) quit(1);
      argv++;
    }
  }
  quit(0);
}
