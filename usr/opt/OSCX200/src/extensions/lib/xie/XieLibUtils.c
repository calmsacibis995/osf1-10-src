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
**	This module contains XieLib utility routines.
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
**	September 21, 1989
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
#define _XieLibUtils

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
#include <XieProto.h>			/* XIE Req/Reply protocol defs	    */
#include <XieLib.h>			/* XIE public  definitions	    */
#include <XieLibint.h>			/* XIE private definitions	    */

/*
**  Table of contents
*/
    /*
    **  PUBLIC utility entry points
    */
unsigned char	 XieIsBitonal();
unsigned char	 XieIsCpp();
unsigned char	 XieIsGrayScale();
unsigned char	 XieIsIdc();
unsigned char	 XieIsPhoto();
unsigned char	 XieIsPhotoflo();
unsigned char	 XieIsPhotomap();
unsigned char	 XieIsPhototap();
unsigned char	 XieIsRGB();
unsigned char	 XieIsRoi();
unsigned char	 XieIsTmp();
unsigned long	 XiePhotoCount();
unsigned long	 XiePlaneCount();
unsigned long	 XieXId();
    /*
    **  PUBLIC memory management entry points
    */
void		*XieCalloc();
unsigned char	*XieCallocBits();
void		*XieCfree();
void		*XieFree();
unsigned char	*XieFreeBits();
void		*XieMalloc();
unsigned char	*XieMallocBits();
void		*XieRealloc();
unsigned char	*XieReallocBits();
    /*
    **  PRIVATE entry points
    */
void		_XieBufToUdp();	    /* copy image data from buffer to Udp   */
void		_XieUdpToBuf();	    /* copy image data from Udp to buffer   */
void		_XieLibError();	    /* client error report routine	    */
void            _XiePacketTrace();  /* trace packets sent from client side  */
void		_XieSrvError();	    /* server error report routine	    */



/*
**  Macro definitions
*/

/*
**  Equated symbols
*/

/*
**  Externals
*/

/*
**  Local storage
*/

/*
**  XieLib macros as callable routines.
*/
unsigned char XieIsBitonal( photo_id )
 XiePhoto photo_id;
{
    return( XieIsPhoto(photo_id) && IsBitonal_(photo_id) );
}

unsigned char XieIsCpp( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsCpp_(res_id) );
}

unsigned char XieIsGrayScale( photo_id )
 XiePhoto photo_id;
{
    return( XieIsPhoto(photo_id) && IsGrayScale_(photo_id) );
}

unsigned char XieIsIdc( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsIdc_(res_id) );
}

unsigned char XieIsPhoto( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsPhoto_(res_id) );
}

unsigned char XieIsPhotoflo( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsPhotoflo_(res_id) );
}

unsigned char XieIsPhotomap( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsPhotomap_(res_id) );
}

unsigned char XieIsPhototap( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsPhototap_(res_id) );
}

unsigned char XieIsRGB( photo_id )
 XiePhoto photo_id;
{
    return( XieIsPhoto(photo_id) && IsRGB_(photo_id) );
}

unsigned char XieIsRoi( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsRoi_(res_id) );
}

unsigned char XieIsTmp( res_id )
 XieResource  res_id;
{
    return( res_id != NULL && IsTmp_(res_id) );
}

unsigned long XiePhotoCount( photo_id )
 XiePhoto photo_id;
{
    return( XieIsPhoto(photo_id) ? PhotoCount_(photo_id) : 0 );
}

unsigned long XiePlaneCount( img )
 XieImage img;
{
    return( img != NULL ? PlaneCount_(img) : 0 );
}

unsigned long XieXId( res_id )
 XieResource  res_id;
{
    return( res_id != NULL ? XId_(res_id) : 0 );
}

/*
**  Memory management entry points.
*/
void *XieCalloc(number,size)
 unsigned number;
 unsigned size;
{
    return (void *)Xcalloc((number),(size));
}

unsigned char *XieCallocBits(bits)
 unsigned bits;
{
    return (unsigned char *)Xcalloc(1,(bits)+39>>3);
}

void  *XieCfree(ptr)
 char *ptr;
{
    if( ptr != NULL )
	Xfree(ptr);
    return( NULL );
}

void  *XieFree(ptr)
 char *ptr;
{
    if( ptr != NULL )
	Xfree(ptr);
    return( NULL );
}

unsigned char *XieFreeBits(ptr)
 unsigned char *ptr;
{
    if( ptr != NULL )
	Xfree(ptr);
    return( NULL );
}

void *XieMalloc(size)
 unsigned size;
{
    return (void *)Xmalloc(size);
}

unsigned char *XieMallocBits(bits)
 unsigned bits;
{
    return (unsigned char *)Xmalloc((bits)+39>>3);
}

void *XieRealloc(ptr,size)
 char *ptr;
 unsigned size;
{
    return (void *)Xrealloc(ptr,size);
}

unsigned char *XieReallocBits(ptr,bits)
 unsigned char *ptr;
 unsigned bits;
{
    return (unsigned char *)Xrealloc(ptr,(bits)+39>>3);
}

/*******************************************************************************
**  _XieBufToUdp
**
**  FUNCTIONAL DESCRIPTION:
**
**	Copy image data from buffer to udp.  The image data in the buffer is
**	assumed to have the same format as described by the udp.
**
**  FORMAL PARAMETERS:
**
**	buf	- src buffer pointer
**	dst	- dst udp pointer
**	x	- X coordinate within udp to begin data storage
**	y	- Y coordinate within udp to begin data storage
**	width	- width  of data to copy (in pixels)
**	height	- height of data to copy (in pixels)
**
*******************************************************************************/
void _XieBufToUdp( buf, dst, x, y, width, height )
 unsigned char *buf;
 UdpPtr		dst;
 long		x, y;
 unsigned long  width, height;
{
    UdpRec src;

    src = *dst;
    src.UdpL_ScnStride	= width  * src.UdpL_PxlStride;
    src.UdpL_X1		= 0;
    src.UdpL_X2		= width  - 1;
    src.UdpL_Y1		= 0;
    src.UdpL_Y2		= height - 1;
    src.UdpL_PxlPerScn	= width;
    src.UdpL_ScnCnt	= height;
    src.UdpL_Pos	= 0;
    src.UdpA_Base	= buf;
    src.UdpL_ArSize	= height * src.UdpL_ScnStride;

    SmiCopy( &src, dst, 0, 0, x, y, width, height );
}				    /* end _XieBufToUdp */

/*******************************************************************************
**  _XieUdpToBuf
**
**  FUNCTIONAL DESCRIPTION:
**
**	Copy image data from udp udp to buffer.  The image data in the buffer 
**	will have the same format as described by the udp.
**
**  FORMAL PARAMETERS:
**
**	src	- src udp pointer
**	buf	- dst buffer pointer
**	x	- X coordinate within udp to begin data retrieval
**	y	- Y coordinate within udp to begin data retrieval
**	width	- width  of data to copy (in pixels)
**	height	- height of data to copy (in pixels)
**
*******************************************************************************/
void _XieUdpToBuf( src, buf, x, y, width, height )
 UdpPtr		src;
 unsigned char *buf;
 long		x, y;
 unsigned long  width, height;
{
    UdpRec dst;

    dst = *src;
    dst.UdpL_ScnStride	= width  * dst.UdpL_PxlStride;
    dst.UdpL_X1		= 0;
    dst.UdpL_X2		= width  - 1;
    dst.UdpL_Y1		= 0;
    dst.UdpL_Y2		= height - 1;
    dst.UdpL_PxlPerScn	= width;
    dst.UdpL_ScnCnt	= height;
    dst.UdpL_Pos	= 0;
    dst.UdpA_Base	= buf;
    dst.UdpL_ArSize	= height * dst.UdpL_ScnStride;

    SmiCopy( src, &dst, x, y, 0, 0, width, height );
}				    /* end _XieUdpToBuf */

/*******************************************************************************
**  _XieLibError
**
**  FUNCTIONAL DESCRIPTION:
**
**	XieLib client error routine (exit on application programmer goofs).
**
**  FORMAL PARAMETERS:
**
**	error	- error code: selected from <errno.h>
**	text	- pointer to informational text.
**
*******************************************************************************/
void _XieLibError( error, text )
 unsigned long	 error;
 char		*text;
{
    errno = error;
    perror( text );
    exit(1);
}				    /* end _XieLibError */

/*******************************************************************************
**  _XiePacketTrace
**
**  FUNCTIONAL DESCRIPTION:
**
**	XieLib protocol packet trace
**
**  FORMAL PARAMETERS:
**
**	opcode - Xie protocol opcode
**
**
*******************************************************************************/
void _XiePacketTrace( opcode )
 int	opcode;
{
    externalref char *_XieFunctionName[];	/* The function names  */

    if (opcode > 0 && opcode < X_ieLastRequest)
	printf("Packet Trace : %s\n", _XieFunctionName[opcode] );
    else
	printf("Packet Trace : Bad Request %d\n", opcode);
}    

/*******************************************************************************
**  _XieSrvError
**
**  FUNCTIONAL DESCRIPTION:
**
**	XieLib server error reporting routine.
**
**  FORMAL PARAMETERS:
**
**	dpy	- X11 display
**	opcode	- minor op-code
**	id	- resource id
**	seqnum	- reply sequence number
**	error	- error code
**	text	- pointer to informational text.
**
*******************************************************************************/
void _XieSrvError( dpy, opcode, id, seqnum, error, text )
 Display	*dpy;
 unsigned long	 opcode, id, seqnum, error;
 char		*text;
{
    xError err;

    err.type		= X_Error;
    err.errorCode	= error;
    err.sequenceNumber	= seqnum;
    err.resourceID	= id;
    err.minorCode	= opcode;
    err.majorCode	= SesOpCode_(_XieGetSession(dpy));

    if( text != NULL && *text != '\0' )
        fprintf(stderr,"\n%s> %s.\n", XieS_Name, text );

    UnlockDisplay(dpy);

#ifdef VMS
    SyncHandle();
    exit(1);			/* _XError() isn't in the transfer vector ! */
#else
    /*
    **	Now tell the rest of the story.
    */
    _XError( dpy, &err );
    SyncHandle();
#endif
}				    /* end _XieSrvError */

/* end module XieLibUtils.c */
