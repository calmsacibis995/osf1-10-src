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
static char SccsId[] = "@(#)geimgio.c	1.7\t5/9/89";
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
**	GEIMGIO	             	       Image object IO handler
**
**  ABSTRACT:
**
**      Pt.x,y
**      X-------------------------------
**      |         |                    |
**      |         |                    |<--- Image
**      |       Bx.y1                  |
**      |         |                    |
**      |         V                    |
**      |        Bx.x1,y1       Bx.x2,y2
**      |--Bx.x1->X--------------X     |
**      |         |              |     |
**      |         |              |<--- Crop box
**      |         |              |     |
**      |         |              |     |
**      |         |              |     |
**      |         |              |     |
**      |         |              |     |
**      |         |              |     |
**      |         X--------------X     |
**      |        Bx.x4,y4       Bx.x3,y3
**      |                              |
**      |                              |
**      --------------------------------
**
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 03/07/88 Created
**
**--
**/

#include "geGks.h"
#include <stdio.h>


extern char   *geMalloc();
extern XImage *geImgCr(), *geXCrImg();

geImgIO(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
#define		GE_LOC_SWAPPED_NONE	0
#define		GE_LOC_SWAPPED_COLS	1
#define		GE_LOC_SWAPPED_FILL	2

char           *ImgData, *ptr, *error_string;
short		ErrReportSav, swapped;
int             nchar, i, Width, Height, Format, NumBytes, c, bpl, temp, temp1;
long            Flags;
unsigned int    depth;
struct GE_IMG   *priv, *privnu;
struct GE_COL_OBJ   ColTemp;

priv = (struct GE_IMG *)ASegP->Private;

if (cmd == GEWRITE)
  {if (geVersionOut == GECURRENTVERSION)
     {/*
       * Writing out LATEST version
       */
      if (geGenSIOTM (cmd, &geCTM))  		                return;
      fprintf(geFdO, " %d %d",
	      ASegP->Z,
	      ASegP->Visible);
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      if (geGenSIOPt (cmd, &priv->Pt))                return;
      if (geGenSIOBx (cmd, &priv->Bx))                return;
      fprintf(geFdO, " %d %d",
	      priv->Type,
	      priv->WritingMode);
      if (geGenSIOCol(cmd, &priv->ColForeGround))     return;
      if (geGenSIOCol(cmd, &priv->ColBackGround))     return;
      /*
       * Determine the image storage mechanism - try to do it the way the user
       *desires, but if the requested storage format is REFERENCE and the image
       *has no associated filename with it, then override the requested storage
       *format and imbed the image data within the file.
       */
      priv->Flags |= geState.ImgStorage;
      if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF && !priv->FileName)
	{priv->Flags = (priv->Flags ^ GEIMGSTORAGE) | GEIMGSTORAGE_DATA;
	 
#ifdef GERAGS

	 XBell(geDispDev, GEBEEP_ERR);
	 geMnStat(GEDISP, 89);

#endif

        }

      fprintf(geFdO, " %d %d %d\n",
	      priv->Mirrored,
	      priv->RotAngle,
	      priv->Flags);
      if (geGenSIODesc(cmd, ASegP))			        return;
      if (geGenSIOAnim(cmd, ASegP))				return;

      if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
	fprintf(geFdO, "%d %s",
		strlen(priv->FileName),
		priv->FileName);
      else
	{fprintf(geFdO, "%d %d %d",
		 (priv->ImgPtr->bytes_per_line << 3),
		 priv->ImgPtr->height,
		 priv->ImgPtr->format);
	 /*
	  * Write the image data
	  */
	 ptr = priv->ImgPtr->data;
	 for (i = 1; i <= priv->ImgPtr->height; i++)
	   {fputc  ('\n' ,geFdO);
	    fwrite(ptr, priv->ImgPtr->bytes_per_line, 1, geFdO);
	    ptr += priv->ImgPtr->bytes_per_line;
	   }
        }
     }  
  else                                         /* VERSION 6                  */
  if (geVersionOut == 6)
    {fprintf(geFdO, " %d %d",
	     ASegP->Z,
	     ASegP->Visible);
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
     fprintf(geFdO, " %d", ASegP->FillWritingMode);
     if (geGenSIOPt (cmd, &priv->Pt))                return;
     if (geGenSIOBx (cmd, &priv->Bx))                return;
     fprintf(geFdO, " %d %d",
	     priv->Type,
	     priv->WritingMode);
     if (geGenSIOCol(cmd, &priv->ColForeGround))     return;
     if (geGenSIOCol(cmd, &priv->ColBackGround))     return;
     /*
      * Determine the image storage mechanism - try to do it the way the user
      *desires, but if the requested storage format is REFERENCE and the image
      *has no associated filename with it, then override the requested storage
      *format and imbed the image data within the file.
      */
     priv->Flags |= geState.ImgStorage;
     if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF && !priv->FileName)
       {priv->Flags = (priv->Flags ^ GEIMGSTORAGE) | GEIMGSTORAGE_DATA;
	
#ifdef GERAGS

	XBell(geDispDev, GEBEEP_ERR);
	geMnStat(GEDISP, 89);

#endif

       }

     fprintf(geFdO, " %d %d %d",
	     priv->Mirrored,
	     priv->RotAngle,
	     priv->Flags);
     if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
       fprintf(geFdO, " %d %s",
	       strlen(priv->FileName),
		priv->FileName);
     else
       {fprintf(geFdO, " %d %d %d",
		(priv->ImgPtr->bytes_per_line << 3),
		priv->ImgPtr->height,
		priv->ImgPtr->format);
	/*
	 * Write the image data
	 */
	ptr = priv->ImgPtr->data;
	for (i = 1; i <= priv->ImgPtr->height; i++)
	  {fputc  ('\n' ,geFdO);
	   fwrite(ptr, priv->ImgPtr->bytes_per_line, 1, geFdO);
	   ptr += priv->ImgPtr->bytes_per_line;
	  }
       }
    }
  else                                         /* VERSION 5                  */
    {fprintf(geFdO, " %d %d",
	     ASegP->Z,
	     ASegP->Visible);
     if (geGenSIOPt (cmd, &priv->Pt))                return;
     if (geGenSIOBx (cmd, &priv->Bx))                return;
     fprintf(geFdO, " %d %d",
	     priv->Type,
	     priv->WritingMode);
/*
 * My apologia - 10/10/91 - GNE
 *
 * For Metafile VERSION 5 compatiblity, the color or the opacity of the
 * image may have to be adjusted to compensate for a bug fix in the image
 * display code.
 *
 * Basically, the bug was introduced in this way:  complementing
 * an image which originally had a dark Foreground and whose Background
 * was set to Transparent or Translucent and which lay on top of a white
 * page, made that image disappear.  In order to avoid this happening, in the
 * display code I SWAPPED the Foreground and Background colors along with
 * translucence whenever I came across a Translucent or Transparent image
 * whose Foreground luminance value exceeded 50% white.
 *
 * This produced the desired effect for the case of COMPLEMENT, but resulted
 * in very erratic behavior in the case of the user just setting the
 * Foreground color of an image above 50% luminance.
 *
 * For VERSION 7 we have decided to remove this peculiar behavior, but since
 * we are still reading and writing VERSION 5 metafiles, we must adjust the
 * images whose Foreground and Opacity cause them to fall into the category
 * described above.
 *
 * The case of reading these images is most easily solved.  We've added a
 * bit flag in the image's private Flags word which gets set when a version
 * 5 image is read in.  The display code checks for this flag and if it's
 * set and the image has the attributes in question, it continues to perform
 * the swapping as the old RAGS did.  This insures that V5 images display
 * in the new RAGS precisely as they did in the old RAGS.
 *
 * The case of writing these images for Version 5 is also no problem:  if the
 * V5 flag is set, the image is written without any changes.  Thus the
 * round-trip problem,
 *	(V5 RAGS) -> (V5 image) -> (V7 RAGS) -> (V5 image) -> (V5 RAGS)
 * is solved.
 *
 * The problem is what to do on output to V5 with such images that were
 * either were created in V7 RAGS or which started as V5 images but were
 * modified in V7 RAGS, which naturally will not have the V5 flag set.
 *
 * Since the OLD RAGS will perform this swapping business on the image's
 * Foreground-Background we must twist the image around on output so that it
 * will be twisted back correctly by the OLD RAGS on input.
 *
 * Here is what we've decide to do in the various cases:
 *
 *		IMAGE TYPE		       MODIFICATION
 *		-----------------------  -----------------------
 *	1 The image originated from V5		NONE - The V7 display code
 *	  RAGS (the flag GEV5IMG is ON)		       detects that the image
 *						       originated from V5 and
 *						       puts it through the same
 *						       swapping so it looks as
 *						       it did in V5.
 *
 *	2 Image is Opaque			NONE - OK
 *
 *	3 Image has DARK Foreground and		NONE - OK
 *	            DARK Background
 *
 *	4 Image has DARK Foreground and		NONE - OK
 *	            LITE Background
 *
 *     @5 Image has LITE Foreground and		Make it OPAQUE and set its
 *	            DARK Background			background to that of
 *							the Page.  This is all
 *							that can be done for it
 *
 *	6 Image has LITE Foreground and		Swap Foreground-Background
 *	            LITE Background			colors - V5 will swap
 *							them back and it will
 *							look as it should
 *
 * @ This is the only real problem case.  There is just nothing more that can
 *   be done here and so long as there is nothing beneath the image that is
 *   expected to show through the background it will look identical.
 */

     swapped = GE_LOC_SWAPPED_NONE;
     if (!(priv->Flags & GEV5IMG) && (ASegP->FillStyle != FillOpaqueStippled &&
	      priv->ImgPtr->depth == 1))
       {temp = (priv->ColForeGround.RGB.red +
		priv->ColForeGround.RGB.green +
		priv->ColForeGround.RGB.blue) / 3;

        temp1 = (priv->ColBackGround.RGB.red +
		 priv->ColBackGround.RGB.green +
		 priv->ColBackGround.RGB.blue) / 3;

	if (temp > (GEMAXUSHORT >> 1))
	  {if(temp1 > (GEMAXUSHORT >> 1))
	     {swapped = GE_LOC_SWAPPED_COLS;
	      ColTemp = priv->ColForeGround;
	      priv->ColForeGround = priv->ColBackGround;
	      priv->ColBackGround = ColTemp;
	     }
	   else
	     {swapped 	       = GE_LOC_SWAPPED_FILL;
	      Flags            = priv->Flags;
	      ColTemp 	       = priv->ColBackGround;
	      priv->Flags     &= ~GEIMGOPACITY_CLR;
	      priv->ColBackGround = geSeg0->Col;
	      priv->ColBackGround.OverPrint = ColTemp.OverPrint;
	     }
	  }  
       }

     if (geGenSIOCol(cmd, &priv->ColForeGround)) goto img_done;
     if (geGenSIOCol(cmd, &priv->ColBackGround)) goto img_done;

     /*
      * Determine the image storage mechanism - try to do it the way the user
      *desires, but if the requested storage format is REFERENCE and the image
      *has no associated filename with it, then override the requested storage
      *format and imbed the image data within the file.
      */
     priv->Flags |= geState.ImgStorage;
     if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF && !priv->FileName)
	{priv->Flags = (priv->Flags ^ GEIMGSTORAGE) | GEIMGSTORAGE_DATA;

#ifdef GERAGS

	XBell(geDispDev, GEBEEP_ERR);
	geMnStat(GEDISP, 89);

#endif

       }

     fprintf(geFdO, " %d %d %d",
	     priv->Mirrored,
	     priv->RotAngle,
	     priv->Flags);
     if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
       fprintf(geFdO, " %d %s",
	       strlen(priv->FileName),
	       priv->FileName);
     else
       {fprintf(geFdO, " %d %d %d",
		(priv->ImgPtr->bytes_per_line << 3),
		priv->ImgPtr->height,
		priv->ImgPtr->format);
	fwrite(priv->ImgPtr->data, priv->ImgPtr->bytes_per_line,
	       priv->ImgPtr->height, geFdO);
       }

img_done:

     if (swapped == GE_LOC_SWAPPED_COLS)
       {ColTemp = priv->ColForeGround;
	priv->ColForeGround = priv->ColBackGround;
	priv->ColBackGround = ColTemp;
       }
     else if (swapped == GE_LOC_SWAPPED_FILL)
       {priv->Flags         = Flags;
	priv->ColBackGround = ColTemp;
       }
    }						/* END Version 5	     */


   return;
 }

/*
 * Must be reading in - the first case = latest version
 */
switch (geVersionIn)
  {case 8:
   case 7:
     /*
      * Diff between 7 and prev - this contains the transformation matrix
      * associated with the object.
      */
     ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
     priv 		  = (struct GE_IMG *)ASegP->Private;
     if (geGenSIOTM (cmd, &geCTM))  		                return;
     geFscanf2(geFdI, " %d %d",
	       &ASegP->Z,
	       &i);
     ASegP->Visible = i;
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
     geFscanf1(geFdI, " %d",&ASegP->FillWritingMode);
     if (geGenSIOPt (cmd, &priv->Pt))                break;
     if (geGenSIOBx (cmd, &priv->Bx))                break;
     geFscanf2(geFdI, " %d %d",
	       &priv->Type,
	       &priv->WritingMode);
     if (geGenSIOCol(cmd, &priv->ColForeGround))     break;
     if (geGenSIOCol(cmd, &priv->ColBackGround))     break;
     geFscanf3(geFdI, " %d %d %d",
	       &priv->Mirrored,
	       &priv->RotAngle,
	       &priv->Flags);
     c = geFgetc(geFdI);                     	/* Read past new line        */
     if (geGenSIODesc(cmd, ASegP))		     break;
     if (geGenSIOAnim(cmd, ASegP))		     break;

     if ((priv->Flags & GEIMGOPACITY_CLR)) ASegP->FillStyle = GETRANSPARENT;
     if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
       {/*
	 * Storage method is REFERENCE - read file name from the Meta file
	 */
	 geFscanf1(geFdI, "%d",
		   &nchar);
	 priv->FileName = (unsigned char *)geMalloc(nchar + 1);
	 i = geFgetc(geFdI);
	 geFgets(priv->FileName, nchar + 1, geFdI);
	 priv->ImgPtr = geImgCr(priv->FileName, priv->Type);
	 if (priv->ImgPtr == NULL)
	   {strcpy (geUtilBuf, priv->FileName);
	    geFree(&priv->FileName, 0);
	    geSegKil(&ASegP);
	    geState.ASegP = ASegP;
	    ErrReportSav = geRun.ErrReport;
	    geRun.ErrReport = TRUE;
	    error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE",
						   MrmRtypeChar8);
	    if (error_string != NULL) 
	      {sprintf(geErrBuf, error_string, geUtilBuf);
	       geError(geErrBuf, FALSE);
	       XtFree(error_string);
	      }
	    geRun.ErrReport = ErrReportSav;
	    break;
	   }
        }
     else
       {/*
	 * Storage method is DATA - read image data from the Meta file
	 */
	 priv->FileName = NULL;
	 geFscanf3(geFdI, "%d %d %d",
		   &Width,
		   &Height,
		   &Format);
	 
	 if (Format == ZPixmap) depth = 8;
	 else                   depth = 1;
	 bpl      = (Width >> 3) * depth;
	 NumBytes = bpl * Height;
	 ImgData  = geMalloc(NumBytes);
	 ptr = ImgData;
	 /*
	  * Read the image data
	  */
	 for (i = 1; i <= Height; i++)
	   {c     = geFgetc(geFdI);             /* Read past new line        */
	    if (geMemIO.InMem)
	      {memcpy(ptr, geMemIO.PtrC, bpl);
	       geMemIO.PtrC += bpl;
	      }
	    else
	      nchar = fread(ptr, bpl, 1, geFdI);
	    ptr += bpl;
	   }

	 priv->ImgPtr = geXCrImg(geDispDev,
				 XDefaultVisual(geDispDev, geScreen),
				 depth,
				 Format, 0, ImgData, Width, Height,
				 8, 0);
	 priv->FileName = NULL;
        }
     priv->Mirrored = priv->RotAngle = 0;
     break;

   case 6:
        /*
         * Diff between 6 and previous -
         *    - 6 is reading and writing a newline before start of image
	 *        data and following each scan line
         */
        ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
	priv 		  = (struct GE_IMG *)ASegP->Private;
	geFscanf2(geFdI, " %d %d",
		  &ASegP->Z,
		  &i);
        ASegP->Visible = i;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d",&ASegP->FillWritingMode);
	if (geGenSIOPt (cmd, &priv->Pt))                break;
	if (geGenSIOBx (cmd, &priv->Bx))                break;
	geFscanf2(geFdI, " %d %d",
		  &priv->Type,
		  &priv->WritingMode);
	if (geGenSIOCol(cmd, &priv->ColForeGround))     break;
	if (geGenSIOCol(cmd, &priv->ColBackGround))     break;
	geFscanf3(geFdI, " %d %d %d",
		  &priv->Mirrored,
		  &priv->RotAngle,
		  &priv->Flags);
	if ((priv->Flags & GEIMGOPACITY_CLR)) ASegP->FillStyle = GETRANSPARENT;
	if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
	  {/*
	    * Storage method is REFERENCE - read file name from the Meta file
	    */
	   geFscanf1(geFdI, " %d",
		  &nchar);
	   priv->FileName = (unsigned char *)geMalloc(nchar + 1);
	   i = geFgetc(geFdI);
	   geFgets(priv->FileName, nchar + 1, geFdI);
	   priv->ImgPtr = geImgCr(priv->FileName, priv->Type);
	   if (priv->ImgPtr == NULL)
	     {strcpy (geUtilBuf, priv->FileName);
	      geFree(&priv->FileName, 0);
	      geSegKil(&ASegP);
	      geState.ASegP = ASegP;
	      ErrReportSav = geRun.ErrReport;
	      geRun.ErrReport = TRUE;
      	      error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
 	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, geUtilBuf);
 		 geError(geErrBuf, FALSE);
		 XtFree(error_string);
		}
	      geRun.ErrReport = ErrReportSav;
	      break;
	     }
	  }
	else
	  {/*
	    * Storage method is DATA - read image data from the Meta file
	    */
	    priv->FileName = NULL;
	    geFscanf3(geFdI, " %d %d %d",
		      &Width,
		      &Height,
		      &Format);

	    if (Format == ZPixmap) depth = 8;
	    else                   depth = 1;
	    bpl      = (Width >> 3) * depth;
	    NumBytes = bpl * Height;
	    ImgData  = geMalloc(NumBytes);
	    ptr = ImgData;
	    /*
	     * Read the image data
	     */
	    for (i = 1; i <= Height; i++)
	      {c     = geFgetc(geFdI);          /* Read past new line        */
	       if (geMemIO.InMem)
		 {memcpy(ptr, geMemIO.PtrC, bpl);
		  geMemIO.PtrC += bpl;
		 }
	       else
		 nchar = fread(ptr, bpl, 1, geFdI);
	       ptr += bpl;
	      }

	    priv->ImgPtr = geXCrImg(geDispDev,
				    XDefaultVisual(geDispDev, geScreen),
				    depth,
				    Format, 0, ImgData, Width, Height,
				    8, 0);
	    priv->FileName = NULL;
	  }
	priv->Mirrored = priv->RotAngle = 0;
	break;

   case 5:
   case 4:
   case 3:
        /*
         * Diff between 3 and 2:
         *    - 3 is reading and writing the Visible flag
	 *    - 3 is capable of reading files which contain the image data
	 *        imbeded
         */
        ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
	priv 		  = (struct GE_IMG *)ASegP->Private;
	geFscanf2(geFdI, " %d %d",
		  &ASegP->Z,
		  &i);
        ASegP->Visible = i;
	if (geGenSIOPt (cmd, &priv->Pt))                break;
	if (geGenSIOBx (cmd, &priv->Bx))                break;
	geFscanf2(geFdI, " %d %d",
		  &priv->Type,
		  &priv->WritingMode);
	if (geGenSIOCol(cmd, &priv->ColForeGround))     break;
	if (geGenSIOCol(cmd, &priv->ColBackGround))     break;
	geFscanf3(geFdI, " %d %d %d",
		  &priv->Mirrored,
		  &priv->RotAngle,
		  &priv->Flags);
	if ((priv->Flags & GEIMGSTORAGE) == GEIMGSTORAGE_REF)
	  {/*
	    * Storage method is REFERENCE - read file name from the Meta file
	    */
	   geFscanf1(geFdI, " %d",
		  &nchar);
	   priv->FileName = (unsigned char *)geMalloc(nchar + 1);
	   i = geFgetc(geFdI);
	   geFgets(priv->FileName, nchar + 1, geFdI);
	   priv->ImgPtr = geImgCr(priv->FileName, priv->Type);
	   if (priv->ImgPtr == NULL)
	     {strcpy (geUtilBuf, priv->FileName);
	      geFree(&priv->FileName, 0);
	      geSegKil(&ASegP);
	      geState.ASegP = ASegP;
	      ErrReportSav = geRun.ErrReport;
	      geRun.ErrReport = TRUE;
      	      error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
 	      if (error_string != NULL) 
	        {sprintf(geErrBuf, error_string, geUtilBuf);
 		 geError(geErrBuf, FALSE);
		 XtFree(error_string);
		}
	      geRun.ErrReport = ErrReportSav;
	      break;
	     }
	  }
	else
	  {/*
	    * Storage method is DATA - read image data from the Meta file
	    */
	    priv->FileName = NULL;
/*
 * Kludge - 9/8/88 - There is a bug in writing the image data embedded within
 *the meta file - there should be a space between "Format" and the data.  Since
 *I can't change the format now because of VOILA - for the time being I'm
 *going to make the dangerous assumption that the Format is just 1 byte.  Fix this
 *as soon as VOILA permits.  The fix is to (in WRITE above) output a space
 *following the writing of Format.
 *                  The following is what the READ "should" look like -
	    geFscanf3(geFdI, " %d %d %d",
		      &Width,
		      &Height,
		      &Format);
*/
	    geFscanf2(geFdI, " %d %d",
		      &Width,
		      &Height);
	    if (geMemIO.InMem)
	      {while (*geMemIO.PtrC == ' ') geMemIO.PtrC++;
	       Format = atoi(geMemIO.PtrC); geMemIO.PtrC++;
	      }
	    else
	      fscanf(geFdI, " %d", &Format);
/* end of Kludge */
	    if (Format == ZPixmap) depth = 8;
	    else                   depth = 1;
	    NumBytes = (Width >> 3) * Height * depth;
	    ImgData = geMalloc(NumBytes);
	    if (geMemIO.InMem)
	      {memcpy(ImgData, geMemIO.PtrC, NumBytes);
	       geMemIO.PtrC += NumBytes;
	      }
	    else
	      nchar = fread(ImgData, 1, NumBytes, geFdI);
	    priv->ImgPtr = geXCrImg(geDispDev,
				    XDefaultVisual(geDispDev, geScreen),
				    depth,
				    Format, 0, ImgData, Width, Height,
				    8, 0);
	    priv->FileName = NULL;
	  }
	priv->Mirrored = priv->RotAngle = 0;
	break;

    case 2:
    case 1:
        ASegP->Private    = geMalloc(sizeof(struct GE_IMG));
	priv 		  = (struct GE_IMG *)ASegP->Private;
	geFscanf1(geFdI, " %d", &ASegP->Z);
	if (geGenSIOPt (cmd, &priv->Pt))                break;
	if (geGenSIOBx (cmd, &priv->Bx))                break;
	geFscanf2(geFdI, " %d %d",
		  &priv->Type,
		  &priv->WritingMode);
	if (geGenSIOCol(cmd, &priv->ColForeGround))     break;
	if (geGenSIOCol(cmd, &priv->ColBackGround))     break;
	geFscanf4(geFdI, " %d %d %d %d",
		  &priv->Mirrored,
		  &priv->RotAngle,
		  &priv->Flags,
		  &nchar);
	priv->FileName = (unsigned char *)geMalloc(nchar + 1);
	i = geFgetc(geFdI);
	geFgets(priv->FileName, nchar + 1, geFdI);
	priv->ImgPtr = geImgCr(priv->FileName, priv->Type);
	if (priv->ImgPtr == NULL)
	  {strcpy (geUtilBuf, priv->FileName);
	   geFree(&priv->FileName, 0);
	   geSegKil(&ASegP);
	   geState.ASegP = ASegP;
	   ErrReportSav = geRun.ErrReport;
	   geRun.ErrReport = TRUE;
      	   error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
 	   if (error_string != NULL) 
	     {sprintf(geErrBuf, error_string, geUtilBuf);
 	      geError(geErrBuf, FALSE);
	      XtFree(error_string);
	     }
	   break;
	  }
        ASegP->Visible = TRUE;
	priv->Mirrored = priv->RotAngle = 0;
	break;

    default:
	break;
   }

if (ASegP)
   {GEVEC(GEBOUNDS, ASegP);
    /*
     * Kludge
     */
    if (priv->Flags & GEIMGOPACITY_CLR) ASegP->FillStyle = GETRANSPARENT;
    else                                ASegP->FillStyle = FillOpaqueStippled;
    /*
     * These did not exist prior to version 7 - the equivalent is that they
     * were OFF.  Though these are not used at this point, set them
     * appropriately to avoid possible problems in the future.
     */
    if (geVersionIn < 7)
      {ASegP->WhiteOut = FALSE;
       ASegP->ConLine  = FALSE;
       priv->Flags    |= GEV5IMG;
      }
   }
}
