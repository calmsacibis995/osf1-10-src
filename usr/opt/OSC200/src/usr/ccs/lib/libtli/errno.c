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

#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: errno.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/16 23:19:31 $";
#endif

#ifdef	errno
#undef	errno
#endif	/* errno */
extern int	errno;

#ifdef	_THREAD_SAFE
#include	<lib_data.h>

extern lib_data_functions_t	_libc_data_funcs;
extern void			*_errno_hdl;

#define	Get_Error_Ref	((int *)lib_data_ref(_libc_data_funcs, _errno_hdl))

/*
 * Function:
 *	_Errno
 *
 * Return value:
 *	address of the per-thread errno if available or else
 *	address of the global errno
 *
 * Description:
 *	Allow access to a per-thread errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global errno in case per-thread data is not available.
 */
int *
_Errno()
{
	int	*err;

	return ((err = Get_Error_Ref) ? err : &errno);
}
#endif	/* _THREAD_SAFE */

/*
 * Function:
 *	_Geterrno
 *
 * Return value:
 *	value of the per-thread errno if available or else
 *	value of the global errno
 *
 * Description:
 *	For libraries only. Retrieve the value of errno from the per-thread
 *	or glabal variable.
 */
int
_Geterrno()
{
#ifdef	_THREAD_SAFE
	int	*err;

	return ((err = Get_Error_Ref) ? *err : errno);
#else	/* _THREAD_SAFE */
	return (errno);
#endif	/* _THREAD_SAFE */
}

/*
 * Function:
 *	_Seterrno
 *
 * Parameters:
 *	error	- value to set errno to
 *
 * Return value:
 *	new value of errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global errnos.
 *	_THREAD_SAFE case helps code which still uses the global errno.
 */
int
_Seterrno(int error)
{
#ifdef	_THREAD_SAFE
	int	*err;

	if ((err = Get_Error_Ref))
		*err = error;
#endif	/* _THREAD_SAFE */
	errno = error;
	return (error);
}
