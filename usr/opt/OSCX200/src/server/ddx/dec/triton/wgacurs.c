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
static char *rcsid = "@(#)$RCSfile: wgacurs.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/22 17:35:34 $";
#endif

#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#undef NEED_EVENTS
#include "input.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "miscstruct.h"
#include "colormapst.h"

#include <stdio.h>

#include <c_asm.h>
#include "vga.h"
#include "vgaprocs.h"
#include "wga.h"
#include "wgaio.h"


extern Bool Must_have_memory;

/**********************************************************************
/*
 * Cursor state variables, initilized by colorCursorInit()
 *
 * If Current_cursor == NullCursor, video_busy and cursor_hidden
 * state variables will not change because no cursor drawing takes
 * place.
 *
 */
extern CursorPtr Current_cursor;

/* Flag set when CursorLocation is being read or written outside of MoveCursor */
#ifdef __alpha
int cursor_moving;
#else
extern int cursor_moving;
#endif

#ifdef SOFTWARE_CURSOR
#ifdef __alpha
unsigned long (*MoveCursor)(int x, int y);
#else
#if defined (DWDOS286)
extern unsigned long (_loadds *MoveCursor)(int x, int y);
#else
extern unsigned long (*MoveCursor)(int x, int y);
#endif
#endif
#endif /* SOFTWARE_CURSOR */

/*
 * Initialize these variables to force them into the
 * default data segment (286)
 */

/* Flag set TRUE at the start of any Hide function and FALSE at the end
** of the Show function.  While this flag is set, the MoveCursor function
** is not allowed to draw anything.  Instead it sets the cursor_moved flag
** and the cursor position is updated when the Show function is called. 
*/
extern int video_busy;
extern int cursor_moved;

extern int cursor_hidden;
static int colorCursorRedoColors = FALSE;
static ColormapPtr colorCursorColormap = NULL;
static int colorCursorBeingRecolored = FALSE;
extern Pixel colorCursorForegroundPixel;
extern Pixel colorCursorBackgroundPixel;

static int Video_width_pixels;
static int Video_height_pixels;

#ifdef __alpha
extern unsigned int cursor_width_pixels;
extern unsigned int cursor_width_bytes;
extern unsigned int cursor_height_pixels;
extern unsigned char fgbits[4 * 32];
extern unsigned char bgbits[4 * 32];
#else
extern unsigned int cursor_width_pixels, cursor_width_bytes;
extern unsigned int cursor_height_pixels;
extern unsigned char fgbits[];
extern unsigned char bgbits[];
#endif

/* XXX Hack definitions for now, but the idea is correct */
#ifdef __alpha
typedef struct {
  int x, y;
} BPOINT;
extern BPOINT CursorHot;

typedef struct {
  int xmin, ymin, xmax, ymax;
} BRECT;
extern BRECT CursorBounds;
#else
extern struct {
  int x, y;
} CursorHot;

extern struct {
  int xmin, ymin, xmax, ymax;
} CursorBounds;
#endif

#ifdef SOFTWARE_CURSOR

/*
 *	Set the new cursor pattern. The triton has a hardware
 *	cursor.
 */

void pwgaSetNewCursor (ScreenPtr pScr, CursorPtr pCurs)
{
unsigned char *privSource, *privMask;
int cursor_addr,row,col;
int w,h,bw;
unsigned char * dp;
int padBytes;
int privBytes;

  w = min(pCurs->bits->width,32);;
  h = min(pCurs->bits->height,32);

  padBytes = (((pCurs->bits->width + 31) & ~31) >> 3) - privBytes;
  privBytes = (w&7)>>3;
#ifdef NOTYET
  privSource = pCurs->devPriv[ pScr->myNum ];
  privMask = privSource+(privBytes*h);

#else
  privSource = pCurs->bits->source;
  privMask = pCurs->bits->mask;
#endif

  CursorHot.x = pCurs->bits->xhot;
  CursorHot.y = pCurs->bits->yhot;

  /* Load the cursor into the RAM */

  outp (CURSOR_WRITE,0x00);  
#ifdef CURSOR_DEBUG
  dumpBitmap (privSource, w, h);
#endif
  for (row=0,dp=privSource; row<32; row++,dp=privSource+(row*8)){
    for (col=0; col<32; col+=8){
      if ((col < w) && (row<h)){
        outp(CURSOR_DATA,reverseByte(*dp++));       
      }
      else{
        outp(CURSOR_DATA,0x00);
      }
    }
  }
#ifdef CURSOR_DEBUG
  dumpBitmap (privMask, w, h);
#endif
  for (row=0,dp=privMask; row<32; row++,dp=privMask+(row*8)){
    for (col=0; col<32; col+=8){
      if ((col < w) && (row<h)){
        outp(CURSOR_DATA,reverseByte(*dp++));       
      }
      else{
        outp(CURSOR_DATA,0x00);
      }
    }
  }

/*
 * Load the colors
 */
  outp(CO_COLOR_WRITE,0x00); /* Foreground */
  outp(CO_COLOR_DATA,(pCurs->foreRed>>8)&0xff); /* Red */
  outp(CO_COLOR_DATA,(pCurs->foreGreen>>8)&0xff); /* Green */
  outp(CO_COLOR_DATA,(pCurs->foreBlue>>8)&0xff); /* Blue */

  outp(CO_COLOR_WRITE,0x01); /* Background */
  outp(CO_COLOR_DATA,(pCurs->backRed>>8)&0xff); /* Red */
  outp(CO_COLOR_DATA,(pCurs->backGreen>>8)&0xff); /* Green */
  outp(CO_COLOR_DATA,(pCurs->backBlue>>8)&0xff); /* Blue */

  outp(CO_COLOR_WRITE,0x02); /* Foreground */
  outp(CO_COLOR_DATA,(pCurs->foreRed>>8)&0xff); /* Red */
  outp(CO_COLOR_DATA,(pCurs->foreGreen>>8)&0xff); /* Green */
  outp(CO_COLOR_DATA,(pCurs->foreBlue>>8)&0xff); /* Blue */

  outp(CO_COLOR_WRITE,0x03); /* Background */
  outp(CO_COLOR_DATA,(pCurs->backRed>>8)&0xff); /* Red */
  outp(CO_COLOR_DATA,(pCurs->backGreen>>8)&0xff); /* Green */
  outp(CO_COLOR_DATA,(pCurs->backBlue>>8)&0xff); /* Blue */

}

void pwgaDrawCursor(x, y)
int x, y;
{
int source_x, mask_x, source_mask_y;
int width, height;

  width = cursor_width_pixels;
  height = cursor_height_pixels;

  if (x < 0) {
    width += x;
    x = 0;
  }
  else if (x + width > Video_width_pixels){
    width = Video_width_pixels - x;
  }

  if (y < 0){
    height += y;
    y = 0;
  }
  else if (y + width > Video_height_pixels) {
    height = Video_height_pixels - y;
  }

  if ((width < 1) || (height < 1))
    return;

  CursorBounds.xmin = x;
  CursorBounds.ymin = y;
  CursorBounds.xmax = x + --width;
  CursorBounds.ymax = y + --height;

  outpw (DAC_CMD_2,0x23);

}


/*
 *	pwgaUndrawCursor - Remove the cursor from the display
 */

void pwgaUndrawCursor()
{
  outpw (DAC_CMD_2,0x00);
}

#endif /* SOFTWARE_CURSOR */
