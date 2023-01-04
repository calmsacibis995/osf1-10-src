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
static char SccsId[] = "@(#)gegrfio.c	1.2\t1/31/90";
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
**	GEGRFIO	             	       Graph object IO handler
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
**	GNE 04/13/87 Created
**
**--
**/

#include "geGks.h"
#include <math.h>

extern char          *geMalloc();
extern	             geLin(), geImg();

geGrfIO(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
static int           	c, PixWidth, MMWidth, i, i1, i2, i3, i4, i5, i6, i7, 
                        i8, i9, i10;
static float		CurRes, OldRes;

char                 	p[10];
unsigned char		*up1, *up2;
float                   MagFXSav, MagFYSav;
struct GE_SEG        	*GSegP, *SSegP, TSegP;

#ifdef GERAGS
static int              ttag = GERESET;
#endif

if (!(SSegP = ASegP)) return;


if (cmd == GEWRITE)
  {if (geVersionOut == GECURRENTVERSION)
     {GEVEC(GEBOUNDS, geSeg0);
      fprintf(geFdO, " %d", geState.PrintPgBg);
      if (geGenSIOCo (cmd, &ASegP->Co))  		        return;
      if (geGenSIOCo (cmd, &geState.APagP->Crop)) 	        return;
      if (geGenSIOCol(cmd, &ASegP->Col))  		        return;
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      geState.APagP->PixWidth = XDisplayWidth  (geDispDev, geScreen);
      geState.APagP->MMWidth  = XDisplayWidthMM(geDispDev, geScreen);
      fprintf(geFdO, " %d %d",
	      geState.APagP->PixWidth,
	      geState.APagP->MMWidth);
      fprintf(geFdO, " %d %d %d %d %d %d %d %d %d %d",
	      geState.Grid.On,
	      geState.Grid.Xorg,
	      geState.Grid.Yorg,
	      geState.Grid.MajorX,
	      geState.Grid.MajorY,
	      geState.Grid.DivX,
	      geState.Grid.DivY,
	      geState.Grid.Top,
	      geState.Grid.XAlign,
	      geState.Grid.YAlign);
      if (geGenSIOTM (cmd, &geCTM))  		                return;
      fprintf(geFdO, " %d\n",
	      geState.APagP->PixmapAnim);
      /*
       * Before writing out the page description, make sure it and the
       * description contained in Seg0 agree; if they are different, stuff
       * the one in the page struct into the segment struct.
       */
      if (!geState.APagP->Descrip  || !ASegP->Descrip ||
	  strcmp(geState.APagP->Descrip, ASegP->Descrip))
	{/*
	  * They differ, so stuff the page description into Seg0
	  */
	 if (ASegP->Descrip)
	   geFree(&ASegP->Descrip, 0);

	 if (geState.APagP->Descrip && (i = strlen(geState.APagP->Descrip)))
	   {ASegP->Descrip = (unsigned char *)geMalloc(i + 1);
	    strcpy(ASegP->Descrip, geState.APagP->Descrip);
	   }
	 }

      if (geGenSIODesc(cmd, ASegP))				return;
      if (geGenSIOAnim(cmd, ASegP))				return;


      while (SSegP = SSegP->Next)
	{if (SSegP->Live && SSegP->Handler)
	   {fputc  ('{' ,geFdO);
	    fputs  (SSegP->Handle ,geFdO);
	    /*
	     * For all objects, except line, if the interior is not going
	     *to be filled, then set its color to black so that 1) it will
	     *take less space in the meta file and 2) it will not needlessly
	     *exhaust a color cell.
	     */
	    if (SSegP->FillStyle == GETRANSPARENT && SSegP->Handler != geLin)
	      SSegP->Col.RGB = geBlack;
	    GEVEC(GEWRITE, SSegP);
	    fputc  ('}' ,geFdO);
	    fputc  ('\n' ,geFdO);
	   }
        }
     }
  else                                       /* VERSION 5                    */
    {GEVEC(GEBOUNDS, geSeg0);
     fprintf(geFdO, " %d", geState.PrintPgBg);
     if (geGenSIOCo (cmd, &ASegP->Co))  		        return;
     if (geGenSIOCo (cmd, &geState.APagP->Crop)) 	        return;
     if (geGenSIOCol(cmd, &ASegP->Col))  		        return;
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))   return;
     fprintf(geFdO, " %d", ASegP->FillWritingMode);
     geState.APagP->PixWidth = XDisplayWidth  (geDispDev, geScreen);
     geState.APagP->MMWidth  = XDisplayWidthMM(geDispDev, geScreen);
     fprintf(geFdO, " %d %d",
	     geState.APagP->PixWidth,
	     geState.APagP->MMWidth);
     fputc  ('\n' ,geFdO);
     
     while (SSegP = SSegP->Next)
       {if (SSegP->Live && SSegP->Handler)
	  {fputc  ('{' ,geFdO);
	   fputs  (SSegP->Handle ,geFdO);
	   /*
	    * For all objects, except line, if the interior is not going
	    *to be filled, then set its color to black so that 1) it will
	    *take less space in the meta file and 2) it will not needlessly
	    *exhaust a color cell.
	    */
	   if (SSegP->FillStyle == GETRANSPARENT && SSegP->Handler != geLin)
	     SSegP->Col.RGB = geBlack;
	   GEVEC(GEWRITE, SSegP);
	   fputc  ('}' ,geFdO);
	   fputc  ('\n' ,geFdO);
	  }
       }
    }

   return;
 }

/*
 * Must be reading in - the input format is dependant on the version number.
 * First case is latest version.
 */
switch (geVersionIn)
   {case 8:
     /*
      * Diff between 8 and prev:
      *		- the transformation matrix has been moved beyond the bounding
      *           box info so DOCUMENT still works correctly
      *		- Moved the descriptor to BEFORE the animation parameters, this
      *		  is to elliminate the possibility of confusion for TEXT objs.
      *		  whose content happens to be exactly the descriptor start
      *		  string.
      */
     if (GSegP = ASegP)
       {geFscanf1(geFdI, " %d", &i);
	geState.PrintPgBgNu = i;
	geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	   if (geGenSIOFil(cmd, &TSegP.FillStyle, &TSegP.FillHT))  return;
	   geFscanf1(geFdI, " %d", &TSegP.FillWritingMode);
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	   geAttr.PagCol = ASegP->Col;
	  }

	geFscanf2(geFdI, " %d %d",
	       &geState.APagP->PixWidth,
	       &geState.APagP->MMWidth);

	geFscanf10(geFdI, " %d %d %d %d %d %d %d %d %d %d",
		  &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9, &i10);
	if (!geState.LiveSegs)
	  {geState.Grid.On     = i1;
	   geState.Grid.Xorg   = i2;
	   geState.Grid.Yorg   = i3;
	   geState.Grid.MajorX = i4;
	   geState.Grid.MajorY = i5;
	   geState.Grid.DivX   = i6;
	   geState.Grid.DivY   = i7;
	   geState.Grid.Top    = i8;
	   geState.Grid.XAlign = i9;
	   geState.Grid.YAlign = i10;
	   geState.Grid.FMajorX = (float)geState.Grid.MajorX;
	   geState.Grid.FMajorY = (float)geState.Grid.MajorY;
	   /*
	    * Minor X and Y divisions in SUIs
	    */
	   if (geState.Grid.DivX)
  	     {geFT = geState.Grid.FMajorX / (float)geState.Grid.DivX;
	      GEINT(geFT, geState.Grid.MinorX);
	     }	
	   if (geState.Grid.DivY)
  	     {geFT = geState.Grid.FMajorY / (float)geState.Grid.DivY;
	      GEINT(geFT, geState.Grid.MinorY);
	     }	
#ifdef GERAGS
	   geMnGrid(NULL, &ttag, NULL);
#endif
	  }

        if (geGenSIOTM (cmd, &geCTM))  		                return;
	geFscanf1(geFdI, " %d",
		  &i);

	geState.APagP->PixmapAnim = i;

	c = geFgetc(geFdI);                     /* Read past new line        */

	if (geGenSIODesc(cmd, ASegP))			     return;
	/*
	 * If there are NO segments already on the page use the new
	 * DESCRIPTION; otherwise keep the current one.
	 */
	if (!geState.LiveSegs)
	  {if (geState.APagP->Descrip)
	     geFree(&geState.APagP->Descrip, 0);

	   if (ASegP->Descrip && (i = strlen(ASegP->Descrip)) >= 1)
	     {geState.APagP->Descrip = (unsigned char *)geMalloc(i + 1);
	      strcpy(geState.APagP->Descrip, ASegP->Descrip);
	     }
	  }

	if (geGenSIOAnim(cmd, ASegP))			     return;

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */

	   if (SSegP->Handle)
	     {GEVEC(cmd, SSegP);}

	   c = geFgetc(geFdI);                 /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                 /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;

    case 7:
     /*
      * Diff between 7 and prev:
      *		- this contains the transformation matrix associated with
      *		  the page.
      */
     if (GSegP = ASegP)
       {if (geGenSIOTM (cmd, &geCTM))  		                return;
        geFscanf1(geFdI, " %d", &i);
	geState.PrintPgBgNu = i;
	geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	   if (geGenSIOFil(cmd, &TSegP.FillStyle, &TSegP.FillHT))  return;
	   geFscanf1(geFdI, " %d", &TSegP.FillWritingMode);
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	   geAttr.PagCol = ASegP->Col;
	  }

	geFscanf2(geFdI, " %d %d",
	       &geState.APagP->PixWidth,
	       &geState.APagP->MMWidth);

	geFscanf10(geFdI, " %d %d %d %d %d %d %d %d %d %d",
		  &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9, &i10);
	if (!geState.LiveSegs)
	  {geState.Grid.On     = i1;
	   geState.Grid.Xorg   = i2;
	   geState.Grid.Yorg   = i3;
	   geState.Grid.MajorX = i4;
	   geState.Grid.MajorY = i5;
	   geState.Grid.DivX   = i6;
	   geState.Grid.DivY   = i7;
	   geState.Grid.Top    = i8;
	   geState.Grid.XAlign = i9;
	   geState.Grid.YAlign = i10;
	   geState.Grid.FMajorX = (float)geState.Grid.MajorX;
	   geState.Grid.FMajorY = (float)geState.Grid.MajorY;
	   /*
	    * Minor X and Y divisions in SUIs
	    */
	   if (geState.Grid.DivX)
  	     {geFT = geState.Grid.FMajorX / (float)geState.Grid.DivX;
	      GEINT(geFT, geState.Grid.MinorX);
	     }	
	   if (geState.Grid.DivY)
  	     {geFT = geState.Grid.FMajorY / (float)geState.Grid.DivY;
	      GEINT(geFT, geState.Grid.MinorY);
	     }	
#ifdef GERAGS
	   geMnGrid(NULL, &ttag, NULL);
#endif
	  }
	c = geFgetc(geFdI);                     /* Read past new line        */

	if (geGenSIOAnim(cmd, ASegP))			     return;
	if (geGenSIODesc(cmd, ASegP))			     return;
	/*
	 * If there are NO segments already on the page use the new
	 * DESCRIPTION; otherwise keep the current one.
	 */
	if (!geState.LiveSegs)
	  {if (geState.APagP->Descrip)
	     geFree(&geState.APagP->Descrip, 0);

	   if (ASegP->Descrip && (i = strlen(ASegP->Descrip)) >= 1)
	     {geState.APagP->Descrip = (unsigned char *)geMalloc(i + 1);
	      strcpy(geState.APagP->Descrip, ASegP->Descrip);
	     }
	  }

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */

	   if (SSegP->Handle)
	     {GEVEC(cmd, SSegP);}

	   c = geFgetc(geFdI);                 /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                 /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;
    case 6:
     /*
      * Diff between 6 and prev - this contains a description of the grid
      * associated with the page.
      */
     if (GSegP = ASegP)
       {geFscanf1(geFdI, " %d", &i);
	geState.PrintPgBgNu = i;
	geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	   if (geGenSIOFil(cmd, &TSegP.FillStyle, &TSegP.FillHT))  return;
	   geFscanf1(geFdI, " %d", &TSegP.FillWritingMode);
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	   geAttr.PagCol = ASegP->Col;
	  }

	geFscanf2(geFdI, " %d %d",
	       &geState.APagP->PixWidth,
	       &geState.APagP->MMWidth);

	geFscanf10(geFdI, " %d %d %d %d %d %d %d %d %d %d",
		  &i1, &i2, &i3, &i4, &i5, &i6, &i7, &i8, &i9, &i10);
	if (!geState.LiveSegs)
	  {geState.Grid.On     = i1;
	   geState.Grid.Xorg   = i2;
	   geState.Grid.Yorg   = i3;
	   geState.Grid.MajorX = i4;
	   geState.Grid.MajorY = i5;
	   geState.Grid.DivX   = i6;
	   geState.Grid.DivY   = i7;
	   geState.Grid.Top    = i8;
	   geState.Grid.XAlign = i9;
	   geState.Grid.YAlign = i10;
	   /*
	    * Note: the following (to the "}") was left out of the original
	    * version 6 reading code which was packaged with the Bookreader
	    * and the CBI
	    */
	   geState.Grid.FMajorX = (float)geState.Grid.MajorX;
	   geState.Grid.FMajorY = (float)geState.Grid.MajorY;
	   /*
	    * Minor X and Y divisions in SUIs
	    */
	   if (geState.Grid.DivX)
	     {geFT = geState.Grid.FMajorX / (float)geState.Grid.DivX;
	      GEINT(geFT, geState.Grid.MinorX);
	     }
	   if (geState.Grid.DivY)
	     {geFT = geState.Grid.FMajorY / (float)geState.Grid.DivY;
	      GEINT(geFT, geState.Grid.MinorY);
	     }
#ifdef GERAGS
	   if (geUI.WidgetArray[GEWGRID])
	     geMnGrid(geUI.WidgetArray[GERESET], &ttag, NULL);
#endif
	  }

	c = geFgetc(geFdI);                     /* Read past new line        */

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;
	   c = geFgetc(geFdI);                  /* Get parm seperator - " "  */
	   if (geFeof(geFdI) || (char)c != ' ') /* Something is wrong - might*/
	     break;                             /*as well just stop gracefuly*/

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */

	   if (SSegP->Handle)
	     {GEVEC(cmd, SSegP);}

	   c = geFgetc(geFdI);                 /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                 /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;

    case 5:
     /*
      * Diff between 5 and prev - 5 has a newline following the GRF description
      */
     if (GSegP = ASegP)
       {geFscanf1(geFdI, " %d", &i);
	geState.PrintPgBgNu = i;
	geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	   if (geGenSIOFil(cmd, &TSegP.FillStyle, &TSegP.FillHT))  return;
	   geFscanf1(geFdI, " %d", &TSegP.FillWritingMode);
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
if (geVersionIn == 1)
  {if (!ASegP->Col.RGB.red && !ASegP->Col.RGB.green && !ASegP->Col.RGB.blue)
      ASegP->Col.RGB = geWhite;
  }
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	   geAttr.PagCol = ASegP->Col;
	  }

	geFscanf2(geFdI, " %d %d",
	       &geState.APagP->PixWidth,
	       &geState.APagP->MMWidth);

	c = geFgetc(geFdI);                     /* Read past new line        */

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;
	   c = geFgetc(geFdI);                  /* Get parm seperator - " "  */
	   if (geFeof(geFdI) || (char)c != ' ') /* Something is wrong - might*/
	     break;                             /*as well just stop gracefuly*/

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */

	   if (SSegP->Handle)
	     {GEVEC(cmd, SSegP);}

	   c = geFgetc(geFdI);                 /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                 /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;

    case 4:
     /*
      * Diff between 4 and 3 - 4 is reading-writing the flag for displaying
      * the page background.  The incoming flag is ignored by all except
      * voyer-voila code.  It is stored in "geState.PrintPgBgNu"
      */
     if (GSegP = ASegP)
       {geFscanf1(geFdI, " %d", &i);
	geState.PrintPgBgNu = i;
	geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).  The incoming crop is ignored under all
	 *circumstances.
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	   if (geGenSIOFil(cmd, &TSegP.FillStyle, &TSegP.FillHT))  return;
	   geFscanf1(geFdI, " %d", &TSegP.FillWritingMode);
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
if (geVersionIn == 1)
  {if (!ASegP->Col.RGB.red && !ASegP->Col.RGB.green && !ASegP->Col.RGB.blue)
      ASegP->Col.RGB = geWhite;
  }
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
	   if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
	   geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	   geAttr.PagCol = ASegP->Col;
#ifdef GERAGS
	   if (ASegP->FillStyle == GETRANSPARENT)
	     XClearWindow(geDispDev, geState.APagP->Surf.self);
	   geGrf(GEDISP, geSeg0);
#endif
	  }

	geFscanf2(geFdI, " %d %d",
		  &geState.APagP->PixWidth,
		  &geState.APagP->MMWidth);

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;
	   c = geFgetc(geFdI);                  /* Get parm seperator - " "  */
	   if (geFeof(geFdI) || (char)c != ' ') /* Something is wrong - might*/
	     break;                             /*as well just stop gracefuly*/

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */
	   if (SSegP->Handle)
	     {GEVEC(cmd, SSegP);}
	   
	   c = geFgetc(geFdI);                  /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                  /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;

    case 3:
    case 2:
    case 1:
     if (GSegP = ASegP)
       {geState.LiveSegs    = 0;
        GEVEC(GEINQLIVE, ASegP);
	/*
	 * If there are already segments on this page then ignore the
	 *Coords and Color of the incoming page (still, however, must
	 *read past 'em).
	 */
	if (geState.LiveSegs)
	  {if (geGenSIOCo (cmd, &TSegP.Co))   	       break;
	   if (geGenSIOCo (cmd, &TSegP.Co))            break;
	   if (geGenSIOCol(cmd, &TSegP.Col))  	       break;
	  }
	else
	  {if (geGenSIOCo (cmd, &ASegP->Co))  	       break;
	   if (geGenSIOCo (cmd, &geState.APagP->Crop)) break;
	   if (geGenSIOCol(cmd, &ASegP->Col)) 	       break;
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
if (geVersionIn == 1)
  {if (!ASegP->Col.RGB.red && !ASegP->Col.RGB.green && !ASegP->Col.RGB.blue)
      ASegP->Col.RGB = geWhite;
  }
/*
 * MAJOR KLUDGE - take this out immediately as v1.2 of rags has gone out
 */
	   if (ASegP->Col.RGB.red == GEMAXUSHORT &&
	       ASegP->Col.RGB.green == GEMAXUSHORT &&
	       ASegP->Col.RGB.blue == GEMAXUSHORT)
	     ASegP->FillStyle = GETRANSPARENT;
	   else
	     {ASegP->FillStyle = FillOpaqueStippled;
	      ASegP->FillHT    = 100;
	      ASegP->FillWritingMode = GXcopy;
	     }
	   geAttr.PagCol = ASegP->Col;
	  }

	geFscanf2(geFdI, " %d %d",
		  &geState.APagP->PixWidth,
		  &geState.APagP->MMWidth);

	while (!geFeof(geFdI))
	  {c = geFgetc(geFdI);                  /* Get seg STRT char  - "{"  */
	   if (geFeof(geFdI)) break;
	   if ((char)c != '{')                  /* Either there is a problem */
	     {geUngetc((char)c, geFdI); break;} /*or this is done            */
	   
	   if (!geFgets(p, 4, geFdI))           /* Get seg descriptor - "SSS"*/
		break;
	   c = geFgetc(geFdI);                  /* Get parm seperator - " "  */
	   if (geFeof(geFdI) || (char)c != ' ') /* Something is wrong - might*/
	     break;                             /*as well just stop gracefuly*/

	   geSegCr();                           /* Create a new segment slot */
	   SSegP = geState.ASegP;
	   SSegP->Handle[0] = p[0];
	   SSegP->Handle[1] = p[1];
	   SSegP->Handle[2] = p[2];
	   SSegP->Handle[3] = '\0';		/* TERMINATOR		     */
	   geGenHandle(SSegP);                  /* Get Obj handler ptr       */
	   if (SSegP->Handle) GEVEC(cmd, SSegP);
	   
	   c = geFgetc(geFdI);                  /* Get seg END  char  - "}"  */
	   if (geFeof(geFdI)) break;
	   c = geFgetc(geFdI);                  /* Get CR		     */

	   if (geFeof(geFdI)) break;
	  }
       }
     break;

    default:
     break;
    }
/*
 * These did not exist prior to version 7 - the equivalent is that they
 * were OFF.  Though these are not used at this point, set them appropriately
 * to avoid possible problems in the future.
 */
if (geVersionIn < 7)
  {ASegP->WhiteOut = FALSE;
   ASegP->ConLine  = FALSE;
  }

#ifdef GEDPI75
return;
#else

/*
 * Scaling must be turned off for MOPS - it doesn't care what the screen
 * resolution is.
 */
if (geRun.MopsCalling) return;

/*
 * If resolution of incoming file does not match the current
 *screen resolution then scale the incoming drawing by
 *		CurrentRes
 *		----------
 *		OldRes
 */
PixWidth = XDisplayWidth  (geDispDev, geScreen);
MMWidth  = XDisplayWidthMM(geDispDev, geScreen);

/*
 * Kludge - 6/29/89 - have to fudge old server 19" screen width of 325
 * to 346.
 */
if (geVersionIn <= 4 && geState.APagP->MMWidth == 325)
  geState.APagP->MMWidth = 346;

if ((geState.APagP->PixWidth && geState.APagP->MMWidth) &&
    (PixWidth != geState.APagP->PixWidth ||
     MMWidth  != geState.APagP->MMWidth))
  {CurRes = (float)PixWidth / ((float)MMWidth / 25.4);

   if (fabs(CurRes - GEVSDPI) < GEMIN_DPI_DELTA) CurRes = GEVSDPI;
   OldRes = (float)geState.APagP->PixWidth /
            ((float)geState.APagP->MMWidth / 25.4);
   if (fabs(OldRes - GEVSDPI) < GEMIN_DPI_DELTA) OldRes = GEVSDPI;
/*
 * Kludge - have to put error bars on the resolution differences because
 * the server sometimes changes its opinion about the resolution of the
 * same device (7/19/88 GNE).
 */
   if (fabs(CurRes - OldRes) > GEMIN_DPI_DELTA)
     {geMagGrp   = FALSE;
      MagFXSav   = geMagFX;
      MagFYSav   = geMagFY;

      geMagFX    = (((float)PixWidth / (float)MMWidth) * 100.) /
	((float)geState.APagP->PixWidth / (float)geState.APagP->MMWidth);

      geMagFX    = CurRes * 100. / OldRes;
      geMagFY	 = geMagFX;
      geMagMode  = GENOCONSTRAINT;
      geGrf(GEMAGRESD, geSeg0);
      geGrf(GEBOUNDS,  geSeg0);
      geMagFX    = MagFXSav;
      geMagFY    = MagFYSav;
     }
  }
#endif

}


geGenSIODesc(cmd, ASegP)
long		cmd;
struct GE_SEG	*ASegP;
{
int		i, c, len;
char		*beg, *end, *p;
unsigned char	*up1, *up2;

switch(cmd)
   {case GEWRITE:
      beg = GEDESCRIP_BEG_V8;
      end = GEDESCRIP_END_V8;
      if (ASegP->Descrip && (i = strlen(ASegP->Descrip)))
	{/*
	  * Only write out description clauses if there is content.
	  */
	 strcpy (geUtilBuf, beg);
	 fprintf(geFdO, "%s\n",
		 geUtilBuf);
	 
	 if (*(ASegP->Descrip + i - 1) == '\n')
	   fprintf(geFdO, "%s",			/* There's already a new-line*/
		   ASegP->Descrip);		/* on text, don't add another*/
	 else
	   fprintf(geFdO, "%s\n",		/* Tack on a new-line        */
		   ASegP->Descrip);
      
	 strcpy (geUtilBuf, end);		/* Write out description end */
	 fprintf(geFdO, "%s\n",			/* string                    */
		 geUtilBuf);
        }
      break;

    case GEREAD:
      switch (geVersionIn)
	{case 8:
	   /*
	    * Diff between 8 and prev:
	    *         - DESCRIPTOR begin and end strings have been slightly
	    *           shortened for V8
	    */
	   beg = GEDESCRIP_BEG_V8;
	   end = GEDESCRIP_END_V8;
	   break;
	  
	 case 7:
	   beg = GEDESCRIP_BEGIN;
	   end = GEDESCRIP_END;
	   break;

	 default:
	   return;
	}

      /*
       * Need to determine if this is the descriptor beginning inidcator.
       * Check the next characters one at a time.  If any of them differ
       * from the descriptor indicator, put 'em back and bail out of here.
       */
      i   = 0;
      p   = beg;
      len = strlen(beg);
      do
	{c = geFgetc(geFdI);                  	/* Get first descriptor char */
	 geUtilBuf[i++]  = (char)c;

	 if ((char)c != *p++)                  	/* Not the descriptor beging */
	   {while (i) geUngetc(geUtilBuf[--i], geFdI);
	    return(0);
	   }

        } while(i < len);

      c = geFgetc(geFdI);                  	/* Get CR		     */

      up1 = up2 = NULL;
      for (;;)
	{geFgets(geUtilBuf, GE_MAX_MSG_CHAR, geFdI);	/* Get a line        */
	 if (geFeof(geFdI)) return;			/* Something VERY bad*/
	 
	 i = strlen(geUtilBuf);				/* Length of a line  */
	 if (!i) continue;				/* Empty-Next line   */
	 
	 if (!strncmp(geUtilBuf, end, strlen(end)))
	   break;					/* Found END         */
	   
	 /*
	  * Collect up description lines, including line feeds
	  */
	 if (up1)					/* Already have one? */
	   {i += strlen(up1);				/* Yes, concatonate  */
	    up2 = (unsigned char *)geMalloc(i + 1);	/* the one just read */
	    strcpy(up2, up1);				/* with what we      */
	    strcat(up2, geUtilBuf);			/* already have      */
	    geFree(&up1, 0);				/* Release this line */
	    up1 = up2;					/* up1 ends up point-*/
	    up2 = NULL;					/* ing to it         */
	   }
	 else						/* This is the first */
	   {up1 = (unsigned char *)geMalloc(i + 1);   	/* line of the       */
	    strcpy(up1, geUtilBuf);			/* description       */
	   }
        }						/* Got all the line  */
      /*
       * Stuff it into the segment
       */
      if (ASegP->Descrip)
	geFree(&ASegP->Descrip, 0);
      
      ASegP->Descrip = up1;
      break;

    default:
	break;
   }

return(0);
}
