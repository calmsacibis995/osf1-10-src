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
 * @(#)$RCSfile: libpip.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 92/09/25 18:50:29 $
 */
#ifndef LIBPIP_H
#define LIBPIP_H
/*
** File:
**
**   libpip.h --- libpip.c header file
**
** Author:
** 
**   David Carver (Digital Workstation Engineering/Project Athena)
**
** Revisions:
**
**   29.08.91 Carver
**     - added support for gray scaled video
**     - changed all interfaces to take RopPtr as first argument
**       RopPtr is defined in rop.h
**
**   Carver --- 04.06.91
**     - original
**
*/

#define PIP_NTSC      (1L<<0)
#define PIP_PAL       (1L<<1)
#define PIP_SECAM     (1L<<2)
#define PIP_COMPOSITE (1L<<3)
#define PIP_SVIDEO    (1L<<4)
#define PIP_RGB       (1L<<5)
#define PIP_GRAY      (1L<<6)

#define PIP_SOURCE_TIMING (PIP_NTSC | PIP_PAL | PIP_SECAM)
#define PIP_SOURCE_TYPE (PIP_COMPOSITE | PIP_SVIDEO | PIP_RGB)

#endif /* LIBPIP_H */
