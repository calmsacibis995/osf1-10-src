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
static char *rcsid = "@(#)$RCSfile: vgacurs.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:33 $";
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
CursorPtr Current_cursor = NullCursor;

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
int video_busy = FALSE;
int cursor_moved = FALSE;

int cursor_hidden = TRUE;
static int colorCursorRedoColors = FALSE;
static ColormapPtr colorCursorColormap = NULL;
static int colorCursorBeingRecolored = FALSE;
Pixel colorCursorForegroundPixel = 0;
Pixel colorCursorBackgroundPixel = 0;

static int Video_width_pixels;
static int Video_height_pixels;

#ifdef __alpha
unsigned int cursor_width_pixels = 32;
unsigned int cursor_width_bytes = 4;
unsigned int cursor_height_pixels = 32;
unsigned char fgbits[4 * 32] = { 0 };
unsigned char bgbits[4 * 32] = { 0 };
#else
extern unsigned int cursor_width_pixels, cursor_width_bytes;
extern unsigned int cursor_height_pixels;
extern unsigned char fgbits[];
extern unsigned char bgbits[];
#endif
/**********************************************************************/
/*
 *	Table of Contents
 */
void colorHideCursorAlways();
void colorHideCursorAlways();
void colorPointerNonInterestBox( ScreenPtr pScr, BoxPtr pBox );
static void colorCursorFindColors(CursorPtr pCursor);
void vgaSetNewCursor(ScreenPtr pScr,CursorPtr pCurs);
static reverseByte(int b);
unsigned long colorMoveCursor(int delta_x, int delta_y);
#ifdef SOFTWARE_CURSOR
Bool colorRealizeCursor (ScreenPtr pScr, CursorPtr pCurs);
Bool colorUnrealizeCursor(ScreenPtr pScr, CursorPtr pCurs);
Bool colorDisplayCursor (ScreenPtr pScr, CursorPtr pCurs);
void colorRecolorCursor (ScreenPtr pScr, CursorPtr pCurs, int displayed);
void colorConstrainCursor( ScreenPtr pScr, BoxPtr pBox );
void colorShowCursor();
void colorCursorLimits( ScreenPtr pScr, CursorPtr pCurs, BoxPtr pHotBox, BoxPtr pTopLeftBox);
void colorHideCursorInXYWH(int x, int y, int w, int h);
void colorHideCursorInSpans(DDXPointPtr ppt, int * pwidth, int count);
void colorHideCursorInLine(int x1, int y1, int x2, int y2);
Bool colorSetCursorPosition( ScreenPtr pScr, int x, int y, Bool generateEvent );
void colorCursorInstallColormap (ColormapPtr pMap);
void colorCursorStoreColors (ColormapPtr pMap, int ndef, xColorItem *pdef);
void colorCursorInit(ScreenPtr pScr);
#endif
/**********************************************************************/

/* XXX Hack definitions for now, but the idea is correct */
#ifdef __alpha
typedef struct {
  int x, y;
} BPOINT;
BPOINT CursorLocation = { 320, 240 };
BPOINT CursorHot = { 16, 16 };

typedef struct {
  int xmin, ymin, xmax, ymax;
} BRECT;
BRECT CursorBounds = { 0, 0, 0, 0 };
BRECT CursorConstrain = { 0, 0, 640, 480 };
#else
extern struct {
  int x, y;
} CursorLocation, CursorHot;

extern struct {
  int xmin, ymin, xmax, ymax;
} CursorBounds, CursorConstrain;
#endif

static unsigned char endmasks[8] 
	= { 0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };


/*
** Allocate storage for a private copy of the cursor image.
** Truncate the private cursor image to the implementation limit of
** 32x32 pixels.  Zero any pad bits on the right edge of the private
** cursor image.
*/
Bool colorRealizeCursor(ScreenPtr pScr,	CursorPtr pCurs )
{
unsigned char *cursorSource, *cursorMask;
unsigned char *privSource, *privMask;
unsigned char eMask;
int privBytes, padBytes;
int width, height;
int wtmp;

  cursorSource = pCurs->bits->source;
  cursorMask = pCurs->bits->mask;

  width = min(pCurs->bits->width, 32);
  height = min(pCurs->bits->height, 32);

  eMask = endmasks[width & 0x7];

  privBytes = (width + 7) >> 3;
/* cursor bitmaps are really 32 bit pad, 
	padBytes = ((pCurs->bits->width + 7) >> 3) - privBytes;
*/
  padBytes = (((pCurs->bits->width + 31) & ~31) >> 3) - privBytes;

  Must_have_memory = TRUE; /* XXX */
  privSource = (unsigned char *) Xalloc(privBytes * height * 2);
  Must_have_memory = FALSE; /* XXX */
  privMask = privSource + (privBytes * height);
  pCurs->devPriv[ pScr->myNum ] = (pointer)privSource;

  while (height--){
    for (wtmp=0; wtmp<privBytes; wtmp++){
      *privMask++ = *cursorMask++;
      *privSource++ = *cursorSource++;
    }
    cursorMask += padBytes;
    cursorSource += padBytes;
  }

  return TRUE;
}


Bool colorUnrealizeCursor( pScr, pCurs)
ScreenPtr pScr;
CursorPtr pCurs;
{
  Xfree( pCurs->devPriv[ pScr->myNum]);
  return TRUE;
}



#ifdef SOFTWARE_CURSOR

void colorHideCursorAlways(ScreenPtr pScreen)
{
  if (Current_cursor == NullCursor)
    return;

  video_busy = TRUE;
  if (!cursor_hidden){
    UndrawCursorFuncPtr UndrawCursorFunc =
      ((DrawFuncs *)(pScreen->devPrivate))->UndrawCursorFunc;

    (*UndrawCursorFunc)();
    cursor_hidden = TRUE;
  }
}

#endif


/* Find the pixel values for foreground and background colors */
static void colorCursorFindColors(pCursor)
CursorPtr pCursor;
{
xColorItem foregroundColor, backgroundColor;
ColormapPtr pColormap;

  if (colorCursorRedoColors){
    colorCursorBeingRecolored = TRUE;
    foregroundColor.red = pCursor->foreRed;
    foregroundColor.green = pCursor->foreGreen;
    foregroundColor.blue = pCursor->foreBlue;
    FakeAllocColor(colorCursorColormap, &foregroundColor);

    backgroundColor.red = pCursor->backRed;
    backgroundColor.green = pCursor->backGreen;
    backgroundColor.blue = pCursor->backBlue;
    FakeAllocColor(colorCursorColormap, &backgroundColor);

    /* "free" the pixels right away, don't let this confuse you */
    FakeFreeColor(colorCursorColormap, foregroundColor.pixel);
    FakeFreeColor(colorCursorColormap, backgroundColor.pixel);

    colorCursorForegroundPixel = foregroundColor.pixel;
    colorCursorBackgroundPixel = backgroundColor.pixel;
    colorCursorBeingRecolored = FALSE;
  }
  colorCursorRedoColors = FALSE;
} 


#ifdef SOFTWARE_CURSOR

void colorShowCursor(ScreenPtr pScreen)
{
  if (Current_cursor == NullCursor)
    return;

  if (colorCursorRedoColors)
    colorCursorFindColors(Current_cursor);

  if (cursor_moved) {
    /* If cursor moved while video was busy, force it hidden
       so it will be redrawn in the new position. */
    colorHideCursorAlways();
    cursor_moved = FALSE;
  }

  /* If the cursor is hidden, draw it */
  if (cursor_hidden){
    DrawCursorFuncPtr DrawCursorFunc = 
      ((DrawFuncs *)(pScreen->devPrivate))->DrawCursorFunc;
    
    cursor_moving = TRUE;
    (*DrawCursorFunc)(CursorLocation.x - CursorHot.x, CursorLocation.y - CursorHot.y);
    cursor_moving = FALSE;
    cursor_hidden = FALSE;
  }

  /* End of Hide/Draw/Show cycle.  Video is no longer busy. */
  video_busy = FALSE;
}




Bool colorDisplayCursor(pScr, pCurs)
ScreenPtr pScr;
CursorPtr pCurs;
{
  SetNewCursorFuncPtr SetNewCursorFunc =
    ((DrawFuncs *)(pScr->devPrivate)->SetNewCursorFunc;

  if (Current_cursor == pCurs) /* No change */
    return TRUE;

  /* Switch to new cursor after hiding old cursor */
  colorHideCursorAlways();
  Current_cursor = pCurs;
  (*SetNewCursorFunc)(pScr, pCurs);
  colorCursorRedoColors = TRUE;
  colorShowCursor();
  return TRUE ;
}





/* Called from vgaInstallColormap to take care of colormap switches */
void
colorCursorInstallColormap(pMap)
    ColormapPtr pMap;
{
    if (colorCursorColormap != pMap)
    {
#ifdef OLD
	colorHideCursorAlways();
	colorCursorColormap = pMap;
	colorCursorRedoColors = TRUE;
	colorShowCursor();
#else
/*	colorHideCursorAlways(); */
	colorCursorColormap = pMap;
        (*pMap->pScreen->RecolorCursor)(pMap->pScreen, Current_cursor, 1);
#endif
    }
} 


/* Called from vgaStoreColors to take care of color changes */

void
colorCursorStoreColors(pMap, ndef, pdef)
ColormapPtr pMap;
int ndef;
xColorItem *pdef;
{
int i;

  if (colorCursorColormap == pMap) {
    for (i = 0; i < ndef; i++){
      /*
       * XXX direct color will affect pixels other than
       * pdef[i].pixel -- this will be more difficult...
       */
      if (pdef[i].pixel == colorCursorForegroundPixel || 
  	  pdef[i].pixel == colorCursorBackgroundPixel) {
        (*pMap->pScreen->RecolorCursor) (pMap->pScreen, Current_cursor, 1);
	break;
      }
    }
  }
}




void
colorRecolorCursor(pScr, pCurs, displayed)
ScreenPtr pScr;
CursorPtr pCurs;
int       displayed;
{
  if ( (pCurs == Current_cursor) && (!colorCursorBeingRecolored) ) {
    colorHideCursorAlways();
    colorCursorRedoColors = TRUE;
    colorShowCursor();
  }
}



void colorConstrainCursor( pScr, pBox )
ScreenPtr	pScr ;
BoxPtr		pBox ;
{
  CursorConstrain.xmin = pBox->x1;
  CursorConstrain.ymin = pBox->y1;
  CursorConstrain.xmax = pBox->x2;
  CursorConstrain.ymax = pBox->y2;
}

#endif /* SOFTWARE_CURSOR */


void colorPointerNonInterestBox( pScr, pBox )
ScreenPtr	pScr ;
BoxPtr		pBox ;
{
}



#ifdef SOFTWARE_CURSOR

void colorCursorLimits( pScr, pCurs, pHotBox, pTopLeftBox )
ScreenPtr	pScr ;
CursorPtr	pCurs ;
BoxPtr		pHotBox ;
BoxPtr		pTopLeftBox ;
{
  pTopLeftBox->x1 = max( pHotBox->x1, 0 ) ;
  pTopLeftBox->y1 = max( pHotBox->y1, 0 ) ;
  pTopLeftBox->x2 = min( pHotBox->x2, pScr->width ) ;
  pTopLeftBox->y2 = min( pHotBox->y2, pScr->height ) ;
}


Bool colorSetCursorPosition( pScr, x, y, generateEvent )
ScreenPtr pScr ;
int x, y ;
Bool generateEvent ;
{
DevicePtr pointer_device;
xEvent	motion;

  colorHideCursorAlways();
  cursor_moving = TRUE;
  CursorLocation.x = x;
  CursorLocation.y = y;
  cursor_moving = FALSE;
  colorShowCursor();

  if (generateEvent){
    pointer_device = (DevicePtr)LookupPointerDevice();
    motion.u.u.detail = 0; /* ABSOLUTE COORDINATES */
    motion.u.keyButtonPointer.rootX = CursorLocation.x;
    motion.u.keyButtonPointer.rootY = CursorLocation.y;
    motion.u.keyButtonPointer.time = GetTimeInMillis();
    motion.u.u.type = MotionNotify;
    (*pointer_device->processInputProc)((xEvent *)&motion, pointer_device, 1);
  }

  return TRUE ;
}




void colorHideCursorInXYWH(x, y, w, h)
int x, y, w, h;
{
  video_busy = TRUE;
  if (!cursor_hidden &&
      (x+w  > CursorBounds.xmin) &&
      (x   <= CursorBounds.xmax) &&
      (y+h  > CursorBounds.ymin) &&
      (y   <= CursorBounds.ymax)){
    colorHideCursorAlways();
  }
}


void colorHideCursorInSpans(ppt, pwidth, count)
register DDXPointPtr ppt;
register int *pwidth;
register count;
{
  video_busy = TRUE;
  if (!cursor_hidden){
    for ( ; count-- > 0 ; ppt++, pwidth++ ){
      if ((ppt->x + *pwidth  > CursorBounds.xmin) &&
	  (ppt->x           <= CursorBounds.xmax) &&
	  (ppt->y           >= CursorBounds.ymin) &&
	  (ppt->y           <= CursorBounds.ymax)){
        colorHideCursorAlways();
	return;
      }
    }
  }
}


void colorHideCursorInLine(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
int _t;

  video_busy = TRUE;

  if (!cursor_hidden){
    if (x2 < x1) {
      _t = x1; x1 = x2; x2 = _t;
    }

    if (y2 < y1){
      _t = y1; y1 = y2; y2 = _t;
    }

    if ((x1 <= CursorBounds.xmax) &&
	(x2 >= CursorBounds.xmin) &&
	(y1 <= CursorBounds.ymax) &&
	(y2 >= CursorBounds.ymin)) {
      colorHideCursorAlways();
    }
  }
}

#endif /* SOFTWARE_CURSOR */


void vgaSetNewCursor(pScr, pCurs)
ScreenPtr pScr;
CursorPtr pCurs;
{
unsigned char *privSource, *privMask;

  cursor_width_pixels = min(pCurs->bits->width, 32);
  cursor_width_pixels = (cursor_width_pixels + 31) & ~31;
  cursor_width_bytes = cursor_width_pixels >> 3;
  cursor_height_pixels = min(pCurs->bits->height, 32);

  privSource = (unsigned char *) pCurs->devPriv[ pScr->myNum ];
  privMask = privSource + (cursor_width_bytes * cursor_height_pixels);

  bcopy((const char *) privSource, (char *) fgbits, cursor_width_bytes * cursor_height_pixels);
  bcopy((const char *) privMask, (char *) bgbits, cursor_width_bytes * cursor_height_pixels);

  CursorHot.x = pCurs->bits->xhot;
  CursorHot.y = pCurs->bits->yhot;
}


static unsigned char masktable[] = {
      0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static unsigned char revtable[] = {
      0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static reverseByte(int b)
{
int t = 0;
int i;

  for (i = 0; i<8; i++){
   if (b&masktable[i])
     t |= revtable[i];
  }  
  return (t&0xff);
}

#ifdef SOFTWARE_CURSOR

/*
** Change the position of the screen cursor relative to its curent position
** and return the new position as a long.  The upper 16 bits are current x
** and the lower 16 bits are current y.
*/
unsigned long
colorMoveCursor(delta_x, delta_y)
int delta_x, delta_y;
{
register int newx, newy;

  LimitMotionOutsideScreen(CursorLocation.x, CursorLocation.y,
			    &delta_x, &delta_y);

  newx = CursorLocation.x + delta_x;
  newy = CursorLocation.y + delta_y;
  if (newx < CursorConstrain.xmin)
    newx = CursorConstrain.xmin;
  else if (newx > CursorConstrain.xmax)
    newx = CursorConstrain.xmax;

  if (newy < CursorConstrain.ymin)
    newy = CursorConstrain.ymin;
  else if (newy > CursorConstrain.ymax)
    newy = CursorConstrain.ymax;

  if (newx != CursorLocation.x ||
      newy != CursorLocation.y)  {
    CursorLocation.x = newx;
    CursorLocation.y = newy;
    if (video_busy)
      cursor_moved = TRUE;
    else {
      colorHideCursorAlways();
      colorShowCursor();
    }
  }

  return (((unsigned long)(unsigned short)newx) << 16) | 
	  ((unsigned short) newy);
}


void colorCursorInit(pScr)
ScreenPtr pScr;
{
  Video_width_pixels = pScr->width;
  Video_height_pixels = pScr->height;

  /* Initialize Cursor State Variables */

  Current_cursor = NullCursor;
  cursor_moving = FALSE;
  video_busy = FALSE;
  cursor_moved = FALSE;
  cursor_hidden = TRUE;

  MoveCursor = colorMoveCursor;
}

#endif /* SOFTWARE_CURSOR */

#ifdef CURSOR_DEBUG
static dumpBitmap(unsigned char *bits, int width, int height)
{     
unsigned char *dp;
int row, column;
int padded = ((width & 7) != 0);
static char chars[] = {'-','#'};
    
    fprintf (stderr,"width: %d, height: %d\n",width,height);
    
    for (row = 0,dp=bits; row < height; row++,dp=bits+(row*8)) {
        for (column = 0; column < width; column++) {
            int i = (column & 7);
    
            if (*dp & masktable[i]) {
                putc (chars[1], stderr);
            } else {
                putc (chars[0], stderr);
            }
   
            if (i == 7) dp++;
        }
        putc ('\n', stderr);

    }
    
    
  return;
}
#endif
