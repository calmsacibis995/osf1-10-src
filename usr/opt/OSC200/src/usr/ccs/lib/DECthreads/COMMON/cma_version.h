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
 * @(#)$RCSfile: cma_version.h,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/11/23 23:46:15 $
 */

/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	This module contains a version string
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	15 April 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	17 April 1992
 *		Turn it into a .h file, & define macros instead of externs.
 *
 */

/*
 *  INCLUDE FILES
 */

/*
 * GLOBAL MACROS
 */

/*
 * Define two macros -- one with just the version itself, and the other with
 * a string pattern compatible with the 'what' command ("@(#)" prefix) or a
 * simple "strings|grep 'version'" filter. They're somewhat redundant, but
 * without counting on ANSI C, we can't concatinate macros into strings, so
 * it's better to just let the build's filter give both forms.
 */
#define cma__c_version	"V1.11-AgS2"
#define cma__c_vref	"@(#) DECthreads version V1.11-AgS2"

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */
