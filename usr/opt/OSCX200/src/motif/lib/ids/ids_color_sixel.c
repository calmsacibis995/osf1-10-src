
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
**      This module contains code for generating color sixel data.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.1, ULTRIX V3.0
**
**  AUTHOR(S):
**
**      John Weber
**
**  CREATION DATE:
**
**      November 23, 1988 (Happy Thanksgiving)
**
**  MODIFICATION HISTORY:
**
*****************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
#include <string.h>

#include <img/ChfDef.h>			    /* Condition Handling Functions */

#include <img/ImgDef.h>			    /* ISL public symbols	    */
#include <img/ImgStatusCodes.h>

#include    <ids__macros.h>		    /* IDS private macros	    */
#ifndef NODAS_PROTO
#include <idsprot.h>		/* IDS prototypes */
#endif

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
	long	 IdsExportColorSixel();

static	unsigned long VerifyFrameAttributes();
static	long	 ExportSixelScanline();
static	int	 ExportRepeatSequence();
#else
PROTO(static unsigned long VerifyFrameAttributes, (unsigned long /*fid*/));
PROTO(static long ExportSixelScanline, (char */*srcstr*/, char **/*obptr*/, int */*obcnt*/));
PROTO(static int ExportRepeatSequence, (char */*outptr*/, char /*repeat_char*/, int /*repeat_count*/));
#endif

/*
**  MACRO definitions
*/

/*
**  Equated Symbols
*/

#define SIXEL_CR '$'
#define SIXEL_NL '-'

/*
**  External References
*/
#ifdef NODAS_PROTO
extern char		*_ImgCalloc();
extern void		 _ImgFree();
extern unsigned long	 _ImgGet();
extern char		*_ImgMalloc();
extern struct UDP	*_ImgSetRoi();
#endif

/*
**	Local Storage
*/

/*****************************************************************************
**  IdsExportHrColorSixels
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine exports color sixel data from an ISL image frame. The
**	input frame must have spectral component mapping RGB or CMY, and have
**	one bit per component.
**
**	Sixels represent a column of six vertical pixels of the image. Color
**	sixels will have three sixel scanlines, one for each component color
**	(RGB or CMY).
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
unsigned long IdsExportHrColorSixels(fid, roi, bufadr, buflen, bytcnt, flags, action,
				 usrprm)
unsigned long fid;		    /* ISL frame identifier			    */
unsigned long roi;		    /* ISL Region of Interest identifier	    */
char	*bufadr;	    /* Sixel buffer address			    */
int	 buflen;	    /* Sixel buffer length			    */
int	*bytcnt;	    /* Total bytes exported			    */
int	 flags;		    /* Processing flags				    */
long	(*action)();	    /* Application action routine		    */
long	 usrprm;	    /* Application action routine parameter	    */
{
    char *cbuff, *mbuff, *ybuff;
    char *cbptr, *mbptr, *ybptr;

    struct UDP  src;
    int pelcnt, scncnt;

    char *obptr = bufadr;
    int   obcnt = buflen;

    int pel,scanline;

    int offset, mask;

    union
    {
	int int_value;
	struct
	{
	    unsigned cyan    : 1;
	    unsigned magenta : 1;
	    unsigned yellow  : 1;
	    unsigned         : 29;
	}   cmy_value;
    }	cmy_pel;

    int sixel_bit_pos;

#ifdef TRACE
printf( "Entering Routine IdsExportHrColorSixels in module IDS_COLOR_SIXEL \n");
#endif

    /*
    **	First verify the frame attributes are reasonable.
    */
    VerifyFrameAttributes(fid);
    /*
    **	Fetch necessary ISL frame attributes
    */
    GetIsl_(fid, Img_Udp, src, 0);
    if( roi != 0 )
	/*
	**  Apply the ROI to our copy of the UDP.
	*/
	_ImgSetRoi(&src, roi);
    /*
    **	Allocate sixel scanline buffers, one for each component. Each scanline
    **	must be able to contain a 2 byte color specifier, all pixel data, a
    **	graphics carriage return or new line, and a NULL terminator (total of
    **	four overhead bytes).
    */
    pelcnt = src.UdpL_X2 - src.UdpL_X1 + 1;
    scncnt = src.UdpL_Y2 - src.UdpL_Y1 + 1;

    cbuff = (char *) _ImgCalloc(1,pelcnt + 4);
    mbuff = (char *) _ImgCalloc(1,pelcnt + 4);
    ybuff = (char *) _ImgCalloc(1,pelcnt + 4);
    /*
    **	Initialize sixel color information. Write the color definitions for
    **	CYAN, MAGENTA, and YELLOW into the sixel output buffer. Write the
    **	respective color introducers for each of the sixel scanline buffers.
    */
    sprintf(obptr,"#1;%s\n#2;%s\n#3;%s\n", 
			"2;25;75;75" /* CYAN   */, 
			"2;75;25;75" /* MAGENTA*/, 
			"2;75;75;25" /* YELLOW */);
    obcnt -= strlen(obptr);
    obptr += strlen(obptr);
    
    sprintf(cbuff,"#1");
    sprintf(mbuff,"#2");
    sprintf(ybuff,"#3");
    /*
    **	Now we're ready to roll...
    */
    mask = (1 << src.UdpW_PixelLength) - 1;
    for (scanline = 0;  scanline < scncnt;  scanline++)
	{
	    /*
	    **	Initialize sixel scanline buffer pointers and bit position.
	    */
	    cbptr = cbuff + 2; mbptr = mbuff + 2; ybptr = ybuff + 2;
	    sixel_bit_pos = scanline % 6;
	    /*
	    **	Get to work
	    */
	    for (pel = 0;  pel < pelcnt;  pel++)
		{
		    /*
		    **	CMY = NOT RGB
		    */
		    offset = src.UdpL_Pos + 
			     scanline * src.UdpL_ScnStride + 
			     pel * src.UdpL_PxlStride ;
		    cmy_pel.int_value = ~GetField_(src.UdpA_Base,offset,mask);

		    PutField_(cbptr,sixel_bit_pos,1,cmy_pel.cmy_value.cyan);
		    PutField_(mbptr,sixel_bit_pos,1,cmy_pel.cmy_value.magenta);
		    PutField_(ybptr,sixel_bit_pos,1,cmy_pel.cmy_value.yellow);
		    /*
		    **	Add sixel offset, update pointers.
		    */
		    cbptr++; mbptr++; ybptr++;
		}
	    /*
	    **	If an entire sixel scanline has been constructed, export it.
	    */
	    if (sixel_bit_pos == 5)
		{
		    *cbptr = SIXEL_CR;  *mbptr = SIXEL_CR;  *ybptr = SIXEL_NL;
		     cbptr = cbuff + 2;  mbptr = mbuff + 2;  ybptr = ybuff + 2;
		    for (pel = 0;  pel < pelcnt;  pel++)
			{
			    *cbptr++ += 077; *mbptr++ += 077; *ybptr++ += 077;
			}
		    ExportSixelScanline(cbuff,&obptr,&obcnt);
		    ExportSixelScanline(mbuff,&obptr,&obcnt);
		    ExportSixelScanline(ybuff,&obptr,&obcnt);
		    /*
		    **	Clear scanline following color introducer
		    */
		    memset(cbuff+2,0,pelcnt+2);
		    memset(mbuff+2,0,pelcnt+2);
		    memset(ybuff+2,0,pelcnt+2);
		}
	}
	/*
	**  If less than six full scanlines at the end, export what remains now.
	*/
	if (sixel_bit_pos != 5)
	    {
		*cbptr = SIXEL_CR;  *mbptr = SIXEL_CR;  *ybptr = SIXEL_NL;
		 cbptr = cbuff + 2;  mbptr = mbuff + 2;  ybptr = ybuff + 2;
		for (pel = 0;  pel < pelcnt;  pel++)
		    {
			*cbptr++ += 077; *mbptr++ += 077; *ybptr++ += 077;
		    }
		ExportSixelScanline(cbuff,&obptr,&obcnt);
		ExportSixelScanline(mbuff,&obptr,&obcnt);
		ExportSixelScanline(ybuff,&obptr,&obcnt);
	    }
    /*
    **	Return total bytes processed, if requested.
    */
    if (bytcnt != 0)
	*bytcnt = buflen - obcnt;
    /*
    **	Deallocate scanline buffers
    */
    _ImgFree(cbuff);
    _ImgFree(mbuff);
    _ImgFree(ybuff);

#ifdef TRACE
printf( "Leaving Routine IdsExportHrColorSixels in module IDS_COLOR_SIXEL \n");
#endif
    return(fid);
}

/*****************************************************************************
**  IdsExportLrColorSixels
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine exports color sixel data from an ISL image frame. The
**	input frame must have spectral component mapping RGB or CMY, and have
**	one bit per component.
**
**	Sixels represent a column of six vertical pixels of the image. Color
**	sixels will have three sixel scanlines, one for each component color
**	(RGB or CMY).
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
unsigned long IdsExportLrColorSixels(fid, roi, bufadr, buflen, bytcnt, flags, action,
				 usrprm)
unsigned long fid;		    /* ISL frame identifier			    */
unsigned long roi;		    /* ISL Region of Interest identifier	    */
char	*bufadr;	    /* Sixel buffer address			    */
int	 buflen;	    /* Sixel buffer length			    */
int	*bytcnt;	    /* Total bytes exported			    */
int	 flags;		    /* Processing flags				    */
long	(*action)();	    /* Application action routine		    */
long	 usrprm;	    /* Application action routine parameter	    */
{
    char *sixlbuf[7];
    char *sixlptr[7];
    int  numclrs;

    struct UDP  src;
    int pelcnt, scncnt;

    char *obptr = bufadr;
    int   obcnt = buflen;

    int pel,scanline;

    int offset, mask;

    union
    {
	int int_value;
	struct
	{
	    unsigned cyan    : 1;
	    unsigned magenta : 1;
	    unsigned yellow  : 1;
	    unsigned         : 29;
	}   cmy_value;
    }	cmy_pel;

    int sixel_bit_pos;

#ifdef TRACE
printf( "Entering Routine IdsExportLrColorSixels in module IDS_COLOR_SIXEL \n");
#endif


    /*
    **	First verify the frame attributes are reasonable.
    */
    VerifyFrameAttributes(fid);
    /*
    **	Fetch necessary ISL frame attributes
    */
    GetIsl_(fid, Img_Udp, src, 0);
    if( roi != 0 )
	/*
	**  Apply the ROI to our copy of the UDP.
	*/
	_ImgSetRoi(&src, roi);
    /*
    **	Allocate sixel scanline buffers, one for each component. Each scanline
    **	must be able to contain a 2 byte color specifier, all pixel data, a
    **	graphics carriage return or new line, and a NULL terminator (total of
    **	four overhead bytes).
    */
    pelcnt = src.UdpL_X2 - src.UdpL_X1 + 1;
    scncnt = src.UdpL_Y2 - src.UdpL_Y1 + 1;
    
    for ( numclrs = 0; numclrs < 7; numclrs++)
	{
	    sixlbuf[numclrs] = (char *) _ImgCalloc(1,pelcnt + 4);
	    sixlptr[numclrs] = sixlbuf[numclrs];
	    sprintf(sixlptr[numclrs],"#%d",numclrs+1); 
	} 
    /*
    **	Initialize sixel color information. Write the color definitions for
    **	CYAN, MAGENTA, YELLOW, RED, GREEN, BLUE and BLACK into the sixel 
    **	output buffer. Write the respective color introducers for each of the 
    **  sixel scanline buffers. Skip #0 and #7 because sixel terminals use
    **  these slots to map foreground and background colors.  This way the
    **  image colors may look the same on a sixel terminal as on a LJ250. 
    */
    sprintf(obptr,"#1;%s\n#2;%s\n#3;%s\n#4;%s\n#5;%s\n#6;%s\n#8;%s\n", 
			"2;000;000;000" /* BLACK  */, 
			"2;100;000;000" /* RED    */, 
			"2;000;100;000" /* GREEN  */, 
			"2;100;100;000" /* YELLOW */, 
			"2;000;000;100" /* BLUE   */, 
			"2;100;000;100" /* MAGENTA*/, 
			"2;000;100;100" /* CYAN   */);
    obcnt -= strlen(obptr);
    obptr += strlen(obptr);
    /*
    **	Now we're ready to roll...
    */
    mask = (1 << src.UdpW_PixelLength) - 1;
    for (scanline = 0;  scanline < scncnt;  scanline++)
	{
	    /*
	    **	Initialize sixel scanline buffer pointers and bit position.
	    */
	    for (numclrs = 0; numclrs < 7; numclrs++)
		sixlptr[numclrs] = sixlbuf[numclrs] + 2;
	    sixel_bit_pos = scanline % 6;
	    /*
	    **	Get to work
	    */
	    for (pel = 0;  pel < pelcnt;  pel++)
		{
		    /*
		    **	CMY = NOT RGB
		    */
		    offset = src.UdpL_Pos + 
			     scanline * src.UdpL_ScnStride + 
			     pel * src.UdpL_PxlStride ;
		    cmy_pel.int_value = GetField_(src.UdpA_Base,offset,mask);
		    if (cmy_pel.int_value < 7) 
			{
			    PutField_(sixlptr[cmy_pel.int_value],
					sixel_bit_pos,1,1);
			}
		    /*
		    **	Add sixel offset, update pointers.
		    */
		    for (numclrs = 0; numclrs<7; numclrs++)
			sixlptr[numclrs]++;
		}
	    /*
	    **	If an entire sixel scanline has been constructed, export it.
	    */
	    if (sixel_bit_pos == 5)
		{
		    for (numclrs = 0; numclrs<6; numclrs++ )
			{
			    *sixlptr[numclrs] =  SIXEL_CR;
			    sixlptr[numclrs] = sixlbuf[numclrs] + 2;
			}
		    *sixlptr[6] = SIXEL_NL;
		    sixlptr[6] = sixlbuf[6] + 2;
		    for (pel = 0;  pel < pelcnt;  pel++)
			{
			    for (numclrs = 0; numclrs<7; numclrs++ )
				{
				    *sixlptr[numclrs]++ += 077;
				}
			}
                    for (numclrs = 0; numclrs<7; numclrs++ )
			{		    
			    ExportSixelScanline(sixlbuf[numclrs],&obptr,&obcnt);
			}
		    /*
		    **	Clear scanline following color introducer
		    */
		    for (numclrs = 0; numclrs<7; numclrs++ )
			{
			    memset(sixlbuf[numclrs]+2,0,pelcnt+2);
			}
		}
	}
	/*
	**  If less than six full scanlines at the end, export what remains now.
	*/
	if (sixel_bit_pos != 5)
	    {
		for (numclrs = 0; numclrs<6; numclrs++ )
		    {
			*sixlptr[numclrs] =  SIXEL_CR;
			sixlptr[numclrs] = sixlbuf[numclrs] + 2;
		    }
		*sixlptr[6] = SIXEL_NL;
		sixlptr[6] = sixlbuf[6] + 2;
		for (pel = 0;  pel < pelcnt;  pel++)
		    {
		    for (numclrs = 0; numclrs<7; numclrs++ )
			{
			    *sixlptr[numclrs]++ += 077;
			}
		    }
                for (numclrs = 0; numclrs<7; numclrs++ )
                    {		    
			ExportSixelScanline(sixlptr[numclrs],&obptr,&obcnt);
		    }
	    }
    /*
    **	Return total bytes processed, if requested.
    */
    if (bytcnt != 0)
	*bytcnt = buflen - obcnt;
    /*
    **	Deallocate scanline buffers
    */
     for (numclrs = 0; numclrs<7; numclrs++ )
         {		    
	    _ImgFree(sixlbuf[numclrs]);
	 }

#ifdef TRACE
printf( "Leaving Routine IdsExportLrColorSixels in module IDS_COLOR_SIXEL \n");
#endif
    return(fid);
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
    int spectral_mapping;
    int component_organization;
    int bits_per_component;
    int i;

#ifdef TRACE
printf( "Entering Routine VerifyFrameAttributes in module IDS_COLOR_SIXEL \n");
#endif

    GetIsl_(fid,Img_SpectralMapping,spectral_mapping,0);
    if (spectral_mapping != ImgK_RGBMap)
	ChfSignal(1,ImgX_INVCMPMAP);

    GetIsl_(fid,Img_CompSpaceOrg,component_organization,0);
    if (component_organization != ImgK_OrgFulPxlCmp)
	ChfSignal(1,ImgX_INVCODTYP);
	
    for (i = 0;  i < 3;  i++)
	{
	    GetIsl_(fid,Img_ImgBitsPerComp,bits_per_component,i);
	    if (bits_per_component != 1)
		ChfSignal(1,ImgX_INVBITPXL);
	}

#ifdef TRACE
printf( "Leaving Routine VerifyFrameAttributes in module IDS_COLOR_SIXEL \n");
#endif
	
    return(fid);
}

/*****************************************************************************
**  ExportSixelScanline
**
**  FUNCTIONAL DESCRIPTION:
**
**	Perform sixel data compression on source scanline. Copy sixel
**	compressed sixel scanline data from source to target buffer without
**	overflowing the size of the output buffer. Update output buffer pointer
**	and count.
**
**  FORMAL PARAMETERS:
**
**      srcstr - source ASCIZ sixel scanline data to copy
**	obptr  - address of output buffer pointer
**	obcnt  - address of size of output buffer
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      obptr is updated to point to the end of the copied string
**	obcnt is updated to reflect the bytes left in the output buffer
**
**  FUNCTION VALUE:
**
**      Count of bytes actually exported.
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
static long	ExportSixelScanline(srcstr, obptr, obcnt)
char *srcstr;
char **obptr;
int  *obcnt;
{

#define MIN_REPEAT_COUNT 3

    char *srcptr = srcstr;
    char *cpyptr = srcstr;
    int   cpycnt, actcnt;

#ifdef TRACE
printf( "Entering Routine ExportSixelScanline in module IDS_COLOR_SIXEL \n");
#endif


    /*
    **	First, perform sixel compression. Search the scanline for runs of a
    **	particular sixel byte. If its worth it, replace that run with the
    **	sixel repeat sequence
    */
    while (*srcptr != 0)
	{
	    int  repeat_count = 0;
	    char repeat_char;
	    
	    for (repeat_char = *srcptr; repeat_char == *srcptr; srcptr++)
		repeat_count++;

	    if (repeat_count > MIN_REPEAT_COUNT)
		cpyptr += ExportRepeatSequence(cpyptr,repeat_char,repeat_count);
	    else
		{
		    memset(cpyptr,repeat_char,repeat_count);
		    cpyptr += repeat_count;
		}
	}
    /*
    **	Now, copy the data to the output buffer. If insufficient space exists,
    **	copy as much as possible and signal an error.
    */
    cpycnt = cpyptr - srcstr;
    actcnt = cpycnt > *obcnt ? *obcnt : cpycnt;

    memcpy (*obptr,srcstr,actcnt);

    *obptr += actcnt;
    *obcnt -= actcnt;

    if (actcnt < cpycnt)
	ChfSignal(1,ImgX_BUFOVRFLW);

#ifdef TRACE
printf( "Leaving Routine ExportSixelScanline in module IDS_COLOR_SIXEL \n");
#endif
    return(actcnt);
}

/*****************************************************************************
**  ExportRepeatSequence
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine outputs a sixel repeat sequence of the form:
**
**	    !<count><char>
**
**  FORMAL PARAMETERS:
**
**      outptr       - pointer to output buffer
**	repeat_char  - character to repeat
**	repeat_count - number of times to repeat
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
**      count - number of bytes output to buffer
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
static int	ExportRepeatSequence(
     char   *outptr
    ,char   repeat_char
    ,int    repeat_count
    )
{
/*
**  Assumes maximum signed integer has 10 digits plus introducer and repeat
**  character with a little to spare.
*/
#define MAX_REPEAT_SIZE 15

    char *tmp = (char *) _ImgMalloc(MAX_REPEAT_SIZE);

    int remainder = repeat_count;
    char *strptr = tmp + MAX_REPEAT_SIZE - 1;
    int strcnt = 0;
    int digit;

#ifdef TRACE
printf( "Entering Routine ExportRepeatSequence in module IDS_COLOR_SIXEL \n");
#endif


    /*
    **	String is built backwards in a temporary buffer, then copied into the
    **	output buffer.
    */
    *strptr-- = repeat_char; strcnt++;

    while (remainder != 0)
    {
	digit = remainder % 10;
	remainder /= 10;
	*strptr-- = digit + 060; strcnt++;
    }

    *strptr = '!'; strcnt++;

    memcpy(outptr,strptr,strcnt);

    _ImgFree(tmp);

    return(strcnt);
#ifdef TRACE
printf( "Leaving Routine ExportRepeatSequence in module IDS_COLOR_SIXEL \n");
#endif
}
