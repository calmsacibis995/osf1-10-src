
/****************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
***************************************************************************/

/*****************************************************************************
**
**  FACILITY:
**
**      Image Display Services
**
**  ABSTRACT:
**
**      This module contains code for generating PostScript encoded images.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3, ULTRIX V3.0
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      April 1, 1990	(April Fool)
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include <time.h>

#ifndef VMS
#include <sys/types.h>
#endif

#include <img/ChfDef.h>			    /* Condition Handling Functions */

#include <img/ImgDef.h>			    /* ISL public symbols	    */
#include <img/ImgStatusCodes.h>

#ifdef IDS_NOX
#include    <ids__widget_nox.h> /* IDS public/private without X11 references*/
#else
#include    <ids__widget.h> /* IDS public/private, Dwtoolkit, Xlib defs.    */
#endif
#include <ids__macros.h>		    /* IDS private macros	    */
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Local structure definitions
*/
typedef struct _BufferIoStruct {
  char *bufptr;
  int   buflen;
  char *wrtptr;
  long  (*action)();
  long   usrprm;
} BioRec, *BioPtr;

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
	unsigned long IdsExportPs();
static  int	_IdsExportPs();

static	unsigned long VerifyFrameAttributes();
static	int	 ExportPsScan();
static	int	 ExportToPsBuffer();
static  int      ExportColorImageDefinition();
#else
PROTO(static unsigned long VerifyFrameAttributes, (unsigned long /*fid*/));
PROTO(static int ExportPsScan, (struct UDP */*udp*/, int /*scanline*/, BioPtr /*bio*/));
PROTO(static int ExportToPsBuffer, (char */*strptr*/, BioPtr /*bio*/));
PROTO(static int ExportColorImageDefinition, (int /*bpp*/, int /*width*/, BioPtr /*bio*/));
#endif


/*
**  MACRO definitions
*/
#define MAXBUFSIZ 511
/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**	Local Storage
*/
static char hex_char[] = 
    {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

static unsigned char one_bit_swap[] = {
0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

static unsigned char two_bit_swap[] = {
0x00,0x40,0x80,0xC0,0x10,0x50,0x90,0xD0,0x20,0x60,0xA0,0xE0,0x30,0x70,0xB0,0xF0,
0x04,0x44,0x84,0xC4,0x14,0x54,0x94,0xD4,0x24,0x64,0xA4,0xE4,0x34,0x74,0xB4,0xF4,
0x08,0x48,0x88,0xC8,0x18,0x58,0x98,0xD8,0x28,0x68,0xA8,0xE8,0x38,0x78,0xB8,0xF8,
0x0C,0x4C,0x8C,0xCC,0x1C,0x5C,0x9C,0xDC,0x2C,0x6C,0xAC,0xEC,0x3C,0x7C,0xBC,0xFC,
0x01,0x41,0x81,0xC1,0x11,0x51,0x91,0xD1,0x21,0x61,0xA1,0xE1,0x31,0x71,0xB1,0xF1,
0x05,0x45,0x85,0xC5,0x15,0x55,0x95,0xD5,0x25,0x65,0xA5,0xE5,0x35,0x75,0xB5,0xF5,
0x09,0x49,0x89,0xC9,0x19,0x59,0x99,0xD9,0x29,0x69,0xA9,0xE9,0x39,0x79,0xB9,0xF9,
0x0D,0x4D,0x8D,0xCD,0x1D,0x5D,0x9D,0xDD,0x2D,0x6D,0xAD,0xED,0x3D,0x7D,0xBD,0xFD,
0x02,0x42,0x82,0xC2,0x12,0x52,0x92,0xD2,0x22,0x62,0xA2,0xE2,0x32,0x72,0xB2,0xF2,
0x06,0x46,0x86,0xC6,0x16,0x56,0x96,0xD6,0x26,0x66,0xA6,0xE6,0x36,0x76,0xB6,0xF6,
0x0A,0x4A,0x8A,0xCA,0x1A,0x5A,0x9A,0xDA,0x2A,0x6A,0xAA,0xEA,0x3A,0x7A,0xBA,0xFA,
0x0E,0x4E,0x8E,0xCE,0x1E,0x5E,0x9E,0xDE,0x2E,0x6E,0xAE,0xEE,0x3E,0x7E,0xBE,0xFE,
0x03,0x43,0x83,0xC3,0x13,0x53,0x93,0xD3,0x23,0x63,0xA3,0xE3,0x33,0x73,0xB3,0xF3,
0x07,0x47,0x87,0xC7,0x17,0x57,0x97,0xD7,0x27,0x67,0xA7,0xE7,0x37,0x77,0xB7,0xF7,
0x0B,0x4B,0x8B,0xCB,0x1B,0x5B,0x9B,0xDB,0x2B,0x6B,0xAB,0xEB,0x3B,0x7B,0xBB,0xFB,
0x0F,0x4F,0x8F,0xCF,0x1F,0x5F,0x9F,0xDF,0x2F,0x6F,0xAF,0xEF,0x3F,0x7F,0xBF,0xFF
};

static unsigned char four_bit_swap[] = {
0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80,0x90,0xA0,0xB0,0xC0,0xD0,0xE0,0xF0,
0x01,0x11,0x21,0x31,0x41,0x51,0x61,0x71,0x81,0x91,0xA1,0xB1,0xC1,0xD1,0xE1,0xF1,
0x02,0x12,0x22,0x32,0x42,0x52,0x62,0x72,0x82,0x92,0xA2,0xB2,0xC2,0xD2,0xE2,0xF2,
0x03,0x13,0x23,0x33,0x43,0x53,0x63,0x73,0x83,0x93,0xA3,0xB3,0xC3,0xD3,0xE3,0xF3,
0x04,0x14,0x24,0x34,0x44,0x54,0x64,0x74,0x84,0x94,0xA4,0xB4,0xC4,0xD4,0xE4,0xF4,
0x05,0x15,0x25,0x35,0x45,0x55,0x65,0x75,0x85,0x95,0xA5,0xB5,0xC5,0xD5,0xE5,0xF5,
0x06,0x16,0x26,0x36,0x46,0x56,0x66,0x76,0x86,0x96,0xA6,0xB6,0xC6,0xD6,0xE6,0xF6,
0x07,0x17,0x27,0x37,0x47,0x57,0x67,0x77,0x87,0x97,0xA7,0xB7,0xC7,0xD7,0xE7,0xF7,
0x08,0x18,0x28,0x38,0x48,0x58,0x68,0x78,0x88,0x98,0xA8,0xB8,0xC8,0xD8,0xE8,0xF8,
0x09,0x19,0x29,0x39,0x49,0x59,0x69,0x79,0x89,0x99,0xA9,0xB9,0xC9,0xD9,0xE9,0xF9,
0x0A,0x1A,0x2A,0x3A,0x4A,0x5A,0x6A,0x7A,0x8A,0x9A,0xAA,0xBA,0xCA,0xDA,0xEA,0xFA,
0x0B,0x1B,0x2B,0x3B,0x4B,0x5B,0x6B,0x7B,0x8B,0x9B,0xAB,0xBB,0xCB,0xDB,0xEB,0xFB,
0x0C,0x1C,0x2C,0x3C,0x4C,0x5C,0x6C,0x7C,0x8C,0x9C,0xAC,0xBC,0xCC,0xDC,0xEC,0xFC,
0x0D,0x1D,0x2D,0x3D,0x4D,0x5D,0x6D,0x7D,0x8D,0x9D,0xAD,0xBD,0xCD,0xDD,0xED,0xFD,
0x0E,0x1E,0x2E,0x3E,0x4E,0x5E,0x6E,0x7E,0x8E,0x9E,0xAE,0xBE,0xCE,0xDE,0xEE,0xFE,
0x0F,0x1F,0x2F,0x3F,0x4F,0x5F,0x6F,0x7F,0x8F,0x9F,0xAF,0xBF,0xCF,0xDF,0xEF,0xFF
};


/*****************************************************************************
**  IdsExportPs
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**      fid	    ISL image frame identifier of image to be exported.
**	roi	    Optional region of interest. If value zero, the entire 
**		    image is exported.
**	bufadr	    Address of buffer to receive sixel data.
**	buflen	    Length of buffer to receive sixel data.
**	bytcnt	    Count of total bytes returned to caller.
**	flags	    Processing flags.
**	action	    Application action routine.
**	usrprm	    Application action routine parameter.
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fid	    Fid of frame from which data was exported.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
unsigned long IdsExportPs(nfid, render_ctx, roi, bufadr, buflen, bytcnt, 
							flags, action, usrprm)
unsigned long nfid;           /* ISL native frame identifier		    */
RenderContext render_ctx;   /* Render process context			    */
unsigned long roi;		    /* ISL Region of Interest identifier	    */
char	*bufadr;	    /* Sixel buffer address			    */
int	 buflen;	    /* Sixel buffer length			    */
int	*bytcnt;	    /* Total bytes exported			    */
int	 flags;		    /* Processing flags				    */
long	(*action)();	    /* Application action routine		    */
long	 usrprm;	    /* Application action routine parameter	    */
{
    struct UDP		*udplst[3];
    struct UDP		 srcudp;

    int			 bpp;
    int			 cmpcnt;
    int			 component;
    int			 bytes_exported;

    int			 spectral_mapping;
    int			 brt_polarity;

    BioPtr               bio;

    unsigned long fid;
    struct ITMLST  *attr = 0;
    unsigned long  new_cs;
    IdsRenderCallback rcb = PropRnd_(render_ctx);


#ifdef TRACE
printf( "Entering Routine IdsExportPs in module IDS_EXPORT_PS \n");
#endif

    /*
    **	First verify the frame attributes are reasonable.
    */
    VerifyFrameAttributes(nfid);
    
    /*
    **  Do the required BandByPixel space conversion
    */ 
    GetIsl_(nfid, Img_CompSpaceOrg, new_cs, 0);
    fid = nfid;
    if ( new_cs == ImgK_BandIntrlvdByPlane )
      {
	new_cs = ImgK_BandIntrlvdByPixel;
	IdsSetItmlst(&attr, Img_CompSpaceOrg,     new_cs,         0);
	fid = ImgConvertFrame(nfid,attr,0);
	GetIsl_(fid, Img_CompSpaceOrg, new_cs, 0);
	IdsFreeItmlst( &attr );
      }
    /*
    **	Get source frame attributes
    */
    ImgGet(fid,Img_Udp,&srcudp,sizeof(srcudp),0,0);
    ImgGet(fid,Img_SpectralMapping,&spectral_mapping,sizeof(spectral_mapping),
	    0,0);
    ImgGet(fid,Img_BrtPolarity,&brt_polarity,sizeof(brt_polarity),0,0);
    /*
    **	If ROI specified, apply it now...
    */
    if( roi != 0 )
	/*
	**  Apply the ROI to our copy of the UDP.
	*/
	_ImgSetRoi(&srcudp, roi);
    /*
    **	Create the first component UDP
    */
    udplst[0] = (struct UDP *)malloc(sizeof(struct UDP));
    *udplst[0] = srcudp;
/*
**   ImgGet(fid,Img_BitsPerComp,&bpp,sizeof(bpp),0,0);
**  Should really be using the bits/component from the rcb ...
*/


    if ( spectral_mapping == ImgK_MonochromeMap )
        {
        SetBitsPerComponent_( bpp, iGRA_(rcb) );
        }
    else
        {
        SetBitsPerComponent_( bpp, iRGB_(rcb)[0] );
        }
    udplst[0]->UdpW_PixelLength = (short) bpp;
    /*
    **	Create an array of component UDPs...should be 1 or 3 components.
    */
    ImgGet(fid,Img_NumberOfComp,&cmpcnt,sizeof(cmpcnt),0,0);
    for (component = 1;  component < cmpcnt;  component++)
    {
        udplst[component] = (struct UDP *)malloc(sizeof(struct UDP));

	*udplst[component] = *udplst[component-1];
/*
**	ImgGet(fid,Img_BitsPerComp,&bpp,sizeof(bpp),0,component);
**   Do'nt need to fetch bpp again, since for PostScript:
**	 iRGB_(rcb)[0] == iRGB_(rcb)[2] == iRGB_(rcb)[2]
*/
	udplst[component]->UdpW_PixelLength  = (short)bpp;
	udplst[component]->UdpL_Pos += udplst[component-1]->UdpW_PixelLength;
    }
    /*
    **  Setup Buffer I/O Structure.
    */
    bio = (BioPtr)malloc(sizeof(BioRec));
    bio->bufptr = bufadr;
    bio->buflen = buflen;
    bio->wrtptr = bufadr;
    bio->action = action;
    bio->usrprm = usrprm;
    /*
    **	Convert to PostScript
    */
    bytes_exported = 
	_IdsExportPs(udplst,cmpcnt,flags,bio,spectral_mapping,brt_polarity);
    /*
    **	Free component UDPs and buffer I/O structures.
    */
    for (component = 0;  component < cmpcnt;  component++)
        free(udplst[component]);

    free(bio);
    /*
    **	Return total bytes processed, if requested.
    */
    if (bytcnt != NULL)
	*bytcnt = bytes_exported;

    if (fid != nfid)
	{
	ImgDeleteFrame(fid);
	return (unsigned long) nfid;
	}

#ifdef TRACE
printf( "Leaving Routine IdsExportPs in module IDS_EXPORT_PS \n");
#endif
    return (unsigned long) fid;
}

/*****************************************************************************
**  VerifyFrameAttributes
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine verifies that frame attributes are suitable for processing
**	by this routine.
**
**  FORMAL PARAMETERS:
**
**      fid - frame who's attributes are to be checked
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fid - frame who's attributes were checked
**
**  SIGNAL CODES:
**
**      [TBD]
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static unsigned long VerifyFrameAttributes(fid)
unsigned long fid;
{
    int		 spectral_mapping;

#ifdef TRACE
printf( "Entering Routine VerifyFrameAttributes in module IDS_EXPORT_PS \n");
#endif


    /*
    **	Verify assumptions:
    **
    **	.5) PCM encoding
    **	 1) All components have the same spatial extent.
    **	 2) RGB or monochrome.
    **	 3) All components have the same depth.
    **	 4) Component depth is 1, 2, 4, or 8.
    */
    ImgGet(fid,Img_SpectralMapping,&spectral_mapping,sizeof(spectral_mapping),
	    0,0);
    if (spectral_mapping != ImgK_RGBMap && 
	spectral_mapping != ImgK_MonochromeMap)
	ChfStop(1,ImgX_UNSSPCTYP);
    

#ifdef TRACE
printf( "Leaving Routine VerifyFrameAttributes in module IDS_EXPORT_PS \n");
#endif

    return(fid);
}

/*****************************************************************************
**  _IdsExportPs
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**      [TBD]
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
int	           _IdsExportPs(udplst,udpcnt,flags,bio,cmpspc,brtpol)
struct UDP         *udplst[];
int	            udpcnt;
int	            flags;
BioPtr              bio;
int	            cmpspc;
int	            brtpol;
{
    int	     scncnt = udplst[0]->UdpL_Y2 - udplst[0]->UdpL_Y1 + 1;
    int	     pxlcnt = udplst[0]->UdpL_X2 - udplst[0]->UdpL_X1 + 1;
    int	     bytes_per_scan = (pxlcnt * udplst[0]->UdpW_PixelLength + 7) / 8;
    char     tmp[80];

    int	     bytcnt = 0;
    int	     scanline;
    int	     component;

    time_t	 bintim;

#ifdef TRACE
printf( "Entering Routine _IdsExportPs in module IDS_EXPORT_PS \n");
#endif

    if (!(flags & Ids_EncapsulatedPS) )
    {
	/*
	**  Standard Header for printable postscript
	*/
	/* Enable postscript identification of file */
	bytcnt += 
	  ExportToPsBuffer("%!PS-Adobe-2.0",bio);/* Conform to Adobe Inc */
	bytcnt += 
#ifdef VMS
	  ExportToPsBuffer("%%Creator: DECimage Application Services V3.1",bio);
#else
	  ExportToPsBuffer("%%Creator: DECimage Application Services V3.0",bio);
#endif
	bytcnt += 
	  ExportToPsBuffer("%%Title: IDS Hardcopy Device Dependent Postscript",bio);
	/* 
	** for CDA Postscript Viewer, Number of Showpages in the document
	** for Showpages
	** postscript command.
	*/
	bytcnt += ExportToPsBuffer("%%Pages: 1",bio);
	/*
	** No more header postscript comments
	**
	*/
	bytcnt += ExportToPsBuffer("%%EndComments",bio);
	/*
	**   Indicate processing of the first page for document for
	**   random access (viewer stuff)
	*/
	bytcnt += ExportToPsBuffer("%%Page: ? 1",bio);
	/*
	** Define a procedure called inch, multip 72 (72 points to the inch)
	**  (for convenience)
	*/
	bytcnt += ExportToPsBuffer("/inch {72 mul} def",bio);
	/*
	** Creates a 6 element array on the stack which contains the
	** identity matrix { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0}
	**
	** store it on the stack
	**    { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0}
	**
	** to translate postscript unit to device units
	**
	** dtransform takes 72 and the -72 and the matrix
	** transform the distance vector (dx = 72, dy = -72)
	**
	** (figure out the device resolution)
	**
	*/
	bytcnt += ExportToPsBuffer("72 -72 matrix defaultmatrix dtransform",bio);
	/*
	** stored the y resolution of the device calc. by 
	** the dtransform
	**
	**
	*/
	bytcnt += ExportToPsBuffer("abs /yres exch def",bio);
	/*
	** stored the x resolution of the device calc. by 
	** the dtransform
	**
	**
	*/
	bytcnt += ExportToPsBuffer("abs /xres exch def",bio);
	/*
	** Move origin (0,0) to location on page
	** 45,140 (.625 inch to right, 1.94 inch up)
	**
	** Forget that stuff, do only .25 inch rigth .25 inch up
	** 18 18
	**
	**
	*/
/*	bytcnt += ExportToPsBuffer("45 140 translate",bio); */
	bytcnt += ExportToPsBuffer("18 18 translate",bio);
	/*
	**  Width = of image in pixels, height = of image in pixels
	**
	** Take width * 72 divide by x resolution of device
	** Take height * 72 divide by y res.
	**
	** Scale it up to the real size of the image
	** 
	*/
	sprintf(tmp,"%d inch xres div %d inch yres div scale",
		pxlcnt,scncnt);
	bytcnt += ExportToPsBuffer(tmp,bio);

	bytcnt += 
	    ExportToPsBuffer("%%BeginDocument: \"Internally Generated\"",bio);

    } /* End of if for printable postscript */

    /*
    **	Standard header comments for encapsulated postscript.
    */
    bytcnt += ExportToPsBuffer("%!PS-Adobe-2.0 EPSF-1.2",bio);

    sprintf(tmp,"%%%%BoundingBox: 0 0 %d %d",pxlcnt,scncnt);
    bytcnt += ExportToPsBuffer(tmp,bio);

    bytcnt += 
#ifdef VMS
      ExportToPsBuffer("%%Creator: DECimage Application Services V3.1",bio);
#else
      ExportToPsBuffer("%%Creator: DECimage Application Services V3.0",bio);
#endif
    bytcnt += ExportToPsBuffer("%%Title: Image Content",bio);

    bintim = time(NULL);
    sprintf(tmp,"%%%%CreationDate: %s",ctime(&bintim));
    *(tmp + strlen(tmp) - 1) = '\0';		    /* Remove trailing LF */
    bytcnt += ExportToPsBuffer(tmp,bio);
    bytcnt += ExportToPsBuffer("%%EndComments",bio);
    /*
    **	Declare a local dictionary and save current context
    */
    bytcnt += ExportToPsBuffer("10 dict begin",bio);
    bytcnt += ExportToPsBuffer("/saveobj save def",bio);
    /*
    **	Declare string variables used by the image reading procedures.
    */
    if (udpcnt > 1)
    {
	sprintf(tmp,"/redstr %d string def",bytes_per_scan);
	bytcnt += ExportToPsBuffer(tmp,bio);
	sprintf(tmp,"/grnstr %d string def",bytes_per_scan);
	bytcnt += ExportToPsBuffer(tmp,bio);
	sprintf(tmp,"/blustr %d string def",bytes_per_scan);
	bytcnt += ExportToPsBuffer(tmp,bio);
    }
    else
    {
        sprintf(tmp,"/imgstr %d string def",bytes_per_scan);
	bytcnt += ExportToPsBuffer(tmp,bio);
    }
    /*
    **  Define the color image operator for bitonal printers.
    */
    if (udpcnt > 1)
    {
	bytcnt += 
	  ExportColorImageDefinition(udplst[0]->UdpW_PixelLength,pxlcnt,bio);
    }
    /*
    **	If the brightness polarity of the image is opposite of what PostScript
    **	expects, then modify the transfer function appropriately.
    */
    if (brtpol == ImgK_ZeroMaxIntensity)
    {
	bytcnt += ExportToPsBuffer("{1 exch sub} settransfer",bio);
    }
    /*
    **	Build arguments common to both image and colorimage.
    */
    sprintf(tmp,"%d %d %d",pxlcnt,scncnt,udplst[0]->UdpW_PixelLength);
    bytcnt += ExportToPsBuffer(tmp,bio);
    sprintf(tmp,"[%d 0 0 -%d 0 %d]",pxlcnt,scncnt,scncnt);
    bytcnt += ExportToPsBuffer(tmp,bio);
    /*
    **	Do colorimage vs image specific operations.
    */
    if (udpcnt > 1)
    {
	/*
	**  Set up the colorimage call.
	*/
	bytcnt += ExportToPsBuffer("{currentfile",bio);
	bytcnt += ExportToPsBuffer("	redstr readhexstring pop}",bio);
	bytcnt += ExportToPsBuffer("{currentfile",bio);
	bytcnt += ExportToPsBuffer("	grnstr readhexstring pop}",bio);
	bytcnt += ExportToPsBuffer("{currentfile",bio);
	bytcnt += ExportToPsBuffer("	blustr readhexstring pop}",bio);
        bytcnt += ExportToPsBuffer("true 3",bio);
        bytcnt += ExportToPsBuffer("colorimage",bio);
    }
    else
    {
	/*
	**  Set up the image call.
	*/
        bytcnt += ExportToPsBuffer("{currentfile",bio);
        bytcnt += ExportToPsBuffer("	imgstr readhexstring pop}",bio);
        bytcnt += ExportToPsBuffer("image",bio);
    }
    /*
    **	Format image data.
    */
    for (scanline = 0;  scanline < scncnt;  scanline++)
    {
        for (component = 0;  component < udpcnt;  component++)
        {
            bytcnt += ExportPsScan(udplst[component],scanline,bio);
        }
    }
    /*
    **	Restore context.
    */
    bytcnt += ExportToPsBuffer("saveobj restore",bio);
    bytcnt += ExportToPsBuffer("end",bio);
    /*
    **  Printable Postscript Shell Ending
    */
    if (!(flags & Ids_EncapsulatedPS) )
    {
	bytcnt += ExportToPsBuffer("%%EndDocument",bio);
	bytcnt += ExportToPsBuffer("showpage",bio);
	bytcnt += ExportToPsBuffer("%%Trailer",bio);
    } /* End of Printable Postscript Shell Ending

    /*
    **	Return number of bytes generated.
    */

#ifdef TRACE
printf( "Leaving Routine _IdsExportPs in module IDS_EXPORT_PS \n");
#endif
    return  bytcnt;
}

/*****************************************************************************
**  ExportColorImageDefinition
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine defines the PostScript colorimage operator for printers
**	that don't support color extensions.
**
**  FORMAL PARAMETERS:
**
**	bpp	    bits per pixel
**	width	    pixels per scanline
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**      [TBD]
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static int	 ExportColorImageDefinition(bpp,width,bio)
int		 bpp;
int		 width;
BioPtr           bio;
{
    int	    bytes_per_scan = (bpp * width + 7) / 8;
    int	    mask;
    int     bytcnt = 0;
    char    tmp[80];

#ifdef TRACE
printf( "Entering Routine ExportColorImageDefinition in module IDS_EXPORT_PS \n");
#endif

    /*
    **	Only define this stuff is colorimage is not defined.
    */
    bytcnt += ExportToPsBuffer("/colorimage where",bio);
    /* True proc for if, pop dict from stack */
    bytcnt += ExportToPsBuffer("{pop}",bio);
    /* False proc, defines colorimage */
    bytcnt += ExportToPsBuffer("{",bio);
    /*
    **	The following is code that converts color to grayscale. Two versions
    **	are provided: a general purpose routine that will work slowly for all
    **	supported depths, and an "optimized" routine that can deal with bytes
    **	more efficiently.
    */
    switch (bpp)
    {
        case 1:
        case 2:
        case 4:
	    mask = (1 << bpp) - 1;

	    bytcnt += ExportToPsBuffer("/cvtgry {",bio);
	    bytcnt += ExportToPsBuffer("    10 dict begin",bio);
	    bytcnt += ExportToPsBuffer("    /blu exch def",bio);
	    bytcnt += ExportToPsBuffer("    /grn exch def",bio);
	    bytcnt += ExportToPsBuffer("    /red exch def",bio);
	    bytcnt += ExportToPsBuffer("    0",bio);
	    sprintf(tmp,"    0 %d 8 {",bpp);
	    bytcnt += ExportToPsBuffer(tmp,bio);
	    bytcnt += ExportToPsBuffer("        neg /shft exch def",bio);
	    sprintf(tmp,"        red shft bitshift %d and .299 mul",mask);
	    bytcnt += ExportToPsBuffer(tmp,bio);
	    sprintf(tmp,"        grn shft bitshift %d and .587 mul",mask);
	    bytcnt += ExportToPsBuffer(tmp,bio);
	    bytcnt += ExportToPsBuffer("        add",bio);
	    sprintf(tmp,"        blu shft bitshift %d and .114 mul",mask);
	    bytcnt += ExportToPsBuffer(tmp,bio);
	    bytcnt += ExportToPsBuffer("        add",bio);
	    bytcnt += ExportToPsBuffer("        cvi",bio);
	    sprintf(tmp,"        %d and shft neg bitshift or",mask);
	    bytcnt += ExportToPsBuffer(tmp,bio);
	    bytcnt += ExportToPsBuffer("    } for",bio);
	    bytcnt += ExportToPsBuffer("    end",bio);
	    bytcnt += ExportToPsBuffer("} bind def",bio);
	    break;
        case 8:
	    bytcnt += ExportToPsBuffer("/cvtgry {",bio);
	    bytcnt += ExportToPsBuffer("    .114 mul",bio);
	    bytcnt += ExportToPsBuffer("    exch .587 mul",bio);
	    bytcnt += ExportToPsBuffer("    add",bio);
	    bytcnt += ExportToPsBuffer("    exch .299 mul",bio);
	    bytcnt += ExportToPsBuffer("    add",bio);
	    bytcnt += ExportToPsBuffer("    cvi",bio);
	    bytcnt += ExportToPsBuffer("} bind def",bio);
	    break;
        default:
            ChfSignal(1,ImgX_INVBITPXL);
    }

    bytcnt += ExportToPsBuffer("/colorimage {",bio);
    bytcnt += ExportToPsBuffer("    10 dict begin",bio);
    sprintf(tmp,"    /grystr %d string def",bytes_per_scan);
    bytcnt += ExportToPsBuffer(tmp,bio);
    bytcnt += ExportToPsBuffer("    pop",bio);	/* Pop component count */
    bytcnt += ExportToPsBuffer("    pop",bio);	/* Pop multiproc boolean */
    bytcnt += ExportToPsBuffer("    pop",bio);	/* Pop blue procedure    */
    bytcnt += ExportToPsBuffer("    pop",bio);	/* Pop green procedure   */
    bytcnt += ExportToPsBuffer("    pop",bio);	/* Pop red procedure     */
    bytcnt += ExportToPsBuffer("    {",bio);	/* Redefine input procedure */
    bytcnt += 
      ExportToPsBuffer("        currentfile redstr readhexstring pop pop",bio);
    bytcnt += 
      ExportToPsBuffer("        currentfile grnstr readhexstring pop pop",bio);
    bytcnt += 
      ExportToPsBuffer("        currentfile blustr readhexstring pop pop",bio);
    bytcnt += ExportToPsBuffer("        0 1 grystr length 1 sub {",bio);
    bytcnt += ExportToPsBuffer("            /idx exch def",bio);
    bytcnt += ExportToPsBuffer("            redstr idx get",bio);
    bytcnt += ExportToPsBuffer("            grnstr idx get",bio);
    bytcnt += ExportToPsBuffer("            blustr idx get",bio);
    bytcnt += ExportToPsBuffer("            cvtgry",bio);
    bytcnt += ExportToPsBuffer("            grystr exch idx exch put",bio);
    bytcnt += ExportToPsBuffer("        } for",bio);
    bytcnt += ExportToPsBuffer("        grystr",bio);
    bytcnt += ExportToPsBuffer("    } image",bio);
    bytcnt += ExportToPsBuffer("    end",bio);
    bytcnt += ExportToPsBuffer("} bind def",bio);
    /*
    **	Close off conditional.
    */
    bytcnt += ExportToPsBuffer("} ifelse",bio);

#ifdef TRACE
printf( "Leaving Routine ExportColorImageDefinition in module IDS_EXPORT_PS \n");
#endif
    return bytcnt;    
}

/*****************************************************************************
**  ExportPsScan
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**      [TBD]
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static int	 ExportPsScan(udp,scanline,bio)
struct UDP	*udp;
int		 scanline;
BioPtr           bio;
{
    int	     pixel;

    int	     srcpos = udp->UdpL_Pos + scanline * udp->UdpL_ScnStride;
    int	     pxlcnt = udp->UdpL_PxlPerScn;

    int	     img_bytes = (pxlcnt * udp->UdpW_PixelLength + 7) / 8;
    int	     psc_bytes = img_bytes * 2;
    int	     exp_bytes;

    int	     value;
    int	     mask = (1 << udp->UdpW_PixelLength) - 1;

    char    *srcptr;
    char    *dstptr;

    int	     byte;
    int      src_incr;

    int	     high_nibble;
    int	     low_nibble;
    /*
    **	Allocate a local buffer twice the size of what's required to hold the
    **	raw image data. This buffer will be used to hold the raw image data and
    **	hex encoded image data.
    */
    char    *imgbuf = (char *)malloc(psc_bytes + 1);
    /*
    **	Copy data into the local buffer.
    */
    if (udp->UdpW_PixelLength == udp->UdpL_PxlStride && 
                                                  udp->UdpL_ScnStride % 8 == 0)
    {
	/*
	**  If pixel stride equal to bits/pixel and scanlines are byte aligned
	**  we can copy using a string copy operation.
	*/
        memcpy(imgbuf,udp->UdpA_Base+srcpos/8,img_bytes);
    }
    else if (udp->UdpW_PixelLength == 8 && srcpos % 8 == 0 && 
                                                 udp->UdpL_PxlStride % 8 == 0)
    {
	/*
	**  If pixel stride is not equal to bits/pixel, but pixels are bytes
	**  and byte aligned, then we can do byte moves.
	*/
	dstptr = imgbuf;
	srcptr = (char *) ( (long) udp->UdpA_Base + ((srcpos + 7) / 8));

	src_incr = udp->UdpL_PxlStride / 8;
	for (pixel = 0;  pixel < pxlcnt;  pixel++)
	{
	    *dstptr++ = *srcptr;
	     srcptr += (int) src_incr;
	}
     }
    else
    {
	int dstpos = 0;

	memset(imgbuf,0,psc_bytes+1);

	for (pixel = 0;  pixel < pxlcnt;  pixel++)
	{
	    value = GetField_(udp->UdpA_Base,srcpos,mask);
	    PutField_(imgbuf,dstpos,mask,value);
	    srcpos += udp->UdpL_PxlStride;
	    dstpos += udp->UdpW_PixelLength;
	}
    }
    /*
    **	Perform any data reordering necessary.
    */
    switch (udp->UdpW_PixelLength)
    {
        case 1:
	    _IpsMovtcLong(img_bytes,imgbuf,0,one_bit_swap,img_bytes,imgbuf);
            break;
        case 2:
	    _IpsMovtcLong(img_bytes,imgbuf,0,two_bit_swap,img_bytes,imgbuf);
            break;
        case 4:
	    _IpsMovtcLong(img_bytes,imgbuf,0,four_bit_swap,img_bytes,imgbuf);
            break;
        case 8:
            break;	/* None necessary */
        default:
            ChfSignal(1,ImgX_INVBITPXL);
    }
    /*
    **	Now...convert to hex string format. The image data is contained in the
    **	beginning of the temporary buffer. Translate image data begining from 
    **	the last byte moving backwards so we can use the same buffer. Image data
    **	at the end of the buffer will be overwritten as translation proceeds.
    */
    srcptr = imgbuf + img_bytes - 1;
    dstptr = imgbuf + psc_bytes;
    *dstptr-- = '\0';
    for (byte = 0;  byte < img_bytes;  byte++)
    {
        low_nibble = *srcptr & 0xf;
	high_nibble = (*srcptr & 0xf0) >> 4;

	*dstptr-- = hex_char[low_nibble];
	*dstptr-- = hex_char[high_nibble];
	 srcptr--;
    }
    /*
    **	Now, write out the data.
    */
    exp_bytes = ExportToPsBuffer(imgbuf,bio);
    /*
    **	That's it.
    */
    free(imgbuf);
    return exp_bytes;
}

/*****************************************************************************
**  ExportToPsBuffer
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**  SIGNAL CODES:
**
**      [TBD]
**
**  SIDE EFFECTS:
**
**      none
**
*****************************************************************************/
static int   ExportToPsBuffer(strptr,bio)
char	    *strptr;
BioPtr       bio;
{
    int   strsiz = strlen(strptr);
    int   remaining = strsiz;
    char *tmp = strptr;
    int	  blksiz;
    int   bufsiz;
    int	  count;
    int   bytcnt = 0;
    int   block;

    for (block = 0; remaining / MAXBUFSIZ > 0; block++)
    {
        blksiz = MAXBUFSIZ;
        while (blksiz > 0)
        {
	    bufsiz = bio->buflen - (bio->wrtptr - bio->bufptr);
	    count = blksiz <= blksiz ? blksiz : bufsiz;

	    memcpy(bio->wrtptr,tmp,count);

	    if (count != blksiz)
	    {
	        if (bio->action != NULL)
	        {
		    (*bio->action)(bio->bufptr,bio->buflen,bio->usrprm);
	        }
	        else
	        {
		    ChfSignal(1,ImgX_BUFOVRFLW);
	        }
	        bio->wrtptr = bio->bufptr;
	    }
	    else
	        bio->wrtptr += count;

	    tmp += count;
	    remaining -= count;
	    blksiz -=count;
	    bytcnt += count;
        }
	*bio->wrtptr++ = '\n';
         bytcnt++;
    }

    while (remaining > 0)
    {
	bufsiz = bio->buflen - (bio->wrtptr - bio->bufptr);
	count = remaining <= bufsiz ? remaining : bufsiz;

	memcpy(bio->wrtptr,tmp,count);

	if (count != remaining)
	{
	    if (bio->action != NULL)
	    {
		(*bio->action)(bio->bufptr,bio->buflen,bio->usrprm);
	    }
	    else
	    {
		ChfSignal(1,ImgX_BUFOVRFLW);
	    }
	    bio->wrtptr = bio->bufptr;
	}
	else
	    bio->wrtptr += count;

	tmp += count;
	remaining -= count;
	bytcnt += count;
    }
    *bio->wrtptr++ = '\n';
    bytcnt++;

    return(bytcnt);
}
