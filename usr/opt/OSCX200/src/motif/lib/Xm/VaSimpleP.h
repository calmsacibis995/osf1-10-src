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
 * @(#)$RCSfile: VaSimpleP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:56:23 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: VaSimpleP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:56:23 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmVaSimpleP_h
#define _XmVaSimpleP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>

#ifdef _NO_PROTO
# include <varargs.h>
# define Va_start(a,b) va_start(a)
#else
# include <stdarg.h>
# define Va_start(a,b) va_start(a,b)
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XtTypedArg {
    String      name;
    String      type;
    XtArgVal    value;
    int         size;
} XtTypedArg;
 
typedef struct _XtTypedArg* XtTypedArgList;

#define StringToName(string) XrmStringToName(string)


/********  Private Function Declarations  ********/
#ifdef _NO_PROTO
extern void _XmCountVaList() ;
extern void _XmVaToTypedArgList() ;
#else
extern void _XmCountVaList( 
                        va_list var,
                        int *button_count,
                        int *args_count,
                        int *typed_count,
                        int *total_count) ;
extern void _XmVaToTypedArgList( 
                        va_list var,
                        int max_count,
                        XtTypedArgList *args_return,
                        Cardinal *num_args_return) ;
#endif /* _NO_PROTO */
/********  End Private Function Declarations  ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmVaSimpleP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
