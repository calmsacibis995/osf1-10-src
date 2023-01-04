
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
**  IMG__BLOCK_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	Low level (Utility Level II) routines for allocation and
**	deallocation of ISL data structure blocks.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**	Mark Sornson,  "       "         "
**
**  CREATION DATE:     
**
**	28-NOV-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
** Table of contents
*/
#ifdef NODAS_PROTO
struct DCB  *_ImgAllocAndInitDcb();	
struct DCB  *_ImgAllocateDcb();

struct FCT  *_ImgAllocFct();
struct BHD  *_ImgBlkAlloc();
void	     _ImgBlkDealloc();
void	     _ImgDeallocateDcb();
void	     _ImgDeallocFct();
void	     _ImgVerifyDcb();
#endif


/*
** INCLUDE FILES:
*/
#include    <stdio.h>
#include    <stdlib.h>

#include    <img/ChfDef.h>
#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
#include    <ImgVectorTable.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

/*
** MACROS: none
*/

/*
** EQUATED SYMBOLS: none
*/

/*
** OWN STORAGE: none
*/

/*
** EXTERNAL REFERENCES: 
**
**  External routine declarations	<- from module ->
*/
#ifdef NODAS_PROTO
void	ChfSignal();			/* CHF$OBJLIB.OLB   */
void	ChfStop();			/* CHF$OBJLIB.OLB   */

char	*_ImgCalloc();			/* IMG$$MEMORY_MGT  */
void	 _ImgCfree();			/* IMG$$MEMORY_MGT  */
#endif

/*
**  External symbol definitions
**
**	Status codes.
*/
#include    <img/ImgStatusCodes.h>

/*
** Fct trace info
*/
#if defined (__VAXC) || defined(VAXC)
noshare static struct LLST {
    struct FCT	*LlstA_Flink;
    struct FCT	*LlstA_Blink;
    } ImgA_FidList  =	{ (struct FCT*)&ImgA_FidList, 
			  (struct FCT*)&ImgA_FidList };

noshare static long	ImgL_FidCount	= 0;

#else
static struct LLST {
    struct FCT	*LlstA_Flink;
    struct FCT	*LlstA_Blink;
    } ImgA_FidList  =	{ (struct FCT*)&ImgA_FidList, 
			  (struct FCT*)&ImgA_FidList };

static long	ImgL_FidCount	= 0;
#endif

#ifdef NODAS_PROTO
static void	_ImgInsertFct();
static void	_ImgRemoveFct();
void		ImgShowFidList();
#else
PROTO(static void _ImgInsertFct, (struct FCT */*new_fct*/));
PROTO(static void _ImgRemoveFct, (struct FCT */*fct*/));
PROTO(void ImgShowFidList, (void));
#endif


/******************************************************************************
**  _ImgAllocAndInitDcb
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate and initialize a document context block.
**
**  FORMAL PARAMETERS:
**
**	access_mode	    Access mode (read/import or write/export)
**			    Passed by value
**	ctx_mode	    Context mode (file or stream)
**			    Passed by value
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
**	dcb		    Document context block.
**			    Passed by reference.
**
**  SIGNAL CODES:
**
**	none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
struct DCB *_ImgAllocAndInitDcb( access_mode, ctx_mode )
long	access_mode;
long	ctx_mode;
{                                       
struct DCB  *dcb;

dcb = _ImgAllocateDcb();
dcb->DcbL_Flags.DcbV_AccessMode = access_mode;
dcb->DcbL_Flags.DcbV_CtxMode = ctx_mode;
dcb->DcbA_Self = dcb;

dcb->DcbL_IobExtend = DcbK_DefIobExt;
dcb->DcbA_PrefixRtn = NULL;
dcb->DcbL_ActionBytCnt = 0;

return dcb;
} /* end routine _ImgAllocAndInitDcb */


/******************************************************************************
**  _ImgAllocateDcb
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a document context block.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	 none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	dcb		Document context block
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	_ImgAllocAndInitDcb		(in this module)
**
******************************************************************************/
struct DCB *_ImgAllocateDcb()	/* DDIF Context Blk	*/
{
struct DCB *dcb;

dcb = (struct DCB *) _ImgBlkAlloc( sizeof(struct DCB) , IMG_K_BLKTYP_DCB );

return dcb;
} /* end routine _ImgAllocateDcb */


/************************************************************************
**  _ImgAllocFct()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocates an FCT structure. Fills in block type, block size, and
**      Internally allocated flag.
**
**  FORMAL PARAMETERS:
**
**      none
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
**      Address of FCT structure allocated, if successful.
**
**  SIGNAL CODES:
**
**      none, except as returned by _ImgBlkAlloc
**
**  SIDE EFFECTS:
**
**      none
**
***************************************************************************/
struct FCT *_ImgAllocFct()
{
struct FCT  *fct_adr;

fct_adr = (struct FCT *) _ImgBlkAlloc(sizeof(struct FCT),ImgK_BlktypFct);

fct_adr->FctR_Fctstk.LhdA_Flink = (struct BHD *) &(fct_adr->FctR_Fctstk);
fct_adr->FctR_Fctstk.LhdA_Blink = (struct BHD *) &(fct_adr->FctR_Fctstk);
fct_adr->FctR_Fctstk.LhdB_Type = ImgK_BlktypLhd;
fct_adr->FctR_Fctstk.LhdW_ListCnt = 0;

_ImgInsertFct( fct_adr );

return fct_adr;
} /* end of _ImgAllocFct */


/************************************************************************
**  _ImgBlkAlloc()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocates a structure of a given block type and points the listhead 
**      to itself. It uses the ISL "calloc" function to ensure that other
**      fields are zeroed out.
**
**  FORMAL PARAMETERS:
**
**	blk_size - Unsigned int by value. Size of block to allocate.
**
**      blk_type - Unsigned char by value. Block type to be allocated.
**                 Block types are defined in img$framedef.h.
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
**      Address of structure allocated, if successful.
**
**  SIGNAL CODES:
**
**	ImgX_INVBUFLEN - invalid buffer length, because the size to
**			 be allocated is less than the size of a BHD.
**
**  SIDE EFFECTS:
**
**      none
**
***************************************************************************/
struct BHD *_ImgBlkAlloc( blk_size, blk_type )
long	blk_size;
long	blk_type;
{
struct BHD  *ret_ptr;

if (blk_size < sizeof(struct BHD))
    ChfStop( 1,  ImgX_INVBUFLEN);

ret_ptr = (struct BHD *) _ImgCalloc( 1, blk_size ); /* alloc block o mem*/

ret_ptr->BhdA_Flink  = ret_ptr;    /* initialize block head fields   */
ret_ptr->BhdA_Blink  = ret_ptr;
ret_ptr->BhdW_Length = blk_size;
ret_ptr->BhdB_Type   = blk_type;
ret_ptr->BhdL_Flags.BhdV_InternalAlloc = TRUE;

return ret_ptr;
} /* end of _ImgBlkAlloc */


/************************************************************************
**  _ImgBlkDealloc()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocates a structure after verifying that the structure has been
**      internally allocated and has a zero reference count.
**
**  FORMAL PARAMETERS:
**
**      block - address of the block to be deallocated
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
**      void
**
**  SIGNAL CODES:
**
**      ImgX_BLKNOTINT - block was not internally allocated by ISL routines
**      
**  SIDE EFFECTS:
**
**      none
**
***************************************************************************/
void _ImgBlkDealloc( block )
struct BHD *block;
{

if (block->BhdL_Flags.BhdV_InternalAlloc != TRUE)
    ChfSignal( 3,  ImgX_BLKNOTINT, 1, block);
else 
    _ImgCfree( block );

return;
} /* end of _ImgBlkDealloc */


/******************************************************************************
**  _ImgDeallocateDcb
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate the DDIF context block.
**
**  FORMAL PARAMETERS:
**
**	dcb	Document context block, by reference.
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
**	none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgDeallocateDcb( dcb )
struct DCB  *dcb;
{
long	status;
     
/*
** Deallocate the iobuffer if it was internally allocated
*/
if (dcb->DcbL_Flags.DcbV_IntIoBuf == 1)
    _ImgCfree( dcb->DcbA_IobAdr );

/*
** Deallocate the memory for the DCB                                       
*/
_ImgBlkDealloc( (struct BHD *)dcb );

return;
} /* end routine _ImgDeallocateDcb */


/************************************************************************
**  _ImgDeallocFct()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocates an FCT structure after verifying that the structure 
**      has been internally allocated and really is an FCT.
**
**  FORMAL PARAMETERS:
**
**      fct_block - address of the FCT block to be deallocated
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
**      void
**
**  SIGNAL CODES:
**
**      ImgX_INVBLKTYP - invalid blk type (not an FCT)
**
**  SIDE EFFECTS:
**
**      none
**
***************************************************************************/
void _ImgDeallocFct( fct_adr )
struct FCT  *fct_adr;
{

_ImgRemoveFct( fct_adr );

/*
** Clear first longword of FCT so that ISL services can
** verify that fids are no longer valid.  Then delete the FCT.
*/
*((long *)fct_adr) = 0;

_ImgBlkDealloc((struct BHD *)fct_adr);

return;
} /* end of _ImgDeallocFct */


/******************************************************************************
**  _ImgVerifyDcb
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify a document context block address -- verify that it points
**	to a valid dcb.
**
**  FORMAL PARAMETERS:
**
**	dcb		Document context block, by reference
**	access_mode	DDIF stream access mode (import or export), by value
**	ctx_mode	Context mode, either file or stream, by value.
**
**  IMPLICIT INPUTS:
**
**	 none
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
**	ImgX_INCACCMOD		Inconsistent access mode.  The mode value
**				(IMG$K_MODE_xxPORT) did not correspond to
**				the function of the IMG$xxPORT_DDIF_FRAME 
**				routine.
**      
**	ImgX_INVALDSID		Invalid stream identifier.  The value passed
**				in for the stream id was not the address of
**				a valid dcb.  Either the block head was
**				bad or the self identifier field, containing
**				the address of the DCB, was bad.
**
**	ImgX_INVCTXMOD		Invalid context mode.  The context mode
**				identifies whether the context-id identifies
**				file or stream mode.  Invalid context means
**				that the value passed in did not match the
**				actual value (set as a flag) in the DCB.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	IMG$EXPORT_DDIF_FRAME		(in module IMG$DDIF_EXPORT_FRAME)
**	IMG$IMPORT_DDIF_FRAME		(in module IMG$DDIF_IMPORT_FRAME)
**
******************************************************************************/
void _ImgVerifyDcb( dcb, access_mode, ctx_mode )
struct DCB  *dcb;
long	     access_mode;
long	     ctx_mode;
{
if (ctx_mode != 0)
    if ( dcb->DcbL_Flags.DcbV_CtxMode != ctx_mode )
	ChfStop( 1,  ImgX_INVCTXUSE );

if ( dcb->DcbR_Bhd.BhdB_Type != IMG_K_BLKTYP_DCB ||
     dcb->DcbA_Self != dcb )
    ChfStop( 1,  ImgX_INVALDSID );		/* invalid stream id */

if ( dcb->DcbL_Flags.DcbV_AccessMode != access_mode )
    ChfStop( 1,  ImgX_INCACCMOD );		/* inconsistent access mode */

return;
} /* end of _ImgVerifyDcb */


/********************
** _ImgInsertFct
*********************/
static void _ImgInsertFct( new_fct )
struct FCT  *new_fct;
{
char	    *img_show_fid_alloc	    = 0;
struct FCT  *last_fct		    = ImgA_FidList.LlstA_Blink;

if ( img_show_fid_alloc = getenv( "IMG_SHOW_FID_ALLOC" ) )
    {
    printf( "allocating fid: %d\n", new_fct );
    }

new_fct->FctR_Blkhd.BhdA_Blink = (struct BHD *)last_fct;
new_fct->FctR_Blkhd.BhdA_Flink = last_fct->FctR_Blkhd.BhdA_Flink;
last_fct->FctR_Blkhd.BhdA_Flink = (struct BHD *)new_fct;
ImgA_FidList.LlstA_Blink = new_fct;

++ImgL_FidCount;
return;
} /* end of _ImgInsertFct */


/******************
** _ImgRemoveFct
*******************/
static void _ImgRemoveFct( fct )
struct FCT  *fct;
{
char	    *img_show_fid_dealloc   = 0;
struct FCT  *prev_fct		    = (struct FCT *)fct->FctR_Blkhd.BhdA_Blink;
struct FCT  *next_fct		    = (struct FCT *)fct->FctR_Blkhd.BhdA_Flink;

if ( img_show_fid_dealloc = getenv( "IMG_SHOW_FID_DEALLOC" ) )
    {
    printf( "deallocating fid: %d\n", fct );
    }

prev_fct->FctR_Blkhd.BhdA_Flink = (struct BHD *)next_fct;
next_fct->FctR_Blkhd.BhdA_Blink = (struct BHD *)prev_fct;

--ImgL_FidCount;
return;
} /* end of _ImgRemoveFct */


/******************
** ImgShowFidList
*******************/
void ImgShowFidList()
{
char	     user_label[512];
long	     idx;
long	     retlen	    = 0;
long	     user_label_cnt;
long	     user_label_len;
struct FCT  *cur_fct;

if ( ImgL_FidCount > 0 )
    {
    cur_fct = ImgA_FidList.LlstA_Flink;
    for ( idx = 0; idx < ImgL_FidCount; ++idx )
	{
	printf( "fid:  %d\n", cur_fct );
	cur_fct = (struct FCT *)cur_fct->FctR_Blkhd.BhdA_Flink;
	}
    }

printf( "ImgL_FidCount:  %d\n", ImgL_FidCount );

return;
} /* end of _ImgShowFidList */
