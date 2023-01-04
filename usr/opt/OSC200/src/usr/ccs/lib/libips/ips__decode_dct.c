/*  DEC/CMS REPLACEMENT HISTORY, Element SMIDECODEDCT.C */
/*  *11   23-JAN-1991 15:05:44 WEBER "Fix bigendian bugs" */
/*  *10    3-JAN-1991 09:27:05 GREBUS "Added big-endian fixed received from Congruent" */
/*  *9    18-DEC-1990 17:55:25 SHELLEY "ADD ERROR HANDING (IDS PRIORITY)" */
/*  *8     1-NOV-1990 10:50:41 SHELLEY "first pass adding error handling" */
/*  *7    12-OCT-1990 16:51:35 WEBER "IPS integration" */
/*  *6     8-OCT-1990 14:26:07 WEBER "Marge changes and debug" */
/*   4A1   2-OCT-1990 06:49:04 WEBER "Fix problem with MDU count" */
/*  *5    28-SEP-1990 12:51:45 SHELLEY "change SmiDecodeDctSegment interface" */
/*  *4    24-SEP-1990 15:53:27 WEBER "Fix free NULL pointer and ROI problems" */
/*  *3    21-SEP-1990 15:49:38 SHELLEY "fix pointer warning in ultrix" */
/*  *2    18-SEP-1990 08:09:05 WEBER "Integrate XIE/IPS" */
/*  *1    18-SEP-1990 08:00:19 WEBER "DCT compression/decompression sources" */
/*  DEC/CMS REPLACEMENT HISTORY, Element SMIDECODEDCT.C */

/***********************************************************
Copyright 1990-1991 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/******************************************************************************
**++
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      This module contains the interface to the DCT decompressesion routines.
**
**  ENVIRONMENT:
**
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHORS:
**
**	John Nadai
**	John Weber
**
**  CREATION DATE:
**
**	July 31, 1990
**
*******************************************************************************/

/*
**  INCLUDE FILES
**/
#include <math.h>
#include <string.h>

#if	defined(XIESMI)
#include <X.h>
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieUdpDef.h>
#include <SmiDctDef.h>
#include <SmiDctMacros.h>
#include <SmiDecodeDct.h>
#include <SmiMemMgt.h>
#elif	defined(IPS)
#include <IpsDef.h>
#include <IpsStatusCodes.h>
#include <IpsMacros.h>
#include <IpsDctDef.h>
#include <IpsDctMacros.h>
#include <IpsDecodeDct.h>
#if	defined(IPS)
#ifndef NODAS_PROTO
#include <ipsprot.h>		/* Ips prototypes */
#endif
#endif
#endif

/*
**  Table of Contents
*/
#ifdef NODAS_PROTO
#if	defined(XIESMI)
int			    SmiDecodeDct();
int			    SmiDecodeDctSegment();
DecodeDctStatePtr	    SmiDecodeDctInitializeState();
void			    SmiDecodeDctDestroyState();
#elif	defined(IPS)
long			    _IpsDecodeDct();
static long		    IpsSetUdp();
static int		    SmiDecodeDctSegment();
static DecodeDctStatePtr    SmiDecodeDctInitializeState();
static void		    SmiDecodeDctDestroyState();
#endif

static int		    DcdGetToken();

static int		    DcdDataAction();
static int		    DcdSosAction();
static int		    DcdSofAction();
static int		    DcdRscAction();

static void		    DcdDecode();
static short int	    DcdGetHuf();

static int		    DctUnstuffData();
static int		    DcdDecodeData();

static int		    IDCT8x8_16S_8U();
#else
#if	defined(XIESMI)
int			    SmiDecodeDct();
int			    SmiDecodeDctSegment();
DecodeDctStatePtr	    SmiDecodeDctInitializeState();
void			    SmiDecodeDctDestroyState();
#elif	defined(IPS)
PROTO(static long IpsSetUdp, (DecodeDctStatePtr /*state*/, int /*i*/));
PROTO(static int SmiDecodeDctSegment, (UdpPtr /*cdp*/, UdpPtr /*udplst*/[], int /*udpcnt*/, DecodeDctStatePtr /*state*/));
PROTO(static DecodeDctStatePtr SmiDecodeDctInitializeState, (void));
PROTO(static void SmiDecodeDctDestroyState, (DecodeDctStatePtr /*state*/));
#endif

PROTO(static int DcdGetToken, (DecodeDctStatePtr /*state*/));
PROTO(static int DcdDataAction, (DecodeDctStatePtr /*state*/));
PROTO(static int DcdSosAction, (DecodeDctStatePtr /*state*/));
PROTO(static int DcdSofAction, (DecodeDctStatePtr /*state*/));
PROTO(static int DcdRscAction, (DecodeDctStatePtr /*state*/));
PROTO(static int DcdDecodeData, (DecodeDctStatePtr /*state*/, unsigned char */*bufptr*/, int /*buflen*/));
PROTO(static void DcdDecode, (unsigned char */*inptr*/, int /*inlen*/, int */*bitoff*/, HuffDecodeTablePtr /*dc_table*/, HuffDecodeTablePtr /*ac_table*/, short int */*prv_dc*/, short int */*coef*/));
PROTO(static short int DcdGetHuf, (unsigned char */*bufptr*/, int /*buflen*/, int */*bufoff*/, HuffDecodeTable */*huff_decode_ptr*/));
PROTO(static void build_huffman_decoder, (unsigned char */*huff_spec*/, int */*huff*/));
PROTO(static int IDCT8x8_16S_8U, (short int */*COEFS*/, unsigned char */*PIXELS*/, int /*SCAN_WIDTH*/, int /*PEL_WIDTH*/));
PROTO(static int DctUnstuffData, (unsigned char */*srcptr*/, int /*srclen*/, unsigned char */*dstptr*/));
#endif

/*
**  External References
*/

/*
**  Equated symbols
*/
#define	DctK_NoComponent    0xFFFFFFFF

/*
**  Local MACROs
*/

/*  DEQUANT_ZIGZAG MACRO

       Inputs:  > 16-bit signed, quantized DCT coefficients 
                > organized as sequence of blocks (within blocks, the
                    coefficients stored in zigzag order)

       Outputs: > 16-bit signed, UNquantized DCT coefficients
                > organized as sequence of blocks (within blocks, the
                    coefficients stored by rows)
*/

#define ZQ_VARIABLES \
\
double d_dtemp ;\
short int *d_in_ptr,*d_out_ptr,*d_z_ptr;\
unsigned char *d_q_ptr ;\
int d_i ;

#define DEQUANT_ZIGZAG(IN_PT,OUT_PT,Z_PT,Q_PT) \
            d_in_ptr  = (short int *)(IN_PT) ;\
            d_out_ptr = (short int *)(OUT_PT) ;\
            d_q_ptr   = (unsigned char *)(Q_PT) ;\
            d_z_ptr   = (short int *)(Z_PT) ;\
	    for( d_i=0 ; d_i<=63 ; d_i++ )\
              *(d_out_ptr+(*d_z_ptr++)) = (*d_in_ptr++) * (*d_q_ptr++) ;

/*
**  Local storage
*/
    /*
    **	State transition table.
    */
static int next_state[DctK_StateMax][DctK_TokenMax] = {
    {DctK_StateErr, DctK_StateSOI, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateSOS, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateData, DctK_StateRSC, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateSOF, DctK_StateErr, DctK_StateErr,  DctK_StateRSC, DctK_StateEOI, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateData, DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
    {DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr, DctK_StateErr,  DctK_StateErr, DctK_StateErr, DctK_StateErr},
};
    /*
    **	Transition routine table.
    */
static int (*action[DctK_StateMax][DctK_TokenMax])() = {
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, DcdSofAction, NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, DcdSofAction, DcdSosAction, NULL,          NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         DcdDataAction, DcdRscAction, NULL, NULL},
    {NULL, NULL, DcdSofAction, NULL,         NULL,          DcdRscAction, NULL, NULL},
    {NULL, NULL, NULL,         NULL,         DcdDataAction, NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
    {NULL, NULL, NULL,         NULL,         NULL,          NULL,         NULL, NULL},
};

/*****************************************************************************
**  SmiDecodeDct
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
#if	defined(XIESMI)
int	    SmiDecodeDct(cdp,udplst,udpcnt)
#elif	defined(IPS)
long	    _IpsDecodeDct(cdp,udplst,udpcnt)
#endif
UdpPtr	    cdp;
UdpPtr	    udplst[];
int	    udpcnt;
{
    int			i;
    int			status;
    DecodeDctStatePtr	state;

    /*
    **	Verify input UDPs
    */
    if ((cdp->UdpB_Class != UdpK_ClassUBS) || (cdp->UdpB_DType != UdpK_DTypeVU))
	return (DctX_UnsOption);
    /*
    **	Initialize state, return if an error.
    */
    state = SmiDecodeDctInitializeState();
    if (state == NULL)
	return( DctX_BadAlloc );
    /*
    **	Decode data.
    */
    status = SmiDecodeDctSegment( cdp, udplst, udpcnt, state );
    /*
    **	Deallocate state block.
    */
    SmiDecodeDctDestroyState(state);
    /*
    **	Return based on state of decode.
    */
#if	defined(XIESMI)
    if( status > DctK_StatusDone )
        return( status );
#endif
    if (status != DctK_StatusDone)
        return (DctX_DecodeFail);
    else
        return (DctX_Success);
}

/*****************************************************************************
**  SmiDecodeDctSegment
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
#if	defined(IPS)
static
#endif
int		    SmiDecodeDctSegment(cdp,udplst,udpcnt,state)
UdpPtr		    cdp;
UdpPtr		    udplst[];
int		    udpcnt;
DecodeDctStatePtr   state;
{
    int		     token;
    unsigned char   *newptr;
    int		     newlen;
    unsigned char   *tmp;
    int		     i;
    /*
    **	Set target UDPs as destination in state block.
    */
    for (i = 0;  i < udpcnt;  i++)
    {
#ifdef IPS
	udplst[i]->UdpL_CompIdx = DctK_NoComponent;
#endif
        DcdDstUdp_(state,i) = udplst[i];
    }
    /*
    **	If new data present..
    */
    if (DcdNewDat_(state))
    {
	/*
	**  Calculate start and length of new data.
	*/
	newptr = cdp->UdpA_Base + cdp->UdpL_Pos / 8;
	newlen = (cdp->UdpL_ArSize - cdp->UdpL_Pos) / 8;
	/*
	**  Reset input exhausted state.
	*/
	DcdInpExh_(state) = FALSE;
	/*
	**  If data remains from last segment...
	*/
	if (DcdRemLen_(state) > 0)
	{
	    DcdRemBuf_(state) = (unsigned char *)
		DctRealloc_(DcdRemBuf_(state),DcdRemLen_(state) + newlen);
	    if (DcdRemBuf_(state) == NULL)  return DctX_BadAlloc;

	    memcpy(DcdRemBuf_(state) + DcdRemLen_(state),newptr,newlen);
	    DcdRemLen_(state) += newlen;

	    DcdCurPtr_(state) = DcdRemBuf_(state);
	    DcdCurRem_(state) = DcdRemLen_(state);
	}
	else
	{
	    DcdCurPtr_(state) = newptr;
	    DcdCurRem_(state) = newlen;
	}
	/*
	**  Reset new data flag
	*/
	DcdNewDat_(state) = FALSE;
    }

    /*
    **  If destination full last time, pick up from where we left off.
    */
    if (DcdDstFul_(state))
    {
	DcdDataAction(state);
	if (DcdDstFul_(state))
	    return DctK_StatusOutput;
    }
    /*
    **	While tokens remain in the buffer, process...
    */
    while (TRUE)
    {
	/*
	**  Attempt to get the next token.
	*/
        token = DcdGetToken(state);
	/*
	**  If input buffer exhausted.
	*/
        if (DcdInpExh_(state))
	{
	    /*
	    **  If any data remains, save it...
	    */
	    if (DcdCurRem_(state) > 0)
	    {
		/*
		**  Special case...if no complete token was found in new data
		**  no need to recopy....
		*/
		if (DcdCurPtr_(state) != DcdRemBuf_(state))
		{
		    if (DcdRemBuf_(state) != NULL)
			DctFree_(DcdRemBuf_(state));

		    DcdRemBuf_(state) = 
			(unsigned char *) DctMalloc_(DcdCurRem_(state));
		    if (DcdRemBuf_(state) == NULL)  return DctX_BadAlloc;
		    memcpy(DcdRemBuf_(state),DcdCurPtr_(state),DcdCurRem_(state));
		    DcdRemLen_(state) = DcdCurRem_(state);
		}
	    }
	    else
	    {
		/*
		**  No data remains...make sure remainder buffer is empty.
		*/
		if (DcdRemBuf_(state) != NULL)
		{
		    DctFree_(DcdRemBuf_(state));
		    DcdRemBuf_(state) = NULL;
		}
		DcdRemLen_(state) = DcdCurRem_(state);
	    }
	    DcdNewDat_(state) = TRUE;	    /* assume new data next time    */
	    return DctK_StatusInput;
	}
	/*
        **  Call transition action routine if specified.
	*/
        if (action[DcdState_(state)][token] != NULL)
          (*action[DcdState_(state)][token])(state);
	/*
	**  Transition to new state.
	*/
	DcdState_(state) = next_state[DcdState_(state)][token];
        /*	 
        **  If destination exhausted after transition, return now.
        */	 
        if (DcdDstFul_(state))	return	DctK_StatusOutput;
        /*	 
        **  If at an end state, stop
        */	 
        switch (DcdState_(state))
        {
            case DctK_StateErr:
                return DctK_StatusError;
            case DctK_StateEOI:
                return DctK_StatusDone;
            default:
                continue;
        }
    }
}

/*****************************************************************************
**  SmiDecodeDctInitializeState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
#if defined(IPS)
static
#endif
DecodeDctStatePtr   SmiDecodeDctInitializeState()
{
    int			i;
    /*
    **	Allocate new state block.
    */
    DecodeDctStatePtr state =
		(DecodeDctStatePtr) DctCalloc_(1,sizeof(DecodeDctStateRec));
    /*
    **	Return on bad alloc...
    */
    if (state == NULL) return NULL;
    /*
    **	Initialize global locations
    */
    DcdNewDat_(state) = TRUE;			    /* Always start with    */
						    /* new data		    */
    DcdHacTbl_(state,0) = &DctR_AcDecodeTable[0];   /* Default AC tables    */
    DcdHacTbl_(state,1) = &DctR_AcDecodeTable[1];
    DcdHdcTbl_(state,0) = &DctR_DcDecodeTable[0];   /* Default DC tables    */
    DcdHdcTbl_(state,1) = &DctR_DcDecodeTable[1];

    DcdQntTbl_(state,0) = &DctR_QuantDefault[0];    /* Default quantization */
    DcdQntDef_(state,0) = TRUE;			    /* tables.		    */
    DcdQntTbl_(state,1) = &DctR_QuantDefault[1];    /*  		    */
    DcdQntDef_(state,1) = TRUE;
    /*
    **	Return state block pointer.
    */
    return  state;
}

/*****************************************************************************
**  SmiDecodeDctDestroyState
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
#if defined(IPS)
static
#endif
void		    SmiDecodeDctDestroyState(state)
DecodeDctStatePtr   state;
{
    int	    i;
    UdpPtr  udp;

#if	defined(XIESMI)
    if( !IsPointer_(state) ) return;
#endif
    /*
    **	Free any user defined quantization tables.
    */
    for (i = 0;  i < DctK_MaxQuantTables;  i++)
#if	defined(XIESMI)
        if ((!DcdQntDef_(state,i)) && (IsPointer_(DcdQntTbl_(state,i))))
#elif	defined(IPS)
        if ((!DcdQntDef_(state,i)) && (DcdQntTbl_(state,i) != NULL))
#endif
            DctFree_(DcdQntTbl_(state,i));

    /*
    **	Free the state block.
    */
    DctFree_(state);
}

/*****************************************************************************
**  DcdGetToken
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	    DcdGetToken(state)
DecodeDctStatePtr   state;
{
    unsigned char   *ptr = DcdCurPtr_(state);
    int		     remaining = DcdCurRem_(state);
    int		     token;
    unsigned char   *token_ptr;
    short	     token_len;
    int		     marker;
    /*
    **	Make sure there's data present.
    */
    if (remaining <= 0)
    {
	DcdInpExh_(state) = TRUE;
	return DctK_TokenNone;
    }
    /*
    **  Determine if this is a marker token or data token.
    */
    if (*ptr == DctK_Marker)
    {
	/*
	**	This is a marker token, skip marker bytes until we reach
	**	the marker code.
	*/
	while (*ptr == DctK_Marker)
	{
	    *ptr++; remaining--;
	    if (remaining <= 0)
	    {
		DcdInpExh_(state) = TRUE;
		return DctK_TokenNone;
	    }
	}
	/*
	**	Read marker code, translate into respective token.
	*/
	remaining--;
	marker = *ptr++;
	switch (marker)
	{
	    case DctK_MarkerSOI:
		token = DctK_TokenSOI;
		break;
	    case DctK_MarkerEOI:
		token = DctK_TokenEOI;
		break;
	    case DctK_MarkerRSC0:
	    case DctK_MarkerRSC1:
	    case DctK_MarkerRSC2:
	    case DctK_MarkerRSC3:
	    case DctK_MarkerRSC4:
	    case DctK_MarkerRSC5:
	    case DctK_MarkerRSC6:
	    case DctK_MarkerRSC7:
		token = DctK_TokenRSC;
		break;
	    case DctK_MarkerSOF:
		token = DctK_TokenSOF;
		break;
	    case DctK_MarkerSOS:
		token = DctK_TokenSOS;
		break;
	    default:
		return DctK_TokenError;
	}
	/*	 
	**  If this token includes data, establish that it is contained
	**	within this buffer.
	*/	 
	if (token == DctK_TokenSOS || token == DctK_TokenSOF)
	{
	    /*
	    **  Next two bytes specify the length of additional data. Make
	    **  sure they're available.
	    */
	    if (remaining < 2)
	    {
		DcdInpExh_(state) = TRUE;
		return DctK_TokenNone;
	    }
	    /*	 
	    **  Read the token length and make sure there's enough data
	    **  remaining in the buffer.
	    */	 
	    token_len = *ptr++;
	    token_len += (*ptr++ << 8);
	    if (remaining < token_len)
	    {
		DcdInpExh_(state) = TRUE;
		return DctK_TokenNone;
	    }
	    token_ptr = ptr;
	    ptr += token_len - 2;
	    remaining -= token_len;
	}
	else
	{
	    token_len = 0;
	    token_ptr = NULL;
	}
    }
    else
    {
	/*	
	**	This is a data segment...
	*/
	token = DctK_TokenData;
	token_ptr = ptr;
	for (token_len = 0; remaining > 0; token_len++, remaining--, ptr++)
	    if (*ptr == DctK_Marker)
		if (remaining > 1)
		    if (*(ptr+1) != 0)
			break;
	/*
	**  At this point we're either pointing to the next marker, or we're
	**  out of data. If out of data, return now...
	*/
	if (remaining <= 0)
	{
	    DcdInpExh_(state) = TRUE;
	    return DctK_TokenNone;
	}
    }
    /*
    **	Update state with new source pointer and current token information.
    */
    DcdToken_(state) = token;
    DcdTokPtr_(state) = token_ptr;
    DcdTokLen_(state) = token_len;
    DcdCurPtr_(state) = ptr;
    DcdCurRem_(state) = remaining;
    /*
    **	Return token just parsed.
    */
    return  token;
}

/*****************************************************************************
**  DcdDataAction
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	    DcdDataAction(state)
DecodeDctStatePtr   state;
{
    int	    length;
    int	    i;
    UdpPtr  udp;
    long    status;
    /*
    **	Check the output UDPs. If pointer is NULL, allocate a new UDP for this
    **	segment. Otherwise, make sure the destination UDP can accept the output
    **	of the current segment. If not, set destination full and return before
    **	decompressing this segment.
    */
    for (i = 0;  i < DcdCmpCnt_(state);  i++)
    {
	udp = DcdDstUdp_(state,i);
#if	defined(XIESMI)
        if (udp == NULL)
            return DcdK_ActionError;
        else
#elif	defined(IPS)
	status = IpsSetUdp(state,DcdDstIdx_(state,i));
	if (status != DctX_Success)
	    return status;
#endif
        if ((udp->UdpL_Y2 < (DcdCurY_(state,i) + DcdScnSeg_(state,i) - 1) ) &&
	    (udp->UdpL_Y2 < (DcdCmpHgt_(state,i) - 1)))
            DcdUdpFul_(state,DcdDstIdx_(state,i)) = TRUE;
        else
            DcdUdpFul_(state,DcdDstIdx_(state,i)) = FALSE;
    }

    if (DcdDstFul_(state))
	return DcdK_ActionSuccess;
    /*
    **	Now, remove "stuff" bytes from data and decompress it.
    */
    length = DctUnstuffData(DcdTokPtr_(state),	    /* Source data pointer  */
			    DcdTokLen_(state),	    /* Source data length   */
			    DcdTokPtr_(state));	    /* Destination data	    */
						    /*  pointer		    */
    DcdDecodeData(state,DcdTokPtr_(state),length);

    return DcdK_ActionSuccess;
}

/*****************************************************************************
**  DcdSosAction
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	    DcdSosAction(state)
DecodeDctStatePtr   state;
{
    unsigned char   *ptr = DcdTokPtr_(state);
    unsigned char    len = DcdTokLen_(state);
    int		     current_component;
    unsigned char    huff_specifier;
    int		     huff_idx;
    int		     huff_sel;
    int		     i;
    /*
    **	Reset resync enable for each scan.
    */
    DcdRstEnb_(state) = FALSE;
    /*
    **	First byte specifies the component to be worked on.
    */
    current_component = *ptr++;
    /*
    **	Read per component huffman table assignments. One byte per component.
    **	The DC index is in the low four bits, AC index in the high four bits.
    */
    for (i = 0;  i < DcdCmpCnt_(state);  i++)
    {
        DcdCmpHdc_(state,i) = *ptr & 0x0F;
	DcdCmpHac_(state,i) = (*ptr++ >> 4) & 0x0F;
    }
    /*
    **	Read huffman table specifiers. Since compression doesn't generate
    **	custom huffman tables, ignore that capability for now...
    */
    while ((huff_specifier = *ptr++) != 0x80)
    {
	/*
	**  The huffman specifier contains the huffman table index in the low
	**  nibble, and selector in the high nibble.
	*/
        huff_idx = huff_specifier & 0x0F;
	huff_sel = (huff_specifier >> 4) & 0x0F;
        /*	 
        **  Take action based on selector value.
        */	 
        switch (huff_sel)
        {
	    /*
	    **	These cases call for loading private tables...not implemented.
	    */
            case 0:
            case 1:
                return DcdK_ActionError;
	    /*
	    **	Load default DC huffman table.
	    */
            case 2:
                DcdHdcTbl_(state,huff_idx) = &DctR_DcDecodeTable[0];
		DcdSdcTbl_(state,huff_idx) = &DctR_DcSpecTable[0];
		break;
	    /*
	    **	Load default AC huffman table.
	    */
            case 3:
                DcdHacTbl_(state,huff_idx) = &DctR_AcDecodeTable[0];
                DcdSacTbl_(state,huff_idx) = &DctR_AcSpecTable[0];
		break;
	    /*
	    **	Anything else is RIGHT OUT...
	    */
            default:
                return DcdK_ActionError;
        }
    }
    /*
    **	Reset horizontal and vertical interleave counters.
    */
    DcdHrzCnt_(state) = 0;
    DcdVrtCnt_(state) = 0;

    return DcdK_ActionSuccess;
}

/*****************************************************************************
**  DcdSofAction
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	    DcdSofAction(state)
DecodeDctStatePtr   state;
{
    unsigned char   *ptr = DcdTokPtr_(state);
    unsigned char    len = DcdTokLen_(state);
    int		     vert_max = 0;
    int		     horz_max = 0;
    int		     i;
    int		     j;
    char	     qntidx;
    char	     qntprc;
    /*
    **	Parse SOF fixed header fields.
    */
    DcdMode_(state)    = *ptr++;
    DcdPrec_(state)    = *ptr++;
    DcdFrmHgt_(state)  = *ptr++;
    DcdFrmHgt_(state) += *ptr++ << 8;
    DcdFrmWid_(state)  = *ptr++;
    DcdFrmWid_(state) += *ptr++ << 8;
    DcdRstInt_(state)  = *ptr++;
    DcdRstInt_(state) += *ptr++ << 8;
    /*
    **	Get per component information.
    */
    DcdCmpCnt_(state)  = *ptr++;
    for (i = 0;  i < DcdCmpCnt_(state);  i++)
    {
	/*
	**  Get component index from DCT stream. Determine which UDP
	**  corresponds to this component.
	*/
        DcdCmpIdx_(state,i) = *ptr++;

        for (j = 0;  j < DcdCmpCnt_(state);  j++)
            if (DcdDstUdp_(state,j)->UdpL_CompIdx == DcdCmpIdx_(state,i))
		break;
	    else if (DcdDstUdp_(state,j)->UdpL_CompIdx == DctK_NoComponent)
	    {
		DcdDstUdp_(state,j)->UdpL_CompIdx = DcdCmpIdx_(state,i);
		break;
	    }

        if (j < DcdCmpCnt_(state))
            DcdDstIdx_(state,i) = j;
        else
            return  DcdK_ActionError;
	/*
	**  Get sampling rates.
	*/
	DcdCmpVrt_(state,i) = *ptr & 0x0F;
	DcdCmpHrz_(state,i) = (*ptr++ >> 4) & 0x0F;
	
        if (DcdCmpVrt_(state,i) > vert_max) vert_max = DcdCmpVrt_(state,i);
        if (DcdCmpHrz_(state,i) > horz_max) horz_max = DcdCmpHrz_(state,i);
    }
    /*
    **	Calculate sizes for interleaving.
    */
    DcdIntCol_(state) = (DcdFrmWid_(state)+(8*horz_max-1)) / (8*horz_max);
    DcdIntLin_(state) = (DcdFrmHgt_(state)+(8*vert_max-1)) / (8*vert_max);
    /*
    **	Now, compute component sizes based on subsampling frequency.
    */
    for (i = 0;  i < DcdCmpCnt_(state);  i++)
    {
        DcdCmpWid_(state,i) = DcdFrmWid_(state)*DcdCmpHrz_(state,i)/horz_max;
        DcdCmpHgt_(state,i) = DcdFrmHgt_(state)*DcdCmpVrt_(state,i)/vert_max;
	DcdCmpStr_(state,i) = DcdIntCol_(state) * 8 * DcdCmpHrz_(state,i);
	DcdCmpLen_(state,i) = DcdIntLin_(state) * 8 * DcdCmpVrt_(state,i);
	/*
	**  Initially, scans per segement is entire image (for no resync).
	*/
	DcdScnSeg_(state,i) = DcdCmpHgt_(state,i);
    }
    /*
    **	Next, extract per component quantization table assignments. These values
    **	are four bits each packed two per byte.
    */
    switch (DcdCmpCnt_(state))
    {
        case 3:
            DcdCmpQnt_(state,2) = *(ptr+1) & 0x0F;
        case 2:
            DcdCmpQnt_(state,1) = (*ptr >> 4) & 0x0F;
        case 1:
            DcdCmpQnt_(state,0) = *ptr & 0x0F;
	    break;
        case 4:
        default:
            return DcdK_ActionError;
    }
    ptr += (DcdCmpCnt_(state) + 1) / 2;
    /*
    **	Now, comes quantization table specifiers. 
    */
    while (*ptr++ != 0x80)
    {
        qntidx = *(ptr-1) & 0x0F;
	qntprc = *(ptr-1) >> 4 & 0x0F;
	/*
	**  If this slot contains a non-default quantization table, deallocate
	**  the memory.
	*/
	if (DcdQntTbl_(state,qntidx) != NULL && !DcdQntDef_(state,qntidx))
	    DctFree_(DcdQntTbl_(state,qntidx));
	/*
	**  Read in user defined quantization table.
	*/
	DcdQntTbl_(state,qntidx) = 
	    (QuantTablePtr) DctMalloc_(sizeof(QuantTable));
        if (DcdQntTbl_(state,qntidx) == NULL) return DctX_BadAlloc;

	memcpy(DcdQntTbl_(state,qntidx),ptr,sizeof(QuantTable));
	ptr += sizeof(QuantTable);
    }
    /*
    **	Now, validate what we found in the JPEG stream against what's being
    **	specified by the caller.
    */

    /*
    **	Return from wence we came...
    */
    return DcdK_ActionSuccess;
}

/*****************************************************************************
**  DcdRscAction
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	    DcdRscAction(state)
DecodeDctStatePtr   state;
{
    int	i;
    
    DcdRstEnb_(state) = TRUE;

    for (i = 0;  i < DcdCmpCnt_(state);  i++)
    {
	/*
	**  NOTE : Sampling rate needs to be considered here...
	*/
	DcdScnSeg_(state,i) =
	    ((DcdRstInt_(state) * DctK_BlockSize)
	    / DcdCmpWid_(state,i)) * DctK_BlockSize;
    }

    return  DcdK_ActionSuccess;
}

/*****************************************************************************
**  DcdDecodeData
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int	     DcdDecodeData(state,bufptr,buflen)
DecodeDctStatePtr    state;
unsigned char	    *bufptr;
int		     buflen;
{
    ZQ_VARIABLES

    int		     h, v, i, c;
    short int	     work1[64];
    short int	     work2[64];

    int		     scn_stride[DctK_MaxComponents];
    int		     pix_stride[DctK_MaxComponents];
    unsigned char   *base[DctK_MaxComponents];
    unsigned char   *scnptr;
    unsigned char   *pxlptr;
    int		     bufoff = 0;
    UdpPtr	     udp;
    short int	     previous_dc[DctK_MaxComponents];

    unsigned char    tmp[64];
    int		     dstrow;
    int		     dstcol;
    int		     x;
    int		     y;
    int		     row;
    int		     col;
    unsigned char   *tmpptr;
    unsigned char   *dstptr;
    /*              
    **  Assign local pointers to speed things up inside loops
    */
    for(c = 0; c < DcdCmpCnt_(state); c++) 
    {
	udp = DcdDstUdp_(state,DcdDstIdx_(state,c));

	scn_stride[c] = udp->UdpL_ScnStride / 8;
	pix_stride[c]  = udp->UdpL_PxlStride / 8;
	base[c] = udp->UdpA_Base + udp->UdpL_Pos / 8;
	previous_dc[c] = 0;
    }
    /*
    **	This is the main decompression loop...once around for each MDU in this
    **	code segment.
    */
    for (i = 0;  i < DcdRstInt_(state);  i++)
    {
	/*
	**  For each component in the MDU
	*/
    	for (c = 0;  c < DcdCmpCnt_(state);  c++)
	{
	    x = DcdCurX_(state,c);
	    y = DcdCurY_(state,c);
	    udp = DcdDstUdp_(state,DcdDstIdx_(state,c));
	    /*
	    **	Calculate address to start of tile.
	    */
	    scnptr = base[c] +
		     (y - udp->UdpL_Y1) * scn_stride[c] +
		     (x - udp->UdpL_X1) * pix_stride[c];
	    /*
	    **	For each vertical sample of this component.
	    */
	    for (v = 0;  v < DcdCmpVrt_(state,v);  v++)
	    {
	        pxlptr = scnptr;
		/*
		**  For each horizontal sample in this component.
		*/
	    	for (h = 0;  h < DcdCmpHrz_(state,v);  h++)
		{
		    /*
		    **	Turn huffman codes into quantized coefficients.
		    */
		    DcdDecode(DcdTokPtr_(state),
			      DcdTokLen_(state),
			      &bufoff,
			      (HuffDecodeTablePtr)DcdHdcTbl_(state,DcdCmpHdc_(state,c)),
			      (HuffDecodeTablePtr)DcdHacTbl_(state,DcdCmpHac_(state,c)),
			      &previous_dc[c],
			      work1);
		    /*
		    **	Dequantize and put coefficients in raster order.
		    */
		    DEQUANT_ZIGZAG(work1,
				   work2,
				   DctAW_ZigZag,
				   DcdQntTbl_(state,DcdCmpQnt_(state,c)));
		    /*
		    **	If this block goes beyond the limits of the image,
		    **	buffer it in a temporary, then copy into the image
		    **	buffer.
		    */
                    if (((x + DctK_BlockSize - 1) > udp->UdpL_X2) || 
			((y + DctK_BlockSize - 1) > udp->UdpL_Y2))
                    {
                        IDCT8x8_16S_8U(work2,tmp,8,1);
			/*
			**  Calculate remaining rows and columns...if greater
			**  than 8, then set to maximum.
			*/
			dstrow = udp->UdpL_Y2 - y + 1;
                        if (dstrow > 8) dstrow = 8;
			dstcol = udp->UdpL_X2 - x + 1;
                        if (dstcol > 8) dstcol = 8;
			/*
			**  Copy from temp buffer into image buffer.
			*/
                        for (row = 0;  row < dstrow;  row++)
                        {
			    tmpptr = tmp + 8 * row;
			    dstptr = pxlptr + scn_stride[c] * row;
                            for (col = 0;  col < dstcol;  col++)
                            {
                                *dstptr  = *tmpptr++;
				 dstptr += pix_stride[c];
                            }
                        }
                    }
                    else
                    {
			/*
			**  Do inverse DCT directly into image buffer.
			*/
			IDCT8x8_16S_8U(work2,pxlptr,scn_stride[c],pix_stride[c]);
                    }
		    /*
		    **	Update pointer to next horizontal sample.
		    */
		    pxlptr += (8 * pix_stride[c]);
		    x += 8;
		}
		/*
		**  Update pointer to next vertical sample.
		*/
		scnptr += (8 * scn_stride[c]);
		y += 8;
	    }
	    /*
	    **	Update current X,Y location pointers. If at end of scan, reset
	    **	X pointer and increment Y pointer.
	    */
	    DcdCurX_(state,c) += (8 * DcdCmpHrz_(state,c));
	    if (DcdCurX_(state,c) >= DcdCmpWid_(state,c))
	    {
	    	DcdCurX_(state,c) = 0;
		DcdCurY_(state,c) += (8 * DcdCmpVrt_(state,c));
		if (DcdCurY_(state,0) >= DcdCmpHgt_(state,0))
		    break;
	    }
	}
        if (DcdCurY_(state,0) >= DcdCmpHgt_(state,0))
	    break;
    }

    return(DcdK_ActionSuccess) ;
} 

/*  
**  DECODE : 
**	Huffman decodes one block of quantized zigzag scanned DCT coefs.
**	- most bit banging is done by the macros GetBit_ and GET_VALUE_
*/
static void	 DcdDecode(inptr,inlen,bitoff,dc_table,ac_table,prv_dc,coef)
unsigned char	    *inptr;
int		     inlen;
int		    *bitoff;
HuffDecodeTablePtr   dc_table;
HuffDecodeTablePtr   ac_table;
short int	    *prv_dc;
short int	    *coef;
{
    int		 i, coef_index;
    int		 dc_exp, dc_man;
    int		 ac_exp, ac_man, ac_run;

    int		 temp;
    short int	 stemp;
    /*
    **	Decode DC coefficient of block
    */
    dc_exp = DcdGetHuf(inptr,inlen,bitoff,dc_table);

    if (dc_exp & 15)
    {
#ifndef MSB_FIRST
	dc_man = GET_VALUE_(inptr,*bitoff,(1<<(dc_exp-1))-1); *bitoff+=(dc_exp-1);
	temp = GetBit_(inptr,*bitoff); (*bitoff)++;
#else
	temp = GetBit_(inptr,*bitoff); (*bitoff)++;
	dc_man = GET_VALUE_(inptr,*bitoff,(1<<(dc_exp-1))-1); *bitoff+=(dc_exp-1);
#endif
	if (temp == 0) 
	    dc_man += ( (-1<<dc_exp)+1 );
	else
	    dc_man += (   1<<(dc_exp-1)) ;
    }
    else if (dc_exp == 0)
	dc_man = 0 ;
    else
	dc_man = -32768 ; /* i.e. dc_exp == 16 */

    *coef = dc_man + (*prv_dc) ;
    (*prv_dc) = *coef++ ;

    /* Decode AC coefficients of block   */

    /* 
    **  coef_index points to the NEXT coef to be processed
    */
    for (coef_index = 1; coef_index <= 63; ) 
    {
	stemp = DcdGetHuf(inptr,inlen,bitoff,ac_table);

	ac_run = stemp >> 4;
	ac_exp = stemp & 15;	    /* stemp - 16 * ac_run ; */

	if (ac_exp == 0)	    /* The 2 special codes  */
	{
	    if(ac_run)		    /* Max-length-run (i.e. ac_run==15)  */
		for(i = 0; i <= 15; i++,coef_index++)  
		    *coef++ = 0;
	    else		    /* End-of-block (i.e.ac_run==0)      */
		for( ;coef_index <= 63; coef_index++) 
		    *coef++ = 0;
	}
	else		    /* Regular codes  */
	{
#ifndef MSB_FIRST
	    ac_man = GET_VALUE_(inptr,*bitoff,(1<<(ac_exp-1))-1); 
	    *bitoff += (ac_exp - 1);
	    temp = GetBit_(inptr,*bitoff); (*bitoff)++;
#else
	    temp = GetBit_(inptr,*bitoff); (*bitoff)++;
	    ac_man = GET_VALUE_(inptr,*bitoff,(1<<(ac_exp-1))-1); 
	    *bitoff += (ac_exp - 1);
#endif
	    if (temp == 0)
		ac_man += ( (-1<<ac_exp)+1 );
	    else
		ac_man += (   1<<(ac_exp-1));

	    for(i = 0; i < ac_run; i++, coef_index++) 
		*coef++ = 0;
	    *coef++ = ac_man;  
	     coef_index++;
	}

    } /* Done with a block  */
    return ;
}

/*
**  DcdGetHuf 
**
**  Fetches the next Huffman code from a bit stream pointed to by BIT_STREAM,
**  of a set of codes specified by HUFF_DECODE.  BITS_AVAILABLE specifies the
**  number of bits available to be read from the current byte,  and is updated
**  after reading.  BYTE_COUNT is the number of bytes whose last bit was read
**  during this call to GETHUF.
*/

static short int    DcdGetHuf(bufptr,buflen,bufoff,huff_decode_ptr) 
unsigned char	    *bufptr;
int		     buflen;
int		    *bufoff;
HuffDecodeTable	    *huff_decode_ptr;
{
    int	    temp;
    int	    index = 0;
    short   rlen;

    do
    {
	temp = GetBit_(bufptr,*bufoff); (*bufoff)++;
	if (temp)				    /* i.e., if temp==1 */
	    index += huff_decode_ptr->half_table_size;
	rlen  = huff_decode_ptr->huff_decode_entry[index].w1;
	index = huff_decode_ptr->huff_decode_entry[index].w0;
    } while (index);
						    /* i.e., != 0       */

    return(rlen);
}

/* 
**  BUILD HUFFMAN DECODER: 
**	Builds a Huff decode table from a compressed Huff table specification.
*/              
static void	 build_huffman_decoder(huff_spec,huff) 
unsigned char   *huff_spec ;
int             *huff ;
{
    int           i, j, k  ;
    int           *huff_decode ;
    unsigned char *sorted_symbols, sizo[256] ;
    int           half_table, temp, next, index, curr_code, curr_size ;

    half_table = *huff_spec++ ;

    /* Generate array of codelengths   */

    for(i = 1, k = 0; i <= 16; i++, huff_spec++)
        for(j = 1; j <= *huff_spec; j++) 
	    sizo[k++] = i;

    sorted_symbols = huff_spec ;

    /* First word is the number of symbols = half the table size  */

    huff[0] = half_table ;

    /* Decode table follows the symbol count  */

    huff_decode = &huff[1] ;

    /* Build decoder table:   */

    next = 0 ;
    curr_code = -1 ;
    curr_size = sizo[0] ;

    for(i=0; i<=half_table-1; i++)
    {
	/* 
	**  Get codelength and calculate code for next most frequent symbol
	*/
	curr_code++ ;
	if(sizo[i] != curr_size)
	{
	    curr_code = (curr_code << sizo[i]-curr_size) ;	  
	    curr_size = sizo[i] ;
	}
        /* 
	** Enter links into decode table to allow bit-by-bit decoding of
	** this symbol.
        */
	index = 0;
	temp = curr_code << (32-curr_size);

	for(j = 1; j <= curr_size - 1; j++)
	{
	    if (temp < 0) 
		index += half_table;

	    if (huff_decode[index] != 0)
		index = huff_decode[index];
	    else
	    {
	        next++;
	        huff_decode[index] = next;
	        index = next;
	    }
	    temp <<= 1 ;
	}

	if (temp < 0)
	    index += half_table ;

	/* enter symbol into decode table; link is zero   */

	huff_decode[index] = (((int)sorted_symbols[i])<<16) ;
    }
    return ;
}

/*
**  Inverse DCT routine.
*/
static int	 IDCT8x8_16S_8U(COEFS,PIXELS,SCAN_WIDTH,PEL_WIDTH)
short int	*COEFS;
unsigned char	*PIXELS;
int		 SCAN_WIDTH;
int		 PEL_WIDTH;
{
    DCT_VARIABLES

    d_ps1 = COEFS;
    d_pi1 = d_work;

    for(d_k=0 ; d_k<8; d_k++)
    {
	 d_ap = d_a;
	*d_ap++ = (d_ps1[0])<<2;
	*d_ap++ = (d_ps1[4*8])<<2;
	*d_ap++ = (d_ps1[2*8])<<2;
	*d_ap++ = (d_ps1[6*8])<<2;
	*d_ap++ = ((d_spi16*d_ps1[1*8]  - d_s7pi16*d_ps1[7*8]) + (1<<13))>>14;
	*d_ap++ = ((d_s5pi16*d_ps1[5*8] - d_s3pi16*d_ps1[3*8]) + (1<<13))>>14;
	*d_ap++ = ((d_c3pi16*d_ps1[3*8] + d_c5pi16*d_ps1[5*8]) + (1<<13))>>14;
	*d_ap   = ((d_cpi16*d_ps1[1*8]  + d_c7pi16*d_ps1[7*8]) + (1<<13))>>14;
	 d_ps1++;

	 d_bp = d_b;
	*d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16;
	*d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16;
	*d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = d_a[4] + d_a[5];
	*d_bp++ = d_a[4] - d_a[5];
	*d_bp++ = d_a[7] - d_a[6];
	*d_bp   = d_a[7] + d_a[6];

	 d_cp = d_c;
	*d_cp++ = d_b[0] + d_b[3];
	*d_cp++ = d_b[1] + d_b[2];
	*d_cp++ = d_b[1] - d_b[2];
	*d_cp++ = d_b[0] - d_b[3];
	*d_cp++ = d_b[4];
	*d_cp++ = (d_cpi4*(d_b[6]-d_b[5])        + 32768)>>16;
	*d_cp++ = (d_cpi4*(d_b[6]+d_b[5])        + 32768)>>16;
	*d_cp   = d_b[7];

	 d_pi2 = d_c;
	 d_pi3 = d_c + 7;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2++) + (*d_pi3--)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2  ) + (*d_pi3  )   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2--) - (*d_pi3++)   + 1) >> 1;
	*d_pi1++ = ((*d_pi2  ) - (*d_pi3  )   + 1) >> 1;
    }

    d_pi2 = d_work;
    d_pi4 = d_c;
    d_pi5 = d_c + 7;
    d_puc2 = PIXELS;

    for(d_k=0 ; d_k <8; d_k++)
    {
	 d_ap = d_a;
	*d_ap++ = d_pi2[0];
	*d_ap++ = d_pi2[4*8];
	*d_ap++ = d_pi2[2*8];
	*d_ap++ = d_pi2[6*8];
	*d_ap++ = (d_spi16*d_pi2[1*8]  - d_s7pi16*d_pi2[7*8] + 32768)>>16;
	*d_ap++ = (d_s5pi16*d_pi2[5*8] - d_s3pi16*d_pi2[3*8] + 32768)>>16;
	*d_ap++ = (d_c3pi16*d_pi2[3*8] + d_c5pi16*d_pi2[5*8] + 32768)>>16;
	*d_ap   = (d_cpi16*d_pi2[1*8]  + d_c7pi16*d_pi2[7*8] + 32768)>>16;
	 d_pi2++;

	 d_bp = d_b;
	*d_bp++ = (d_cpi4*(d_a[0]+d_a[1])         + 32768)>>16;
	*d_bp++ = (d_cpi4*(d_a[0]-d_a[1])         + 32768)>>16;
	*d_bp++ = (d_spi8*d_a[2] - d_s3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = (d_cpi8*d_a[2] + d_c3pi8*d_a[3] + 32768)>>16;
	*d_bp++ = d_a[4] + d_a[5];
	*d_bp++ = d_a[4] - d_a[5];
	*d_bp++ = d_a[7] - d_a[6];
	*d_bp   = d_a[7] + d_a[6];

	 d_cp = d_c;
	*d_cp++ = (d_b[0] + d_b[3]              + 4      )>>3;
	*d_cp++ = (d_b[1] + d_b[2]              + 4      )>>3;
	*d_cp++ = (d_b[1] - d_b[2]              + 4      )>>3;
	*d_cp++ = (d_b[0] - d_b[3]              + 4      )>>3;
	*d_cp++ = (d_b[4]                       + 4      )>>3;
	*d_cp++ = (d_cpi4*(d_b[6]-d_b[5])       + (1<<18))>>19;
	*d_cp++ = (d_cpi4*(d_b[6]+d_b[5])       + (1<<18))>>19;
	*d_cp   = (d_b[7]                       + 4      )>>3;

	d_puc1 = d_puc2;
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4++) + (*d_pi5--)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;
	if( (d_temp = (*d_pi4)   + (*d_pi5))   > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127) 
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4--) - (*d_pi5++)) > 127)
	    *d_puc1 = 255;
	else if(d_temp < -128)
	    *d_puc1 = 0;
	else                     
	    *d_puc1 = d_temp + 128;

	d_puc1 += PEL_WIDTH;                                       
	if( (d_temp = (*d_pi4)   - (*d_pi5))   > 127) 
	    *d_puc1   = 255;
	else if(d_temp < -128)
	    *d_puc1   = 0;
	else
	    *d_puc1   = d_temp + 128;

	d_puc2   += SCAN_WIDTH;
    }
}

/*****************************************************************************
**  DctUnstuffData
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
static int  DctUnstuffData(srcptr,srclen,dstptr)
unsigned char *srcptr;
int	       srclen;
unsigned char *dstptr;
{
    unsigned char *inptr  = srcptr;
    unsigned char *outptr = dstptr;
    int	i;
    /*
    **	Copy first byte unconditionally.
    */
    *outptr++ = *inptr++;
    /*
    **	Copy replacing the sequence 0xFF00 with 0xFF.
    */
    for (i = 1;  i < srclen;  i++)
        if (*inptr == '\0' && *(inptr - 1) == 0xFF )
            inptr++;
        else
            *outptr++ = *inptr++;

    return (outptr - dstptr);
}

/*****************************************************************************
**  IpsSetupUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
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
**      none
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
#ifdef IPS
static long	    IpsSetUdp(state,i)
DecodeDctStatePtr   state;
int		    i;
{
    UdpPtr  udp = DcdDstUdp_(state,i);
    int	    min_arsize;
    /*
    **	Fill in UDP based on current component parameters.
    */
    udp->UdpW_PixelLength   = DcdPrec_(state);
    switch (DcdPrec_(state))
    {
        case 8:
            udp->UdpB_DType = UdpK_DTypeBU;
	    udp->UdpB_Class = UdpK_ClassA;
	    break;
        case 16:
            udp->UdpB_DType = UdpK_DTypeWU;
	    udp->UdpB_Class = UdpK_ClassA;
	    break;
        default:
            udp->UdpB_DType = UdpK_DTypeVU;
	    udp->UdpB_Class = UdpK_ClassUBA;
    }
    udp->UdpL_PxlStride	    = udp->UdpW_PixelLength;
    udp->UdpL_ScnStride	    = DcdCmpStr_(state,i) * 8;
    udp->UdpL_X1	    = 0;
    udp->UdpL_X2	    = DcdCmpWid_(state,i) - 1;
    udp->UdpL_Y1	    = 0;
    udp->UdpL_Y2	    = DcdCmpHgt_(state,i) - 1;
    udp->UdpL_PxlPerScn	    = DcdCmpWid_(state,i);
    udp->UdpL_ScnCnt	    = DcdCmpHgt_(state,i);
    udp->UdpL_CompIdx	    = DcdCmpIdx_(state,i);
    udp->UdpL_Levels	    = 1 << udp->UdpW_PixelLength;
    /*
    **	If buffer already exists, try to use it.
    */
    min_arsize = (udp->UdpL_ScnStride * udp->UdpL_ScnCnt + 7) / 8; 
    if (udp->UdpA_Base == NULL)
    {
	udp->UdpL_ArSize    = min_arsize * 8;
	udp->UdpL_Pos	    = 0;
	udp->UdpA_Base	    = 
	    (unsigned char *) DctMallocBits_(udp->UdpL_ArSize);
	if (udp->UdpA_Base == NULL)
	    return DctX_BadAlloc;
    }
    else if ((udp->UdpL_ArSize - udp->UdpL_Pos) < min_arsize)
    {
	return	DctX_OutBufSiz;
    }

    return DctX_Success;
}
#endif
