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
static char *rcsid = "@(#)$RCSfile: namecache.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/11/17 14:54:45 $";
#endif

#include <stdio.h>
#include "krash.h"

FieldRec namecache_fields[] = {
  { ".nc_forw", NUMBER, NULL, NULL },
  { ".nc_nxt", NUMBER, NULL, NULL },
  { ".nc_vp", NUMBER, NULL, NULL },
  { ".nc_vpid", NUMBER, NULL, NULL },
  { ".nc_nlen", NUMBER, NULL, NULL },
  { ".nc_name", STRING, NULL, NULL },
  { ".nc_dvp", NUMBER, NULL, NULL },
};

#define NUM_NAMECACHE_FIELDS (sizeof(namecache_fields)/sizeof(namecache_fields[0]))

static char *help_string =
"namecache - print the namecache                                      \\\n\
   Usage : namecache [options]                                           \\\n\
     (no options)   print the namecache data structures on the system     \\\n\
";

main(int argc, char **argv)
{
  int i, count=0, allflag=0, ncflag=0 ;
  DataStruct namecache, ele;
  long  nchsize, ncarg=0, namecache_addr;
  char bufout[256], *error, *ptr, nc[12], ncvp[12], ncdvp[12];

  check_args(argc, argv, help_string);

  for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-all"))  allflag = 1;
      else if (!strcmp(argv[i], "-nc"))  {ncflag = 1; ncarg = strtoul(argv[++i],(char**)NULL,16);}
      else {
        fprintf(stderr, "%s: invalid option, `%s'\n", argv[0], argv[i]);
        quit(1);
      }

  if(!check_fields("struct namecache", namecache_fields, NUM_NAMECACHE_FIELDS, NULL)){
    field_errors(namecache_fields, NUM_NAMECACHE_FIELDS);
    quit(1);
  }

  namecache = read_sym("namecache");
  if(!read_sym_val("nchsize", NUMBER, &nchsize, &error)) {
    fprintf(stderr, "Couldn't read nchsize:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }

    print(" namecache      nc_vp    nc_vpid  nc_nlen    nc_dvp          nc_name");
    print("===========  =========== =======  ======= ============ =======================");

  for (i=0; i < nchsize; i++) {

    if((ele = array_element(namecache, i, &error)) == NULL){
      fprintf(stderr, "Couldn't get array element\n");
      fprintf(stderr, "%s\n", error);
      return(False);
    }

    namecache_addr = (long) struct_addr(ele);
    if (ncflag && (namecache_addr != ncarg)) continue;

    if(!read_field_vals(ele, namecache_fields, NUM_NAMECACHE_FIELDS)){
      field_errors(namecache_fields, NUM_NAMECACHE_FIELDS);
      quit(1);
    }

    /* skip invalid entries if all not wanted */
    if ((!allflag) && ((long)namecache_fields[2].data == 0)) continue;

    sprintf(bufout,"%s  %s %6d  %6d   %s  %s ", 
      format_addr(namecache_addr,nc), format_addr((long)namecache_fields[2].data,ncvp), 
      namecache_fields[3].data, namecache_fields[4].data,
      format_addr((long)namecache_fields[6].data,ncdvp), (char *)namecache_fields[5].data); 
    print(bufout);

    count++;

  } /* end of for loop */
  sprintf(bufout,"count=%d", count);
  print(bufout);

}
