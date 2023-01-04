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
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Thomas Roell, roell@informatik.tu-muenchen.de
 *
 * $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/x386/vga/vga.h,v 1.2 91/12/15 12:42:16 devrcs Exp $
 */

#include "X.h"
#include "misc.h"
#include "x386.h"

extern Bool    vgaProbe();
extern Bool    vgaScreenInit();
extern void    vgaEnterLeaveVT();
extern void    vgaAdjustFrame();
extern void    vgaSwitchMode();

extern Bool    vgaSaveScreen();
extern Bool    vgaCloseScreen();
extern void    vgaBitBlt();
extern void    vgaImageRead();
extern void    vgaImageWrite();
extern void    vgaPixBitBlt();
extern void    vgaImageGlyphBlt();

extern void    vgaHWInit();
extern void    vgaHWRestore();
extern void *  vgaHWSave();
extern void    vgaGetClocks();

extern int     vgaListInstalledColormaps();
extern void    vgaStoreColors();
extern void    vgaInstallColormap();
extern void    vgaUninstallColormap();


/*
 * structure for accessing the video chip`s functions
 */
typedef struct {
  Bool (* ChipProbe)();
  void (* ChipEnterLeave)();
  void (* ChipInit)();
  void * (* ChipSave)();
  void (* ChipRestore)();
  void (* ChipAdjust)();
  void (* ChipSetRead)();
  void (* ChipSetWrite)();
  void (* ChipSetReadWrite)();
  int ChipMapSize;
  int ChipSegmentSize;
  int ChipSegmentShift;
  int ChipSegmentMask;
  int ChipReadBottom;
  int ChipReadTop;
  int ChipWriteBottom;
  int ChipWriteTop;
  
} vgaVideoChipRec, *vgaVideoChipPtr;

extern vgaVideoChipRec ET3000;
extern vgaVideoChipRec ET4000;
extern vgaVideoChipRec GVGA;
extern vgaVideoChipRec PVGA1;


/*
 * hooks for communicating with the VideoChip on the VGA
 */
extern void (* vgaInitFunc)();
extern void (* vgaEnterLeaveFunc)();
extern void * (* vgaSaveFunc)();
extern void (* vgaRestoreFunc)();
extern void (* vgaAdjustFunc)();
extern void (* vgaSetReadFunc)();
extern void (* vgaSetWriteFunc)();
extern void (* vgaSetReadWriteFunc)();
extern int vgaMapSize;
extern int vgaSegmentSize;
extern int vgaSegmentShift;
extern int vgaSegmentMask;
extern int vgaIOBase;

#include "vgaBank.h"
extern pointer vgaOrigVideoState;    /* buffers for all video information */
extern pointer vgaNewVideoState;
extern pointer vgaBase;              /* the framebuffer himself */


typedef struct {
  unsigned char MiscOutReg;     /* */
  unsigned char CRTC[25];       /* Crtc Controller */
  unsigned char Sequencer[5];   /* Video Sequencer */
  unsigned char Graphics[9];    /* Video Graphics */
  unsigned char Attribute[21];  /* Video Atribute */
  unsigned char DAC[768];       /* Internal Colorlookuptable */
  unsigned char NoClock;        /* number of selected clock */
  pointer FontInfo;             /* save area for fonts */ 
} vgaHWRec, *vgaHWPtr;

#define BITS_PER_GUN 6
#define COLORMAP_SIZE 256

extern void vgaImageGlyphBlt();
extern void vgaDoBitBlt();

extern ScrnInfoRec vga256InfoRec;
