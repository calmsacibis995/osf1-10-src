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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: MrmDecls.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 13:45:12 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef MrmDecls_H
#define MrmDecls_H
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/*----------------------------------*/
/* URM external routines (Motif)    */
/*----------------------------------*/
#ifndef _ARGUMENTS
#ifdef _NO_PROTO
#define _ARGUMENTS(arglist) ()
#else
#define _ARGUMENTS(arglist) arglist
#endif
#endif

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* mrminit.c */
extern void MrmInitialize  _ARGUMENTS(( void ));

/* mrmlread.c */
extern Cardinal MrmFetchLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Display *display , XtPointer *value_return , MrmCode *type_return ));
extern Cardinal MrmFetchIconLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Screen *screen , Display *display , Pixel fgpix , Pixel bgpix , Pixmap *pixmap_return ));
extern Cardinal MrmFetchBitmapLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Screen *screen , Display *display , Pixmap *pixmap_return , Dimension *width , Dimension *height));
extern Cardinal MrmFetchColorLiteral  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Display *display , Colormap cmap , Pixel *pixel_return ));

/* Deal with Wide stuff now because there is an error in Saber 3.0 */

#if NeedWidePrototypes

extern Cardinal MrmOpenHierarchy  _ARGUMENTS(( int num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmOpenHierarchyPerDisplay  _ARGUMENTS(( Display *display , int num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmRegisterNames  _ARGUMENTS(( MrmRegisterArglist reglist ,int num_reg ));
extern Cardinal MrmRegisterNamesInHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id , MrmRegisterArglist reglist , int num_reg ));
extern Cardinal MrmRegisterClass  _ARGUMENTS(( int class_code , String class_name , String create_name , Widget (*creator )(), WidgetClass class_record ));

#else

extern Cardinal MrmOpenHierarchy  _ARGUMENTS(( MrmCount num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmOpenHierarchyPerDisplay  _ARGUMENTS(( Display *display , MrmCount num_files , String *name_list , MrmOsOpenParamPtr *os_ext_list , MrmHierarchy *hierarchy_id_return ));
extern Cardinal MrmRegisterNames  _ARGUMENTS(( MrmRegisterArglist reglist ,MrmCount num_reg ));
extern Cardinal MrmRegisterNamesInHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id , MrmRegisterArglist reglist , MrmCount num_reg ));
extern Cardinal MrmRegisterClass  _ARGUMENTS(( MrmType class_code , String class_name , String create_name , Widget (*creator )(), WidgetClass class_record ));

#endif 

extern Cardinal MrmCloseHierarchy  _ARGUMENTS(( MrmHierarchy hierarchy_id ));
extern Cardinal MrmFetchInterfaceModule  _ARGUMENTS(( MrmHierarchy hierarchy_id , char *module_name , Widget parent , Widget *w_return ));
extern Cardinal MrmFetchWidget  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Widget parent , Widget *w_return , MrmType *class_return ));
extern Cardinal MrmFetchWidgetOverride  _ARGUMENTS(( MrmHierarchy hierarchy_id , String index , Widget parent , String ov_name , ArgList ov_args , Cardinal ov_num_args , Widget *w_return , MrmType *class_return ));
extern Cardinal MrmFetchSetValues  _ARGUMENTS(( MrmHierarchy hierarchy_id , Widget w , ArgList args , Cardinal num_args ));

/* mrmwci.c */

/* extern Cardinal XmRegisterMrmCallbacks () ; */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#undef _ARGUMENTS

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* MrmDecls_H */
/* DON'T ADD STUFF AFTER THIS #endif */
