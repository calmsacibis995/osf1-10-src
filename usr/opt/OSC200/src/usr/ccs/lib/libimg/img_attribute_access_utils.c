
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
**  IMG_ATTRIBUTE_ACCESS_UTILS
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	The utility functions in this module provide high level, object-
**	oriented access to frame attributes, allowing them to be read,
**	written, or erased.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHORS:
**
**	Mark Sornson
**
**  CREATION DATE:     
**
**	20-SEP-1989
**
************************************************************************/

/*
**  Include files 
*/
#include <img/ChfDef.h>
#include <img/ImgDef.h>
#include <ImgDefP.h>
#include <ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include <ddif$def.h>
#include <cda$msg.h>
#else			/* from /usr/include/X11 (DDIF Toolkit) */
#if defined(NEW_CDA_SYMBOLS)
#include <ddifdef.h>
#include <cdamsg.h>
#else
#include <ddif_def.h>
#include <cda_msg.h>
#endif
#if defined(NEW_CDA_CALLS)
#include <cdaptp.h>
#else
#include <cda_ptp.h>
#endif
#endif

/*
**  Table of contents
**
**	VMS specific global entry point names
*/
#if defined(__VMS) || defined(VMS)
struct FCT  *IMG$ERASE();
struct FCT  *IMG$GET();
struct FCT  *IMG$GET_FRAME_ATTRIBUTES();	/* Return frame attributes  */
struct FCT  *IMG$GET_FRAME_SIZE();
struct FCT  *IMG$SET();
struct FCT  *IMG$SET_FRAME_ATTRIBUTES();	/* Modify frame attributes  */
struct FCT  *IMG$SET_FRAME_SIZE();
#endif

/* 
**	Portable global entry point names
*/
#ifdef NODAS_PROTO
struct FCT  *ImgErase();
struct FCT  *ImgGet();
struct FCT  *ImgGetFrameAttributes();	/* Return frame attributes  */
struct FCT  *ImgGetFrameSize();
struct FCT  *ImgSet();
struct FCT  *ImgSetFrameAttributes();	/* Modify frame attributes  */
struct FCT  *ImgSetFrameSize();
#endif


/*
**  MACRO definitions
**
**	none
*/

/*
**  Equated Symbols
**
**	none
*/

/*
**  External References		    <---- from module ---->
*/
#ifdef NODAS_PROTO
struct	FCT *_ImgErase();	    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
struct	FCT *_ImgGet();		    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
long	     _ImgGetVerifyStatus();
struct	FCT *_ImgPut();		    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
void         _ImgLoadSelector();    /* IMG__ATTRIBUTE_ACCESS_UTILS  */
void	     _ImgVerifyStructure(); /* IMG__VERIFY_UTILS	    */

void	     ChfSignal();   /* from CHF$OBJLIB.OLB  */
void         ChfStop();	    /* from CHF$OBJLIB.OLB  */
#endif

/*
** External symbol definitions (status codes) CURRENTLY COMMENTED OUT
*/    
#include <img/ImgStatusCodes.h>

/*
**  Local Storage
**
**	none
*/

/******************************************************************************
**  IMG$ERASE
**  ImgErase
**
**  FUNCTIONAL DESCRIPTION:
**
**	Erase the value of a single item code from a frame.
**
**  FORMAL PARAMETERS:
**
**	fid		= Frame-id of frame to erase the attribute from.
**
**	selector	= Item selector code.
**
**      index           = index value for items with "ARRAY_OF_xxxx" type
**			  This argument is optional.
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
**	fid		= Frame-id of the frame passed in.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	Invalid argument count.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$ERASE( fid, selector, index )
struct FCT  *fid;
long	     selector;
long	     index;
{

return ( ImgErase( fid, selector, index ) );
} /* end of IMG$ERASE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgErase( fid, itemcode, index )
struct FCT	*fid;
long       	 itemcode;
long		 index;
{
struct ITEMCODE selector;

_ImgLoadSelector(itemcode,&selector);

/*
** Verify that the structure of the frame is OK.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
** Verify that the item code is not restricted at this level.
*/
if ( selector.ItmV_IslPrvt )
    ChfStop( 1,  ImgX_PVTITMCOD );	/* private item code trap   */
if ( selector.ItmV_Nowrite )
    ChfStop( 3,  ImgX_NOWRTACC, 1, itemcode );
if ( selector.ItmV_UserNowrite )
    ChfStop( 3,  ImgX_NOWRTACC, 1, itemcode );

/*
** Erase the item code data using the lower level routine.
*/
_ImgErase( fid, itemcode, index );

return fid;
} /* end of ImgErase */


/******************************************************************************
**  IMG$GET
**  ImgGet
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get a single item code from a frame.
**
**  FORMAL PARAMETERS:
**
**	fid		= Frame-id of frame to get item from.
**
**	selector	= ISL Item selector code
**
**	bufptr		= Address of the (receive) buffer into which
**			  the returned item is put.
**
**	buflen		= Length in bytes of the receive buffer.
**
**	retlen		= address of a word to receive the length
**			  in bytes of the returned data item.
**			  (Optional: not used if not present.)
**
**	index		= index value for an array-structured item.
**			  passed by value.
**			  optional (default = 0)
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
**	fid		= Frame-id of frame passed in.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	    Invalid argument count
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$GET( fid, selector, bufptr, buflen, retlen, index )
struct FCT  *fid;
long	     selector;
char	    *bufptr;
long	     buflen;
long	    *retlen;
long	     index;
{

return ( ImgGet(fid, selector, bufptr, buflen, retlen, index ) );
} /* end of IMG$GET */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgGet( fid, selector, bufptr, buflen, retlen, index )
struct FCT  *fid;
long	     selector;
char	    *bufptr;
long	     buflen;
long	    *retlen;
long	     index;
{

/*
** Verify that the structure of the frame is OK.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
** Get the item code data using the lower level routine.
*/
_ImgGet( fid, selector, bufptr, buflen, retlen, index );

return fid;
} /* end of ImgGet */


/************************************************************************
**  IMG$GET_FRAME_ATTRIBUTES
**  ImgGetFrameAttributes 
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return image frame attributes to the user based on items requested
**	in the item list.
**
**  FORMAL PARAMETERS:
**
**	fid		frame id of image frame to get attributes from
**      itmlst		pointer to GET_ITMLST structure
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
**      retfid		value of input parameter fid
**
**  SIGNALLED ERROR CODES:
**
**	ImgX_INVARGCNT	    Invalid number of arguments.
**	ImgX_PVTITMCOD	    Private item code, for ISL internal use only.
**
**  SIDE EFFECTS:
**
**      none
************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$GET_FRAME_ATTRIBUTES(fid,itmlst)
struct FCT *fid;
struct GET_ITMLST *itmlst[];
{
return ( ImgGetFrameAttributes( fid, itmlst ) );
} /* end of IMG$GET_FRAME_ATTRIBUTES */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgGetFrameAttributes(fid,itmlst)
struct FCT *fid;
struct GET_ITMLST *itmlst;
{
struct ITEMCODE	    item_code;
struct GET_ITMLST   *item;

/*
**  Verify contents of input frame structure.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
**  Go through each ITMLST entry and process user specified value.
*/
for (item = (struct GET_ITMLST *) itmlst; item->GetL_Code != 0; item++)
  {
    _ImgLoadSelector(item->GetL_Code,&item_code);
    
    if ( item_code.ItmV_IslPrvt == TRUE )
      ChfStop( 1,  ImgX_PVTITMCOD );	/* private item code trap   */
    else
      _ImgGet(fid,
	      item->GetL_Code,
	      item->GetA_Buffer,
	      item->GetL_Length,
	      (long *)item->GetA_Retlen,
	      item->GetL_Index );
  }

/*
**	Return frame id to user...
*/
return fid;
} /* end of ImgGetFrameAttributes */


/******************************************************************************
**  IMG$GET_FRAME_SIZE
**  ImgGetFrameSize
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function will get the frame size paramters in BMUs, translating
**	frame size information passed in in any of the following formats:
**
**		Frame resolution in DPI	    (Floating Point or Integer)
**		Frame size in inches	    (Floating Point)
**		Frame size in centimeters   (Floating Point)
**		Frame size in BMUs	    (Integer)
**
**	The frame size is determined by the difference between the upper
**	right coordinates and the lower left.  The lower left x and y
**	coordinates are set to zero, which means that the upper right
**	values are synonymous with the frame size.
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id.
**		    Passed by value.
**
**	x_size	    Size of image in X direction.
**		    Passed by reference.
**
**	Y_SIZE	    Size of image in Y direction.
**		    OPTIONAL.  If omitted, this parameter is set
**		    equal to the x_size parameter.
**		    Passed by reference.
**
**	TYPE	    Type of size parameter passed in for X and Y size.
**		    Valid symbolic values are:
**
**			    ImgK_ResolutionI	(dpi in integer)
**			    ImgK_ResolutionF	(dpi in floating point)
**			    ImgK_Inches	(floating point)
**			    ImgK_Centimeters	(floating point)
**			    ImgK_Bmus		(integer)
**
**		    OPTIONAL.  Default value in ImgK_ResolutionI
**		    Passed by value.
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
**	fid	    Frame id of input frame.
**		    Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT
**	ImgX_INVTYPCOD
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$GET_FRAME_SIZE( fid, x_size, y_size, type )
struct FCT  *fid;
float	    *x_size;
float	    *y_size;
long	     type;
{

return ( ImgGetFrameSize( fid, x_size, y_size, type ) );
} /* end of IMG$GET_FRAME_SIZE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgGetFrameSize( fid, x_size, y_size, type )
struct FCT  *fid;
float	    *x_size;
float	    *y_size;
long	     type;
{
long	 frm_box_ll_x;
long	 frm_box_ll_y;
long	 frm_box_ur_x;
long	 frm_box_ur_y;
long	 item;
long	 x_size_bmu;
long	 y_size_bmu;
long	 zero	    = 0;

if (type == 0)
    type = ImgK_Inches;

/*
** Get the frame size info (in BMUs) in the frame.
*/
_ImgGet( fid, Img_FrmBoxLLX, &frm_box_ll_x, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_FrmBoxLLY, &frm_box_ll_y, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_FrmBoxURX, &frm_box_ur_x, LONGSIZE, 0, 0 );
_ImgGet( fid, Img_FrmBoxURY, &frm_box_ur_y, LONGSIZE, 0, 0 );
x_size_bmu = frm_box_ur_x - frm_box_ll_x;
y_size_bmu = frm_box_ur_y - frm_box_ll_y;

/*
** Calculate frame size to return based on input type.
*/
switch( type )
    {
    case ImgK_ResolutionI:
	/*
	** Get pixels per line and line count.  Divide them by resolution
	** to get number of inches; then convert to BMUs.  NOTE that the
	** x and y size are recast as integer pointers.
	*/
	_ImgGet( fid, Img_PixelsPerLine, &item, LONGSIZE, 0, 0 );
	*((int *)x_size) = (float)item / ( (float)x_size_bmu / 1200.0 );
	_ImgGet( fid, Img_NumberOfLines, &item, LONGSIZE, 0, 0 );
	*((int *)y_size) = (float)item / ( (float)y_size_bmu / 1200.0 );
	break;
    case ImgK_ResolutionF:
	/*
	** Get pixels per line and line count.  Divide them by resolution
	** to get number of inches; then convert to BMUs.
	*/
	_ImgGet( fid, Img_PixelsPerLine, &item, LONGSIZE, 0, 0 );
	*x_size = item / ( x_size_bmu / 1200 );
	_ImgGet( fid, Img_NumberOfLines, &item, LONGSIZE, 0, 0 );
	*y_size = item / ( y_size_bmu / 1200 );
	break;
    case ImgK_Inches:
	/*
	** Convert size in BMUs to inches
	*/
	*x_size = (float)x_size_bmu / 1200.0;
	*y_size = (float)y_size_bmu / 1200.0;
	break;
    case ImgK_Centimeters:
	/*
	** Convert size in BMUs to inches, and then convert to centimeters.
	*/
	*x_size = ((float)x_size_bmu / 1200.0 ) * 2.54;
	*y_size = ((float)y_size_bmu / 1200.0 ) * 2.54;
	break;
    case ImgK_Bmus:
	/*
	** Use input size parameters as is, but recast them as integers.
	*/
	*((int *)x_size) = x_size_bmu;
	*((int *)y_size) = y_size_bmu;
	break;
    default:
	ChfStop( 1,  ImgX_INVTYPCOD );
    }

return fid;
} /* end of ImgGetFrameSize */


/******************************************************************************
**  IMG$SET
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set the value of a single item code in the frame.
**
**  FORMAL PARAMETERS:
**
**	fid		= Frame-id of frame to put item code into.
**                                                                	
**	itemcode	= Item selector code                                
**                                                                	
**	bufptr		= Address of the (source) buffer from which 	
**			  the item to be stored is gotten.		
**                                                                	
**	buflen		= Length in bytes of the source buffer. (Required)
**
**	index		= Index value of an array-structured item.
**			  This argument is optional, default = 0.
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
**	fid		= Frame-id of frame passed in.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	    Invalid argument count.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SET( fid, itemcode, bufptr, buflen, index )
struct FCT  *fid;
long             itemcode;
char		*bufptr;
long		 buflen;
long		 index;
{

return ( ImgSet( fid, itemcode, bufptr, buflen, index ) );
} /* end of IMG$SET */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgSet( fid, itemcode, bufptr, buflen, index )
struct FCT	*fid;
long             itemcode;
char		*bufptr;
long		 buflen;
long		 index;
{
struct ITEMCODE	 selector;

_ImgLoadSelector(itemcode,&selector);

/*
** Verify that the structure of the frame is OK.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
** Verify that the item code is not restricted at this level.
*/
if ( selector.ItmV_IslPrvt )
    ChfStop( 1,  ImgX_PVTITMCOD );	/* private item code trap   */
if ( selector.ItmV_Nowrite )
    ChfStop( 3,  ImgX_NOWRTACC, 1, itemcode );
if ( selector.ItmV_UserNowrite )
    ChfStop( 3,  ImgX_NOWRTACC, 1, itemcode );

/*
** Put the item code data using the lower level routine.
** Not that if the buffer address is zero or the buffer length is
** zero, the request is to be interpretted as an erase.
*/
if ( bufptr == NULL || buflen == 0 )
    _ImgErase( fid, itemcode, index );
else
    _ImgPut( fid, itemcode, bufptr, buflen, index );

return fid;
} /* end of ImgSet */


/************************************************************************
**  IMG$SET_FRAME_ATTRIBUTES
**  ImgSetFrameAttributes
**
**  FUNCTIONAL DESCRIPTION:
**
**      Set attributes in an existing frame structure from a user specified
**	item list.
**
**  FORMAL PARAMETERS:
**
**      fid		frame id, known internally as a pointer to a FCT block
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
**      retfid		returns the input frame id value
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT	Invalid number of arguments
**	ImgX_PVTITMCOD	ISL private item code.
**	ImgX_NOWRTACC	No write access for this item.
**
**  SIDE EFFECTS:
**
**	none
**
************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SET_FRAME_ATTRIBUTES( fid, itmlst )
struct FCT *fid;
struct PUT_ITMLST *itmlst[];
{

return ( ImgSetFrameAttributes( fid, itmlst ) );
} /* end of IMG$SET_FRAME_ATTRIBUTES */
#endif

/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgSetFrameAttributes( fid, itmlst )
struct FCT	    *fid;
struct PUT_ITMLST   *itmlst;
{
struct ITEMCODE	    item_code;
struct PUT_ITMLST   *item;

/*
**  Verify contents of input frame structure.
*/
if ( !VERIFY_OFF_ )
    _ImgVerifyStructure( fid );

/*
**  Go through each ITMLST entry and apply user specified value.
*/
for (item = itmlst; item->PutL_Code != 0; item++)
  {
    _ImgLoadSelector(item->PutL_Code,&item_code);
    if ( item_code.ItmV_IslPrvt )
	ChfStop( 1,  ImgX_PVTITMCOD );	/* private item code trap   */
    if ( item_code.ItmV_Nowrite )
	ChfStop( 3,  ImgX_NOWRTACC, 1, item->PutL_Code );
    if ( item_code.ItmV_UserNowrite )
	ChfStop( 3,  ImgX_NOWRTACC, 1, item->PutL_Code );
    else
	if ( item->PutA_Buffer == NULL || item->PutL_Length == 0 )
	    _ImgErase(	fid,
			item->PutL_Code,
			item->PutL_Index );
	else
	    _ImgPut(	fid,
			item->PutL_Code,
			item->PutA_Buffer,
			item->PutL_Length,
			item->PutL_Index );
    }

/*
**  Return frame id to user...
*/
return fid;
} /* end of ImgSetFrameAttributes */


/******************************************************************************
**  IMG$SET_FRAME_SIZE
**  ImgSetFrameSize
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function will set the frame size paramters in BMUs, translating
**	frame size information passed in in any of the following formats:
**
**		Frame resolution in DPI	    (Floating Point or Integer)
**		Frame size in inches	    (Floating Point)
**		Frame size in centimeters   (Floating Point)
**		Frame size in BMUs	    (Integer)
**
**	The frame size is determined by the difference between the upper
**	right coordinates and the lower left.  The lower left x and y
**	coordinates are set to zero, which means that the upper right
**	values are synonymous with the frame size.
**
**  FORMAL PARAMETERS:
**
**	fid	    Frame id.
**		    Passed by value.
**
**	x_size	    Size of image in X direction.
**		    Passed by reference.
**
**	Y_SIZE	    Size of image in Y direction.
**		    OPTIONAL.  If omitted, this parameter is set
**		    equal to the x_size parameter.
**		    Passed by reference.
**
**	TYPE	    Type of size parameter passed in for X and Y size.
**		    Valid symbolic values are:
**
**			    ImgK_ResolutionI	(dpi in integer)
**			    ImgK_ResolutionF	(dpi in floating point)
**			    ImgK_Inches	(floating point)
**			    ImgK_Centimeters	(floating point)
**			    ImgK_Bmus		(integer)
**
**		    OPTIONAL.  Default value in ImgK_ResolutionI
**		    Passed by value.
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
**	fid	    Frame id of input frame.
**		    Passed by value.
**
**  SIGNAL CODES:
**
**	ImgX_INVARGCNT
**	ImgX_INVTYPCOD
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
#if defined(__VMS) || defined(VMS)
struct FCT *IMG$SET_FRAME_SIZE( fid, x_size, y_size, type )
struct FCT  *fid;
float	*x_size;
float	*y_size;
long	 type;
{

return ( ImgSetFrameSize( fid, x_size, y_size, type ) );
} /* end of IMG$SET_FRAME_SIZE */
#endif


/*******************************************************************************
** Portable entry point
*******************************************************************************/
struct FCT *ImgSetFrameSize( fid, x_size, y_size, type )
struct FCT  *fid;
float	*x_size;
float	*y_size;
long	 type;
{
long	 item;
long	 x_size_bmu;
long	 y_size_bmu;
long	 zero	    = 0;

if (type == 0)
    type = ImgK_Inches;

if (y_size == 0)
    y_size = x_size;

/*
** Calculate frame size based on input type.
*/
switch( type )
    {
    case ImgK_ResolutionI:
	/*
	** Get pixels per line and line count.  Divide them by resolution
	** to get number of inches; then convert to BMUs.  NOTE that the
	** x and y size are recast as integer pointers.
	*/
	_ImgGet( fid, Img_PixelsPerLine, &item, sizeof(int), 0, 0 );
	x_size_bmu = ( item / *((int *)x_size) ) * 1200;
	_ImgGet( fid, Img_NumberOfLines, &item, sizeof(int), 0, 0 );
	y_size_bmu = ( item / *((int *)y_size) ) * 1200;
	break;
    case ImgK_ResolutionF:
	/*
	** Get pixels per line and line count.  Divide them by resolution
	** to get number of inches; then convert to BMUs.
	*/
	_ImgGet( fid, Img_PixelsPerLine, &item, sizeof(int), 0, 0 );
	x_size_bmu = ( item / *x_size ) * 1200;
	_ImgGet( fid, Img_NumberOfLines, &item, sizeof(int), 0, 0 );
	y_size_bmu = ( item / *y_size ) * 1200;
	break;
    case ImgK_Inches:
	/*
	** Convert size in inches to BMUs.
	*/
	x_size_bmu = *x_size * 1200.0;
	y_size_bmu = *y_size * 1200.0;
	break;
    case ImgK_Centimeters:
	/*
	** Convert size in CM to inches, and then convert to BMUs.
	*/
	x_size_bmu = (*x_size / 2.54 ) * 1200.0;
	y_size_bmu = (*y_size / 2.54 ) * 1200.0;
	break;
    case ImgK_Bmus:
	/*
	** Use input size parameters as is, but recast them as long integers.
	*/
	x_size_bmu = *((int *)x_size);
	y_size_bmu = *((int *)y_size);
	break;
    default:
	ChfStop( 1,  ImgX_INVTYPCOD );
    }

/*
** Store the frame size info (in BMUs) in the frame.
*/
_ImgPut( fid, Img_FrmBoxLLX, &zero, sizeof( long ), 0 );
_ImgPut( fid, Img_FrmBoxLLY, &zero, sizeof( long ), 0 );
_ImgPut( fid, Img_FrmBoxURX, &x_size_bmu, sizeof( long ), 0 );
_ImgPut( fid, Img_FrmBoxURY, &y_size_bmu, sizeof( long ), 0 );

return fid;
} /* end of ImgSetFrameSize */
