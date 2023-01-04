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
 * @(#)$RCSfile: RepType.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:45:43 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: RepType.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:45:43 $ */
/*
*  (c) Copyright 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmRepType_h
#define _XmRepType_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif


#include <Xm/Xm.h>


#ifdef __cplusplus
extern "C" {
#endif


#define XmREP_TYPE_INVALID		0x1FFF

#ifdef ALPHA_BUG_FIX
typedef unsigned long XmRepTypeId ; /* should be same size as XtPointer */
    /* Alpha porter note:
       Changed to match id field in XtConvertArgRec which is XtPointer 
       Use of short or int will cause a misalignment of data as they
       have different alignment requirement than long or char * on 
       some platforms (particularly on alpha, sizeof(int) != sizeof(long)).
       To do this 'right', there should be an XtPointer instead of the 
       XmRepTypeId in the XmRepTypeMappedRec & XmRepTypeRec to match 
       XtConvertArgRec and then be cast to the right thing in all places 
       (but more work than can do for this port effort) */
#else
typedef unsigned short XmRepTypeId ;
#endif

typedef struct
{   
    String rep_type_name ;
    String *value_names ;
    unsigned char *values ;
    unsigned char num_values ;
    Boolean reverse_installed ;
    XmRepTypeId rep_type_id ;
    }XmRepTypeEntryRec, *XmRepTypeEntry, XmRepTypeListRec, *XmRepTypeList ;


/********    Public Function Declarations    ********/
#ifdef VMS
/* VMS limit of 31 characters in extern function names */
#define XmRepTypeInstallTearOffModelConverter	XmRepTypeInstallTearOffModelCon
#endif

#ifdef _NO_PROTO

extern XmRepTypeId XmRepTypeRegister() ;
extern void XmRepTypeAddReverse() ;
extern Boolean XmRepTypeValidValue() ;
extern XmRepTypeList XmRepTypeGetRegistered() ;
extern XmRepTypeEntry XmRepTypeGetRecord() ;
extern XmRepTypeId XmRepTypeGetId() ;
extern String * XmRepTypeGetNameList() ;
extern void XmRepTypeInstallTearOffModelConverter() ;

#else

extern XmRepTypeId XmRepTypeRegister( 
                        String rep_type,
                        String *value_names,
                        unsigned char *values,
#if NeedWidePrototypes
                        unsigned int num_values) ;
#else
                        unsigned char num_values) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeAddReverse( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern Boolean XmRepTypeValidValue( 
#if NeedWidePrototypes
                        int rep_type_id,
                        unsigned int test_value,
#else
                        XmRepTypeId rep_type_id,
                        unsigned char test_value,
#endif /* NeedWidePrototypes */
                        Widget enable_default_warning) ;
extern XmRepTypeList XmRepTypeGetRegistered( void ) ;
extern XmRepTypeEntry XmRepTypeGetRecord( 
#if NeedWidePrototypes
                        int rep_type_id) ;
#else
                        XmRepTypeId rep_type_id) ;
#endif /* NeedWidePrototypes */
extern XmRepTypeId XmRepTypeGetId( 
                        String rep_type) ;
extern String * XmRepTypeGetNameList( 
#if NeedWidePrototypes
                        int rep_type_id,
                        int use_uppercase_format) ;
#else
                        XmRepTypeId rep_type_id,
                        Boolean use_uppercase_format) ;
#endif /* NeedWidePrototypes */
extern void XmRepTypeInstallTearOffModelConverter( void ) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/



#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmRepType_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
