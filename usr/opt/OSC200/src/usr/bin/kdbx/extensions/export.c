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
static char *rcsid = "@(#)$RCSfile: export.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/08/27 15:47:49 $";
#endif
/* 
 * This extension prints the exported entries of the system.
 */

#include <stdio.h>
#include "krash.h"

#define MOD 1048576

static char *help_string =
"export - print the export table                                        \\\n\
    Usage : export                                                       \\\n\
";

FieldRec fields[] = {
  { ".e_flags", NUMBER, NULL, NULL },
  { ".e_fsid.val", ARRAY, NULL, NULL },
  { ".e_fid", STRUCTURE, NULL, NULL },
  { ".e_rootmap", NUMBER, NULL, NULL },
  { ".e_pathlen", NUMBER, NULL, NULL },
  { ".e_path", STRING, NULL, NULL },
  { ".e_next", NUMBER, NULL, NULL },
  { ".e_dev", NUMBER, NULL, NULL },
  { ".e_ino", NUMBER, NULL, NULL },
  { ".e_gen", NUMBER, NULL, NULL },
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

main(int argc, char **argv)
{
  DataStruct export;
  long next;
  unsigned int major, minor;
  char buf[256],  *error, *arg;

  check_args(argc, argv, help_string);
  if(!check_fields("struct kexport", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }  
  if(!read_sym_val("exported", NUMBER, &next, &error)){
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  sprintf(buf, "ADDR EXPORT        MAJ  MIN   INUM          GEN  MAP FLAGS PATH");
  print(buf);
  print("================== === ===== =====  =========== ==== ===== ===================");
  while(next != 0)
  {
    if(!cast(next, "struct kexport", &export, &error)){
      fprintf(stderr, "Couldn't cast to a export:\n");
      fprintf(stderr, "%s:\n", error);
    }
    if(!read_field_vals(export, fields, NUM_FIELDS)){
      field_errors(fields, NUM_FIELDS);
      break;
    }
    major = (long) fields[7].data / MOD;
    minor = (long) fields[7].data % MOD;
    sprintf(buf, "0x%p %2d %5d %6d %12d %4d %5d %-20s", next, major, minor, fields[8].data, fields[9].data, (short) fields[3].data, fields[0].data, (char *)fields[5].data);
    print(buf);
    next = (long) fields[6].data;
  };
  quit(0);
}
