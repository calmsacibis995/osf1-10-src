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
 *	@(#)$RCSfile: langinfo.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/08 01:01:54 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#ifndef _LANGINFO_H_
#define _LANGINFO_H_
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.8  com/inc/langinfo.h, libcnls, bos320, 9132320 7/30/91 11:27:44 
 */

#include <standards.h>
#include <nl_types.h>

#ifdef _XOPEN_SOURCE

#define D_T_FMT    1  /* string for formatting date and time */
#define D_FMT      2  /* string for formatting date */
#define T_FMT      3  /* string for formatting time */
#define AM_STR     4  /* string for a.m. */
#define PM_STR     5  /* string for p.m. */

#define ABDAY_1    6  /* abbreviated first day of the week (Sun) */
#define ABDAY_2    7  /* abbreviated second day of the week (Mon) */
#define ABDAY_3    8  /* abbreviated third day of the week (Tue) */
#define ABDAY_4    9  /* abbreviated fourth day of the week (Wed) */
#define ABDAY_5   10  /* abbreviated fifth day of the week (Thu) */
#define ABDAY_6   11  /* abbreviated sixth day of the week (Fri) */
#define ABDAY_7   12  /* abbreviated seventh day of the week (Sat) */

#define DAY_1     13  /* name of the first day of the week (Sunday) */
#define DAY_2     14  /* name of the second day of the week (Monday) */
#define DAY_3     15  /* name of the third day of the week (Tuesday) */
#define DAY_4     16  /* name of the fourth day of the week (Wednesday) */
#define DAY_5     17  /* name of the fifth day of the week (Thursday) */
#define DAY_6     18  /* name of the sixth day of the week (Friday) */
#define DAY_7     19  /* name of the seventh day of the week (Saturday) */

#define ABMON_1   20  /* abbreviated first month (Jan) */
#define ABMON_2   21  /* abbreviated second month (Feb) */
#define ABMON_3   22  /* abbreviated third month (Mar) */
#define ABMON_4   23  /* abbreviated fourth month (Apr) */
#define ABMON_5   24  /* abbreviated fifth month (May) */
#define ABMON_6   25  /* abbreviated sixth month (Jun) */
#define ABMON_7   26  /* abbreviated seventh month (Jul) */
#define ABMON_8   27  /* abbreviated eighth month (Aug) */
#define ABMON_9   28  /* abbreviated ninth month (Sep) */
#define ABMON_10  29  /* abbreviated tenth month (Oct) */
#define ABMON_11  30  /* abbreviated eleventh month (Nov) */
#define ABMON_12  31  /* abbreviated twelveth month (Dec) */

#define MON_1     32  /* name of the first month (January) */
#define MON_2     33  /* name of the second month (February) */
#define MON_3     34  /* name of the third month (March) */
#define MON_4     35  /* name of the fourth month (April) */
#define MON_5     36  /* name of the fifth month (May) */
#define MON_6     37  /* name of the sixth month (June) */
#define MON_7     38  /* name of the seventh month (July) */
#define MON_8     39  /* name of the eighth month (August) */
#define MON_9     40  /* name of the ninth month (September) */
#define MON_10    41  /* name of the tenth month (October) */
#define MON_11    42  /* name of the eleventh month (November) */
#define MON_12    43  /* name of the twelveth month (December) */

#define RADIXCHAR 44  /* radix character */
#define THOUSEP   45  /* separator for thousands */
#define YESSTR    46  /* affiramitive response for yes/no queries */
#define NOSTR     47  /* negative response for yes/no queries */
#define CRNCYSTR  48  /* currency symbol; - leading, + trailing */
#define CODESET 49   /* codeset name */
#define _M_D_RECENT	50	/* OSF extension  recent month-date format */
#define _M_D_OLD	51	/* OSF extension  old month-date format */
#define	ERA		52	/* Era description segment */
#define ERA_D_FMT	53	/* Era  date format */
#define ERA_D_T_FMT	54	/* Era date and time */
#define ERA_T_FMT	55	/* Era time format */

/*
 * WARNING!!!
 * 	nl_langinfo() table space in existing locales only permits
 *	55 entries.  Because this wasn't at the end of an object, we have to
 *	make a secondary 'spill' area for additional components.  These aren't
 * 	in the nl_info[] structure, rather they're found in the nl_info2[] structure
 */

#define T_FMT_AMPM	56	/* AM or PM time format string */
#define ALT_DIGITS	57	/* alternative symbols for digits */
#define NOEXPR		58	/* Negative response expression */
#define YESEXPR		59	/* Affermative response expression */
#define __UNUSED_1	60	/* Available */

/**********
** if this number changes, it MUST be changed
** in sys/localedef.h
**********/
#ifndef _NL_NUM_ITEMS
#define _NL_NUM_ITEMS 61
#endif

#if defined(__cplusplus)
extern "C" {
#endif
extern char *nl_langinfo __((nl_item));
#if defined(__cplusplus)
}
#endif

#endif /* _XOPEN_SOURCE */
#endif /* _LANGINFO_H_ */
