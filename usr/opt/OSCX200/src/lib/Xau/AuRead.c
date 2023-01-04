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
/*
 * Xau - X Authorization Database Library
 *
 * $XConsortium: AuRead.c,v 1.5 91/01/08 15:09:31 gildea Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xauth.h>

static
read_short (shortp, file)
unsigned short	*shortp;
FILE		*file;
{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}

static
read_counted_string (countp, stringp, file)
unsigned short	*countp;
char	**stringp;
FILE	*file;
{
    unsigned short  len;
    char	    *data, *malloc ();

    if (read_short (&len, file) == 0)
	return 0;
    if (len == 0) {
	data = 0;
    } else {
    	data = malloc ((unsigned) len);
    	if (!data)
	    return 0;
    	if (fread (data, (int) sizeof (char), (int) len, file) != len) {
	    free (data);
	    return 0;
    	}
    }
    *stringp = data;
    *countp = len;
    return 1;
}

Xauth *
XauReadAuth (auth_file)
FILE	*auth_file;
{
    Xauth   local;
    Xauth   *ret;
    char    *malloc ();

    if (read_short (&local.family, auth_file) == 0)
	return 0;
    if (read_counted_string (&local.address_length, &local.address, auth_file) == 0)
	return 0;
    if (read_counted_string (&local.number_length, &local.number, auth_file) == 0) {
	if (local.address) free (local.address);
	return 0;
    }
    if (read_counted_string (&local.name_length, &local.name, auth_file) == 0) {
	if (local.address) free (local.address);
	if (local.number) free (local.number);
	return 0;
    }
    if (read_counted_string (&local.data_length, &local.data, auth_file) == 0) {
	if (local.address) free (local.address);
	if (local.number) free (local.number);
	if (local.name) free (local.name);
	return 0;
    }
    ret = (Xauth *) malloc (sizeof (Xauth));
    if (!ret) {
	if (local.address) free (local.address);
	if (local.number) free (local.number);
	if (local.name) free (local.name);
	if (local.data) free (local.data);
	return 0;
    }
    *ret = local;
    return ret;
}
