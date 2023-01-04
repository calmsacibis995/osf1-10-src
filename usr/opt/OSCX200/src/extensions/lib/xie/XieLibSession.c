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

/*******************************************************************************
**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**
**                          All Rights Reserved
**
**  Permission to use, copy, modify, and distribute this software and its 
**  documentation for any purpose and without fee is hereby granted, 
**  provided that the above copyright notice appear in all copies and that
**  both that copyright notice and this permission notice appear in 
**  supporting documentation, and that the names of Digital or MIT not be
**  used in advertising or publicity pertaining to distribution of the
**  software without specific, written prior permission.  
**
**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
**  SOFTWARE.
**
*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension (XIE)
**
**  ABSTRACT:
**
**      This module contains XIE library routines for session managment.
**	
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**	Robert NC Shelley
**
**  CREATION DATE:
**
**      March 18, 1989
**
**  MODIFICATION HISTORY:
**
*******************************************************************************/

/*
**  Definitions required by X11 include files
*/
#define NEED_EVENTS
#define NEED_REPLIES

/*
**  Definition to let include files know who's calling.
*/
#define _XieLibSession

/*
**  Include files
*/
    /*
    **  Standard C include files
    */
#include <stdio.h>

    /*
    **  X11 and XIE include files
    */
#ifdef VMS
#include <Xlibint.h>			/* X11 internal lib/transport defs  */
#else
#include <X11/Xlibint.h>		/* X11 internal lib/transport defs  */
#endif
#include <XieDdx.h>			/* XIE client/server common defs    */
#include <XieProto.h>			/* XIE Req/Reply protocol defs	    */
#include <XieLib.h>			/* XIE public  definitions	    */
#include <XieLibint.h>			/* XIE private definitions	    */

/*
**  Table of contents
*/
    /*
    **  PUBLIC entry points
    */
char	 *XieCheckFunction();	/* check server support of single function  */
char	**XieListFunctions();	/* list  server support of all functions    */

void      XieSetOpDefaults();
void      XieQueryOpDefaults();

    /*
    **  Low level global entry points
    */
XieSessionPtr	_XieGetSession();	    /* get session info for display */
    /*
    **  internal routines
    */
static XExtCodes	*InitExtension();   /* initialize server extension  */
static XieSessionPtr	 InitSession();	    /* initiate a session	    */
int		 	 CloseDisplayProc();/* what to do on close display  */
static int		 FindDpyNum();	    /* stub for multi-dpy support   */

/*
**  MACRO definitions
*/
    /*
    **	On VMS the connection number is not obtained per THE book (pg 499).
    */
#ifdef VMS
#define DpyNumber_(dpy)         (dpy->next ? FindDpyNum(dpy) : 0)
#else
#define DpyNumber_(dpy)         ((dpy)->fd)
#endif

/*
**  Equated Symbols
*/

/*
**  External References
*/

/*
**  Local/Global Storage
*/
    /*
    **	Array of XieSessionPtr for session wide information.
    **  This should really be the maximum number of open files
    **  the process can have (like NOFILE from <limits.h>) but there
    **  doesn't seem to be a portable way to do that.
    */
static XieSessionPtr Session[64] = {NULL};

    /*
    ** Flag that controls packet tracing on the client side.
    */
externaldef(xiePacketTrace) BOOL xiePacketTrace;

    /*
    **	Array of Xie protocol function names.
    */
externaldef(XieFunctionName) char *_XieFunctionName[256] = {
	NULL,
	"X_ieInitSession",
        "X_ieTermSession",
        "X_ieQueryEvents",
        "X_ieSelectEvents",
	"X_ieSetOpDefaults",
	"X_ieQueryOpDefaults",
        "X_ieCreateByReference",
        "X_ieCreateByValue",
        "X_ieDeleteResource",
        "X_ieQueryResource",
        "X_ieBindPhotomap",
        "X_ieAbortPhotoflo",
        "X_ieExecutePhotoflo",
        "X_ieTapPhotoflo",
        "X_ieAbortTransport",
        "X_ieGetStream",
        "X_ieGetTile",
        "X_iePutStream",
        "X_iePutTile",
        "X_ieSetTransport",
        "X_ieExport",
        "X_ieFreeExport",
        "X_ieImport",
	"X_ieQueryExport",
	"X_ieArea",
	"X_ieAreaStats",
	"X_ieArith",
	"X_ieCalcHist",
	"X_ieChromeCom",
	"X_ieChromeSep",
	"X_ieCompare",
	"X_ieConstrain",
	"X_ieCrop",
	"X_ieDither",
	"X_ieFill",
	"X_ieLogical",
	"X_ieLuminance",
	"X_ieMatchHistogram",
	"X_ieMathf",
	"X_ieMirror",
	"X_iePoint",
	"X_iePointStats",
	"X_ieRotate",
	"X_ieScale",
	"X_ieTranslate",
	NULL};

/*******************************************************************************
**  XieCheckFunction
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return the pointer to the protocol name string for the requested
**	protocol function.  The pointer is NULL if not implemented.
**
**  FORMAL PARAMETERS:
**
**	dpy	    - pointer to X11 display structure
**	function    - minor-opcode of function requested
**
**  FUNCTION VALUE:
**
**	string pointer - DO NOT free !
**
*******************************************************************************/
char *XieCheckFunction( dpy, function )
 Display *dpy;
 int	  function;
{
    Bool  supported = False;
    char *name;

    if( function < 8 * sizeof(XieSessionRec) )
	/*
	**  See if this function is supported.
	*/
	supported = GET_VALUE_( SesFunVec_(_XieGetSession(dpy)), function, 1 );

    /*
    **  Set the function name string pointer.
    */
    name = supported ? _XieFunctionName[function] : NULL;

    return( name );
}				    /* end XieCheckFunction */

/*******************************************************************************
**  XieListFunctions
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return an array of pointers to the protocol name strings for all
**	supported Xie protocol functions.  NULL pointers are returned for
**	functions that are not yet implemented or unsupported by the server.
**
**  FORMAL PARAMETERS:
**
**	dpy	    - pointer to X11 display structure
**
**  FUNCTION VALUE:
**
**	array of string pointers - use XieFree to free array, don't free strings
**
*******************************************************************************/
char **XieListFunctions( dpy )
 Display *dpy;
{
    int	 function;
    char **strings = (char **) XieMalloc(256 * sizeof(char *));

    for( function = 0; function < 256; function++ )
	strings[function] = XieCheckFunction(dpy, function);

    return( strings );
}				    /* end XieListFunctions */

/*******************************************************************************
**  XieSetOpDefaults
**
**  FUNCTIONAL DESCRIPTION:
**
**	Set the operational defaults associated with a session.
**
**  FORMAL PARAMETERS:
**	dpy    	- pointer to X11 display structure
**      model   - Constraint model to be used:
**                 {XieK_HardClip, XieK_ClipScale, XieK_HardScale}
**      levels  - Array of levels to constrain each component.
**	
**
*******************************************************************************/
void XieSetOpDefaults( dpy, model, levels)
 Display            *dpy;
 int      	    model;
 unsigned long int  levels[XieK_MaxComponents];
{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieSetOpDefaultsReq *req;
    int i;
    
    /*
    **	Create a request to set the session operational defaults.
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieSetOpDefaults,req);
    req->model = model;
    for( i = 0; i < XieK_MaxComponents; i++)
	req->levels[i] = levels[i];
    XieReqDone_(dpy);
}				    /* end XieSetOpDefaults */

/*******************************************************************************
**  XieQueryOpDefaults
**
**  FUNCTIONAL DESCRIPTION:
**
**	Obtain session operational defaults info from the server.
**
**  FORMAL PARAMETERS:
**
**	dpy         - Pointer to X11 display structure
**      model_ret   - Constraint model return pointer
**      levels_ret  - Constraint levels array
**
*******************************************************************************/
void XieQueryOpDefaults( dpy, model_ret, levels_ret )
 Display              *dpy;
 int	              *model_ret;
 unsigned long int    *levels_ret;
{
    XieSessionPtr ses = _XieGetSession(dpy);
    xieQueryOpDefaultsReq  *req;
    xieQueryOpDefaultsReply rep;
    int i;

    /*
    **	Create request to query the session defaults
    */
    XieReq_(dpy,0,SesOpCode_(ses),ieQueryOpDefaults,req);    
    if( !_XReply( dpy, &rep, 0, True) )
	_XieSrvError(dpy, req->opcode, 0, rep.sequenceNumber,
		     BadLength, "QueryOpDefaults: reply error");
    else {
	XieReqDone_(dpy);
	/*
        *  Unload the reply packet.
	*/
	if(  model_ret   != NULL )
	    *model_ret  = rep.model;
	if(  levels_ret  != NULL ) {
	    for (i = 0; i < XieK_MaxComponents; i++)
		levels_ret[i] = rep.levels[i];
	}
	    
    }
}				    /* end XieQueryOpDefaults */

/*******************************************************************************
**  _XieGetSession
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return the pointer to the XieSessionRec structure belonging to this 
**	connection.  The image extension is initialized and a session 
**	initiated, if necessary.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**
**  FUNCTION VALUE:
**
**	XieSessionPtr
**
*******************************************************************************/
XieSessionPtr _XieGetSession( dpy )
 Display *dpy;
{
    XExtCodes *codes;

    if( Session[DpyNumber_(dpy)] == NULL )
	{
	/*
	**  Initialize the server's image extension.
	*/
	codes = InitExtension( dpy );

	/*
	**  Initiate an image extension session.
	*/
	Session[DpyNumber_(dpy)] = InitSession( dpy, codes );
	}

    /*
    **	Return the pointer to this session's parameters.
    */
    return( Session[DpyNumber_(dpy)] );
}				    /* end _XieGetSession */

/*******************************************************************************
**  InitExtension
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initialized the image extension.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**
**  FUNCTION VALUE:
**
**	XExtCodes - pointer to X11 extension information about Xie
**
*******************************************************************************/
static XExtCodes *InitExtension( dpy )
 Display *dpy;
{
    XExtCodes    *codes;

    /*
    **	Initialize the extension.
    */
    codes = XInitExtension( dpy, XieS_Name );
    if( codes == NULL )
	_XieLibError( EACCES, "XInitExtension" );

    /*
    **	Hook our proceedures into Xlib.
    */
    XESetCloseDisplay(dpy, codes->extension, (int (*)()) CloseDisplayProc );
    XESetWireToEvent( dpy, codes->first_event + XieK_ComputationEvent,
					       _XieConvertEventProc );
    XESetWireToEvent( dpy, codes->first_event + XieK_PhotofloEvent,
					       _XieConvertEventProc );
    XESetWireToEvent( dpy, codes->first_event + XieK_DisplayEvent,
					       _XieConvertEventProc );
    /*
    ** Set the packet trace variable as requested.
    */
    if( (char *)getenv("XIE_PACKET_TRACE") != NULL)
	xiePacketTrace = True;
    else
	xiePacketTrace = False;

    return( codes );
}				    /* end InitExtension */

/*******************************************************************************
**  InitSession
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initiate a session with the server image extension.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	codes	- pointer to X11 extension information about Xie
**
**  FUNCTION VALUE:
**
**	XieSessionPtr
**
*******************************************************************************/
static XieSessionPtr InitSession( dpy, codes )
 Display   *dpy;
 XExtCodes *codes;
{
    xieInitSessionReply rep;
    xieInitSessionReq  *req;
    XieSessionPtr	ses = NULL;
    char text[80];
    /*
    **  Create request to initialize the session.
    */
    XieReq_(dpy,0,codes->major_opcode,ieInitSession,req);
    req->major_version	= XieK_MajorVersion;
    req->minor_version  = XieK_MinorVersion;

    if( !_XReply( dpy, (xReply *) &rep, 0, False )
				|| rep.length != sizeof(XieFunctionsRec) >> 2 )
	_XieLibError( EACCES, "InitSession: reply error" );

    else if( !rep.success || rep.major_version != XieK_MajorVersion
			  || rep.minor_version != XieK_MinorVersion )
	{
	sprintf(text, "InitSession: %s server V%d.%d, %s client V%d.%d",
		XieS_Name, rep.major_version, rep.minor_version,
		XieS_Name, XieK_MajorVersion, XieK_MinorVersion );
	_XieLibError( EACCES, text );
	}
    else
	{   /*
	    **	Create an XieSessionRec.
	    */
	ses = (XieSessionPtr) XieCalloc(1,sizeof(XieSessionRec));
	SesCodes_(ses) = codes;
	_XReadPad( dpy, SesFunVec_(ses), sizeof(XieFunctionsRec) );
	XieReqDone_(dpy);
	}
    return( ses );
}				    /* end InitSession */

/*******************************************************************************
**  CloseDisplayProc
**
**  FUNCTIONAL DESCRIPTION:
**
**	What to do when client calls XCloseDisplay.
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**	codes	- pointer to XExtCodes
**
*******************************************************************************/
int CloseDisplayProc( dpy, codes )
 Display    *dpy;
 XExtCodes  *codes;
{
    xieTermSessionReq *req;
    XieResPtr nxt, res;
    XieSessionPtr  ses = Session[DpyNumber_(dpy)];

    /*
    **	Create request to terminate the session.
    */
    XieReq_(dpy,0,codes->major_opcode,ieTermSession,req);
    XieReqDone_(dpy);

    /*
    **	Free every XieResRec left on the list.
    */
    for( res = SesResLst_(ses); res != NULL; res = nxt )
	{
	nxt = ResLnk_(res);
	XieFree(res);
	}
    /*
    **	Free the session info.
    */
    XieFree( ses );
    Session[DpyNumber_(dpy)] = NULL;

    return( Success );
}				    /* end CloseDisplayProc */

/*******************************************************************************
**  FindDpyNum
**
**  FUNCTIONAL DESCRIPTION:
**
**	%%%%%%%% search a list and compare dpy's with indices %%%%%%%%
**
**  FORMAL PARAMETERS:
**
**	dpy	- pointer to X11 display structure
**
**  FUNCTION VALUE:
**
**	some magic number ???
**
*******************************************************************************/
static int FindDpyNum( dpy )
 Display    *dpy;
{
    /*
    **	NYI -- stay tuned to XDPS.C (from display Postscript) for what to do.
    */
    _XieLibError(EMLINK, "FindDpyNum (one display per process)");
}				    /* end FindDpyNum */
/* end module XieLibSession.c */
