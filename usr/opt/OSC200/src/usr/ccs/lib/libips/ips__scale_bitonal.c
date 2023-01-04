/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S.
**  Government is subject to restrictions as set forth in Subparagraph
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**
**  ABSTRACT:
**
**	This module scales (larger or smaller) a frame (or ROI) that is
**	bitonal.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**
**  AUTHOR(S):
**
**      Bob Shelley
**	Richard Piccolo (ISL Version 3.0 rework)
**
**
**  CREATION DATE:
**
**	October 15, 1986
**	November 14, 1989 (Version 3.0)
**
*****************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
long	_IpsScaleBitonal();		    /* takes new argument list	      */
#endif

#ifdef	DEBUG_SCALE
    /*
    **  Horizontal and Vertical CHANGELIST dump routines
    */
    static void dv();
    static void cdump();
    static void vdump();
#endif

/*
**  Include files 
*/
#include <IpsDef.h>			    /* Image definitions	    */
#include <IpsMacros.h>			    /* IPS macro definitions	    */
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#include <IpsStatusCodes.h>		    /* IPS Status Codes		    */
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
/*
**  MACRO definitions -- none
*/

/*
**  Equated Symbols
*/
#define	BLANK 1	/* _IpsBuildChangelist size returned if scanline is BLANK */
#define	ALL0  8 /* VLIST window pre-set for NEXT == CURRENT == OUT == 0's */
#define	ALL1  7 /* VLIST window pre-set for NEXT == CURRENT == OUT == 1's */
#define OUT   1 /* VLIST window mask for OUT polarity bit		  */
#define NIL   0 /* Table entries that are never accessed		  */

/*
**  type definition: "Pointer to a Function returning a pointer to a UDP"
*/
typedef struct UDP *(*PF_UDP)();

    /*
    **  structure describing vertical run transition points
    */
static struct VPOINT
    {
    long		flink;			/* link to next active column	    */
    long		window;			/* vertical window state	    */
    };

    /*
    **  structure containing parameters for scale routines
    */
static struct SC_PARAMS
    {
    double
	x_scale, y_scale;	/* scale factors			    */
    long
	s_pos,  d_pos,		/* src and dst pixel offset from BASE	    */
	s_xcnt, d_xcnt,		/* src and dst pixel count in X dimension   */
	s_ycnt, d_ycnt,    	/* src and dst pixel count in Y dimension   */
	s_lin,  d_lin,    	/* src and dst scanline numbers		    */
	*h_run, size,		/* horizontal CHANGELIST and size	    */
	*v_table,		/* IpsM_SaveVertical decision table	    */
	*x_map;			/* pre-scaled X position array		    */
    PF_UDP
	horizontal;		/* horizontal scaling routine		    */
    struct UDP
	*s, *d;			/* src and dst UDP descriptors		    */
    struct VPOINT
	*v_run;			/* IpsM_SaveVertical linked VLIST array   */
    };


/*
**  External References, status codes
*/
/*
**  External References from Ips		<- from module ->
*/
#ifdef NODAS_PROTO
long		*_IpsBuildChangelist();		/* IPS__CHANGE_LIST	*/
long		 _IpsCopyData();		/* IPS__COPY_UTILS	*/
void		 _IpsPutRuns();			/* IPS__CHANGE_LIST	*/
long		_IpsVerifyNotInPlace();		/* from Ips__udp_utils  */
#endif
/* 
** Internal routines
*/
/*
**  Horizontal and Vertical scaling routines
*/
#ifdef NODAS_PROTO
static long    _IpsScaleBitonalSpecial();   /* takes a parameter block as args*/
static void	load_sp();		    /* convert to new udp (for v2)    */
static long		*detail_h();	    /* shrink and save X detail	    */
static long		*shrink_h();	    /* shrink X, allow discard	    */
static long		*expand_h();	    /* expand X	run lengths	    */

static struct UDP	*retain_v();	    /* retain Y, scale X or copy    */
static struct UDP	*detail_v();	    /* shrink and save Y detail	    */
static struct UDP	*shrink_v();	    /* shrink Y, allow discard	    */
static struct UDP	*expand_v();	    /* expand Y	run lengths	    */

static struct SC_PARAMS *analyze_src();	    /* build VLIST from CHANGELISTs */
static long		*extract_dst();	    /* build CHANGELIST from VLIST  */
#else
PROTO(static void load_sp, (struct UDP */*src_udp*/, struct UDP */*dst_udp*/, struct SC_PARAMS */*sp*/, double /*fxscale*/, double /*fyscale*/));
PROTO(static long _IpsScaleBitonalSpecial, (struct SC_PARAMS */*sp*/, int /*options*/));
PROTO(static long *detail_h, (long */*map*/, long */*lst*/));
PROTO(static long *shrink_h, (long */*map*/, long */*lst*/));
PROTO(static long *expand_h, (long */*map*/, long */*lst*/));
PROTO(static struct UDP *retain_v, (struct SC_PARAMS */*sp*/));
PROTO(static struct UDP *detail_v, (struct SC_PARAMS */*sp*/));
PROTO(static struct UDP *shrink_v, (struct SC_PARAMS */*sp*/));
PROTO(static struct UDP *expand_v, (struct SC_PARAMS */*sp*/));
PROTO(static struct SC_PARAMS *analyze_src, (struct SC_PARAMS */*sp*/));
PROTO(static long *extract_dst, (struct SC_PARAMS */*sp*/));
#endif

/*
**  External References from VMS RTL             <- usage ->
*/

/*
**  Local Storage
**
**  Look-up tables for vertical scaling option: IpsM_SaveVertical
**
**  Source CHANGELISTs are processed in groups, by 'analyze_src', to create a 
**  destination CHANGELIST, via 'extract_dst', which can then be horizontally 
**  scaled and written to the destination image.
**
**  Each SRC group consists of the N source scanlines required to make up a DST 
**  scanline.  An array of VPOINT structures is treated as a linked list called
**  the VLIST.  The array elements represent vertical columns of the SRC bitmap 
**  bracketed by column -1 (left of left) and X+1 (right of right).  The VLIST 
**  forms a sliding window which covers two groups of scanlines at a time.
**  Each VLIST element holds a 'flink' which links it to the next "active" 
**  column, and a 7 bit 'window' consisting of 3 fields:
**
**        +--6--+--5--+--4--+--3--+--2--+--1--+--0--+
**        |       NEXT      |     CURRENT     | OUT |
**        |< N ><N...1>< 0 >|< N ><N...1>< 0 >|     |
**        +-----+-----+-----+-----+-----+-----+-----+
**   - OUT	DST pixel polarity OUTput for the previous DST CHANGELIST,
**   - CURRENT  SRC group data for the CURRENT DST CHANGELIST being created,
**   - NEXT	SRC group data for the NEXT DST CHANGELIST to create.
**
**  The CURRENT and NEXT fields each consist of three bits (right to left):
**   - initial polarity of the group (ie. pixel polarity of CHANGELIST 0),
**   - first transition polarity within the group
**				     (ie. found in any CHANGELIST 1 thru N),
**   - final polarity of the group   (ie. pixel polarity of CHANGELIST N).
**	(If a group consists of a single CHANGELIST or no vertical transitions
**	      are encountered, all three bits will equal the initial polarity.)
**
**  Routine 'analyze_src' processes the NEXT SRC group.  Any newly "active"
**  column is linked in, and the contents of 'window' are used as an index
**  to look-up table 'next_0' or 'next_1' depending on whether the pixel
**  at the column being processed is a 0 or 1 respectively.  Only the NEXT
**  field is affected by the look-up as follows:
**
**	  previous  NEXT from	NEXT from
**	    NEXT      next_0	  next_1
**	    000		000	    110
**	    001		001	    101
**	    010		010	    110
**	    011		000	    111	    011 and 100 are initial values set
**	    100		000	    111	     by 'extract_dst' (see below).
**	    101		001	    101
**	    110		010	    110
**	    111		001	    111
**
**	(Entries containing  NIL in the following tables are never accessed.)
*/

/*
**  0 bit read from next SRC CHANGELIST
*/
#if defined(__VAXC) || defined(VAXC)
static readonly long next_0[] = {
#else
static long next_0[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
0x00,0x01,0x02,0x03,0x04,0x05, NIL,0x0F,0x00, NIL,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15, NIL, NIL, NIL, NIL,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25, NIL, NIL, NIL, NIL,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x00,0x01,0x02,0x03,0x04,0x05, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,
 NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15, NIL, NIL, NIL, NIL,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25, NIL, NIL, NIL, NIL,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x10,0x11,0x12,0x13,0x14,0x15, NIL, NIL, NIL, NIL,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
				};

/*
**  1 bit read from next SRC CHANGELIST
*/
#if defined(__VAXC) || defined(VAXC)
static readonly long next_1[] = {
#else
static long next_1[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
0x60,0x61,0x62,0x63,0x64,0x65, NIL,0x7F,0x70, NIL,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x50,0x51,0x52,0x53,0x54,0x55, NIL, NIL, NIL, NIL,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65, NIL, NIL, NIL, NIL,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,
 NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x50,0x51,0x52,0x53,0x54,0x55, NIL, NIL, NIL, NIL,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65, NIL, NIL, NIL, NIL,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75, NIL, NIL, NIL, NIL,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
				};
/*
**  Routine 'extract_dst' re-initializes each "active" VLIST element by using
**  the contents of 'window' as the index to the look-up table requested by the
**  state of option flags IpsM_DisablePreference and IpsM_ReversePreference
**  (the default PREFERENCE is '1', the usual foreground polarity).  The choice
**  of look-up tables and their contents implements the following algorithm to 
**  determine the new value for OUT:
**
**	IF (NOT( IpsM_DisablePreference )
**	    AND( initial polarity of CURRENT equals OUT equals PREFERENCE )
**	    AND( there follows a SINGLE transition )
**	    AND( initial polarity of NEXT is not PREFERENCE )
**
**	    THEN    OUT will be of the PREFERENCE polarity,
**
**	    ELSE IF ( CURRENT contains any pixel not equal to OUT )
**
**		 THEN	complement OUT,
**
**		 ELSE	OUT remains unchanged.
**
**  Besides choosing the polarity of OUT, the table look-up also copies NEXT 
**  into CURRENT, and re-initializes NEXT to the special value, 011 or 100, 
**  depending on whether the 'final polarity' of NEXT had been 0 or 1.

**  If the new window state of a column is identical to the window state of 
**  the nearest "active" column to its left, the column is redundant, and 
**  therefore is unlinked within the VLIST (making it not "active").
*/

/*
**  Decision table: PREFERENCE = 0
*/
#if defined(__VAXC) || defined(VAXC)
static readonly long prefer_0[] = {
#else
static long prefer_0[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x30,0x31,0x30, NIL,ALL1,ALL0, NIL,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32, NIL, NIL, NIL, NIL,0x33,0x32,0x32,0x32,0x33,0x33,
0x34,0x34,0x35,0x34,0x35,0x34, NIL, NIL, NIL, NIL,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL0,ALL1,ALL0, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,
 NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,ALL1,ALL0,ALL0,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A, NIL, NIL, NIL, NIL,0x4B,0x4A,0x4A,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4C,0x4D,0x4C, NIL, NIL, NIL, NIL,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E, NIL, NIL, NIL, NIL,0x4F,0x4E,0x4E,0x4E,0x4F,ALL1
				};

/*
**  Decision table: PREFERENCE = 1
*/
#if defined(__VAXC) || defined(VAXC)
static readonly long prefer_1[] = {
#else
static long prefer_1[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x31,0x31,0x30, NIL,ALL1,ALL0, NIL,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32, NIL, NIL, NIL, NIL,0x33,0x32,0x33,0x32,0x33,0x33,
0x34,0x34,0x35,0x35,0x35,0x34, NIL, NIL, NIL, NIL,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL1,ALL1,ALL0, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,
 NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,ALL1,ALL0,ALL1,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A, NIL, NIL, NIL, NIL,0x4B,0x4A,0x4B,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4D,0x4D,0x4C, NIL, NIL, NIL, NIL,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E, NIL, NIL, NIL, NIL,0x4F,0x4E,0x4F,0x4E,0x4F,ALL1
				};

/*
**  Decision table: treat 0 and 1 equally
*/
#if defined(__VAXC) || defined(VAXC)
static readonly long prefer_both[] = {
#else
static long prefer_both[] = {
#endif
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E   F*/
ALL0,0x30,0x31,0x30,0x31,0x30, NIL,ALL1,ALL0, NIL,0x31,0x30,0x31,0x30,0x31,0x31,
0x32,0x32,0x33,0x32,0x33,0x32, NIL, NIL, NIL, NIL,0x33,0x32,0x33,0x32,0x33,0x33,
0x34,0x34,0x35,0x34,0x35,0x34, NIL, NIL, NIL, NIL,0x35,0x34,0x35,0x34,0x35,0x35,
ALL0,ALL0,ALL1,ALL0,ALL1,ALL0, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,
 NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL,ALL1,ALL0,ALL1,ALL0,ALL1,ALL1,
0x4A,0x4A,0x4B,0x4A,0x4B,0x4A, NIL, NIL, NIL, NIL,0x4B,0x4A,0x4B,0x4A,0x4B,0x4B,
0x4C,0x4C,0x4D,0x4C,0x4D,0x4C, NIL, NIL, NIL, NIL,0x4D,0x4C,0x4D,0x4C,0x4D,0x4D,
0x4E,0x4E,0x4F,0x4E,0x4F,0x4E, NIL, NIL, NIL, NIL,0x4F,0x4E,0x4F,0x4E,0x4F,ALL1
				};

#ifdef	DEBUG_SCALE
    long	c_num, v_num;	/* CHANGELIST and VLIST numbers for DEBUG print out */
#endif

/*
**  IpsScaleBitonal - SCALE bitonal image
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scales a bitonal image 
**
**  FORMAL PARAMETERS:
**
**	sp - structure pointer to scaling parameters and src frame info
**	options	- optional scaling modifier flags
**		    bit  <0>	= IpsM_SaveVertical
**		    bit  <1>	= IpsM_SaveHorizontal
**		    bit  <2>	= IpsM_ReversePreference
**		    bit  <3>	= IpsM_DisablePreference
**		    bit  <4,5>	= continuous tone flags
**		    bits <31..6>   ( unused )
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
**      UDP pointer of destination frame
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
long _IpsScaleBitonal (src_udp, dst_udp, fxscale, fyscale, options)
struct UDP *src_udp;
struct UDP *dst_udp;
float fxscale, fyscale;
int options;

{
long dst_npx;
long dst_nsl;
struct SC_PARAMS sp_area;
struct SC_PARAMS *sp = &sp_area;
unsigned long size;
long	status;

/*
** validate source class and type
*/

switch (src_udp->UdpB_Class)
    {
    case UdpK_ClassUBA:
	switch (src_udp->UdpB_DType)
	    {
	    case UdpK_DTypeVU: break;

	    case UdpK_DTypeBU:
	    case UdpK_DTypeWU:
	    case UdpK_DTypeLU:
	    case UdpK_DTypeF:
	    case UdpK_DTypeV:
	    default: return(IpsX_UNSOPTION); break;
	    }; break;

    case UdpK_ClassA:
    case UdpK_ClassUBS:
    case UdpK_ClassCL:
    default: 
	return(IpsX_UNSOPTION); 
    break;
    };

/*
** Initialize destination UDP fixed fields & data buffer
*/

dst_udp->UdpB_DType = src_udp->UdpB_DType;
dst_udp->UdpB_Class = src_udp->UdpB_Class;
dst_udp->UdpW_PixelLength = src_udp->UdpW_PixelLength;
dst_udp->UdpL_Levels = src_udp->UdpL_Levels;
dst_udp->UdpL_PxlStride = src_udp->UdpL_PxlStride;
dst_udp->UdpL_CompIdx = src_udp->UdpL_CompIdx;

dst_npx = src_udp->UdpL_PxlPerScn * fxscale;
dst_nsl = src_udp->UdpL_ScnCnt * fyscale;

dst_udp->UdpL_X1 = 0;
dst_udp->UdpL_Y1 = 0;
dst_udp->UdpL_X2 = dst_npx - 1;
dst_udp->UdpL_Y2 = dst_nsl - 1;
dst_udp->UdpL_ScnStride = dst_npx * dst_udp->UdpL_PxlStride;

/* pad out so that scanlines are byte aligned */
dst_udp->UdpL_ScnStride += ((8 - (dst_npx % 8))) % 8;
dst_udp->UdpL_PxlPerScn = dst_npx;
dst_udp->UdpL_ScnCnt    = dst_nsl;
size = dst_udp->UdpL_ScnCnt * dst_udp->UdpL_ScnStride;

if (dst_udp->UdpA_Base != 0)
    {
    /* No in place allowed */
    status = _IpsVerifyNotInPlace (src_udp, dst_udp);
    if (status != IpsX_SUCCESS) 
        return (status);
    if ((dst_udp->UdpL_ArSize - dst_udp->UdpL_Pos) < size)
	return (IpsX_INSVIRMEM);
    }
else
    {
    dst_udp->UdpA_Base = (unsigned char *)
	(*IpsA_MemoryTable[IpsK_AllocateDataPlane])
	    (((size + 7) >> 3), IpsM_InitMem, 0);
    if (!dst_udp->UdpA_Base) return (IpsX_INSVIRMEM);
    dst_udp->UdpL_ArSize = size;
    dst_udp->UdpL_Pos = 0;
    }

/*
** conform from version 3.0 interface to old (sp)
*/
load_sp (src_udp, dst_udp, sp, fxscale, fyscale);

/*
** Dispatch to layer 2 interpolation scale functions
*/

return(_IpsScaleBitonalSpecial(sp,options)); 

}

/* 
** This routine is used to convert the Version 3.0 arguments to that of Version
** 2.0
*/
static void load_sp (src_udp, dst_udp, sp, fxscale, fyscale)
struct UDP *src_udp;
struct UDP *dst_udp;
struct SC_PARAMS *sp;
float fxscale, fyscale;
{
    sp->x_scale = fxscale;
    sp->y_scale = fyscale;

    /*
    **	Calculate source and destination image dimensions.
    */
    sp->s_xcnt = src_udp->UdpL_PxlPerScn;
    sp->d_xcnt = dst_udp->UdpL_PxlPerScn;
    sp->s_ycnt = src_udp->UdpL_ScnCnt;
    sp->d_ycnt = dst_udp->UdpL_ScnCnt;

    /*
    **	Set initial src and dst pixel offsets.
    **	Stash working UDP descriptor pointers.
    */
    sp->s_pos = src_udp->UdpL_Pos + src_udp->UdpL_ScnStride * 
	src_udp->UdpL_Y1 + src_udp->UdpL_X1;
    sp->d_pos = dst_udp->UdpL_Pos;
    sp->s = src_udp;
    sp->d = dst_udp;

    /*
    **	Allocate working storage:
    **	    - space for the pre-scale X position array,
    **	    - space for the CHANGELIST array,
    **	    - space for a pair of VLIST arrays.
    */
    sp->size  = sp->s_xcnt + 2;		    /* size of scanline work spaces */
    sp->x_map = (long *) (*IpsA_MemoryTable[IpsK_Alloc])
		( sp->size * sizeof(long)		/*1 pre-scaled map */
		+ sp->size * sizeof(long)		/* 1 CHANGELIST	    */
		+ sp->size * sizeof(struct VPOINT),0,0);/* 1 VLIST	    */
    sp->h_run = (long*)( sp->x_map + sp->size );		/* CHANGELIST	    */
    sp->v_run = (struct VPOINT*)(sp->h_run + sp->size);	/* VLIST	    */

}


static long _IpsScaleBitonalSpecial(sp, options)
struct SC_PARAMS *sp;
int options;
{
    long    *ptmp, tmp;		/* temps				    */
    PF_UDP v_scale;

#ifdef  DEBUG_SCALE
c_num = 0;
v_num = 0;
#endif

    /*
    **	If there is horizontal scaling to do, map source X positions to 
    **	pre-scaled destination positions.
    */
    if( sp->d_xcnt != sp->s_xcnt )
	for( ptmp = sp->x_map, tmp = 0; tmp <= sp->s_xcnt; ++ptmp, ++tmp )
	    *ptmp = tmp * sp->x_scale;

   

    /*
    **	Select horizontal scaling function by comparing SRC and DST widths.
    **	If DST is smaller we also check for the SAVE_HORIZONTAL option.
    */
    sp->horizontal = (PF_UDP) ZERO_TEST_( sp->d_xcnt - sp->s_xcnt,
			 ( options & IpsM_SaveHorizontal ?
				detail_h :  /* shrink and save X detail	    */
				shrink_h ), /* shrink X, allow discard	    */
				0,	    /* no horizontal scaling	    */
				expand_h ); /* expand X run lengths	    */

    /*
    **	Select vertical scaling polarity PREFERENCE.
    */
    if( options & IpsM_DisablePreference )
	sp->v_table = prefer_both;
    else if( options & IpsM_ReversePreference )
	sp->v_table = prefer_0;
    else
	sp->v_table = prefer_1;


    /*
    **	Select vertical scaling function by comparing SRC and DST heights.
    **	If DST is smaller we also check for the SAVE_VERTICAL option.
    **  
    **	And call the vertical scaling function
    **	    (from which the horizontal scaling function is called)
    **	then return the scaled frame identifier.
    */
    v_scale = (PF_UDP) ZERO_TEST_( sp->d_ycnt - sp->s_ycnt,
		   ( options & IpsM_SaveVertical ?
			detail_v :	    /* shrink and save Y detail     */
			shrink_v ),	    /* shrink Y, allow discard      */
			retain_v,	    /* no vertical scaling	    */
			expand_v );	    /* expand Y run lengths         */

    sp->d = (*v_scale)(sp);

    return (IpsX_SUCCESS);
}

/*
**  detail_h
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale down CHANGELIST transition points (i.e. shrink scanline width).
**
**	When scaling down, runs may shrink out of existance (black or white).
**	This results in the disappearance of thin vertical lines.
**	Under these circumstances, for the sake of retaining detail, we will
**	borrow a pixel from the beginning of the next run, if the combined 
**	scaled length of the current run and next run will result in at least 
**	two pixels.  Otherwise we will discard the current run or the next
**	run depending on run length (a run is a pair of transition points).
**
**  FORMAL PARAMETERS:
**
**	map	- address of pre-scaled transition point array
**	lst	- address of CHANGELIST to scale
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST passed in
**
*****************************************************************************/
static long *detail_h( map, lst )
long	    *map;		    /* array of pre-scaled transition points*/
long	    *lst;		    /* CHANGELIST to scale		    */

{
    long	     beg,		    /* scaled begin point of current run    */
	     end,		    /* scaled end point of current run	    */
	     cnt = *lst,	    /* number of transitions in CHANGELIST  */
	    *cr  =  lst + 1,	    /* current run pointer		    */
	    *nr  =  lst + 1;	    /* next run pointer			    */

    beg = map[*nr++];		    /* scale begin of initial run of ones   */

    while( --cnt > 0 )		    /* if at least a pair of points exists  */
	{
	end = map[*nr++];		/* scale end of current run	    */
	if( beg < end )			/* if run is at least one pixel long*/
	    *cr++ = beg;		/* ...save scaled begin point	    */

	else if( --cnt > 0 )		/* else, if another point exists    */
	    {
	    end = map[*nr++];		/* ...scale end of next run	    */
	    if( end - beg >= 2 )	/* if combined length of current    */
		{			/* ...and next run is >= 2 pixels   */
		*cr++ = beg;		/* ...save single pixel run	    */
		*cr++ = beg + 1;	/* ...and begin of next run	    */
		}
	    else if(  nr[-1] - nr[-2]	/* else, if next run is shorter	    */
		   <= nr[-2] - nr[-3] )	/* ...than its predecessor	    */
		continue;		/* ...discard the next run	    */
	    }				/* (else discard current run)	    */

	beg = end;			/* current end point is next begin  */
	}

    *cr = beg;			    /* scale scanline length		    */
    *lst = cr - lst;		    /* update size of CHANGELIST	    */
    return( lst );		    /* return updated CHANGELIST	    */
}

/*
**  shrink_h
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale down CHANGELIST transition points (i.e. shrink scanline width).
**
**  FORMAL PARAMETERS:
**
**	map	- address of pre-scaled transition point array
**	lst	- address of CHANGELIST to scale
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST passed in
**
*****************************************************************************/
static long *shrink_h( map, lst )
long	    *map;		    /* array of pre-scaled transition points*/
long	    *lst;		    /* CHANGELIST to scale		    */
{
    long	     beg,		    /* scaled begin point of current run    */
	     end,		    /* scaled end point of current run	    */
	     cnt = *lst,	    /* number of transitions in CHANGELIST  */
	    *cr  =  lst + 1,	    /* current run pointer		    */
	    *nr  =  lst + 1;	    /* next run pointer			    */

    beg = map[*nr++];		    /* scale begin of initial run of ones   */

    while( --cnt > 0 )		    /* if at least a pair of points exists  */
	{
	end = map[*nr++];		/* scale end of current run	    */

	if( beg < end )			/* if run is at least one pixel	    */
	    *cr++ = beg;		/* ...save scaled begin point	    */
	else				/* else discard current run.	    */
	    if( --cnt > 0 )		    /* if another point exists	    */
		end = map[*nr++];	    /* ...scale it		    */

	beg = end;			/* current end point is next begin  */
	}

    *cr = beg;			    /* scale scanline length		    */
    *lst = cr - lst;		    /* update size of CHANGELIST	    */
    return( lst );		    /* return updated CHANGELIST	    */
}

/*
**  expand_h
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale up CHANGELIST transition points (i.e. expand scanline width).
**
**  FORMAL PARAMETERS:
**
**	map	- address of pre-scaled transition point array
**	lst	- address of CHANGELIST to scale
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST passed in
**
*****************************************************************************/
static long *expand_h( map, lst )
long	    *map;		    /* array of pre-scaled transition points*/
long	    *lst;		    /* CHANGELIST to scale		    */
{
    long	     i,			    /* point counter			    */
	    *cr;		    /* current pointer			    */

    /*
    **	Scale up CHANGELIST transition points.
    */
    for( cr = lst, i = *cr++; i > 0; ++cr, --i )
	*cr = map[*cr];				/* scale current point	    */

    return( lst );				/* return CHANGELIST	    */
}

/*
**  retain_v
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale (or copy) image: retain vertical size, scale horizontally only.
**
**  FORMAL PARAMETERS:
**
**	sp  - address of scale parameter data
**
**  FUNCTION VALUE:
**
**	dst - address of destination image UDP from scale parameter data.
**
*****************************************************************************/
static struct UDP	*retain_v( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */
    {
    long status = IpsX_SUCCESS;

    if( sp->horizontal == 0 )
	{
	/* no scaling required, just copy   */
	_IpsCopyData( sp->s, sp->d );
        }
    else
	for( sp->d_lin = 0;
		    ++sp->d_lin <= sp->d_ycnt;
			    sp->s_pos += sp->s->UdpL_ScnStride,
			    sp->d_pos += sp->d->UdpL_ScnStride )
	    if(*_IpsBuildChangelist(sp->s->UdpA_Base, sp->s_pos,
				      sp->s_xcnt, sp->h_run, sp->size) != BLANK)
		_IpsPutRuns( sp->d->UdpA_Base, sp->d_pos,
			     (*sp->horizontal)( sp->x_map, sp->h_run ));

    /* deallocate work area	    */
    (*IpsA_MemoryTable[IpsK_Dealloc])( sp->x_map ); 
    return( sp->d );
    }

/*
**  detail_v
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale image vertically by locating and scaling vertical runs.
**
**	Vertical runs may shrink out of existance (e.g. thin horizontal lines,
**	black or white).  To prevent the loss of such detail we will detect
**	vertical transitions within the group of source scanlines which will
**	become the next destination scanline.  Any transition will result in
**	a destination pixel polarity change from that of the pixel in the
**	same column on the preceeding destination scanline.  If more than
**	two transitions occur the additional detail must be discarded.
**
**  FORMAL PARAMETERS:
**
**	sp  - address of scale parameter data
**
**  FUNCTION VALUE:
**
**	dst - address of destination image UDP from scale parameter data.
**
*****************************************************************************/
static struct UDP	*detail_v( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */

{
    /*
    **  Initialize VLIST to contain single "end of scanline" element.
    **	- The first VLIST element is initialized as an imaginary SRC scanline
    **	  column left of the left edge (ie. it's treated as VLIST[-1] ).
    **	- The last VLIST element is initialized as an imaginary SRC scanline 
    **	  column right of the right edge (ie. it's treated as VLIST[N+1] ).
    */
					    /* link VLIST[-1] to VLIST[N+1] */
    sp->v_run->flink = (long) (sp->v_run + sp->s_xcnt + 1);
    sp->v_run->window = ALL0;		    /* preset window for VLIST[-1]  */
    sp->v_run[sp->s_xcnt + 1] = *sp->v_run; /* link VLIST[N+1] to itself    */

    /*
    **  Process the remaining src scanlines and write dst scanlines.
    */
    for( sp->s_lin = 0, sp->d_lin = 0, extract_dst( analyze_src( sp ));
		--sp->d_ycnt > 0;
			sp->d_pos += sp->d->UdpL_ScnStride )
	if( *extract_dst( analyze_src( sp )) != BLANK )
	    {
	    if( sp->horizontal != 0 )
		(*sp->horizontal)( sp->x_map, sp->h_run );
	    _IpsPutRuns( sp->d->UdpA_Base, sp->d_pos, sp->h_run );
	    }

    /*
    **  Extract and write the final dst scanline.
    */
    if( *extract_dst( sp ) != BLANK )
	{
	if( sp->horizontal != 0 )
	    (*sp->horizontal)( sp->x_map, sp->h_run );
	_IpsPutRuns( sp->d->UdpA_Base, sp->d_pos, sp->h_run );
	}

    (*IpsA_MemoryTable[IpsK_Dealloc])(sp->x_map );/* deallocate work area */
    return( sp->d );
}

/*
**  shrink_v
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale image vertically by using only every (1/y_scale)th scanline.
**
**  FORMAL PARAMETERS:
**
**	sp  - address of scale parameter data
**
**  FUNCTION VALUE:
**
**	dst - address of destination image UDP from scale parameter data.
**
*****************************************************************************/
static struct UDP	*shrink_v( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */
{
    long offset;

    offset = sp->s->UdpL_ScnStride * sp->s->UdpL_Y1 + sp->s->UdpL_X1;
    for( sp->d_lin = 0; ++sp->d_lin <= sp->d_ycnt;
		sp->d_pos += sp->d->UdpL_ScnStride,
		sp->s_pos  = sp->s->UdpL_ScnStride * 
		    (long)( sp->d_lin / sp->y_scale ) + offset)
	if( *_IpsBuildChangelist( sp->s->UdpA_Base, sp->s_pos,
				    sp->s_xcnt, sp->h_run, sp->size ) != BLANK )
	    {
	    if( sp->horizontal != 0 )
		(*sp->horizontal)( sp->x_map, sp->h_run );
	    _IpsPutRuns( sp->d->UdpA_Base, sp->d_pos, sp->h_run );
	    }

    (*IpsA_MemoryTable[IpsK_Dealloc])(sp->x_map );/* deallocate work area */
    return( sp->d );
}

/*
**  expand_v
**
**  FUNCTIONAL DESCRIPTION:
**
**	Scale image vertically by replicating each scanline y_scale times.
**
**  FORMAL PARAMETERS:
**
**	sp  - address of scale parameter data
**
**  FUNCTION VALUE:
**
**	dst - address of destination image UDP from scale parameter data.
**
*****************************************************************************/
static struct UDP	*expand_v( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */
{
    long end_pos;

    for(sp->d_lin = 0; ++sp->d_lin <= sp->s_ycnt; sp->s_pos += sp->s->UdpL_ScnStride)
	{
	end_pos = (long)( sp->d_lin * sp->y_scale ) * sp->d->UdpL_ScnStride
						   + sp->d->UdpL_Pos;
	if( *_IpsBuildChangelist( sp->s->UdpA_Base, sp->s_pos,
				    sp->s_xcnt, sp->h_run, sp->size ) != BLANK )
	    {
	    if( sp->horizontal != 0 )
		(*sp->horizontal)( sp->x_map, sp->h_run );
	    for( ; sp->d_pos < end_pos; sp->d_pos += sp->d->UdpL_ScnStride )
		_IpsPutRuns( sp->d->UdpA_Base, sp->d_pos, sp->h_run );
	    }
	else
	    sp->d_pos = end_pos;
    	}
    (*IpsA_MemoryTable[IpsK_Dealloc])(sp->x_map );/* deallocate work area */
    return( sp->d );
}

/*
**  analyze_src
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is responsible for detecting polarity changes in each 
**	column of pixels by analyzing the NEXT group of SRC CHANGELISTs.
**	New "active" columns are linked in, then the contents of 'window' are 
**	used as an index to look-up table 'next_0' or 'next_1' depending on 
**	whether the pixel at the column being processed is a '0' or '1' 
**	respectively.  Only the NEXT field is actually affected by the look-up.
**
**  FORMAL PARAMETERS:
**
**	sp  - address of scale parameter data.
**
**  FUNCTION VALUE:
**
**	sp  - address of scale parameter data as passed in.
**
*****************************************************************************/
static struct SC_PARAMS *analyze_src( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */
{
    long
	append,			    /* number of src scanlines to append    */
	*clst = sp->h_run,	    /* address of CHANGELIST		    */
	cind,			    /* index into CHANGELIST		    */
	vold;			    /* window state from previous scanline  */
    struct VPOINT
	*cnxt,			    /* next CHANGELIST column within VLIST  */
	*v0 = sp->v_run + 1,	    /* VLIST pointer to column 0	    */
	*vcur,			    /* current VLIST column		    */
    	*vnxt,			    /* next active VLIST column		    */
	*vend = v0 + sp->s_xcnt;    /* end of VLIST			    */

    for(append = ++sp->d_lin / sp->y_scale - sp->s_lin, sp->s_lin += append;
		append > 0;
			sp->s_pos += sp->s->UdpL_ScnStride, --append )
	{
	_IpsBuildChangelist( sp->s->UdpA_Base, sp->s_pos, sp->s_xcnt,
						  sp->h_run, sp->size );
	vcur = sp->v_run;
    	vnxt = (struct VPOINT *) vcur->flink;
	vold = vcur->window;
	cind =  1;
	cnxt = v0 + clst[cind++];

	while( vnxt < vend || cnxt < vend )
	    {
	    if( vnxt > cnxt )
		{			    /* CHANGELIST has next column   */
		vcur->flink = (long) cnxt;    /* link in new column	    */
		vcur = cnxt;
		vcur->flink = (long)vnxt;
		vcur->window = vold;	    /* state from previous scanline */
		cnxt = v0 + clst[cind++];   /* get next CHANGELIST column   */
		}
	    else
		{			    /* VLIST has next column	    */
		if( vnxt == cnxt )
		    cnxt = v0+clst[cind++]; /* get next CHANGELIST column   */
		vcur = vnxt;		    /* current column		    */
		vold = vcur->window;	    /* save state of prev scanline  */
		vnxt = (struct VPOINT *) vcur->flink;   /* next column	    */
		}
	    if(( cind & 1 ) == 0 )
		vcur->window = 
		    next_0[vcur->window];   /* update NEXT field with a 0   */
	    else
		vcur->window = 
		    next_1[vcur->window];   /* update NEXT field with a 1   */
	    }
#ifdef	DEBUG_SCALE
vdump( sp->v_run, v_num++ );
#endif
	}
    return( sp );
}

/*
**  extract_dst
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extracts a DST CHANGELIST from the VLIST window state and 
**	re-initializes the VLIST by unlinking redundant columns and updating
**	the window state of "active" columns.
**
**  FORMAL PARAMETERS:
**
**	sp	- address of scale parameter data
**
**  FUNCTION VALUE:
**
**	lst	- address of CHANGELIST from scale parameter data.
**
*****************************************************************************/
static long *extract_dst( sp )
struct SC_PARAMS *sp;			/* address of scale parameter data  */
{
    long
	*lst = sp->h_run,		/* address of CHANGELIST	    */
	cind = 1,		        /* index into CHANGELIST	    */
	vnew;				/* temporary VLIST window state	    */
    struct VPOINT
	*v0 = sp->v_run + 1,		/* VLIST pointer to column 0	    */
	*prev = sp->v_run,		/* previous active column	    */
    	*curr = (struct VPOINT *) prev->flink,	/* current column	    */
	*vend = v0 + sp->s_xcnt;	/* end of VLIST			    */

    while( curr < vend )
	{
	vnew = sp->v_table[curr->window];	/* choose new OUT, copy NEXT*/
						/*  to CURRENT, re-init NEXT*/
	if(( prev->window + vnew & OUT ) != 0 )	/* X polarity change ?	    */
	    lst[cind++] = curr - v0;		/*  create CHANGELIST entry */
	else if( prev->window == vnew )		/* is this column redundant?*/
	    {
	    curr = (struct VPOINT *) curr->flink;/* unlink current column    */
	    prev->flink = (long) curr;
	    continue;
	    }
	curr->window = vnew;			/* save new window state    */
	prev = curr;				/* link to next column	    */
	curr = (struct VPOINT *) curr->flink;
	}

    lst[cind] = curr - v0;			/* stash scanline length    */
    *lst = cind;				/* stash CHANGELIST size    */
#ifdef	DEBUG_SCALE
cdump( lst, c_num++, sp->v_run );
#endif
    return( lst );				/* return CHANGELIST	    */
}

#ifdef	DEBUG_SCALE
/*
**	Debug dump routines for VLISTs and CHANGELISTs.
**
*****************************************************************************/
static void dv( vlst )
struct VPOINT	*vlst;
{
    struct VPOINT *v;

    for( printf("\ncol:"), v = vlst;
	    v < v->flink;
		v = v->flink, printf("%3d", v-vlst-1 ));
    for( printf("\nwin:"), v = vlst;
	    v < v->flink;
		v = v->flink, printf(" %2X", v->window ));
    printf("\n");
}

static void cdump( clst, cnum, vlst )
long		*clst;
long		 cnum;
struct VPOINT	*vlst;
{
    long	*c, i;
    struct VPOINT *v;

    printf("\nCHANGELIST %3d (%3d ) ", cnum, *clst );
    for( c = clst, i = *c++; i > 0; ++c, --i )
	printf("%3d",*c);
    dv( vlst );
}

static void vdump( vlst, vnum )
struct VPOINT	*vlst;
long		 vnum;
{
    int	i;
    struct VPOINT *v;

    printf("\nSRC scanline %3d appended.", vnum );
    dv( vlst );
}
#endif
