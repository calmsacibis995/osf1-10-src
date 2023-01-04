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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmICCC.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:28:29 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#include <X11/Xatom.h>
#include <X11/Xmd.h>


/*
 * Value definitions and macros:
 */



/*************************************<->*************************************
 *
 *  ICCC property data structures ...
 *
 *
 *  Description:
 *  -----------
 *  These data structures are mwm versions of the window manager data
 *  stuctures that are specified in the ICCCM and the Xlib specification
 *  for client/window manager communication.  In general these data
 *  structures correspond to client window property values.
 *
 *************************************<->***********************************/


/* mwm version of the xPropSizeHints structure: */
#ifdef DEC_MOTIF_BUG_FIX
/* Fix for alpha and correct sizes */

typedef struct _PropSizeHints
{
    unsigned long flags;
    long	x;				/* R2 conventions */
    long	y;				/* R2 conventions */
    long	width;				/* R2 conventions */
    long	height;				/* R2 conventions */
    long	minWidth;
    long	minHeight;
    long	maxWidth;
    long	maxHeight;
    long	widthInc;
    long	heightInc;
    long	minAspectX;
    long	minAspectY;
    long	maxAspectX;
    long	maxAspectY;
    long	baseWidth;			/* current conventions */
    long	baseHeight;			/* current conventions */
    long	winGravity;			/* current conventions */

} PropSizeHints;

#else

typedef struct _PropSizeHints
{
    CARD32	flags;
    INT32	x;				/* R2 conventions */
    INT32	y;				/* R2 conventions */
    INT32	width;				/* R2 conventions */
    INT32	height;				/* R2 conventions */
    INT32	minWidth;
    INT32	minHeight;
    INT32	maxWidth;
    INT32	maxHeight;
    INT32	widthInc;
    INT32	heightInc;
    INT32	minAspectX;
    INT32	minAspectY;
    INT32	maxAspectX;
    INT32	maxAspectY;
    INT32	baseWidth;			/* current conventions */
    INT32	baseHeight;			/* current conventions */
    INT32	winGravity;			/* current conventions */

} PropSizeHints;

#endif /* DEC_MOTIF_BUG_FIX */
#define PROP_SIZE_HINTS_ELEMENTS	18


/* mwm version of the XSizeHints structure: */

typedef struct _SizeHints
{
    int		icccVersion;
    long	flags;
    int		x;				/* R2 conventions */
    int		y;				/* R2 conventions */
    int		width;				/* R2 conventions */
    int		height;				/* R2 conventions */
    int		min_width;
    int		min_height;
    int		max_width;
    int		max_height;
    int		width_inc;
    int		height_inc;
    AspectRatio min_aspect;
    AspectRatio max_aspect;
    int		base_width;			/* current conventions */
    int		base_height;			/* current conventions */
    int		win_gravity;			/* current conventions */

} SizeHints;

/* mwm version of the xPropWMState structure: */

#ifdef DEC_MOTIF_BUG_FIX
/* Fix for alpha and correct sizes */

typedef struct _PropWMState
{
    CARD32	state;
    BITS32	icon;
} PropWMState;

#else

typedef struct _PropWMState
{
    CARD32	state;
    BITS32	icon;
} PropWMState;

#endif /* DEC_MOTIF_BUG_FIX */

#define PROP_WM_STATE_ELEMENTS		2


/* ICCC versions (icccVersion): */
#define ICCC_R2		0
#define ICCC_CURRENT	1
#define ICCC_UNKNOWN	ICCC_CURRENT


/* SizeHints flags field values: */
#define US_POSITION		(1L << 0)
#define US_SIZE			(1L << 1)
#define P_POSITION		(1L << 2)
#define P_SIZE			(1L << 3)
#define P_MIN_SIZE		(1L << 4)
#define P_MAX_SIZE		(1L << 5)
#define P_RESIZE_INC		(1L << 6)
#define P_ASPECT		(1L << 7)
#define P_BASE_SIZE		(1L << 8)
#define P_WIN_GRAVITY		(1L << 9)

/* PropWMState state field value: */
#define WithdrawnSTATE		0
