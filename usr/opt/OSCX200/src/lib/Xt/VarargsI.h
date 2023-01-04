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
/* $XConsortium: VarargsI.h,v 1.14 91/04/10 20:28:15 rws Exp $ */
/*

Copyright 1985, 1986, 1987, 1988, 1989 by the
Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
M.I.T. makes no representations about the suitability of
this software for any purpose.  It is provided "as is"
without express or implied warranty.

*/

#ifndef _VarargsI_h_ 
#define _VarargsI_h_ 
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#if NeedVarargsPrototypes
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#else
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#endif

typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg;
 
/* private routines */

extern void _XtCountVaList(
#if NeedFunctionPrototypes
    va_list /*var*/, int* /*total_count*/, int* /*typed_count*/
#endif
);

extern void _XtVaToArgList(
#if NeedFunctionPrototypes
   Widget /*widget*/, va_list /*var*/, int /*max_count*/, ArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern void _XtVaToTypedArgList(
#if NeedFunctionPrototypes
    va_list /*var*/, int /*count*/, XtTypedArgList* /*args_return*/, Cardinal* /*num_args_return*/
#endif
);

extern XtTypedArgList _XtVaCreateTypedArgList(
#if NeedFunctionPrototypes
    va_list /*var*/, int /*count*/
#endif
);

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _VarargsI_h_ */
