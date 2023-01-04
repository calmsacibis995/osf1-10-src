
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**
**  ImgDitherFrame
**
**  FACILITY:
**
**      Image Services Library
**
**
**  ABSTRACT:
**
**      ISL user level dithering routine. 
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**
**  AUTHOR(S):
**
**      Ken MacDonald 
**	Richard Piccolo
**	Revised for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**	February 1988
**
*****************************************************************************/

#include <img/ChfDef.h>			    /* Condition handler	    */
#include <img/ImgDef.h>			    /* Image definitions	    */
#include <ImgDefP.h>			    /* ISL private definitions	    */
#include <ImgMacros.h>			    /* Common macro definitions	    */
#include <ImgVectorTable.h>		    /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
**  Table of contents
**
**	Global entry points
*/
#if defined(__VMS) || defined(VMS)
struct FCT	*IMG$DITHER();	       /* Dither routine VMS bindings       */
struct FCT	*IMG$DITHER_FRAME();   /* V3 Dither routine VMS bindings    */
#endif
#ifdef NODAS_PROTO
struct FCT      *ImgDither();	       /* Dither routine portable binding   */
struct FCT      *ImgDitherFrame();     /* V3 Dither routine portable binding*/
#endif

/*
**	Module local entry points
*/
#ifdef NODAS_PROTO
static long	*Set_output_levels();
#else
PROTO(static long *Set_output_levels, (struct FCT * /*fid*/, unsigned long */*output_levels*/, struct PUT_ITMLST */*itmlst*/));
#endif

/*
**  MACRO definitions -- none
*/

/*
**  Equated Symbols
*/

/*
**  External References from ISL                 <- from module ->
*/
#ifdef NODAS_PROTO
long	     ImgCvtCompSpaceOrg();
long	     ImgCreateFrame();
void	     ImgDeallocateFrame();	/* IMG_FRAME_UTILS		*/
long	     ImgGetFrameSize();		/* IMG_ATTRIBUTE_ACCESS_UTILS	*/
long	     ImgSetFrameSize();		/* IMG_ATTRIBUTE_ACCESS_UTILS	*/
long	     ImgSetRectRoi();		/* IMG_ROI_UTILS		*/
long	     ImgStandardizeFrame();	/* IMG_FRAME_UTILS		*/
long	     ImgUnsetRectRoi();		/* IMG_ROI_UTILS		*/
void	     ImgVerifyFrame();		/* IMG_FRAME_UTILS		*/

char	    *_ImgCalloc();
struct FCT  *_ImgCheckNormal();		/* img__verify_utils		*/
struct FCT  *_ImgCloneFrame();		/* img__frame_utils		*/
void	     _ImgErrorHandler();	/* img_error_handler		*/
long	     _ImgDitherBluenoise();	/* img__dither_bluenoise	*/
void	     _ImgFree();
struct FCT  *_ImgGet();			/* img__attribute_access_utils	*/
long	     _ImgGetVerifyStatus();
struct FCT  *_ImgPut();			/* img__attribute_access_utils	*/
struct UDP  *_ImgSetRoi();		/* img__roi			*/
long	     _ImgValidateRoi();		/* img_roi_utils		*/
long	     _ImgVerifyStandardFormat();/* IMG__VERIFY_UTILS		*/

/*
** External references form Condition Handling Facility
*/
void	ChfStop();				/* Exception stop	*/
void	ChfSignal();				/* Exception signal	*/
#endif

/*
**	Status codes
*/
#include <img/ImgStatusCodes.h>         /* ISL Status Codes             */



/*****************************************************************************
**	    VMS  and Ultrix version 2 entry points                          **
*****************************************************************************/

/*
** NOTE:
**	The itmlst parameter and flag parameter is accepted
**	(though ignored) by the two following calls
*/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$DITHER( src_fid, itmlst, dither_type, threshold, flags, roi,
			output_levels)

struct FCT	    *src_fid;		  /* input frame                    */
struct PUT_ITMLST   *itmlst;		  /* attrib. for new frame          */
unsigned long	    dither_type;          /* Type of dithering to perform   */
unsigned long	    threshold;		  /* dither matrix size or order    */
unsigned long	    flags;                /* special processing flags       */
struct ROI	    *roi;                 /* region of interest             */
unsigned long	    output_levels[];	  /* number of output levels / comp */
{
return (ImgDither( src_fid, itmlst, dither_type, threshold, flags, roi, 
		    output_levels ));
} /* end of IMG$DITHER */
#endif


struct FCT *ImgDither( src_fid, itmlst, dither_type, threshold, flags, roi,
			output_levels)

struct FCT	    *src_fid;		  /* input frame                    */
struct PUT_ITMLST   *itmlst;		  /* attrib. for new frame          */
unsigned long	    dither_type;          /* Type of dithering to perform   */
unsigned long	    threshold;  	  /* dither matrix size or order    */
unsigned long	    flags;                /* special processing flags       */
struct ROI	    *roi;                 /* region of interest             */
unsigned long	    output_levels[];	  /* number of output levels / comp */
{
long	*local_output_levels	= (long *) output_levels;
struct FCT	*ret_fid;
long	std_cs_org		= ImgK_BandIntrlvdByPlane;
long	saved_cs_org;
long	saved_data_class;
long	saved_dp_signif;
long	status;
struct FCT *tmp_fid_1;
struct FCT *tmp_fid_2;

/*
** Verify the source frame ...
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

/*
** Verify various combinations of input args ...
*/
if (( itmlst == 0) && (output_levels == 0))
    ChfStop( 1, ImgX_ATTLSTREQ );

/*
** Convert the source frame to standard format if necessary
*/
_ImgGet( src_fid, Img_ImageDataClass, &saved_data_class, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0, 0 );
_ImgGet( src_fid, Img_PlaneSignif, &saved_dp_signif, LONGSIZE, 0, 0 );
if ( (saved_data_class == ImgK_ClassBitonal ||
      saved_data_class == ImgK_ClassGreyscale ) &&
      saved_cs_org == ImgK_BandIntrlvdByPixel )
    {
    _ImgPut( src_fid, Img_CompSpaceOrg, &std_cs_org, LONGSIZE, 0 );
    }

status = _ImgVerifyStandardFormat( src_fid, ImgM_NoChf );
if ( !(status & 1) )
    tmp_fid_1 = ImgStandardizeFrame( src_fid, 0 );
else
    tmp_fid_1 = src_fid;

/*
** Set up local output levels if they weren't specified by the caller 
*/
local_output_levels = Set_output_levels( tmp_fid_1, output_levels, itmlst );

/*
** Call V3.0 entry point ...
*/
ImgSetRectRoi( tmp_fid_1, roi, 0 );
tmp_fid_2 = ImgDitherFrame( tmp_fid_1, dither_type, threshold, 
				(unsigned long *)local_output_levels, flags );
ImgUnsetRectRoi( tmp_fid_1 );
if ( local_output_levels != (long *) output_levels )
    _ImgFree( local_output_levels );

/*
** Convert Comp. Space Org of the resultant frame back to the original
** saved value if the original value was not band interleaved by plane.
*/
if ( saved_cs_org != ImgK_BandIntrlvdByPlane )
    {
    ret_fid = ImgCvtCompSpaceOrg( tmp_fid_2, saved_cs_org, saved_dp_signif, 0 );
    ImgDeallocateFrame( tmp_fid_2 );
    _ImgPut( src_fid, Img_CompSpaceOrg, &saved_cs_org, LONGSIZE, 0 );
    }
else
    ret_fid = tmp_fid_2;

if ( tmp_fid_1 != src_fid )
    ImgDeallocateFrame( tmp_fid_1 );

if ( VERIFY_ON_ )
    ImgVerifyFrame( ret_fid, ImgM_NonstandardVerify );

return (struct FCT *) ret_fid;
} /* end of ImgDither */


#if defined(__VMS) || defined(VMS)
struct FCT *IMG$DITHER_FRAME(src_fid,dither_type,threshold,output_levels,flags)
struct FCT	    *src_fid;		  /* input frame                    */
unsigned long	    dither_type;          /* Type of dithering to perform   */
unsigned long	    threshold;  	  /* dither matrix size or order    */
unsigned long	   *output_levels;	  /* number of output levels / comp */
unsigned long	    flags;
{

return (ImgDitherFrame( src_fid, dither_type, threshold, output_levels, flags));
} /* end of IMG$DITHER_FRAME */
#endif

/****************************************************************************
**  ImgDitherFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**      User entry point for bluenoise/ordered dithering routines. Grey scale 
**	images may be dithered by a variety of techniques into images 
**	of lesser depth (i.e. bits/pixel). This routine performs two types
**	of dither: bluenoise and ordered dot.  
**	For the ordered dither, there are two classes of dithering: clustered 
**	and dispersed dot dithering.  Within each class a unique threshold 
**	matrix is created based on a user specified parameter called the
**	threshold (see below table).  For clustered dithering this
**	specification is referred to as the "M" factor while for the dispersed
**	dithering this specification refers to the "order".
**	This specification will	define the size (period) and the number 
**	of grey levels rendered in the target image.  Larger threshold 
**	matrix sizes will decrease the low visibility of the image but 
**	will render more levels of grey.
**
**	DISPERSED DITHER
**	|------------------------------------------------------------------|
**	| THRESHOLD_SPEC (ORDER)  |  LEVELS (2**ORDER)+1)    |  MATRIX SIZE|
**	|------------------------------------------------------------------|
**	|     	   1	  	  |	       3	     |	  2 X 1    |
**	|------------------------------------------------------------------|
**	|     	   2	  	  |	       5	     |	  2 X 2	   |
**	|------------------------------------------------------------------|
**	|     	   3	          |	       9	     |	  4 X 2    |
**	|------------------------------------------------------------------|
**	|     	   4	  	  |	      17	     |	  4 X 4    |
**	|------------------------------------------------------------------|
**	|     	   5*	  	  |	      33	     |	  8 X 4    |
**	|------------------------------------------------------------------|
**	|     	   6		  |	      65	     |	  8 X 8	   |
**	|------------------------------------------------------------------|
**	|     	   7	  	  |	     129	     |	  16 X 8   |
**	|------------------------------------------------------------------|
**	|     	   8	  	  |	     257	     |	  16 X 16  |
**	|------------------------------------------------------------------|
**
**	CLUSTERD DITHER*
**	|------------------------------------------------------------------|
**	| THRESHOLD_SPEC (M)      |  LEVELS (2*M*M)+1)       |  MATRIX SIZE|
**	|------------------------------------------------------------------|
**	|          1		  |	       3	     |	  2 X 2    |
**	|------------------------------------------------------------------|
**	|     	   2		  |	       9	     |	  4 X 4    |
**	|------------------------------------------------------------------|
**	|     	   3		  |	       19	     |	  6 X 6	   |
**	|------------------------------------------------------------------|
**	|     	   4		  |	       33	     |	  8 X 8	   |
**	|------------------------------------------------------------------|
**	|          5*	  	  |	       51	     |	  10 X 10  |
**	|------------------------------------------------------------------|
**	|     	   6		  |	       73	     |	  12 X 12  |
**	|------------------------------------------------------------------|
**	|     	   7		  |	       99	     |	  14 X 14  |
**	|------------------------------------------------------------------|
**	|     	   8		  |	       129	     |	  16 X 16  |
**	|------------------------------------------------------------------|
**	|     	   9		  |	       163	     |	  18 X 18  |
**	|------------------------------------------------------------------|
**	|     	   10	  	  |	       201	     |	  20 X 20  |
**	|------------------------------------------------------------------|
**
**	* -> default.
**	Matrix size refers to the size of one period of the threshold matrix.
**
**	For bluenoise dithering the following error filter is used:
**
**	      (1/16) * |   origin 7 |
**		       |3    5    1 |    	
**		

**  FORMAL PARAMETERS:
**
**      fid            - frame ID of frame to be dithered
**      dither_type    - dithering algorithm to use [optional]
**	threshold      - used for ordered dither only - refers to the type
**			 of threshold matrix used when dithering.
**			 For clustered dithering (default) threshold 
**			 must be between 1 and 8 (default=5)and for dispersed
**			 dithering threshold spec must be between 1 and
**			 10 (default=5).
**	output_levels  - ptr to the number of output levels per comp [required]
**	flags		 no flags defined at present
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
**      retfid - frame id of created image frame
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	- invalid argument count
**
**  SIDE EFFECTS:
**
**	none
**
*****************************************************************************/
struct FCT *ImgDitherFrame( src_fid, dither_type, threshold, output_levels, 
			    flags)
struct FCT	    *src_fid;		  /* input frame                    */
unsigned long	    dither_type;          /* Type of dithering to perform   */
unsigned long	    threshold;	          /* dither matrix size or order    */
unsigned long	   *output_levels;	  /* number of output levels / comp */
unsigned long	    flags;
{
struct	FCT *dst_fid;				  /* Destination fid	    */
struct	UDP src_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Source UDP descriptor  */
struct	UDP dst_udp[MAX_NUMBER_OF_COMPONENTS];	  /* Dest   UDP descriptor  */

long	brt_polarity;			/* brightness polarity		    */
long	cs_org;				/* comp space organization	    */
long	num_of_comp;			/* number of spectral components    */
long	spectral_type;			/* spectral type		    */
long	status;				/* status of layer 2 routine	    */
float	x_size;
float	y_size;
unsigned long	private_flag = 0;	/* layer 2 flags		    */
unsigned long	idx;			/* scratch index variable	    */


/*****************************************************************************
**	    start of validation & conformance checking                      **
*****************************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, 0 );

_ImgGet(src_fid,Img_BrtPolarity,    &brt_polarity,sizeof(brt_polarity),0,0);
_ImgGet(src_fid,Img_ImageDataClass, &spectral_type,sizeof(spectral_type),0,0);
_ImgGet(src_fid,Img_CompSpaceOrg,   &cs_org,sizeof(cs_org),0,0);
_ImgGet(src_fid,Img_NumberOfComp,   &num_of_comp,sizeof(num_of_comp),0,0);

/*
** Validate Component Organization, Number of Components, Bits per Component
*/
switch (spectral_type)
    {
    case ImgK_ClassBitonal:
       ChfStop(1,ImgX_INVDTHBIT);
       break;
    case ImgK_ClassGreyscale:
       {
       /*
       ** If dithering to bitonal alert layer 2 routines
       */
       if (output_levels[0] == 2)
	   private_flag = IpsM_DitherToBitonal;
       };
       break;
    case ImgK_ClassMultispect:
       break;
    default:
      ChfStop(1,ImgX_UNSDATACL);
      break;
    };/* end switch */

/*
** Verify parameters and set default conditions
*/
if (dither_type == 0) 
    dither_type = ImgK_DitherClustered;
if (threshold == 0) 
    threshold = 5;

/*
** Check threshold based on dither type 
*/
switch (dither_type)
    {
    case ImgK_DitherClustered:
        if (threshold > 10 || threshold < 1 )
            ChfStop( 1, ImgX_INVTHRSPC);
    break;

    case ImgK_DitherDispersed:
	if (threshold < 1 || threshold > 8)
	    ChfStop( 1, ImgX_INVTHRSPC);
    break;

    case ImgK_DitherBluenoise:
    break;

    default:
	ChfStop ( 1, ImgX_INVDTHTYP);
    break;
    }

/*
** Validate output_levels and process source udp
*/
for (idx=0;idx<num_of_comp;idx++)
    {
    /*
    ** Validate and set ROI for each component
    */
    _ImgGet(src_fid,Img_Udp,&src_udp[idx],sizeof(struct UDP),0,idx);

    /*
    ** Verify that the output_levels parameter is valid
    */
    if (output_levels[idx] > src_udp[idx].UdpL_Levels ||
        output_levels[idx] < 1 )
	    ChfStop(1,ImgX_INVDTHLEV);

    }/* end for components */
/*****************************************************************************
**	    End ROI validation						    **
*****************************************************************************/


/*****************************************************************************
**	    Start Dispatch to layer 2 dither routines			    **
*****************************************************************************/

for (idx=0;idx<num_of_comp;idx++)
    {
    /*
    ** VERY IMPORTANT!!! Clear the base addr in the dst udp so that
    ** IPS will allocate the dst data plane.
    */
    dst_udp[idx].UdpA_Base = 0;

    /*
    ** Copy the UDP if the src and dst levels are the same ...
    */
    if ( output_levels[idx] == src_udp[idx].UdpL_Levels )
	{
	status = (*ImgA_VectorTable[ ImgK_Copy ])(
					     &src_udp[idx],
					     &dst_udp[idx],
					     0 );
	if ((status & 1) != 1)
	    _ImgErrorHandler(status);
	}
    /*
    ** Else, dither the image.
    */
    else
	{
	switch (dither_type)
	    {/* switch on dither_type */
	    case (ImgK_DitherClustered):
		status = (*ImgA_VectorTable[ImgK_OrderedDither])
	      	      (&src_udp[idx],&dst_udp[idx],dither_type,threshold,
		       output_levels[idx],private_flag);
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		break;

	    case (ImgK_DitherDispersed):
		status = (*ImgA_VectorTable[ImgK_OrderedDither])
	      	      (&src_udp[idx],&dst_udp[idx],dither_type,threshold,
		       output_levels[idx],private_flag);
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		break;

	    case (ImgK_DitherBluenoise):
		status = (*ImgA_VectorTable[ImgK_BluenoiseDither])
	      	      (&src_udp[idx],&dst_udp[idx],
		       output_levels[idx],private_flag);
		if ((status & 1) != 1)
		    _ImgErrorHandler(status);
		break;

	    default:
		ChfStop(1,ImgX_INVDTHTYP);	
		break;
	    }/* end switch on dither type */
	} /* end else */

    }/* end component loop */

/*****************************************************************************
**	    End dispatch to layer 2					    **
*****************************************************************************/

/*
** if dithering one component to 2 levels create a bitonal frame using original
** brightness polarity
*/
if ((output_levels[0] == 2) && (num_of_comp == 1))
    {
    dst_fid = (struct FCT*)ImgCreateFrame(0,ImgK_ClassBitonal);
    _ImgPut(dst_fid,Img_BrtPolarity,&brt_polarity,sizeof(brt_polarity),0);
    ImgGetFrameSize( src_fid, &x_size, &y_size, ImgK_Bmus );
    ImgSetFrameSize( dst_fid, &x_size, &y_size, ImgK_Bmus );
    }
else
    dst_fid = (struct FCT*)_ImgCloneFrame(src_fid);

for (idx=0;idx<num_of_comp;idx++)
    {
    /*
    ** Load destination udp into destination fid
    */
    _ImgPut(dst_fid,Img_Udp,&dst_udp[idx],sizeof(struct UDP),idx);
    _ImgPut(dst_fid,Img_QuantLevelsPerComp,&dst_udp[idx].UdpL_Levels,
	    sizeof(dst_udp[idx].UdpL_Levels),idx);
    }

/*
** Verify the return frame and return it.
*/
if ( VERIFY_ON_ )
    ImgVerifyFrame( dst_fid, 0 );

return dst_fid;
} /* end of ImgDither */


/******************************************************************************
**  Set_output_levels()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set up an output levels list using bits per comp values in a
**	user supplied itemlist if an output levels list was not supplied
**	directly.
**
**  FORMAL PARAMETERS:
**
**	fid		source frame identifier, passed by value
**	output_levels	output levels list of longword values, passed by ref.
**	itmlst		itemlist of user supplied attributes, passed by ref.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	ret_output_levels   list of output levels to be used by the dither
**			    algorithm. passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static long *Set_output_levels( fid, output_levels, itmlst )
struct FCT	    *fid;
unsigned long	    *output_levels;
struct PUT_ITMLST   *itmlst;
{
long	 bits_per_comp;
long	 comp_cnt;
long	 idx;
long	 levels_per_comp;
long	*ret_output_levels  = (long *)output_levels;

struct PUT_ITMLST   *local_itmlst   = itmlst;

if ( output_levels == 0 )
    {
    /*
    ** Allocate a new output levels list, and initialize it to the
    ** number of levels in the frame that was passed in.
    */
    _ImgGet( fid, Img_NumberOfComp, &comp_cnt, LONGSIZE, 0, 0 );
    ret_output_levels = (long *)_ImgCalloc( comp_cnt, LONGSIZE );
    for ( idx = 0; idx < comp_cnt; ++idx )
	{
	_ImgGet( fid, Img_QuantLevelsPerComp, &(ret_output_levels[idx]),
		    LONGSIZE, 0, idx );
	}

    /*
    ** Now loop through the itemlist looking for bits per component,
    ** and set the corresponding output levels value for each component
    ** whose bit cnt is specified.
    */
    do
	{
	if ( itmlst->PutL_Code == Img_BitsPerComp )
	    {
	    if ( itmlst->PutL_Index < comp_cnt )
		{
		bits_per_comp = *((long *)itmlst->PutA_Buffer);
		levels_per_comp = 1 << bits_per_comp;
		ret_output_levels[ itmlst->PutL_Index ] = levels_per_comp;
		}
	    }
	++itmlst;
	}
    while ( itmlst->PutL_Code != 0 );
    }

return ret_output_levels;
} /* end of Set_output_levels */
