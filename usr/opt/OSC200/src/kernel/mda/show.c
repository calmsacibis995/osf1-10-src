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
static char	*sccsid = "@(#)$RCSfile: show.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:33 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "mda.h"
#include "mmax/vm_types.h"

extern int dump_fd;

static  char	buf[256];
char	last_symbol_shown[256];


show_binary(value, count, size, format)
     unsigned int *value;
     int count, size, format;
{
  unsigned int pvalue;
  int i, result;
}

show_decimal(value, count, size, format)
     unsigned int *value;
     int count, size, format;
{
  unsigned int pvalue;
  int i, result;

  printf("0x%x: ",value);
  for (i=1; i<= count; i++)
     { result = phys(value,&pvalue,ptb0);
       if (result != SUCCESS)
	 {
	   printf("mda: Could not map address 0x%.8x\n", value);
	   return(SUCCESS);
	 }
       printf("%.10d\t",MAP(pvalue));
		(char *)value += size;
       if ( ((i%4) == 0) && (i<count) )
	 printf("\n0x%x: ",value);
     }
   printf("\n");
   strcpy(last_symbol_shown,buf);
}

show_hex(value, count, size, format)
     unsigned int *value;
     int count, size, format;
{
  unsigned int pvalue;
  int i, result;

  printf("0x%x: ",value);
  for (i=1; i<= count; i++)
     { result = phys(value,&pvalue,ptb0);
       if (result != SUCCESS)
	 {
	   printf("mda: Could not map address 0x%.8x\n", value);
	   return(SUCCESS);
	 }
       printf("0x%.8x\t",MAP(pvalue));
		(char *)value += size;
       if ( ((i%4) == 0) && (i<count) )
	 printf("\n0x%x: ",value);
     }
   printf("\n");
   strcpy(last_symbol_shown,buf);
}


show_octal(value, count, size, format)
     unsigned int *value;
     int count, size, format;
{
  unsigned int pvalue;
  int i, result;

  printf("0x%x: ",value);
  for (i=1; i<= count; i++)
     { result = phys(value,&pvalue,ptb0);
       if (result != SUCCESS)
	 {
	   printf("mda: Could not map address 0x%.8x\n", value);
	   return(SUCCESS);
	 }
       printf("0o%.11o\t",MAP(pvalue));
		(char *)value += size;
       if ( ((i%4) == 0) && (i<count) )
	 printf("\n0x%x: ",value);
     }
   printf("\n");
}

show_string(value, count, size, format)
     unsigned int *value;
     int count, size, format;
{
  unsigned int pvalue;
  int i, result;

  result = phys(value,&pvalue,ptb0);
  if (result != SUCCESS)
    {
      printf("mda: Could not map address 0x%.8x\n", value);
      return(SUCCESS);
    }
  printf("0x%x: %s\n",value, MAPPED(pvalue));
}




char *format_choices[] = {
#define F_BINARY 0
  "binary",
#define F_DECIMAL (F_BINARY+1)
  "decimal",
#define F_HEXADECIMAL (F_DECIMAL+1)
  "hexadecimal",
#define F_OCTAL (F_HEXADECIMAL+1)
  "octal",
#define F_STRING (F_OCTAL+1)
  "string",
  0
};



show_cmd(arglist)
     char *arglist;
{
  char	*saved_arglist;
  char	data_name[256];
  int	i, count, size, format;
  int	result;
  unsigned int	*value;
  void 	(*show_function)();

  
  dohist("show", arglist);
  if (dump_fd == -1)
    {
      printf("No dump file read yet\n");
      return(FAILED);
    }

  saved_arglist = arglist;

  if (strncmp(arglist,"0x",2) == 0)
    {
      value = (int *)hexarg(&arglist,0,"Hex address: ",
			    0, 0xffffffff, last_symbol_shown);
    }
  else if (*arglist == '0')
    {
      value = (int *)octarg(&arglist,0,"Octal address: ",
			    0, 0xffffffff, last_symbol_shown);
    }      
  else if (strncmp(arglist,"0o",2) == 0)
    {
      arglist +=2;
      value = (int *)octarg(&arglist,0,"Octal address: ",
			    0, 0xffffffff, last_symbol_shown);
    }
  else
    {
      (void) strarg(&arglist,0,"symbol to show",last_symbol_shown,buf);
      data_name[0] = '_';
      strcpy(&data_name[1],buf);
      result = get_address(data_name,&value);
      switch (result)
	{
	case NO_SUCH_SYMBOL:
	  {
	    printf("mda: No such symbol (%s)\n", data_name);
	    return(FAILED);
	  }
	case FAILED:
	  {
	    printf("mda: get_address(%s) failed\n", data_name);
	    return(FAILED);
	  }
	case SUCCESS:
	  {
	    break;
	  }
	default:
	  {
	    printf("mda: Internal error in show_cmd\n");
	    return(FAILED);
	  }
	}
    }
  
  count = 1;
  if (*arglist != (char)0)
    count = intarg(&arglist,0,"Count: ", 1, 10000, 1);
  
  format = F_HEXADECIMAL;
  if (*arglist != (char)0)
    {
      format = stabarg(&arglist,0,"display format",&format_choices[0],
		       "hexadecimal");
    }
  switch (format)
    {
    case F_HEXADECIMAL:
      {
	show_function = show_hex;
	break;
      }
    case F_DECIMAL:
      {
	show_function = show_decimal;
	break;
      }
    case F_OCTAL:
      {
	show_function = show_octal;
      }
    case F_STRING:
      {
	show_function = show_string;
	break;
      }
    }


  
  size = 4;			/* *** Permit arg *** */

  show_function(value, count, size, format);

  return(SUCCESS);
}
