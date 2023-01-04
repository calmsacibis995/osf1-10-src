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
**	The XieMain module does the following :
**	
**	    - identifies X Image Extension to X11
**	    - initializes the extension resources class and types
**	    - invokes DDX-specific initialization routine
**	    - identifies X Image Extension Request
**	    - checks length, performs the XIE request preprocessing
**	      and processing.
**	    - swaps request, send replies, errors and events
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
**      January 31, 1989
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
#include <XieDdx.h>		/* Get the DDX interface */
#include <XieAppl.h>
#include <XieProto.h>
#include <XieServer.h>
#include <XieMacros.h>
#include <XiePipeInterface.h>

/*
**  Table of contents
*/
void	XieInit();			    /* Xie Init routine		    */
int	XieDispatcher();		    /* Xie Request Dispatcher	    */
int	XieSwapDispatcher();		    /* Swap Request then Dispatch   */
void	XieReset();			    /* Xie Reset		    */
void	XiePacketTrace();		    /* Packet trace function	    */

/*
**  Equated Symbols
*/

/*
**  MACRO definitions
*/

/*
**  External References
*/
#if   defined(X11R3) &&  !defined(DWV3)
    /* Nothing */
#else
extern	    int UtilFreeResource();
#endif
#undef		FatalError

externalref int (*xieDixProc[X_ieLastRequest])();
externalref int (*xieDixSwap[X_ieLastRequest])();

extern CARD32	xiePipeTimeslice;

/*
**	Global Storage
*/
CARD32		xieClients;		    /* num of clients served	    */
CARD32		xieEventBase;		    /* Base for Xie events	    */
CARD32		xieReqCode;		    /* XIE main opcode		    */
#if   defined(X11R3) && !defined(DWV3)
CARD16		RC_xie;			    /* XIE Class		    */
CARD16		RT_photo;		    /* Photo{flo|map|tap} resource  */
CARD16		RT_idc;			    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
RESTYPE		RC_xie;			    /* XIE Class		    */
RESTYPE		RT_photo;		    /* Photo{flo|map|tap} resource  */
RESTYPE		RT_idc;			    /* Cpp, Roi & Tmp resource types*/
#endif

/*
**  Local storage
*/
static BOOL	xiePacketTrace;		    /* Enable packet tracing	    */

static char *opstr[] = {
        "NotRequest",			/*  0 */
        "InitSession",			/*  1 */
	"TermSession",			/*  2 */
	"QueryEvents",			/*  3 */
	"SelectEvents",			/*  4 */
	"SetOpDefaults",		/*  5 */
	"QueryOpDefaults",		/*  6 */
        "ClonePhoto",			/*  7 */
        "CreateResource",		/*  8 */
	"DeleteResource",		/*  9 */
	"QueryResource",		/* 10 */
	"BindMapToFlo",			/* 11 */
	"AbortFlo",			/* 12 */
	"ExecuteFlo",			/* 13 */
	"TapFlo",			/* 14 */
        "AbortTransport",		/* 15 */
	"GetStream",			/* 16 */
	"GetTile",			/* 17 */
	"PutStream",			/* 18 */
	"PutTile",			/* 19 */
        "SetTransport",			/* 20 */
        "Export",			/* 21 */
	"FreeExport",			/* 22 */
	"Import",			/* 23 */
	"QueryExport",			/* 24 */
	"Area",				/* 25 */
	"AreaStats",			/* 26 */
	"Arith",	                /* 27 */
	"CalcHistogram",		/* 28 */
	"ChromeCom",		        /* 29 */
	"ChromeSep",			/* 30 */
	"Compare",			/* 31 */
	"Constrain",			/* 32 */
	"Crop",				/* 33 */
	"Dither",			/* 34 */
	"Fill",				/* 35 */
	"Logical",			/* 36 */
	"Luminance",			/* 37 */
	"MatchHistogram",		/* 38 */
	"Math",				/* 39 */
	"Mirror",			/* 40 */
	"Point",			/* 41 */
	"PointStats",			/* 42 */
	"Rotate",			/* 43 */
	"Scale",			/* 44 */
	"Translate"			/* 45 */
};


/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**      This module contains the XIE intialization code.  All XIE software
**	related tasks are performed by XieInit.  This includes defining an
**	XIE resource class and several XIE resource types - Photomap ,Cpp etc.
**	In addition, XieInit must identify the extension, its errors and 
**	events to the CORE X.
**
**	All DDX related intialization tasks will be performed by the
**	DdxInit procedure.  In particular, this module will initialize
**	the DDX procedure dispatch table.
**
**      If the DDX initialization fails, the DDXInit routine should
**      record the reason in the server error log, but not abort
**      the server.  We will only abort if the core X server returns
**      an error when we attempt to add the extension.
**      
**
**  FORMAL PARAMETERS:
**
**      The command line arguments argc,argv are passed in.
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
**      Triggers fatal server error if extension cannot be bound to the
**      core server.
**
************************************************************************/

/*
**  XieInit
**
**  Called by InitExtensions in main() or from QueryExtension() if the
**  extension is dynamically loaded.
**
*/
void XieInit( argc, argv )
 int	argc;
 char	*argv[];
{
    int	    status;
    char    *envstr;

    char    *getenv();

    ExtensionEntry *extEntry;

    /*	 
    **  Initialize the DDX support first.  If this fails, we will not
    **  add the extension, and thus cause the client's InitExtension or
    **  QueryExtension call to fail.
    */	 
    if ( DdxInit_() != 0 )
            ErrorF("XieExtensionInit: DDX Init failed\n");
    else {

	if (InitPipelineInterface_() != Success)
	    FatalError("XieExtensionInit: Pipeline interface init failed\n");
	else {

#ifdef   X11R3
	    extEntry = AddExtension( XieS_Name, XieK_NumEvents, XieK_NumErrors,
				XieDispatcher, XieSwapDispatcher, XieReset );
#else /* X11R4 */
	    extEntry = AddExtension( XieS_Name, XieK_NumEvents, XieK_NumErrors,
				XieDispatcher, XieSwapDispatcher, XieReset,
				StandardMinorOpcode );
#endif
	
	    if( extEntry == NULL ) 
		FatalError("XieExtensionInit: AddExtension failed\n");
	    else
	    {
#ifdef   X11R3
		(void) MakeAtom( XieS_Name, strlen(XieS_Name), TRUE );
#else /* X11R4 */
		if( !MakeAtom( XieS_Name, strlen(XieS_Name), TRUE ))
		    FatalError("XieExtensionInit: MakeAtom failed\n");
#endif
		xieClients	= 0;
		xieEventBase	= extEntry->eventBase;
		xieReqCode	= extEntry->base;
		RC_xie		= CreateNewResourceClass();
#if   defined(X11R3) && !defined(DWV3)
		RT_photo	= CreateNewResourceType();
		RT_idc		= CreateNewResourceType();
#else /* X11R4 || DWV3 */
		RT_photo	= CreateNewResourceType(UtilFreeResource);
		RT_idc		= CreateNewResourceType(UtilFreeResource);
#endif
	    /*	ParseCommandLineArguments( argc, argv ); */
	    }
	    /*
	    **  Set packet trace variable accordingly...
	    */
	    envstr = getenv ("XIE_PACKET_TRACE");
	    if (envstr != NULL)
	    {
		xiePacketTrace = TRUE;
	    }
	    else
	    {
		xiePacketTrace = FALSE;
	    }
#ifdef VMS
	    /*
	    **  Disable server yielding...
	    */
	    envstr = getenv ("XIE_DISABLE_YIELD");
	    if (envstr != NULL)
	    {
		xiePipeTimeslice = 0;
	    }
#endif
	}
    }
}					/* end XieInit */

/*****************************************************************************
**  ProcXieDispatcher
******************************************************************************/
int XieDispatcher( client )
 ClientPtr client;	/* client pointer				    */
{
    REQUEST(xieReq);
    /*
    **	Print packet trace if necessary
    */
    if (xiePacketTrace)
    {
        XiePacketTrace(stuff);
    }

    if ( stuff->opcode < X_ieLastRequest )
	/*
	**  Call the appropriate 'Proc' function.
	*/
	return( CallProc_(xieDixProc)(client) );

    else
	/*
	**  Call the BadRequest function.
	*/
	return( ProcNotRequest(client) );

}				    /* end XieDispatcher */

int XieSwapDispatcher(client)
 ClientPtr client;	/* client pointer				    */
{
    REQUEST(xieReq);

    /*
    **	Print packet trace if necessary
    */
    if (xiePacketTrace)
    {
        XiePacketTrace(stuff);
    }

    if( stuff->opcode < X_ieLastRequest )
	/*
	**  Call the appropriate 'Proc' function.
	*/
	return( CallProc_(xieDixSwap)(client) );

    else
	/*
	**  Call the BadRequest function.
	*/
	return( ProcNotRequest(client) );

}				    /* end XieSwapDispatcher */


/************************************************************************
**
**  FUNCTIONAL DESCRIPTION:
**
**	XieReset is responsible for reseting the extension and the attached
**	DDX.
**
**  FORMAL PARAMETERS:
**
**	none
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
************************************************************************/

void XieReset()
{
    /*	 
    **  Reset extension-specific state
    */	 


    /*	 
    **  Reset any attached accelerator
    */	 
    DdxReset_();
}                   /* end XieReset */


int ProcNotRequest(client)
 ClientPtr client;	/* client pointer				    */
{
    REQUEST(xieReq);

    SendErrorToClient( client, xieReqCode, stuff->opcode, 0, BadRequest );
    return( BadRequest );
}				    /* end ProcNotRequest */

int ProcNotImplemented(client)
 ClientPtr client;	/* client pointer				    */
{
    REQUEST(xieReq);

    SendErrorToClient(client, xieReqCode, stuff->opcode, 0, BadImplementation);
    return( BadImplementation );
}				    /* end ProcNotImplemented */

void	XiePacketTrace(stuff)
xieReq	*stuff;
{
    if ( stuff->opcode < X_ieLastRequest )
	ErrorF("Packet Trace : %s\n",opstr[stuff->opcode]);
    else
	ErrorF("Packet Trace : Bad Request %d\n",stuff->opcode);
}

/* end module XieMain.c */
