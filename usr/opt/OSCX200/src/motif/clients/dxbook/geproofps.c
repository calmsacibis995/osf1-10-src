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
static char SccsId[] = "@(#)geproofps.c	1.4\t7/4/89";
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
**	GEPROOFPS			Writes currently active drawing to a
**					PS file
**
**  ABSTRACT:
**
**
** This routine will generate a chekpoint on the currently active page, then
** it will call MOPS with the checkpoint file to generate an encapsulated
** PostScript rendition of the drawing.
**
** This routine is called ONLY from RAGS
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 04/02/90 Created
**
**--
**/

#include "geGks.h"

extern char *geMalloc();

#ifdef GERAGS

geProofPS(FileName)
char	*FileName;

{
char 	*InFile, *OutFile; 
short	ChkPtOn;
int  	offset;


/*
 * Construct Input file name - this will be checkpoint file
 *
 * PrePend "RCKP_" to the file name portion of the file spec
 */
InFile     = geMalloc(strlen(geState.APagP->Name) + strlen(geDefChkPt) + 1);
offset       = geGenParseF(geState.APagP->Name);
if (!offset)
   {strcpy(InFile, geDefChkPt);
    strcat(InFile, geState.APagP->Name);
   }
else
   {strcpy(InFile, geState.APagP->Name);
    InFile[offset] = '\0';
    strcat(InFile, geDefChkPt);
    strcat(InFile, geState.APagP->Name + offset);
   }

/*
 * Construct Output file name.
 */
geGenFileExt(FileName, geDefProofPs, FALSE);
OutFile = geMalloc(strlen(geUtilBuf) + 1);
strcpy (OutFile, geUtilBuf);    
/*
 * Force a checkpoint - going to use the checkpoint file to generate
 * the PostScript output
 */
ChkPtOn = geState.ChkPt.On;
geState.ChkPt.On = TRUE;
geState.ChkPt.Cur = geState.ChkPt.DoItAt;
geChkPt();
geState.ChkPt.On = ChkPtOn;

#ifdef vms
strcpy (geUtilBuf, "rags/convert ");
strcat (geUtilBuf, InFile);
strcat (geUtilBuf, "/output=");
#else
strcpy (geUtilBuf, "mops ");
strcat (geUtilBuf, InFile);
strcat (geUtilBuf, " -o");
#endif

strcat (geUtilBuf, OutFile);
system(geUtilBuf);

geFree(&InFile, 0);
geFree(&OutFile, 0);

return(TRUE);
}

#endif

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
**	GEPROOFPSIMG			Writes given window to PostScript file
**
**  ABSTRACT:
**
** This routine is compliments of Jeff Orthober.
**
** This routine will convert the image structure passed to
** a PostScript file written to the filename passed.
**
**  ENVIRONMENT:
**
**	VAX/VMS, ULTRIX Operating Systems
**
**  MODIFICATION HISTORY:
**
**	GNE 01/31/89 Created - compliments of Jeff Orthober
**
**--
**/

geProofPSImg(Win, image, FileName, x0, y0, width, height)
Window       Win;
XImage       *image;
char         *FileName;
int          x0, y0;
unsigned int width, height;

{
    int  picas_width,bytes,alone,format;
    int  x,y,cnt;
    char *pntr,  *error_string;
    unsigned char foo;
    FILE *f;
    XImage *TImgPtr;

    geGenFileExt(FileName, geDefProofPs, FALSE);
    if (!(f = fopen(geUtilBuf, "w", "rfm=var", "rat=cr")))
      {error_string = (char *) geFetchLiteral("GE_ERR_XOPENIMAGE", MrmRtypeChar8);
       if (error_string != NULL) 
	  {sprintf(geErrBuf, error_string, geUtilBuf);
	   geError(geErrBuf, FALSE);
	   XtFree(error_string);
	  }
       return(NULL);
      }
    
    format = XYPixmap;
    if (!image)
      {if (!(image = TImgPtr = XGetImage(geDispDev, Win, x0, y0, width,
					 height, 1, format)))
	 return(NULL);
      }
    else
      TImgPtr = NULL;

/*    image->width            = image->bytes_per_line << 3; */
    image->byte_order       = LSBFirst;
    image->bitmap_bit_order = LSBFirst;
    alone                   = TRUE;

    picas_width = (float) image->width / 80 * 6 + 1;

    fprintf(f,"%% this file generated by RAGS\n");
    if (alone)
       fprintf(f,"%% this file is meant to be printed alone\n");
    else
       fprintf(f,"%% this file is meant to be included in another ps file (document)\n");
    fprintf(f,"%% adapted from output from SIXTOPS\n");
    fprintf(f,"\n");
    fprintf(f,"mark	%% save current graphics state\n");
    fprintf(f,"/sixtops-instring 128 string def\n");
    fprintf(f,"/sixtops-inches {72 mul} def\n");
    fprintf(f,"/sixtops-picas {12 mul} def\n");
    fprintf(f,"/sixtops-points {} def\n");
    fprintf(f,"\n");
    fprintf(f,"/sixtops-new-x  6  sixtops-picas def\n");
    fprintf(f,"/sixtops-new-y  6.66667  sixtops-picas def\n");
    fprintf(f,"/sixtops-xbits %d def\n",image->width);
    fprintf(f,"/sixtops-ybits %d def\n",image->height);
    fprintf(f,"/sixtops-xy-factor 1 def\n");
    fprintf(f,"/sixtops-xy-ratio sixtops-ybits sixtops-xbits div\n");
    fprintf(f,"          sixtops-xy-factor mul def\n");
    fprintf(f,"\n");
    fprintf(f,"/sixtops-x  %d  sixtops-picas def\n",picas_width);
    fprintf(f,"/sixtops-y sixtops-x sixtops-xy-ratio mul def\n");
    fprintf(f,"%% Image scaled to a width of %d picas.\n",picas_width);
    fprintf(f,"\n");
    fprintf(f,"/sixtops-print-image	%%----------procedure-------------\n");
    fprintf(f,"               		%% This procedure does the actual imaging.\n");
    fprintf(f,"               		%%--------------------------------\n");
    fprintf(f," {\n");
    if (alone)
       fprintf(f,"   sixtops-new-x sixtops-new-y translate %% set the origin\n");
    fprintf(f,"\n");
    fprintf(f,"   0 0 moveto\n");
    fprintf(f,"   0 8 -1 mul rmoveto\n");
    fprintf(f,"   /Helvetica findfont   6  scalefont setfont\n");
    fprintf(f,"   () dup stringwidth pop\n");
    fprintf(f,"   334 exch sub 0 rmoveto\n");
    fprintf(f,"   show\n");
    fprintf(f,"\n");
    fprintf(f,"   sixtops-x sixtops-y scale  		  %% scale for the sixel image\n");
    fprintf(f,"\n");
    fprintf(f,"     sixtops-xbits sixtops-ybits 1\n");
    fprintf(f,"     [ sixtops-xbits 0 0 0 sixtops-ybits sub 0 sixtops-ybits ]\n");
    fprintf(f,"     { currentfile sixtops-instring readhexstring pop }\n");
    fprintf(f,"     image\n");
    fprintf(f,"\n");
    if (alone)
       fprintf(f,"   showpage\n");
    fprintf(f," } def\n");
    fprintf(f,"\n");
    fprintf(f,"sixtops-print-image\n");

    bytes = image->width / 8;
    if (bytes * 8 < image->width)
       bytes = bytes + 1;

    for (y = 0; y < image->height; y++) {
        pntr = image->data + image->bytes_per_line * y;
        cnt = 0;
        for (x = 1; x <= bytes; x++) {
            foo = *pntr;
            foo =   ((foo &   1) * 128) |
                    ((foo &   2) *  32) |
                    ((foo &   4) *   8) |
                    ((foo &   8) *   2) |
                    ((foo &  16) /   2) |
                    ((foo &  32) /   8) |
                    ((foo &  64) /  32) |
                    ((foo & 128) / 128);

            fprintf(f,"%02X",foo);
            pntr++; cnt++;
            if (cnt > 50) {
               fprintf(f,"\n");
               cnt = 0;
            }
        }
        fprintf(f,"\n");

        /* these are needed to make document happy */

    }

    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    fprintf(f,"%%000000000000000000000000000000000000000000000000000000000000000000000000000000\n");

    fprintf(f,"cleartomark\n");
    fclose(f);
/*
 * Release image resources
 */
if (TImgPtr)
  geXDestImg(TImgPtr);

    return(TRUE);
}
