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
static char     *sccsid = "@(#)$RCSfile: ISO8859-1_ct.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/06/28 23:01:31 $";
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
 * 1.4  com/cmd/iconv/fold/ISO8859-1_ct.c, cmdiconv, bos320, 9130320 7/16/91 03:06:1
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "iconv_local.h"	/* from usr/ccs/lib/libiconv */
#include "iconvP.h"		/* from usr/ccs/lib/libiconv */
#include "fcs.h"		/* from usr/ccs/lib/libiconv */
#include "fold.h"

static iconv_rec	invalid;

static int	_iconv_exec(_LC_fold_iconv_t *cd, 
	unsigned char** inbuf, size_t *inbytesleft, 
	unsigned char** outbuf, size_t *outbytesleft)
{
	unsigned char 	*in;		/* point inbut buffer */
	unsigned char 	*out;		/* point output buffer */
	unsigned char	*ptr;
	size_t	inlen, outlen;
	int	ret_value;
	int	idx;
	size_t	len;
	EscTbl	*etbl;
	unsigned char	*pre_inbuf = NULL;

	if (!cd){
		errno = EBADF; return 0;
	}
	if (!inbuf) {
		cd->curcd = NULL;
		return ICONV_DONE;
	}

	etbl = cd->ett->etbl;
	while (1) {
		if (cd->curcd) {
			if ((ret_value = iconv(cd->curcd,
				(char **)inbuf, inbytesleft, (char **)outbuf, outbytesleft)) !=	
				ICONV_INVAL)
				return ret_value;
		}
		else {
			if ((ret_value = _ascii_exec(inbuf, inbytesleft,
				outbuf, outbytesleft)) != ICONV_INVAL)
				return ret_value;
		}
		if (*inbuf == pre_inbuf){ 		
			errno = EILSEQ; return ICONV_INVAL;
		}
		pre_inbuf = *inbuf;
		idx = (*cd->ett->csidx)(*inbuf, *inbytesleft);
		if (idx == NEEDMORE){
			errno = EINVAL; return ICONV_TRUNC;
		}
		if (idx == INVALIDCSID){
			errno = EILSEQ; return ICONV_INVAL;
		}
		if (idx == CONTROLCSID) {
			if (*outbytesleft == 0){
				errno = E2BIG; return ICONV_OVER;
			}
			*(*outbuf)++ = *(*inbuf)++;
			(*inbytesleft)--;
			(*outbytesleft)--;
			continue;
		}
		len = etbl[idx].len;
		if (*outbytesleft < len){
			errno = E2BIG; return ICONV_OVER;
		}
		if (!etbl[idx].seg) {
			memcpy(*outbuf, etbl[idx].str, len);
			*outbuf += len;
			*outbytesleft -= len;
			cd->curcd = cd->cds[idx];
			continue;
		}
		len += etbl[idx].seglen + 2;
		while (1) {
			if (*outbytesleft < len){
				errno = E2BIG; return ICONV_OVER;
			}
			in = *inbuf;
			inlen = *inbytesleft;
			out = *outbuf + len;
			outlen = *outbytesleft - len;
			if (outlen > 0x3fff - etbl[idx].seglen)
				outlen = 0x3fff - etbl[idx].seglen;
			ret_value = iconv(cd->cds[idx], (char **)&in, &inlen,
				(char **)&out, &outlen);
			if (in == *inbuf) {
				errno = EILSEQ; return ICONV_INVAL;
			}
			if (ret_value == ICONV_TRUNC){
				errno = EINVAL; return ret_value;
			}
			ptr = *outbuf;
			memcpy(ptr, etbl[idx].str, etbl[idx].len);
			ptr += etbl[idx].len + 2;
			ptr[-2] = (out - ptr) / 128 | 0x80;
			ptr[-1] = (out - ptr) % 128 | 0x80;
			memcpy(ptr, etbl[idx].seg, etbl[idx].seglen);
			*inbuf = in;
			*inbytesleft = inlen;
			ptr = *outbuf;
			*outbuf = out;
			*outbytesleft -= out - ptr;
			if (ret_value == ICONV_DONE)
				return ret_value;
			if (ret_value == ICONV_INVAL)
				break;
		}
	}
}

static void	_iconv_close(_LC_fold_iconv_t *cd)
{
	int	i;

	i = cd->ncds;
	while (i--)
		if (cd->cds[i])
			iconv_close(cd->cds[i]);
	if (cd)
		free(cd);
	else
		errno = EBADF;
}

static _LC_fold_iconv_t	*init(_LC_core_iconv_t *core_cd, 
				 char* toname, char* fromname)
{
        _LC_fold_iconv_t	*cd;
	int	i, j;
	EscTblTbl	*ett;

	if (strcmp("ct", toname) == 0)
		ett = _iconv_ct_ett;
	else if (strcmp("fold7", toname) == 0)
		ett = _iconv_fold7_ett;
	else if (strcmp("fold8", toname) == 0)
		ett = _iconv_fold8_ett;
	else
		return NULL;

	for (i = 0; 1; i++) {
		if (!ett[i].name)
			return NULL;
		if (strcmp(fromname, ett[i].name) == 0)
			break;
	}
		
        if (!(cd = (_LC_fold_iconv_t *)malloc(sizeof (_LC_fold_iconv_t) +
			ett[i].netbl * sizeof (iconv_t))))
                return NULL;

        cd->core = *core_cd;
	cd->ncds = ett[i].netbl;
	cd->cds = (iconv_t *)&((char *)cd)[sizeof (_LC_fold_iconv_t)];

	for (j = 0; j < cd->ncds; j++) {
		if (!(toname = ett[i].etbl[j].name))
			cd->cds[j] = NULL;
		else {
			if (!(cd->cds[j] = iconv_open(toname, fromname))) {
				while (j--)
					if (cd->cds[j])
						iconv_close(cd->cds[j]);
				return NULL;
			}
		}
	}

	cd->ett = &ett[i];
	cd->gl = cd->defgl = cd->cds[ett[i].defgl];
	cd->defgr = cd->cds[ett[i].defgr];
	if (cd->defgl == cd->defgr)
		cd->defgr = &invalid;
	cd->gr = cd->defgr;
	cd->curcd = cd->gl;
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
