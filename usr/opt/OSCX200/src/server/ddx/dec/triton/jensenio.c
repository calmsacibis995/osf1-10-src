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
static char *rcsid = "@(#)$RCSfile: jensenio.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:18 $";
#endif

/*
 *	This module has the routines that setup the qvision
 *	device registers to read and write. You should have read
 *	the Jensen Hardware manual(KN121 System Module Programmer's
 *	reference Information). It describes the bit swizzling done
 *	in greater detail.
 *
 *	Written: 1-May-1993, Henry R. Tumblin
 */
/*	inp/outp/outpw routines for osf/1	*/

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/workstation.h>  
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <c_asm.h>
#include "wga.h"
#include "wgaio.h"

#define WMB()   asm("wmb")

static ws_depth_descriptor * dpth;      /* ws depth data */
static ws_map_control * map;             /* ws map data */

extern int wsFd;		/* channel for /dev/mouse */

/*
 *	Initialize the jensen, open /dev/mouse and get the
 *	mapping information back from the driver.
 */
void initJensenVGA (int index)
{
  int err;
  static char init_flags[32];		/* OK, has to change if >32 heads */
  ws_cursor_control cc;

  /*
   *      Map the frame
   */

  if (map == NULL)
    map = (ws_map_control *) Xcalloc(sizeof(map));
  if (!init_flags[index]) {
    map->map_unmap = MAP_SCREEN;
    map->screen = index;
    if ((err=ioctl(wsFd,MAP_SCREEN_AT_DEPTH, map))) {
      perror("Failed to map screen at depth");
      exit (1);
    }
    init_flags[index] = 1;
  }
    
  /*
   *      Fetch the depth info
   */
  
  if (dpth == NULL)  {
    dpth = (ws_depth_descriptor *) Xcalloc(sizeof(ws_depth_descriptor));
    dpth->screen = index;
    if ((err=ioctl(wsFd,GET_DEPTH_INFO, dpth))) {
      perror("Failed to get depth info");
      exit (1);
    }
  }

  /*
   *	Turn off the cursor
   */

  cc.screen = index;
  cc.control = CURSOR_OFF;
  if (ioctl(wsFd, CURSOR_ON_OFF, &cc) == -1) {
    ErrorF( "error enabling/disabling cursor\n");
    exit(1);
  }
  return;

}


int * GetFrameAddress (int index)
{
  return (int *) dpth->pixmap;
}

char * GetVgaAddress (int index)
{
  return dpth->plane_mask;
}

char * GetVgaPhysAddress (int index)
{
  return dpth->physaddr;
}

/*
 *      inp(reg)        - Input 8 bits of data
 *      inpw(reg)       - Input 16 bits of data
 */
unsigned int inp(reg)
unsigned int reg;
{

  unsigned int * rP = (unsigned int *)dpth->plane_mask;
  unsigned int data=0;

  rP = (unsigned int *) ((unsigned long) rP +    /* Base address of registers */
                          (reg<<7) +             /* Register address */
                         0x00);                  /* Set 8 bit write mode */

  data = * rP;
  data = (data>>((reg&0x3)*8)) &0xff;
  return data;
}



inpw (reg)
unsigned int reg;
{
  unsigned int * rP = (unsigned int *)dpth->plane_mask;
  unsigned int tmp;

  rP = (unsigned int *) ((unsigned long) rP +    /* Base address of registers */
                          (reg<<7) +             /* Register address */
                         0x20);                  /* Set 16 bit write mode */
  tmp = (*rP >> ((reg&0x03)*8))&0xffff;
  return tmp;
}
