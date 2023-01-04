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
static char SccsId[] = "@(#)geterm.c	1.11\t9/19/89";
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
**	GETERM			Termination and clean up
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
**	GNE 10/30/86 Created
**
**--
**/

#include "geGks.h"


geTerm()
{                       
char                    *ptr;
int	                i;
static unsigned long    pixels[1];
struct GE_ALLOC_COL     *ColP, *ColPN;
struct GE_READ_COL_NAME *RColP, *RColPN;
struct GE_COL_LIST      *ColListP, *ColListPN;
struct GE_PAG		*APagP;
/*
 * Free (hopefully) all allocated buffers
 *
 * Free the X font structs
 */
for (i = 0; i < GE_MAX_FNTS; i++)
  {if (geFontUser[i] && geFontUser[i] != geDefaultFont)
     {XFreeFont(geDispDev, geFontUser[i]);
      geFontUser[i] = 0;
     }
  }

if (geFontMenu && geDefaultFont)
  {XFreeFont(geDispDev, geFontMenu);
   geFontMenu = 0;
  }
if (geDefaultFont)
  {XFreeFont(geDispDev, geDefaultFont);
   geDefaultFont = 0;
  }

/*
 * Free the allocated Graphics Contexts structs
 */
if (geGC1)
  {XFreeGC(geDispDev, geGC1);
   geGC1 = 0;
  }
if (geGC2)
  {XFreeGC(geDispDev, geGC2);
   geGC2 = 0;
  }
if (geGC3)
  {XFreeGC(geDispDev, geGC3);
   geGC3 = 0;
  }
if (geGC4)
  {XFreeGC(geDispDev, geGC4);
   geGC4 = 0;
  }
if (geGC5)
  {XFreeGC(geDispDev, geGC5);
   geGC5 = 0;
  }
if (geGC6)
  {XFreeGC(geDispDev, geGC6);
   geGC6 = 0;
  }
if (geGCImg)
  {XFreeGC(geDispDev, geGCImg);
   geGCImg = 0;
  }
if (geGCHT)
  {XFreeGC(geDispDev, geGCHT);
   geGCHT = 0;
  }
/*
 * Free the speciality cursors
 */
if (geWaitCursor)
  {XFreeCursor(geDispDev, geWaitCursor);
   geWaitCursor = 0;
  }
if (geTxtOCursor)
  {XFreeCursor(geDispDev, geTxtOCursor);
   geTxtOCursor = 0;
  }
if (geHandCursor)
  {XFreeCursor(geDispDev, geHandCursor);
   geHandCursor = 0;
  }
if (geTxtICursor)
  {XFreeCursor(geDispDev, geTxtICursor);
   geTxtICursor = 0;
  }
if (geAnimCursor)
  {XFreeCursor(geDispDev, geAnimCursor);
   geAnimCursor = 0;
  }
if (geMouseCursor)
  {XFreeCursor(geDispDev, geMouseCursor);
   geMouseCursor = 0;
  }
if (geFingerCursor)
  {XFreeCursor(geDispDev, geFingerCursor);
    geFingerCursor = 0;
  }
if (geCrossHairCursor)
  {XFreeCursor(geDispDev, geCrossHairCursor);
   geCrossHairCursor = 0;
  }
/*
 * Free the pixmaps used in the half-tone fills
 */
for (i = 0; i < GEMAXHT; i++)
  {if (geHTStipple[i]) XFreePixmap(geDispDev, geHTStipple[i]);
   geHTStipple[i] = 0;
  }
/*
 * Free the pixmaps used fill patterns
 */
for (i = 0; i < GEMAXPAT; i++)
  {if (gePATStipple[i]) XFreePixmap(geDispDev, gePATStipple[i]);
   gePATStipple[i] = 0;
  }
/*
 * Free Cut buff, if it exists
 */
if (geSegCut) geSegKil(&geSegCut);
/*
 * Free all the objects and buffers associated with a page and the page struct
 * itself.
 */
if (APagP = gePag0)
  {while (APagP->Next) APagP = APagP->Next;

   while (APagP)
     {geState.APagP = APagP;
      APagP = APagP->Prev;
      gePagKil(geState.APagP);
      if(APagP) APagP->Next = NULL;
     }

   gePag0 = NULL;
  }  

/*
 * If at least one color has already been allocated,
 * indicated by geCol0 being other than NULL, then free all colors.
 */
if (!GEMONO && (ColP = geCol0))
  {do
     {pixels[0] = ColP->Col.pixel;
      XFreeColors(geDispDev, geCmap, pixels, 1, 0);
      ColPN = ColP->Next;
      geFree(&ColP, sizeof(struct GE_ALLOC_COL));
     }while ((ColP = ColPN));
  }
geCol0 = 0;
/*
 * If linked list of named colors have been allocated then free them now
 */
if (RColP = geReadColName0)
  {do
     {geFree (&RColP->Name, NULL);
      RColPN = RColP->Next;
      geFree(&RColP, sizeof(struct GE_READ_COL_NAME));
     }while ((RColP = RColPN));
  }
geReadColName0 = 0;
/*
 * If user defined color list (from resource file) has been allocated, then
 * free it now.
 */
if (ColListP = geColList0)
  {do
     {geFree (&(ColListP->Col.Name), NULL);
      ColListPN = ColListP->Next;
      geFree(&ColListP, sizeof(struct GE_COL_LIST));
     }while ((ColListP = ColListPN));
  }
geColList0 = 0;
/*
 * Free the color map, if it's our own
 */
if (!GEMONO && geCmap && geCmap != XDefaultColormap(geDispDev, geScreen))
  XFreeColormap(geDispDev, geCmap);
geCmap = 0;

/*
 * Free the font, message and units strings
 */
i = 0;
while (i < GE_MAX_FNTS && geFntAlloced[i]) {geFree(&geFntAlloced[i], 0); i++;}
i = 0;
while (i < GE_MAX_UNITS && geU.Str[i])     {geFree(&geU.Str[i],      0); i++;}

#ifdef GERAGS
i = 0;
while (i < GE_MAX_MSGS && geMsgPtr[i])     {geFree(&geMsgPtr[i],     0); i ++;}
/*
 * Explicitly close the display
 */
MrmCloseHierarchy(geUI.Id);
XCloseDisplay(geDispDev);                       /* close display */
geDispDev = 0;
geTimeStamp(1);
if (geFdL)
    {fclose(geFdL);
     geFdL = NULL;
    }
#endif
                
} 
