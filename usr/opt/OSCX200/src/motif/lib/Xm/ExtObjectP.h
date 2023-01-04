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
/*   $RCSfile: ExtObjectP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:37:14 $ */
/*
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
#ifndef  _XmExtObjectP_h
#define _XmExtObjectP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

enum{	XmCACHE_EXTENSION = 1,			XmDESKTOP_EXTENSION,
	XmSHELL_EXTENSION,			XmPROTOCOL_EXTENSION,
	XmDEFAULT_EXTENSION
	} ;

#ifndef XmIsExtObject
#define XmIsExtObject(w) XtIsSubclass(w, xmExtObjectClass)
#endif /* XmIsExtObject */

/* Class record constants */

typedef struct _XmExtRec *XmExtObject;
typedef struct _XmExtClassRec *XmExtObjectClass;

externalref WidgetClass xmExtObjectClass;

#define XmNUM_ELEMENTS 4
#define XmNUM_BYTES 99

/* Class Extension definitions */

typedef struct _XmExtClassPart{
    XmSyntheticResource *syn_resources;   
    int                	num_syn_resources;   
#ifdef notdef
    XtResourceList	ext_resources;
    XtResourceList	compiled_ext_resources;
    Cardinal		num_ext_resources;
    Boolean		use_sub_resources;
#endif /* notdef */
    XtPointer		extension;
}XmExtClassPart, *XmExtClassPartPtr;

typedef struct _XmExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart	 	ext_class;
}XmExtClassRec;

typedef struct {
    Widget		logicalParent;
    unsigned char	extensionType;
} XmExtPart, *XmExtPartPtr;

externalref XmExtClassRec 	xmExtClassRec;

typedef struct _XmExtRec{
    ObjectPart			object;
    XmExtPart			ext;
}XmExtRec;

typedef struct _XmExtCache {
   char    data[XmNUM_BYTES];
   Boolean inuse;
}XmExtCache;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern char * _XmExtObjAlloc() ;
extern void _XmExtObjFree() ;
extern void _XmBuildExtResources() ;

#else

extern char * _XmExtObjAlloc( 
                        int size) ;
extern void _XmExtObjFree( 
                        XtPointer element) ;
extern void _XmBuildExtResources( 
                        WidgetClass c) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif  /* _XmExtObjectP_h */
