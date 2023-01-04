
/*******************************************************************************
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
*******************************************************************************/

/************************************************************************
**  IMG_REMAP_SERVICES.C
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**      IMG_REMAP_SERVICES() creates a new image frame with pixels
**      modified by passing them through a Lookup Table (LUT) provided as an
**      argument to the routine.
**
**      As an option the user may provide a routine which computes the new
**      pixel value instead of using a Lookup Table.
**
**      If neither a LUT or remap function is provided, the effect will
**      be just a copy operation, except that image attributes such as
**      strides, spectral type, etc. may be modified.
**
**      Usually, the LUT will consist of "2**n" entries (n = bits in the
**      Z dimension of the image.
**
**      If the size of the Lookup Table provided is not "2**n" entries, the
**      remapping will be scaled to the number of entries actually present
**      in the table. I.E. if there are less entries in the LUT than
**      possible input values, several input values may map to a single
**      output value; and if there are more LUT entries than possible input
**      values, not all elements of the LUT will be mapped.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      November, 1987
**
**  MODIFICATION HISTORY:
**
**	V1-001	    MWS001  Mark Sornson
**		    Rework of IMG$PIXEL_REMAP module to produce
**		    IMG_PIXEL_REMAP, to fit it into the V3 environment.
************************************************************************/

/*
**  Include files:
*/
#include	<stdlib.h>
#include	<string.h>

#include        <img/ChfDef.h>
#include        <img/ImgDef.h>
#include        <img/ImgDefP.h>
#include        <ImgMacros.h>
#ifndef NODAS_PROTO
#include        <imgprot.h>		    /* IMG prototypes */
#endif

/*
**  MACRO definitions:
*/

/* Several MACROS for getting/putting info in and out of bit arrays, described*/
/* by the parameters in descriptor class A or base/bit offset. Based on shift */
/* and mask algortihm and will work up to getting/putting 25 bit values.      */
/* IMG$AL_ONES_ASCENDING[] is a global array of values 0, 1, 11, 111, 1111 ...*/

/* get value up to 25 bits long described by the base and bit offset      */
/* user of the macro must supply correct mask to mask out unwanted bits   */
#define GET_VALUE_(base,offset,mask) \
   (READ32_(base + (offset >> 3)) >> (offset & 0x7) & mask)

/* put value up to 25 bits long described by the base and bit offset      */
/* user must supply correct mask (IMG$AL_ONES_ASCENDING[nbit])            */
#define PUT_VALUE_(base,offset,mask,value) \
    XOR_WRITE32_((base + (offset >> 3)), (value & mask) << (offset & 0x7))

/* macro to determine if a given combination of LUT descriptor type & class   */
/* are acceptable together in the pixel_remap_lut() function.                 */

#define VALID_DSC_TYPE_(lut) \
(lut->dsc_b_class == DSC_K_CLASS_A && \
lut->dsc_b_dtype == DSC_K_DTYPE_BU) ||      /* array byte unsigned */ \
(lut->dsc_b_class == DSC_K_CLASS_A && \
lut->dsc_b_dtype == DSC_K_DTYPE_WU) ||      /* array word unsigned */ \
(lut->dsc_b_class == DSC_K_CLASS_A && \
lut->dsc_b_dtype == DSC_K_DTYPE_LU) ||      /* array long unsigned */ \
(lut->dsc_b_class == DSC_K_CLASS_UBA && \
lut->dsc_b_dtype == DSC_K_DTYPE_VU)        /* UBA bit unaligned   */

/* macro to determine if the data plane & LUT are both aligned on byte       */
/* boundaries, no fill bits, etc. so that a simple call to _IpsMovtcLong   */
/* will correctly perform the remap.                                         */

#define SPECIAL_CASE_() \
(lut->dsc_b_dtype == DSC_K_DTYPE_BU &&    /* LUT is byte unsigned   */ \
lut->dsc_w_length == 1 &&                 /* 1 byte / value         */ \
/*?lut->dsc_l_arsize == 256 &&                2**8 entries           */ \
pixel_stride == 8 &&                      /* data pixels not padded */ \
new_pixel_stride == 8 && \
(data_offset & 0x7) == 0 &&               /* offset is even # bytes */ \
(new_data_offset & 0x7) == 0 && \
/*? bits_per_pixel == 8 &&  data pixels 8 bits ea. */ \
/*? new_bits_per_pixel == 8 && */ \
pixels_per_line == new_pixels_per_line && \
number_of_lines == new_number_of_lines && \
new_pixels_per_line * new_pixel_stride == new_scanline_stride && \
pixels_per_line * pixel_stride == scanline_stride)/* scanlines not padded*/

/*
**  Equated Symbols:
*/

#define MAX_PXL_SIZE 25                 /* 25 bits largest pixel supported   */

/*
** Structure dsc_descriptor_a, copied from <descrip.h>, to remove
** the dependency of this module on <descrip.h>
*/
struct  dsc_descriptor_a
{
        unsigned short  dsc_w_length;
        unsigned char   dsc_b_dtype;
        unsigned char   dsc_b_class;    
        char           *dsc_a_pointer;
        char            dsc_b_scale;
        unsigned char   dsc_b_digits;
/*
** NOTE:    This field has to be a char field, and not a struct, so that
**	    it aligns properly (i.e., it doesn't get padded, but stays 
**	    packed) under ALPHA builds.
*/
	unsigned char	dsc_b_aflags;
/*
**        struct
**        {
**                unsigned                 : 3;
**                unsigned dsc_v_fl_binscale : 1;
**                unsigned dsc_v_fl_redim  : 1;
**                unsigned dsc_v_fl_column : 1;
**                unsigned dsc_v_fl_coeff  : 1;
**                unsigned dsc_v_fl_bounds : 1;
**        }               dsc_b_aflags;
*/
        unsigned char   dsc_b_dimct;
        unsigned long   dsc_l_arsize;
        /*
         * One or two optional blocks of information may follow contiguously at
         * the first block contains information about the dimension multipliers
         * dsc_b_aflags.dsc_v_fl_coeff is set), the second block contains infor
         * the dimension bounds (if present, dsc_b_aflags.dsc_v_fl_bounds is se
         * bounds information is present, the multipliers information must also
         *
         * The multipliers block has the following format:
         *      char    *dsc_a_a0;              Address of the element whose su
         *      long    dsc_l_m [DIMCT];        Addressing coefficients (multip
         *
         * The bounds block has the following format:
         *      struct
         *      {
         *              long    dsc_l_l;        Lower bound
         *              long    dsc_l_u;        Upper bound
         *      } dsc_bounds [DIMCT];
         *
         * (DIMCT represents the value contained in dsc_b_dimct.)
         */
};

/*
 *	Unaligned Bit Array Descriptor:
 */
struct	dsc_descriptor_uba
{
	unsigned short	dsc_w_length;	/* length of data item in bits */
	unsigned char	dsc_b_dtype;	/* data type code = DSC_K_DTYPE_VU */
	unsigned char	dsc_b_class;	/* descriptor class code = DSC_K_CLASS_UBA */
	char		*dsc_a_base;	/* address to which effective bit offset is relative */
	char		dsc_b_scale;	/* reserved;  must be zero */
	unsigned char	dsc_b_digits;	/* reserved;  must be zero */
	unsigned char	dsc_b_aflags;
/*
**	struct
**	{
**		unsigned		 : 3;	/* reserved;  must be zero
**		unsigned dsc_v_fl_binscale : 1;	/* must be zero
**		unsigned dsc_v_fl_redim	 : 1;	/* must be zero
**		unsigned 		 : 3;	/* reserved;  must be zero
**	}		dsc_b_aflags;	/* array flag bits
*/
	unsigned char	dsc_b_dimct;	/* number of dimensions */
	unsigned long	dsc_l_arsize;	/* total size of array in bits */
	/*
	 * Three blocks of information must follow contiguously at this point;  the first block
	 * contains information about the difference between the bit addresses of two adjacent
	 * elements in each dimension (the stride).  The second block contains information
	 * about the dimension bounds.  The third block is the relative bit position with
	 * respect to dsc_a_base of the first actual bit of the array.
	 *
	 * The strides block has the following format:
	 *	long		dsc_l_v0;		Bit offset of the element whose subscripts are all zero,
	 *						with respect to dsc_a_base
	 *	unsigned long	dsc_l_s [DIMCT];	Strides
	 *
	 * The bounds block has the following format:
	 *	struct
	 *	{
	 *		long	dsc_l_l;		Lower bound
	 *		long	dsc_l_u;		Upper bound
	 *	} dsc_bounds [DIMCT];
	 *
	 * The last block has the following format:
	 *	long	dsc_l_pos;
	 *
	 * (DIMCT represents the value contained in dsc_b_dimct.)
	 */
	};

/*
**  External References:
*/

#ifdef NODAS_PROTO

/* VMS runtime library                                                       */

void ChfSignal();                       /* error signal routine             */
void ChfStop();                         /* signal and stop                  */

/* Image Services Library routines                                           */

struct FCT  *ImgGetFrameAttributes();
struct FCT  *ImgSetFrameAttributes();

struct FCT  *ImgAllocateFrame();

char		*_ImgAllocateDataPlane();
struct FCT	*_ImgCloneFrame();
struct ITMLST	*_ImgCreateItmlst();
void		 _ImgDeleteItmlst();
struct FCT	*_ImgGet();
struct FCT	*_ImgPut();
void		 _ImgSetRoi();
void		 _ImgStoreDataPlane();
long		 _ImgTotalPutlstItems();

void		 _IpsMovtcLong();
#endif

/* mask array for the shift-and-mask MACROs                                  */
#if defined(__VAXC) || defined(VAXC)
globalref unsigned int IMG_AL_ONES_ASCENDING[33];
#else
extern unsigned int IMG_AL_ONES_ASCENDING[33];
#endif

/* error messages                                                            */
#include        <img/ImgStatusCodes.h>    /* status codes defns.               */

/*
**  Local Storage: 
*/

/*
**  Table of contents:
**
**	Global entry points
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$PIXEL_REMAP();	    /* remap pixels using LUT or user func  */
#endif
#ifdef NODAS_PROTO
struct FCT *ImgPixelRemap();	    /* DECWindow style entry point	    */
struct FCT *_remap_UDP();	    /* remap a data plane		    */
#else
struct FCT *ImgPixelRemap(struct FCT */*fid*/, struct dsc_descriptor_a */*lut*/, struct PUT_ITMLST */*attr*/, unsigned int /*flags*/, int (*/*user_remap*/)(), unsigned int /*userparm*/, struct ROI */*roi*/);
struct FCT *_remap_UDP(struct FCT */*fid*/, char */*new_dp*/, struct dsc_descriptor_a */*lut*/, struct PUT_ITMLST */*attr*/, unsigned long /*flags*/, struct ROI */*roi*/);
#endif

/* 
**	Module local routines
*/
#ifdef NODAS_PROTO
unsigned char *_remap_lut();  /* create new LUT for special cases */

static void img__pixel_remap_lut();   /* remap image by pixel with a LUT      */
static void img__pixel_remap_user();  /* remap image by pixel w/ user function*/
static void img__pixel_remap_copy();  /* remap image, unchanged pixel values  */

static void _remap_1_byte();      /* remap rtns. for 1,2,3 & 4 byte LUTs  */
static void _remap_2_byte();
static void _remap_3_byte();
static void _remap_4_byte();

static struct ITMLST	*Cvt_putlst_to_itmlst();
#else
PROTO(unsigned char *_remap_lut, (struct dsc_descriptor_a */*lut*/, int /*bits_per_pixel*/, unsigned char */*new_lut*/, int /*new_bits_per_pixel*/));
PROTO(static void img__pixel_remap_lut, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct dsc_descriptor_a */*lut*/, struct ROI */*roi*/));
PROTO(static void img__pixel_remap_user, (struct FCT */*fid*/, struct FCT */*new_fid*/, int (*/*user_remap*/)(), unsigned int /*userparm*/, struct ROI */*roi*/));
PROTO(static void img__pixel_remap_copy, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct ROI */*roi*/));
PROTO(static void _remap_1_byte, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct dsc_descriptor_a */*lut*/, struct ROI */*roi*/));
PROTO(static void _remap_2_byte, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct dsc_descriptor_a */*lut*/, struct ROI */*roi*/));
PROTO(static void _remap_3_byte, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct dsc_descriptor_a */*lut*/, struct ROI */*roi*/));
PROTO(static void _remap_4_byte, (struct FCT */*fid*/, struct FCT */*new_fid*/, struct dsc_descriptor_a */*lut*/, struct ROI */*roi*/));
PROTO(static struct ITMLST *Cvt_putlst_to_itmlst, (struct PUT_ITMLST */*putlst*/));
#endif

/******************************************************************************
**  IMG$PIXEL_REMAP
**  ImgPixelRemap
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**     fid        - input frame id
**     LUT        - descriptor for user's remap table.
**     ATTR       - list of attributes applied to the new frame
**     FLAGS      - special processing flags
**     USER_REMAP - address of user-supplied routine to remap the data. Routine
**                  will be called with longword values representing a data
**                  value, and x/y coordinates and return a longword 
**                  remapped value.
**     USERPARM   - user parameter to be supplied to the user remap function
**     ROI        - region of interest descriptor (optional)
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**     Returns new frame identifier of remapped frame created by the routine.
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT	Invalid argument count
**
**  SIDE EFFECTS:
**
*******************************************************************************
** VMS Entry point
*******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$PIXEL_REMAP(fid,lut,attr,flags,user_remap,userparm,roi)
struct FCT		 *fid;		    /* input frame to remap         */
struct dsc_descriptor_a	 *lut;		    /* Lookup table descriptor      */
struct PUT_ITMLST	 *attr;		    /* attributes for new frame     */
unsigned int		  flags;	    /* special processing flags     */
long			(*user_remap)();    /* User supplied remap function */
unsigned long		  userparm;	    /* parm for USER_REMAP()        */
struct ROI		 *roi;		    /* Region of Interest           */
{

return (ImgPixelRemap(fid,lut,attr,flags,user_remap,userparm,roi));
} /* end of IMG$PIXEL_REMAP */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgPixelRemap(fid,lut,attr,flags,user_remap,userparm,roi)
struct FCT    *fid;                    /* input frame to remap            */
struct dsc_descriptor_a *lut;          /* Lookup table descriptor         */
struct PUT_ITMLST *attr;               /* attributes for new frame        */
unsigned int  flags;                   /* special processing flags        */
int           (*user_remap)();         /* User supplied remap function    */
unsigned int  userparm;                /* parm for USER_REMAP()           */
struct ROI    *roi;                    /* Region of Interest              */
{
char	*new_plane_data_base;

long	 bits_per_pixel;
long	 compression_type;
long	 idx;
long	 local_flags;
long	 new_number_of_lines;
long	 new_pixels_per_line;
long	 new_pixel_stride;
long	 new_scanline_stride;
long	 new_bits_per_pixel;
long	 new_data_offset;
long	 new_size_bits,new_size_bytes;

struct GET_ITMLST    itmlst[3];
struct ITMLST	    *local_itmlst;
struct FCT	    *new_fid;                /* new frame to be returned        */
struct GET_ITMLST    new_itmlst[7];
struct UDP	     udp;

idx = 0;

/* dp: commented the following block as sizeof(int) should be sizeof(long)
 * ootb_bug 428. 
 */

/*********
INIT_GET_(itmlst,idx,Img_CompressionType,sizeof(int),&compression_type,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel, 0,0);
**********/
INIT_GET_(itmlst,idx,Img_CompressionType,sizeof(long),&compression_type,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(long),&bits_per_pixel, 0,0);
END_GET_(itmlst,idx);

idx = 0;
/*************
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine, sizeof(int), &new_pixels_per_line, 0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines, sizeof(int), &new_number_of_lines, 0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel, sizeof(int), &new_bits_per_pixel, 0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int), &new_scanline_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride, sizeof(int), &new_pixel_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset, sizeof(int), &new_data_offset, 0,0);
****************/
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine, sizeof(long), &new_pixels_per_line, 0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines, sizeof(long), &new_number_of_lines, 0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel, sizeof(long), &new_bits_per_pixel, 0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(long), &new_scanline_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride, sizeof(long), &new_pixel_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset, sizeof(long), &new_data_offset, 0,0);
END_GET_(new_itmlst,idx);

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    /* verify that input frame is OK to work on                               */

    if(compression_type != ImgK_PcmCompression)    /* image is compressed */
        ChfSignal( 1, ImgX_UNSCMPTYP);

    if (bits_per_pixel > MAX_PXL_SIZE)             /* max for GET_VALUE macro */
        ChfStop( 1, ImgX_PXLTOOLRG);

    /* 
    ** build a new frame from the old one
    */
    local_flags =   ImgM_NoDataPlaneAlloc |
		    ImgM_NoStandardize |
		    ImgM_NonstandardVerify ;
    local_itmlst = Cvt_putlst_to_itmlst( attr );
    new_fid = ImgAllocateFrame( 0, local_itmlst, fid, local_flags );
    if ( local_itmlst != 0 )
	_ImgDeleteItmlst( local_itmlst );

    if (roi != 0)   /* override new frame pix/line, # lines if ROI present    */
        {
        _ImgSetRoi(&udp,roi);
        new_number_of_lines = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;
        new_pixels_per_line = udp.UdpL_X2 - udp.UdpL_X1 + 1;
        new_scanline_stride = new_pixels_per_line * udp.UdpL_PxlStride;

        _ImgPut(new_fid,Img_ScanlineStride,&new_scanline_stride,sizeof(int), 0);
        _ImgPut(new_fid,Img_NumberOfLines,&new_number_of_lines,sizeof(int), 0);
        _ImgPut(new_fid,Img_PixelsPerLine,&new_pixels_per_line,sizeof(int), 0);
        }

    if (attr != 0)
        ImgSetFrameAttributes(new_fid, attr);

    /* get frame attributes for new frame, set ROI                            */

    ImgGetFrameAttributes(new_fid,new_itmlst);

    /* see if the frame attributes are consistent                             */

    if (new_number_of_lines != (udp.UdpL_Y2 - udp.UdpL_Y1 + 1))
        ChfStop( 1, ImgX_INCFRMATT);                 /* inconsistent frame attrs */
    if (new_pixels_per_line != (udp.UdpL_X2 - udp.UdpL_X1 + 1))
        ChfStop( 1, ImgX_INCFRMATT);
    if ((new_pixels_per_line * new_pixel_stride) > new_scanline_stride)
        ChfSignal( 1, ImgX_INVSCNSTR);
    if (new_bits_per_pixel > new_pixel_stride)
        ChfSignal( 1, ImgX_UNSPXLSTR);

    /* figure new image size necessary                                       */

    new_size_bits = new_data_offset + 
                   (new_number_of_lines * new_scanline_stride);
    new_size_bytes = (new_size_bits+7) / 8;
    new_plane_data_base = _ImgAllocateDataPlane(new_size_bytes, 0);
    _ImgStoreDataPlane( new_fid, new_plane_data_base );

    /* call one of three internal routines depending on whether lut, user_remap*/
    /* or neither specified. Notice if lut is specified, user_remap is ignored*/

    if (lut != 0)
        img__pixel_remap_lut(fid,new_fid,lut,roi);
    if (lut == 0 && user_remap == 0)
        img__pixel_remap_copy(fid,new_fid,roi);
    if (user_remap != 0 && lut == 0)
        img__pixel_remap_user(fid,new_fid,user_remap,userparm,roi);

    return (new_fid);
    }

/************************************************************************
**
**  _remap_UDP()
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**     FID        - input frame id
**     NEW_DP	  - new data plane
**     LUT        - descriptor for user's remap table.
**     ATTR       - list of attributes applied to the new frame
**     FLAGS      - special processing flags
**     ROI        - region of interest descriptor (optional)
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**     Returns new frame identifier of remapped frame created by the routine.
**
**  SIGNAL CODES:
**
**      ImgX_INVARGCNT	Invalid argument count
**
**  SIDE EFFECTS:
**
************************************************************************/
struct FCT *_remap_UDP(fid,new_dp,lut,attr,flags,roi)
    struct FCT		    *fid;	/* input frame to remap	    */
    char		    *new_dp;	/* new data plane	    */
    struct dsc_descriptor_a *lut;	/* Lookup table descriptor  */
    struct PUT_ITMLST	    *attr;	/* attributes for new frame */
    unsigned long	     flags;	/* special processing flags */
    struct ROI		    *roi;	/* Region of Interest	    */

    {
    struct FCT    *new_fid;                /* new frame to be returned       */
    int           bits_per_pixel;
    int           compression_type;
    int           new_number_of_lines;
    int           new_pixels_per_line;
    int           new_pixel_stride;
    int           new_scanline_stride;
    int           new_bits_per_pixel;
    int           new_data_offset;
    int           new_size_bits,new_size_bytes;
    char         *new_plane_data_base;

    struct UDP udp;

    int idx;
    struct GET_ITMLST itmlst[3];
    struct GET_ITMLST new_itmlst[7];

idx = 0;

INIT_GET_(itmlst,idx,Img_CompressionType,sizeof(int),&compression_type,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel, 0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine, sizeof(int), &new_pixels_per_line, 0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines, sizeof(int), &new_number_of_lines, 0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel, sizeof(int), &new_bits_per_pixel, 0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int), &new_scanline_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride, sizeof(int), &new_pixel_stride, 0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset, sizeof(int), &new_data_offset, 0,0);
END_GET_(new_itmlst,idx);

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    /* verify that input frame is OK to work on                               */

    if(compression_type != ImgK_PcmCompression)    /* image is compressed */
        ChfSignal( 1, ImgX_UNSCMPTYP);

    if (bits_per_pixel > MAX_PXL_SIZE)             /* max for GET_VALUE macro */
        ChfStop( 1, ImgX_PXLTOOLRG);

    /* build a new frame from the old one                                     */
    new_fid = (struct FCT *) _ImgCloneFrame (fid);

    if (roi != 0)   /* override new frame pix/line, # lines if ROI present    */
        {
        _ImgSetRoi(&udp,roi);
        new_number_of_lines = udp.UdpL_Y2 - udp.UdpL_Y1 + 1;
        new_pixels_per_line = udp.UdpL_X2 - udp.UdpL_X1 + 1;
        new_scanline_stride = new_pixels_per_line * udp.UdpL_PxlStride;

        _ImgPut(new_fid,Img_ScanlineStride,&new_scanline_stride,sizeof(int), 0);
        _ImgPut(new_fid,Img_NumberOfLines,&new_number_of_lines,sizeof(int), 0);
        _ImgPut(new_fid,Img_PixelsPerLine,&new_pixels_per_line,sizeof(int), 0);
        }

    if (attr != 0)
        ImgSetFrameAttributes(new_fid, attr);

    if (attr != 0)
        ImgSetFrameAttributes(fid, attr);

    /* get frame attributes for new frame, set ROI                            */

    ImgGetFrameAttributes(new_fid,new_itmlst);

    /* see if the frame attributes are consistent                             */

    if (new_number_of_lines != (udp.UdpL_Y2 - udp.UdpL_Y1 + 1))
        ChfStop( 1, ImgX_INCFRMATT);                 /* inconsistent frame attrs */
    if (new_pixels_per_line != (udp.UdpL_X2 - udp.UdpL_X1 + 1))
        ChfStop( 1, ImgX_INCFRMATT);
    if ((new_pixels_per_line * new_pixel_stride) > new_scanline_stride)
        ChfSignal( 1, ImgX_INVSCNSTR);
    if (new_bits_per_pixel > new_pixel_stride)
        ChfSignal( 1, ImgX_UNSPXLSTR);

    /* figure new image size necessary                                       */

    new_size_bits = new_data_offset + 
                   (new_number_of_lines * new_scanline_stride);
    new_size_bytes = (new_size_bits+7) / 8;

    new_plane_data_base = new_dp;
    /* Use the same data plane to process one component at a time */

    if (new_plane_data_base == 0)
        new_plane_data_base = _ImgAllocateDataPlane(new_size_bytes, 0);

    _ImgStoreDataPlane( new_fid, new_plane_data_base );
    /* call one of three internal routines depending on whether lut, user_remap*/
    /* or neither specified. Notice if lut is specified, user_remap is ignored*/

    if (lut != 0)
        img__pixel_remap_lut(fid,new_fid,lut,roi);
    if (lut == 0)
        img__pixel_remap_copy(fid,new_fid,roi);
    return (new_fid);
    } /* end of _remap_udp */

/***************************************************************************
**
** _remap_lut()
**
** Internal routine which remaps a smaller LUT into a 256 byte LUT to allow
** quicker transfers of data using _IpsMovtcLong instruction
**
****************************************************************************/
unsigned char *_remap_lut(lut,bits_per_pixel,new_lut,new_bits_per_pixel)
    struct dsc_descriptor_a *lut;
    int bits_per_pixel, new_bits_per_pixel;
    unsigned char *new_lut;
    {/* start _remap_lut */
    char lut_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];
    int i;
    int num_lut_entries = 1 << bits_per_pixel;
    char *src_ptr = lut->dsc_a_pointer;
    /*
    ** Populate the new LUT, replicating the original LUT 
    */
    for (i=0;  i < 256;  i++)
	*(new_lut+i) = *(src_ptr + (i % num_lut_entries)) & lut_mask;
    return(new_lut);
    }/* end _remap_lut */

/***************************************************************************
**
** img__pixel_remap_lut()
**
** Internal routine which performs remapping of pixels using a lookup table
** specified as a descriptor by the user.
**
****************************************************************************/
static void img__pixel_remap_lut(fid,new_fid,lut,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct dsc_descriptor_a *lut;
    struct ROI *roi;
    {
    /*
    **      Unaligned bit array descriptor: this is the ISL version which
    **      includes the optional blocks needed for a 1-D array descriptor.
    */
    struct  IMG_DESCRIPTOR_UBA 
        {
        struct  dsc_descriptor_uba uba;
        /*
         * Three blocks of information must follow contiguously at this point;  the first block
         * contains information about the difference between the bit addresses of two adjacent
         * elements in each dimension (the stride).  The second block contains information
         * about the dimension bounds.  The third block is the relative bit position with
         * respect to dsc_a_base of the first actual bit of the array.
         *
         * The strides block has the following format:
         *      long            dsc_l_v0;               Bit offset of the element whose subscripts are all zero,
         *                                              with respect to dsc_a_base
         *      unsigned long   dsc_l_s [DIMCT];        Strides
         *
         * The bounds block has the following format:
         *      struct  {
         *              long    dsc_l_l;                Lower bound
         *              long    dsc_l_u;                Upper bound
         *              } dsc_bounds [DIMCT];
         *
         * The last block has the following format:
         *      long    dsc_l_pos;
         *
         * (DIMCT represents the value contained in dsc_b_dimct.)
         */
        long   dsc_l_v0; /* Bit offset of element whose subscripts are all zero,*/
                         /* with respect to dsc_a_base         */
        unsigned long   dsc_l_s [1]; /*       Strides  */
        struct  {
                long    dsc_l_l;                /*  Lower bound  */
                long    dsc_l_u;                /*  Upper bound  */
                } dsc_bounds [1];
        long    dsc_l_pos;                      /* bit offset to 1st bit     */
        } *uba_lut;

    int           n_args;
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,lut_value;
    unsigned int  lut_bit_offset;

    unsigned int  get_mask,put_mask,lut_mask;
    unsigned char new_lut[256];           /* buffer for special case new LUT  */
    unsigned char *new_lut_ptr;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP	udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine, sizeof(int), &pixels_per_line, 0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride, sizeof(int),&scanline_stride,0,0); 
INIT_GET_(itmlst,idx,Img_PixelStride, sizeof(int), &pixel_stride, 0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines, sizeof(int), &number_of_lines, 0,0);
INIT_GET_(itmlst,idx,Img_DataOffset, sizeof(int), &data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    uba_lut = (struct IMG_DESCRIPTOR_UBA *)lut;

    /* verify that LUT descriptor is a type we understand                     */

    if (lut->dsc_b_dimct != 1)                      /* LUT must be a 1D vector*/
        ChfStop( 1, ImgX_INVDSCTYP);

    if (!(VALID_DSC_TYPE_(lut)))
        ChfStop( 1, ImgX_INVDSCTYP);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame UDP                         */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    if (lut->dsc_b_class == DSC_K_CLASS_A)
        {
	/* LUT is an aligned byte array. 
        ** check for special case of 8-bit aligned data and 8-bit aligned LUT
        ** and new image having same alignment, bits/pixel etc. as old
        ** because we can just do a MOVTC_LONG of the whole mess.
	*/
        if (SPECIAL_CASE_())
	    {
	    /*
	    ** There are 2 instances in which _IpsMovtcLong may be used
	    ** in the second instance a new LUT must be created
	    */
	    if (bits_per_pixel == 8 && 
		new_bits_per_pixel == 8 &&
		lut->dsc_l_arsize == 256)
                {
                /* 
                ** Special case with no change to the LUT 
                */
		new_lut_ptr = (unsigned char *) lut->dsc_a_pointer;
		}/* end special case with no change to the LUT */
	    else
		{
		/*
		** Create a new 256 byte LUT by remapping the smaller
		** LUT and masking the values with the new_bits_per_pixel
		*/
		new_lut_ptr = _remap_lut(lut,bits_per_pixel,
                    new_lut,new_bits_per_pixel);
		}/* end "else" create new LUT */
            _IpsMovtcLong(pixels_per_line*number_of_lines,
	        plane_data_base+(data_offset >> 3),
                0,
                new_lut_ptr,
                pixels_per_line*number_of_lines,
                new_plane_data_base+(new_data_offset >> 3));
            }/* end special case */
         else
             {
             /* 
	     ** Will do special case for each size of LUT value (1,2,3,4 bytes)
	     ** to make them run as quickly as possible...                  
	     */
             switch (lut->dsc_w_length)
                 {/* switch on lut length */
                 case 1:
                    _remap_1_byte(fid,new_fid,lut,roi);
                    break;
                 case 2:
                    _remap_2_byte(fid,new_fid,lut,roi);
                    break;
                 case 3:
                    _remap_3_byte(fid,new_fid,lut,roi);
                    break;
                 case 4:
                    _remap_4_byte(fid,new_fid,lut,roi);
                    break;
                 default:
                    ChfStop( 1, ImgX_INVDSCTYP); /* invalid length in LUT descriptor */
                    break;
                 }/* end switch on LUT length */
             }/* end "else" of non-special case */
        }/* end LUT is an aligned byte array. */
    else  
        {
        /* LUT is passed as a UBA descriptor
        ** make a lut_mask for GET_VALUE macro, 
	** since LUT is unaligned now too.
	*/
        lut_mask = IMG_AL_ONES_ASCENDING[uba_lut->uba.dsc_w_length];

        for (iy = 0; iy < new_number_of_lines; iy++)
            {
            /* compute bit offset to beginning of this scanline.       */

            line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
            new_line_offset = new_data_offset + iy * new_scanline_stride;

            for (ix = 0; ix < new_pixels_per_line; ix++)
                {
                /*
		** Compute bit offset within this line
                */
                data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
                new_data_bit_offset = new_line_offset + ix * new_pixel_stride;

		/*      
                ** Get the original data value
		*/
                orig_value = GET_VALUE_(plane_data_base,data_bit_offset,
                    get_mask);

                /* look up the LUT value corresponding to the original  */
                /* bit offset is pos + stride * value                   */

                lut_bit_offset = uba_lut->dsc_l_pos + 
                    orig_value * uba_lut->dsc_l_s[0];
                lut_value = GET_VALUE_(uba_lut->uba.dsc_a_base,lut_mask,
                    lut_bit_offset);

                /* replace the original value with the transformed value*/

                PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                    put_mask,lut_value);
                }/* End inner "for"	*/
            }/* End outer "for"         */
        }/* End "else" LUT is unaligned */
    return;
    } /* end of img__pixel_remap_lut */

/***************************************************************************
**
** img__pixel_remap_user()
**
** Internal routine which allows the user to specified a remap routine
** which will provide the output frame pixel values, instead of using a
** lookup table.
**
****************************************************************************/
static void img__pixel_remap_user(fid,new_fid,user_remap,userparm,roi)
    struct FCT    *fid;                   /* input frame ID                   */
    struct FCT    *new_fid;               /* output frame ID                  */
    int           (*user_remap)();        /* remap function address           */
    unsigned int  userparm;               /* user's parm to remap function    */
    struct ROI    *roi;                   /* ROI pointer                      */
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,user_value;
 
    unsigned int  get_mask,put_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;
 
    struct UDP	  udp;
 
    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames,data address,set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame's UDP....                   */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
      
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
      
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                          */
         
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* call user remap routine to provide output pixel value            */
      
            user_value = (*user_remap)(orig_value,ix,iy,userparm);
      
            /* replace the original value with the transformed value*/
         
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,user_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of img__pixel_remap_user */

/***************************************************************************
**
** img__pixel_remap_copy()
**
** Internal routine which is invoked when the user has not specified 
** either a LUT or a user_remap() function. The output pixel values are taken
** unchanged from the input frame, however, stride and other attributes
** may have been modified in the output frame, and pixels may be truncated
** if the output frame bits/pixel is less than the original frame bits/pixel.
**
****************************************************************************/
static void img__pixel_remap_copy(fid,new_fid,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct ROI    *roi;
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value;

    unsigned int  get_mask,put_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP    udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame's UDP                       */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
      
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
      
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                                      */
       
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* move original value to output frame                              */
      
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,orig_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of img__pixel_remap_copy */

static void _remap_1_byte(fid,new_fid,lut,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct dsc_descriptor_a *lut;
    struct ROI *roi;
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,lut_value;
    unsigned int  lut_bit_offset;

    unsigned int  get_mask,put_mask,lut_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP    udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame UDP                         */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
            
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
         
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                          */
      
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* look up the LUT value corresponding to the original  */
      
            lut_value = *(lut->dsc_a_pointer + orig_value);
      
            /* replace the original value with the transformed value*/
      
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,lut_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of _remap_1_byte */

static void _remap_2_byte(fid,new_fid,lut,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct dsc_descriptor_a *lut;
    struct ROI *roi;
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,lut_value;
    unsigned int  lut_bit_offset;

    unsigned int  get_mask,put_mask,lut_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP    udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame UDP                         */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
            
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
         
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                          */
      
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* look up the LUT value corresponding to the original  */
      
            lut_value = *((short int *)lut->dsc_a_pointer + orig_value);
      
            /* replace the original value with the transformed value*/
      
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,lut_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of _remap_2_byte */

static void _remap_3_byte(fid,new_fid,lut,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct dsc_descriptor_a *lut;
    struct ROI *roi;
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,lut_value;
    unsigned int  lut_bit_offset;

    unsigned int  get_mask,put_mask,lut_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP	  udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame UDP                         */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
            
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
         
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                          */
      
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* look up the LUT value corresponding to the original and       */
            /* use memcpy to move it to lut_value                            */
      
            memcpy(&lut_value, lut->dsc_a_pointer + (orig_value * 3),3);
      
            /* replace the original value with the transformed value*/
      
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,lut_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of _remap_3_byte */

static void _remap_4_byte(fid,new_fid,lut,roi)
    struct FCT *fid;
    struct FCT *new_fid;
    struct dsc_descriptor_a *lut;
    struct ROI *roi;
    {
    int           iy,ix;
    int           bits_per_pixel,new_bits_per_pixel;
    unsigned int  number_of_lines,new_number_of_lines;
    unsigned int  pixels_per_line,new_pixels_per_line;
    unsigned int  scanline_stride,new_scanline_stride;
    unsigned int  pixel_stride,new_pixel_stride;
    unsigned int  data_offset,new_data_offset;
    unsigned int  line_offset,new_line_offset;
    unsigned int  data_bit_offset,new_data_bit_offset;
    unsigned char *plane_data_base,*new_plane_data_base;
    unsigned int  orig_value,lut_value;
    unsigned int  lut_bit_offset;

    unsigned int  get_mask,put_mask,lut_mask;
    unsigned int  new_size_bits,new_size_bytes;
    int           status;

    struct UDP    udp;

    int idx;
    struct GET_ITMLST itmlst[8];
    struct GET_ITMLST new_itmlst[8];

idx = 0;
INIT_GET_(itmlst,idx,Img_PixelsPerLine,sizeof(int),&pixels_per_line,0,0);
INIT_GET_(itmlst,idx,Img_ScanlineStride,sizeof(int),&scanline_stride,0,0);
INIT_GET_(itmlst,idx,Img_PixelStride,sizeof(int),&pixel_stride,0,0);
INIT_GET_(itmlst,idx,Img_NumberOfLines,sizeof(int),&number_of_lines,0,0);
INIT_GET_(itmlst,idx,Img_DataOffset,sizeof(int),&data_offset,0,0);
INIT_GET_(itmlst,idx,Img_BitsPerPixel,sizeof(int),&bits_per_pixel,0,0);
INIT_GET_(itmlst,idx,Img_DataPlaneBase,sizeof(char *),&plane_data_base,0,0);
END_GET_(itmlst,idx);

idx = 0;
INIT_GET_(new_itmlst,idx,Img_PixelsPerLine,sizeof(int),&new_pixels_per_line,0,0);
INIT_GET_(new_itmlst,idx,Img_ScanlineStride, sizeof(int),&new_scanline_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_PixelStride,sizeof(int),&new_pixel_stride,0,0);
INIT_GET_(new_itmlst,idx,Img_NumberOfLines,sizeof(int),&new_number_of_lines,0,0);
INIT_GET_(new_itmlst,idx,Img_DataOffset,sizeof(int),&new_data_offset,0,0);
INIT_GET_(new_itmlst,idx,Img_BitsPerPixel,sizeof(int),&new_bits_per_pixel,0,0);
INIT_GET_(new_itmlst,idx,Img_DataPlaneBase,sizeof(char *),&new_plane_data_base,0,0);
END_GET_(new_itmlst,idx);

    /* get frame attributes for old & new frames, data address, set ROI       */

    ImgGetFrameAttributes(fid,itmlst);
    _ImgGet(fid, Img_Udp, &udp, sizeof(struct UDP), 0, 0);

    ImgGetFrameAttributes(new_fid,new_itmlst);

    if (roi != 0)   /* set ROI parms in old frame UDP                         */
        _ImgSetRoi(&udp,roi);

    /* get correct bit-masks for GET_VALUE and PUT_VALUE macros               */

    get_mask = IMG_AL_ONES_ASCENDING[bits_per_pixel];
    put_mask = IMG_AL_ONES_ASCENDING[new_bits_per_pixel];

    for (iy = 0; iy < new_number_of_lines; iy++)
        {
        /* compute bit offset to beginning of this scan in new & old*/
        /* image planes.                                           */
            
        line_offset = udp.UdpL_Pos + iy * udp.UdpL_ScnStride;
        new_line_offset = new_data_offset + iy * new_scanline_stride;

        for (ix = 0; ix < new_pixels_per_line; ix++)
            {
            /* compute bit offset within this line, for new & old   */
            /* image planes.                                        */
         
            data_bit_offset = line_offset + ix * udp.UdpL_PxlStride;
            new_data_bit_offset = new_line_offset + ix * new_pixel_stride;
      
            /* get the original data value                          */
      
            orig_value = GET_VALUE_(plane_data_base,data_bit_offset,get_mask);
      
            /* look up the LUT value corresponding to the original  */
      
            lut_value = *((int *)lut->dsc_a_pointer + orig_value);
      
            /* replace the original value with the transformed value*/
      
            PUT_VALUE_(new_plane_data_base,new_data_bit_offset,
                put_mask,lut_value);
            }
        }   /* end outer "for" */
    return;
    } /* end of _remap_4_byte */


static struct ITMLST *Cvt_putlst_to_itmlst( putlst )
struct PUT_ITMLST *putlst;
{
long		 element_count	= 0;
struct ITMLST	*itmlst		= 0;
struct ITMLST	*ret_itmlst	= 0;

if ( putlst != 0 )
    {
    element_count = _ImgTotalPutlstItems( putlst );
    ++element_count;				/* add terminator element   */

    itmlst = _ImgCreateItmlst( element_count );
    ret_itmlst = itmlst;

    while ( putlst->PutL_Code != 0 )
	{
	itmlst->ItmL_Code   = putlst->PutL_Code;
	itmlst->ItmL_Length = putlst->PutL_Length;
	itmlst->ItmA_Buffer = putlst->PutA_Buffer;
	itmlst->ItmA_Retlen = 0;
	itmlst->ItmL_Index  = putlst->PutL_Index;

	++putlst;
	++itmlst;
	}
    }

return ret_itmlst;
} /* end of Cvt_putlst_to_itmlst */
