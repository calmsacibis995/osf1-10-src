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
static char *rcsid = "@(#)$RCSfile: tstrerror.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/23 18:30:40 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <tli/common.h>
#include <stdio.h>
#include <nl_types.h>
#include "libtli_msg.h"

static	char *strerror;

#ifndef	_THREAD_SAFE

#define	_tstrerror()	(&strerror)

#else	/* _THREAD_SAFE */

extern lib_data_functions_t	__t_data_funcs;
extern void			*__tstrerror_hdl;

#define	Get_tstrerror_Ref	((char **)lib_data_ref(__t_data_funcs, __tstrerror_hdl))

/*
 * Function:
 *	_tstrerror
 *
 * Return value:
 *	address of the per-thread strerror if available or else
 *	address of the global strerror
 *
 * Description:
 *	Allow access to a per-thread strerror. Returning the address enables
 *	existing l/r-value rules to get the correct value. Default to
 *	the global strerror in case per-thread data is not available.
 */
char **
_tstrerror()
{
	char	**err;

	return ((err = Get_tstrerror_Ref) ? err : &strerror);
}
#endif	/* _THREAD_SAFE */

char *
t_strerror ( int errnum)
{

	nl_catd catd;
	extern int	t_nerr;
	char *msg, **strptr;

	catd = catopen(MF_LIBTLI, NL_CAT_LOCALE);

	strptr = _tstrerror();
	if (*strptr) 
		free(*strptr);
	if (errnum < 0 || errnum >= t_nerr) {
		msg = catgets(catd,MS_LIBTLI,TERROR_STRERR,"%d: error unknown");
		/* 
		 * allocate I18N message plus room for errnum to be added
		 */
		*strptr = (char *)malloc(strlen(msg) + 64);
		sprintf(*strptr, msg, errnum);
	} else {
		int len;
		/* Valid error code in XPG4 range - skip 3 catalog entries */
#ifdef XPG4
		if (errnum >= TINDOUT)
			msg = catgets(catd, MS_LIBTLI, errnum+3, t_errlist[errnum]);
		else
#endif
			msg = catgets(catd, MS_LIBTLI, errnum, t_errlist[errnum]);
		len = strlen(msg) + 1;
		*strptr = (char *)malloc(len);
		memcpy(*strptr, msg, len);
	}
	catclose(catd);
	return *strptr;
}
