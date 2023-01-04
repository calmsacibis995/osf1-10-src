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
static char SccsId[] = "@(#)gegenrotx.c	1.1\t11/22/88";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
** 	GEGENROTX              	       Rotate given pt about given axis x deg.
**
**  ABSTRACT:
**
** Note the following quadrant numbering convention is hereby adopted
**
**                     -
**
**                     Y
**
**                     |->
**            3        |->     4  
**                     |->     
**        ^ ^ ^ ^ ^ ^  |->
**        | | | | | |  |->
**    -  ----------------------------- X +
**                   <-| | | | | | | |
**                   <-| V V V V V V V
**                   <-|
**            2      <-|       1
**                   <-|
**
**                     +
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
** 	GNE 05/12/87 Created
**
**--                
**/
#include "geGks.h"
#include <math.h>

geGenRotX(x, y)
long  *x, *y;
{
double r,                                       /* Dist. pt(x,y) to rot axis */
       alpha,                                   /* Angle of rot wrt cur loc  */
       beta,                                    /* Cur angle wrt to x-axis   */
       gama;                                    /* Result angle wrt x-axis   */

if (geRot.Clockwise) alpha =  (double)geRot.Beta;
else                 alpha = -(double)geRot.Alpha;
*x -= geEditBox.Pt.x;                           /* Normalize pt wrt to center*/
*y -= geEditBox.Pt.y;                           /*of rotation                */
*y  = -*y;                                      /* Y axis topsy-turvy        */
if (r = hypot((double)*x, (double)*y))
  {beta = asin((double)((double)*y / r)) * GE_RAD_TO_DEG;
   if (*x < 0) beta = 180 - beta;
   gama = beta - alpha;
   *x   = r * cos(gama * GE_DEG_TO_RAD);
   *y   = r * sin(gama * GE_DEG_TO_RAD);
  }
*y  = -*y;                                      /* Y axis topsy-turvy        */
*x += geEditBox.Pt.x;                           /* Remap to original coord   */
*y += geEditBox.Pt.y;                           /*space                      */

}
                                                 
geGenRotObj(ASegP, Angle)
struct GE_SEG *ASegP;
float	      Angle;
{
float    RotSavA, RotSavB;
unsigned RotSavC;

/*
 * Save global rotation constants
 */
RotSavA 	= geRot.Alpha;
RotSavB 	= geRot.Beta;
RotSavC 	= geRot.Clockwise;
/*
 * Perform the operation
 */
if (Angle > 0.) geRot.Clockwise = FALSE;
else            geRot.Clockwise = TRUE;
geRot.Alpha = geRot.Beta = fabs(Angle);
GEVEC(GEROTFIXED, ASegP);
/*
 * Restore global rotation constants
 */
geRot.Alpha     = RotSavA;
geRot.Beta      = RotSavB;
geRot.Clockwise = RotSavC;

}

geGenRotPt(Pt, Angle)
struct GE_PT *Pt;
float	      Angle;
{
float    RotSavA, RotSavB;
unsigned RotSavC;

/*
 * Save global rotation constants
 */
RotSavA 	= geRot.Alpha;
RotSavB 	= geRot.Beta;
RotSavC 	= geRot.Clockwise;
/*
 * Perform the operation
 */
if (Angle > 0.) geRot.Clockwise = FALSE;
else            geRot.Clockwise = TRUE;
geRot.Alpha = geRot.Beta = fabs(Angle);
geGenRotX(&Pt->x, &Pt->y);
/*
 * Restore global rotation constants
 */
geRot.Alpha     = RotSavA;
geRot.Beta      = RotSavB;
geRot.Clockwise = RotSavC;

}
