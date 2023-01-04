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
static char SccsId[] = "@(#)gepsio.c	1.7\t11/5/91";
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1991 BY                            *
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
**	GEPSIO	             	       PS object IO handler
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
**	MAS 11/05/91 Created
**
**--
**/

#include "geGks.h"
#include <stdio.h>


extern char *geMalloc ();

gePSIO (cmd, ASegP)
    long    cmd;
    struct GE_SEG  *ASegP;
{
#define		GE_LOC_SWAPPED_NONE	0
#define		GE_LOC_SWAPPED_COLS	1
#define		GE_LOC_SWAPPED_FILL	2

    char   *ImgData,
           *ptr,
           *error_string,
	    rec[256];
    short   ErrReportSav,
            swapped;
    int     nchar,
            i,
            Width,
            Height,
            Format,
            NumBytes,
            c,
            bpl,
            temp,
            temp1;
    long    Flags;
    struct GE_PS  *priv,
                   *privnu;
    struct GE_COL_OBJ   ColTemp;

    priv = (struct GE_PS   *) ASegP->Private;



    if (cmd == GEWRITE) {
	if (geVersionOut == GECURRENTVERSION) {
	/* 
	 * Writing out LATEST version
	 */
	
	    if (geGenSIOTM (cmd, &(priv->tmatrix)))			return;
	    fprintf (geFdO, " %d %d", ASegP->Z, ASegP->Visible);
	    if (geGenSIOCol (cmd, &ASegP->Col))			return;
	    if (geGenSIOFil (cmd, &ASegP->FillStyle, &ASegP->FillHT))   return;
	    fprintf (geFdO, " %d",
		     ASegP->FillWritingMode);
	    if (geGenSIOFlags (cmd, ASegP))			        return;
	    if (geGenSIODesc(cmd, ASegP))			        return;
	    if (geGenSIOAnim(cmd, ASegP))				return;
	    fprintf (geFdO, "%d %d", priv->w_pix, priv->h_pix);
	    fprintf (geFdO, " %f %f %f %f %f %f %f %f",
		    priv->llx_pts, priv->lly_pts, priv->lrx_pts, priv->lry_pts,
		    priv->urx_pts, priv->ury_pts, priv->ulx_pts, priv->uly_pts);
	    if (geGenSIOPt (cmd, &priv->Pt))			return;
	    if (geGenSIOBx (cmd, &priv->Bx))			return;
	    fprintf (geFdO, " %d %d", priv->PenHT, priv->PenStyle);
	    fprintf (geFdO, " %d", priv->Flags);
	    fprintf (geFdO, " %s", priv->Filename);
	    fprintf (geFdO, " %d", priv->PsBufLen);

	    fprintf (geFdO, "\n%s\n", GEPSDATA_BEGIN);
	    fwrite (priv->PsBuf, 1, priv->PsBufLen, geFdO);
	    fprintf (geFdO, "\n%s\n", GEPSDATA_END);

	} else {		/* older than version 7 */

	}			/* END older than version 7 */
	return;
    }


/*
 * Must be reading in - the first case = latest version
 */
    switch (geVersionIn) {
	case 8: 
	case 7: 
	    ASegP->Private = geMalloc (sizeof (struct GE_PS));
	    priv = (struct GE_PS  *) ASegP->Private;

	    /* First, the common SEG fields */
	    if (geGenSIOTM (cmd, &(priv->tmatrix)))		break;
	    geFscanf2 (geFdI, " %d %d", &(ASegP->Z), &i);
	    ASegP->Visible = i;
	    if (geGenSIOCol (cmd, &(ASegP->Col)))		break;
	    if (geGenSIOFil (cmd, &(ASegP->FillStyle), &(ASegP->FillHT)))
								break;
	    geFscanf1 (geFdI, " %d",
		       &ASegP->FillWritingMode);
	    if (geGenSIOFlags (cmd, ASegP))			break;
	    if (geGenSIODesc(cmd, ASegP))			break;
	    if (geGenSIOAnim(cmd, ASegP))			break;
	    geFscanf2 (geFdI, "%d %d", &(priv->w_pix), &(priv->h_pix));
	    geFscanff8 (geFdI, " %f %f %f %f %f %f %f %f",
		&(priv->llx_pts), &(priv->lly_pts),
		&(priv->lrx_pts), &(priv->lry_pts),
		&(priv->urx_pts), &(priv->ury_pts),
		&(priv->ulx_pts), &(priv->uly_pts));

	    priv->rt_pts  = max(max(max(priv->llx_pts, priv->lrx_pts),
				    priv->urx_pts),
				priv->ulx_pts);
	    priv->lft_pts = min(min(min(priv->llx_pts, priv->lrx_pts),
				    priv->urx_pts),
				priv->ulx_pts);
	    priv->top_pts = max(max(max(priv->lly_pts, priv->lry_pts),
				    priv->ury_pts),
				priv->uly_pts);
	    priv->bot_pts = min(min(min(priv->lly_pts, priv->lry_pts),
				    priv->ury_pts),
				priv->uly_pts);

	    if (geGenSIOPt (cmd, &priv->Pt))			break;
	    if (geGenSIOBx (cmd, &priv->Bx))			break;
	    GEVEC (GEBOUNDS, ASegP);
	    geFscanf2 (geFdI, " %d %d", &(priv->PenHT), &(priv->PenStyle));
	    geFscanf1 (geFdI, " %d", &(priv->Flags));
	    geFscanf  (geFdI, " %s", geUtilBuf);
	    priv->Filename = (unsigned char *) geMalloc(strlen(geUtilBuf) + 1);
	    strcpy(priv->Filename, geUtilBuf);
	    geFscanf1 (geFdI, " %d", &(priv->PsBufLen));
	    c = geFgetc (geFdI);		/* Read past new line */

	    geFscanf  (geFdI, "%s", rec);
	    c = geFgetc (geFdI);		/* Read past new line */
	    if (strcmp(rec, GEPSDATA_BEGIN) != 0) {
		/* Error condition - not yet implemented */
	    }

	    /* 
	     * Read the PS data
	     */
	    priv->PsBuf = (unsigned char *) geMalloc((priv->PsBufLen) + 1);
	    if (geMemIO.InMem) {
		memcpy (priv->PsBuf, geMemIO.PtrC, priv->PsBufLen);
		geMemIO.PtrC += priv->PsBufLen;
	    } else {
		nchar = fread (priv->PsBuf, 1, priv->PsBufLen, geFdI);
	    }

	    c = geFgetc (geFdI);		/* Read past new line */
	    geFscanf  (geFdI, "%s", rec);
	    c = geFgetc (geFdI);		/* Read past new line */
	    if (strcmp(rec, GEPSDATA_END) != 0) {
	      /* Error condition - not yet implemented */
	    }

	    break;
    }
}
