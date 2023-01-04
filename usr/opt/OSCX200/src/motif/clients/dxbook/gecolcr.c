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
static char SccsId[] = "@(#)gecolcr.c	1.2\t11/27/89";
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
**	GECOLCR			Allocates a new color cell (given RGB or name)
**
**  ABSTRACT:
**
** This routine will return a pixel value if it successfully makes the
** requested allocation (otherwise it returns 0).  The rgb values (as well as
** the pixel) are set in the global "geGenCol".
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/13/88 Created                            
**
**--
**/

#include "geGks.h"

extern	char          *geMalloc();

unsigned long
geColCr(R, G, B, NewCol)
unsigned short R, G, B;
short          NewCol;
{                                                                         
static short            second_try = FALSE, LastAllocSuc = TRUE;
unsigned long	        plane_masks[1], pixels[1], lumi;
struct GE_SEG           *ASegPSav, *Seg0Sav;
struct GE_PAG		*SPagP;
XColor                  Col, ColUsed;
struct GE_ALLOC_COL     *ColP, *ColP1, *Prev, *Next;
char			*error_string;

/*
 * If workstation is monochrome => return black or white depending on
 * luminence value.
 */
if (GEMONO)
   {lumi = (R + G + B) / 3;
    if (lumi < ((GEMAXUSHORT) >> 1))
	return(geBlack.pixel);
    else
    	return(geWhite.pixel);
   }
/*
 * If this color doesn't necessarily have to be unique, and at least one color has already been
 * allocated, indicated by geCol0 being other than NULL, then search the color list
 * to see if this RGB already exists.  If it does then return its pixel,
 * otherwise allocate a new color slot and cell and update the chain.
 */
if ((ColP = geCol0) && !NewCol)
  {for(;;)
     {if (ColP->Col.red == R && ColP->Col.green == G && ColP->Col.blue == B)
	{geGenCol     = ColP->Col;
	 LastAllocSuc = TRUE;
/*	 if (!geCol0User) geCol0User = ColP; */
	 return(geGenCol.pixel);
        }
      if (!ColP->Next) break;
      ColP = ColP->Next;
     }
  }
else
  ColP = geColL;
/*
 * Couldn't find a match among the existing colors - allocate a new color cell.
 */
Col.flags = DoRed|DoGreen|DoBlue;
Col.red   = R;
Col.green = G;
Col.blue  = B;
ColUsed   = Col;

geStat = XAllocColor(geDispDev, geCmap, &ColUsed);
/*
 * Color successfully allocated?
 */
if (geStat)                                     /* SUCCESS                   */
  {Col.pixel = ColUsed.pixel;
   geGenCol  = Col;
   if (ColP)
     {ColP->Next =
	(struct GE_ALLOC_COL *)geMalloc(sizeof(struct GE_ALLOC_COL));
      ColP = ColP->Next;
     }
   else      ColP = geCol0 =
     (struct GE_ALLOC_COL *)geMalloc(sizeof(struct GE_ALLOC_COL));
   geColL       = ColP;
   ColP->Next   = NULL;

   ColP->Col    = geGenCol;
   LastAllocSuc = TRUE;
   if (!geCol0User) geCol0User = ColP;
   return(geGenCol.pixel);
  }
#ifdef GERAGS
else                                            /* FAILURE                   */
  {if (!geCol0User || (geColStat  & GECOL_CMAP_INHIBIT_FREE) ||
       ((geColStat & GECOL_CMAP_INHIBIT_FREE_COND) &&
	(geColStat & GECOL_CMAP_FILLED_PASS1)) ||
       (geColStat  & GECOL_CMAP_FILLED_PASS2) || second_try || !gePag0 ||
       !gePag0->Next || !geSeg0)
     {
      		                                /* If already tried cleaning */
						/* or in initialization phase*/
						/* or there are no graphics  */
						/* segments, or not allowed  */
						/* to free unused cells      */
						/* return Black or */
						/* White, whichever is closer*/
	geColStat |= GECOL_CMAP_FILLED_PASS1;	/* Set flag - failed pass1   */

      if (LastAllocSuc && (geColStat & GECOL_OKTO_REPORT_NOALLOC))
	  					/* Yep, so tell the user   */
     	{error_string = (char *) geFetchLiteral("GE_ERR_XHARDCOL", MrmRtypeChar8);
         if (error_string != NULL) 		/* (ONLY ONCE) that there    */
	   {geError(error_string, FALSE);
	    XtFree(error_string);
	   }
      	 LastAllocSuc = FALSE;			/* are no more colors avail- */
	 geColStat |= GECOL_CMAP_INHIBIT_FREE_COND;
	}

      lumi = (R + G + B) / 3;			/* able and return BLACK or  */
      if (lumi < ((GEMAXUSHORT) >> 1))		/* WHITE pixel based on lumi-*/
	    geGenCol = geBlack;			/* nance value.		     */
      else
    	    geGenCol = geWhite;
      geColStat |= GECOL_CMAP_NOALLOC;		/* Returning Black or White  */
      return(geGenCol.pixel);
     }						/* END No retry, any reason  */

   else if (geCol0User)			        /* Try cleaning up unused    */
     {geColStat |= GECOL_CMAP_FILLED_PASS1;	/* Set flag - failed pass1   */
      second_try = TRUE;
      ASegPSav   = geState.ASegP;
      Seg0Sav    = geSeg0;

      geSetWaitCurs(TRUE);
      geMnStat(GEDISP, 78);			/* Put up message that       */
      XFlush(geDispDev);			/*we're doing it             */

      /*
       * Find the segment just before the 1st user allocated color - this
       * will have to be repointed, if the first user allocated color is
       * freed.
       */
      if (geCol0 && geCol0User && geCol0 != geCol0User)
	{ColP = geCol0;
	 while (ColP->Next && ColP->Next != geCol0User) ColP = ColP->Next;
	}
      if ((Prev = ColP))			/* If there is no previous   */
        {ColP       = geCol0User;		/* something VERY wrong	     */
         while (ColP)
	    {ColP1 = geCol0;			/* Go through colors alloca- */
	     geState.ASegP = NULL;              /* to menus and if this pixel*/
	     while (ColP1 != geCol0User)	/* is found there then       */
		{if (ColP1->Col.pixel == ColP->Col.pixel)
		    {geState.ASegP = geSeg0;	/* DON'T release it	     */
		     break;
		    }
		 ColP1 = ColP1->Next;
		}

	     if (!geState.ASegP)		/* Now search objects for    */
	     	{geGenCol = ColP->Col;          /* this color		     */
	         if (gePag0) SPagP = gePag0->Next;
		 else        SPagP = NULL;
	     	 while (SPagP && !geState.ASegP)
	   	    {geSeg0 = SPagP->Seg0;
	    	     geGrf(GEGETSEG_COL, geSeg0);
	    	     SPagP  = SPagP->Next;
	   	    }
		
	     	 if (!geState.ASegP)		/* There are NO segments    */
	   	    {pixels[0] = ColP->Col.pixel;/* using this color - free  */
	    	     XFreeColors(geDispDev, geCmap, pixels, 1, 0);
	    	     /*
	     	      * Adjust segment chain
	     	      */
		     Next       = ColP->Next;
	      	     Prev->Next = Next;
		     if (ColP == geCol0User) geCol0User = Next;
		     if (ColP == geColL) 	 geColL = Prev;
	       	     geFree(&ColP, sizeof(struct GE_ALLOC_COL));
	       	     ColP = Next;
	       	     continue;
	   	    }
		}
	     if (Prev) Prev = Prev->Next;
	     if (ColP) ColP = ColP->Next;

            }                                   /* Gone through all colors  */
	}
      geGenCol.pixel = geColCr(R, G, B, TRUE);
						/* Try to allocate the color*/
      second_try    = FALSE;                    /* again                    */
      geSeg0        = Seg0Sav;
      geState.ASegP = ASegPSav;

      geSetWaitCurs(FALSE);
      geMnStat(GEDISP, geState.MsgNum);

      return(geGenCol.pixel);
     }						/* END clean-up and retry    */
  }						/* END Color alloc FAILURE   */



/*
 * Allocation has failed
 */
geColStat |= GECOL_CMAP_FILLED_PASS2;

if (LastAllocSuc && geState.State != GEREAD)	/* Tell the user  	     */
   {error_string = (char *) geFetchLiteral("GE_ERR_XHARDCOL", MrmRtypeChar8);
    if (error_string != NULL)  			/* (ONLY ONCE) that there    */
      {geError(error_string, FALSE);		/* are no more colors avail- */
       XtFree(error_string);			/* able                      */
      }
    LastAllocSuc = FALSE;
   }

#endif						/* End of retries for RAGS   */

lumi = (R + G + B) / 3;				/* Return BLACK or  	     */
if (lumi < ((GEMAXUSHORT) >> 1))		/* WHITE pixel based on lumi-*/
    geGenCol = geBlack;				/* nance value.		     */
else
    geGenCol = geWhite;

#ifndef GERAGS
geColStat |= GECOL_CMAP_FILLED_PASS1;
#endif

geColStat |= GECOL_CMAP_NOALLOC;		/* Returning Black or White  */

return(geGenCol.pixel);
}
