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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbdecline.h,v 1.1.2.3 92/03/30 17:46:24 Jim_Ludwig Exp $ */
/* Line painters */
extern void cfbLineSS();	    /* Solid single-pixel lines		    */
extern void cfbSegmentSS();	    /* Solid single-pixel segments	    */
extern void cfbDashLine();	    /* Solid single-pixel dashed lines      */
extern void cfbDashSegment();	    /* Solid single-pixel dashed segments   */


/* Dashed line stuff */
extern void InitDashStuff();	    /* Compute starting dash info	    */
extern void NewDashPos();	    /* Compute new dash info		    */

typedef struct {
    unsigned char   *pDash;	    /* Pointer to dash array		*/
    int		    numDashes;      /* Number of dashes in array	*/
    int		    dashLength;     /* +/pDash[0..numDashes-1]		*/
} DashDesc;

#define EVENDASH   0
#define ODDDASH    (~EVENDASH)

typedef struct {
    int		    major;	    /* index in pDash[0..numDashes]     */
    int		    minor;	    /* # left from pDash[major]		*/
    int		    which;	    /* EVEN_DASH or ODD_DASH		*/
} DashPos;


/* Line clipping stuff */

extern int  cfbClipLine();	    /* Compute clipped endpoints	    */

/* Out of clip region codes */
#define OUT_LEFT 0x08
#define OUT_RIGHT 0x04
#define OUT_ABOVE 0x02
#define OUT_BELOW 0x01

#define SHIFT_LEFT 3
#define SHIFT_RIGHT 2
#define SHIFT_ABOVE 1
#define SHIFT_BELOW 0


/* I haven't really tested the shifting outcode version on anything but an
   R2000.  It definitely runs faster on that by a substantial margin, probably
   because the scheduler is happier given large basic blocks rather than many
   small ones.  Maybe this code should be ifdefed if we find the opposite is
   true on other machines.
*/

/*
#define OUTCODES(result, x, y, pbox)    \
    result = 0;				\
    if (x < pbox->x1)			\
	result = OUT_LEFT;		\
    else if (x >= pbox->x2)		\
	result = OUT_RIGHT;		\
    if (y < pbox->y1)			\
	result |= OUT_ABOVE;		\
    else if (y >= pbox->y2)		\
	result |= OUT_BELOW;
*/
#define OUTCODES(result, x, y, pbox)    \
    result = ((x <  pbox->x1) << SHIFT_LEFT)    \
	   | ((x >= pbox->x2) << SHIFT_RIGHT)   \
	   | ((y <  pbox->y1) << SHIFT_ABOVE)   \
	   | ((y >= pbox->y2) << SHIFT_BELOW)


