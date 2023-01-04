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
static char	*sccsid = "@(#)$RCSfile: nl_langinfo.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:31:01 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: nl_langinfo
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/loc/nl_langinfo.c, libcloc, 9130320 7/17/91 15:05:42
 */
/*
 * FUNCTION: nl_langinfo
 *
 * DESCRIPTION: stub function which invokes locale specific method 
 * which implements the nl_langinfo() function.
 *
 * RETURNS:
 * char * ptr to locale string.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak nl_langinfo = __nl_langinfo
#endif
#endif
#include <sys/localedef.h>
#include <langinfo.h>
#include <string.h>

/*
 * These are the old nl_langinfo() constants from OSF/1 1.0.
 * convert_item() converts them to their 1.2 values.
 */

#define OLD_D_T_FMT   0x0001 /* string for formatting date and time */
#define OLD_D_FMT     0x0002 /* string for formatting date */
#define OLD_T_FMT     0x0003 /* string for formatting time */
#define OLD_AM_STR    0x0004 /* string for a.m. */
#define OLD_PM_STR    0x0005 /* string for p.m. */
#define OLD_DAY_1     0x0101 /* name of the first day of the week (Sunday) */
#define OLD_DAY_2     0x0102 /* name of the second day of the week (Monday) */
#define OLD_DAY_3     0x0103 /* name of the third day of the week (Tuesday) */
#define OLD_DAY_4     0x0104 /* name of the fourth day of the week (Wednesday) */
#define OLD_DAY_5     0x0105 /* name of the fifth day of the week (Thursday) */
#define OLD_DAY_6     0x0106 /* name of the sixth day of the week (Friday) */
#define OLD_DAY_7     0x0107 /* name of the seventh day of the week (Saturday) */
#define OLD_ABDAY_1   0x0201 /* abbreviated first day of the week (Sun) */
#define OLD_ABDAY_2   0x0202 /* abbreviated second day of the week (Mon) */
#define OLD_ABDAY_3   0x0203 /* abbreviated third day of the week (Tue) */
#define OLD_ABDAY_4   0x0204 /* abbreviated fourth day of the week (Wed) */
#define OLD_ABDAY_5   0x0205 /* abbreviated fifth day of the week (Thu) */
#define OLD_ABDAY_6   0x0206 /* abbreviated sixth day of the week (Fri) */
#define OLD_ABDAY_7   0x0207 /* abbreviated seventh day of the week (Sat) */
#define OLD_MON_1     0x0301 /* name of the first month (January) */
#define OLD_MON_2     0x0302 /* name of the second month (February) */
#define OLD_MON_3     0x0303 /* name of the third month (March) */
#define OLD_MON_4     0x0304 /* name of the fourth month (April) */
#define OLD_MON_5     0x0305 /* name of the fifth month (May) */
#define OLD_MON_6     0x0306 /* name of the sixth month (June) */
#define OLD_MON_7     0x0307 /* name of the seventh month (July) */
#define OLD_MON_8     0x0308 /* name of the eighth month (August) */
#define OLD_MON_9     0x0309 /* name of the ninth month (September) */
#define OLD_MON_10    0x030a /* name of the tenth month (October) */
#define OLD_MON_11    0x030b /* name of the eleventh month (November) */
#define OLD_MON_12    0x030c /* name of the twelveth month (December) */
#define OLD_ABMON_1   0x0401 /* abbreviated first month (Jan) */
#define OLD_ABMON_2   0x0402 /* abbreviated second month (Feb) */
#define OLD_ABMON_3   0x0403 /* abbreviated third month (Mar) */
#define OLD_ABMON_4   0x0404 /* abbreviated fourth month (Apr) */
#define OLD_ABMON_5   0x0405 /* abbreviated fifth month (May) */
#define OLD_ABMON_6   0x0406 /* abbreviated sixth month (Jun) */
#define OLD_ABMON_7   0x0407 /* abbreviated seventh month (Jul) */
#define OLD_ABMON_8   0x0408 /* abbreviated eighth month (Aug) */
#define OLD_ABMON_9   0x0409 /* abbreviated ninth month (Sep) */
#define OLD_ABMON_10  0x040a /* abbreviated tenth month (Oct) */
#define OLD_ABMON_11  0x040b /* abbreviated eleventh month (Nov) */
#define OLD_ABMON_12  0x040c /* abbreviated twelveth month (Dec) */
#define OLD_RADIXCHAR 0x0501 /* radix character */
#define OLD_THOUSEP   0x0601 /* separator for thousands */
#define OLD_YESSTR    0x0701 /* affiramitive response for yes/no queries */
#define OLD_NOSTR     0x0801 /* negative response for yes/no queries */
#define OLD_CRNCYSTR  0x0901 /* currency symbol; - leading, + trailing */

static nl_item convert_item(nl_item old_item)
{
    nl_item new_item;

    switch (old_item) {
    case OLD_D_T_FMT:   new_item = D_T_FMT;     break;
    case OLD_D_FMT:     new_item = D_FMT;       break;
    case OLD_T_FMT:     new_item = T_FMT;       break;
    case OLD_AM_STR:    new_item = AM_STR;      break;
    case OLD_PM_STR:    new_item = PM_STR;      break;
    case OLD_DAY_1:     new_item = DAY_1;       break;
    case OLD_DAY_2:     new_item = DAY_2;       break;     
    case OLD_DAY_3:     new_item = DAY_3;       break;     
    case OLD_DAY_4:     new_item = DAY_4;       break;     
    case OLD_DAY_5:     new_item = DAY_5;       break;     
    case OLD_DAY_6:     new_item = DAY_6;       break;     
    case OLD_DAY_7:     new_item = DAY_7;       break;     
    case OLD_ABDAY_1:   new_item = ABDAY_1;     break;   
    case OLD_ABDAY_2:   new_item = ABDAY_2;     break;   
    case OLD_ABDAY_3:   new_item = ABDAY_3;     break;   
    case OLD_ABDAY_4:   new_item = ABDAY_4;     break;   
    case OLD_ABDAY_5:   new_item = ABDAY_5;     break;   
    case OLD_ABDAY_6:   new_item = ABDAY_6;     break;   
    case OLD_ABDAY_7:   new_item = ABDAY_7;     break;   
    case OLD_MON_1:     new_item = MON_1;       break;     
    case OLD_MON_2:     new_item = MON_2;       break;     
    case OLD_MON_3:     new_item = MON_3;       break;    
    case OLD_MON_4:     new_item = MON_4;       break;     
    case OLD_MON_5:     new_item = MON_5;       break;     
    case OLD_MON_6:     new_item = MON_6;       break;     
    case OLD_MON_7:     new_item = MON_7;       break;     
    case OLD_MON_8:     new_item = MON_8;       break;     
    case OLD_MON_9:     new_item = MON_9;       break;     
    case OLD_MON_10:    new_item = MON_10;      break;    
    case OLD_MON_11:    new_item = MON_11;      break;
    case OLD_MON_12:    new_item = MON_12;      break;
    case OLD_ABMON_1:   new_item = ABMON_1;     break;
    case OLD_ABMON_2:   new_item = ABMON_2;     break;
    case OLD_ABMON_3:   new_item = ABMON_3;     break;
    case OLD_ABMON_4:   new_item = ABMON_4;     break;
    case OLD_ABMON_5:   new_item = ABMON_5;     break;
    case OLD_ABMON_6:   new_item = ABMON_6;     break;
    case OLD_ABMON_7:   new_item = ABMON_7;     break;
    case OLD_ABMON_8:   new_item = ABMON_8;     break;
    case OLD_ABMON_9:   new_item = ABMON_9;     break;
    case OLD_ABMON_10:  new_item = ABMON_10;    break;
    case OLD_ABMON_11:  new_item = ABMON_11;    break;
    case OLD_ABMON_12:  new_item = ABMON_12;    break;
    case OLD_RADIXCHAR: new_item = RADIXCHAR;   break;
    case OLD_THOUSEP:   new_item = THOUSEP;     break;
    case OLD_YESSTR:    new_item = YESSTR;      break;
    case OLD_NOSTR:     new_item = NOSTR;       break;
    case OLD_CRNCYSTR:  new_item = CRNCYSTR;    break;
    default:		new_item = old_item;	break;
    }

    return new_item;
}

/*
 *  FUNCTION: __nl_langinfo_std
 *
 *  DESCRIPTION:
 *  Returns the locale database string which corresponds to the specified
 *  nl_item.
 */

#define BUFLEN	20

static char yesbuf[BUFLEN+1];
static char nobuf[BUFLEN+1];

char *
__nl_langinfo_std( nl_item item, _LC_locale_t *locale)
{
    char *ptr1, *ptr2;
    int  len;

    if (item >= _NL_NUM_ITEMS || item < 0)
	return "";

    if ((item == YESSTR) || (item == NOSTR)){
	ptr1 = locale->nl_info[item];
	ptr2 = strchr(ptr1,':');      	     /* This may cause problems */
	if (ptr2 == NULL)                    /* because : is not a protected */
	    return ptr1;                     /* character in 932            */
	else {
	    if ((len = (ptr2 - ptr1)) > BUFLEN)  
	        ptr2 = (char *)malloc(len+1);
	    else
	        ptr2 = ((item == YESSTR) ? yesbuf : nobuf);
	    strncpy(ptr2,ptr1,len);
	    ptr2[len] = '\0';
	    return ptr2;
        }
    }
    else if (item >= _NL_OLD_SIZE)
	/*
	 * Check for a new vs. an old locale.  Old ones don't have
	 * the new XPG4 fields.
	 */
	if (locale->core.hdr.size >= sizeof(_LC_locale_t))
	    return locale->nl_info2[item-_NL_OLD_SIZE];
    	else
	    return "";
    else
        return locale->nl_info[item];
}

char *
nl_langinfo(nl_item item)
{
	/*
	 * If the item is invalid, assume it's from OSF/1 1.0, and try
	 * to convert it to its 1.2 value.
	 */
	if (item >= _NL_NUM_ITEMS)
		item = convert_item(item);

	if (METHOD(__lc_locale,nl_langinfo) == NULL)
		return __nl_langinfo_std(item, __lc_locale);
	else
  		return METHOD(__lc_locale,nl_langinfo)( item, __lc_locale);
}

char *__nl_numinfo() {return (NULL);}
char *__nl_moninfo() {return (NULL);}
char *__nl_respinfo() {return (NULL);}
char *__nl_timinfo() {return (NULL);}
char *__nl_csinfo() {return (NULL);}
