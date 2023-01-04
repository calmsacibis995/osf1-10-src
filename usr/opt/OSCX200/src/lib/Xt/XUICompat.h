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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1990 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**	MASSACHUSSETTS INSTITUTE OF TECHNOLOGY, CAMBRIDGE, MASS.	    *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION OR MIT.                                                     *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      Intrinsics to support XUI toolkit
**
**  ABSTRACT:
**
**      This module contains macros which allow the Intrinsics to
**	support XUI V2 semantics
**
**  ENVIRONMENT:
**
**      < to be supplied >
**
**  MODIFICATION HISTORY:
**
**      Jan 90 - Initial Version for DECwindows V3 - Leo Treggiari
**
**--
**/

/*
 *  These macro distinguish between a widget created through an
 *  XUI compatibility interface vs. a "normal" widget.  They do
 *  this by checking to see if the widget name is allocated
 *  immediately after the widget instance record.
 */
#define IsXUIV2Widget(widget) \
    *(char *)((int)(widget) + (widget)->core.widget_class->core_class.widget_size) != 0

#if defined(VMS_THIN_LAYER) && !defined(_Xm_h)
#define dxm_externaldef(psect, kind, name)\
     globaldef {"psect"} noshare kind _/**/name
#else
#if defined(VMS) && !defined(__alpha)
#define dxm_externaldef(psect, kind, name)\
     globaldef {"psect"} noshare kind name
#else
#define dxm_externaldef(psect, kind, name)\
     kind name
#endif
#endif

#ifdef VMS
/*
 *  PRECOMPILED_TM and PRECOMPILED_RL require the Xlib built-in quark
 *  optimization and are mainly useful with shared libraries.
 */  
#ifdef VMS_THIN_LAYER	    /* don't turn this on for the layered product kit */
#define PRECOMPILED_TM
#define PRECOMPILED_RL
#endif
/*
 *  DIRECT_INSTANCE_GET changes some uses of XtGetValues into direct
 *  access into the widget instance record.  With shared libraries
 *  it doesn't matter that this references code modules that the
 *  application may not otherwise use.
 */
#define DIRECT_INSTANCE_GET
#ifdef _Xm_h
#include <Xm/ArrowBP.h>
#include <Xm/SashP.h>
#endif
#define XMCLASS(mw) XtClass(mw) >= xmArrowButtonWidgetClass &&\
		    XtClass(mw) <= xmSashWidgetClass
#endif

#if !defined(XtFree) && defined(DEC_EXTENSION) && !defined(DEC_MOTIF_MEM_DEBUG) && defined(VMS)
/*
 *  Define an XtFree macro for performance reasons.  This must
 *  be the same as the XtFree routine in Alloc.c
 */
#define XtFree(ptr) \
 if (1) { char* _p_; if ((_p_ = (ptr)) != NULL) lib$vm_free((_p_)); } else
#endif

