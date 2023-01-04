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
**  Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts,
**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts.
**
**                        All Rights Reserved
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
**      XIE - X11 Image Extension
**  
**
**  ABSTRACT:
**
**      This module contains private definitions required by XieLib.
**
**  ENVIRONMENT:
**
**      VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**      Robert NC Shelley
**
**  CREATION DATE:
**
**      February 9, 1990
**
*******************************************************************************/

    /*
    **  Symbol XIELIBINT allows XieLibint.h to be included multiple times.
    */
#ifndef XIELIBINT
#define XIELIBINT       /* the "endif" MUST be the last line of this file   */

    /*
    **	XieLib session structure.
    */
typedef struct _XieSessionRec {
    struct _XieResRec		   *flink;  /* Xie resource list	    */
    XExtCodes			   *codes;  /* X11 extension codes pointer  */
    CARD8 FunVec[sizeof(XieFunctionsRec)];  /* functions supported by server*/
} XieSessionRec, *XieSessionPtr;
    /*
    **  Access macros for XieLib session info.
    */
#define	SesResLst_(ses)	((ses)->flink)
#define	SesCodes_(ses)	((ses)->codes)
#define	SesExtNum_(ses)	((ses)->codes->extension)
#define	SesOpCode_(ses)	((ses)->codes->major_opcode)
#define	SesError_(ses)	((ses)->codes->first_error)
#define	SesEvent_(ses)	((ses)->codes->first_event)
#define	SesFunVec_(ses)	((ses)->FunVec)
    /*
    **  XieImageRec extension for Stream transport state info
    */
typedef struct _XieStreamRec {
    XID		     id;		    /* X11 resource-id		    */
    Display	    *dpy;		    /* X11 display pointer	    */
    unsigned long    bytes[XieK_MaxPlanes]; /* bytes transported	    */
} XieStreamRec, *XieStreamPtr;
#define Xport_(img)         ((img)->Xport)
#define xpId_(xp)           (xp->id)
#define xpDpy_(xp)          (xp->dpy)
#define xpBytes_(xp,ind)    (xp->bytes[(ind)])
#define xpBits_(xp,ind)	    (xp->bytes[(ind)]<<3)
    /*
    **	XieLib resource structure.
    */
typedef struct _XieResRec {
    struct _XieResRec	*flink;	    /* X11 display Xie resource list flink  */
    Display		*dpy;	    /* X11 display pointer		    */
    XID			 id;	    /* X11 resource-id			    */
    unsigned char	 type;	    /* offset must match RES_TYPE	    */
    unsigned char	 mapping;   /* offset must match RES_MAPPING	    */
    unsigned short	_reserved;
    struct _XieResRec	*floLnk;    /* linkage to Photoflo		    */
    struct _XieResRec	*photoLnk;  /* linkage to Photo{map|tap} {list}	    */
} XieResRec, *XieResPtr,	    /* generic resource			    */
	     *PhotoPtr,		    /* Photoflo, Photomap, or Phototap	    */
	     *IdcPtr;		    /* Cpp, Roi, or Tmp			    */
    /*
    **  Access macros for XieLib resources.
    */
#define	ResLnk_(res)	((res)->flink)
#define	ResDpy_(res)	((res)->dpy)
#define	ResId_(res)	((res)->id)
#define	ResType_(res)	((res)->type)
#define	ResMap_(res)	((res)->mapping)
#define	FloLnk_(res) 	((res)->floLnk)
#define	PhotoLnk_(res)	((res)->photoLnk)

/*
**  Macros
*/
    /*
    **	XieLib interface to Xlibint.h macros.
    **
    **	WARNING: some Xlibint.h macros assume variable "Display *dpy" exists.
    */
	/*
	**  The argument "shortened_proto_name" that is passed to Xlibint
	**  macros, GetReq() and GetReqExtra(), is a partial name for the 
	**  Xie protocol function to be built.  Using the Crop function as
	**  an example, these macros would expect the name "ieCrop" from
	**  which they create the following names:
	**
	**	XieCropReq	- typedef name for protocol structure
	**	X_ieCrop	- symbol  name for protocol minor op-code
	**	sz_xieCropReq	- symbol  name for protocol structure size
	**
	**  Unfortunately there are no special macros for extensions, so we
	**  have to move our minor_opcode to the correct field and write our
	**  own major_opcode after the macros have performed their magic.
	*/
#define	XieReq_(dpy,gc,major_opcode,shortened_proto_name,proto_ptr) \
    LockDisplay(dpy); \
    if(gc) FlushGC(dpy,(GC)gc); \
    GetReq(shortened_proto_name,proto_ptr); \
    if(xiePacketTrace) _XiePacketTrace(proto_ptr->reqType); \
    proto_ptr->opcode  = proto_ptr->reqType; \
    proto_ptr->reqType = major_opcode

#define	XieReqExtra_(dpy,gc,major_opcode,shortened_proto_name,proto_ptr,extra) \
    LockDisplay(dpy); \
    if(gc) FlushGC(dpy,(GC)gc); \
    GetReqExtra(shortened_proto_name,extra,proto_ptr); \
    proto_ptr->opcode  = proto_ptr->reqType; \
    proto_ptr->reqType = major_opcode

#define	XieReqDone_(dpy) \
    {UnlockDisplay(dpy); SyncHandle();}

#ifndef externalref
#if (defined(VMS) && defined(VAXC))
#define externalref globalref
#else
#define externalref extern
#endif
#endif

#ifndef externaldef
#if (defined (VMS) && defined(VAXC))
#define externaldef(psect) globaldef {"psect"} noshare
#else
#define externaldef(psect)
#endif
#endif

/*
**  XieLib private entry points.
*/
#ifndef _XieLibEvents
extern Bool	     _XieConvertEventProc();
#endif
#ifndef _XieLibResource
extern void	     _XieAddResource();
extern Status	     _XieQueryResource();
extern XieResPtr     _XieGetDst();
#endif
#ifndef _XieLibSession
externalref BOOL xiePacketTrace;
extern XieSessionPtr _XieGetSession();
#endif
#ifndef _XieLibTransport
extern void	     _XieSetTransport();
#endif
#ifndef _XieLibUtils
extern void          _XieBufToUdp();
extern void          _XieUdpToBuf();
extern void          _XieLibError();
extern void          _XiePacketTrace();
extern void          _XieSrvError();
#endif

/*
**  This "endif" MUST be the last line of this file.
*/
#endif  /* end of XIELIBINT -- NO DEFINITIONS ALLOWED BEYOND THIS POINT	    */
