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
static char rcsid[] = "@(#)$RCSfile: topen.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/22 21:35:35 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** topen.c 1.1, last change 8/9/89
 **/

#include <tli/common.h>
#include <stropts.h>
#include <tli/timod.h>
#ifdef XTI
#include <fcntl.h>
#endif


/* 
 * If something fails and we need to call t_close then save the errors, so
 * that the error returned will be meaningful for the caller to analyze.
 *
 */
#define T_close(fd)	{	\
				int te; int e; \
				te = _Get_terrno(); e = _Geterrno();\
				t_close(fd);\
				_Set_terrno(te); _Seterrno(e);\
			}

int
t_open (
	char *		name,
	int		oflag,
	struct t_info * tinfo)
{
	int		fd;
        int     	code;

        code = -1;
#ifdef XTI
	if (oflag & ~(O_RDWR | O_NONBLOCK)) {
		_Set_terrno(TBADFLAG);
		goto rtn;
	}
	if (oflag & O_NONBLOCK) {
		oflag &= ~O_NONBLOCK;
		oflag |= O_NDELAY;
	}
#endif
	if ((fd = stream_open(name, oflag)) != -1) {
		if ( stream_ioctl(fd, I_PUSH, "timod") != -1 ) {
#ifdef XTI
			if (tli_ioctl(fd, TI_XTI_MODE, nilp(char), 0) == -1) {
#else
			if (tli_ioctl(fd, TI_TLI_MODE, nilp(char), 0) == -1) {
#endif
				T_close(fd);
				goto rtn;
			}
			if (tinfo  &&  t_getinfo(fd, tinfo) == -1) {
				T_close(fd);
				goto rtn;
			}
			(void)t_sync(fd);
			code = fd;
			goto rtn;
		}
		stream_close(fd);
	}
#ifdef XTI
	_Set_terrno((_Geterrno() == ENOENT) ? TBADNAME : TSYSERR);
#else
	_Set_terrno(TSYSERR);
#endif

rtn:
#ifdef XTIDBG
        tr_open (name, oflag, tinfo, code);
#endif
	return code;
}
