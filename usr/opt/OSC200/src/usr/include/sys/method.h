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
 * @(#)$RCSfile: method.h,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/07/29 18:00:34 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDLOC) Locale Database Commands
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.1  com/inc/sys/method.h, libcloc, bos320, 9130320 7/17/91 17:25:11
 */
#ifndef __H_METHOD
#define __H_METHOD

#define MAX_METHOD_NAME   32

#define CHARMAP___MBSTOPCS      0x00
#define CHARMAP___MBTOPC        0x01
#define CHARMAP___PCSTOMBS      0x02
#define CHARMAP___PCTOMB        0x03
/*
 * Gap 0x04
 */
#define CHARMAP_CHARMAP_INIT    0x05
#define CHARMAP_MBLEN           0x06
#define CHARMAP_MBSTOWCS        0x07
#define CHARMAP_MBTOWC          0x08
#define CHARMAP_NL_LANGINFO     0x09 
/*
 * Gap 0x0a
 */
#define CHARMAP_WCSTOMBS        0x0b
#define CHARMAP_WCSWIDTH        0x0c
#define CHARMAP_WCTOMB          0x0d
#define CHARMAP_WCWIDTH         0x0e
#define COLLATE_FNMATCH         0x0f
#define COLLATE_REGCOMP         0x10
#define COLLATE_REGERROR        0x11
#define COLLATE_REGEXEC         0x12
#define COLLATE_REGFREE         0x13
#define COLLATE_STRCOLL         0x14
#define COLLATE_STRXFRM         0x15
#define COLLATE_WCSCOLL         0x16
#define COLLATE_WCSXFRM         0x17
#define COLLATE_COLLATE_INIT    0x18
#define CTYPE_WCTYPE            0x19
#define CTYPE_CTYPE_INIT        0x1a
#define CTYPE_ISWCTYPE          0x1b
#define CTYPE_TOWLOWER          0x1c
#define CTYPE_TOWUPPER          0x1d
#define LOCALE_LOCALE_INIT      0x1e
#define LOCALE_LOCALECONV       0x1f
#define LOCALE_NL_LANGINFO      0x20
#define MONETARY_MONETARY_INIT  0x21
#define MONETARY_NL_LANGINFO    0x22
#define MONETARY_STRFMON        0x23
/*
 * Gap 0x24..0x2a
 */
#define NUMERIC_NUMERIC_INIT    0x2b
#define NUMERIC_NL_LANGINFO     0x2c
#define RESP_RESP_INIT          0x2d
#define RESP_NL_LANGINFO        0x2e
#define RESP_RPMATCH            0x2f
#define TIME_TIME_INIT          0x30
#define TIME_NL_LANGINFO        0x31
#define TIME_STRFTIME           0x32
#define TIME_STRPTIME           0x33
#define TIME_WCSFTIME           0x34

#define LAST_METHOD             0x34


#define	SB_CODESET	0
#define	USR_CODESET	1

#define MX_METHOD_CLASS	2

typedef struct {
  char *method_name;			/* CLASS.component notation */
  int (*instance[MX_METHOD_CLASS])();	/* Entrypoint Address */
  char *c_symbol[MX_METHOD_CLASS];	/* Entrypoint (function name) */
  char *package[MX_METHOD_CLASS];	/* Package name */
  char *lib_name[MX_METHOD_CLASS];	/* Containing library */

  char *meth_proto;			/* Required calling conventions */
} method_t;

extern method_t *std_methods;
extern int method_class;		/* Controls which family of methods is used */

extern int mb_cur_max;

#define METH_PROTO(m)	(std_methods[m].meth_proto)
#define METH_NAME(m)    (std_methods[m].c_symbol[method_class])
#define METH_OFFS(m)    (std_methods[m].instance[method_class])

#endif  /* __H_METHOD */
