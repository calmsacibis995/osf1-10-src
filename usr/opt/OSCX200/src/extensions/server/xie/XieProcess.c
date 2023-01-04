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

*******************************************************************************/

/*******************************************************************************
**
**  FACILITY:
**
**      X Image Extension DIX
**
**  ABSTRACT:
**
**	This Xie module consists of DIX procedures for process
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

/*
**  Table of contents
*/
    /*
    **  Xie protocol proceedures called from XieMain
    */
int  ProcArea();
int  ProcAreaStats();
int  ProcArith();
int  ProcChromeCom();
int  ProcChromeSep();
int  ProcCompare();
int  ProcConstrain();
int  ProcCrop();
int  ProcDither();
int  ProcFill();
int  ProcCalcHistogram();
int  ProcLogical();
int  ProcLuminance();
int  ProcMatchHistogram();
int  ProcMath();
int  ProcMirror();
int  ProcPoint();
int  ProcPointStats();
int  ProcRotate();
int  ProcScale();
int  ProcTranslate();
    /*
    **  routines used internally by XieProcess
    */
static RoiPtr	RoiEntirePhotomap();

/*
**  Equated Symbols
*/
/*
**  MACRO definitions
*/
#define QueNewFloDst_(flo,ele,id) \
	    AllocOkIf_(QueResource((flo),(ele),TRUE),(UtilFreeResource(ele),id))
#define QueResource_(flo,res,id) \
	    AllocOkIf_(QueResource((flo),(res),FALSE),id);

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
extern int	    xieDoSendEvent();	    /* XieEvents.c		    */
extern int	    LookupCppRoi();	    /* called by LookupCppRoi_ macro */
extern int	    LookupIdc();	    /* called by LookupIdc_ macro   */
extern int	    LookupPhotos();	    /* called by LookupPhotos_ macro*/
extern int	    LookupTmp();	    /* called by LookupTmp_ macro   */
extern int	    UtilBePCM();	    /* called by BePCM_ macro	    */
extern Bool	    QueResource();	    /* Queue a Photoflo'd resource  */
extern PhotomapPtr  UtilCreatePhotomap();   /* create a Photomap structure  */
extern void         GetConstraintModel();   /* Get this sessions constraint */


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
externalref void (*xieDixSwapData[])();
extern void	SwapFloatWrite();

/*
**	Local Storage
*/

/*-----------------------------------------------------------------------
---------------------------  Area  Procedure ----------------------------
------------------------------------------------------------------------*/
int ProcArea( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr src,  dst, realdst = NULL;
    UdpPtr	   idcudp = NULL;
    CommonPartPtr  idcptr = NULL;
    CommonPartPtr  kern_idc;
    TmpDatPtr	   kern_dat;
    CARD32 s, i, model, comp, lvl[XieK_MaxComponents];
    REQUEST( xieAreaReq );
    REQUEST_SIZE_MATCH( xieAreaReq );

    /* Locate source Photomap {and Photoflo} and verify destination Id */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);

    /* Locate template Idc containing the kernel, and convert it */
    LookupTmp_(&kern_idc, &kern_dat, stuff->ipct_id);

    /* Validate the operator codes */
    ValueOkIf_(((stuff->op1 == XieK_AreaOp1Add ||
		 stuff->op1 == XieK_AreaOp1Mult)
	       && (stuff->op2 >= XieK_AreaOp2Min &&
		   stuff->op2 <= XieK_AreaOp2Sum)), stuff->src_photo_id);

    if( stuff->ipcc_id != 0 )
	{
        /* Get a UDP describing the ROI or CPP */
	LookupCppRoi_(&idcptr, &idcudp, stuff->ipcc_id);
	ValueOkIf_(UtilValidRegion(src, idcudp), stuff->ipcc_id);
	}
    /* Create destination based on the source operand */
    for( i = 0; i < XieK_MaxComponents; i++)
	lvl[i] = 0;		/* Make destination photo  unconstrained */

    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignArea_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    /* If the result should be constrained, get another photomap */
    if (stuff->constrain)
	{
	GetConstraintModel(client, &model, lvl);
	realdst = UtilCreatePhotomap(CmpMap_(dst), lvl, PxlRatio_(dst),
				     PxlProg_(dst), ScnProg_(dst), PxlPol_(dst),
				     Width_(dst), Height_(dst),
				     flo == NULL, FALSE, DdxAlignArea_ );
	AllocOkIf_(realdst, (UtilFreeResource(dst), stuff->dst_photo_id));
	}
    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr,   FALSE))
		       && QueResource(flo, kern_idc, FALSE)
		       && QueResource(flo, dst,      TRUE)) ? BadAlloc
	  : DdxCreateArea_( Pipe_(flo), Sink_(src), Sink_(dst),
			    kern_dat, idcudp, stuff->op1, stuff->op2 );
	if( s == Success && stuff->constrain )
	    s = !QueResource(flo, realdst, TRUE) ? BadAlloc
	      :  DdxCreateConstrain_( Pipe_(flo), Sink_(dst),
				      Sink_(realdst), model);
	OkIf_( s == Success, (UtilFreeResource(dst),
			      UtilFreeResource(realdst), ResId_(flo)), s );
	}
    else
	{
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    {   /* Call Area routine (once per component) */
	    s = DdxArea_( Udp_(src,comp), Udp_(dst,comp), kern_dat, idcudp,
			  stuff->op1, stuff->op2 );
	    if( s == Success && stuff->constrain )
		s  = DdxConstrain_( Udp_(dst,comp), Udp_(realdst,comp), model);
	    }
	if( stuff->constrain )
	    {
	    UtilFreeResource(dst);
	    dst = realdst;
	    }
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_( stuff->dst_photo_id, RT_photo, dst );
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcArea */

/*-----------------------------------------------------------------------
-----------------------  Area  Statistics Procedure ---------------------
------------------------------------------------------------------------*/
int ProcAreaStats( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr	    src, dst, realdst = NULL;
    CommonPartPtr   kern_idc;
    TmpDatPtr	    kern_dat;
    CommonPartPtr   idcptr = NULL;
    UdpPtr	    idcudp = NULL;
    CARD32	    s, i, model, comp, lvl[XieK_MaxComponents];

    REQUEST( xieAreaStatsReq );
    REQUEST_SIZE_MATCH( xieAreaStatsReq );

    /* Locate source Photomap and verify destination Id */
    LookupPhotos_(NULL, &src, stuff->src_photo_id, stuff->dst_photo_id);

    /* Resource MUST be monochrome */
    MatchOkIf_(!IsRGB_(src), stuff->src_photo_id);

    /* Validate the statistics type */
    ValueOkIf_((stuff->stat_type >= XieK_Minimum &&
		stuff->stat_type <= XieK_Variance), stuff->src_photo_id);

    /* Lookup IDC if specified */
    if( stuff->idc_id != 0 )  {
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src, idcudp ), stuff->idc_id );
    }

    /* Create the output photomap.  If the input is unconstrained, then
     * force the output to be unconstrained.  If the input is constrained,
     * force the output to be the number of levels we will constrain to
     * under the current constraint mode.
     */
    GetConstraintModel(client, &model, lvl);
    if (!Constrained_(src))
	for( i = 0;  i < XieK_MaxComponents;  lvl[i++] = 0 );

    BePCM_( src, ResId_(src) );
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      TRUE, FALSE, DdxAlignDefault_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    /* If we are not operating over the entire image, fill the destination
     * UDP from the source, then operate over the IDC.
     */
    if( idcudp != NULL )
	DdxConvert_(Udp_(src, 0), Udp_(dst, 0), XieK_MoveMode);

    s = DdxStatsArea_( Udp_(src,0), Udp_(dst,0), idcudp,
		       stuff->stat_type, stuff->edge_size, model );
    OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
    AddResource_(stuff->dst_photo_id, RT_photo, dst);
    xieDoSendEvent( client, dst, XieK_ComputationEvent );

    return( Success );
}				/* End of ProcAreaStats */

/*-----------------------------------------------------------------------
---------------------------  Arith  Procedure ----------------------------
------------------------------------------------------------------------*/
int ProcArith( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, flo2;
    PhotomapPtr dst, src1, src2 = NULL, realdst = NULL;
    CommonPartPtr idcptr = NULL;
    UdpPtr        idcudp = NULL;
    CARD32 s, i, model, comp, lvl[XieK_MaxComponents];
    REQUEST( xieArithReq );
    REQUEST_SIZE_MATCH( xieArithReq );
    /* 
    **	Locate source Photomap(s) {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify the second operand is bound to it.
    */
    LookupPhotos_(&flo, &src1, stuff->src1_photo_id, stuff->dst_photo_id);
    if( stuff->src2_photo_id != 0 )
	{
        LookupPhotos_(&flo2, &src2, stuff->src2_photo_id, -1);
	AccessOkIf_( flo == flo2, stuff->src2_photo_id);
	MatchOkIf_(CmpCnt_(src1) == CmpCnt_(src2), stuff->src2_photo_id);
	}
    if( stuff->idc_id != 0 ) {
        /* Get a UDP describing the ROI or CPP */
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_(UtilValidRegion( src1, idcudp ), stuff->idc_id );
	if (stuff->src2_photo_id != 0)
	    ValueOkIf_(UtilValidRegion( src2, idcudp ), stuff->idc_id );
    }
    /*
    **	Create destination based on first source operand.
    */
    for( i = 0; i < XieK_MaxComponents; i++ )
	lvl[i] = 0;	/* Make destination Photo unconstrained */
    dst = UtilCreatePhotomap( CmpMap_(src1), lvl, PxlRatio_(src1),
			      PxlProg_(src1), ScnProg_(src1), PxlPol_(src1),
			      Width_(src1), Height_(src1), 
			      flo == NULL, FALSE, DdxAlignArith_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( stuff->constrain )
	{
	GetConstraintModel(client, &model, lvl);
	realdst = UtilCreatePhotomap(CmpMap_(dst), lvl, PxlRatio_(dst),
				     PxlProg_(dst), ScnProg_(dst), PxlPol_(dst),
				     Width_(dst), Height_(dst),
				     flo == NULL, FALSE, DdxAlignArith_ );
	AllocOkIf_(realdst, (UtilFreeResource(dst), stuff->dst_photo_id));
	}
    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateArith_( Pipe_(flo), Sink_(src1), src2 ? Sink_(src2) : NULL,
			     stuff->constants, Sink_(dst), idcudp, stuff->op);
	if( s == Success && stuff->constrain )
	    s = !QueResource(flo, realdst,  TRUE) ? BadAlloc
	      :  DdxCreateConstrain_( Pipe_(flo), Sink_(dst),
				      Sink_(realdst), model);
	OkIf_(s == Success, (UtilFreeResource(dst),
			     UtilFreeResource(realdst), ResId_(flo)), s);
	}
    else
	{
	BePCM_( src1, (UtilFreeResource(dst),
		       UtilFreeResource(realdst), ResId_(src1)) );
	BePCM_( src2, (UtilFreeResource(dst),
		       UtilFreeResource(realdst), ResId_(src2)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src1) && s == Success; comp++)
	    {   /* Call Arithmetic routine (once per component) */
	    s = DdxArith_(Udp_(src1,comp), src2 ? Udp_(src2,comp) : NULL,
			  stuff->constants, Udp_(dst,comp), idcudp, stuff->op);
	    if( s == Success && stuff->constrain )
		s  = DdxConstrain_(Udp_(dst,comp), Udp_(realdst,comp), model);
	    }
	if( stuff->constrain )
	    {
	    UtilFreeResource(dst);
	    dst = realdst;
	    }
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
    }
    return( Success );
}					/* end ProcArith */

/*-----------------------------------------------------------------------
-------------------------  Chrominance Combine --------------------------
------------------------------------------------------------------------*/
int ProcChromeCom( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, flo2;
    PhotomapPtr src[XieK_MaxComponents], dst;
    CARD32 	s, height, width, comp, lvl[XieK_MaxComponents];
    REQUEST( xieChromeComReq );
    REQUEST_SIZE_MATCH( xieChromeComReq );
    /* 
    **	Locate source Photomaps {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify all the operands are bound to it.
    */
    LookupPhotos_(&flo, &src[0], stuff->photo_id1, stuff->dst_photo_id);
    MatchOkIf_( !IsRGB_(src[0]), stuff->photo_id1);

    LookupPhotos_(&flo2, &src[1], stuff->photo_id2, -1);
    AccessOkIf_( flo == flo2, stuff->photo_id2);
    MatchOkIf_( !IsRGB_(src[1]), stuff->photo_id2);
    MatchOkIf_(Constrained_(src[1]) == Constrained_(src[0]), stuff->photo_id2);

    LookupPhotos_(&flo2, &src[2], stuff->photo_id3, -1);
    AccessOkIf_( flo == flo2, stuff->photo_id3);
    MatchOkIf_( !IsRGB_(src[2]), stuff->photo_id3);
    MatchOkIf_(Constrained_(src[2]) == Constrained_(src[0]), stuff->photo_id3);
    /*
    **	Create the destination photomap
    */
    for (comp = 0; comp < XieK_MaxComponents; comp++)
	lvl[comp] = uLvl_(src[comp],0);
    width  = min( Width_(src[0]), min( Width_(src[1]),  Width_(src[2])));
    height = min(Height_(src[0]), min(Height_(src[1]), Height_(src[2])));

    dst = UtilCreatePhotomap(XieK_RGB, lvl, PxlRatio_(src[0]),
			     PxlProg_(src[0]), ScnProg_(src[0]),PxlPol_(src[0]),
			     width, height,
			     flo == NULL, FALSE, DdxAlignChromeCom_ );
    AllocOkIf_(dst, stuff->dst_photo_id);
    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateChromeCom_( Pipe_(flo),
				 Sink_(src[0]), Sink_(src[1]), Sink_(src[2]),
				 Sink_(dst) );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{
	for( comp = 0; comp < XieK_MaxComponents; comp++ )
	    { /* Call the ChromeCom routine for each input */
	    BePCM_( src[comp], (UtilFreeResource(dst), ResId_(src[comp])) );
	    s = DdxChromeCom_(Udp_(src[comp],0), Udp_(dst,comp), XieK_MoveMode);
	    OkIf_(s==Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	    }
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcChromeCom */

/*-----------------------------------------------------------------------
-------------------------  Chrominance Separate -------------------------
------------------------------------------------------------------------*/
int ProcChromeSep( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, flo2;
    PhotomapPtr src, dst;
    CARD32 	s, i, comp,lvl[XieK_MaxComponents];
    REQUEST( xieChromeSepReq );
    REQUEST_SIZE_MATCH( xieChromeSepReq );
    /* 
    **	Locate source Photomaps {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify all the operands are bound to it.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo, stuff->dst_photo);
    MatchOkIf_(CmpCnt_(src) > 1, stuff->dst_photo);
    ValueOkIf_(stuff->component >=0 &&
	       stuff->component < XieK_MaxComponents, 0);
    /*
    **	Create destination based on first source operand.
    */
    lvl[0] = uLvl_(src,stuff->component);
    lvl[1] = 0;
    lvl[2] = 0;
    dst = UtilCreatePhotomap(lvl[0] == 2 ? XieK_Bitonal : XieK_GrayScale, lvl,
			     PxlRatio_(src), PxlProg_(src), ScnProg_(src),
			     PxlPol_(src), Width_(src), Height_(src),
			     flo == NULL, FALSE, DdxAlignChromeSep_ );
    AllocOkIf_(dst, stuff->dst_photo);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateChromeSep_( Pipe_(flo), Sink_(src), Sink_(dst),
				 stuff->component );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{ /* Call the ChromeSep routine for the desired component */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	s = DdxChromeSep_(Udp_(src,stuff->component),Udp_(dst,0),XieK_MoveMode);
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo), s);
	AddResource_(stuff->dst_photo, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcChromeSep */

/*-----------------------------------------------------------------------
---------------------------  Compare Procedure --------------------------
------------------------------------------------------------------------*/
int ProcCompare( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, flo2;
    PhotomapPtr src1, src2 = NULL, dst;
    CommonPartPtr idcptr = NULL;
    UdpPtr        idcudp = NULL;
    UdpPtr        tmpudp = NULL;
    CARD32 s, comp, comp_out, combine_op, lvl[XieK_MaxComponents];
    REQUEST( xieCompareReq );
    REQUEST_SIZE_MATCH( xieCompareReq );
    /* 
    **	Locate source Photomap(s) {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify the second operand is bound to it.
    */
    LookupPhotos_(&flo, &src1, stuff->src1_photo_id, stuff->dst_photo_id);

    if( stuff->src2_photo_id != 0 )
	{
        LookupPhotos_(&flo2, &src2, stuff->src2_photo_id, -1);
	AccessOkIf_(flo==flo2,stuff->src2_photo_id);
	MatchOkIf_(CmpCnt_(src1) == CmpCnt_(src2), stuff->src2_photo_id);
	}

    if( stuff->idc_id != 0 ) {
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src1, idcudp ), stuff->idc_id);
	if (stuff->src2_photo_id != 0)
	    ValueOkIf_( UtilValidRegion( src2, idcudp ), stuff->idc_id);
    }

    /* Only "equal" and "not equal" are defined for RGB images */ 
    MatchOkIf_(!IsRGB_(src1) || stuff->op == XieK_EQ || stuff->op == XieK_NE,
		stuff->src1_photo_id);
    /*
    **	Create destination based on first source operand.
    **  Destination is always bitonal or RGB (2 levels).
    */
    comp_out = stuff->combine ? 1 : CmpCnt_(src1);
    for( comp = 0; comp < comp_out; lvl[comp] = 2, comp++);
    dst = UtilCreatePhotomap( comp_out == 1 ? XieK_Bitonal : XieK_RGB, lvl,
			      PxlRatio_(src1), PxlProg_(src1), ScnProg_(src1),
			      PxlPol_(src1), Width_(src1), Height_(src1),
			      flo == NULL, FALSE, DdxAlignCompare_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateCompare_( Pipe_(flo), Sink_(src1),
				    src2 ? Sink_(src2) : NULL,
			       stuff->constants, Sink_(dst), idcudp,
			       stuff->op, stuff->combine );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{ /* Call Compare routine (per component), then combine results */
	combine_op = stuff->op == XieK_NE ? XieK_OR : XieK_AND;
	BePCM_( src1, (UtilFreeResource(dst), ResId_(src1)) );
	BePCM_( src2, (UtilFreeResource(dst), ResId_(src2)) );
	
	if( comp_out < CmpCnt_(src1) )
	    {	/* Get a temporary UDP to hold the intermediate results */
	    tmpudp = (UdpPtr) DdxMalloc_(sizeof(UdpRec));
	    AllocOkIf_(tmpudp, (UtilFreeResource(dst), stuff->dst_photo_id));
	    *tmpudp = *Udp_(dst,0);
	    tmpudp->UdpA_Base = DdxMallocBits_(tmpudp->UdpL_ArSize);
	    AllocOkIf_(tmpudp->UdpA_Base,
		      (UtilFreeResource(dst), stuff->dst_photo_id));
	    }
	/* Process the first component */
	s = DdxCompare_( Udp_(src1,0), src2 ? Udp_(src2,0) : NULL,
			 stuff->constants, Udp_(dst,0), idcudp, stuff->op );

	for( comp = 1; comp < CmpCnt_(src1) && s == Success; comp++ )
	    {
	    s = DdxCompare_( Udp_(src1,comp), src2 ? Udp_(src2,comp) : NULL,
			     stuff->constants, 
			     stuff->combine ? tmpudp : Udp_(dst,comp),
			     idcudp, stuff->op);
	    if( s == Success && stuff->combine )
		s  = DdxLogical_( tmpudp, Udp_(dst,0), NULL,
				  Udp_(dst,0), idcudp, combine_op );
	    }
	if( tmpudp != NULL )
	    UtilFreeUdp(tmpudp);
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcCompare */

/*-----------------------------------------------------------------------
---------------------------  Constrain Procedure ------------------------
------------------------------------------------------------------------*/
int ProcConstrain( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr	src, dst;
    CARD32 s, i, model, comp, compmpg, lvl[XieK_MaxComponents];
    REQUEST( xieConstrainReq );
    REQUEST_SIZE_MATCH( xieConstrainReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    /*
    **	Get the constraint model info.
    */
    if( stuff->model != 0 )
	{
	ValueOkIf_( stuff->model == XieK_HardClip
		 || stuff->model == XieK_ClipScale
		 || stuff->model == XieK_HardScale, 0 );
	model = stuff->model;
	for( i = 0; i < XieK_MaxComponents; i++ )
	    {
	    lvl[i] = stuff->levels[i];
	    ValueOkIf_(lvl[i] > 0, stuff->dst_photo_id);
	    }
	}
    else
	GetConstraintModel(client, &model, lvl);

    /* Adjust component mapping */
    if (CmpCnt_(src) > 1)
	compmpg = XieK_RGB;
    else
	compmpg = lvl[0] > 2 ? XieK_GrayScale : XieK_Bitonal;

    dst = UtilCreatePhotomap(compmpg, lvl, PxlRatio_(src),
			     PxlProg_(src), ScnProg_(src), PxlPol_(src),
			     Width_(src), Height_(src),
			     flo == NULL, FALSE, DdxAlignConstrain_ );
    AllocOkIf_(dst, stuff->dst_photo_id);
    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateConstrain_(Pipe_(flo), Sink_(src), Sink_(dst), model);
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Constrain routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++ )
	    s = DdxConstrain_(Udp_(src,comp), Udp_(dst,comp), model);
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}					/* end ProcConstrain */

/*-----------------------------------------------------------------------
---------------------------- Crop Procedure -----------------------------
------------------------------------------------------------------------*/
int ProcCrop( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr  flo;
    PhotomapPtr  src,  dst;
    UdpRec	 roi_udp;
    RoiRec	 all, *roi;
    CARD32	 s, comp, lvl[XieK_MaxComponents];
    CARD32	 width, height;
    REQUEST( xieCropReq );
    REQUEST_SIZE_MATCH( xieCropReq );
    /* 
    **	Verify src Photo{flo&|map}, verify dst Id, and establish the Roi.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    if( stuff->idc_id == 0 )
	roi = RoiEntirePhotomap(src, &all, &roi_udp);
    else {
	LookupIdc_(&roi,stuff->idc_id,XieK_IdcRoi);
	ValueOkIf_( UtilValidRegion( src, RoiUdp_(roi)), stuff->idc_id );
    }

    for( comp = 0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++);

    /*
    ** Clip the ROI to the source image bounds.
    */
    width =  min(RoiX_(roi) + RoiW_(roi) - 1, uX2_(src, 0)) - RoiX_(roi) + 1;
    height = min(RoiY_(roi) + RoiH_(roi) - 1, uY2_(src, 0)) - RoiY_(roi) + 1;

    dst = UtilCreatePhotomap(CmpMap_(src),  lvl, PxlRatio_(src),
			     PxlProg_(src), ScnProg_(src), PxlPol_(src),
			     width, height,
			     flo == NULL, FALSE, DdxAlignCrop_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateCrop_( Pipe_(flo), Sink_(src), Sink_(dst),
			    RoiX_(roi), RoiY_(roi), width, height );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Crop routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxCrop_(Udp_(src,comp), Udp_(dst,comp),    /* Udps	    */
			 RoiX_(roi),	 RoiY_(roi), 0, 0,  /* coordinates  */
			 width,	 height );	    /* dimensions   */
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcCrop */

/*-----------------------------------------------------------------------
------------------------------  Dither Procedure ------------------------
------------------------------------------------------------------------*/
int ProcDither( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr src, dst;
    CARD32  s, comp, compmpg;
    REQUEST( xieDitherReq );
    REQUEST_SIZE_MATCH( xieDitherReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    **	Create destination Photomap.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    MatchOkIf_(Constrained_(src), stuff->src_photo_id);

    for( comp = 0; comp < CmpCnt_(src); comp++ )
	{
	MatchOkIf_(stuff->levels[comp] <= uLvl_(src,comp), stuff->src_photo_id);
	ValueOkIf_(stuff->levels[comp] > 0, stuff->src_photo_id);
	}
    compmpg = IsGrayScale_(src) && stuff->levels[0] <= 2 ? XieK_Bitonal
							 : CmpMap_(src);
    dst = UtilCreatePhotomap( compmpg, stuff->levels, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignDither_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateDither_( Pipe_(flo), Sink_(src), Sink_(dst) );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Dither routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxDither_(Udp_(src,comp), Udp_(dst,comp));
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}				/* End of ProcDither */

/*-----------------------------------------------------------------------
-----------------------------  Fill Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcFill( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo;
    PhotomapPtr src, dst;
    UdpPtr	   idcudp = NULL;
    CommonPartPtr *idcptr = NULL;
    CARD32 s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieFillReq );
    REQUEST_SIZE_MATCH( xieFillReq );
    /*	
    **	Verify src Photomap and setup Roi.
    **  Src Photo is either updated in place, or it's a Photoflo.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    if( stuff->dst_idc_id != 0 ) {
	LookupCppRoi_(&idcptr, &idcudp, stuff->dst_idc_id);
	ValueOkIf_( UtilValidRegion( src, idcudp ), stuff->dst_idc_id );
    }

    for( comp = 0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++);
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			     PxlProg_(src), ScnProg_(src), PxlPol_(src),
			     Width_(src), Height_(src),
			     flo == NULL, FALSE, DdxAlignFill_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if (flo != NULL)
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateFill_( Pipe_(flo), Sink_(src), Sink_(dst),
			    idcudp, stuff->constant );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Fill routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxFill_( Udp_(src,comp), Udp_(dst,comp), idcudp,
					  stuff->constant[comp]);
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcFill */

/*-----------------------------------------------------------------------
--------------------- Calculate Histogram Procedure ---------------------
------------------------------------------------------------------------*/
int ProcCalcHistogram( client )
ClientPtr client;		    /* client pointer			    */
{
    PhotomapPtr		 img;
    UdpPtr		 idcudp = NULL;
    CommonPartPtr	*idcptr = NULL;
    xieCalcHistReply	 rep;
    int			 repsiz;
    CARD32 s, i,j, *repptr, *cmpcnt, *ptr, histcnt=0, *hist[XieK_MaxComponents];
    REQUEST(xieCalcHistReq);
    REQUEST_SIZE_MATCH(xieCalcHistReq);
    /*
    **	Lookup source photomap...MUST NOT be a pipe.
    */
    LookupPhotos_(NULL, &img, stuff->photo_id, 0);
    /*
    **	Lookup ROI if specified.
    */
    if( stuff->idc_id != NULL ) {
    	LookupCppRoi_(&idcptr,&idcudp,stuff->idc_id);
	ValueOkIf_( UtilValidRegion( img, idcudp ), stuff->idc_id );
    }
    BePCM_( img, ResId_(img) );
    /*
    **	Process depending on "by_component" flag setting.
    */
    if( stuff->by_component )
	{
    	for( i = 0;  i < CmpCnt_(img);  i++ )
	    {
	    hist[i] = (CARD32 *) DdxMalloc_(uLvl_(img,i) * sizeof(CARD32));
	    /*
	    **  Do histogram calculation.
	    */
	    s = hist[i] ? DdxHistogram_( Udp_(img,i), idcudp, hist[i] )
			: BadAlloc;
	    if( s != Success )
		{
		while( i-- > 0 )
		    DdxFree_(hist[i]);
		OkIf_(FALSE, stuff->photo_id, s);
		}
	    /*
	    **	Count number of non-zero histogram entries.
	    */
	    for( j = 0;  j < uLvl_(img,i);  j++ )
		if( hist[i][j] )
		    histcnt++;
	    }
	/*
	**  Allocate reply buffer for histogram data. For "by component" 
	**  histogram entry size is 2 CARD32s, plus 1 CARD32 for each component
	**  for the entry count field.
	*/
	repsiz = (histcnt * 2 + CmpCnt_(img) ) * sizeof(CARD32);
	repptr = (CARD32 *) DdxMalloc_(repsiz);
	if( !repptr )
	    {
	    for( i = 0;  i < CmpCnt_(img);  i++ )
		DdxFree_(hist[i]);
	    BadAlloc_(stuff->photo_id);
	    }
	/*
	**  Fill in the reply buffer...
	*/
	for( ptr = repptr, i = 0;  i < CmpCnt_(img);  i++ )
	    {
	     cmpcnt = ptr++;		/* Save pointer to entry count field */
	    *cmpcnt = 0;		/* Initialize to zero		    */
	    for( j = 0;  j < uLvl_(img,i);  j++ )
	    	if( hist[i][j] )
		    {
		    *ptr++ = j;
		    *ptr++ = hist[i][j];
		    ++*cmpcnt;
		    }
	    /*
	    **	Free histogram buffer.
	    */
	    DdxFree_(hist[i]);
	    }
	}
    else
	{   /*
	    **	NOT by_component
	    */
	UdpRec	udp;
	/*
	**  For now...bad match if greater than one component. What belongs
	**  here is code that will combine multiple components into a single
	**  plane for histogram purposes. For now, just use the only existing
	**  component plane.
	*/
	MatchOkIf_(!IsRGB_(img), stuff->photo_id);
	udp = *Udp_(img,0);
	/*
	**  Allocate a histogram buffer.
	*/
	hist[0] = (CARD32 *) DdxMalloc_(udp.UdpL_Levels * sizeof(CARD32));
	AllocOkIf_(hist[0], stuff->photo_id);
	/*
	**  Do histogram calculation.
	*/
	s = DdxHistogram_( &udp, idcudp, hist[0] );
	OkIf_(s == Success, (DdxFree_(hist[0]), stuff->photo_id), s);
	/*
	**  Count number of non-zero histogram entries.
	*/
	for( j = 0;  j < udp.UdpL_Levels;  j++ )
	    if( hist[0][j] )
		histcnt++;
	/*
	**  Allocate reply buffer for histogram data. For "NOT by component"
	**  histogram entry size is CmpCnt_ CARD32s, plus 1 CARD32 for the
	**  entry count field.
	*/
	repsiz = (histcnt * CmpCnt_(img) + 1) * sizeof(CARD32);
	repptr = (CARD32 *) DdxMalloc_(repsiz);
	AllocOkIf_(repptr, (DdxFree_(hist[0]), stuff->photo_id));
	/*
	**  Fill in the reply buffer...
	*/
	 ptr   = repptr;
	*ptr++ = histcnt;
	for( j = 0;  j < udp.UdpL_Levels;  j++ )
	     if( hist[0][j] )
		 {
		 for( i = 0;  i < CmpCnt_(img);  i++ )
		     *ptr++ = j;
		 *ptr++ = hist[0][j];
		 }
	/*
	**  Free histogram buffer.
	*/
	DdxFree_(hist[0]);
	}
    /*
    **  Send completed reply back to client.
    */
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = repsiz >> 2;

    WriteXieReplyToClient(client,sizeof(xieCalcHistReply),&rep);
    SetSwappedDataRtn(client, CARD32);
    WriteSwappedXieDataToClient(client,repsiz,repptr);

    DdxFree_(repptr);

    return( Success );
}

/*-----------------------------------------------------------------------
---------------------------  Logical Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcLogical( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, flo2;
    PhotomapPtr src1, src2 = NULL, dst;
    CommonPartPtr idcptr = NULL;
    UdpPtr        idcudp = NULL;
    CARD32 s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieLogicalReq );
    REQUEST_SIZE_MATCH( xieLogicalReq );
    /* 
    **	Locate source Photomap(s) {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify the second operand is bound to it.
    */
    LookupPhotos_(&flo, &src1, stuff->src1_photo_id, stuff->dst_photo_id);
    MatchOkIf_(Constrained_(src1), stuff->src1_photo_id);

    if( stuff->src2_photo_id != 0 )
	{
        LookupPhotos_(&flo2, &src2, stuff->src2_photo_id, -1);
	AccessOkIf_(flo==flo2,stuff->src2_photo_id);
	MatchOkIf_(CmpCnt_(src1) == CmpCnt_(src2), stuff->src2_photo_id);
        MatchOkIf_(Constrained_(src2), stuff->src2_photo_id);
	}
    if( stuff->idc_id != 0 ) {
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src1, idcudp ), stuff->idc_id );
	if (stuff->src2_photo_id != 0)
	    ValueOkIf_( UtilValidRegion( src2, idcudp ), stuff->idc_id );
    }
    /*
    **	Create destination based on first source operand.
    */
    for( comp = 0; comp < CmpCnt_(src1); lvl[comp] = uLvl_(src1,comp), comp++);
    dst = UtilCreatePhotomap( CmpMap_(src1), lvl, PxlRatio_(src1),
			      PxlProg_(src1), ScnProg_(src1), PxlPol_(src1),
			      Width_(src1), Height_(src1),
			      flo == NULL,  FALSE, DdxAlignLogical_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateLogical_( Pipe_(flo), Sink_(src1),
				    src2 ? Sink_(src2) : NULL,
			       stuff->constants, Sink_(dst), idcudp, stuff->op);
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Logical routine (once per component) */
	BePCM_( src1, (UtilFreeResource(dst), ResId_(src1)) );
	BePCM_( src2, (UtilFreeResource(dst), ResId_(src2)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src1) && s == Success; comp++)
	    s = DdxLogical_( Udp_(src1,comp), src2 ? Udp_(src2,comp) : NULL,
			     stuff->constants, Udp_(dst,comp),
			     idcudp, stuff->op );
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcLogical */

/*-----------------------------------------------------------------------
------------------------------  Luminance Procedure ---------------------
------------------------------------------------------------------------*/
int ProcLuminance( client )
 ClientPtr client;	/* client pointer				    */
{
    static CARD32   lvl[] = {256,0,0};
    PhotofloPtr	    flo;
    PhotomapPtr	    src, dst;
    CARD32	    s;
    REQUEST( xieLuminanceReq );
    REQUEST_SIZE_MATCH( xieLuminanceReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    MatchOkIf_(Constrained_(src), stuff->src_photo_id);
    MatchOkIf_(IsRGB_(src), stuff->src_photo_id);
    dst = UtilCreatePhotomap( XieK_GrayScale, lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignLuminance_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateLuminance_(Pipe_(flo), Sink_(src), Sink_(dst));
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Luminance routine */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	s = DdxLuminance_(Udp_(src,0),Udp_(src,1),Udp_(src,2),Udp_(dst,0));
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			/* End of ProcLuminance */

/*-----------------------------------------------------------------------
------------------------Match Histogram Procedure -----------------------
------------------------------------------------------------------------*/
int ProcMatchHistogram( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr src, dst;
    CommonPartPtr idcptr = NULL;
    UdpPtr        idcudp = NULL;
    CARD32 s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieMatchHistogramReq );
    REQUEST_SIZE_MATCH( xieMatchHistogramReq );
    /* 
    **	Locate source Photomap(s) {and Photoflo} and verify destination Id.
    */
    LookupPhotos_(&flo, &src, stuff->photo_id1, stuff->photo_id2);
    MatchOkIf_( Constrained_(src) && !IsRGB_(src), stuff->photo_id1 );

    if( stuff->idc_id != 0 ) {
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src, idcudp ), stuff->idc_id );
    }
    /*
    **	Create destination based on source operand.
    */
    for( comp = 0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++ );
    dst = UtilCreatePhotomap(CmpMap_(src), lvl, PxlRatio_(src),
			     PxlProg_(src), ScnProg_(src), PxlPol_(src),
			     Width_(src), Height_(src),
			     flo == NULL, FALSE, DdxAlignMatchHistogram_ );
    AllocOkIf_(dst, stuff->photo_id2);

    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateMatchHistogram_( Pipe_(flo), Sink_(src), Sink_(dst),
				      idcudp, stuff->h_shape,
				      MiDecodeDouble(&stuff->param1),
				      MiDecodeDouble(&stuff->param2) );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call MatchHistogram routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxMatchHistogram_( Udp_(src,comp), Udp_(dst,comp),
				    idcudp, stuff->h_shape,
				    MiDecodeDouble(&stuff->param1),
				    MiDecodeDouble(&stuff->param2) );
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->photo_id2), s);
	AddResource_(stuff->photo_id2, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcMatchHistogram */

/*-----------------------------------------------------------------------
---------------------------  Math  Procedure ----------------------------
------------------------------------------------------------------------*/
int ProcMath( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr src, dst;
    PhotomapPtr realdst;
    CommonPartPtr idcptr = NULL;
    UdpPtr      idcudp = NULL;
    CARD32 s, i, model, comp, lvl[XieK_MaxComponents];
    REQUEST( xieMathReq );
    REQUEST_SIZE_MATCH( xieMathReq );
    /* 
    **	Locate source Photomap(s) {and Photoflo} and verify destination Id.
    **  If there is a Photoflo, verify the second operand is bound to it.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);

    if( stuff->idc_id != 0 ) {
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src, idcudp ), stuff->idc_id );
    }
    /*
    **	Create destination based on first source operand.
    */
    for( i = 0; i < XieK_MaxComponents; i++)
	lvl[i] = 0;	     /* Make the destination Photo unconstrained */
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignMath_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    /* If the result should be constrained, get another photomap */
    if (stuff->constrain)
	{
	GetConstraintModel(client, &model, lvl);
	realdst = UtilCreatePhotomap(CmpMap_(dst), lvl, PxlRatio_(dst),
				     PxlProg_(dst), ScnProg_(dst), PxlPol_(dst),
				     Width_(dst), Height_(dst),
				     flo == NULL, FALSE, DdxAlignMath_ );
	AllocOkIf_(realdst, (UtilFreeResource(dst), stuff->dst_photo_id));
	}
    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreateMath_( Pipe_(flo), Sink_(src), Sink_(dst),
			    idcudp, stuff->op );
	if( s == Success && stuff->constrain )
	    s = !QueResource(flo, realdst,  TRUE) ? BadAlloc
	      :  DdxCreateConstrain_( Pipe_(flo), Sink_(dst),
				      Sink_(realdst), model);
	OkIf_(s == Success, (UtilFreeResource(dst),
			     UtilFreeResource(realdst), ResId_(flo)), s);
	}
    else
	{   /* Call Math routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst),
		      UtilFreeResource(realdst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    {
	    s = DdxMath_( Udp_(src,comp), Udp_(dst,comp), idcudp, stuff->op );
	    if( s == Success && stuff->constrain )
		s  = DdxConstrain_( Udp_(dst,comp), Udp_(realdst,comp), model );
	    }
	if( stuff->constrain )
	    {
	    UtilFreeResource(dst);
	    dst = realdst;
	    }
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}					/* end ProcMath */

/*-----------------------------------------------------------------------
---------------------------  Mirror Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcMirror( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr src, dst;
    CARD32 s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieMirrorReq );
    REQUEST_SIZE_MATCH( xieMirrorReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    **	Create destination with same geometry as source
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    for( comp=0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++ );
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignMirror_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateMirror_( Pipe_(flo), Sink_(src), Sink_(dst),
			      stuff->x_mirror,  stuff->y_mirror);
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Mirror routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxMirror_( Udp_(src,comp),  Udp_(dst,comp),
			    stuff->x_mirror, stuff->y_mirror );
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcMirror */

/*-----------------------------------------------------------------------
---------------------------  Point Procedure ----------------------------
------------------------------------------------------------------------*/
int ProcPoint( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo, trans_flo;
    PhotomapPtr src, dst, trans;
    CommonPartPtr idcptr = NULL;
    UdpPtr        idcudp = NULL;
    CARD32 s, comp, srccomp, lvl[XieK_MaxComponents];
    REQUEST( xiePointReq );
    REQUEST_SIZE_MATCH( xiePointReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    MatchOkIf_(Constrained_(src), stuff->src_photo_id);
    /*
    ** Locate the transfer function(s) (lookup table) Photomap.
    ** Verify the Photomap is bound to the current Photoflo (if one present).
    ** Verify the Photomap has enough components.
    */
    LookupPhotos_(&trans_flo, &trans, stuff->trans_photo_id, -1);
    MatchOkIf_(Constrained_(trans), stuff->trans_photo_id);
    AccessOkIf_( flo == trans_flo,  stuff->trans_photo_id);
    MatchOkIf_(CmpCnt_(trans) >= CmpCnt_(src), stuff->trans_photo_id);
    /*
     **** Remove the following when the Point element is fixed to handle
     **** the translate table from a Photoflo.
     */
    MatchOkIf_(IsPhotomap_(trans), stuff->trans_photo_id);
    BePCM_( trans, ResId_(trans) );

    if( stuff->idc_id != 0 )
	{
	LookupCppRoi_(&idcptr, &idcudp, stuff->idc_id);
	ValueOkIf_( UtilValidRegion( src, idcudp ), stuff->idc_id );
	}
    /*
    **	Create destination with same geometry as source, but with levels
    **  as specified by the transfer functions.
    */
    for( comp = 0; comp < CmpCnt_(trans); comp++ )
	{
	lvl[comp] = uLvl_(trans,comp);
	MatchOkIf_( !idcptr || lvl[comp] == uLvl_(src,comp), stuff->idc_id );
	}
    dst = UtilCreatePhotomap( CmpMap_(trans), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      Width_(src), Height_(src),
			      flo == NULL, FALSE, DdxAlignPoint_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !((!idcptr || QueResource(flo, idcptr, FALSE))
		       && QueResource(flo, dst,    TRUE)) ? BadAlloc
	  : DdxCreatePoint_( Pipe_(flo), Sink_(src), Sink_(dst),
			     idcudp, &Udp_(trans,0));
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /*
	    **	Call Point routine (once per component in TRANSFER FUNCTION).
	    **  If the source has fewer component, supply one multiple times.
	    */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(trans) && s==Success; comp++)
	    {
	    srccomp = comp > (CmpCnt_(src)-1) ? 0 : comp;
	    s = DdxPoint_( Udp_(src,srccomp), Udp_(dst,comp),
			   idcudp, Udp_(trans,comp));
	    }
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcPoint */

/*-----------------------------------------------------------------------
---------------------------  Point Stats Procedure ----------------------
------------------------------------------------------------------------*/
int ProcPointStats( client )
ClientPtr client;	/* client pointer				    */
{
    PhotomapPtr		    img;
    CARD32		    s, comp, data_bytes;
    xiePointStatsReply	    rep;
    xiePointStatsDataPtr    data;
    double		    pixel, minimum, maximum, mean, std_dev, variance;
    REQUEST(xiePointStatsReq);
    REQUEST_SIZE_MATCH(xiePointStatsReq);

    LookupPhotos_(NULL, &img, stuff->photo_id, 0);
    BePCM_( img, ResId_(img) );
    /*
    **	Allocate space for statistics.
    */
    data_bytes = CmpCnt_(img) * sizeof(xiePointStatsData);
    data = (xiePointStatsDataPtr) DdxMalloc_(data_bytes);
    AllocOkIf_(data, stuff->photo_id);
    /*
    **	Setup reply packet.
    */
    rep.type		  = X_Reply;
    rep.sequenceNumber	  = client->sequence;
    rep.length		  = data_bytes >> 2;
    /*
    **	Do calcuation.
    */
    for( comp = 0;  comp < CmpCnt_(img);  comp++ )
	{
	s = DdxStatsPoint_( Udp_(img,comp), stuff->x, stuff->y,
		        stuff->width, stuff->height, &pixel,
		       &minimum, &maximum, &mean, &std_dev, &variance );
	OkIf_(s == Success, (DdxFree_(data), stuff->photo_id), s);
	MiEncodeDouble(&data[comp].pixel,   pixel);
	MiEncodeDouble(&data[comp].minimum, minimum);
	MiEncodeDouble(&data[comp].maximum, maximum);
	MiEncodeDouble(&data[comp].mean,    mean);
	MiEncodeDouble(&data[comp].std_dev, std_dev);
	MiEncodeDouble(&data[comp].variance,variance);
	}
    /*
    **	Send result to client.
    */
    WriteXieReplyToClient(client, sizeof(xiePointStatsReply), &rep);
    client->pSwapReplyFunc = SwapFloatWrite;
    WriteSwappedXieDataToClient(client, data_bytes, data);
    DdxFree_(data);

    return( Success );
}			    /* End of ProcPointStats */

/*-----------------------------------------------------------------------
---------------------------  Rotate Procedure ---------------------------
------------------------------------------------------------------------*/
int ProcRotate( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	flo;
    PhotomapPtr src, dst;
    CARD32 s, comp, width, height, lvl[XieK_MaxComponents];
    double angle, sina, cosa;
    REQUEST( xieRotateReq );
    REQUEST_SIZE_MATCH( xieRotateReq );
    /* 
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    **	Create destination based on rotate angle and client passed attributes.
    */
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    angle  = MiDecodeDouble( &stuff->angle );
    cosa   = fabs( cos( angle * RADIANS_PER_DEGREE ));
    sina   = fabs( sin( angle * RADIANS_PER_DEGREE ));
    width  = stuff->width  != 0 ? stuff->width
				: cosa * Width_(src) + sina * Height_(src);
    height = stuff->height != 0 ? stuff->height
				: sina * Width_(src) + cosa * Height_(src);
    for( comp = 0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++ );
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      width, height,
			      flo == NULL, FALSE, DdxAlignRotate_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateRotate_( Pipe_(flo), Sink_(src), Sink_(dst),
			      angle, stuff->fill );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Rotate routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxRotate_( Udp_(src,comp), Udp_(dst,comp),
			    angle, stuff->fill[comp]);
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcRotate */


/*-----------------------------------------------------------------------
---------------------------  Scale Procedure ----------------------------
------------------------------------------------------------------------*/
int ProcScale( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr	    flo;
    PhotomapPtr	    src, dst;
    CARD32	    s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieScaleReq );
    REQUEST_SIZE_MATCH( xieScaleReq );

    /* 
    **	Test dimensions.
    **	Locate source Photomap {and Photoflo} and verify destination Id.
    **	Create a Photomap with new dimensions.
    */
    ValueOkIf_(stuff->width != 0 && stuff->height != 0, stuff->src_photo_id);
    LookupPhotos_(&flo, &src, stuff->src_photo_id, stuff->dst_photo_id);
    for( comp = 0; comp < CmpCnt_(src); lvl[comp] = uLvl_(src,comp), comp++);
    dst = UtilCreatePhotomap( CmpMap_(src), lvl, PxlRatio_(src),
			      PxlProg_(src), ScnProg_(src), PxlPol_(src),
			      stuff->width, stuff->height,
			      flo == NULL, FALSE, DdxAlignDefault_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !QueResource( flo, dst, TRUE ) ? BadAlloc
	  : DdxCreateScale_(Pipe_(flo), Sink_(src), Sink_(dst));
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Scale routine (once per component) */
	BePCM_( src, (UtilFreeResource(dst), ResId_(src)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src) && s == Success; comp++)
	    s = DdxScale_( Udp_(src,comp), Udp_(dst,comp) );
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
#ifdef PEZD
	LIB$SHOW_TIMER();
	printf("PEZD : End scale\n");
#endif
	}
    return( Success );
}			    /* End of ProcScale */


/*-----------------------------------------------------------------------
---------------------------  Translate Procedure ------------------------
------------------------------------------------------------------------*/
int ProcTranslate( client )
 ClientPtr client;	/* client pointer				    */
{
    PhotofloPtr flo, flo2;
    PhotomapPtr src1, src2, dst;
    UdpPtr	  sidc_udp = NULL, didc_udp = NULL;
    CommonPartPtr sidc     = NULL, didc	    = NULL;
    CARD32 s, comp, lvl[XieK_MaxComponents];
    REQUEST( xieTranslateReq );
    REQUEST_SIZE_MATCH( xieTranslateReq );

    /* Verify source and destination Photomaps */
    LookupPhotos_(&flo,  &src1, stuff->src1_photo_id, stuff->dst_photo_id);
    LookupPhotos_(&flo2, &src2, stuff->src2_photo_id, -1);
    AccessOkIf_(flo==flo2, stuff->src2_photo_id);
    MatchOkIf_(CmpCnt_(src1) == CmpCnt_(src2), stuff->src2_photo_id);

    /* Verify or create source and destination IDC's */
    if( stuff->src_idc_id != 0 )
	{
	LookupCppRoi_(&sidc, &sidc_udp, stuff->src_idc_id);
	ValueOkIf_(UtilValidRegion( src1, sidc_udp ), stuff->src_idc_id );
	}
    if( stuff->dst_idc_id != 0 )
	{
	LookupCppRoi_(&didc, &didc_udp, stuff->dst_idc_id);
	ValueOkIf_(UtilValidRegion( src2, didc_udp ), stuff->dst_idc_id);
	}
    for( comp = 0; comp < CmpCnt_(src1); comp++ )
	{
	lvl[comp] = uLvl_(src1,comp);
	MatchOkIf_(lvl[comp] == uLvl_(src2,comp), stuff->src2_photo_id);
	}
    dst = UtilCreatePhotomap( CmpMap_(src2), lvl, PxlRatio_(src2),
			      PxlProg_(src2), ScnProg_(src2), PxlPol_(src2),
			      Width_(src2), Height_(src2),
			      flo == NULL, FALSE, DdxAlignTranslate_ );
    AllocOkIf_(dst, stuff->dst_photo_id);

    if( flo != NULL )
	{
	s = !((!sidc || QueResource(flo, sidc, FALSE))
	    &&(!didc || QueResource(flo, didc, FALSE))
		     && QueResource(flo, dst,  TRUE)) ? BadAlloc
	  : DdxCreateTranslate_( Pipe_(flo), Sink_(src1), Sink_(src2),
				 Sink_(dst), sidc_udp, didc_udp );
	OkIf_(s == Success, (UtilFreeResource(dst), ResId_(flo)), s);
	}
    else
	{   /* Call Translate routine (once per component) */
	BePCM_( src1, (UtilFreeResource(dst), ResId_(src1)) );
	BePCM_( src2, (UtilFreeResource(dst), ResId_(src2)) );
	for(s = Success, comp = 0; comp < CmpCnt_(src2) && s == Success; comp++)
	    s = DdxTranslate_( Udp_(src1,comp), Udp_(src2,comp),
			       Udp_(dst,comp), sidc_udp, didc_udp );
	OkIf_(s == Success, (UtilFreeResource(dst), stuff->dst_photo_id), s);
	AddResource_(stuff->dst_photo_id, RT_photo, dst);
	xieDoSendEvent( client, dst, XieK_ComputationEvent );
	}
    return( Success );
}			    /* End of ProcTranslate */

/*-----------------------------------------------------------------------
----------- routine:  initialize a RoiRec to use full Photomap ----------
------------------------------------------------------------------------*/
static RoiPtr RoiEntirePhotomap( img, roi, udp )
 PhotomapPtr    img;	/* Photo{map|tap} pointer			    */
 RoiPtr		roi;	/* pointer to Roi to be filled in		    */
 UdpPtr		udp;	/* pointer to Udp to be used by Roi		    */
{
    /*
    **	Setup the Roi to describe the entire Photomap.
    */
    memset( roi, 0, sizeof(RoiRec) );
    memset( udp, 0, sizeof(UdpRec) );
    ResType_(roi)   = XieK_IdcRoi;
    RoiUdp_(roi)    = udp;
    upDType_(udp)   = UdpK_DTypeVU;
    upClass_(udp)   = UdpK_ClassUBA;
    upWidth_(udp)   = Width_(img);
    upHeight_(udp)  = Height_(img);
    upX2_(udp)	    = upWidth_(udp)  - 1;
    upY2_(udp)	    = upHeight_(udp) - 1;

    return( roi );
}				/* End of RoiEntirePhotomap */
/* End of MODULE XIEPROCESS.C */
