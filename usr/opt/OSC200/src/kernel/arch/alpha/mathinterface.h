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
 * @(#)$RCSfile: mathinterface.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/14 17:43:39 $
 */

/*****************************************************************************/
/*FILE: MATHINTERFACE.H - ISP Math routines interface header file            */
/*****************************************************************************/
 
/*****************************************************************************
! REVISION HISTORY:
! Who   When            What
!---------------------------------------------------------------
! HA    24-Sep-1991     First Created
 *****************************************************************************/
#ifndef _MATHINTERFACE_H_
#define _MATHINTERFACE_H_

#define OSF_KERNEL

/* Return codes */
#define MATH_NOTRAP  1
#define SWC	    (1<<1)
#define INV	    (1<<2)
#define DZE	    (1<<3)
#define FOV	    (1<<4)
#define UNF	    (1<<5)
#define INE	    (1<<6)
#define IOV	    (1<<7)

#endif
