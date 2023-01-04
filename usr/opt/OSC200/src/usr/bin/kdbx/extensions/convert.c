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
static char *rcsid = "@(#)$RCSfile: convert.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/25 15:00:58 $";
#endif
#include <stdio.h>
#include "krash.h"

static char *help_string =
"convert - convert number from one base to another                        \\\n\
    Usage : convert [-in 2|8|10|16] -out 2|8|10|16 [args...]              \\\n\
    The -in and -out switches specify the input and output bases,         \\\n\
    respectively.  If -in is not present, the input base is inferred from \\\n\
    the arguments.  Base 8 is inferred from a leading '0'; base 16 is     \\\n\
    inferred from a leading '0x'; base 10 is inferred if neither of those \\\n\
    conditions hold.  Base 2 can't be inferred; if the input is binary,   \\\n\
    -in 2 must be specified.  The arguments may be either numbers or      \\\n\
    variables.                                                            \\\n\
";

/* *** Fix convert 10 *** */

static void Usage(void){
  print("Usage : convert [-in 2|8|10|16] -out 2|8|10|16 [numeric args...]");
  quit(1);
}

static char *convert(long n, int base)
{
  static char buf[128];
  char *ptr;

  if(n == 0){
    sprintf(buf, "0");
    return(buf);
  }
  else {
    ptr = &buf[sizeof(buf)/sizeof(buf[0]) - 1];
    while(n && (ptr != buf)){
      *ptr = (n % base) + '0';
      if(!isdigit(*ptr)) *ptr = 'a' + *ptr - '9' - 1;
      ptr--;
      n = n/base;
    }
  }
  if(base == 8) *ptr-- = '0';
  else if(base == 16){
    *ptr-- = 'x';
    *ptr-- = '0';
  }
  return(ptr + 1);
}

static int get_base(char *arg)
{
  int n;

  n = atoi(arg);
  if((n == 0) || ((n != 2) && (n != 8) && (n != 10) && (n != 16))) Usage();
  return(n);
}

main(int argc, char **argv)
{
  char **arg, *start, *ptr, *error;
  int in_base, out_base, base;
  long n;
  Boolean calc_base, symbol;

  check_args(argc, argv, help_string);
  if(argc == 1) Usage();
  in_base = 0;
  out_base = 0;
  argc--;
  argv++;
  while((argc > 0) && (*argv[0] == '-')){
    if(!strcmp(argv[0], "-in")){
      in_base = get_base(argv[1]);
      argc -= 2;
      argv += 2;
    }
    else if(!strcmp(argv[0], "-out")){
      out_base = get_base(argv[1]);
      argc -= 2;
      argv += 2;
    }
    else Usage();
  }
  if(in_base == 0) calc_base = True;
  else calc_base = False;
  for(arg= argv;*arg;arg++){
    symbol = False;
    for(ptr= *arg;*ptr;ptr++){
      if(!isdigit(*ptr) && ((*ptr < 'a') || (*ptr > 'f')) &&
	 ((*ptr < 'A') || (*ptr > 'F')) && (*ptr != 'x') && (*ptr != 'X')){
	symbol = True;
	break;
      }
    }
    if(symbol){
      if(!read_sym_val(*arg, NUMBER, &n, &error)){
	fprintf(stderr, "Couldn't read %s:\n", *arg);
	fprintf(stderr, "%s\n", error);
	continue;
      }
    }
    else {
      if(!strncmp(*arg, "0x", 2)){
	start = *arg + 2;
	base = 16;
      }
      else if(**arg == '0'){
	start = *arg + 1;
	base = 8;
      }
      else {
	start = *arg;
	base = 10;
      }
      if(calc_base){
	for(ptr=start;*ptr;ptr++){
	  if(isdigit(*ptr)){
	    if(*ptr - '0' >= base) base = *ptr - '0' + 1;
	  }
	  else if((*ptr >= 'a') && (*ptr <= 'f')){
	    if(*ptr - 'a' + 10 >= base) base = *ptr - 'a' + 11;
	  }
	  else if((*ptr >= 'A') && (*ptr <= 'F')){
	    if(*ptr - 'A' + 10 >= base) base = *ptr - 'A' + 11;
	  }	
	}
	if(base > 10) base = 16;
	else if(base > 8) base = 10;
	else if(base > 0) base = 8;
	else {
	  fprintf(stderr, "Couldn't convert \"%s\"\n", arg);
	  continue;
	}
	in_base = base;
      }
      if(in_base == 0) Usage();
      n = 0;
      for(ptr=start;*ptr;ptr++){
	if(isdigit(*ptr)) n = n * in_base + *ptr - '0';
	else if((*ptr >= 'a') && (*ptr <= 'f'))
	  n = n * in_base + *ptr - 'a' + 10;
	else if((*ptr >= 'A') && (*ptr <= 'F'))
	  n = n * in_base + *ptr - 'A' + 10;
      }
    }
    print(convert(n, out_base));
  }
  quit(0);
}
