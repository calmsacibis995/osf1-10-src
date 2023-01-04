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
/*
 * FILE:
 *     drawbuf.h
 *
 *  Author:
 *     MPW 2/4/93 (moved defs out of LGI)
 *
 */

#ifndef _DRAWBUF_H_
#define _DRAWBUF_H_ 

/* XXX: Change LGI to use these defs and remove them from N_Proto.h */


/*
 * Buffer Types
 */
#define D_BufferOff                     0
#define D_BufferVM                      1
#define D_BufferFB                      2
#define D_BufferHX                      3
#define D_BufferPX                      4
#define D_BufferPV                      5

/*
 * buffer mask bits
 */
#define D_FRONT_LEFT_BUFFER_MASK_BIT        0
#define D_FRONT_RIGHT_BUFFER_MASK_BIT       1
#define D_BACK_LEFT_BUFFER_MASK_BIT         2
#define D_BACK_RIGHT_BUFFER_MASK_BIT        3
#define D_AUX0_BUFFER_MASK_BIT              4
#define D_AUX1_BUFFER_MASK_BIT              5
#define D_AUX2_BUFFER_MASK_BIT              6
#define D_AUX3_BUFFER_MASK_BIT              7
#define D_DEPTH_0_BUFFER_MASK_BIT           8
#define D_STENCIL_BUFFER_MASK_BIT           9
#define D_ACCUM_BUFFER_MASK_BIT             10
#define D_DEPTH_1_BUFFER_MASK_BIT           11
#define D_TRANS_BUFFER_MASK_BIT             12
#define D_FRONT_LEFT_ALPHA_BUFFER_MASK_BIT  16
#define D_FRONT_RIGHT_ALPHA_BUFFER_MASK_BIT 17
#define D_BACK_LEFT_ALPHA_BUFFER_MASK_BIT   18
#define D_BACK_RIGHT_ALPHA_BUFFER_MASK_BIT  19
#define D_AUX0_ALPHA_BUFFER_MASK_BIT        20
#define D_AUX1_ALPHA_BUFFER_MASK_BIT        21
#define D_AUX2_ALPHA_BUFFER_MASK_BIT        22
#define D_AUX3_ALPHA_BUFFER_MASK_BIT        23

/*
 * buffer mask
 */
#define D_NONE_BUFFER_MASK              0x0000

#define D_FRONT_BUFFER_SHIFT            0x0      /* Provided for gl */
#define D_FRONT_LEFT_BUFFER_MASK        0x0001
#define D_FRONT_RIGHT_BUFFER_MASK       0x0002
#define D_BACK_LEFT_BUFFER_MASK         0x0004
#define D_BACK_RIGHT_BUFFER_MASK        0x0008
#define D_FRONT_BUFFER_MASK             0x0003
#define D_BACK_BUFFER_MASK              0x000c
#define D_LEFT_BUFFER_MASK              0x0005
#define D_RIGHT_BUFFER_MASK             0x000a
#define D_AUX0_BUFFER_MASK              0x0010
#define D_AUX1_BUFFER_MASK              0x0020
#define D_AUX2_BUFFER_MASK              0x0040
#define D_AUX3_BUFFER_MASK              0x0080
#define D_COLOR_BUFFER_MASK             0x00ff

#define D_ALPHA_BUFFER_SHIFT                 0x10 /* Provided for gl */
#define D_FRONT_LEFT_ALPHA_BUFFER_MASK       0x010000
#define D_FRONT_RIGHT_ALPHA_BUFFER_MASK      0x020000
#define D_BACK_LEFT_ALPHA_BUFFER_MASK        0x040000
#define D_BACK_RIGHT_ALPHA_BUFFER_MASK       0x080000
#define D_FRONT_ALPHA_BUFFER_MASK            0x030000
#define D_BACK_ALPHA_BUFFER_MASK             0x0c0000
#define D_LEFT_ALPHA_BUFFER_MASK             0x050000
#define D_RIGHT_ALPHA_BUFFER_MASK            0x0a0000
#define D_AUX0_ALPHA_BUFFER_MASK             0x100000
#define D_AUX1_ALPHA_BUFFER_MASK             0x200000
#define D_AUX2_ALPHA_BUFFER_MASK             0x400000
#define D_AUX3_ALPHA_BUFFER_MASK             0x800000
#define D_ALPHA_BUFFER_MASK                  0xff0000

#define D_DEPTH_0_BUFFER_MASK           0x0100
#define D_DEPTH_1_BUFFER_MASK           0x0800
#define D_DEPTH_BUFFER_MASK             0x0900

#define D_STENCIL_BUFFER_MASK           0x0200
#define D_ACCUM_BUFFER_MASK             0x0400
#define D_TRANS_BUFFER_MASK             0x1000

/*
 * NOTE: This must match the N_BufferAttr definition in N_Protostr.h. The
 * reason for having this definition in two places is to avoid the
 * special constructs that would be required in order for D_BufferAttr 
 * to be used on the host side as well as on a graphics adaptor processor 
 * (such as the N10 ). It is important to divorce drawlib mi code from LGI
 * includes so it can support MBUF on non-3D devices. However, LGI/PXG 
 * cannot use these definitions since pointers must be defined as
 * N_VAddrXX in order to work on both the host-side and the N10.
 */

#if 1
/*
** SREE 3/28/93
** THIS IS BROKEN IF YOU ARE AN EVMS/PXG/AXG SYSTEM 
** (SO, WE MUST HAVE SOME CONDITIONALS HERE)
*/

#if defined(VMS) && defined(PXG)
#if !defined(uint64)
#include <ints.h>
#endif
#define	HostVirtAddrB uint64
#define	HostVirtAddrI uint64
#else
#define	HostVirtAddrB unsigned char *
#define	HostVirtAddrI int *
#endif	/* VMS && PXG */

typedef struct 
{
    HostVirtAddrB    addr;
    HostVirtAddrI    devPriv;
    int		     mode;
    int		     type;
    int              depth;
    int              x;
    int              y;
    int              width;
    int              height;
    int              pixelStride;    /* bytes from pix to pix */
    int              scanStride;     /* bytes from scanline to scanline */
    int              pad;/* osf compiler does this anyway; make explicit */
} D_BufferAttr, *D_BufferAttrPtr;

#else

typedef struct 
{
    char	     *addr;
    int		     *devPriv;
    int		     mode;
    int		     type;
    int              depth;
    int              x;
    int              y;
    int              width;
    int              height;
    int              pixelStride;    /* bytes from pix to pix */
    int              scanStride;     /* bytes from scanline to scanline */
    int              pad;/* osf compiler does this anyway; make explicit */
} D_BufferAttr, *D_BufferAttrPtr;
#endif

#endif /* DRAWBUF_H */
