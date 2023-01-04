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
static char SccsId[] = "@(#)gegenmagbx.c	1.2\t10/16/89";
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
**	GEGENMAGBX	             	       Scale given box coords by Mag %
**
**  ABSTRACT:
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 10/04/87 Created
**
**--
**/

#include "geGks.h"


geGenMagBx(Bx)
struct GE_BX	*Bx;
{
float f;

geMagMode = geState.Constraint;
if (geMagMode == GEHORIZONTAL || geMagMode == GENOCONSTRAINT)
   {Bx->x1     -= geMagCx;
    Bx->x2     -= geMagCx;
    Bx->x3     -= geMagCx;
    Bx->x4     -= geMagCx;
    f           = (float)Bx->x1 * geMagFX / 100.0;
    GEINT(f, Bx->x1);
    f           = (float)Bx->x2 * geMagFX / 100.0;
    GEINT(f, Bx->x2);
    f           = (float)Bx->x3 * geMagFX / 100.0;
    GEINT(f, Bx->x3);
    f           = (float)Bx->x4 * geMagFX / 100.0;
    GEINT(f, Bx->x4);
    Bx->x1     += geMagCx;
    Bx->x2     += geMagCx;
    Bx->x3     += geMagCx;
    Bx->x4     += geMagCx;
   }
if (geMagMode == GENOCONSTRAINT || geMagMode == GEVERTICAL)
   {Bx->y1     -= geMagCy;
    Bx->y2     -= geMagCy;
    Bx->y3     -= geMagCy;
    Bx->y4     -= geMagCy;
    f           = (float)Bx->y1 * geMagFY / 100.0;
    GEINT(f, Bx->y1);
    f           = (float)Bx->y2 * geMagFY / 100.0;
    GEINT(f, Bx->y2);
    f           = (float)Bx->y3 * geMagFY / 100.0;
    GEINT(f, Bx->y3);
    f           = (float)Bx->y4 * geMagFY / 100.0;
    GEINT(f, Bx->y4);
    Bx->y1     += geMagCy;
    Bx->y2     += geMagCy;
    Bx->y3     += geMagCy;
    Bx->y4     += geMagCy;
   }

}
