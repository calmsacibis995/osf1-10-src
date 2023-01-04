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
static char SccsId[] = "@(#)gegenrot90.c	1.1\t11/22/88";
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
** 	GEGENROT90	             	       Rotate given pt about given axis 90 deg.
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
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
** 	GNE 05/11/87 Created
**
**--                
**/
#include "geGks.h"

geGenRot90(x, y)
long *x, *y;
{long t;

*x -= geEditBox.Pt.x;                           /* Normalize pt wrt to center*/
*y -= geEditBox.Pt.y;                           /*of rotation                */
if (*x >= 0)                                    /* Quad 1 or 4               */
  {if (*y >= 0)                                 /* Quad 1                    */
    {if (!geRot.Clockwise)                      /* Counter-Clock -> Quad 4   */
       {t  = *x; *x = *y; *y = -t;}
     else                                       /*         Clock -> Quad 2   */
       {t  = *x; *x = -(*y); *y = t;}
     }
   else                                         /* Quad 4                    */
    {if (!geRot.Clockwise)                      /* Counter-Clock -> Quad 3   */
       {t  = *x; *x = *y; *y = -t;}
     else                                       /*         Clock -> Quad 1   */
       {t  = *x; *x = -(*y); *y = t;}
    }
  }
else
if (*x < 0)                                     /* Quad 2 or 3               */
  {if (*y >= 0)                                 /* Quad 2                    */
    {if (!geRot.Clockwise)                      /* Counter-Clock -> Quad 1   */
       {t  = *x; *x = *y; *y = -t;}
     else                                       /*         Clock -> Quad 3   */
       {t  = *x; *x = -(*y); *y = t;}
    }
   else                                         /* Quad 3                    */
    {if (!geRot.Clockwise)                      /* Counter-Clock -> Quad 2   */
       {t  = *x; *x = *y; *y = -t;}
     else                                       /*         Clock -> Quad 4   */
       {t  = *x; *x = -(*y); *y = t;}
    }
  }
*x += geEditBox.Pt.x;                           /* Remap to original coord   */
*y += geEditBox.Pt.y;                           /*space                      */

}
                                                 
