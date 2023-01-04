/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */

/***********************************************************
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of DIX procedures for Session service requests.
**	
**  ENVIRONMENT:
**
**	VAX/VMS V5.4
**	ULTRIX  V4.0
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      May 3, 1989
**
************************************************************************/

/*
**  Include files
*/
#include <stdio.h>
/*
**  Core X Includes
*/
#include <X.h>
#include <Xproto.h>
#include <misc.h>
#include <extnsionst.h>
#include <dixstruct.h>
/*
**  XIE Includes
*/
#include <XieAppl.h>
#include <XieDdx.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>

/*
**  Table of contents
*/
    /*
    **  Xie protocol proceedures called from XieMain
    */
int	ProcInitSession();
int	ProcTermSession();

int     ProcSetOpDefaults();
int     ProcQueryOpDefaults();

    /*
    **  routines referenced by other modules.
    */
void    GetConstraintModel();

/*
**  Equated Symbols
*/
/*
**  MACRO definitions
*/
/*
**  External References
**
*/
extern CARD32		xieClients;	    /* num of clients served        */
extern CARD32		xieEventBase;	    /* Base for Xie events          */
extern CARD32		xieReqCode;	    /* XIE main opcode              */
#if   defined(X11R3) && !defined(DWV3)
extern CARD16		RC_xie;		    /* XIE Class		    */
extern CARD16		RT_photo;	    /* Photo{flo|map|tap} resource  */
extern CARD16		RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
extern RESTYPE		RC_xie;		    /* XIE Class		    */
extern RESTYPE		RT_photo;	    /* Photo{flo|map|tap} resource  */
extern RESTYPE		RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#endif

externalref int (*xieDixSwap[X_ieLastRequest])(); /* Reply swap dispatch
						   * table */
externalref int (*xieDixSwapReply[])();
externalref void (*xieDixSwapData[])();

/*
**  Global Storage
*/
externaldef(xieClientTable) ClientTableRec xieClientTable[MAXCLIENTS];

/*
**  Local Storage
*/

/*-----------------------------------------------------------------------
-------------------------  InitSession Procedure ------------------------
------------------------------------------------------------------------*/
int ProcInitSession( client )
 ClientPtr client;	/* client pointer				    */
{
    CARD8   function[sizeof(XieFunctionsRec)];
    CARD32  i;
    xieInitSessionReply reply;
    REQUEST( xieInitSessionReq );
    REQUEST_SIZE_MATCH( xieInitSessionReq );

    memset(function, 0, sizeof(XieFunctionsRec));
    /*
    **	Functions supported by this implementation.
    */
    PUT_VALUE_( function, X_ieInitSession,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieTermSession,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieQueryEvents,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieSelectEvents,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieSetOpDefaults,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieQueryOpDefaults,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieCreateByReference,    TRUE, 1 );
    PUT_VALUE_( function, X_ieCreateByValue,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieDeleteResource,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieQueryResource,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieBindPhotomap,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieAbortPhotoflo,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieExecutePhotoflo,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieTapPhotoflo,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieAbortTransport,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieGetStream,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieGetTile,		    TRUE, 1 );
    PUT_VALUE_( function, X_iePutStream,	    TRUE, 1 );
    PUT_VALUE_( function, X_iePutTile,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieSetTransport,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieExport,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieFreeExport,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieImport,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieQueryExport,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieArea,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieAreaStats,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieArith,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieCalcHist,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieChromeCom,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieChromeSep,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieCompare,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieConstrain,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieCrop,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieDither,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieFill,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieLogical,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieLuminance,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieMatchHistogram,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieMath,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieMirror,		    TRUE, 1 );
    PUT_VALUE_( function, X_iePoint,		    TRUE, 1 );
    PUT_VALUE_( function, X_iePointStats,	    TRUE, 1 );
    PUT_VALUE_( function, X_ieRotate,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieScale,		    TRUE, 1 );
    PUT_VALUE_( function, X_ieTranslate,	    TRUE, 1 );

    reply.type		 = X_Reply;
    reply.success	 = stuff->major_version == XieK_MajorVersion &&
			   stuff->minor_version == XieK_MinorVersion;
    reply.sequenceNumber = client->sequence;
    reply.length	 = sizeof(XieFunctionsRec) >> 2;
    reply.major_version	 = XieK_MajorVersion;
    reply.minor_version	 = XieK_MinorVersion;

    if( reply.success )
	{   /*
	    **	Define client dependent defaults for xieSession.
	    */
	xieClientTable[client->index].event_mask    = 0;
	xieClientTable[client->index].model	    = XieK_HardClip;
	for( i = 0; i < XieK_MaxComponents; i++ )
	    xieClientTable[client->index].levels[i] = 256;
	/*
	**  One more happy client served.
	*/
        xieClients++;
	}
    /*
    **	Tell the client how happy it is.
    */
    WriteXieReplyToClient( client, sizeof(xieInitSessionReply), &reply );
    SetSwappedDataRtn( client, CARD8 );
    WriteSwappedXieDataToClient( client, sizeof(XieFunctionsRec),   function );

    return( Success );
}				    /* end ProcInitSession */

/*-----------------------------------------------------------------------
-------------------------  TermSession Procedure ------------------------
------------------------------------------------------------------------*/
int ProcTermSession( client )
 ClientPtr client;	/* client pointer				    */
{
    REQUEST( xieTermSessionReq );
    REQUEST_SIZE_MATCH( xieTermSessionReq );

    /*
    **	One less happy client served.
    */
    xieClients--;

    return( Success );
}				    /* end ProcTermSession */

/*-----------------------------------------------------------------------
-------------------Set Operational Defaults  Procedure ------------------
------------------------------------------------------------------------*/
int ProcSetOpDefaults( client )
 ClientPtr client;	/* client pointer				    */
{
    CARD32 i;
    REQUEST( xieSetOpDefaultsReq );
    REQUEST_SIZE_MATCH( xieSetOpDefaultsReq );

    ValueOkIf_( stuff->model == XieK_HardClip
	     || stuff->model == XieK_ClipScale
	     || stuff->model == XieK_HardScale, 0 );
    xieClientTable[client->index].model = stuff->model;

    for( i = 0; i < XieK_MaxComponents; i++ )
	{
	ValueOkIf_(stuff->levels[i] > 0, 0);
	xieClientTable[client->index].levels[i] = stuff->levels[i];
	}

    return( Success );
}				    /* end ProcSetOpDefaults */

/*-----------------------------------------------------------------------
-----------------Query Operational Defaults Procedure -------------------
------------------------------------------------------------------------*/
int ProcQueryOpDefaults( client )
 ClientPtr client;	/* client pointer				    */
{
    xieQueryOpDefaultsReply rep;
    CARD32 model;

    REQUEST( xieQueryOpDefaultsReq );
    REQUEST_SIZE_MATCH( xieQueryOpDefaultsReq );

    rep.type            = X_Reply;
    rep._reserved       = 0;
    rep.sequenceNumber  = client->sequence;
    rep.length          = 0;
    GetConstraintModel(client, &model, &rep.levels[0]);
    rep.model = model;

    WriteXieReplyToClient( client, sizeof(xieQueryOpDefaultsReply), &rep );

    return( Success );
}				    /* end ProcQueryOpDefaults */

/*-----------------------------------------------------------------------
-------------------------  routine:  GetConstraintModel -----------------
------------------------------------------------------------------------*/
void GetConstraintModel( client, model, levels )
 ClientPtr   	 client;	/* client pointer			    */
 CARD32		*model; 	/* model return pointer                     */
 CARD32		*levels;	/* levels return array                      */
{
    CARD32  i;

    *model = xieClientTable[client->index].model;

    for( i = 0; i < XieK_MaxComponents; i++ )
	levels[i] = xieClientTable[client->index].levels[i];

}				    /* end GetConstraintModel */
/* end module XieSession.c */
