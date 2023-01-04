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
static char rcsid[] = "@(#)$RCSfile: terrno.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/03/16 23:20:22 $";
#endif
#include <tli/common.h>

#ifdef	t_errno
#undef	t_errno
#endif	/* t_errno */
int	t_errno;

#ifdef	_THREAD_SAFE

extern lib_data_functions_t	__t_data_funcs;
extern void			*__terrno_hdl;

#define	Get_terrno_Ref	((int *)lib_data_ref(__t_data_funcs, __terrno_hdl))

/*
 * Function:
 *	_terrno
 *
 * Return value:
 *	address of the per-thread t_errno if available or else
 *	address of the global t_errno
 *
 * Description:
 *	Allow access to a per-thread t_errno. Returning the address enables
 *	existing l/r-value rules to set/get the correct value. Default to
 *	the global errno in case per-thread data is not available.
 */
int *
_terrno()
{
	int	*err;

	return ((err = Get_terrno_Ref) ? err : &t_errno);
}
#endif	/* _THREAD_SAFE */

/*
 * Function:
 *	_Get_terrno
 *
 * Return value:
 *	value of the per-thread terrno if available or else
 *	value of the global errno
 *
 * Description:
 *	For libraries only. Retrieve the value of terrno from the per-thread
 *	or glabal variable.
 */
int
_Get_terrno()
{
#ifdef	_THREAD_SAFE
	int	*err;

	return ((err = Get_terrno_Ref) ? *err : t_errno);
#else	/* _THREAD_SAFE */
	return (t_errno);
#endif	/* _THREAD_SAFE */
}

/*
 * Function:
 *	_Set_terrno
 *
 * Parameters:
 *	error	- value to set terrno to
 *
 * Return value:
 *	new value of t_errno
 *
 * Description:
 *	For libraries only. Set both the per-thread and global t_errnos.
 *	_THREAD_SAFE case helps code which still uses the global t_errno.
 */
int
_Set_terrno(int error)
{
#ifdef	_THREAD_SAFE
	int	*err;

	if ((err = Get_terrno_Ref))
		*err = error;
#endif	/* _THREAD_SAFE */
	t_errno = error;
	return (error);
}
