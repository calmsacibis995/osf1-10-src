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
static char SccsId[] = "@(#)getxtio.c	1.4\t10/16/89";
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
**	GETXTIO	             	       Text object IO handler
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
**	GNE 03/07/88 Created
**
**--
**/

#include "geGks.h"
#include "geFntConv.h"

extern char * gcvt();
extern char * geMalloc();
extern XFontStruct * geLoadFont();

geTxtIO(cmd, ASegP)
long     	cmd;
struct GE_SEG	*ASegP;
{                       
int			nchar, i, i1;
struct GE_TXT   	*priv;

priv = (struct GE_TXT *)ASegP->Private;

if (cmd == GEWRITE)
  {priv->DispBg = TRUE;				/* Archaeic, but required   */
   if (geVersionOut == GECURRENTVERSION)
     {/*
       * Writing out LATEST version
       */
      if (geGenSIOTM (cmd, &geCTM))  		                return;
      fprintf(geFdO, " %d %d %d",
	      ASegP->Z,
	      priv->Just,
	      priv->Kern);
      if (geGenSIOPt (cmd, &priv->Pt))                          return;
      if (geGenSIOCol(cmd, &ASegP->Col))                        return;
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      if (geGenSIOCol(cmd, &priv->Col)) return;
      if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    return;
      fprintf(geFdO, " %d %d %d",
	      priv->WritingMode,
	      priv->DispBg,
	      priv->Font);
      geGenSIOFlags(cmd, ASegP);
      if (geGenSIODesc(cmd, ASegP))			        return;
      if (geGenSIOAnim(cmd, ASegP))				return;
      fprintf(geFdO, "%s\n",
	      priv->Str);
     }
   else                                    /* VERSION 6                      */
   if (geVersionOut == 6)
     {fprintf(geFdO, " %d %d %d %d",
	      ASegP->Z,
	      ASegP->Visible,
	      priv->Just,
	      priv->Kern);
      if (geGenSIOPt (cmd, &priv->Pt))                          return;
      if (geGenSIOCol(cmd, &ASegP->Col))                        return;
      if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
      fprintf(geFdO, " %d", ASegP->FillWritingMode);
      if (geGenSIOCol(cmd, &priv->Col)) return;
      if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    return;
      fprintf(geFdO, " %d %d %d\n%s\n",
	      priv->WritingMode,
	      priv->DispBg,
	      priv->Font,
	      priv->Str);
     }
   else                                    /* VERSION 5                      */
    {fprintf(geFdO, " %d %d",
	     ASegP->Z,
	     ASegP->Visible);
     if (geGenSIOPt (cmd, &priv->Pt))                          return;
     if (geGenSIOCol(cmd, &ASegP->Col))                        return;
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  return;
     fprintf(geFdO, " %d", ASegP->FillWritingMode);
     if (geGenSIOCol(cmd, &priv->Col)) return;
     if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    return;
     fprintf(geFdO, " %d %d %d\n%s\n",
	     priv->WritingMode,
	     priv->DispBg,
	     priv->Font,
	     priv->Str);
    }

   return;
 }

/*
 * Must be reading in - the first case = latest version
 */
switch(geVersionIn)
  {case 8:
   case 7:
     /*
      * Diff between 7 and prev - this contains the transformation matrix
      * associated with the object.
      */
     geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
     priv 		          = (struct GE_TXT *)geState.ASegP->Private;
     if (geGenSIOTM (cmd, &geCTM))  		                return;
     geFscanf3(geFdI, " %d %d %d",
	       &ASegP->Z,
	       &i1,
	       &priv->Kern);
     priv->Just     = i1;
     if (geGenSIOPt (cmd, &priv->Pt))                          break;
     if (geGenSIOCol(cmd, &ASegP->Col))                        break;
     if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
     geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
     if (geGenSIOCol(cmd, &priv->Col)) return;
     if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
     geFscanf3(geFdI, " %d %d %d",
	       &priv->WritingMode,
	       &i,
	       &priv->Font);
     priv->DispBg = i;
     geGenSIOFlags(cmd, ASegP);
     if (geGenSIODesc(cmd, ASegP))			       break;
     if (geGenSIOAnim(cmd, ASegP))			       break;
     geFgets(geUtilBuf, GE_MAX_MSG_CHAR, geFdI);     
     /*
      * Done with IO - if string is empty, trash the segment
      */
     if ((i = strlen(geUtilBuf)) < 2)
       {geSegKil(&ASegP);
	break;
      }
     geUtilBuf[i-1] = '\0';                     /* Don't want new line       */
     priv->Str = (unsigned char *)geMalloc(i);
     strcpy(priv->Str, geUtilBuf);
     /*
      * Load the font now if necessary
      */
     if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	 !geFntPtr[priv->Font])
       priv->Font = GEFONT_USER_DEFAULT;
     if (!geFontUser[priv->Font])
       geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
     break;
     
   case 6:
     /*
      * Diff between this and previous versions:
      *    - writing/reading text justification 
      */
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geFscanf4(geFdI, " %d %d %d %d",
		  &ASegP->Z,
		  &i,
		  &i1,
		  &priv->Kern);
	ASegP->Visible = i;
	priv->Just     = i1;
	if (geGenSIOPt (cmd, &priv->Pt))                          break;
	if (geGenSIOCol(cmd, &ASegP->Col))                        break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	if (geGenSIOCol(cmd, &priv->Col)) return;
	if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
	geFscanf3(geFdI, " %d %d %d",
		  &priv->WritingMode,
		  &i,
		  &priv->Font);
  	priv->DispBg = i;
	i = geFgetc(geFdI);                     /* Read past new line        */
	geFgets(geUtilBuf, GE_MAX_MSG_CHAR, geFdI);

	/*
	 * Done with IO - if string is empty, trash the segment
	 */
	if ((i = strlen(geUtilBuf)) < 2)
	    {geSegKil(&ASegP);
	     break;
	    }
	geUtilBuf[i-1] = '\0';                  /* Don't want new line       */
	priv->Str = (unsigned char *)geMalloc(i);
	strcpy(priv->Str, geUtilBuf);
	/*
	 * Load the font now if necessary
 	 */
	if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	    !geFntPtr[priv->Font])
	   priv->Font = GEFONT_USER_DEFAULT;
	if (!geFontUser[priv->Font])
	   geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
	break;

   case 5:
     /*
      * Diff between 5 and previous versions:
      *    - Font list has changed, requiring use of FntConv2 for all
      * prior versions
      */
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geFscanf2(geFdI, " %d %d",
		  &ASegP->Z,
		  &i);
	ASegP->Visible = i;
	if (geGenSIOPt (cmd, &priv->Pt))                          break;
	if (geGenSIOCol(cmd, &ASegP->Col))                        break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	if (geGenSIOCol(cmd, &priv->Col)) return;
	if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
	geFscanf3(geFdI, " %d %d %d",
		  &priv->WritingMode,
		  &i,
		  &priv->Font);
    	priv->DispBg = i;
	i = geFgetc(geFdI);                     /* Read past new line        */
	geFgets(geUtilBuf, GE_MAX_MSG_CHAR, geFdI);

	/*
	 * Done with IO - if string is empty, trash the segment
	 */
	if ((i = strlen(geUtilBuf)) < 2)
	    {geSegKil(&ASegP);
	     break;
	    }
	geUtilBuf[i-1] = '\0';                  /* Don't want new line       */
	priv->Str = (unsigned char *)geMalloc(i);
	strcpy(priv->Str, geUtilBuf);
	/*
	 * Load the font now if necessary
 	 */
	if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	    !geFntPtr[priv->Font])
	   priv->Font = GEFONT_USER_DEFAULT;
	if (!geFontUser[priv->Font])
	   geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
	break;

   case 4:
   case 3:
     /*
      * Diff between 3 and previous versions:
      *    - 3 has additional parmameter (FillHT) controlling the half-tone
      *      value of the text foreground
      *    - 3 is reading and writing the Visible flag and DispBg flag
      */
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geFscanf2(geFdI, " %d %d",
		  &ASegP->Z,
		  &i);
	ASegP->Visible = i;
	if (geGenSIOPt (cmd, &priv->Pt))                          break;
	if (geGenSIOCol(cmd, &ASegP->Col))                        break;
	if (geGenSIOFil(cmd, &ASegP->FillStyle, &ASegP->FillHT))  break;
	geFscanf1(geFdI, " %d", &ASegP->FillWritingMode);
	if (geGenSIOFil(cmd, &priv->LineStyle, &priv->LineHT))    break;
	geFscanf1(geFdI, " %d", &priv->Font);
	/*
	 * Convert Font (see geFntConv.h)
	 */
	if (priv->Font >= 0 && priv->Font < FntConv2Max)
	  priv->Font = FntConv2[priv->Font];
	else
	  {if (priv->Font >= FntConv2Max)
	     priv->Font += FntConv2Custom;
	   else
	  priv->Font = GEFONT_USER_DEFAULT;
	  }
	geFscanf1(geFdI, " %d", &nchar);
	priv->Str = (unsigned char *)geMalloc(nchar + 1);
	i = geFgetc(geFdI);
	geFgets(priv->Str, nchar + 1, geFdI);
	if (geMemIO.InMem) geMemIO.PtrC++;      /* Move past "sp"            */
	geFscanf1(geFdI, " %d", &priv->WritingMode);
	if (geGenSIOCol(cmd, &priv->Col)) return;
	geFscanf1(geFdI, " %d", &i);
	/*
	 * Done with IO - if string is empty, trash the segment
	 */
	if (!strlen(priv->Str))
	    {geSegKil(&ASegP);
	     break;
	    }
  	priv->DispBg = i;
	/*
	 * Load the font now if necessary
 	 */
	if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	    !geFntPtr[priv->Font])
	   priv->Font = GEFONT_USER_DEFAULT;
	if (!geFontUser[priv->Font])
	   geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
	break;

    case 2:
     /*
      * Diff between 2 and 1:
      *    - 2 has additional parmameter (FillStyle) controlling whether text is
      *      is written with or without background
      *    - 2 has additional color parameter which dictates the color of the
      *      background pixels with XDrawImageString.  Also, the foreground
      *      color, which used to reside in the SEG has been moved to priv and
      *      the SEG color now = the background color.
      *    - Font list has changed, requiring use of FntConv1 for version 1 files.
      */
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geFscanf1(geFdI, " %d", &ASegP->Z);
	if (geGenSIOPt (cmd, &priv->Pt))   break;
	if (geGenSIOCol(cmd, &ASegP->Col)) break;
	geFscanf2(geFdI, " %d %d", &ASegP->FillStyle, &priv->Font);
	/*
	 * Convert Font (see geFntConv.h)
	 */
	if (priv->Font >= 0 && priv->Font < FntConv2Max)
	  priv->Font = FntConv2[priv->Font];
	else
	  {if (priv->Font >= FntConv2Max)
	     priv->Font += FntConv2Custom;
	   else
	  priv->Font = GEFONT_USER_DEFAULT;
	  }
	geFscanf1(geFdI, " %d", &nchar);
	priv->Str = (unsigned char *)geMalloc(nchar + 1);
	i = geFgetc(geFdI);
	geFgets(priv->Str, nchar + 1, geFdI);
        if (geMemIO.InMem) geMemIO.PtrC++;     /* Move past "sp"            */
	geFscanf1(geFdI, " %d", &priv->WritingMode);
	if (geGenSIOCol(cmd, &priv->Col)) return;
	/*
	 * Done with IO - if string is empty, trash the segment
	 */
	if (!strlen(priv->Str))
	    {geSegKil(&ASegP);
	     break;
	    }

	if (ASegP->FillStyle != GETRANSPARENT)
	  {priv->DispBg  = TRUE;
	   ASegP->FillHT = 100;
	  }
	else
  	  {priv->DispBg  = FALSE;
	  }
        ASegP->Visible         = TRUE;
        ASegP->FillWritingMode = GXcopy;
        priv->LineStyle        = FillSolid;
        priv->LineHT           = 100;
	/*
	 * Load the font now if necessary
 	 */
	if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	    !geFntPtr[priv->Font])
	   priv->Font = GEFONT_USER_DEFAULT;
	if (!geFontUser[priv->Font])
	   geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
	break;

    case 1:
	geState.ASegP->Private    = geMalloc(sizeof(struct GE_TXT));
	priv 		          = (struct GE_TXT *)geState.ASegP->Private;
	geFscanf1(geFdI, " %d", &ASegP->Z);
	if (geGenSIOPt (cmd, &priv->Pt))   break;
	ASegP->Col.RGB            = geBlack;
	ASegP->FillStyle          = GETRANSPARENT;
	if (geGenSIOCol(cmd, &priv->Col)) break;
	geFscanf1(geFdI, " %d", &priv->Font);
	/*
	 * Convert Font (see geFntConv.h)
	 */
	if (priv->Font >= 0 && priv->Font < FntConv1Max)
	  priv->Font = FntConv2[FntConv1[priv->Font]];
	else
	  priv->Font = GEFONT_USER_DEFAULT;
	geFscanf1(geFdI, " %d", &nchar);
	priv->Str = (unsigned char *)geMalloc(nchar + 1);
	i = geFgetc(geFdI);
	geFgets(priv->Str, nchar + 1, geFdI);
        if (geMemIO.InMem) geMemIO.PtrC++;     /* Move past "sp"            */
	geFscanf1(geFdI, " %d", &priv->WritingMode);
	/*
	 * Done with IO - if string is empty, trash the segment
	 */
	if (!strlen(priv->Str))
	    {geSegKil(&ASegP);
	     break;
	    }
  	priv->DispBg 	       = FALSE;
        ASegP->Visible         = TRUE;
	ASegP->FillHT          = 100;
        ASegP->FillWritingMode = GXcopy;
        priv->LineStyle        = FillSolid;
        priv->LineHT           = 100;
	/*
	 * Load the font now if necessary
 	 */
	if (priv->Font < 0 || priv->Font >= GE_MAX_FNTS ||
	    !geFntPtr[priv->Font])
	   priv->Font = GEFONT_USER_DEFAULT;
	if (!geFontUser[priv->Font])
	   geFontUser[priv->Font] = geLoadFont(geFntXName[priv->Font]);
	break;

    default:
	break;
   }

if (ASegP)
  {GEVEC(GEBOUNDS, ASegP);}
else
   return;

/*
 * These did not exist prior to version 7 - the equivalent is that they
 * were OFF.
 */
if (geVersionIn < 7)
  {ASegP->WhiteOut = FALSE;
   ASegP->ConLine  = FALSE;
  }
if (geVersionIn < 6)
  {priv->Just = GEJUSTTXTL;
   priv->Kern = 0;
  }
if (!priv->DispBg)			/* This MUST be originating from     */
   {ASegP->FillStyle = GETRANSPARENT;	/* OLD rags so let the DispBackGround*/
    priv->DispBg = TRUE;		/* flag override the fillstyle	     */
   }
/*
 * The following is for the benefit of MOPS - it does not deal in FillSolid
 */
if (ASegP->FillStyle == FillSolid)
  {ASegP->FillStyle = FillOpaqueStippled;
   ASegP->FillHT    = 100;
 }
if (priv->LineStyle == FillSolid)
  {priv->LineStyle = FillOpaqueStippled;
   priv->LineHT    = 100;
 }

}

/*
 * GELOADFONT				Given the name of a font this routine
 *					will load it and return a pointer to
 *					the font structure.
 *
 *					If it can't do load the font for some
 *					reason, it then tries
 *						1) The "user specified" default
 *						   font
 *					and finally
 *						2) "fixed" font
 *
 *					If the "fixed" font cannot be loaded,
 *					there something "drastically!" wrong
 *					and the program aborts.
 */
XFontStruct *
geLoadFont(Name)
char *Name;
{
char *p, *p1, *error_string;

XFontStruct *font;

/*
 * This should work under normal conditions.
 */
if (Name && strlen(Name) &&
    (font = XLoadQueryFont(geDispDev, Name))) return(font);

/*
 * If the font being loaded is hard coded to be 75 dpi, try asking for the
 * 100 dpi font; that is, in the font name string replace "75-75" with "*-*"
 * and try loading that font
 */
p = Name;
for (;;)
   {if(!(p1 = (char *)strchr(p, '7'))) break;
    if (*(p1 + 1) == '5')
	{if (!strncmp(p1, "75-75", 5))
	   {/*
	     * Found the 75 dpi invocation, change it to *-*
	     */
	    strncpy(geUtilBuf, Name, (p1 - Name));
	    geUtilBuf[p1-Name] = '\0';
	    strcat (geUtilBuf, "*-*");
	    strcat (geUtilBuf, (p1 + 5));
	    if ((font = XLoadQueryFont(geDispDev, geUtilBuf))) return(font);
	    else break;
	   }
	}
    p = p1 + 1;
   }
/*
 * Try the generic "geDefaultFont" - but first put out an error message.
 */
if (Name && strlen(Name))			/* Name of font causing the  */
    strcpy(geUtilBuf, Name);			/* problem		     */
else						/* Can't even figure out name*/
    strcpy(geUtilBuf, "????");			/* of the font		     */

                                                /* Report the error	     */
error_string = (char *) geFetchLiteral("GE_ERR_BADFNTFILE", MrmRtypeChar8);
if (error_string != NULL) 
  {sprintf(geErrBuf, error_string, geUtilBuf);
   geError(geErrBuf, FALSE);
   XtFree(error_string);
  }

if (geDefaultFont)				/* If the default font is set*/
   return(geDefaultFont); 			/* return it.		     */

/*
 * Try the "user specified" default font
 */
if (geFontUser[GEFONT_USER_DEFAULT])		
   return((geDefaultFont = geFontUser[GEFONT_USER_DEFAULT]));

/*
 * User default font not yet loaded.  Try loading it now.
 */
if (geFntXName[GEFONT_USER_DEFAULT] && strlen(geFntXName[GEFONT_USER_DEFAULT]))
   {if ((geDefaultFont = geFontUser[GEFONT_USER_DEFAULT] =
	XLoadQueryFont(geDispDev, geFntXName[GEFONT_USER_DEFAULT])))
	return(geDefaultFont);
   }

/*
 * Couldn't load user default font, try loading "fixed" font.
 */
if ((geDefaultFont = XLoadQueryFont(geDispDev, GEXFIXED_NAME)))
    return(geDefaultFont);

/*
 * Couldn't load "fixed" font !!!
 * SOMETHING EXTREMELY WRONG - abort!
 */
strcpy(geUtilBuf, GEXFIXED_NAME);
error_string = (char *) geFetchLiteral("GE_ERR_FATALFNT", MrmRtypeChar8);
if (error_string != NULL) 
  {sprintf(geErrBuf, error_string, geUtilBuf); 
   geError(geErrBuf, TRUE);
   XtFree(error_string);
  }
return(NULL);				/* Should never get here - above is   */
					/* FATAL error - program should ABORT */
}
