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
# ifdef __osf__
char	*_ctype__;
char	*_pctype;
init_osf_compat() 
{
	char	*compat_ctype();

	_ctype__ = compat_ctype();
	/* assume we want the same thing */
	_pctype = _ctype__;
}

/* TRICKY -- This will redefine _ctype */

# include	<ctype.h>

/*
 * ret the address of the OSF ctype struct which is a # define
 */
char	*
compat_ctype()
{
	return (_ctype__);
}
# include	<sys/timeb.h>

/* because we don't have this in OSF?
 */
void
ftime(tp)
struct timeb	*tp;
{
	bzero((char *)tp, sizeof *tp);
	(void)time(&tp->time);
}
# endif
