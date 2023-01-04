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
static char rcsid[] = "@(#)$RCSfile: tgetinfo.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/03/16 23:20:45 $";
#endif

/** Copyright (c) 1989  Mentat Inc.
 ** tgetinfo.c 1.2, last change 11/8/89
 **/

#include <tli/common.h>
#include <sys/stream.h>
#include <stropts.h>
#include <tli/tihdr.h>
#include <tli/timod.h>

int
t_getinfo (
	int			fd,
	struct t_info * 	info)
{
	struct T_info_ack	tinfoack;
        int			code;

        code = -1;
	tinfoack.PRIM_type = T_INFO_REQ;
	if (tli_ioctl(fd, TI_GETINFO, (char *)&tinfoack, sizeof(tinfoack)) == -1)
		goto rtn;
	info->addr = tinfoack.ADDR_size;
	info->options = tinfoack.OPT_size;
	info->tsdu = tinfoack.TSDU_size;
	info->etsdu = tinfoack.ETSDU_size;
	info->connect = tinfoack.CDATA_size;
	info->discon = tinfoack.DDATA_size;
	info->servtype = tinfoack.SERV_type;
        code = 0;
rtn:
#ifdef XTIDBG
	tr_getinfo (fd, info, code);
#endif
	return code;
}

int
tli_ioctl (
	int			fd,
	int			cmd,
	char *			dp,
	int			len)
{
	int			ret;
	struct strioctl		stri;

	stri.ic_cmd = cmd;
	stri.ic_timout = -1;
	stri.ic_len = len;
	stri.ic_dp = dp;
	ret = stream_ioctl(fd, I_STR, (char *)&stri);
	if (ret == -1) {
		t_unix_to_tli_error();
		return -1;
	}
	if (ret != 0) {
		_Set_terrno(ret & 0xff);
		_Seterrno(ret >> 8);
		return -1;
	}
	return 0;
}
