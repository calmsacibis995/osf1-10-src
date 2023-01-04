
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
**  IMG_HISTOGRAM
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains the user level services that build and delete
**	histograms from source image frames.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson
**	John Poltrack
**	Revised for V3.0 by John Poltrack
**
**  CREATION DATE:
**
**	31-JAN-1988
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
*/
#if defined(__VMS) || defined(VMS)
struct HCB *IMG$CREATE_HISTOGRAM();    /* Create histogram context block    */
void	    IMG$DELETE_HISTOGRAM();    /* Delete histogram context block    */
#endif
#ifdef NODAS_PROTO
struct HCB *ImgCreateHistogram();      /* portable entry		    */
void	    ImgDeleteHistogram();      /* portable entry		    */
void		_ImgVerifyHcb();
#endif


/*
** Include files
*/
#include <img/ChfDef.h>			    /* Condition handler	    */
#include <img/ImgDef.h>			    /* Image definitions	    */
#include <ImgMacros.h>			    /* Common macro definitions	    */
#include <ImgVectorTable.h>		    /* Dispatch definitions	    */
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif
#include <math.h>			    /* C math functions		    */

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
unsigned long	ImgStandardizeFrame();
void		ImgVerifyFrame();		/* IMG_FRAME_UTILS	    */

void		 _ImgCfree();
struct FCT	*_ImgCheckNormal();		/* img__verify		    */
void		 _ImgErrorHandler();		/* img_error_handler	    */
struct FCT	*_ImgGet();			/* img__access_utils	    */
long		 _ImgGetVerifyStatus();
struct FCT	*_ImgPut();			/* img__access_utils	    */
struct UDP	*_ImgSetRoi();			/* img__roi		    */
long		 _ImgValidateRoi();		/* img_roi_utils	    */
long		 _ImgVerifyStandardFormat();	/* IMG__VERIFY_UTILS	    */

/*
** External references form Condition Handling Facility
*/
void	ChfStop();				/* Exception stop	*/
void	ChfSignal();				/* Exception signal	*/
#endif

/*
** Status codes
*/
#include <img/ImgStatusCodes.h>


/*****************************************************************************
**	    VMS  and Ultrix version 2 entry points                          **
*****************************************************************************/

#if defined(__VMS) || defined(VMS)
struct HCB *IMG$CREATE_HISTOGRAM(src_fid,roi,table_type,flags,component_idx )
    struct FCT	  *src_fid;	/* input frame to histogram		    */
    struct ROI	  *roi;		/* region of interest			    */
    unsigned long table_type;	/* implicit or explicit table for histogram */
    unsigned long flags;	/* if explicit, sort key		    */
    unsigned long component_idx;/* for multispectral, 0 is all components   */
    {
    return (ImgCreateHistogram(src_fid,roi,table_type,flags,component_idx));
    }
#endif

#if defined(__VMS) || defined(VMS)
void IMG$DELETE_HISTOGRAM(hcb)
    struct HCB  *hcb;
    {
    ImgDeleteHistogram(hcb);
    }
#endif
/*****************************************************************************
**		    END OF V2.0 ENTRY POINTS				    **
*****************************************************************************/

/******************************************************************************
**  ImgCreateHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate basic histogram structure and populate it using image data
**
**  FORMAL PARAMETERS:
**
**	src_fid	    Image frame identifier, by value. Longword value that 
**		    uniquely identifies the source image frame.
**
**	roi	    Region of interest identifier, by value. Longword value that
**		    uniquely identifies the region of interest to be processed.
**		    If zero, entire range is assumed.
**
**	table_type  ImgK_HtypeImplicitIndex, for array of longwords:
**		          histogram_table[pixel_value] = frequency
**
**		    ImgK_HtypeExplicitIndex, for array of paired longwords:
**			  histogram_table[0]   = pixel value
**			  histogram_table[1]   = frequency (of previous pixel)
**				.
**				.
**			  histogram_table[n-1] = pixel value
**			  histogram_table[n]   = frequency (of previous pixel)
**
**	flags	    ImgM_SortByFreq, to use frequency as primary key to sort
**		    explicit histogram table.
**
**	component_idx	Number of the component in an image with multiple
**			spectral components, by value. If zero all components
**			are tabulated as a single value.
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
**	histid	Returns histogram object identifier in a longword. Histid
**		is actually the address of the histogram context block (HCB).
**
**  SIGNAL CODES:
**
**	ImgX_UNSCSPORG	Unsupported component space organization type.
**	ImgX_PXLTOOLRG	Pixels greater than 25 bits not supported in this
**			implementation.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct HCB *ImgCreateHistogram(src_fid, roi, table_type, flags, component_idx)
    struct FCT	    *src_fid;		  /* input frame                    */
    struct ROI	    *roi;                 /* region of interest             */
    unsigned long    table_type;	  /* implicit or explicit table	    */
    unsigned long    flags;		  /* sort key on explicit table	    */
    unsigned long    component_idx;	  /* used on multi-spectral frames  */

{
struct FCT *std_fid;				  /* Standardized frame	    */

struct UDP src_udp[MAX_NUMBER_OF_COMPONENTS];	  /* source UDP descriptor  */
struct UDP merged_udp;				  /* special temporary udp  */
struct UDP *udp_ptr_list[MAX_NUMBER_OF_COMPONENTS]; /* source UDP pointer(s)*/
struct UDP *udp_ptr;				  /* layer 2 UDP pointer    */
struct HCB *hcb;				  /* output histogram ptr.  */

long	cs_org;				/* comp space organization	    */
long	num_of_comp;			/* number of spectral components    */
long	spectral_type;			/* spectral type		    */
long	status;				/* status of layer 2 routine	    */
long	idx;				/* scratch index variable	    */

unsigned long element_count;		/* number of table entries	    */
unsigned long size;			/* size for allocation routines	    */


/*****************************************************************************
**	    start of validation & conformance checking                      **
*****************************************************************************/
if ( VERIFY_ON_ )
    ImgVerifyFrame( src_fid, ImgM_NonstandardVerify );

status = _ImgVerifyStandardFormat( src_fid, ImgM_NoChf );
if ( !(status & 1) )
    std_fid = (struct FCT *) ImgStandardizeFrame( src_fid, 0 );
else
    std_fid = src_fid;

_ImgGet(std_fid,Img_ImageDataClass, &spectral_type,sizeof(spectral_type),0,0);
_ImgGet(std_fid,Img_CompSpaceOrg,   &cs_org,sizeof(cs_org),0,0);
_ImgGet(std_fid,Img_NumberOfComp,   &num_of_comp,sizeof(num_of_comp),0,0);

/*
** Validate Number of Components, Component Organization, Bits per Component
*/
if (component_idx > num_of_comp)
    ChfStop(1,ImgX_INVFLGARG);

switch (spectral_type)
    {
    case ImgK_ClassGreyscale:
    case ImgK_ClassMultispect:
       break;
    case ImgK_ClassBitonal:
    default:
      ChfStop(1,ImgX_UNSOPTION);
      break;
    };/* end switch */

for (idx=0;idx<num_of_comp;idx++)
    {
    /*
    ** Validate and set ROI for each component
    */
    _ImgGet(std_fid,Img_Udp,&src_udp[idx],sizeof(struct UDP),0,idx);

    if (roi != 0)
	{ 
	if (! _ImgValidateRoi(roi,&src_udp[idx]))
	    ChfStop(1,ImgX_INVROI);
	else
	    _ImgSetRoi(&src_udp[idx],roi);
	};

    };/* end for components */
/*****************************************************************************
**	    End ROI validation						    **
*****************************************************************************/


/*
** Allocate the basic histogram structures and fill in hcb structure
*/
size = sizeof(struct HCB);
hcb = (struct HCB*)
	    (*ImgA_VectorTable[ImgK_Alloc])(size,ImgM_InitMem,0);
if (!hcb)
    ChfStop(1,ImgX_INSVIRMEM);

/*
** fill in other parts of the structure
*/
hcb->HcbR_Blkhd.BhdB_Type = ImgK_BlktypHcb;
hcb->HcbL_TableType = table_type;
*((long*)&(hcb->HcbL_Flags)) = flags;	/* recast flags struct as a longword */
hcb->HcbL_ComponentIdx = component_idx;

if (component_idx != 0)
    {
    /*
    ** Point to the proper udp in preparation for layer 2 call
    */
    udp_ptr = &src_udp[component_idx -1];
    }
else
    {
    /*
    ** If frame is greyscale point to proper udp for layer 2 else
    ** the planes must be merged into a single udp 
    ** 
    */
    if (spectral_type == ImgK_ClassGreyscale)
	udp_ptr = &src_udp[0];
    else
	{
        /*
        ** Initialize merged udp for multi-spectal frames
        */
	merged_udp.UdpA_Base = 0;

        for (idx=0;idx<MAX_NUMBER_OF_COMPONENTS;idx++)
	    {
	    /*
	    ** Load array of pointers to multiple udp's for special case in which
	    ** spectral components are combined
	    */
	    udp_ptr_list[idx] = &src_udp[idx];
	    }	    

	status = (*ImgA_VectorTable[ImgK_MergePlanes])
	    (udp_ptr_list,&merged_udp,num_of_comp,0);
	if ((status & 1) != 1)
            _ImgErrorHandler(status);

	udp_ptr = &merged_udp;
	}/* end else (multi-spectral with component_idx = 0) */
    }/* end else (component_idx == 0) */


/*****************************************************************************
**	    Start Dispatch to layer 2 histogram routines		    **
*****************************************************************************/

switch( table_type )
    {
    case ImgK_HtypeExplicitIndex:
	{
	/*
	** determine which sort key to use
	*/
	unsigned long sort_key;
	if (flags & ImgM_SortByFreq)
	    sort_key = IpsK_ByFreq;
	else
	    sort_key = IpsK_ByPixelValue;

	status = (*ImgA_VectorTable[ImgK_HistogramSorted])
	    (udp_ptr,0,sort_key,&hcb->HcbA_TablePointer,&hcb->HcbL_EntryCount);

	if ((status & 1) != 1)
            _ImgErrorHandler(status);
	}
	break;
    case ImgK_HtypeImplicitIndex:
    default:
	status = (*ImgA_VectorTable[ImgK_Histogram])
		    (udp_ptr,0,&hcb->HcbA_TablePointer);
	if ((status & 1) != 1)
            _ImgErrorHandler(status);

	hcb->HcbL_EntryCount = udp_ptr->UdpL_Levels;
	break;
    }/* end switch */

/*
** Deallocate temporary merged udp base only if it allocated
*/
if ( (spectral_type == ImgK_ClassMultispect) &&
     (component_idx == 0) &&
     (merged_udp.UdpA_Base != 0) )
    {
    status = (*ImgA_VectorTable[ImgK_Dealloc])(merged_udp.UdpA_Base);
    }
return hcb;
}/* end of ImgCreateHistogram */
 
/******************************************************************************
**  ImgDeleteHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**      Delete a histogram context block established by a call to 
**	ImgCreateHistogram.
**	Deallocate the hcb and all local sub-structures
**	attached to it.
**
**  FORMAL PARAMETERS:
**
**	hcb	Histogram context block
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
**	void (none)
**
**  SIGNAL CODES:
**
**	IMG$_INVHSTGRM	Invalid histogram identifier.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void ImgDeleteHistogram( hcb )
struct HCB  *hcb;
{
/*
** Verify that this is really an HCB block
*/
_ImgVerifyHcb( hcb );

/*
** Delete the histogram table
*/
_ImgCfree( hcb->HcbA_TablePointer );

/*
** Delete the HCB block
*/
_ImgCfree( hcb );

return;
} /* end of ImgDeleteHistogram */


/******************************************************************************
**  _ImgVerifyHcb
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function verifies that the hcb address passed in is really
**	a valid hcb.
**
**  FORMAL PARAMETERS:
**
**	hcb	Histogram control block.  Passed by reference.
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
**	void (none)
**
**  SIGNAL CODES:
**
**	IMG$_INVHSTGRM	Invalid histogram identifier
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgVerifyHcb( hcb )
struct HCB  *hcb;
{
if ( hcb->HcbR_Blkhd.BhdB_Type != ImgK_BlktypHcb )
    ChfStop(ImgX_INVHSTGRM, 0);
return;
} /* end of _ImgVerifyHcb */
