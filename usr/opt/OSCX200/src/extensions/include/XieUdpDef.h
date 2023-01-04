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
/********************************************************************************************************************************/
/* Created 23-MAY-1992 03:11:09 by VAX SDL V3.2-12     Source: 12-JUL-1990 18:14:51 VNRESD$:[XIESMI.SRC]XIEUDPDEF.SDL;1 */
/********************************************************************************************************************************/
/******************************************************************************* */
/**  Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts, */
/**  and the Massachusetts Institute of Technology, Cambridge, Massachusetts. */
/**                                                                         */
/**                          All Rights Reserved                            */
/**                                                                         */
/**  Permission to use, copy, modify, and distribute this software and its  */
/**  documentation for any purpose and without fee is hereby granted,       */
/**  provided that the above copyright notice appear in all copies and that */
/**  both that copyright notice and this permission notice appear in        */
/**  supporting documentation, and that the names of Digital or MIT not be  */
/**  used in advertising or publicity pertaining to distribution of the     */
/**  software without specific, written prior permission.                   */
/**                                                                         */
/**  DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING */
/**  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL */
/**  DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR */
/**  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,    */
/**  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, */
/**  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS    */
/**  SOFTWARE.                                                              */
/**                                                                         */
/******************************************************************************* */
/******************************************************************************* */
/**                                                                         */
/**  FACILITY:                                                              */
/**                                                                         */
/**	Sample Machine Independant DDX                                      */
/**      X Image Extensions                                                 */
/**                                                                         */
/**  ABSTRACT:                                                              */
/**                                                                         */
/**      This include file defines low level image data representation      */
/**	structures.                                                         */
/**                                                                         */
/**  ENVIRONMENT:                                                           */
/**                                                                         */
/**	VAX/VMS V5.3                                                        */
/**	ULTRIX  V3.1                                                        */
/**                                                                         */
/**  AUTHOR(S):                                                             */
/**                                                                         */
/**      John Weber                                                         */
/**                                                                         */
/**  CREATION DATE:                                                         */
/**                                                                         */
/**      August 12, 1989                                                    */
/**                                                                         */
/**  MODIFICATION HISTORY:                                                  */
/**                                                                         */
/******************************************************************************* */
 
/*** MODULE XieUdpDef ***/
#ifndef	UDPDEF
#define UDPDEF
struct UDP {
    unsigned short int UdpW_PixelLength;
    unsigned char UdpB_DType;
    unsigned char UdpB_Class;
    unsigned char *UdpA_Base;
    unsigned long int UdpL_ArSize;
    unsigned long int UdpL_PxlStride;
    unsigned long int UdpL_ScnStride;
    long int UdpL_X1;
    long int UdpL_X2;
    long int UdpL_Y1;
    long int UdpL_Y2;
    unsigned long int UdpL_PxlPerScn;
    unsigned long int UdpL_ScnCnt;
    long int UdpL_Pos;
    unsigned long int UdpL_CompIdx;
    unsigned long int UdpL_Levels;
    } ;
typedef struct UDP UdpRec, *UdpPtr;
#define UdpK_DTypeUndefined 0
#define UdpK_DTypeMin 1
#define UdpK_DTypeBU 1
#define UdpK_DTypeWU 2
#define UdpK_DTypeLU 3
#define UdpK_DTypeF 4
#define UdpK_DTypeC 5
#define UdpK_DTypeVU 6
#define UdpK_DTypeV 7
#define UdpK_DTypeCL 8
#define UdpK_DTypeMax 8
#define UdpK_ClassUBA 1
#define UdpK_ClassUBS 2
#define UdpK_ClassA 3
#define UdpK_ClassCL 4
#endif	/* UDPDEF */
