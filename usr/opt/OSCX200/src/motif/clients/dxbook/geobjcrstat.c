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
static char SccsId[] = "@(#)geobjcrstat.c	1.12\t4/20/89";
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
**	GEOBJCRSTAT	          Identifies and reads in object from a file
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
**	GNE 02/02/88 Created
**	MAS 11/18/91 Modified.  Moved geImgCrStat from geImg.c to here and
**			renamed to geObjCrStat.
**
**--
**/

#include "geGks.h"

extern  int		geImg(), gePs();
extern char     	*geMalloc();
extern unsigned char	*gePsCr();
extern XImage   	*geImgCr();

geObjCrStat (Loc, Type)
    long    Loc, Type;
{
    char    FileType[100];
    short   ErrReportSav;
    short   Wcx,
            Wcy,
            Icx,
            Icy;
    int     llx, lly, urx, ury;
    struct GE_IMG  *privImg;
    struct GE_PS   *privEPS;
    struct GE_SEG  *ASegP;

    geSegCr ();

    ErrReportSav = geRun.ErrReport;
    if (Type == GEIMGTYPE_NONE)		/* If caller doesn't know type */
      geRun.ErrReport = FALSE;		/* Turn off error reporting  */

    FileType[0] = '\0';

    /* Try to read it in as an image */
    geState.ASegP->Private = geMalloc (sizeof (struct GE_IMG));
    privImg = (struct GE_IMG  *) geState.ASegP->Private;
    geState.ASegP->Handle[0] = 'I';
    geState.ASegP->Handle[1] = 'M';
    geState.ASegP->Handle[2] = 'G';
    geState.ASegP->Handle[3] = '\0';
    geState.ASegP->Handler = geImg;
    ASegP = geState.ASegP;
    ASegP->FillStyle       = GETRANSPARENT;
    privImg->FileName      = NULL;
    privImg->ImgPtr        = NULL;

    if (Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_FSE)
      {if ((privImg->ImgPtr = geImgCr (geInFile, GEIMGTYPE_FSE)))
	 strcpy (FileType, "BookReader");
      }

    if ((Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_SIX) && !privImg->ImgPtr)
      {if ((privImg->ImgPtr = geImgCr (geInFile, GEIMGTYPE_SIX)))
	 strcpy (FileType, "Sixel");
      }

    if ((Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_X) && !privImg->ImgPtr)
      {if ((privImg->ImgPtr = geImgCr (geInFile, GEIMGTYPE_X)))
	 strcpy (FileType, "XBitmap");
      }

    if ((Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_DDIF) && !privImg->ImgPtr)
      {if ((privImg->ImgPtr = geImgCr (geInFile, GEIMGTYPE_DDIF)))
	 strcpy (FileType, "DDIF");
      }

    if ((Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_PS) && !privImg->ImgPtr) {

	geFree (&privImg->FileName, 0);
	geFree (geState.ASegP->Private, 0);
	geState.Mouse.x = geState.Mouse.y = 0;
	geState.Event.type = GE_Bd;
	geState.Event.xbutton.x = geState.Event.xbutton.y = 0;
	geState.Event.xbutton.window = geState.APagP->Surf.self;
	geState.Event.xbutton.button == GEBUT_L;

	/* Try reading it as PS */
	
	geState.ASegP->Private = geMalloc (sizeof (struct GE_PS));
	geState.ASegP->Handle[0] = 'E';
 	geState.ASegP->Handle[1] = 'P';
	geState.ASegP->Handle[2] = 'S';
	geState.ASegP->Handle[3] = '\0';
	geState.ASegP->Handler = gePs;
	ASegP = geState.ASegP;
	ASegP->FillStyle = GETRANSPARENT;
	privEPS = (struct GE_PS  *) geState.ASegP->Private;
	if (geInFile)
	  {privEPS->Filename = (unsigned char *)geMalloc(strlen(geInFile) + 1);
	   strcpy(privEPS->Filename, geInFile);
	  }
	else
	  privEPS->Filename = NULL;

	privEPS->PsBuf = gePsCr(privEPS->Filename,
				&(privEPS->w_pix), &(privEPS->h_pix),
				&llx, &lly, &urx, &ury,
				&(privEPS->PsBufLen));
	if (privEPS->PsBuf) {
	    privEPS->llx_pts = privEPS->ulx_pts = privEPS->lft_pts = (float)llx;
	    privEPS->lly_pts = privEPS->lry_pts = privEPS->bot_pts = (float)lly;
	    privEPS->urx_pts = privEPS->lrx_pts = privEPS->rt_pts  = (float)urx;
	    privEPS->ury_pts = privEPS->uly_pts = privEPS->top_pts = (float)ury;

	    geState.ASegP->Live = TRUE;

	    switch (Loc) {
		default: 
		case GESHIFT_UL: 
		    privImg->Pt.x = geU.XR = 0;
		    privImg->Pt.y = geU.YR = 0;
		    break;
		case GESHIFT_C: 
		    Wcx = geState.APagP->Surf.width / 2;
		    Wcy = geState.APagP->Surf.height /2;
		    Icx = privEPS->w_pix / 2;
		    Icy = privEPS->h_pix / 2;
		    privEPS->Pt.x = geU.XR = GETSUI (Wcx - Icx);
		    privEPS->Pt.y = geU.YR = GETSUI (Wcy - Icy);
		    break;
	    }

	    /* x1, y1 is upper left corner.  Subsequent numbers go clockwise */
	    privEPS->Bx.x1 = 0;
	    privEPS->Bx.y1 = 0;
	    privEPS->Bx.x3 = privEPS->Bx.x1 + GETSUI (privEPS->w_pix);
	    privEPS->Bx.y3 = privEPS->Bx.y1 + GETSUI (privEPS->h_pix);
	    privEPS->Bx.x2 = privEPS->Bx.x3;
	    privEPS->Bx.y2 = privEPS->Bx.y1;
	    privEPS->Bx.x4 = privEPS->Bx.x1;
	    privEPS->Bx.y4 = privEPS->Bx.y3;

	    privEPS->PenHT = geAttr.LineHT;
	    privEPS->PenStyle = geAttr.LineStyle;

	    privEPS->tmatrix.a = privEPS->tmatrix.d = 1;
	    privEPS->tmatrix.b = privEPS->tmatrix.c = 0;
	    privEPS->tmatrix.tx = 0;
	    privEPS->tmatrix.ty = 0;

	    /* Move UL of PS image to origin */
	    GE_CONCAT_SCALAR(&(privEPS->tmatrix), 1.0, 0.0, 0.0, 1.0,
				-(privEPS->ulx_pts), -(privEPS->uly_pts));

	    privEPS->llx_pts -= privEPS->lft_pts;
	    privEPS->ulx_pts -= privEPS->lft_pts;
	    privEPS->lly_pts -= privEPS->top_pts;
	    privEPS->lry_pts -= privEPS->top_pts;
	    privEPS->urx_pts -= privEPS->lft_pts;
	    privEPS->lrx_pts -= privEPS->lft_pts;
	    privEPS->ury_pts -= privEPS->top_pts;
	    privEPS->uly_pts -= privEPS->top_pts;
	    privEPS->bot_pts -= privEPS->top_pts;
            privEPS->rt_pts  -= privEPS->lft_pts;
	    privEPS->lft_pts -= privEPS->lft_pts; /* do these two last */
	    privEPS->top_pts -= privEPS->top_pts; /* do these two last */

	    privEPS->Flags = 0;

	    GEVEC (GEBOUNDS, geState.ASegP);
	    GEVEC (GEEXTENT, geSeg0);
	    geState.APagP->Crop.x1 = geClip.x1;
	    geState.APagP->Crop.y1 = geClip.y1;
	    geState.APagP->Crop.x2 = geClip.x2;
	    geState.APagP->Crop.y2 = geClip.y2;
	    strcpy (geUtilBuf, "EPS");

	    return(0);

	   }
	
	if ((Type == GEIMGTYPE_NONE || Type == GEIMGTYPE_TEXT) &&
	    !privImg->ImgPtr) {

	    /* Try reading it as text */
	    geFree (&privEPS->Filename, 0);
	    geFree (geState.ASegP->Private, 0);
	    geState.Mouse.x = geState.Mouse.y = 0;
	    geState.Event.type = GE_Bd;
	    geState.Event.xbutton.x = geState.Event.xbutton.y = 0;
	    geState.Event.xbutton.window = geState.APagP->Surf.self;
	    geState.Event.xbutton.button == GEBUT_L;
	    geRun.ErrReport = TRUE;		/* Turn error reporting on   */
	    if (!geImportTxt (geInFile, GECREATESTAT)) {
		GEVEC (GEBOUNDS, geSeg0);
		GEVEC (GEEXTENT, geSeg0);
		geState.APagP->Crop.x1 = geClip.x1;
		geState.APagP->Crop.y1 = geClip.y1;
		geState.APagP->Crop.x2 = geClip.x2;
		geState.APagP->Crop.y2 = geClip.y2;
		strcpy (geUtilBuf, "ASCII Text");
		return (0);
	    }

	}

	geRun.ErrReport = ErrReportSav;
	return (GERRBADIMAGE);
    }

    geRun.ErrReport = ErrReportSav;

    switch (Loc) {
	default: 
	case GESHIFT_UL: 
	    privImg->Pt.x = geU.XR = 0;
	    privImg->Pt.y = geU.YR = 0;
	    break;
	case GESHIFT_C: 
	    Wcx = geState.APagP->Surf.width / 2;
	    Wcy = geState.APagP->Surf.height / 2;
	    Icx = privImg->ImgPtr->width / 2;
	    Icy = privImg->ImgPtr->height / 2;
	    privImg->Pt.x = geU.XR = GETSUI (Wcx - Icx);
	    privImg->Pt.y = geU.YR = GETSUI (Wcy - Icy);
	    break;
    }

    privImg->Bx.x1 = 0;
    privImg->Bx.y1 = 0;
    privImg->Bx.x3 = privImg->Bx.x1 + GETSUI (privImg->ImgPtr->width);
    privImg->Bx.y3 = privImg->Bx.x1 + GETSUI (privImg->ImgPtr->height);
    privImg->Bx.x2 = privImg->Bx.x3;
    privImg->Bx.y2 = privImg->Bx.y1;
    privImg->Bx.x4 = privImg->Bx.x1;
    privImg->Bx.y4 = privImg->Bx.y3;

    privImg->Type = NULL;;
    privImg->WritingMode = geAttr.WritingMode;
    privImg->ColForeGround = geAttr.ImgFgCol;
    privImg->ColBackGround = geAttr.ImgBgCol;
    privImg->Mirrored = privImg->RotAngle = 0;

    GEVEC (GEBOUNDS, geSeg0);
    GEVEC (GEEXTENT, geSeg0);
    geState.APagP->Crop.x1 = geClip.x1;
    geState.APagP->Crop.y1 = geClip.y1;
    geState.APagP->Crop.x2 = geClip.x2;
    geState.APagP->Crop.y2 = geClip.y2;

    strcpy (geUtilBuf, FileType);
    return (0);
}
