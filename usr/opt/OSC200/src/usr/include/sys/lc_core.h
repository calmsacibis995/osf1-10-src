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
 * @(#)$RCSfile: lc_core.h,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/12/15 22:14:27 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* 1.6  com/inc/sys/lc_core.h, libccnv, bos320, 9132320m 8/11/91 14:14:46
 *
 * COMPONENT_NAME: (LIBCLOC) Locale Related Data Structures and API
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef __H_LC_CORE
#define __H_LC_CORE


/*
 * In OSF/1, method elements in the structures point to functions
 */


/*
** Valid type ids for NLS objects
*/
typedef enum __lc_type_id_t {
    _LC_CAR=1,
    _LC_LOCALE=2,
    _LC_CHARMAP=3,
    _LC_CTYPE=4,
    _LC_COLLATE=5,
    _LC_NUMERIC=6,
    _LC_MONETARY=7,
    _LC_TIME=8,
    _LC_RESP=9 } __lc_type_id_t;


typedef struct {

    __lc_type_id_t
	type_id;
    unsigned short
	magic;
    unsigned long
	version;
    
    unsigned long  size;

} _LC_object_t;

/*
** Object magic (Looks like "OSF1")
*/
#define _LC_MAGIC     0x05F1

/*
** Version.  Anytime the data structures in localedef or lc_core change
**	     in an incompatible way, this number should change
*/
#define _LC_VERSION   0x1


typedef struct {

    _LC_object_t  hdr;

    /* locale info method */
    char     *(*nl_langinfo) ();
    
    /* Process code conversion methods */
    unsigned long   (*mbtowc) ();
    unsigned long   (*mbstowcs) ();
    int      (*wctomb) ();
    unsigned long   (*wcstombs) ();
    
    /* Character encoding length method */
    int      (*mblen) ();
    
    /* Character display width methods */
    unsigned long   (*wcswidth) ();
    unsigned long   (*wcwidth) ();
    
    /* private PC/CP converters */
    int      (*__mbtopc) ();
    int      (*__mbstopcs) ();
    int      (*__pctomb) ();
    int      (*__pcstombs) ();
    
    /* implementation initialization */
    _LC_charmap_t *(*init) ();
    void     *data;
} _LC_core_charmap_t;


typedef struct {
 
    _LC_object_t  hdr; 
    
    /* case convertersion methods */
    unsigned int  (*towupper) ();
    unsigned int  (*towlower) ();
    
    /* classification methods */
    unsigned int    (*wctype) ();
    int      (*iswctype) ();
    
    /* implementation initialization */
    _LC_ctype_t   *(*init) ();
    void     *data;
} _LC_core_ctype_t;

typedef struct {

    _LC_object_t  hdr;

    /* character collation methods */
    int      (*strcoll) ();
    unsigned long   (*strxfrm) ();
    
    /* process code collation methods */
    int      (*wcscoll) ();
    unsigned long   (*wcsxfrm) ();
    
    /* filename matching methods */
    int      (*fnmatch) ();
    
    /* regular expression methods */

    int      (*regcomp) ();
    unsigned long   (*regerror) ();
    int      (*regexec) ();
    void     (*regfree) ();

    /* implementation initialization */
    _LC_collate_t *(*init) ();
    void     *data;
} _LC_core_collate_t;


struct tm;
typedef struct {

    _LC_object_t  hdr;
    
    /* time info method */
    char     *(*nl_langinfo) ();
    
    /* time character string formatting methods */
    unsigned long   (*strftime) ();
    char     *(*strptime) ();
    
    /* time process code string formatting methods */
    unsigned long   (*wcsftime) ();
    
    /* implementation initialization */
    _LC_time_t    *(*init) ();
    void     *data;
} _LC_core_time_t;


typedef struct {

    _LC_object_t  hdr;

    /* monetary info method */
    char     *(*nl_langinfo) ();
    
    /* character string monetary formatting method */
    unsigned long   (*strfmon) ();
    
    /* implementation initialization */
    _LC_monetary_t        *(*init) ();
    void     *data;
} _LC_core_monetary_t;


typedef struct {

    _LC_object_t  hdr;

    /* langinfo method */
    char     *(*nl_langinfo) ();
    
    /* implementation initialization */
    _LC_numeric_t *(*init) ();
    void     *data;
} _LC_core_numeric_t;


typedef struct {

    _LC_object_t  hdr;
    
    /* langinfo method */
    char        *(*nl_langinfo) ();
    
    /* response matching method */
    int		(*rpmatch) ();
    
    /* implementation initialization */
    _LC_resp_t    *(*init) ();
    void        *data;
} _LC_core_resp_t;

/* forward declaration as required by C++ */
#ifdef __cplusplus
struct lconv;
#endif

typedef struct {

    _LC_object_t hdr;

    /* langinfo method */
    char         *(*nl_langinfo) ();
    struct lconv * (*localeconv) ();
    
    /* Initialization */
    _LC_locale_t  *(*init) ();
    void         *data;
} _LC_core_locale_t;

#endif /* __H_LC_CORE */
