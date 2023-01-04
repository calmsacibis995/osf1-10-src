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
static char     *sccsid = "@(#)$RCSfile: ISO8859-1-GR_ISO8859-1.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/06/28 23:02:19 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDICONV)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.3  com/cmd/iconv/fold_lower/ISO8859-1-GR_ISO8859-1.c, cmdiconv, bos320, 9130320 7/16/91 03:07:26
 */

#include <stdlib.h>
#include "iconv_local.h"	/* from usr/ccs/lib/libiconv */
#include "fold_lower.h"
#include "ISO8859-1-GR_ISO8859-1.h"

static int	_iconv_exec(_LC_fold_lower_iconv_t *cd, 
	unsigned char** inbuf, size_t *inbytesleft, 
	unsigned char** outbuf, size_t *outbytesleft)
{
	unsigned char 	c;
	unsigned char 	*in, *e_in;		/* point inbut buffer */
	unsigned char 	*out, *e_out;		/* point output buffer */
	int		ret_value;

	if (!cd){
		errno = EBADF; return 0;
	}
        if (!inbuf) return ICONV_DONE;

        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	ret_value = ICONV_DONE;

	while (in < e_in) {
		c = table[*in];
		if (!c) {
			errno = EILSEQ; ret_value = ICONV_INVAL;
			break;
		}
		if (out >= e_out) {
			errno = E2BIG; ret_value = ICONV_OVER;
			break;
		}
		in++;
		*out++ = c;
	}
	*inbytesleft = e_in - in;
	*outbytesleft = e_out - out;
	*inbuf = in;
	*outbuf = out;
	return ret_value;
}

static void	_iconv_close(iconv_t cd)
{
	if (cd)
		free(cd);
	else
		errno = EBADF;
}

static _LC_fold_lower_iconv_t	*init (_LC_core_iconv_t *core_cd, 
				 char* toname, char* fromname)
{
        _LC_fold_lower_iconv_t	*cd;

        if (!(cd = (_LC_fold_lower_iconv_t *) malloc(sizeof(_LC_fold_lower_iconv_t))))
                return (NULL);
        cd->core = *core_cd;
        return cd;
}

_LC_core_iconv_t	*instantiate(void)
{
        static _LC_core_iconv_t	cd;

	cd.hdr.magic = _LC_MAGIC;
	cd.hdr.version = _LC_VERSION;
	cd.hdr.type_id = _LC_ICONV;
	cd.hdr.size = sizeof (_LC_core_iconv_t);
        cd.init = (_LC_core_iconv_t *(*)())init;
        cd.exec = _iconv_exec;
        cd.close = _iconv_close;
        return &cd;
}
