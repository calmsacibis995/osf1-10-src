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
static char *rcsid = "@(#)$RCSfile: wcsftime.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:00:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  wcsftime
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/fmt/wcsftime.c, libcfmt, bos320, 9130320 7/17/91 15:24:18
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak wcsftime = __wcsftime
#endif
#include <sys/localedef.h>
#include <time.h>

size_t __wcsftime_std( wchar_t *, size_t, const char *, const struct tm *, _LC_time_t *);

/*
 * FUNCTION: wcsftime() is a method driven function where the time formatting
 *	     processes are done in the method poninted by 
 *	     __lc_time->core.wcsftime.
 *           This function behaves the same as strftime() except the
 *           ouput buffer is wchar_t. Indeed, wcsftime_std() calls strftime()
 *           which performs the conversion in single byte first. Then the
 *           output from strftime() is converted to wide character string by
 *           mbstowcs().
 *
 * PARAMETERS:
 *           const char *ws  - the output data buffer in wide character
 *                             format.
 *           size_t maxsize  - the maximum number of wide character including
 *                             the terminating null to be output to ws buffer.
 *           const char *fmt - the format string which specifies the expected
 *                             format to be output to the ws buffer.
 *           struct tm *tm   - the time structure to provide specific time
 *                             information when needed.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the number of bytes placed into the
 *             ws buffer not including the terminating null byte.
 *           - if fail for any reason, it returns 0.
 */


size_t 
wcsftime(wchar_t *ws, size_t maxsize, 
                    const char *format, const struct tm *timeptr)
{
	if (METHOD(__lc_time,wcsftime) == NULL)
		return __wcsftime_std(ws, maxsize, format, timeptr, __lc_time);
	else
		return METHOD(__lc_time,wcsftime)( ws, maxsize,
                                           format, timeptr, __lc_time);
}


#include <time.h>
#include <stddef.h>
#include <stdlib.h>

#define RETURN(x)       free(temp); \
                        return (x)
/*
 * FUNCTION: This is the standard method for function wcsftime.
 *	     This function behaves the same as strftime() except the 
 *	     ouput buffer is wchar_t. Indeed, __wcsftime_std() calls strftime()
 *	     which performs the conversion in single byte first. Then the
 *	     output from strftime() is converted to wide character string by
 *	     mbstowcs().
 *
 * PARAMETERS:
 *           _LC_time_t *hdl - pointer to the handle of the LC_TIME
 *                             catagory which contains all the time related
 *                             information of the specific locale.
 *           const char *ws  - the output data buffer in wide character
 *			       format.
 *	     size_t maxsize  - the maximum number of wide character including
 *			       the terminating null to be output to ws buffer.
 *           const char *fmt - the format string which specifies the expected
 *                             format to be output to the ws buffer.
 *           struct tm *tm   - the time structure to provide specific time
 *			       information when needed.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the number of bytes placed into the
 *	       ws buffer not including the terminating null byte.
 *           - if fail for any reason, it returns 0.
 */

size_t 
__wcsftime_std( wchar_t *ws, size_t maxsize, 
                    const char *format, const struct tm *timeptr, _LC_time_t *hdl)
{
	char	*temp;
        size_t  size;
        size_t  rc;
	int	wc_num;
	
	
        size = MB_CUR_MAX * maxsize;
	if ( (temp = (char *)malloc(size+1)) == NULL)
                return(0);
	rc = strftime (temp, size, format, timeptr);
	temp[rc] = '\0';
        if ((wc_num = mbstowcs (ws, temp, maxsize-1)) == -1) {
                RETURN(0);
        }
	ws[wc_num] = L'\0';
	if (rc) {
                if (wc_num < maxsize) {
                        RETURN(wc_num);
                }
                else {
                        RETURN(0);
                }
	}
        else {
                RETURN(0);
        }
}
