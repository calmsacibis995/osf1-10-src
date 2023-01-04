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
static char     *sccsid = "@(#)$RCSfile: method.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/07/29 18:00:31 $";
#endif
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
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/cmd/nls/method.c, cmdnls, bos320 6/1/91 14:47:04
 */

#include <sys/limits.h>
#include <stdlib.h>
#include <sys/method.h>

extern __mbstopcs_sb();
extern __mbtopc_sb();
extern __pctomb_sb();
extern __pcstombs_sb();
extern __nl_csinfo();
extern __mblen_sb();
extern __mbstowcs_sb();
extern __mbtowc_sb();
extern __wcstombs_sb();
extern __wcswidth_latin();
extern __wctomb_sb();
extern __wcwidth_latin();
extern __wctype_std();
extern __iswctype_std();
extern __strcoll_sb();
extern __strcoll_std();
extern __strxfrm_sb();
extern __strxfrm_std();
extern __towlower_std();
extern __towupper_std();
extern __wcscoll_sb();
extern __wcscoll_std();
extern __wcsxfrm_sb();
extern __wcsxfrm_std();
extern __localeconv_std();
extern __nl_langinfo_std();
extern __nl_moninfo();
extern __strfmon_std();
extern __catclose_std();
extern __catgets_std();
extern __nl_respinfo();
extern __nl_numinfo();
extern __rpmatch_std();
extern __nl_timinfo();
extern __strftime_std();
extern __strptime_std();
extern __wcsftime_std();
extern __regcomp_std();
extern __regerror_std();
extern __regexec_std();
extern __regfree_std();
extern __fnmatch_std();
extern __charmap_init();
extern __locale_init();
extern __monetary_init();
extern __numeric_init();
extern __resp_init();
extern __time_init();

/*
 * Define the normal place to search for methods.  The default is
 * in the shared libc (/usr/shlib/libc.so).
 */
#define LIBC		"/usr/shlib/libc.so"

/*
 * Define the standard package names.  Right now libc is it.
 * The NULL is for the extensible method support
 */
#define PKGLIST		"libc", NULL


static method_t std_methods_tbl[LAST_METHOD+1]={
{ "charmap.__mbstopcs", 
__mbstopcs_sb, 0, 
"__mbstopcs_sb", 0,
PKGLIST, LIBC, 0,
"int %s(wchar_t *, size_t, char *, size_t, int, char**, int *,_LC_charmap_t*)" },

{ "charmap.__mbtopc", 
__mbtopc_sb, 0, 
"__mbtopc_sb", 0,
PKGLIST, LIBC, 0,
"int %s(wchar_t *, char *, size_t, int*,_LC_charmap_t*)" },

{ "charmap.__pcstombs",
__pcstombs_sb, 0,
"__pcstombs_sb", 0,
PKGLIST, LIBC, 0,
"int %s(char*,size_t,wchar_t*,size_t,char**, int*,_LC_charmap_t*)" },

{ "charmap.__pctomb", 
__pctomb_sb, 0, 
"__pctomb_sb", 0,
PKGLIST, LIBC, 0,
"int %s(char *,wchar_t*,size_t,int*,_LC_charmap_t*)" },

/* Filler (was csid) */
{ 0,
0,0,
0,0,
PKGLIST, LIBC, 0,
0},


{ "charmap.init",  
__charmap_init, 0,
"__charmap_init", 0,
PKGLIST, LIBC, 0,
"_LC_charmap_t *%s(_LC_locale_t*)"},


{ "charmap.mblen",
__mblen_sb, 0, 
"__mblen_sb", 0,
PKGLIST, LIBC, 0,
"int %s(const char*, size_t,_LC_charmap_t*)" },

{ "charmap.mbstowcs", 
__mbstowcs_sb, 0, 
"__mbstowcs_sb", 0,
PKGLIST, LIBC, 0,
"size_t %s(wchar_t*,const char*,size_t,_LC_charmap_t*)" },

{ "charmap.mbtowc", 
__mbtowc_sb, 0, 
"__mbtowc_sb", 0,
PKGLIST, LIBC, 0,
"size_t %s(wchar_t*,const char*,size_t,_LC_charmap_t*)" },

{ "charmap.nl_langinfo",
__nl_csinfo, 0,
"__nl_csinfo", 0,
PKGLIST, LIBC, 0,
"char * %s(nl_item,_LC_charmap_t*)" },


/* Filler (was wcsid) */
{ 0,
0,0,
0,0,
PKGLIST, LIBC, 0,
0},


{ "charmap.wcstombs", 
__wcstombs_sb, 0, 
"__wcstombs_sb", 0,
PKGLIST, LIBC, 0,
"size_t %s(char*,const wchar_t*,size_t,_LC_charmap_t*)"},

{ "charmap.wcswidth", 
__wcswidth_latin, 0, 
"__wcswidth_latin", 0,
PKGLIST, LIBC, 0,
"size_t %s(const wchar_t *, size_t,_LC_charmap_t*)" },


{ "charmap.wctomb", 
__wctomb_sb, 0, 
"__wctomb_sb", 0,
PKGLIST, LIBC, 0,
"int %s( char*, wchar_t,_LC_charmap_t*)" },

{ "charmap.wcwidth", 
__wcwidth_latin, 0, 
"__wcwidth_latin", 0,
PKGLIST, LIBC, 0,
"size_t %s(const wchar_t,_LC_charmap_t*)" },

{ "collate.fnmatch",
__fnmatch_std, 0,
"__fnmatch_std", 0,
PKGLIST, LIBC, 0,
"int %s( const char*,const char*,const char*,int,_LC_collate_t*)" },

{ "collate.regcomp",
__regcomp_std, 0,
"__regcomp_std", 0,
PKGLIST, LIBC, 0,
"int %s(regex_t*,const char*,int,_LC_collate_t*)" },

{ "collate.regerror",
__regerror_std, 0,
"__regerror_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(int,const regex_t*,char*,size_t,_LC_collate_t*)" },

{ "collate.regexec",
__regexec_std, 0,
"__regexec_std", 0,
PKGLIST, LIBC, 0,
"int %s(const regex_t*,const char*,size_t,regmatch_t*,int,_LC_collate_t*)" },

{ "collate.regfree",
__regfree_std, 0,
"__regfree_std", 0,
PKGLIST, LIBC, 0,
"void %s(regex_t*,_LC_collate_t*)" },

{ "collate.strcoll",
__strcoll_std, 0,
"__strcoll_std", 0,
PKGLIST, LIBC, 0,
"int %s(const char*,const char*,_LC_collate_t*)" },

{ "collate.strxfrm",
__strxfrm_std, 0,
"__strxfrm_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(char*,const char*,size_t,_LC_collate_t*)" },

{ "collate.wcscoll",
__wcscoll_std, 0,
"__wcscoll_std", 0,
PKGLIST, LIBC, 0,
"int %s( const wchar_t*,const wchar_t*,_LC_collate_t*)" },

{ "collate.wcsxfrm",
__wcsxfrm_std, 0,
"__wcsxfrm_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(wchar_t*,const wchar_t*,size_t,_LC_collate_t*)" },

{ "collate.init", 
0,0,			/* No init required for collations */
0,0,			/* No "__name" */
PKGLIST, LIBC, 0,
"_LC_collate_t *%s()" },

{ "ctype.wctype",    
__wctype_std, 0,
"__wctype_std", 0,
PKGLIST, LIBC, 0,
"wctype_t %s( char *,_LC_ctype_t*)" },

{ "ctype.init", 
0,0,			/* No init required for ctype */
0,0,			/* No "name" */
PKGLIST, LIBC, 0,
"_LC_ctype_t *%s(_LC_locale_t*)" },

{ "ctype.iswctype",     
__iswctype_std, 0,
"__iswctype_std", 0,
PKGLIST, LIBC, 0,
"int %s( wchar_t, wctype_t,_LC_ctype_t*)" },

{ "ctype.towlower", 
__towlower_std, 0,
"__towlower_std", 0,
PKGLIST, LIBC, 0,
"wchar_t %s( wchar_t,_LC_ctype_t*)" },

{ "ctype.towupper", 
__towupper_std, 0, 
"__towupper_std", 0,
PKGLIST, LIBC, 0,
"wchar_t %s( wchar_t,_LC_ctype_t*)" },

{ "locale.init", 
__locale_init, 0,
"__locale_init", 0,
PKGLIST, LIBC, 0,
"_LC_locale_t *%s(_LC_locale_t*)" },

{ "locale.localeconv", 
__localeconv_std, 0,
"__localeconv_std", 0,
PKGLIST, LIBC, 0,
"struct lconv *%s()" },

{ "locale.nl_langinfo",
__nl_langinfo_std, 0,
"__nl_langinfo_std", 0,
PKGLIST, LIBC, 0,
"char *%s()" },

{ "monetary.init", 
__monetary_init, 0,
"__monetary_init", 0,
PKGLIST, LIBC, 0,
"_LC_monetary_t *%s(_LC_locale_t*)" },

{ "monetary.nl_langinfo",
__nl_moninfo, 0,
"__nl_moninfo", 0,
PKGLIST, LIBC, 0,
"char *%s(nl_item,_LC_monetary_t*)" },

{ "monetary.strfmon",
__strfmon_std, 0,
"__strfmon_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(char*,size_t,const char *,va_list,_LC_monetary_t*)" },

/*
 * The msg.* methods are unimplemented place-holders
 */

{ "msg.catclose", 0,0, 0,0, PKGLIST, LIBC,0, 0 },

{ "msg.catgets", 0,0, 0,0, PKGLIST, LIBC,0, 0 },

{ "msg.compress",0,0, 0,0, PKGLIST, LIBC,0, 0 },
{ "msg.decompress",0,0, 0,0, PKGLIST, LIBC,0, 0 },
{ "msg.end_compress",0,0, 0,0, PKGLIST, LIBC,0, 0 },
{ "msg.init", 0,0, 0,0, PKGLIST, LIBC,0, 0 },
{ "msg.start_compress",0,0, 0,0, PKGLIST, LIBC,0, 0 },

{ "numeric.init", 
__numeric_init, 0,
"__numeric_init", 0,
PKGLIST, LIBC, 0,
"_LC_numeric_t *%s(_LC_locale_t*)" },

{ "numeric.nl_langinfo", 
__nl_numinfo, 0,
"__nl_numinfo", 0,
PKGLIST, LIBC, 0,
"char *%s()" },

{ "resp.init", 
__resp_init, 0,
"__resp_init", 0,
PKGLIST, LIBC, 0,
"_LC_resp_t *%s(_LC_locale_t*)" },

{ "resp.nl_langinfo",
__nl_respinfo, 0,
"__nl_respinfo", 0,
PKGLIST, LIBC, 0,
"char *%s()" },

{ "resp.rpmatch", 
__rpmatch_std, 0,
"__rpmatch_std", 0,
PKGLIST, LIBC, 0,
"int %s()" },

{ "time.init", 
__time_init, 0,
"__time_init", 0,
PKGLIST, LIBC, 0,
"_LC_time_t *%s(_LC_locale_t*)" },

{ "time.nl_langinfo",    
__nl_timinfo, 0,
"__nl_timinfo", 0,
PKGLIST, LIBC, 0,
"char *%s()" },

{ "time.strftime",
__strftime_std, 0, 
"__strftime_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(char*,size_t,const char*,const struct tm*,_LC_time_t*)" },

{ "time.strptime", 
__strptime_std, 0,
"__strptime_std", 0,
PKGLIST, LIBC, 0,
"char * %s(const char*,const char*,struct tm*,_LC_time_t*)" },

{ "time.wcsftime", 
__wcsftime_std, 0,
"__wcsftime_std", 0,
PKGLIST, LIBC, 0,
"size_t %s(wchar_t*,size_t,const char*,const struct tm*,_LC_time_t*)" },
};

method_t *std_methods = std_methods_tbl;
