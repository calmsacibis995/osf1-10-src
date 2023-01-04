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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: DragIcon.c,v $ $Revision: 1.1.4.4 $ $Date: 1993/10/18 15:29:28 $"
#endif
#endif
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <Xm/DragIconP.h>
#include <Xm/ScreenP.h>
#include "TextDIconI.h"
#include "DragCI.h"
#include "DragICCI.h"
#include "MessagesI.h"

#define MESSAGE1	_XmMsgDragIcon_0000
#define MESSAGE2	_XmMsgDragIcon_0001
#define MESSAGE3	_XmMsgDragIcon_0002

#define PIXMAP_MAX_WIDTH	128
#define PIXMAP_MAX_HEIGHT	128

#define TheDisplay(dd) (XtDisplayOfObject(XtParent(dd)))

typedef struct {
    unsigned int	width, height;
    int			hot_x, hot_y;
    int			offset_x, offset_y;
    char		*dataName;
    char		*data;
    char		*maskDataName;
    char		*maskData;
}XmCursorDataRec, *XmCursorData;


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void FetchScreenArg() ;
static Boolean XmCvtStringToBitmap() ;
static void DragIconClassInitialize() ;
static void DragIconInitialize() ;
static Boolean SetValues() ;
static void Destroy() ;
static void ScreenObjectDestroy() ;

#else

static void FetchScreenArg( 
                        Widget widget,
                        Cardinal *size,
                        XrmValue *value) ;
static Boolean XmCvtStringToBitmap( 
                        Display *dpy,
                        XrmValuePtr args,
                        Cardinal *num_args,
                        XrmValue *from_val,
                        XrmValue *to_val,
                        XtPointer *closure_ret) ;
static void DragIconClassInitialize( void ) ;
static void DragIconInitialize( 
                        Widget req,
                        Widget new_w,
                        ArgList args,
                        Cardinal *numArgs) ;
static Boolean SetValues( 
                        Widget current,
                        Widget req,
                        Widget new_w,
                        ArgList args,
                        Cardinal *num_args) ;
static void Destroy( 
                        Widget w) ;

static void ScreenObjectDestroy(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

/*
 *  The 16x16 default icon data.
 */

#define state16_width 16
#define state16_height 16
#define state16_x_hot 1
#define state16_y_hot 1
#define state16_x_offset -8
#define state16_y_offset -2
static char state16_bits[] =
{
   0x00, 0x00, 0x3e, 0x00, 0x1e, 0x00, 0x0e, 0x00, 0x06, 0x00, 0x02, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char state16M_bits[] =
{
   0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x3f, 0x00, 0x1f, 0x00, 0x0f, 0x00,
   0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static XmCursorDataRec state16CursorDataRec =
{
    state16_width, state16_height,
    state16_x_hot, state16_y_hot,
    state16_x_offset, state16_y_offset,
    "state16",
    state16_bits,
    "state16M",
    state16M_bits,
};

#define move16_width 16
#define move16_height 16
#define move16_x_hot 1
#define move16_y_hot 1
#define move16_x_offset -8
#define move16_y_offset -2
static char move16_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x40, 0x0c,
   0x40, 0x1c, 0x40, 0x3c, 0x40, 0x20, 0x40, 0x20, 0x40, 0x20, 0x40, 0x20,
   0x40, 0x20, 0x40, 0x20, 0xc0, 0x3f, 0x00, 0x00
};
static char move16M_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0xe0, 0x1f, 0xe0, 0x3f,
   0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f,
   0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f, 0xe0, 0x7f
};
static XmCursorDataRec move16CursorDataRec =
{
    move16_width, move16_height,
    move16_x_hot, move16_y_hot,
    move16_x_offset, move16_y_offset,
    "move16",
    move16_bits,
    "move16M",
    move16M_bits,
};

#define copy16_width 16
#define copy16_height 16
#define copy16_x_hot 1
#define copy16_y_hot 1
#define copy16_x_offset -8
#define copy16_y_offset -2
static char copy16_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x80, 0x0f, 0x80, 0x18, 0x80, 0x38, 0xb0, 0x78,
   0x90, 0x40, 0x90, 0x40, 0x90, 0x40, 0x90, 0x40, 0x90, 0x40, 0x90, 0x7f,
   0x10, 0x00, 0x10, 0x08, 0xf0, 0x0f, 0x00, 0x00
};
static char copy16M_bits[] =
{
   0x00, 0x00, 0xc0, 0x1f, 0xc0, 0x3f, 0xc0, 0x7f, 0xf8, 0xff, 0xf8, 0xff,
   0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff, 0xf8, 0xff,
   0xf8, 0xff, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f
};
static XmCursorDataRec copy16CursorDataRec =
{
    copy16_width, copy16_height,
    copy16_x_hot, copy16_y_hot,
    copy16_x_offset, copy16_y_offset,
    "copy16",
    copy16_bits,
    "copy16M",
    copy16M_bits,
};

#define link16_width 16
#define link16_height 16
#define link16_x_hot 1
#define link16_y_hot 1
#define link16_x_offset -8
#define link16_y_offset -2
static char link16_bits[] =
{
   0x00, 0x00, 0x80, 0x0f, 0x80, 0x18, 0x80, 0x38, 0x80, 0x78, 0xb8, 0x40,
   0x88, 0x4e, 0x88, 0x4c, 0x08, 0x4a, 0x08, 0x41, 0xa8, 0x7c, 0x68, 0x00,
   0xe8, 0x04, 0x08, 0x04, 0xf8, 0x07, 0x00, 0x00
};
static char link16M_bits[] =
{
   0xc0, 0x1f, 0xc0, 0x3f, 0xc0, 0x7f, 0xc0, 0xff, 0xfc, 0xff, 0xfc, 0xff,
   0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff, 0xfc, 0xff,
   0xfc, 0x0f, 0xfc, 0x0f, 0xfc, 0x0f, 0xfc, 0x0f
};
static XmCursorDataRec link16CursorDataRec =
{
    link16_width, link16_height,
    link16_x_hot, link16_y_hot,
    link16_x_offset, link16_y_offset,
    "link16",
    link16_bits,
    "link16M",
    link16M_bits,
};

#define source16_width 16
#define source16_height 16
#define source16_x_hot 0
#define source16_y_hot 0
static char source16_bits[] = 
{
   0x00, 0x00, 0xaa, 0xca, 0x54, 0x85, 0xaa, 0xca, 0x54, 0xe0, 0x2a, 0xe3,
   0x94, 0x81, 0xea, 0xf8, 0x54, 0xd4, 0xaa, 0xac, 0x94, 0xd9, 0xca, 0xac,
   0x64, 0xd6, 0x32, 0xab, 0xa4, 0xd6, 0xfe, 0xff
};
static XmCursorDataRec source16CursorDataRec =
{
    source16_width, source16_height,
    source16_x_hot, source16_y_hot,
    0, 0,
    "source16",
    /* a file icon */
    source16_bits,
    NULL,
    NULL,
};

/*
 *  The 32x32 default icon data.
 */

#define state32_width 32
#define state32_height 32
#define state32_x_hot 1
#define state32_y_hot 1
#define state32_x_offset -16
#define state32_y_offset -4
static char state32_bits[] = 
{
   0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
   0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char state32M_bits[] =
{
   0x0f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,
   0x7f, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
   0xff, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static XmCursorDataRec state32CursorDataRec =
{
    state32_width, state32_height,
    state32_x_hot, state32_y_hot,
    state32_x_offset, state32_y_offset,
    "state32",
    state32_bits,
    "state32M",
    state32M_bits,
};

#define move32_width 32
#define move32_height 32
#define move32_x_hot 1
#define move32_y_hot 1
#define move32_x_offset -16
#define move32_y_offset -4
static char move32_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0x20, 0x60, 0x00, 0x00,
   0x20, 0xe0, 0x00, 0x00, 0x20, 0xe0, 0x01, 0x00, 0x20, 0xe0, 0x03, 0x00,
   0x20, 0xe0, 0x07, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00,
   0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00,
   0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00,
   0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00,
   0xe0, 0xff, 0x0f, 0x00, 0xc0, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char move32M_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0x7f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xf0, 0xff, 0x01, 0x00,
   0xf0, 0xff, 0x03, 0x00, 0xf0, 0xff, 0x07, 0x00, 0xf0, 0xff, 0x0f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xe0, 0xff, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static XmCursorDataRec move32CursorDataRec =
{
    move32_width, move32_height,
    move32_x_hot, move32_y_hot,
    move32_x_offset, move32_y_offset,
    "move32",
    move32_bits,
    "move32M",
    move32M_bits,
};

#define copy32_width 32
#define copy32_height 32
#define copy32_x_hot 1
#define copy32_y_hot 1
#define copy32_x_offset -16
#define copy32_y_offset -4
static char copy32_bits[] = 
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0x00, 0x00, 0x20, 0x60, 0x00, 0x00,
   0x20, 0xe0, 0x00, 0x00, 0x20, 0xe0, 0x01, 0x00, 0x20, 0xe0, 0x03, 0x00,
   0x20, 0xe0, 0x07, 0x00, 0x20, 0x00, 0x0c, 0x00, 0x20, 0x00, 0x0c, 0x00,
   0x20, 0x00, 0x2c, 0x00, 0x20, 0x00, 0x6c, 0x00, 0x20, 0x00, 0xec, 0x00,
   0x20, 0x00, 0x8c, 0x01, 0x20, 0x00, 0x8c, 0x01, 0x20, 0x00, 0x8c, 0x01,
   0x20, 0x00, 0x8c, 0x01, 0x20, 0x00, 0x8c, 0x01, 0x20, 0x00, 0x8c, 0x01,
   0xe0, 0xff, 0x8f, 0x01, 0xc0, 0xff, 0x8f, 0x01, 0x00, 0x00, 0x80, 0x01,
   0x00, 0x04, 0x80, 0x01, 0x00, 0x04, 0x80, 0x01, 0x00, 0xfc, 0xff, 0x01,
   0x00, 0xf8, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00
};
static char copy32M_bits[] = 
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0x7f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xf0, 0xff, 0x01, 0x00,
   0xf0, 0xff, 0x03, 0x00, 0xf0, 0xff, 0x07, 0x00, 0xf0, 0xff, 0x0f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x1f, 0x00, 0xf0, 0xff, 0x7f, 0x00,
   0xf0, 0xff, 0xff, 0x00, 0xf0, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03,
   0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x03,
   0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x03,
   0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03,
   0x00, 0xfe, 0xff, 0x03, 0x00, 0xfe, 0xff, 0x03, 0x00, 0xfe, 0xff, 0x03,
   0x00, 0xfe, 0xff, 0x03, 0x00, 0xfc, 0xff, 0x03
};
static XmCursorDataRec copy32CursorDataRec =
{
    copy32_width, copy32_height,
    copy32_x_hot, copy32_y_hot,
    copy32_x_offset, copy32_y_offset,
    "copy32",
    copy32_bits,
    "copy32M",
    copy32M_bits,
};

#define link32_width 32
#define link32_height 32
#define link32_x_hot 1
#define link32_y_hot 1
#define link32_x_offset -16
#define link32_y_offset -4
static char link32_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0x00, 0x00, 0x20, 0x30, 0x00, 0x00,
   0x20, 0x70, 0x00, 0x00, 0x20, 0xf0, 0x00, 0x00, 0x20, 0xf0, 0x01, 0x00,
   0x20, 0x00, 0x7b, 0x00, 0x20, 0x00, 0xc3, 0x00, 0x20, 0x04, 0xc3, 0x01,
   0x20, 0x06, 0xc3, 0x03, 0x20, 0x0f, 0xc2, 0x07, 0x20, 0x36, 0x00, 0x0c,
   0x20, 0xc4, 0x00, 0x0c, 0x20, 0x00, 0x23, 0x0c, 0x20, 0x00, 0x6c, 0x0c,
   0x20, 0x00, 0xf0, 0x0c, 0xe0, 0xff, 0x61, 0x0c, 0xc0, 0xff, 0x23, 0x0c,
   0x00, 0x00, 0x00, 0x0c, 0x00, 0x80, 0x00, 0x0c, 0x00, 0x80, 0x00, 0x0c,
   0x00, 0x80, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static char link32M_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0x3f, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00,
   0xf0, 0xff, 0x01, 0x00, 0xf0, 0xff, 0x03, 0x00, 0xf0, 0xff, 0xff, 0x00,
   0xf0, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03, 0xf0, 0xff, 0xff, 0x07,
   0xf0, 0xff, 0xff, 0x0f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x1f,
   0xe0, 0xff, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0x1f,
   0x00, 0xc0, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0x1f, 0x00, 0x80, 0xff, 0x1f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static XmCursorDataRec link32CursorDataRec =
{
    link32_width, link32_height,
    link32_x_hot, link32_y_hot,
    link32_x_offset, link32_y_offset,
    "link32",
    link32_bits,
    "link32M",
    link32M_bits,
};

#define source32_width 32
#define source32_height 32
#define source32_x_hot 0
#define source32_y_hot 0
static char source32_bits[] =
{
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x54, 0x55, 0x55, 0xd5,
   0xa8, 0xaa, 0xaa, 0xea, 0x54, 0x55, 0x55, 0xd1, 0xa8, 0xaa, 0xaa, 0xe0,
   0x54, 0x55, 0x55, 0xd0, 0xa8, 0xaa, 0xaa, 0xf0, 0x54, 0x55, 0x55, 0xd9,
   0xa8, 0xaa, 0x00, 0xee, 0x54, 0x55, 0x00, 0xd6, 0xa8, 0x2a, 0x3f, 0xea,
   0x54, 0x95, 0x05, 0xd7, 0xa8, 0xea, 0x82, 0xee, 0x54, 0x45, 0xc1, 0xd5,
   0xa8, 0xaa, 0xf4, 0x81, 0x54, 0xd5, 0x78, 0xff, 0xa8, 0xaa, 0xa0, 0xea,
   0x54, 0x95, 0x41, 0xd5, 0xa8, 0x4a, 0x87, 0xea, 0x54, 0x85, 0x0d, 0xd5,
   0xa8, 0xc2, 0x9a, 0xea, 0x54, 0x61, 0x05, 0xd5, 0xa8, 0xb0, 0xc2, 0xea,
   0x54, 0x58, 0x61, 0xd5, 0x28, 0xac, 0xb0, 0xea, 0x14, 0x56, 0x58, 0xd5,
   0x08, 0xab, 0xa8, 0xea, 0x14, 0x55, 0x51, 0xd5, 0x28, 0xaa, 0xaa, 0xea,
   0xfc, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff,
};

static XmCursorDataRec source32CursorDataRec =
{
    source32_width, source32_height,
    source32_x_hot, source32_y_hot,
    0, 0,
    "source32",
    /* a file icon */
    source32_bits,
    NULL,
    NULL,
};

typedef struct _XmQuarkToCursorEntryRec{
    XrmQuark		*xrmName;
    XmCursorDataRec	*cursor;
}XmQuarkToCursorEntryRec, *XmQuarkToCursorEntry;

static XmQuarkToCursorEntryRec	quarkToCursorTable[] = {
    {&_XmValidCursorIconQuark, 	&state32CursorDataRec},
    {&_XmInvalidCursorIconQuark,&state32CursorDataRec},
    {&_XmNoneCursorIconQuark, 	&state32CursorDataRec},
    {&_XmMoveCursorIconQuark,	&move32CursorDataRec},
    {&_XmCopyCursorIconQuark,	&copy32CursorDataRec},
    {&_XmLinkCursorIconQuark,	&link32CursorDataRec},
    {&_XmDefaultDragIconQuark, 	&source32CursorDataRec},
};

static XmQuarkToCursorEntryRec	quarkTo16CursorTable[] = {
    {&_XmValidCursorIconQuark, 	&state16CursorDataRec},
    {&_XmInvalidCursorIconQuark,&state16CursorDataRec},
    {&_XmNoneCursorIconQuark, 	&state16CursorDataRec},
    {&_XmMoveCursorIconQuark,	&move16CursorDataRec},
    {&_XmCopyCursorIconQuark,	&copy16CursorDataRec},
    {&_XmLinkCursorIconQuark,	&link16CursorDataRec},
    {&_XmDefaultDragIconQuark, 	&source16CursorDataRec},
};

#undef Offset
#define Offset(x) (XtOffsetOf( struct _XmDragIconRec, drag.x))

static XContext _XmTextualDragIconContext = (XContext) NULL;

static XtResource resources[]=
{
    {
	XmNdepth, XmCDepth, XmRInt,
        sizeof(int), Offset(depth), 
        XmRImmediate, (XtPointer)1,
    },
    {
	XmNwidth, XmCWidth, XmRDimension,
	sizeof(Dimension), Offset(width),
	XmRImmediate, (XtPointer) 0,
    },
    {
	XmNheight, XmCHeight, XmRDimension,
	sizeof(Dimension), Offset(height),
	XmRImmediate, (XtPointer) 0,
    },
    {
	XmNhotX, XmCHot, XmRPosition,
        sizeof(Position), Offset(hot_x), 
        XmRImmediate, (XtPointer)0,
    },
    {
	XmNhotY, XmCHot, XmRPosition,
        sizeof(Position), Offset(hot_y),
        XmRImmediate, (XtPointer)0,
    },
    {
	XmNmask, XmCPixmap, XmRBitmap,
        sizeof(Pixmap), Offset(mask),
        XmRImmediate, (XtPointer)XmUNSPECIFIED_PIXMAP,
    },
    {
	XmNpixmap, XmCPixmap, XmRBitmap,
        sizeof(Pixmap), Offset(pixmap),
        XmRImmediate, (XtPointer)XmUNSPECIFIED_PIXMAP,
    },
    {
	XmNoffsetX, XmCOffset, XmRPosition,
        sizeof(Position), Offset(offset_x), 
        XmRImmediate, (XtPointer)0,
    },
    {
	XmNoffsetY, XmCOffset, XmRPosition,
        sizeof(Position), Offset(offset_y),
        XmRImmediate, (XtPointer)0,
    },
    {
	XmNattachment, XmCAttachment, XmRIconAttachment,
		sizeof(unsigned char), Offset(attachment),
		XmRImmediate, (XtPointer) XmATTACH_NORTH_WEST
    },
};

#ifdef DEC_MOTIF_BUG_FIX
externaldef(xmdragiconclassrec) XmDragIconClassRec xmDragIconClassRec = {
#else
XmDragIconClassRec xmDragIconClassRec = {
#endif
    {	
	(WidgetClass) &objectClassRec,	/* superclass		*/   
	"XmDragIcon",			/* class_name 		*/   
	sizeof(XmDragIconRec),		/* size 		*/   
	DragIconClassInitialize,	/* Class Initializer 	*/   
	NULL,				/* class_part_init 	*/ 
	FALSE, 				/* Class init'ed ? 	*/   
	DragIconInitialize,		/* initialize         	*/   
	NULL, 				/* initialize_notify    */ 
	NULL,	 			/* realize            	*/   
	NULL,	 			/* actions            	*/   
	0,				/* num_actions        	*/   
	resources,			/* resources          	*/   
	XtNumber(resources),		/* resource_count     	*/   
	NULLQUARK, 			/* xrm_class          	*/   
	FALSE, 				/* compress_motion    	*/   
	FALSE, 				/* compress_exposure  	*/   
	FALSE, 				/* compress_enterleave	*/   
	FALSE, 				/* visible_interest   	*/   
	Destroy,			/* destroy            	*/   
	NULL,		 		/* resize             	*/   
	NULL,				/* expose             	*/   
	SetValues, 			/* set_values		*/
	NULL, 				/* set_values_hook      */ 
	XtInheritSetValuesAlmost,	/* set_values_almost    */ 
	NULL,				/* get_values_hook      */ 
	NULL, 				/* accept_focus       	*/   
	XtVersion, 			/* intrinsics version 	*/   
	NULL, 				/* callback offsets   	*/   
	NULL,				/* tm_table           	*/   
	NULL, 				/* query_geometry       */ 
	NULL,				/* display_accelerator  */ 
	NULL, 				/* extension            */ 
    },	
    {					/* dragIcon		*/
	NULL,				/* extension		*/
    },
};

#ifdef DEC_MOTIF_BUG_FIX
/* Use variable name for psect name, CS 10-Aug-1993 */
externaldef(xmdragIconobjectclass) WidgetClass 
      xmDragIconObjectClass = (WidgetClass) &xmDragIconClassRec;
#else
externaldef(dragIconobjectclass) WidgetClass 
      xmDragIconObjectClass = (WidgetClass) &xmDragIconClassRec;
#endif

#define done( to_rtn, type, value, failure )            \
    {                                                   \
        static type buf ;                               \
                                                        \
        if(    to_rtn->addr    )                        \
        {                                               \
            if(    to_rtn->size < sizeof( type)    )    \
            {                                           \
                failure                                 \
                to_rtn->size = sizeof( type) ;          \
                return( FALSE) ;                        \
                }                                       \
            else                                        \
            {   *((type *) (to_rtn->addr)) = value ;    \
                }                                       \
            }                                           \
        else                                            \
        {   buf = value ;                               \
            to_rtn->addr = (XPointer) &buf ;            \
            }                                           \
        to_rtn->size = sizeof( type) ;                  \
        return( TRUE) ;                                 \
        } 

/************************************************************************
 *
 *  FetchScreenArg
 *
 ************************************************************************/
/*ARGSUSED*/
static void 
#ifdef _NO_PROTO
FetchScreenArg( widget, size, value )
        Widget widget ;
        Cardinal *size ;
        XrmValue *value ;
#else
FetchScreenArg(
        Widget widget,
        Cardinal *size,
        XrmValue *value )
#endif /* _NO_PROTO */
{
    if (widget == NULL) {
	XtErrorMsg("missingWidget", "fetchScreenArg", "XtToolkitError",
		   "FetchScreenArg called without a widget to reference",
		   (String*)NULL, (Cardinal*)NULL);
    }
    while (!XtIsWidget(widget))
	   widget = XtParent(widget);
    value->size = sizeof(Screen*);
    value->addr = (XPointer) XtScreen(widget);
}

static XtConvertArgRec bitmapConvertArgs[] = {
    {XtProcedureArg, (XtPointer)FetchScreenArg, 0},
};

/************************************************************************
 *
 *  XmCvtStringToBitmap
 *
 *  Convert a string to the pixmap of the dragIcon
 ************************************************************************/

static Boolean 
#ifdef _NO_PROTO
XmCvtStringToBitmap( dpy, args, num_args, from_val, to_val, closure_ret )
        Display *dpy ;
        XrmValuePtr args ;
        Cardinal *num_args ;
        XrmValue *from_val ;
        XrmValue *to_val ;
        XtPointer *closure_ret ;
#else
XmCvtStringToBitmap(
        Display *dpy,
        XrmValuePtr args,
        Cardinal *num_args,
        XrmValue *from_val,
        XrmValue *to_val,
        XtPointer *closure_ret )
#endif /* _NO_PROTO */
{
    char 		*imageName = (char *) (from_val->addr);
    Screen		*screen;
    Pixmap		pixmap = XmUNSPECIFIED_PIXMAP;

    if (*num_args != 1) {
	XtAppWarningMsg (XtDisplayToApplicationContext(dpy),
			 "wrongParameters", "cvtStringToBitmap",
			 "XtToolkitError", MESSAGE3,
			 (String *) NULL, (Cardinal *)NULL);
	return False;
    }
    screen = (Screen *)args[0].addr;
    
    pixmap = _XmGetPixmap (screen, imageName, 1, 1, 0);
    if (pixmap == XmUNSPECIFIED_PIXMAP) {
	XtDisplayStringConversionWarning(dpy, imageName, XmRBitmap);
	return False;
    }
    done( to_val, Pixmap, pixmap, ; )
}

/************************************************************************
 *
 *  DragIconClassInitialize
 *
 ************************************************************************/

static void 
#ifdef _NO_PROTO
DragIconClassInitialize()
#else
DragIconClassInitialize( void )
#endif /* _NO_PROTO */
{
    XtSetTypeConverter( XmRString, XmRBitmap, 
		       XmCvtStringToBitmap,
		       bitmapConvertArgs, 
		       XtNumber( bitmapConvertArgs),
		       XtCacheNone, NULL) ;
}


/************************************************************************
 *
 *  DragIconInitialize
 *
 ************************************************************************/

static void 
#ifdef _NO_PROTO
DragIconInitialize( req, new_w, args, numArgs )
        Widget req ;
        Widget new_w ;
        ArgList args ;
        Cardinal *numArgs ;
#else
DragIconInitialize(
        Widget req,
        Widget new_w,
        ArgList args,
        Cardinal *numArgs )
#endif /* _NO_PROTO */
{
    XmDragIconObject	dragIcon = (XmDragIconObject)new_w;
    Screen		*screen = XtScreenOfObject(XtParent(dragIcon));

    dragIcon->drag.isDirty = False;
    if (dragIcon->drag.pixmap == XmUNSPECIFIED_PIXMAP) {

	XmCursorData	cursorData = NULL;
	Cardinal	i = 0;
	XImage 		*image;
	Dimension	maxW, maxH;

	/*
	 *  If this is one of the default cursors (recognized by name)
	 *  then we use the built in images to generate the pixmap, its
	 *  mask (as appropriate), and its dimensions and hot spot.
	 */ 

	_XmGetMaxCursorSize (XtParent(dragIcon), &maxW, &maxH);

	if (maxW < 32 || maxH < 32) {
	    /*
	     *  Use small icons.
	     */
	    for (i = 0; i < XtNumber(quarkTo16CursorTable); i++) {
		if ((*(quarkTo16CursorTable[i].xrmName)) ==
	            dragIcon->object.xrm_name) {
	            cursorData = quarkTo16CursorTable[i].cursor;
	            break;
		}
	    }
	}
	else {
	    /*
	     *  Use large icons.
	     */
	    for (i = 0; i < XtNumber(quarkToCursorTable); i++) {
		if ((*(quarkToCursorTable[i].xrmName)) ==
	            dragIcon->object.xrm_name) {
	            cursorData = quarkToCursorTable[i].cursor;
	            break;
		}
	    }
	}

	if (cursorData) {

	    dragIcon->drag.depth = 1;
	    dragIcon->drag.width = cursorData->width;
	    dragIcon->drag.height = cursorData->height;
	    dragIcon->drag.hot_x = cursorData->hot_x;
	    dragIcon->drag.hot_y = cursorData->hot_y;
	    dragIcon->drag.offset_x = cursorData->offset_x;
	    dragIcon->drag.offset_y = cursorData->offset_y;

	    image = (XImage *) XtMalloc (sizeof (XImage));
	    _XmCreateImage(image, XtDisplay(new_w), cursorData->data,
			dragIcon->drag.width, dragIcon->drag.height, 
			LSBFirst);
    
	    _XmInstallImage(image, cursorData->dataName, 	
		            (int)dragIcon->drag.hot_x, 
		            (int)dragIcon->drag.hot_y);
	    dragIcon->drag.pixmap =
		_XmGetPixmap (screen, cursorData->dataName, 1, 1, 0);
    
	    if (cursorData->maskData) {
		image = (XImage *) XtMalloc (sizeof (XImage));
		_XmCreateImage(image, XtDisplay(new_w), cursorData->maskData,
			    dragIcon->drag.width, dragIcon->drag.height, 
			    LSBFirst);
	
		_XmInstallImage (image, cursorData->maskDataName, 0, 0);
	
		dragIcon->drag.mask =
		    _XmGetPixmap(screen, cursorData->maskDataName, 1, 1, 0);
	    }
	}
    }
    else if (dragIcon->drag.pixmap != XmUNSPECIFIED_PIXMAP) {
	if ((dragIcon->drag.width == 0) || (dragIcon->drag.height == 0)) {
	    
	    int			depth;
	    unsigned int	width, height;
	    int			hot_x, hot_y;
	    String		name;
	    Pixel		foreground, background;
	    
	    if (_XmGetPixmapData(screen,
				 dragIcon->drag.pixmap,
				 &name,
				 &depth, 
				 &foreground, &background,
				 &hot_x, &hot_y,
				 &width, &height)) {
		dragIcon->drag.depth = depth;
		dragIcon->drag.hot_x = hot_x;
		dragIcon->drag.hot_y = hot_y;
		dragIcon->drag.width = (Dimension)width;
		dragIcon->drag.height = (Dimension)height;
	    }
	    else {
		dragIcon->drag.width = 
		  dragIcon->drag.height = 0;
		dragIcon->drag.pixmap = XmUNSPECIFIED_PIXMAP;
		_XmWarning ((Widget) new_w, MESSAGE1);
	    }
	}
    }

    if (dragIcon->drag.pixmap == XmUNSPECIFIED_PIXMAP) {
	_XmWarning ((Widget) new_w, MESSAGE2);
    }
}

/************************************************************************
 *
 *  XmCreateDragIcon
 *
 ************************************************************************/

Widget 
#ifdef _NO_PROTO
XmCreateDragIcon( parent, name, argList, argCount )
        Widget parent ;
        String name ;
        ArgList argList ;
        Cardinal argCount ;
#else
XmCreateDragIcon(
        Widget parent,
        String name,
        ArgList argList,
        Cardinal argCount )
#endif /* _NO_PROTO */
{
    return (XtCreateWidget (name, xmDragIconObjectClass, parent,
		            argList, argCount));
}

/************************************************************************
 *
 *  _XmDestroyDefaultDragIcon ()
 *
 *  A default XmDragIcon's pixmap and mask (if present) were installed in
 *  the Xm pixmap cache from built-in images when the XmDragIcon was
 *  initialized.
 ************************************************************************/

void 
#ifdef _NO_PROTO
_XmDestroyDefaultDragIcon( icon )
	XmDragIconObject icon ;
#else
_XmDestroyDefaultDragIcon(
	XmDragIconObject icon)
#endif /* _NO_PROTO */
{
    Screen	*screen = XtScreenOfObject(XtParent(icon));

    if (icon->drag.pixmap != XmUNSPECIFIED_PIXMAP) {
	XmDestroyPixmap (screen, icon->drag.pixmap);
	icon->drag.pixmap = XmUNSPECIFIED_PIXMAP;
    }
    if (icon->drag.mask != XmUNSPECIFIED_PIXMAP) {
	XmDestroyPixmap (screen, icon->drag.mask);
	icon->drag.mask = XmUNSPECIFIED_PIXMAP;
    }
    XtDestroyWidget ((Widget) icon);
}

/************************************************************************
 *
 *  _XmDragIconIsDirty ()
 *
 *  Test the isDirty member of XmDragIconObject.
 ************************************************************************/

Boolean 
#ifdef _NO_PROTO
_XmDragIconIsDirty( icon )
	XmDragIconObject icon ;
#else
_XmDragIconIsDirty(
	XmDragIconObject icon)
#endif /* _NO_PROTO */
{
    return (icon->drag.isDirty);
}

/************************************************************************
 *
 *  _XmDragIconClean ()
 *
 *  Clear the isDirty member of XmDragIconObjects.
 ************************************************************************/

void
#ifdef _NO_PROTO
_XmDragIconClean( icon1, icon2, icon3 )
	XmDragIconObject icon1 ;
	XmDragIconObject icon2 ;
	XmDragIconObject icon3 ;
#else
_XmDragIconClean(
	XmDragIconObject icon1,
	XmDragIconObject icon2,
	XmDragIconObject icon3)
#endif /* _NO_PROTO */
{
    if (icon1)
	icon1->drag.isDirty = False;
    if (icon2)
	icon2->drag.isDirty = False;
    if (icon3)
	icon3->drag.isDirty = False;
}

/************************************************************************
 *
 *  SetValues
 *
 ************************************************************************/

static Boolean
#ifdef _NO_PROTO
SetValues( current, req, new_w, args, num_args )
    Widget	current;
    Widget	req;
    Widget	new_w;
    ArgList	args;
    Cardinal	*num_args;
#else
SetValues(
    Widget	current,
    Widget	req,
    Widget	new_w,
    ArgList	args,
    Cardinal	*num_args)
#endif /* _NO_PROTO */
{
    XmDragIconObject	newIcon = (XmDragIconObject) new_w;
    XmDragIconObject	oldIcon = (XmDragIconObject) current;

    /*
     *  Mark the icon as dirty if any of its resources have changed.
     */

    if ((newIcon->drag.depth != oldIcon->drag.depth) ||
	(newIcon->drag.pixmap != oldIcon->drag.pixmap) ||
	(newIcon->drag.mask != oldIcon->drag.mask) ||
	(newIcon->drag.width != oldIcon->drag.width) ||
	(newIcon->drag.height != oldIcon->drag.height) ||
	(newIcon->drag.attachment != oldIcon->drag.attachment) ||
	(newIcon->drag.offset_x != oldIcon->drag.offset_x) ||
        (newIcon->drag.offset_y != oldIcon->drag.offset_y) ||
	(newIcon->drag.hot_x != oldIcon->drag.hot_x) ||
        (newIcon->drag.hot_y != oldIcon->drag.hot_y)) {

	newIcon->drag.isDirty = True;
    }
    return False;
}

/************************************************************************
 *
 *  Destroy
 *
 *  Remove any cached cursors referencing this icon.
 ************************************************************************/

static void 
#ifdef _NO_PROTO
Destroy( w )
        Widget w ;
#else
Destroy(
        Widget w )
#endif /* _NO_PROTO */
{
    _XmScreenRemoveFromCursorCache ((XmDragIconObject) w);
}



/* ARGSUSED */
static void
#ifdef _NO_PROTO
ScreenObjectDestroy(w, client_data, call_data)
        Widget w;
        XtPointer client_data;
        XtPointer call_data;
#else
ScreenObjectDestroy(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif
{
   Widget drag_icon = (Widget) client_data;

   XtDestroyWidget(drag_icon);  /* destroy drag_icon */
   XDeleteContext(XtDisplay(w), RootWindowOfScreen(XtScreen(w)),  
		  _XmTextualDragIconContext);
}


Widget
#ifdef _NO_PROTO
_XmGetTextualDragIcon(w )
        Widget w ;
#else
_XmGetTextualDragIcon(
        Widget w )
#endif /* _NO_PROTO */
{
    Widget drag_icon;
    Arg args[10];
    int n = 0;
    Pixmap icon, icon_mask;
    Screen *screen = XtScreen(w);
    XImage *image;
    Window      root = RootWindowOfScreen(XtScreen(w));
    Widget screen_object;

   if (_XmTextualDragIconContext == (XContext) NULL)
      _XmTextualDragIconContext = XUniqueContext();

   if (XFindContext(XtDisplay(w), root,
                    _XmTextualDragIconContext, (char **) &drag_icon)) {
       Dimension height, width;
       int x_hot, y_hot;
       char *icon_bits;
       char *icon_mask_bits;

       _XmGetMaxCursorSize(w, &width, &height);

       if (width < 64 && height < 64) {
          icon_bits = XmTEXTUAL_DRAG_ICON_BITS_16;
          icon_mask_bits = XmTEXTUAL_DRAG_ICON_MASK_BITS_16;
          height = XmTEXTUAL_DRAG_ICON_HEIGHT_16;
          width = XmTEXTUAL_DRAG_ICON_WIDTH_16;
          x_hot = XmTEXTUAL_DRAG_ICON_X_HOT_16;
          y_hot = XmTEXTUAL_DRAG_ICON_Y_HOT_16;
       } else {
          icon_bits = XmTEXTUAL_DRAG_ICON_BITS_32;
          icon_mask_bits = XmTEXTUAL_DRAG_ICON_MASK_BITS_32;
          height = XmTEXTUAL_DRAG_ICON_HEIGHT_32;
          width = XmTEXTUAL_DRAG_ICON_WIDTH_32;
          x_hot = XmTEXTUAL_DRAG_ICON_X_HOT_32;
          y_hot = XmTEXTUAL_DRAG_ICON_Y_HOT_32;
       }

       image = (XImage *) XtMalloc (sizeof (XImage));
       _XmCreateImage(image, XtDisplay(w), icon_bits, width, height, LSBFirst);
       _XmInstallImage(image, "XmTextualDragIcon", x_hot, y_hot);
       icon = _XmGetPixmap(screen, "XmTextualDragIcon", 1, 1, 0);

       image = (XImage *) XtMalloc (sizeof (XImage));
       _XmCreateImage(image, XtDisplay(w), icon_mask_bits, 
		   width, height, LSBFirst);
       _XmInstallImage(image, "XmTextualDragIconMask", x_hot, y_hot);
       icon_mask = _XmGetPixmap(screen, "XmTextualDragIconMask", 1, 1, 0);
       screen_object = XmGetXmScreen(XtScreen(w));

       XtSetArg(args[n], XmNhotX, x_hot);  n++;
       XtSetArg(args[n], XmNhotY, y_hot);  n++;
       XtSetArg(args[n], XmNheight, height);  n++;
       XtSetArg(args[n], XmNwidth, width);  n++;
       XtSetArg(args[n], XmNmaxHeight, height);  n++;
       XtSetArg(args[n], XmNmaxWidth, width);  n++;
       XtSetArg(args[n], XmNmask, icon_mask);  n++;
       XtSetArg(args[n], XmNpixmap, icon);  n++;
       drag_icon = XtCreateWidget("drag_icon", xmDragIconObjectClass,
                                  screen_object, args, n);

       XSaveContext(XtDisplay(w), root,
                    _XmTextualDragIconContext, (char *) drag_icon);

       XtAddCallback(screen_object, XmNdestroyCallback, ScreenObjectDestroy,
                       (XtPointer) drag_icon);
   }

   return drag_icon;
}

