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
/*
 * @(#)$RCSfile: vgareg.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/09/03 14:30:26 $
 */
/*
 * @OSF_COPYRIGHT@
 */

#ifndef _VGAREG_H_
#define _VGAREG_H_

#include <sys/ioctl.h>

/*
 * structure for accessing the video chip`s framebuffer info
 */

#ifdef KERNEL

#define MiscOutReg General[0]

typedef struct {
  unsigned char General[4];     /* General Registers */
  unsigned char CRTC[25];       /* Crtc Controller */
  unsigned char Sequencer[5];   /* Video Sequencer */
  unsigned char Graphics[9];    /* Video Graphics */
  unsigned char Attribute[21];  /* Video Atribute */
  unsigned char DAC[768];       /* Internal Colorlookuptable */
  unsigned char NoClock;        /* number of selected clock */
  unsigned int FontInfo[2048];  /* save area for fonts */ 
  unsigned char TextAttribute;	/* not really a register */
} vgaHWRec, *vgaHWPtr;

#endif /* KERNEL */

#endif /* _VGAREG_H_ */




