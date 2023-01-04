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
static char SccsId[] = "@(#)gegensiocol.c	1.1\t11/22/88";
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
**	GEGENSIOCOL	             	       Read-Write Color (R G B)
**      GEGENNAMERGB                           Given an R,G,B   assign a name
**      GEGENNAMECMYK                          Given an C,M,Y,K assign a name
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
**	GNE 03/19/87 Created
**
**--
**/

#include "geGks.h"


extern char          *geMalloc(), *geGenLinkColName();
extern unsigned long geAllocColor();

geGenSIOCol(cmd, P)
long     	  cmd;
struct GE_COL_OBJ *P;
{                       
int              	r, g, b,c,m,y,k,oprint;
static unsigned long	plane_masks[1];
static unsigned long	pixels[1];
char	*error_string;

switch(cmd)
   {case GEWRITE:
      if (geVersionOut == GECURRENTVERSION)     /* LATEST                    */
	{if (P->Name && strlen(P->Name))
	   fprintf(geFdO, " %s",
		   P->Name);
	 else
	   fprintf(geFdO, " %s",
		   geDefColString);
	 fprintf(geFdO, " %d %d %d %d %d",
		 P->CMYK.C,
		 P->CMYK.M,
		 P->CMYK.Y,
		 P->CMYK.K,
		 P->OverPrint);
        }
      else                                      /* PREVIOUS                  */
	{fprintf(geFdO, " %d %d %d",
		 P->RGB.red,
		 P->RGB.green,
		 P->RGB.blue);
        }
      break;

    case GEREAD:
      switch (geVersionIn)
	{case 8:
	 case 7:
	   /*
	    * Diff between 7 and prev - there is a color name associated the
	    * percentages and the color is now in CMYK rather than RGB space.
	    */
	   geFscanf(geFdI, " %s",
		    geUtilBuf);
	   P->Name = geGenLinkColName(geUtilBuf);

	   geFscanf5(geFdI, " %d %d %d %d %d",
		     &c,
		     &m,
		     &y,
		     &k,
		     &oprint);
	   if (geFeof(geFdI)) 
	     {error_string = (char *) geFetchLiteral("GE_ERR_BADMETA",
						     MrmRtypeChar8);
	      if (error_string != NULL) 
		{geError(error_string, FALSE);
		 XtFree(error_string);
	        }
	      return(GERRBADMETA);
	     }

	   P->CMYK.C = c; P->CMYK.M = m; P->CMYK.Y = y; P->CMYK.K = k;
	   P->OverPrint = oprint;
	   geGenCmykToRgb(&P->CMYK, &P->RGB);

	   P->RGB.pixel = geAllocColor(P->RGB.red, P->RGB.green, P->RGB.blue,
				       NULL);

	   break;

	 case 6:
	 case 5:
	 case 4:
	 case 3:
	 case 2:
	 case 1:
	   geFscanf3(geFdI, " %d %d %d",
		     &r,
		     &g,
		     &b);
	   if (geFeof(geFdI)) 
	     {error_string = (char *) geFetchLiteral("GE_ERR_BADMETA",
						     MrmRtypeChar8);
	      if (error_string != NULL) 
		{geError(error_string, FALSE);
		 XtFree(error_string);
	        }
	      return(GERRBADMETA);
	     }

	   P->RGB.red = r; P->RGB.green = g; P->RGB.blue = b;
	   P->RGB.flags = DoRed|DoGreen|DoBlue;
	   P->RGB.pixel = geAllocColor(P->RGB.red, P->RGB.green, P->RGB.blue,
				       NULL);
	   P->OverPrint = FALSE;
	   /*
	    * Try to give the color a meaningful name
	    */
	   geGenNameRgb(P);                     /* Try to give it a name     */
	   geGenRgbToCmyk(&P->RGB, &P->CMYK);   /* Get CMYK equivalents      */
	   break;

	 default:
	   break;
	}
      break;

    default:
	break;
   }
return(0);
}

/*
 * Transform RGB to CMYK
 */
geGenRgbToCmyk(Rgb, Cmyk)
XColor         *Rgb;
struct GE_CMYK *Cmyk;
{
Cmyk->K = min((int)(GEMAXUSHORT) - (int)Rgb->red,
	      min((int)(GEMAXUSHORT) - (int)Rgb->green,
		  (int)(GEMAXUSHORT) - (int)Rgb->blue));
Cmyk->C = GEMAXUSHORT - Rgb->red   - Cmyk->K;
Cmyk->M = GEMAXUSHORT - Rgb->green - Cmyk->K;
Cmyk->Y = GEMAXUSHORT - Rgb->blue  - Cmyk->K;

}

/*
 * Transform CMYK to RGB
 */
geGenCmykToRgb(Cmyk, Rgb)
struct GE_CMYK *Cmyk;
XColor         *Rgb;
{
Rgb->red   = max(0, (int)(GEMAXUSHORT) - (int)Cmyk->C - (int)Cmyk->K);
Rgb->green = max(0, (int)(GEMAXUSHORT) - (int)Cmyk->M - (int)Cmyk->K);
Rgb->blue  = max(0, (int)(GEMAXUSHORT) - (int)Cmyk->Y - (int)Cmyk->K);
Rgb->flags = DoRed|DoGreen|DoBlue;
}

/*
 * Find ptr to color name in linked list or allocate a new entry for it.
 */
char *
geGenLinkColName(Name)
char              *Name;
{
struct GE_READ_COL_NAME *ColP, *ColPL;
int len;

if (Name && (len = strlen(Name)))
  {if (!geReadColName0)
     {/*
       * This is the first one read in
       */
       geReadColName0 = ColP = (struct GE_READ_COL_NAME *)
	 geMalloc(sizeof(struct GE_READ_COL_NAME));
       ColP->Next = NULL;
       ColP->Name = geMalloc(len + 1);
       strcpy (ColP->Name, Name);
     }
   else
     {/*
       * Try to find color name among those already allocated.
       */
      ColP = geReadColName0;
      do
	{if (ColP->Name && !strcmp(Name, ColP->Name)) break;
	 ColPL = ColP;
	} while (ColP = ColP->Next);
      
      if (!ColP)
	{/*
	  * Couldn't find it in the existing list, allocate a new entry..
	  */
	  ColP = (struct GE_READ_COL_NAME *)
	    geMalloc(sizeof(struct GE_READ_COL_NAME));
	  ColPL->Next = ColP;
	  ColP->Next  = NULL;
	  ColP->Name  = geMalloc(len + 1);
	  strcpy (ColP->Name, Name);
	}
     }

   return (ColP->Name);
 }
else
  return (NULL);                        /* Should NEVER happen               */

}

/*
 * Supplied with an RGB col struct try to give the color a meaningful name
 * The colors that can be readily named are combinations of R,G,B at 0
 * or 100%.
 */
geGenNameRgb(ColP)
struct GE_COL_OBJ *ColP;
{
int               test;

if (!ColP) return;

if ((ColP->RGB.red   > 0 && ColP->RGB.red  < GEMAXUSHORT) ||
    (ColP->RGB.green > 0 && ColP->RGB.green < GEMAXUSHORT) ||
    (ColP->RGB.blue  > 0 && ColP->RGB.blue  < GEMAXUSHORT))
  {ColP->Name = geDefColString;
   return;
  }

test = 0;
if (ColP->RGB.red == GEMAXUSHORT)   test |= 1;
if (ColP->RGB.green == GEMAXUSHORT) test |= 2;
if (ColP->RGB.blue == GEMAXUSHORT)  test |= 4;

switch (test)
  {case 0:  ColP->Name = "Black";   break;
   case 1:  ColP->Name = "Red";     break;
   case 2:  ColP->Name = "Green";   break;
   case 3:  ColP->Name = "Yellow";  break;
   case 4:  ColP->Name = "Blue";    break;
   case 5:  ColP->Name = "Magenta"; break;
   case 6:  ColP->Name = "Cyan";    break;
   default: ColP->Name = "White";   break;
  }

}

/*
 * Supplied with an CMYK col struct try to give the color a meaningful name.
 * The colors that can be readily named are combinations of C,M,Y,K at 0
 * or 100%.
 */
geGenNameCmyk(ColP)
struct GE_COL_OBJ *ColP;
{
int               test;

/* if (!ColP) return; */

if ((ColP->CMYK.C > 0 && ColP->CMYK.C < GEMAXUSHORT) ||
    (ColP->CMYK.M > 0 && ColP->CMYK.M < GEMAXUSHORT) ||
    (ColP->CMYK.Y > 0 && ColP->CMYK.Y < GEMAXUSHORT))
  {ColP->Name = geDefColString;
   return;
  }

test = 0;
if (ColP->CMYK.C == GEMAXUSHORT) test |= 1;
if (ColP->CMYK.M == GEMAXUSHORT) test |= 2;
if (ColP->CMYK.Y == GEMAXUSHORT) test |= 4;
if (ColP->CMYK.K == GEMAXUSHORT) test |= 8;
else
  {if (test != 7)
     {ColP->Name = geDefColString;
      return;
     }
  }
switch (test)
  {case 0:  ColP->Name = "White";   break;
   case 1:  ColP->Name = "Cyan";    break;
   case 2:  ColP->Name = "Magenta"; break;
   case 3:  ColP->Name = "Blue";    break;
   case 4:  ColP->Name = "Yellow";  break;
   case 5:  ColP->Name = "Green";   break;
   case 6:  ColP->Name = "Red";     break;
   default: ColP->Name = "Black"; break;
  }

}
