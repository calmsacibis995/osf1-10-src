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
static char	*sccsid = "@(#)$RCSfile: symtab.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:54 $";
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
#include <nlist.h>
#include <sys/types.h>
#include <stdio.h>

extern char kernel_file[];

get_address(symbol, address)
     char *symbol;		/* Name of symbol */
     long *address;		/* Pointer to address of symbol */
{
  struct nlist	the_list[2];
  int	result;
  int	value;
  int	c;

  result = get_from_cache(symbol,&value);
  if (result == SUCCESS)
  {
    *address = value;
    return(SUCCESS);
  }

  result = get_value(symbol,&value);
  if (result == SUCCESS)
    {
      *address = value;
      return(SUCCESS);
    }

  printf("Couldn't find symbol '%s'.  Search for it? (y/n): ", symbol);
  fflush(stdout);
  if(!yes())
	return(NO_SUCH_SYMBOL);

  bzero(the_list,(2 * sizeof(struct nlist)));
  the_list[0].n_name = symbol;
  the_list[1].n_name = (char *)0;
  printf("Looking for symbol '%s', please wait...", symbol);
  fflush(stdout);
  result = nlist(kernel_file, the_list);
  printf("\n");
  if (result != 0)
    {	perror("mda: nlist failed");
	  return(FAILED);
    }

  if ((the_list[0].n_value == 0) && (the_list[0].n_type == 0))
    return(NO_SUCH_SYMBOL);
  *address = the_list[0].n_value;
  add_to_cache(symbol,*address);
  return(SUCCESS);
}


struct cache_entry
{
    int	  symbol_value;
    char  symbol_name[256];
};

struct
  {
      int    nsymbols;
      struct cache_entry entries[100];
  } symbol_cache;


compare_cache_entries(first,second)
     struct cache_entry *first, *second;
{
    return (strcmp(first->symbol_name, second->symbol_name));
}

init_symbol_cache()
{
    symbol_cache.nsymbols = 0;
    return(SUCCESS);
}

get_from_cache(symbol,value)

     char *symbol;
     int  *value;
{
    int i;

    for (i=0; i<symbol_cache.nsymbols; i++)
      if (strcmp(symbol, symbol_cache.entries[i].symbol_name) == 0)
	{ *value = symbol_cache.entries[i].symbol_value;
	  return(SUCCESS);
	}
    return(FAILED);
  }


  add_to_cache(symbol,value)

     char *symbol;
     int  value;
{
    int i;
    caddr_t p;

    i = symbol_cache.nsymbols++;
    symbol_cache.entries[i].symbol_value = value;
    strcpy(symbol_cache.entries[i].symbol_name,symbol);
    p = &symbol_cache.entries[0];
    qsort(p,symbol_cache.nsymbols,sizeof(struct cache_entry),
	  compare_cache_entries);
    return(SUCCESS);
}

display_symbol_cache()
{
  int i;
  for (i=0; i<symbol_cache.nsymbols; i++)
    {
      printf("%s\t0x%.8x\n",
	     symbol_cache.entries[i].symbol_name,
	     symbol_cache.entries[i].symbol_value);
    }
}

yes()
{
	int	c;

	c = getchar();
	while(getchar() != '\n')
		;
	return(c == 'y' || c == 'Y');
}
