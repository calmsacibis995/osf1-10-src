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
#include <stdio.h>
#include "smpatterns.h"
#include <X11/Intrinsic.h>

/*
 * PAINTDEFS.H
 *
 * This is the definition file for all global variables and constants
 *
 */
#define MakeArg( n, v ){	args[numargs].name = n;\
				args[numargs].value = v;\
				numargs++;\
			}
/* PD stands for picture depth */
#define GC_PD_SOLID 0
#define GC_PD_ERASER 1
#define GC_PD_SQUARE_BRUSH 2
#define GC_PD_ROUND_BRUSH 3
#define GC_PD_OUTLINE 4
#define GC_PD_FILL 5
#define GC_PD_FLOOD 6
#define GC_PD_STROKE 7
#define GC_PD_COPY 8
#define GC_PD_FUNCTION 9
#define GC_PD_INVERT 10
#define GC_PD_SPRAY 11

/* SD stands for screen depth */
#define GC_SD_SOLID 12
#define GC_SD_ERASER 13
#define GC_SD_SQUARE_BRUSH 14
#define GC_SD_ROUND_BRUSH 15
#define GC_SD_OUTLINE 16
#define GC_SD_FILL 17
#define GC_SD_STROKE 18
#define GC_SD_COPY 19

/* D1 stands for depth 1 (bitmap) */
#define GC_D1_COPY 20
#define GC_D1_INVERT 21

#define GC_RUBBERBAND 22
#define NUMBER_OF_GCS 23
GC GCs[NUMBER_OF_GCS];
#define Get_GC(g) ( (int)GCs[g] ? (GC)(GCs[g]) : (GC)(GCs[g]=Create_GC(g)) )

/* Pattern variables */
#define PATTERN_WD 16
#define PATTERN_HT 16

XGCValues gc_values;
long window_fg, window_bg;
Pixmap patterns[NUM_PATTERNS];
Pixmap pattern_pixmap;
Pixmap fill_stipple;
long picture_fg, picture_bg;
Pixmap pxmap;
Pixmap btmap;
int pattern_sz, pattern_space;
