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

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of DIX procedures for resource
**	service requests.
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
*******************************************************************************/

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
#include <XiePipeInterface.h>

/*
**  Table of contents
*/
    /*
    **	Xie protocol proceedures called from XieMain
    */
int  ProcAbortFlo();
int  ProcBindPhotomap();
int  ProcClonePhoto();
int  ProcCreateResource();
int  ProcDeleteResource();
int  ProcExecuteFlo();
int  ProcQueryResource();
int  ProcTapFlo();
    /*
    **	routines referenced by other modules.
    */
int	    FloDone();
int	    LookupIdc();
int	    LookupCppRoi();
int	    LookupPhotos();
Bool	    QueResource();
    /*
    **	routines used internally by XieResource
    */
static int  CloneFlo();
static int  CloneMap();
static int  CreateCpp();
static int  CreatePhoto();
static int  CreateRoi();
static int  CreateTmp();
static int  QueryCpp();
static int  QueryFlo();
static int  QueryMap();
static int  QueryRoi();
static int  QueryTmp();

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
#if   defined(X11R3) && !defined(DWV3)
extern void	    AddResource();	    /* called by AddResource_ macro */
extern int	    UtilFreeResource();	    /* ref'd  by AddResource_ macro */
#else /* X11R4 || DWV3 */
extern Bool	    AddResource();	    /* called by AddResource_ macro */
#endif
extern void	    FreeResource();	    /* ref'd  by FreeResource_ macro*/

extern int	    xieDoSendEvent();	    /* XieEvents.c		    */
extern int	    StreamCreate();	    /* XieTransport.c		    */
extern int	    xieStreamInit();	    /* XieTransport.c		    */
extern int	    StreamPending();	    /* XieTransport.c		    */
extern int	    UtilBePCM();	    /* XieUtils.c		    */
extern PhotomapPtr  UtilCreatePhotomap();   /* XieUtils.c		    */
extern void	    UtilSetPending();	    /* XieUtils.c		    */

extern CARD32	    xieClients;		    /* num of clients served	    */
extern CARD32	    xieEventBase;	    /* Base for Xie events          */
extern CARD32	    xieReqCode;		    /* XIE main opcode		    */
#if   defined(X11R3) && !defined(DWV3)
extern CARD16	    RC_xie;		    /* XIE Class		    */
extern CARD16	    RT_photo;		    /* Photo{flo|map|tap} resource  */
extern CARD16	    RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#else /* X11R4 || DWV3 */
extern RESTYPE	    RC_xie;		    /* XIE Class		    */
extern RESTYPE	    RT_photo;		    /* Photo{flo|map|tap} resource  */
extern RESTYPE	    RT_idc;		    /* Cpp, Roi & Tmp resource types*/
#endif

externalref int (*xieDixSwap[X_ieLastRequest])(); /* Reply swap dispatch tables*/
externalref int (*xieDixSwapReply[])();
extern void	SwapTmpWrite();
/*
**	Local Storage
*/

/*-----------------------------------------------------------------------
-------------------  Abort Photoflo execution Procedure -----------------
------------------------------------------------------------------------*/
int ProcAbortFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    REQUEST( xieAbortPhotofloReq );
    REQUEST_SIZE_MATCH( xieAbortPhotofloReq );
    /* 
    **	Locate Photoflo.
    */
    LookupPhotos_(&flo, NULL, stuff->photoflo_id, 0);

    if( Run_(flo) )
	{   /*
	    **  Kill the DDX Pipeline.
	    */
	Run_(flo) = FALSE;
	AbortPipeline_( flo );
	FloDone(client, flo );
	}
    Make_(flo) = FALSE;

    return( Success );
}			/* End of ProcAbortFlo */

/*-----------------------------------------------------------------------
---------------- Bind a Photomap to a Photoflo Procedure ----------------
------------------------------------------------------------------------*/
int ProcBindMapToFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr img;
    REQUEST( xieBindPhotomapReq );
    REQUEST_SIZE_MATCH( xieBindPhotomapReq );
    /*
    **	Verify the Photomap and Photoflo .
    */
    LookupPhotos_(&flo, NULL, stuff->photoflo_id,-1);
    LookupPhotos_(NULL, &img, stuff->photomap_id, 0);

    /*
    **	Add the Photomap to the Photoflo reference list.
    */
    AllocOkIf_( QueResource(flo, img, FALSE), stuff->photoflo_id );

    return( Success );
}				/* end ProcBindMapToFlo */

/*-----------------------------------------------------------------------
-------------- Clone Photo{flo|map} from Photomap Procedure -------------
------------------------------------------------------------------------*/
int ProcClonePhoto( client )
 ClientPtr client;	/* client pointer				    */
{
    INT32   status;
    REQUEST( xieCreateByReferenceReq );
    REQUEST_SIZE_MATCH( xieCreateByReferenceReq );

    switch( stuff->resource_type )
	{
    case XieK_Photoflo :
	status = CloneFlo( client );
	break;

    case XieK_Photomap :
	status = CloneMap( client );
	break;

    default: BadRequest_(stuff->resource_id);
	}
    return( status );
}				/* end ProcClonePhoto */

/*-----------------------------------------------------------------------
------------------------ CreateResource Procedure -----------------------
------------------------------------------------------------------------*/
int ProcCreateResource( client )
 ClientPtr client;	/* client pointer				    */
{
    INT32   status;
    REQUEST( xieCreateByValueReq );
    REQUEST_AT_LEAST_SIZE( xieCreateByValueReq );

    switch( stuff->resource_type )
	{
    case XieK_IdcCpp :
	status = CreateCpp( client );
	break;

    case XieK_Photoflo :
    case XieK_Photomap :
	status = CreatePhoto( client );
	break;

    case XieK_IdcRoi :
	status = CreateRoi( client );
	break;

    case XieK_IdcTmp :
	status = CreateTmp( client );
	break;

    default: BadRequest_(stuff->resource_id);
	}

    return( status );
}				/* end ProcCreateResource */

/*-----------------------------------------------------------------------
------------------------ DeleteResource Procedure -----------------------
------------------------------------------------------------------------*/
int ProcDeleteResource( client )
 ClientPtr client;	/* client pointer				    */
{
    CommonPartPtr res;
    CARD32	  type;

    REQUEST( xieDeleteResourceReq );
    REQUEST_SIZE_MATCH( xieDeleteResourceReq );

    switch( stuff->resource_type )
	{
    case XieK_Photoflo	:
    case XieK_Photomap	: type = RT_photo; break;

    case XieK_IdcCpp	:
    case XieK_IdcRoi	:
    case XieK_IdcTmp	: type = RT_idc;   break;

    case XieK_Phototap	:
    default: BadRequest_(stuff->resource_id);
	}

    res = (CommonPartPtr) LookupId_(stuff->resource_id, type);
    /*
    **  If we can't find what the client's trying to delete, just go away
    **	quietly -- after all it could be a legitimate client resource that
    **	we were unable to create for some reason.
    */
    if( res == NULL )
	return( Success );

    IDChoiceOkIf_( ResType_(res) == stuff->resource_type, stuff->resource_id );

    FreeResource_( stuff->resource_id );

    return( Success );
}				/* end ProcDeleteResource */

/*-----------------------------------------------------------------------
-----------------------  Execute Photoflo Procedure ---------------------
------------------------------------------------------------------------*/
int ProcExecuteFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    INT32   pending, status;
    REQUEST( xieExecutePhotofloReq );
    REQUEST_SIZE_MATCH( xieExecutePhotofloReq );
#ifdef PEZD
    printf("PEZD : Begin Execute Flo\n");
    LIB$INIT_TIMER();
#endif
    LookupPhotos_( &flo, NULL, stuff->photoflo_id, -1 );

    status = StreamCreate(flo);		    /* create Decoders and Encoders */

    if( status == Success )
	status  = OptimizePipeline_(flo);   /* let the DDX optimize the Flo */

    if( status == Success )
	status  = xieStreamInit(flo);	    /* initialize PutStream Sinks   */

    if( status == Success )
	status  = InitiatePipeline_(flo);   /* initialize Pipeline elements */

    if( status == Success )
	pending = StreamPending(flo);	    /* PutStream requests pending ? */

    if( status == Success )
	{   /*
	    **	Begin Photoflo execution.
	    */
	Make_(flo) = FALSE;
	Run_(flo)  = TRUE;
	RefCnt_(flo)++;
	status = ResumePipeline_( flo );
	}
    OkIf_( status == Success, stuff->photoflo_id, status );

    if( pending == Success && !Yielded_(flo) )
	/*
	**  No PutStream transport was pending so we're already done.
	*/
	status = FloDone( client, flo );

    return( status );
}				/* End of ProcExecuteFlo */

/*-----------------------------------------------------------------------
------------------------ QueryResource Procedure ------------------------
------------------------------------------------------------------------*/
int ProcQueryResource( client )
 ClientPtr client;	/* client pointer				    */
{
    INT32   status;
    REQUEST( xieQueryResourceReq );
    REQUEST_SIZE_MATCH( xieQueryResourceReq );

    switch( stuff->resource_type )
	{
    case XieK_IdcCpp :
	status = QueryCpp( client );
	break;

    case XieK_Photoflo :
	status = QueryFlo( client );
	break;

    case XieK_Photomap :
    case XieK_Phototap :
	status = QueryMap( client );
	break;

    case XieK_IdcRoi :
	status = QueryRoi( client );
	break;

    case XieK_IdcTmp :
	status = QueryTmp( client );
	break;

    default: BadRequest_(stuff->resource_id);
	}

    return( status );
}				/* end ProcQueryResource */

/*-----------------------------------------------------------------------
-----------------------  Tap Photoflo Proceedure  -----------------------
------------------------------------------------------------------------*/
int ProcTapFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr img;
    REQUEST( xieTapPhotofloReq );
    REQUEST_SIZE_MATCH( xieTapPhotofloReq );

    LookupPhotos_(&flo, &img, stuff->photoflo_id, -1);
    IDChoiceOkIf_( flo, stuff->photoflo_id );
    AccessOkIf_( ResId_(img) == 0, ResId_(img) );
    CheckNewResource_( stuff->photo_id, RT_photo );

    /*
    **	Bump RefCnt_, even if ephemeral.  If AddResource_ fails core X11 calls
    **	UtilFreeResource -- our queue would be corrupted if RefCnt_ went to 0.
    */
    RefCnt_(img)++;
    AddResource_( stuff->photo_id, RT_photo, img );
    RefCnt_(img) -= stuff->permanent ? 0 : 1;

    if( stuff->permanent )
	{
	ResType_(img)	      = XieK_Photomap;
	SnkPrm_(Sink_(img))   = TRUE;
	SnkWrite_(Sink_(img)) = TRUE;
	}

    return( Success );
}				/* end ProcTapFlo */

/*-----------------------------------------------------------------------
------------------------- Photoflo Done Procedure -----------------------
------------------------------------------------------------------------*/
int FloDone( client, flo )
 ClientPtr client;	/* client pointer				    */
 PhotofloPtr  flo;	/* Photoflo pointer				    */
{
    INT32	 status;
    FloQuePtr    que = QueSrc_(flo);
    PhotomapPtr  map;
    PipeSinkPtr  snk;
    REQUEST( xieReq );

    if( Run_(flo) )
        {   /*
            **  Kill the DDX Pipeline.
            */
	status = FlushPipeline_( flo );
	OkIf_( status == Success, ResId_(flo), status );
	if( Yielded_(flo) )
	    return( Success );
	/*
	**  Set Photoflo status to indicate execution complete and send event.
	*/
	Done_(flo) = TRUE;
	Run_(flo)  = FALSE;
	xieDoSendEvent( client, flo, XieK_PhotofloEvent );
	}
    /*
    **	Reset Sink attributes on all Photo{map|tap}s.
    */
    do	{
	if( IsPhoto_(FloRes_(que)) )
	    {
	    map			  = FloMap_(que);
	    snk			  = Sink_(map);
	    SnkInited_(snk)	  = FALSE;
	    SnkDtyp_(snk)	  = SnkDatTyp_(snk,0);
	    SnkFull_(snk)	 |= Done_(flo) && IsPhotomap_(map);
	    SnkUdpFinalMsk_(snk) |= Done_(flo) && IsPhotomap_(map)
						? (1<<CmpCnt_(map))-1 : 0;
	    }
	que = FloNxt(que);
	}
    while( que != QueSrc_(flo) );

    /*
    **	"flo" will be freed if the client has already freed it.
    */
    UtilFreeResource(flo);

#ifdef PEZD
    LIB$SHOW_TIMER();
    printf("PEZD : Flo Done\n");
#endif
    return( Success );
}				/* End of FloDone */

/*-----------------------------------------------------------------------
------------------- routine:  Lookup/verify Idc resource ----------------
------------------------------------------------------------------------*/
int LookupIdc( client, idc_ptr, idc_id, type )
 ClientPtr	 client;    /* client pointer, needed by error macros	    */
 CommonPartPtr	*idc_ptr;   /* where to put IdcPtr			    */
 CARD32		 idc_id;    /* X11 resource-id of Idc			    */
 CARD8		 type;	    /* specific Idc type, if checking is wanted	    */
{
    REQUEST( xieReq );

    *idc_ptr = (CommonPartPtr) LookupId_(idc_id, RT_idc);

    IDChoiceOkIf_(*idc_ptr && (!type || type == ResType_(*idc_ptr)), idc_id);

    return( Success );
}				/* end LookupIdc */
/*-----------------------------------------------------------------------
------------------- routine:  Lookup/verify Idc resource-----------------
------------------- which could be either an Roi or a Cpp.---------------
--------------------Return a pointer to the UDP form of the resource ---
------------------------------------------------------------------------*/
int LookupCppRoi( client, idcptr, idcudp, idc_id)
 ClientPtr	 client;    /* client pointer, needed by error macros	    */
 CommonPartPtr   *idcptr;   /* where to return a ptr to the resource        */
 UdpPtr          *idcudp;   /* where to return a ptr to the UDP form        */
 CARD32		 idc_id;    /* X11 resource-id of Idc			    */
{
    CARD8	type;
    CppPtr      cpp;
    RoiPtr      roi;
    REQUEST( xieReq );

    *idcptr = (CommonPartPtr) LookupId_(idc_id, RT_idc);
     type   =  ResType_(*idcptr);

    IDChoiceOkIf_(*idcptr && (type == XieK_IdcRoi || 
			      type == XieK_IdcCpp), idc_id);

    if( type == XieK_IdcCpp )
	{
	cpp = (CppPtr) *idcptr;
	if( idcudp != NULL )
	   *idcudp  = CppUdp_(cpp);
	}
    else
	{
	roi = (RoiPtr) *idcptr;
	if( idcudp != NULL )
	   *idcudp  = RoiUdp_(roi);
	}

    return( Success );
}				/* end LookupCppRoi */

/*-----------------------------------------------------------------------
----------------routine:  Lookup/verify Src & Dst Photo*s ---------------
------------------------------------------------------------------------*/
int LookupPhotos( client, flo, img, src_id, dst_id )
 ClientPtr    client;	/* client pointer, needed by LookupPhotos_ macro    */
 PhotofloPtr *flo;	/* where to put PhotofloPtr: NULL if not allowed    */
 PhotomapPtr *img;	/* where to put PhotomapPtr: NULL if not wanted	    */
 INT32	      src_id;	/* X11 src-id: NOT optional			    */
 INT32	      dst_id;	/* X11 dst-id: don't check = 0, check Make flo = -1 */
{
    CommonPartPtr res;
    REQUEST( xieReq );

    res = (CommonPartPtr) LookupId_( src_id, RT_photo );
    IDChoiceOkIf_( res, src_id );

    switch( ResType_(res) )
	{
    case XieK_Photoflo :
	IDChoiceOkIf_( flo, src_id );		/* Ok if Photoflo is wanted */
	*flo = (PhotofloPtr) res;		/* return Photoflo pointer  */

	if( img != NULL )
	   *img  = DstMap_(*flo);		/* src is end of Photoflo   */
	break;

    case XieK_Photomap :
    case XieK_Phototap :
	IDChoiceOkIf_( img, src_id );		/* Ok if Photomap is wanted */
	*img = (PhotomapPtr) res;		/* return Photomap pointer  */

	if( flo != NULL )
	   *flo  = FloLnk_(*img);		/* return Photoflo pointer  */
	else					/* Ok if not bound to flo   */
	    IDChoiceOkIf_( !FloLnk_(*img), src_id );
	if( !FloLnk_(*img) && dst_id > 0 )
	    CheckNewResource_(dst_id,RT_photo); /* Ok if dst_id is legal    */
	break;

    default : BadIDChoice_(src_id);
	}
    if( flo != NULL  &&  *flo != NULL  &&  dst_id != 0 )
	{
	AccessOkIf_( Make_(*flo), src_id );	/* Ok if under construction */
	IDChoiceOkIf_( dst_id < 0		/* Ok if flo matches dst_id */
	     || *flo == (PhotofloPtr) LookupId_(dst_id, RT_photo), dst_id);
	}
    return( Success );
}				/* end LookupPhotos */

/*-----------------------------------------------------------------------
------------- routine:  Lookup/verify a Template Idc resource------------
------------------------ Return the TmpDat structure --------------------
------------------------------------------------------------------------*/
int LookupTmp( client, idcptr, dat, idc_id)
 ClientPtr	 client;    /* client pointer, needed by error macros	    */
 CommonPartPtr   *idcptr;   /* where to store the resource pointer          */
 TmpDatPtr       *dat;      /* where to store the TmpDat pointer            */
 CARD32		 idc_id;    /* X11 resource-id of Idc			    */
{
    TmpPtr              tmp;
    REQUEST( xieReq );

    *idcptr = (CommonPartPtr) LookupId_(idc_id, RT_idc);
    IDChoiceOkIf_(*idcptr && ResType_(*idcptr) == XieK_IdcTmp, idc_id );

    tmp = (TmpPtr) *idcptr;
    if( dat != NULL )
       *dat  = TmpDat_(tmp);

    return( Success );
}				/* end LookupTmp */

/*-----------------------------------------------------------------------
- routine:  Queue a resource to a flo (ie. remember what's in the Photoflo) -
------------------------------------------------------------------------*/
Bool QueResource( flo, res, NewDst )
PhotofloPtr	flo;	/* Photoflo resource				    */
CommonPartPtr	res;	/* Resource to be added to the queue	            */
Bool		NewDst;	/* True if Photo{map|tap} is new "end-of-Photoflo"  */
{
    FloQuePtr que = QueSrc_(flo);

    while(que != NULL && FloRes_(que) != res)  /* search for resource       */
	  que  = que  == QueEnd_(flo) ?  NULL : FloNxt(que);

    if( que == NULL )
	{
	if( !NewDst || ResId_(DstMap_(flo)) != 0  || Eport_(DstMap_(flo))
		    || QueSrc_(flo)==QueDst_(flo) || Tport_(DstMap_(flo)) )
	    {	/*
		**  Allocate and queue the new resource.
		*/
	    que = (FloQuePtr) DdxMalloc_(sizeof(FloQueRec));
	    if( que == NULL ) return( FALSE );
	    if( QueSrc_(flo) == NULL )
		QueSrc_(flo) = (FloQuePtr) IniQue_(que);    /* init queue   */
	    else
		InsQue_(que,QueSrc_(flo));		    /* insert queue */
	    }
	else
	    {	/*
		**  Delete an untapped Phototap, and re-use its queue element.
		*/
	    SnkDelete_(Sink_(DstMap_(flo))) = TRUE;
	    UtilFreeResource(DstMap_(flo));
	    que = QueDst_(flo);
	    }
	FloRes_(que) = res;		/* stash resource in queue element  */

	if( !IsPhototap_(res) )
	    RefCnt_(res)++;		/* perm resource -- count reference */

	if( IsPhoto_(res) )		/* link Photo{map|tap} to Photoflo  */
	    FloLnk_((PhotomapPtr)res) = flo;
	}
    if( NewDst )
	{
	QueDst_(flo) = que;	/* remember Photo{map|tap} as new DstMap_() */
	SnkFull_(Sink_((PhotomapPtr)res))	 = FALSE;
	SnkUdpFinalMsk_(Sink_((PhotomapPtr)res)) = 0;
	}
    return( TRUE );
}				/* end QueResource */

/*-----------------------------------------------------------------------
----------------- routine:  Clone Photoflo from Photomap ----------------
------------------------------------------------------------------------*/
static int CloneFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr img;
    INT32    status;
    REQUEST( xieCreateByReferenceReq );
    REQUEST_SIZE_MATCH( xieCreateByReferenceReq );
    /*
    **	Verify the Photomap and the new Photoflo id.  Create a Photoflo.
    */
    LookupPhotos_(NULL, &img, stuff->photomap_id, stuff->resource_id);

    flo = (PhotofloPtr) DdxCalloc_(1,sizeof(PhotofloRec));
    AllocOkIf_(flo, stuff->resource_id);
    ResType_(flo) = XieK_Photoflo;
    RefCnt_(flo)  = 1;
    Make_(flo)    = TRUE;
    /*
    **  Create a DDX Pipeline.
    */
    Pipe_(flo) = CreatePipeline_(client, flo);
    status = (int) Pipe_(flo);
    OkIf_( IsPointer_(Pipe_(flo)),
	 ( UtilFreeResource(flo), stuff->resource_id), status );
    /*
    **  Add the Photomap as the source of the Photoflo.
    */
    AllocOkIf_( QueResource( flo, img, FALSE ),
	      ( UtilFreeResource(flo), stuff->resource_id));
    /*
    **  At this point the initial Photomap is also the end of the Photoflo.
    */
    QueDst_(flo) = QueSrc_(flo);

    AddResource_( stuff->resource_id, RT_photo, flo );

    return( Success );
}				/* end CloneFlo */

/*-----------------------------------------------------------------------
----------------- routine:  Clone Photomap from Photomap ----------------
------------------------------------------------------------------------*/
static int CloneMap( client )
 ClientPtr client;	/* client pointer				    */
{
    CARD32	comp, lvl[XieK_MaxComponents];
    PhotomapPtr img, dst;
    REQUEST( xieCreateByReferenceReq );
    REQUEST_SIZE_MATCH( xieCreateByReferenceReq );
    /*
    **	Verify the src Photomap and the new Photomap-id.
    */
    LookupPhotos_(NULL, &img, stuff->photomap_id, stuff->resource_id);

    /*
    **	Create a new Photomap with identical attributes.
    */
    for( comp = 0; comp < CmpCnt_(img); lvl[comp] = uLvl_(img,comp), comp++ );
    dst = UtilCreatePhotomap(CmpMap_(img),  lvl,
			     PxlRatio_(img),PxlProg_(img), ScnProg_(img),
			     PxlPol_(img),  Width_(img),   Height_(img),
			     TRUE, FALSE, DdxAlignDefault_, NULL );
    AllocOkIf_(dst, stuff->resource_id);
    WriteMap_(dst) = TRUE;
    UtilSetPending( dst, 0, XieK_BandByPixel, TRUE );

    AddResource_( stuff->resource_id, RT_photo, dst );

    return( Success );
}				/* end CloneMap */

/*-----------------------------------------------------------------------
---------------------- routine:  Create Cpp Resource --------------------
------------------------------------------------------------------------*/
static int CreateCpp( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr map;
    CppPtr	cpp;
    UdpPtr      u;
    REQUEST( xieCreateCppReq );
    REQUEST_SIZE_MATCH( xieCreateCppReq );

    LookupPhotos_(NULL, &map, stuff->photomap_id, 0);

    CheckNewResource_(stuff->resource_id, RT_idc);
    cpp = (CppPtr) DdxCalloc_(1,sizeof(CppRec));
    AllocOkIf_(cpp, stuff->resource_id);

    ResType_(cpp) = XieK_IdcCpp;
    RefCnt_(cpp)  = 1;
    CppMap_(cpp)  = map;

    /* Describe the CPP data plane in a form suitable for the DDX routines */
    u  = (UdpPtr) DdxCalloc_(1, sizeof(UdpRec));
    AllocOkIf_(u, (UtilFreeResource(cpp), stuff->resource_id));
    CppUdp_(cpp) = u;

    *u = *Udp_(map,0);
    upX1_(u) = stuff->x;
    upY1_(u) = stuff->y;
    upX2_(u) = upX1_(u) + upWidth_(u)  - 1;
    upY2_(u) = upY1_(u) + upHeight_(u) - 1;

    BePCM_( map, (UtilFreeResource(cpp), stuff->photomap_id) );
    RefCnt_(map)++;
    AddResource_( stuff->resource_id, RT_idc, cpp );

    return( Success );
}				/* end CreateCpp */

/*-----------------------------------------------------------------------
------------------ routine:  Create Photo{flo|map} Resource -------------
------------------------------------------------------------------------*/
static int CreatePhoto( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr map;
    PhotofloPtr flo = NULL;
    INT32    status;
    REQUEST( xieCreatePhotoReq );
    REQUEST_SIZE_MATCH( xieCreatePhotoReq );

    ValueOkIf_(stuff->width != 0 && stuff->height != 0, stuff->resource_id);
    CheckNewResource_(stuff->resource_id, RT_photo);
    /*
    **	Create a Photomap (or Phototap if resource_type == Photoflo).
    */
    map = UtilCreatePhotomap( stuff->component_mapping, stuff->levels,
			      MiDecodeDouble( &stuff->aspect_ratio ),
			      stuff->pixel_progression, stuff->line_progression,
			      stuff->polarity,
			      stuff->width, stuff->height,
			      stuff->resource_type == XieK_Photomap,
			      FALSE, DdxAlignDefault_, NULL );
    AllocOkIf_(map, stuff->resource_id);
    WriteMap_(map) = TRUE;
    UtilSetPending( map, 0, XieK_BandByPixel, TRUE );

    if( stuff->resource_type == XieK_Photoflo )
	{   /*
	    **  Create a Photoflo with a DDX Pipeline attached.
	    */
	flo = (PhotofloPtr) DdxCalloc_(1,sizeof(PhotofloRec));
	AllocOkIf_(flo, (UtilFreeResource(map), stuff->resource_id));
	ResType_(flo) = XieK_Photoflo;
	RefCnt_(flo)  = 1;
	Make_(flo)    = TRUE;
	Pipe_(flo)    = CreatePipeline_(client, flo);
	status    = (int) Pipe_(flo);
	OkIf_( IsPointer_(Pipe_(flo)),
	     ( UtilFreeResource(map),
	       UtilFreeResource(flo), stuff->resource_id), status );
	/*
	**  Queue the new Phototap as the source (and end) of the Photoflo.
	*/
	AllocOkIf_( QueResource( flo, map, FALSE ),
		  ( UtilFreeResource(map),
		    UtilFreeResource(flo), stuff->resource_id ));
	QueDst_(flo) = QueSrc_(flo);
	}
    AddResource_( stuff->resource_id, RT_photo, flo ? (CommonPartPtr) flo
						    : (CommonPartPtr) map );
    return( Success );
}				/* end CreatePhoto */

/*-----------------------------------------------------------------------
---------------------- routine:  Create Roi Resource --------------------
------------------------------------------------------------------------*/
static int CreateRoi( client )
 ClientPtr client;	/* client pointer				    */
{
    RoiPtr roi;
    UdpPtr u;
    REQUEST( xieCreateRoiReq );
    REQUEST_SIZE_MATCH( xieCreateRoiReq );

    ValueOkIf_(stuff->width != 0 && stuff->height != 0, stuff->resource_id);
    CheckNewResource_(stuff->resource_id, RT_idc);
    roi = (RoiPtr) DdxCalloc_(1,sizeof(RoiRec));
    AllocOkIf_(roi, stuff->resource_id);
    ResType_(roi) = XieK_IdcRoi;
    RefCnt_(roi)  = 1;

    /* Describe the Roi in a form suitable for the DDX routines */
    u  = (UdpPtr) DdxCalloc_(1, sizeof(UdpRec));
    AllocOkIf_(u, (UtilFreeResource(roi), stuff->resource_id));
    RoiUdp_(roi)    = u;
    upDType_(u)	    = UdpK_DTypeVU;
    upClass_(u)	    = UdpK_ClassUBA;
    upWidth_(u)	    = stuff->width;
    upHeight_(u)    = stuff->height;
    upX1_(u)	    = stuff->x;
    upY1_(u)	    = stuff->y;
    upX2_(u)	    = upX1_(u) + upWidth_(u)  - 1;
    upY2_(u)	    = upY1_(u) + upHeight_(u) - 1;

    AddResource_(stuff->resource_id, RT_idc, roi);

    return( Success );
}				/* end CreateRoi */

/*-----------------------------------------------------------------------
---------------------- routine:  Create Tmp Resource --------------------
------------------------------------------------------------------------*/
static int CreateTmp( client )
 ClientPtr client;	/* client pointer				    */
{
    TmpPtr	tmp;
    TmpEntryPtr entry;
    XieTmpEntry data;
    TmpDatPtr   p;
    TmpDatEntryPtr row;
    CARD32	index, i, j, k, last, max_x, max_y, x, y;
    REQUEST( xieCreateTmpReq );
    REQUEST_AT_LEAST_SIZE( xieCreateTmpReq );

    CheckNewResource_(stuff->resource_id, RT_idc);
    ValueOkIf_(stuff->data_count > 0, 0);

    tmp = (TmpPtr) DdxCalloc_(1, sizeof(TmpRec)
			       + sizeof(TmpEntryRec) * stuff->data_count);
    AllocOkIf_(tmp, stuff->resource_id);
    ResType_(tmp) = XieK_IdcTmp;
    RefCnt_(tmp)  = 1;
    TmpX_(tmp)    = stuff->center_x;
    TmpY_(tmp)    = stuff->center_y;
    TmpCnt_(tmp)  = stuff->data_count;
    entry =  TmpEnt_(tmp);
    data  = (XieTmpEntry) &stuff[1];
    max_x = data[0].x;
    max_y = data[0].y;

    /* Find the x and y size of the kernel and sanity check the indices */
    for( index = 0; index < TmpCnt_(tmp); index++ )
	{
	entry[index].x	    = data[index].x;
	entry[index].y	    = data[index].y;
	ValueOkIf_(entry[index].x >= 0 && entry[index].y >= 0,
					  stuff->resource_id);
	entry[index].value  = MiDecodeDouble( &data[index].value );
	max_x = max(max_x, entry[index].x);
	max_y = max(max_y, entry[index].y);
	}
    /* Describe the template in a form for the DDX routines */
    /* The array has an extra element on each row to act as a marker */
    p  = (TmpDatPtr) DdxMalloc_(sizeof(TmpDatRec));
    AllocOkIf_(p, (UtilFreeResource(tmp), stuff->resource_id));
    p->x_count	    = max_x + 1;
    p->y_count      = max_y + 1;
    p->center_x	    = TmpX_(tmp);
    p->center_y	    = TmpY_(tmp);
    p->array        = (TmpDatEntryPtr) DdxMalloc_((p->x_count + 1)
						 * p->y_count
						 * sizeof(TmpDatEntryRec));
    AllocOkIf_(p->array, (UtilFreeResource(tmp), stuff->resource_id));

    /* Zero the array */
    row = p->array;
    for (i = 0; i < p->y_count; i++) {
	for (j = 0; j < p->x_count+1; j++) {
	    row[j].value = 0.0;
	    row[j].increment = 0;
	}
	row += (p->x_count+1);
    }

    /* Insert the template data into the array.
     * Note that the coordinate system used by the template uses the
     * normal mathematical axes orientation (i.e. y increases up).
     */
    for (index = 0; index < TmpCnt_(tmp); index++) {

	x   = entry[index].x;
	y   = p->y_count - entry[index].y  - 1;
	row = p->array + y * (p->x_count+1);
	row[x].value = entry[index].value;
	row[x].increment = 1;
    }
    /* Compact the rows, ending with a marker (increment==0) */
    row = p->array;
    for (i = 0; i < p->y_count; i++) {

	last = -1;

	for( j = 0; j < p->x_count; j++) {

	    if (row[j].increment == 0) {
		for (k = j + 1; k < p->x_count ; k++) {
		    if (row[k].increment != 0) {
			row[j] = row[k];
			row[j].increment = k - last;
			row[k].increment = 0;
			last = k;
			break;
		    }
		}
		if (k == p->x_count)
		    break;
	    } else
		last++;
	}
	row += (p->x_count+1);
    }
    TmpDat_(tmp) = p;

    AddResource_(stuff->resource_id, RT_idc, tmp);

    return( Success );
}				/* end CreateTmp */

/*-----------------------------------------------------------------------
---------------------- routine:  Query Cpp Resource ---------------------
------------------------------------------------------------------------*/
static int QueryCpp( client )
 ClientPtr client;	/* client pointer				    */
{
    CppPtr	cpp;
    xieQueryCppReply	  rep;
    REQUEST( xieQueryResourceReq );

    LookupIdc_(&cpp, stuff->resource_id, XieK_IdcCpp);
    rep.type		= X_Reply;
    rep._reserved	= 0;
    rep.sequenceNumber	= client->sequence;
    rep.length		= 0;
    rep.x		= CppX_(cpp);
    rep.y		= CppY_(cpp);
    rep.photomap_id	= ResId_(CppMap_(cpp));
    WriteXieReplyToClient( client, sizeof(xieQueryCppReply), &rep );

    return( Success );
}				/* end QueryCpp */

/*-----------------------------------------------------------------------
--------------------- routine:  Query Photoflo Resource -----------------
------------------------------------------------------------------------*/
static int QueryFlo( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    xieQueryPhotofloReply rep;
    REQUEST( xieQueryResourceReq );

    LookupPhotos_(&flo, NULL, stuff->resource_id, 0);
    rep.type		 =  X_Reply;
    rep.sequenceNumber	 =  client->sequence;
    rep.length		 =  0;
    rep.status		 = Make_(flo) ? XieK_PhotofloFormation
			 : Run_(flo)  ? XieK_PhotofloRunning
			 : Done_(flo) ? XieK_PhotofloComplete
				      : XieK_PhotofloAborted;

    WriteXieReplyToClient( client, sizeof(xieQueryPhotofloReply), &rep );

    return( Success );
}				/* end QueryFlo */

/*-----------------------------------------------------------------------
------------------- routine:  Query Photomap Resource -------------------
------------------------------------------------------------------------*/
static int QueryMap( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr img;
    CARD32	index;
    xieQueryPhotomapReply rep;
    REQUEST( xieQueryResourceReq );

    LookupPhotos_(&flo, &img, stuff->resource_id, 0);
    rep.type		  = X_Reply;
    rep.sequenceNumber	  = client->sequence;
    rep.length		  = sizeof(xieQueryPhotomapReply)-32>>2;
    rep.width		  = Width_(img);
    rep.height		  = Height_(img);
    MiEncodeDouble( &rep.aspect_ratio, PxlRatio_(img) );
    rep.polarity	  = PxlPol_(img);
    rep.pixel_progression = PxlProg_(img);
    rep.line_progression  = ScnProg_(img);
    rep.constrained	  = Constrained_(img);
    rep.component_mapping = CmpMap_(img);
    rep.component_count	  = CmpCnt_(img);
    for( index = 0; index < CmpCnt_(img); index++ )
	rep.levels[index] = uLvl_(img,index);
    rep.photo_id	  = ResId_(img);

    WriteXieReplyToClient( client, sizeof(xieQueryPhotomapReply), &rep );

    return( Success );
}				/* end QueryMap */

/*-----------------------------------------------------------------------
---------------------- routine:  Query Roi Resource ---------------------
------------------------------------------------------------------------*/
static int QueryRoi( client )
 ClientPtr client;	/* client pointer				    */
{
    RoiPtr  roi;
    xieQueryRoiReply rep;
    REQUEST( xieQueryResourceReq );

    LookupIdc_(&roi, stuff->resource_id, XieK_IdcRoi);
    rep.type		= X_Reply;
    rep._reserved	= 0;
    rep.sequenceNumber	= client->sequence;
    rep.length		= 0;
    rep.x		= RoiX_(roi);
    rep.y		= RoiY_(roi);
    rep.width		= RoiW_(roi);
    rep.height		= RoiH_(roi);
    WriteXieReplyToClient( client, sizeof(xieQueryRoiReply), &rep );

    return( Success );
}				/* end QueryRoi */

/*-----------------------------------------------------------------------
---------------------- routine:  Query Tmp Resource ---------------------
------------------------------------------------------------------------*/
static int QueryTmp( client )
 ClientPtr client;	/* client pointer				    */
{
    TmpPtr	tmp;
    TmpEntryPtr	get;
    XieTmpEntry put;
    CARD32	index;
    xieQueryTmpReply rep;
    REQUEST( xieQueryResourceReq );

    LookupIdc_(&tmp, stuff->resource_id, XieK_IdcTmp);
    rep.type		= X_Reply;
    rep._reserved	= 0;
    rep.sequenceNumber	= client->sequence;
    rep.length		= TmpCnt_(tmp) * sizeof(XieTmpEntryRec) >> 2;
    rep.center_x	= TmpX_(tmp);
    rep.center_y	= TmpY_(tmp);
    rep.data_count	= TmpCnt_(tmp);
    get = TmpEnt_(tmp);
    put = (XieTmpEntry) DdxMalloc_(TmpCnt_(tmp) * sizeof(XieTmpEntryRec));
    AllocOkIf_(put, stuff->resource_id);
    for( index = 0; index < TmpCnt_(tmp); index++ )
	{
	put[index].x   = get[index].x;
	put[index].y   = get[index].y;
	MiEncodeDouble( &put[index].value, get[index].value );
	}
    WriteXieReplyToClient( client, sizeof(xieQueryTmpReply), &rep );
    client->pSwapReplyFunc = SwapTmpWrite;
    WriteSwappedXieDataToClient( client,
			     sizeof(XieTmpEntryRec) * TmpCnt_(tmp), put );
    DdxFree_( put );

    return( Success );
}				/* end QueryTmp */
/* end module XieResource.c */
