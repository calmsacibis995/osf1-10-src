
/************************************************************************
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
************************************************************************/

/************************************************************************
**  IMG_FRAME_IO_MGT.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	The routines in this module provide generic frame import and
**	export capability.  They are layered on top of format specif
**	import and export functions.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark Sornson 
**
**  CREATION DATE:
**
**	7-MAY-1990
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	VMS specific global entry points
*/
#if defined(__VMS) || defined(VMS)
unsigned long	IMG$EXPORT_FRAME();
unsigned long	IMG$EXPORT_PAGE_BREAK();
unsigned long	IMG$IMPORT_FRAME();
#endif

/*
**	Portable global entry points
*/
#ifdef NODAS_PROTO
unsigned long	ImgExportFrame();
unsigned long	ImgExportPageBreak();
struct FCT	*ImgImportFrame();
#endif


/*
**  Include files:
*/
#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include    <cda$ptp.h>
#else
#if defined(NEW_CDA_CALLS)
#include    <cdaptp.h>
#else
#include    <cda_ptp.h>
#endif
#endif
/*
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
**
**	none
*/

/*
**  External References:
**
**	Symbol Definitions For Message Codes
*/
#include <img/ImgStatusCodes.h>

/*
**	External Entry Points
*/

#ifdef NODAS_PROTO
void	ChfSignal();
void	ChfStop();

unsigned long	ImgExportDDIFFrame();
void		ImgExportDDIFPageBreak();
unsigned long	ImgImportDDIFFrame();
unsigned long	ImgStandardizeFrame();
void		ImgVerifyFrame();

void		_ImgGet();
long		_ImgGetVerifyStatus();
void		_ImgPut();
long		_ImgVerifyStandardFormat();
#endif

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$EXPORT_FRAME
**  ImgExportFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$EXPORT_FRAME( srcfid, ctx, flags )
unsigned long	 srcfid;
struct DCB	*ctx;
unsigned long	 flags;
{

return (ImgExportFrame( srcfid, ctx, flags ));
} /* end of IMG$EXPORT_FRAME */
#endif


/******************************************************************************
** Portable entry point
******************************************************************************/
unsigned long ImgExportFrame( srcfid, ctx, flags )
struct FCT *srcfid;
struct DCB	*ctx;
unsigned long	 flags;
{
unsigned long	 byte_count = 0;
unsigned long	 status;
struct FCB	*fcb	    = ctx->DcbA_Fcb;

if ( VERIFY_ON_ )
    ImgVerifyFrame( srcfid, ImgM_NonstandardVerify );

switch ( ctx->DcbL_Ftype )
    {
    case ImgK_FtypeDDIF:
	ImgExportDDIFFrame(  srcfid
			    ,(long)0	/* no ROI		    */
			    ,ctx
			    ,(long)0,(long)0	/* no bufadr or buflen	    */
			    ,flags
			    ,(long)0,(long)0	/* no action rtn or usrprm  */
			    );

#if defined(NEW_CDA_CALLS)
	status = ddis_get_parse_location(
#else
	status = ddis$get_parse_location(
#endif
		    &(ctx->DcbL_StrmCtx),
		    &byte_count );
	break;
    default:
	ChfSignal( 1, ImgX_INVFTYPE );
    }

return byte_count;
} /* end of ImgExportFrame */


/******************************************************************************
**  IMG$EXPORT_PAGE_BREAK
**  ImgExportPageBreak
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$EXPORT_PAGE_BREAK( ctx, flags )
unsigned long	ctx;
unsigned long	flags;
{

return (ImgExportPageBreak( ctx, flags ));
} /* end of IMG$EXPORT_PAGE_BREAK */
#endif


/******************************************************************************
** Portable entry point
******************************************************************************/
unsigned long ImgExportPageBreak( ctx, flags )
struct DCB     *ctx;
unsigned long	flags;
{
unsigned long	byte_count  = 0;
unsigned long	 status;

switch ( ctx->DcbL_Ftype )
    {
    case ImgK_FtypeDDIF:
	ImgExportDDIFPageBreak( ctx, flags );

#if defined(NEW_CDA_CALLS)
	status = ddis_get_parse_location(
#else
	status = ddis$get_parse_location(
#endif
		    &(ctx->DcbL_StrmCtx),
		    &byte_count );
	break;
    default:
	ChfSignal( 1, ImgX_INVFTYPE );
    }


return byte_count;
} /* end of ImgExportPageBreak */

/******************************************************************************
**  IMG$IMPORT_FRAME
**  ImgImportFrame
**
**  FUNCTIONAL DESCRIPTION:
**
**	[@tbs@]
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
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
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
unsigned long IMG$IMPORT_FRAME( ctx, flags )
struct DCB	*ctx;
unsigned long	 flags;
{

return (ImgImportFrame( ctx, flags ));
} /* end of IMG$IMPORT_FRAME */
#endif


/******************************************************************************
** Portable entry point
******************************************************************************/
struct FCT *ImgImportFrame( ctx, flags )
struct DCB	*ctx;
unsigned long	 flags;
{
long		 compression_type;
long		 comp_space_org;
long		 data_class;
long		 status;
struct FCT *retfid		    = 0;
struct FCB	*fcb		    = ctx->DcbA_Fcb;

switch ( ctx->DcbL_Ftype )
    {
    case ImgK_FtypeDDIF:
	retfid = ImgImportDDIFFrame(  
			     ctx
			    ,0,0	/* no bufadr or buflen	    */
			    ,flags
			    ,0,0	/* no action rtn or usrprm  */
			    );
	break;
    default:
	ChfSignal( 1, ImgX_INVFTYPE );
    }

/*
** Convert the frame to standard, native format if it isn't already
** (and if conversion isn't disabled by a flag).
**
** NOTE: The one exception to this behavior is if the frame is compressed.
**	 Compressed frames are NEVER automatically converted.
*/
if ( retfid != 0 && !(flags & ImgM_NoStandardize) )
    {
    /*
    ** If the frame is bitonal or greyscale, fake out standard
    ** format checking and conversion if all that is wrong is
    ** that the component space org is wrong.
    */
    _ImgGet( retfid, Img_CompSpaceOrg, &comp_space_org, LONGSIZE, 0, 0 );
    _ImgGet( retfid, Img_ImageDataClass, &data_class, LONGSIZE, 0, 0 );
    if ( (data_class == ImgK_ClassBitonal || 
	  data_class == ImgK_ClassGreyscale ) &&
	  comp_space_org == ImgK_BandIntrlvdByPixel )
	{
	comp_space_org = ImgK_BandIntrlvdByPlane;
	_ImgPut( retfid, Img_CompSpaceOrg, &comp_space_org, LONGSIZE, 0);
	}

    /*
    ** See if the frame needs to be converted to standard format.
    **
    **	NOTE that only uncompressed images can be converted to
    **	standard format.
    */
    _ImgGet( retfid, Img_CompressionType, &compression_type, LONGSIZE, 0, 0 );
    if ( compression_type == ImgK_PcmCompression )
	{
	status = _ImgVerifyStandardFormat( retfid, ImgM_NoChf );
	if ( (status&1) != 1  )
	    retfid = ImgStandardizeFrame( retfid, ImgM_AutoDeallocate );
	}
    }

if ( retfid != 0 && VERIFY_ON_ )
    ImgVerifyFrame( retfid, ImgM_NonstandardVerify );

return retfid;
} /* end of ImgImportFrame */
