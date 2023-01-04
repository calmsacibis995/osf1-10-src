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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: sia_globals.c,v $ $Revision: 1.1.7.4 $ (DEC) $Date: 1993/08/04 21:20:51 $";
#endif
/*
 * Declare SIA globals.  globals are defined in
 * the sia include files if SIAGLOBAL is defined.
 * Otherwise the include file simple declare the
 * data as extern's
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak sia_caps = __sia_caps
#pragma weak sia_cap_fps = __sia_cap_fps
#pragma weak sia_handle = __sia_handle
#pragma weak sia_initialized = __sia_initialized
#pragma weak sia_mechs = __sia_mechs
#endif
#endif
#ifdef _THREAD_SAFE
#define SIA_THREAD_GLOBAL
#else
#define	 SIAGLOBAL
#endif
#include "siad.h"
#include "siad_bsd.h"
