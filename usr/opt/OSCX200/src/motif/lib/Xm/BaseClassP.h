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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: BaseClassP.h,v $ $Revision: 1.1.4.4 $ $Date: 1993/07/15 16:05:16 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
#ifndef _XmBaseClassP_h
#define _XmBaseClassP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifndef _XmNO_BC_INCL
#define _XmNO_BC_INCL
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif


#define _XmBCEPTR(wc)	((XmBaseClassExt *)(&(((WidgetClass)(wc))\
					      ->core_class.extension)))
#define _XmBCE(wc)	((XmBaseClassExt)(((WidgetClass)(wc))\
					  ->core_class.extension))

#define _XmGetBaseClassExtPtr(wc, owner) \
    ((_XmBCE(wc) && (((_XmBCE(wc))->record_type) == owner)) \
     ?  \
     _XmBCEPTR(wc)  \
     :  \
     ((XmBaseClassExt *) _XmGetClassExtensionPtr( \
						 ((XmGenericClassExt *)  \
						  _XmBCEPTR( wc)),  \
						 owner)))

/* defines for 256 bit (at least) bit field
 */
#define _XmGetFlagsBit(field, bit) \
	(field[ (bit >> 3) ]) & (1 << (bit & 0x07))

#define _XmSetFlagsBit(field, bit) \
	    (field[ (bit >> 3) ] |= (1 << (bit & 0x07)))


#define _XmFastSubclassInit(wc, bit_field) { \
	if((_Xm_fastPtr = _XmGetBaseClassExtPtr( wc, XmQmotif)) && \
	   (*_Xm_fastPtr)) \
		_XmSetFlagsBit((*_Xm_fastPtr)->flags, bit_field) ; \
   }

/* _XmGetBaseClassExtPtr can return NULL or a pointer to a NULL extension,
   for non Motif classes in particular, so we check that up front.
   We use the global _Xm_fastPtr for that purpose, this variable exists
   already in BaseClass.c for apparently no other use */

#define _XmIsFastSubclass(wc, bit) \
	((_Xm_fastPtr = _XmGetBaseClassExtPtr((wc),XmQmotif)) && \
         (*_Xm_fastPtr)) ? \
	     (_XmGetFlagsBit(((*_Xm_fastPtr)->flags), bit) ? TRUE : FALSE) \
		 : FALSE

#define XmBaseClassExtVersion 2L
#define XmBaseClassExtVersion 2L


#ifdef _NO_PROTO
typedef Cardinal (*XmGetSecResDataFunc) ();
#else
typedef Cardinal (*XmGetSecResDataFunc)( WidgetClass,
					    XmSecondaryResourceData **);
#endif

typedef struct _XmObjectClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
}XmObjectClassExtRec, *XmObjectClassExt;

typedef struct _XmGenericClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
}XmGenericClassExtRec, *XmGenericClassExt;

typedef struct _XmWrapperDataRec{
    struct _XmWrapperDataRec *next;
    WidgetClass		widgetClass;
    XtInitProc		initializeLeaf;
    XtSetValuesFunc	setValuesLeaf;
    XtArgsProc		getValuesLeaf;
    XtRealizeProc	realize;
    XtWidgetClassProc	classPartInitLeaf;
    XtWidgetProc	resize;
    XtGeometryHandler   geometry_manager;
}XmWrapperDataRec, *XmWrapperData;

typedef struct _XmBaseClassExtRec{
    XtPointer 		next_extension;	
    XrmQuark 		record_type;	
    long 		version;	
    Cardinal 		record_size;	
    XtInitProc		initializePrehook;
    XtSetValuesFunc 	setValuesPrehook;
    XtInitProc		initializePosthook;
    XtSetValuesFunc 	setValuesPosthook;
    WidgetClass		secondaryObjectClass;
    XtInitProc		secondaryObjectCreate;
    XmGetSecResDataFunc	getSecResData;
    unsigned char	flags[32];
    XtArgsProc		getValuesPrehook;
    XtArgsProc		getValuesPosthook;
    XtWidgetClassProc	classPartInitPrehook;
    XtWidgetClassProc	classPartInitPosthook;
    XtResourceList	ext_resources;
    XtResourceList	compiled_ext_resources;
    Cardinal		num_ext_resources;
    Boolean		use_sub_resources;
    XmWidgetNavigableProc widgetNavigable;
    XmFocusChangeProc	focusChange;
    XmWrapperData	wrapperData;
}XmBaseClassExtRec, *XmBaseClassExt;


typedef struct _XmWidgetExtDataRec{
    Widget		widget;
    Widget		reqWidget;
    Widget		oldWidget;
}XmWidgetExtDataRec, *XmWidgetExtData;

externalref XrmQuark		XmQmotif;
externalref int 	_XmInheritClass;
externalref XmBaseClassExt * _Xm_fastPtr;
  

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern Boolean _XmIsSlowSubclass() ;
extern XmGenericClassExt * _XmGetClassExtensionPtr() ;
extern void _XmPushWidgetExtData() ;
extern void _XmPopWidgetExtData() ;
extern XmWidgetExtData _XmGetWidgetExtData() ;
extern void _XmFreeWidgetExtData() ;
extern void _XmBaseClassPartInitialize() ;
extern void _XmInitializeExtensions() ;
extern Boolean _XmIsStandardMotifWidgetClass() ;
extern Cardinal _XmSecondaryResourceData() ;
extern void _XmTransformSubResources() ;

#else

extern Boolean _XmIsSlowSubclass( 
                        WidgetClass wc,
                        unsigned int bit) ;
extern XmGenericClassExt * _XmGetClassExtensionPtr( 
                        XmGenericClassExt *listHeadPtr,
                        XrmQuark owner) ;
extern void _XmPushWidgetExtData( 
                        Widget widget,
                        XmWidgetExtData data,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern void _XmPopWidgetExtData( 
                        Widget widget,
                        XmWidgetExtData *dataRtn,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern XmWidgetExtData _XmGetWidgetExtData( 
                        Widget widget,
#if NeedWidePrototypes
                        unsigned int extType) ;
#else
                        unsigned char extType) ;
#endif /* NeedWidePrototypes */
extern void _XmFreeWidgetExtData( 
                        Widget widget) ;
extern void _XmBaseClassPartInitialize( 
                        WidgetClass wc) ;
extern void _XmInitializeExtensions( void ) ;
extern Boolean _XmIsStandardMotifWidgetClass( 
                        WidgetClass wc) ;
extern Cardinal _XmSecondaryResourceData( 
                        XmBaseClassExt bcePtr,
                        XmSecondaryResourceData **secResDataRtn,
                        XtPointer client_data,
                        String name,
                        String class_name,
                        XmResourceBaseProc basefunctionpointer) ;
extern void _XmTransformSubResources( 
                        XtResourceList comp_resources,
                        Cardinal num_comp_resources,
                        XtResourceList *resources,
                        Cardinal *num_resources) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmBaseClassP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
