
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
#include "krash.h"

static char *help_string =
"ds addr - translate address to a name			\\\n\
     Usage : ds virtual_address				\\\n\
     the output shows the name + (offset)		\\\n\
";

Usage(){
   print("Usage: ds virtual_address");
   quit(1);
}

main(argc, argv)
  int argc;
  char *argv[];
{
   char buf[256];
   long  addr;
   char *name;

   check_args(argc, argv, help_string);
   if(argc != 2) Usage();
     else {
       addr = strtoul(argv[1], (char **)NULL, 16); 
       name = addr_to_proc((long) addr);
       sprintf(buf, "0x%lx: %s", addr,  name);
       print(buf);
       free(name);
     }
}
