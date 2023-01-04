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
static char	*sccsid = "@(#)$RCSfile: map.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:20 $";
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
#include <sys/unix_defs.h>
#include "mda.h"
#include <kern/lock.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <machine/pmap.h>
#include <machine/pte.h>
#include <mach/kern_return.h>
#include <mach/machine/vm_types.h>


map(inputfile, ifd, filesize, input_file_address)

     char		*inputfile;	     /* IN:  Name of file to map    */
     int		*ifd;		     /* OUT: Input File Descriptor  */
     int		*filesize;	     /* OUT: Bytes in file	    */
     vm_offset_t	*input_file_address; /* OUT: Where file is mapped   */

{
  int		unix_result;
  kern_return_t	mach_result;
  struct stat	stbuf;
  struct stat	*stptr;
  int		numbytes;

  *ifd = open(inputfile,O_RDONLY);
  if (*ifd < 0 )
    {
      return(FAILED);
    }

  stptr = &stbuf;
  unix_result = fstat(*ifd,stptr);
  if (unix_result != 0)
    {	perror("mda: fstat failed (map)");
	return(FAILED);
    }
  numbytes = stbuf.st_size;
  *input_file_address = (vm_offset_t)0;
  mach_result = map_fd(*ifd, 0, input_file_address, TRUE, numbytes);
  if (mach_result != KERN_SUCCESS) {
    mach_error("mda: map_fd failed", mach_result);
    close(*ifd);
    return(FAILED);
  }

  file_mapped = TRUE;
  *filesize = numbytes;
  return(SUCCESS);
}



initptbr()
{
  int	result;
  int	kernel_pmap;

  result = get_address("_kernel_pmap", &kernel_pmap);
  if (result != SUCCESS)
    {
      printf("mda: Failed to get address of kernel pmap\n");
      return(FAILED);
    }
  kernel_pmap = MAP(kernel_pmap);
  ptb0 = (MAP(kernel_pmap));
}

int va_l1mask = VA_L1MASK;
int va_l2mask = VA_L2MASK;
int pg_l1off = PG_L1OFF;
int pg_l2off = PG_L2OFF;
int pfmask = ~VA_OFFMASK;
int pfoff = PG_PFOFF;

extern int vm_enabled;

phys(vaddr,paddr,ptbx)
     unsigned int vaddr;
     unsigned int *paddr;
     unsigned int ptbx;
{
  int ptindex;
  int lev1pte;
  int	l2pt, lev2pte;

  if (!vm_enabled)
    {
      *paddr = vaddr;
      return(SUCCESS);
    }

  ptindex = (vaddr & va_l1mask) >> pg_l1off;
  lev1pte = MAP(ptbx + 4*ptindex);
  if (lev1pte & PG_V != PG_V)
    {
      printf("mda: Invalid level 1 pte for virtual address 0x%x\n", vaddr);
      return(FAILED);
    }
  ptindex = (vaddr & va_l2mask) >> pg_l2off;
  l2pt = (lev1pte & pfmask);
  lev2pte = MAP(l2pt + 4*ptindex);
  if (lev2pte & PG_V != PG_V)
    {
      printf("mda: Invalid level 2 pte for virtual address 0x%x\n", vaddr);
      return(FAILED);
    }
  *paddr = (lev2pte & pfmask) | (vaddr & (~pfmask));
  return(SUCCESS);
}
