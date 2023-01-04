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
static char	*sccsid = "@(#)$RCSfile: iconv.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1994/01/20 03:25:47 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBICONV)
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
 * 1.4  com/lib/iconv/iconv.c, libiconv, bos320, 9130320t 7/27/91 18:09:21
 */

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/localedef.h>
#include <string.h>
#define _ICONV_INTERNAL
#include "iconv_local.h"
#include "iconvTable.h"


/*
 * NAME: _iconvTable_exec
 *
 * FUNCTION:
 *      This function performs the one-to-one table conversion by a simple
 *	table lookup. This table conversion is confined in a state-independent
 *	character to character conversion by indexing the conversion table
 *	array with the codepoint from the source codeset.
 *
 * RETURN VALUE DESCRIPTION:
 *      (a) If all character from input buffer are successfully converted
 *          and placed into the output buffer, return ICONV_DONE.
 *      (b) If there is no room in the output buffer to place the converted
 *          character, the converted characters is not placed in the output
 *          buffer and the contents of inbuf points to the start of the
 *          character sequence that caused the output buffer overflow,
 *          and returns ICONV_OVER.
 */

static int
_iconvTable_exec(iconvTable_t *cd,
		unsigned char** inbuf, size_t *inbytesleft,
		unsigned char** outbuf, size_t *outbytesleft)
{
	int	left;		
	int	counter;
	unsigned char	*in, *out;
	unsigned char	*table;

	if (!cd){			/* Chech if a converter descripter */
		errno = EBADF;
		return (NULL);		/* is valid.  If not, return error */
	}
        if (!inbuf) return ICONV_DONE;

	in = *inbuf;
	out = *outbuf;
	table = &cd->table.data[0];
	left = *outbytesleft - *inbytesleft;
	counter = left >= 0 ? *inbytesleft : *outbytesleft;

	*inbuf += counter;
	*outbuf += counter;

	while (counter--)		/* Characters are converted here */
		out[counter] = table[in[counter]];

	if (left >= 0) {		/* Return results of conversion */
		*inbytesleft = 0;
		*outbytesleft = left;
		return (ICONV_DONE);
	}
	else {
		*inbytesleft = - left;
		*outbytesleft = 0;
		errno = E2BIG;
		return (ICONV_OVER);
	}
}

/*
 * NAME: _iconvTable_Lower_exec
 *
 * FUNCTION:
 *      This function performs the one-to-one table conversion by a simple
 *	table lookup. This table conversion is confined in a state-independent
 *	character to character conversion by indexing the conversion table
 *	array with the codepoint from the source codeset.
 *
 * RETURN VALUE DESCRIPTION:
 *      (a) If all character from input buffer are successfully converted
 *          and placed into the output buffer, return ICONV_DONE.
 *      (b) If an input character does not belong to the "converted from"
 *          character set, no conversion is attempted, the content of inbuf
 *          points to the start of the unindentified input character, and
 *          returns ICONV_INVAL.
 *      (c) If there is no room in the output buffer to place the converted
 *          character, the converted characters is not placed in the output
 *          buffer and the contents of inbuf points to the start of the
 *          character sequence that caused the output buffer overflow,
 *          and returns ICONV_OVER.
 */

static int
_iconvTable_Lower_exec(iconvTable_t *cd,
		unsigned char** inbuf, size_t *inbytesleft,
		unsigned char** outbuf, size_t *outbytesleft)
{
	unsigned char	*table;
	unsigned char 	c, invalid_data;
	unsigned char 	*in, *e_in;		/* point inbut buffer */
	unsigned char 	*out, *e_out;		/* point output buffer */
	int		ret_value;


	if (!cd){			/* Chech if a converter descripter */
		errno = EBADF;
		return NULL;		/* is valid.  If not, return error */
	}
        if (!inbuf) return ICONV_DONE;

        e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;
	table = &cd->table.data[0];
	ret_value = ICONV_DONE;

	invalid_data = cd->table.inval_char;
	while (in < e_in) {
		c = table[*in];
		if (c == invalid_data) {
			errno = EILSEQ;
			ret_value = ICONV_INVAL;
			break;
		}
		if (out >= e_out) {
			errno = E2BIG;
			ret_value = ICONV_OVER;
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

/*
 * NAME: _iconvTable_close
 *
 * FUNCTION:
 *      This is the standard close function that frees the memory that
 *      cd points to.
 *
 * RETURN VALUE DESCRIPTION:
 *      No errors are defined.
 */

static void
_iconvTable_close(iconvTable_t *cd)
{
	if (cd)
		free(cd);
	else
		errno = EBADF;
}

/*
 * NAME: CopyCoreHdr
 *
 * FUNCTION:
 *	initialize the '_LC_object_t' portion of a converter discripter
 *	for the table converter
 *
 * RETURN VALUE DESCRIPTION:
 *	None
 */
static void
CopyCoreHdr(_LC_object_t *header)
{
	header->magic = _LC_MAGIC;
	header->version = _LC_VERSION;
	header->type_id = _LC_ICONV;
	header->size = sizeof(_LC_core_iconv_t);
}

/*
 * NAME: nextpath
 *
 * FUNCTION:
 *	Get the next path element from the given locpath.
 *	The result followed by '/' is stored in the buffer,
 *	and locpath pointer is advanced.
 *
 * RETURN VALUE DESCRIPTION:
 *	Length of the next path element.
 */
static int
nextpath(char **locpath, char *buf)
{
	char	*locptr;
	int	len;

	locptr = *locpath;
	while (*locptr && *locptr != ':') locptr++;
	len = locptr - *locpath;
	if (len > _POSIX_PATH_MAX - 1)
		return -1;
	if (len == 0) {
		locptr++;
		buf[0] = '.';
		len = 1;
	}
	else {
		if (*locptr && !*++locptr)
			locptr--;
		(void)memcpy(buf, *locpath, len);
	}
	buf[len++] = '/';
	*locpath = locptr;
	return len;
}


iconv_t	
__iconv_open(const char *t, const char *f)
/* t : "to" codeset name */
/* f : "from" codeset name */
{
	iconv_t	cd;
	int	fd;
	char	*locpath, *def_path;
	int	len, pathlen, mtd_filelen, tbl_filelen;
	char	*fn, *mtd_name, *tbl_name;
	_LC_core_iconv_t	*core_cd;
	iconvTable_t 	*tbl_cd = 0;
	char	namebuf[_POSIX_PATH_MAX];

	/*
	 *	MAKE CONVERTER/TABLE NAME
	 *
	 * Construct a filename concatinating "from" and "to".
	 *	"/iconv/from_to"	for method converter
	 *	"/iconvTable/from_to"	for table converter data
	 */
	len = strlen(f) + strlen(t);
	mtd_filelen = len + 9;
	tbl_filelen = len + 14;
	if (!(mtd_name = (char *)malloc(mtd_filelen))){
		errno = ENOMEM;
		return NULL;
	}
	(void)strcpy(mtd_name, "/iconv/");
	(void)strcat(mtd_name, f);
	(void)strcat(mtd_name, "_");	/* codeset name separator */
	(void)strcat(mtd_name, t);

	if (!(tbl_name = (char *)malloc(tbl_filelen))) {
		free(mtd_name);
		errno = ENOMEM;
		return NULL;
	}
	(void)strcpy(tbl_name, "/iconvTable/");
	(void)strcat(tbl_name, &mtd_name[7]);

	/*
	 *	Check if privileged program, and set an appropreate search
	 *	path to find a file which will be loaded.
	 */
	def_path = locpath = NULL;
	if (!__ispriv())
		locpath = getenv("LOCPATH");
	if (!locpath || !*locpath)
		def_path = locpath = DEFAULTPATH;

	while (1) {
		while(*locpath) {
			/*
			 *	Get the next element in locpath
			 */
			pathlen = nextpath(&locpath, namebuf);
			if (pathlen < 0 || _POSIX_PATH_MAX < pathlen + 
				mtd_filelen) {
				errno = ENAMETOOLONG;
				return NULL;
			}
			/*
			 *	Check if method converter is available
			 */
			(void)strcpy(namebuf + pathlen, mtd_name);
			core_cd = (_LC_core_iconv_t *)__lc_load(namebuf, 0);
			if (core_cd) {
				cd = (iconv_t)((*(core_cd)->init)
							(core_cd, t, f));
				if (cd != NULL) {
					free(mtd_name);
					free(tbl_name);
					return cd;	/* Happy End */
				}
			}
			/*
			 *	Check if table data is available
			 */ 
			if (_POSIX_PATH_MAX < pathlen + tbl_filelen) {
				errno = ENAMETOOLONG;
				return NULL;
			}
			(void)strcpy(namebuf + pathlen, tbl_name);
			if ((fd = open(namebuf, O_RDONLY)) < 0)
				continue;
			if (!tbl_cd) {
				if (!(tbl_cd = (iconvTable_t *)malloc(sizeof
					(iconvTable_t)))) {
					close(fd);
					continue;
				}
			}
			if (read(fd, (char*)&tbl_cd->table, sizeof (IconvTable)) !=
				sizeof (IconvTable)) {
				close(fd);
				free (tbl_cd);
				continue;
			}
			close(fd);
			if (tbl_cd->table.magic != ICONV_REL1_MAGIC &&
				tbl_cd->table.magic != ICONV_REL2_MAGIC) {
				continue;
			}

			/*
			 *	SET THE TABLE CONVERTER
			 */
			(void)CopyCoreHdr((_LC_object_t *)&tbl_cd->core.hdr);
			tbl_cd->core.init = NULL;
			if (tbl_cd->table.magic == ICONV_REL2_MAGIC && 
				tbl_cd->table.inval_handle == TRUE)
				tbl_cd->core.exec = 
					(int (*)())_iconvTable_Lower_exec;
			else 
				tbl_cd->core.exec = (int (*)())_iconvTable_exec;
			tbl_cd->core.close = (void (*)())_iconvTable_close;
			free(mtd_name);
			free(tbl_name);
			return ((iconv_t)tbl_cd);
		}
		if (!def_path)
			def_path = locpath = DEFAULTPATH;
		else
			break;
	}
	/*
	 *	NOT FOUND ANYWHERE
	 */
	free(mtd_name);
	free(tbl_name);
	errno = EINVAL;
	return NULL;
}

iconv_t	
iconv_open(const char *t, const char *f)
{
	iconv_t	cd;

	/*
	 * If NULL is returned from __iconv_open(), return -1.
	 */

	return ((cd = __iconv_open(t, f)) ? cd : (iconv_t)(-1));
}


size_t	
iconv(iconv_t cd, char **ibuf, size_t *ilen, char **obuf, size_t *olen)
{
        size_t retval;

	/*
	 * Whatever *(cd)->exec returns, this function returns *ilen which 
	 * holds the number of bytes left to be converted.  If converter 
	 * descriptor is invalid, it sets the errno.
	 */ 	
	if (cd && cd != (iconv_t)-1) {
	    if ((*(cd)->exec)(cd, ibuf, ilen, obuf, olen) != ICONV_DONE)
	        retval = -1;
	    else
	        retval = *ilen;
	} else {
	    errno = EBADF;
	    retval = -1;
	}
	return retval;
}

int
iconv_close(iconv_t cd)
{
	if (cd && cd != (iconv_t)-1)
		(*(cd)->close)(cd);
	else {
		errno = EBADF;
		return(-1);
	}

	return (0);
}

