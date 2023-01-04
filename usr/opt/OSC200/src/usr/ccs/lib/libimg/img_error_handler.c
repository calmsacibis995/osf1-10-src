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
**  IMG_ERROR_HANDLER
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	This module contains the routine that handles error returns from
**  layer 2 routines
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Joe Mauro
**
**  CREATION DATE:
**
**	30-AUG-1989
**
************************************************************************/

/*
**  Table of contents
*/
#ifdef NODAS_PROTO
void	_ImgErrorHandler();	/* error routine    */
#endif


/*
**  Include Files
*/
#include <img/ChfDef.h>		    /* Condition Handling Facility  */
#include <img/IpsStatusCodes.h>	    /* Layer 2 error codes	    */

/*
**  Macro Definitions
*/

/*
**  Equated Symbols
*/


/*
**  External References from ISL                 <- from module ->
*/

/*
**  External References from Conditional Handling Facility
*/

#ifdef NODAS_PROTO
void		 ChfStop();			/* exception stop	*/
void		 ChfSignal();			/* exception signal	*/
#endif

/*
**	Status codes
*/
#include <img/ImgStatusCodes.h>         /* ISL Status Codes             */


/************************************************************************
**  _ImgErrorHandler
**
**  FUNCTIONAL DESCRIPTION: Handle error returns from layer 2 routines.
**  FORMAL PARAMETERS:	    vector  -	layer 2 routine identifier
**			    status  -	ISL status code
**  IMPLICIT INPUTS:	    none
**  IMPLICIT OUTPUTS:	    none
**  FUNCTION VALUE:	    none
**  SIGNAL CODES:	    all ISL status codes
**  SIDE EFFECTS:	    none
**
************************************************************************/
void _ImgErrorHandler(status)
int status;
{
if ((status != IpsX_SUCCESS) || (status != IpsX_NORMAL))
    {
    switch (status)
	{
	case IpsX_BUFOVRFLW:
	    ChfSignal(1, ImgX_IMGNOTCMP );
	    break;
	case IpsX_DCTCOMPIDXERR:
	    ChfStop(1, ImgX_DCTCMPIDX );
	    break;
	case IpsX_DCTDECODEFAIL:
	    ChfStop(1, ImgX_DCTDECFAI );
	    break;
	case IpsX_DCTENCODEFAIL:
	    ChfStop(1, ImgX_DCTENCFAI );
	    break;
	case IpsX_DCTFACTERR:
	    ChfStop(1, ImgX_DCTFACTER );
	    break;
	case IpsX_DLENGTR32:
	    ChfStop(1, ImgX_DLENGTR32);
	    break;
	case IpsX_DSTLENZER:
	    ChfStop(1, ImgX_DSTLENZER);
	    break;
	case IpsX_INCNSARG:
	    ChfStop(1, ImgX_INCNSARG);
	    break;
	case IpsX_INSVIRMEM:
	    ChfStop(1, ImgX_INSVIRMEM);
	    break;
	case IpsX_INVARGCON:
	    ChfStop(1, ImgX_INVARGCON);
	    break;
	case IpsX_INVCODTYP:
	    ChfStop(1, ImgX_INVCODTYP);
	    break;
	case IpsX_INVDARG:
	    ChfStop(1, ImgX_INVDARG);
	    break;
	case IpsX_INVDCPP:
	    ChfStop(1, ImgX_INCNSARG);
	    break;
	case IpsX_INVDMTHARG:
	    ChfStop(1, ImgX_INVDMTHARG);
	    break;
	case IpsX_INVDTHBIT:
	    ChfStop(1, ImgX_INVDTHBIT);
	    break;
	case IpsX_INVDTYPE:
	    ChfStop(1, ImgX_INVDTYPE);
	    break;
	case IpsX_INVSCNLEN:
	    ChfSignal(1, ImgX_INVSCNLEN);
	    break;
	case IpsX_NOINPLACE:
	    ChfStop(1, ImgX_NOINPLACE);
	    break;
	case IpsX_NOMATCH:
	    ChfStop(1, ImgX_NOMATCH);
	    break;
	case IpsX_SLCGTRDLC:
	    ChfStop(1, ImgX_SLCGTRDLC);
	    break;
	case IpsX_SLENGTR32:
	    ChfStop(1, ImgX_SLENGTR32);
	    break;
	case IpsX_SPCGTRDPC:
	    ChfStop(1, ImgX_SPCGTRDPC);
	    break;
	case IpsX_SPLGTRDPL:
	    ChfStop(1, ImgX_SPLGTRDPL);
	    break;
	case IpsX_SPLGTRDPS:
	    ChfStop(1, ImgX_SPLGTRDPS);
	    break;
	case IpsX_SRCLENZER:
	    ChfStop(1, ImgX_SRCLENZER);
	    break;
	case IpsX_UNSOPTION:
	    ChfStop(1, ImgX_UNSOPTION);
	    break;
	case IpsX_UNSPXLSTR:
	    ChfStop(1, ImgX_UNSPXLSTR);
	    break;
	default:
	    ChfStop(1, ImgX_FAILURE);	
	}/* end switch */    
    }/* end none success */;
}
