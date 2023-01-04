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
 * @(#)$RCSfile: logo.h,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/08/02 23:53:40 $
 */
/*****************************************************************************
 * logo.h - header for the logo display module 				     *
 *****************************************************************************/

#ifndef _logo_h
#define _logo_h

/*****************************************************************************
 * Includes								     *
 *****************************************************************************/

#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/CoreP.h>

/*****************************************************************************
 * Prototypes								     *
 *****************************************************************************/

void ShowLogo( Widget topWidget );

#endif /*_logo_h*/
