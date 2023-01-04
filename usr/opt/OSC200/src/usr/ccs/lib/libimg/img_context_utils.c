
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
**  IMG_CONTEXT_UTILS.C
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	This module contains routines that manage context information
**	stored in the FCT.  Context refers to the current image content
**	element (ICE) being pointed to, and the current image data unit
**	(IDU) of the current ICE being pointed to.
**
**	Included are routines that go to first, last, and next ICE and
**	IDU, and that will save and restore a current context, and that
**	will purge all saved contexts, keeping only the current one.
**
**	Although not yet documented as such, these routines are for public
**	application use, to be used when support of other component space
**	organizations (other than full pixel compaction) becomes widely
**	used throughout ISL.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Mark W. Sornson
**
**  CREATION DATE:
**
**     5-OCT-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Include files:
*/
/*
**	From SYS$LIBRARY:VAXCRTL.TLB
*/
#include	<string.h>

#include	<img/ChfDef.h>
#include	<img/ImgDef.h>
#include	<ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include	<cda$msg.h>
#include        <cda$ptp.h>
#include	<ddif$def.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include	<cdamsg.h>
#include	<ddifdef.h>
#else
#include	<cda_msg.h>
#include	<ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include        <cdaptp.h>
#else
#include        <cda_ptp.h>
#endif
#endif

/*
**  Table of contents:
**
**	VMS veneer global entry points
*/
#if defined(__VMS) || defined(VMS)
long	     IMG$FIRST_CONTENT_ELEMENT();
long	     IMG$FIRST_DATA_PLANE();
long	     IMG$LAST_CONTENT_ELEMENT();
long	     IMG$LAST_DATA_PLANE();
long	     IMG$NEXT_CONTENT_ELEMENT();
long	     IMG$NEXT_DATA_PLANE();
struct FCT  *IMG$PURGE_CTX();
struct FCT  *IMG$RESET_CTX();
struct FCT  *IMG$RESTORE_CTX();
struct FCT  *IMG$SAVE_CTX();
long	     IMG$SET_CONTENT_ELEMENT();
long	     IMG$SET_DATA_PLANE();
#endif

/*
**	Portable global entry points
*/
#ifdef NODAS_PROTO
long	     ImgFirstContentElement();
long	     ImgFirstDataPlane();
long	     ImgLastContentElement();
long	     ImgLastDataPlane();
long	     ImgNextContentElement();
long	     ImgNextDataPlane();
struct FCT  *ImgPurgeCtx();
struct FCT  *ImgResetCtx();
struct FCT  *ImgRestoreCtx();
struct FCT  *ImgSaveCtx();
struct FCT  *ImgSetContentElement();
struct FCT  *ImgSetDataPlane();
#endif

/*
**	Low level global entry points 
*/
#ifdef NODAS_PROTO
long	     _ImgFirstContentElement();
long	     _ImgFirstDataPlane();
long	     _ImgLastContentElement();
long	     _ImgLastDataPlane();
long	     _ImgNextContentElement();
long	     _ImgNextDataPlane();
struct FCT  *_ImgSetContentElement();
struct FCT  *_ImgSetDataPlane();
#endif


/*
**	Local Routines
*/
#ifdef NODAS_PROTO
static long	 Verify_condition_handler();
#else
#if !(defined sparc) && !(defined __osf__)
PROTO(static long Verify_condition_handler, (int */*signal*/, ChfMchArgsPtr /*mechanism*/));
#else
PROTO(static long Verify_condition_handler, (int /*signal*/, int /*mechanism*/));
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
*/
#define	FAIL	0	/* return codes for context functions	*/
#define SUCCESS 1

/*
**  External Routine References:
**
**  Forward decls done in <img/ChfDef.h>
*/
#ifdef NODAS_PROTO
long	     _ImgGet();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
long	     _ImgGetVerifyStatus();
long	     _ImgPut();			/* from IMG__ATTRIBUTE_ACCESS_UTILS */
struct FCT  *_ImgAllocFct();		/* from IMG__BLOCK_UTILS	    */
void	     _ImgDeallocFct();		/* from IMG__BLOCK_UTILS	    */
struct BHD  *_ImgLLFirstElement();
void	     _ImgLLPrependElement();	/* from IMG__COMMON_UTILS	    */
struct BHD  *_ImgLLRemoveElement();	/* from IMG__COMMON_UTILS	    */
long	     _ImgVerifyStructure();	/* from IMG__VERIFY_UTILS	    */
#endif

/*
**  External symbol defintions (status codes) currently commented out
*/
#include	<img/ImgStatusCodes.h>

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  IMG$FIRST_CONTENT_ELEMENT
**  ImgFirstContentElement 
**  _ImgFirstContentElement 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set frame context to the first cell (ICE) in the frame.
**	If there is no first cell, nothing bad happens, but
**	a failure code is returned.  [The ICEs are DDIF$_IMG aggregates.]
**
**  FORMAL PARAMETERS:
**
**      fid, by reference	Frame identifier, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS		1, context was set to first cell
**	FAIL		0, context was not set to first cell.
**
**  SIGNAL CODES:
**
**      none (unless frame fails verification)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$FIRST_CONTENT_ELEMENT( fid )
long fid;
{

return ImgFirstContentElement( fid );
} /* end of IMG$FIRST_CONTENT_ELEMENT */
#endif

long ImgFirstContentElement( fid )
struct FCT *fid;
{
long  retcode	= FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgFirstContentElement( fid );

return retcode;
} /* end of ImgFirstContentElement */


long _ImgFirstContentElement( fid )
struct FCT  *fid;
{
CDAaddress item;
CDAagghandle img_aggr	= NULL;
long  length;
long  retcode	= FAIL;
CDArootagghandle root_aggr;
CDAagghandle seg_aggr;
CDAconstant aggregate_item;
int  status;

root_aggr = fid->FctL_RootAggr;
seg_aggr = fid->FctL_SegAggr;

if ( seg_aggr != NULL && root_aggr != NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_SEG_CONTENT;
#else
    aggregate_item = DDIF$_SEG_CONTENT;
#endif
    status = CDA_LOCATE_ITEM_(
		&root_aggr,
		&seg_aggr,
		&aggregate_item,
		&item,
		&length,
		0, 0 );
    switch ( status )
	{
#if defined(NEW_CDA_SYMBOLS)
	case CDA_NORMAL:
#else
	case CDA$_NORMAL:
#endif
	    img_aggr = *(CDAagghandle *)item;
	    fid->FctL_ImgAggr = img_aggr;
	    retcode = SUCCESS;
#if defined(NEW_CDA_SYMBOLS)
	case CDA_EMPTY:   
#else
	case CDA$_EMPTY:   
#endif
	    break;
	default:
	    ChfSignal( 1,  status );
	} /* end switch */
    } /* end if */

return retcode;
} /* end of _ImgFirstContentElement */


/******************************************************************************
**  IMG$FIRST_DATA_PLANE
**  ImgFirstDataPlane 
**  _ImgFirstDataPlane 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set frame context to the first data plane in the current
**	cell (or content element).  If there is no first data plane
**	in the current cell, nothing bad happens, but a failure
**	status is returned.  (The data plane is pointed to by the
**	IDU, which is pointed to by a DDIF$_IDU aggregate.)
**
**  FORMAL PARAMETERS:
**
**      fid, by value	frame identifier, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      FAIL		First data plane could not be found.
**	SUCCESS		First data plane was found and set.
**
**  SIGNAL CODES:
**
**      none (unless verify fails)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$FIRST_DATA_PLANE( fid )
long fid;
{

return ImgFirstDataPlane( fid );
} /* end of IMG$FIRST_DATA_PLANE */
#endif

long ImgFirstDataPlane( fid )
struct FCT *fid;
{
long  retcode = FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgFirstDataPlane( fid );

return retcode;
} /* end of ImgFirstDataPlane */


long _ImgFirstDataPlane( fid )
struct FCT  *fid;
{
CDAagghandle idu_aggr;
CDAagghandle img_aggr;
CDAaddress item;
long  length;
long  retcode = FAIL;
CDArootagghandle root_aggr;
CDAconstant aggregate_item;
int  status;

root_aggr = fid->FctL_RootAggr;
img_aggr = fid->FctL_ImgAggr;

if ( img_aggr != NULL && root_aggr != NULL )
    {
#if defined(NEW_CDA_SYMBOLS)
    aggregate_item = DDIF_IMG_CONTENT;
#else
    aggregate_item = DDIF$_IMG_CONTENT;
#endif
    status = CDA_LOCATE_ITEM_(
		&root_aggr,
		&img_aggr,
		&aggregate_item,
		&item,
		&length,
		0, 0 );
    switch ( status )
	{
#if defined(NEW_CDA_SYMBOLS)
	case CDA_NORMAL:
#else
	case CDA$_NORMAL:
#endif
	    idu_aggr = *(CDAagghandle *)item;
	    fid->FctL_IduAggr = idu_aggr;
	    retcode = SUCCESS;
#if defined(NEW_CDA_SYMBOLS)
	case CDA_EMPTY:
#else
	case CDA$_EMPTY:
#endif
	    break;
	default:
	    ChfSignal( 1,  status );
	}
    }

return retcode;
} /* end of _ImgFirstDataPlane */


/******************************************************************************
**  IMG$LAST_CONTENT_ELEMENT
**  ImgLastContentElement 
**  _ImgLastContentElement 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set the frame context to the last cell in the frame.
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      FAIL		No last cell to set.
**	SUCCESS		Last cell was set.
**
**  SIGNAL CODES:
**
**      none (unless verify fails)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$LAST_CONTENT_ELEMENT( fid )
long fid;
{

return ImgLastContentElement( fid );
} /* end of IMG$LAST_CONTENT_ELEMENT */
#endif

long ImgLastContentElement( fid )
struct FCT *fid;
{
long retcode = FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgLastContentElement( fid );

return retcode;
} /* end of ImgLastContentElement */


long _ImgLastContentElement( fid )
struct FCT  *fid;
{
CDAagghandle cur_img_aggr;
CDAagghandle next_img_aggr;
long retcode = FAIL;
int status;

cur_img_aggr = fid->FctL_ImgAggr;

if ( cur_img_aggr != NULL )
    {
    do
	{
	status = CDA_NEXT_AGGREGATE_( &cur_img_aggr, &next_img_aggr );
	if ( next_img_aggr != NULL )
	    cur_img_aggr = next_img_aggr;
	}
#if defined(NEW_CDA_SYMBOLS)
    while ( status != CDA_ENDOFSEQ );
#else
    while ( status != CDA$_ENDOFSEQ );
#endif
    }

fid->FctL_ImgAggr = cur_img_aggr;

return retcode;
} /* end of _ImgLastContentElement */


/******************************************************************************
**  IMG$LAST_DATA_PLANE
**  ImgLastDataPlane 
**  _ImgLastDataPlane 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set the frame context to point to the last data plane of
**	the current cell (content element).
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      FAIL		No data plane found to be set to last.
**	SUCCESS		Last data plane set.
**
**  SIGNAL CODES:
**
**      none (unless verify fails)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$LAST_DATA_PLANE( fid )
long fid;
{

return ImgLastDataPlane( fid );
} /* end of IMG$LAST_DATA_PLANE */
#endif

long ImgLastDataPlane( fid )
struct FCT *fid;
{
long retcode = FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgLastDataPlane( fid );

return retcode;
} /* end of ImgLastDataPlane */


long _ImgLastDataPlane( fid )
struct FCT  *fid;
{
CDAagghandle cur_idu_aggr;
CDAagghandle next_idu_aggr;
long retcode = FAIL;
int status;

cur_idu_aggr = fid->FctL_IduAggr;

if ( cur_idu_aggr != NULL )
    {
    do
	{
	status = CDA_NEXT_AGGREGATE_( &cur_idu_aggr, &next_idu_aggr );
	if ( next_idu_aggr != NULL )
	    cur_idu_aggr = next_idu_aggr;
	}
#if defined(NEW_CDA_SYMBOLS)
    while ( status != CDA_ENDOFSEQ );
#else
    while ( status != CDA$_ENDOFSEQ );
#endif
    }

fid->FctL_IduAggr = cur_idu_aggr;

return retcode;
} /* end of _ImgLastDataPlane */


/******************************************************************************
**  IMG$NEXT_CONTENT_ELEMENT
**  ImgNextContentElement 
**  _ImgNextContentElement 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set frame context to the next cell in the frame.
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      SUCCESS		1, context was set to next cell
**	FAIL		0, context was not set to next cell.
**
**  SIGNAL CODES:
**
**      none (unless frame fails verification)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$NEXT_CONTENT_ELEMENT( fid )
long fid;
{

return ImgNextContentElement( fid );
} /* end of IMG$NEXT_CONTENT_ELEMENT */
#endif

long ImgNextContentElement( fid )
struct FCT *fid;
{
long retcode = FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgNextContentElement( fid );

return retcode;
} /* end of ImgNextContentElement */


long _ImgNextContentElement( fid )
struct FCT  *fid;
{
CDAagghandle cur_img_aggr;
CDAagghandle next_img_aggr;
long retcode = FAIL;
int status;

cur_img_aggr = fid->FctL_ImgAggr;

if ( cur_img_aggr != NULL )
    {
    status = CDA_NEXT_AGGREGATE_( &cur_img_aggr, &next_img_aggr );
    switch ( status )
	{
#if defined(NEW_CDA_SYMBOLS)
	case CDA_NORMAL:
#else
	case CDA$_NORMAL:
#endif
	    fid->FctL_ImgAggr = next_img_aggr;
	    retcode = SUCCESS;
#if defined(NEW_CDA_SYMBOLS)
	case CDA_ENDOFSEQ: 
#else
	case CDA$_ENDOFSEQ: 
#endif
	    break;
	default:
	    ChfSignal( 1,  status );
	}
    }

return retcode;
} /* end of _ImgNextContentElement */


/******************************************************************************
**  IMG$NEXT_DATA_PLANE
**  ImgNextDataPlane 
**  _ImgNextDataPlane 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set frame context to the next data plane in the current
**	cell (or content element).
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      FAIL		Next data plane could not be found.
**	SUCCESS		Next data plane was found and set.
**
**  SIGNAL CODES:
**
**      none (unless verify fails)
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$NEXT_DATA_PLANE( fid )
long fid;
{

return ImgNextDataPlane( fid );
} /* end of IMG$NEXT_DATA_PLANE */
#endif

long ImgNextDataPlane( fid )
struct FCT *fid;
{
long retcode = FAIL;

if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

retcode = _ImgNextDataPlane( fid );

return retcode;
} /* end of ImgNextDataPlane */


long _ImgNextDataPlane( fid )
struct FCT  *fid;
{
CDAagghandle cur_idu_aggr;
CDAagghandle next_idu_aggr;
long retcode = FAIL;
int status;

cur_idu_aggr = fid->FctL_IduAggr;

if ( cur_idu_aggr != NULL )
    {
    status = CDA_NEXT_AGGREGATE_( &cur_idu_aggr, &next_idu_aggr );
    switch ( status )
	{
#if defined(NEW_CDA_SYMBOLS)
	case CDA_NORMAL:
#else
	case CDA$_NORMAL:
#endif
	    fid->FctL_IduAggr = next_idu_aggr;
	    retcode = SUCCESS;
#if defined(NEW_CDA_SYMBOLS)
	case CDA_ENDOFSEQ: 
#else
	case CDA$_ENDOFSEQ: 
#endif
	    break;
	default:
	    ChfSignal( 1,  status );
	}
    }

return retcode;
} /* end of _ImgNextDataPlane */


/******************************************************************************
**  IMG$PURGE_CTX
**  ImgPurgeCtx 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Purge all the saved frame contexts from the context stack.
**	Keep the current context values.  (If no context was saved,
**	nothing else happens.)
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fct	Frame context block (fid) of current frame.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$PURGE_CTX( fct )
struct	FCT	*fct;
{

return ImgPurgeCtx( fct );
} /* end of IMG$PURGE_CTX */
#endif

struct FCT *ImgPurgeCtx( fct )
struct	FCT	*fct;
{
struct	FCT	*tmp_fct;
struct	FCT	*top_fct;

if ( fct->FctR_Fctstk.LhdW_ListCnt > 0)
    {
    top_fct = (struct FCT *) _ImgLLFirstElement( &(fct->FctR_Fctstk) );
    while ( top_fct != NULL )
    	{
    	/*
    	** 'Pop' the previous fct off the stack -- retrieve it and
    	** then remove it from the list.
    	*/
    	tmp_fct = top_fct;
    	top_fct = (struct FCT *) 
                  _ImgLLRemoveElement( &(fct->FctR_Fctstk), (struct BHD *)top_fct );
    
    	/*
    	** Deallocate the popped block from the stack (throw it away)
    	*/                    
    	_ImgDeallocFct( tmp_fct );
    	} /* end of while loop */
    } /* end if */

return fct;
} /* end of ImgPurgeCtx */


/******************************************************************************
**  IMG$RESET_CTX
**  ImgResetCtx 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Reset (or set) the current frame context, establishing the
**	first lookup table in the frame, the first cell in the frame, 
**	and the first data plane in the first cell as the current 
**	context values.
**
**  FORMAL PARAMETERS:
**
**      fid, by value	    Frame identifier, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fid		    Frame identifier that was passed in.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$RESET_CTX( fid )
long fid;
{

return ImgResetCtx( fid );
} /* end of IMG$RESET_CTX */
#endif

struct FCT *ImgResetCtx( fid )
struct FCT *fid;
{
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

_ImgFirstContentElement( fid );
_ImgFirstDataPlane( fid );

return fid;
} /* end of ImgResetCtx */


/******************************************************************************
**  IMG$RESTORE_CTX
**  ImgRestoreCtx 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Restore the frame context to the previous state prior to the
**	last call to IMG$SAVE_CTX.  If no previous context states
**	were saved, no action is taken, though a condition is signalled.
**
**	Note that previous context states are checked by the verify
**	routine, and that a module-local condition handler has been
**	established to trap the invalid condition codes.
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fct	Frame context block that was passed in.
**
**  SIGNAL CODES:
**
**      ImgX_PRVCTXINV	Previous (or last saved) context values have
**			become invalid since they were saved.
**
**	ImgX_NOSAVDCTX	No saved context.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$RESTORE_CTX( fct )
struct	FCT *fct;
{

return ImgRestoreCtx( fct );
} /* end of IMG$RESTORE_CTX */
#endif

struct FCT *ImgRestoreCtx( fct )
struct	FCT *fct;
{
long	     verify_status;
struct	BHD  saved_fct_bhd;
struct	FCT  local_fct;
struct	FCT *previous_fct;


if (fct->FctR_Fctstk.LhdW_ListCnt > 0)
    {
    /*
    ** 'Pop' the previous fct off the stack -- retrieve it and
    ** then remove it from the list.
    */
    previous_fct = (struct FCT *) _ImgLLFirstElement( &(fct->FctR_Fctstk) );
    _ImgLLRemoveElement( &(fct->FctR_Fctstk), (struct BHD *)previous_fct );
    
    /*
    ** Copy the popped fct into local storage and deallocate the
    ** block from the stack.
    */
    local_fct = *previous_fct;
    _ImgDeallocFct( previous_fct );
    
    /*
    ** Verify that the 'popped' fct is still valid.  Establish a
    ** condition handler that causes _ImgVerifyFct to return a
    ** zero if the fct was invalid.  Otherwise _ImgVerifyFct returns
    ** the address of the fct.
    */
#if !(defined sparc) && !(defined __osf__)
    ChfEstablish( Verify_condition_handler );
#endif

/*
** WORK AROUND NOTE: the frame verify check has been commented out
**		     to avoid an infinite loop condition the occurs
**		     due to frame verify calling save and restore
**		     context itself to check the contents of IDUs
**		     which now use the index variable for direct access
**		     to IDU attributes.  Since get and put access to
**		     IDU variables call save and restore ctx, an
**		     infinite recursion situation exists.
**
**    verify_status = (long) IMG_VERIFY_FRAME_CONTENT_( &local_fct );
**    if (verify_status != 0)
**    	{
*/
    	/*
	** Save the input fct block header, copy the previously saved
	** fct into the currect (and top) fct, and then restore the
	** input fct block header.
    	*/
	memcpy( &saved_fct_bhd, &(fct->FctR_Blkhd), sizeof( struct BHD ) );
	*fct = local_fct;
	memcpy( &(fct->FctR_Blkhd), &saved_fct_bhd, sizeof( struct BHD ) );

/*    	}
**    else
**    	{
**    	ChfSignal( 1,  ImgX_PRVCTXINV );
**    	}
*/
    }
else
    ChfSignal( 1,  ImgX_NOSAVDCTX );

return fct;
} /* end of ImgRestoreCtx */


/******************************************************************************
**  IMG$SAVE_CTX
**  ImgSaveCtx 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Save the current frame context.  The current frame context is
**	saved on a context block stack (dcb->DCB$R_FCTSTK), and may be
**	restored by a call to IMG$RESTORE_CTX.
**
**  FORMAL PARAMETERS:
**
**      fct, by reference	Frame context block, required
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      fct	Frame context block passed in.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SAVE_CTX( fct )
struct	FCT *fct;
{

return ImgSaveCtx( fct );
} /* end of IMG$SAVE_CTX */
#endif

struct FCT *ImgSaveCtx( fct )
struct	FCT *fct;
{
struct	FCT *saved_fct;

/* 
** Allocate an fct block to push on the stack
*/
saved_fct = _ImgAllocFct();		

/*
** Copy current fct into saved_fct
*/
*saved_fct = *fct;

/*
** Push saved_fct on the fct context stack by prepending it
** to the fct list pointed to by the fct stack list head.
*/
_ImgLLPrependElement( &(fct->FctR_Fctstk), (struct BHD *)saved_fct );

return fct;
} /* end of ImgSaveCtx */


/******************************************************************************
**  IMG$SET_CONTENT_ELEMENT
**  ImgSetContentElement 
**  _ImgSetContentElement 
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set a particular conent element to be the current one.  
**	Assume that the first content element has an index value of 0.
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id.  Passed by value
**	plane_idx	Plane index.  Passed by value.
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
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$SET_CONTENT_ELEMENT( fid, content_element_index )
long	fid;
long	content_element_index;
{

return ImgSetContentElement( fid, content_element_index );
} /* end of IMG$SET_CONTENT_ELEMENT */
#endif

struct FCT *ImgSetContentElement( fid, content_element_index )
struct FCT *fid;
long	content_element_index;
{
long	loop_index  = 0;
long	content_element_count;

/*
** Verify that the frame structure is OK.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
** Verify that the content_element_index parameter is valid.
**_ImgGet( fid, Img_PlanesPerPixel, &plane_count, sizeof(long), 0, 0 );
**if ( plane_index == 0 || plane_index > plane_count )
**    ChfStop( 1,  ImgX_INVPLAIND );
*/

return (_ImgSetContentElement( fid, content_element_index ));
} /* end of ImgSetContentElement */


struct FCT *_ImgSetContentElement( fid, content_element_index )
struct FCT *fid;
long	content_element_index;
{
long	loop_index  = 0;
long	content_element_count;

/*
** Start from the first content element, and loop until the 
** content element index has been reached.
*/
_ImgFirstContentElement( fid );
while ( loop_index != content_element_index )
    {
    _ImgNextContentElement( fid );
    ++loop_index;
    } /* end while */

return fid;
} /* end of _ImgSetContentElement */


/******************************************************************************
**  IMG$SET_DATA_PLANE
**  ImgSetDataPlane 
**  _ImgSetDataPlane 
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set a particular data plane to be the current one.  Assume that
**	the first data plane has the index value of 0.
**
**  FORMAL PARAMETERS:
**
**	fid		Frame-id.  Passed by value
**	plane_idx	Plane index.  Passed by value.
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
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
long IMG$SET_DATA_PLANE( fid, plane_index )
long	fid;
long	plane_index;
{

return ImgSetDataPlane( fid, plane_index );
} /* end of IMG$SET_DATA_PLANE */
#endif

struct FCT *ImgSetDataPlane( fid, plane_index )
struct FCT *fid;
long	plane_index;
{
long	loop_index  = 0;
long	plane_count;

/*
** Verify that the frame structure is OK.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
** Verify that the plane index parameter is valid.
*/
_ImgGet( fid, Img_PlanesPerPixel, &plane_count, sizeof(long), 0, 0 );
if ( plane_index > plane_count )
    ChfStop( 1,  ImgX_INVPLAIND );

return (_ImgSetDataPlane( fid, plane_index ));
} /* end of ImgSetDataPlane */


struct FCT *_ImgSetDataPlane( fid, plane_index )
struct FCT *fid;
long	plane_index;
{
long	loop_index  = 0;
long	plane_count;

/*
** Start from the first plane, and loop until the plane_idx has been reached.
*/
_ImgFirstDataPlane( fid );
while ( loop_index != plane_index )
    {
    _ImgNextDataPlane( fid );
    ++loop_index;
    } /* end while */

return fid;
} /* end of _ImgSetDataPlane */


/******************************************************************************
**  Verify_condition_handler
**
**  FUNCTIONAL DESCRIPTION:
**
**      This condition handler is called to handle conditions signalled
**	by a call to _ImgVerifyFct.  It will unwind if the condition
**	simply indicates that the fct was found to be invalid, thereby
**	returning control back to the calling routine (placing a zero
**	in R0 for the return value).  If the condition is unexpected,
**	(i.e., not a value checked for), a resignal will occur.
**
**  FORMAL PARAMETERS:
**
**      signal		- VMS signal array containing signal names
**	mechanism	- VMS mechanism vector containing depth on the stack
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Chf_Resignal (always)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
#if !(defined sparc) && !(defined __osf__)
static long Verify_condition_handler( signal, mechanism )
int     *signal;
ChfMchArgsPtr           mechanism;
{
int     test_condition;
int     condition_match;

/*
** Match the first signalled condition value with a list of
** expected condition codes.  Zero is returned if no match is found.
*/
test_condition = ImgX_INVBLKTYP;
condition_match = ChfMatchCondition(2, &(signal[1]),&test_condition);


if (condition_match != 0)
    {
    /*
    ** Set R0 to zero, which will return a zero to the routine
    ** that called _ImgVerifyFct.
    */
    mechanism->save_R0 = 0;
    /*
    ** Unwind to the stack depth, which effectively returns to
    ** the calling routine as if _ImgVerifyFct returned normally.
    ** (NOTE: zero (for newpc) is returned as the second parameter
    ** which causes execution to continue right after the call to
    ** _ImgVerifyFct.
    */
    ChfUnwind( mechanism->depth, 0 );
    }
return(Chf_Resignal);
} /* end of Verify_condition_handler */
#else
static long Verify_condition_handler( signal, mechanism )
int signal,mechanism;
{
return(2328);
}
#endif
