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
 * header file for transformed coordinate system.  No rotations
 * supported, as elipses cannot be rotated in X.
 */

typedef struct _transform {
	double	mx, bx;
	double	my, by;
} Transform;

typedef struct _TPoint {
	double	x, y;
} TPoint;

typedef struct _TRectangle {
	double	x, y, width, height;
} TRectangle;

# define Xx(x,y,t)	((int)((t)->mx * (x) + (t)->bx + 0.5))
# define Xy(x,y,t)	((int)((t)->my * (y) + (t)->by + 0.5))
# define Xwidth(w,h,t)	((int)((t)->mx * (w) + 0.5))
# define Xheight(w,h,t)	((int)((t)->my * (h) + 0.5))
# define Tx(x,y,t)	((((double) (x)) - (t)->bx) / (t)->mx)
# define Ty(x,y,t)	((((double) (y)) - (t)->by) / (t)->my)
# define Twidth(w,h,t)	(((double) (w)) / (t)->mx)
# define Theight(w,h,t)	(((double) (h)) / (t)->my)
