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
static char SccsId[] = "@(#)geimgcrfse.c	1.2\t1/9/90";
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
**	GEIMGCRFSE			Reads in Field Services type of bitmap
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
**	GNE 02/08/88 Created     
**
**--
**/

#include "geGks.h"
#include <X11/Xutil.h>


extern char   *geMalloc();
extern XImage *geXCrImg();

XImage *
geImgCrFse(FileName)
char  *FileName;
{                                                                         
#define MAXACCEPTABLE 1000000

union l_word				/* allows the reading of a longword */
  {					/* in 4 one byte chunks, and access */
  unsigned char  lw_char[4];		/* as an acutal longword            */
  unsigned long  lw_num;
  };

struct flags_bits			/* flag bits from the CSE header */
  {
  unsigned b0:1;
  unsigned b1:1;
  unsigned b2:1;
  unsigned b3:1;
  unsigned b4:1;
  unsigned b5:1;
  unsigned b6:1;
  unsigned b7:1;
  };

union flags_byte
  {
  unsigned char  chr;
  struct flags_bits  bits;
  };

struct CSE_image			/* the CSE image header */
  {
  char  tag[3];
  char  data_fmt;
  char  orient;
  union l_word  num_pix;
  union l_word  num_lines;
  union l_word  scan_density;
  char  scale_cnt;
  union flags_byte  flags;
  unsigned int  num_bytes;
  unsigned char  *bitmap;
  };

  FILE  *infile;
  struct CSE_image  cseimg;
  char              *Buf;
  short             scanflag, ErrReportSav;
  int               bpl, cropadjust;
  unsigned long     n_bytes_read;
  XImage *          ImgPtr;
  char 	    	    *error_string;

/*
 * This routine currently can only work using an external file as input, it
 * cannot operate on an in-memory buffer.  As such it will kick out if it is
 * either not provided with a filename or it is provided a bogus filename.
 */

if (!FileName || !strlen(FileName)) return(NULL);

geGenFileExt(FileName, geDefProofFSE, FALSE);
if (!(infile = fopen(geUtilBuf, "r")))
  {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
    if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geUtilBuf);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
   return(NULL);
  }
fscanf(infile,"%3c",cseimg.tag);

if (strncmp(cseimg.tag, "CSE", 3))
  {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
    if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geUtilBuf);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
    fclose(infile);
    return(NULL);
  }

fscanf(infile,"%c",&cseimg.data_fmt);
fscanf(infile,"%c",&cseimg.orient);
fscanf(infile,"%4c",cseimg.num_pix.lw_char);
fscanf(infile,"%4c",cseimg.num_lines.lw_char);
fscanf(infile,"%4c",cseimg.scan_density.lw_char);
fscanf(infile,"%c",&cseimg.scale_cnt);
fscanf(infile,"%c",&cseimg.flags.chr);
cropadjust = 0;
fscanf(infile,"%c",&cropadjust);

bpl           = cseimg.num_pix.lw_num >> 3;
cseimg.num_bytes = bpl * cseimg.num_lines.lw_num;
if (cseimg.num_bytes <= 0 || cseimg.num_bytes > MAXACCEPTABLE)
  {fclose(infile);
   return(NULL);
  }
   
Buf = geMalloc(cseimg.num_bytes);
if (Buf == NULL)
  {fclose(infile);
   return(NULL);
  }
/*
 * Read past header pad
 */
if (bpl > 20)
    n_bytes_read = fread(Buf, 1, bpl - 20, infile);
else
if (bpl < 20)
   {/*
     * This is possibly a corrupted FSE file (bytes per line < 20, maybe
     * came from old UTOX which was not padding file to 20 bytes per line)
     * - try to do the best we can.
     *
     * It's got to be at least 11 bytes wide, otherwise the "num_lines"
     * parm is corrupted, in which case EXIT with ERROR.  If it's at least
     * that wide, generate a warning position at first byte of image data -
     * this will be byte #20 - hopefully.
     */
    if (bpl < 11)
      {error_string = (char *) geFetchLiteral("GE_ERR_BADIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
         {sprintf(geErrBuf, error_string, geUtilBuf);
          geError(geErrBuf, FALSE);
          XtFree(error_string);
         }
       return(NULL);
      }
    /*
     * Generate a warning
     */
    ErrReportSav    = geRun.ErrReport;
    geRun.ErrReport = TRUE;
    error_string = (char *) geFetchLiteral("GE_ERR_CORRUPTING", MrmRtypeChar8);
    if (error_string != NULL) 
      {sprintf(geErrBuf, error_string, geUtilBuf);
       geError(geErrBuf, FALSE);
       XtFree(error_string);
      }
    geRun.ErrReport = ErrReportSav;

    rewind (infile);
    n_bytes_read = fread(Buf, 1, 19, infile);
   }
/*
 * Read data
 */
n_bytes_read = fread(Buf, 1, cseimg.num_bytes, infile);

ImgPtr = geXCrImg(geDispDev, XDefaultVisual(geDispDev, geScreen),
		  1, XYPixmap, 0, Buf, cseimg.num_pix.lw_num,
		  cseimg.num_lines.lw_num, 16, 0);

if (cseimg.flags.bits.b0 && cropadjust)
  ImgPtr->width -= cropadjust;
/*
 * Kludge - 10/25/88 - Can only handle mono images for now.
 */
ImgPtr->format = XYBitmap;
fclose(infile);
return(ImgPtr);
}
