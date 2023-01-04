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
 * xdm - display manager daemon
 *
 * $XConsortium: xdmauth.c,v 1.9 92/05/19 17:27:04 keith Exp $
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

/*
 * xdmauth
 *
 * generate authorization data for XDM-AUTHORIZATION-1 as per XDMCP spec
 */

#include "dm.h"

#ifdef HASXDMAUTH

static char	auth_name[256];
static int	auth_name_len;

XdmPrintDataHex (s, a, l)
    char	    *s;
    char	    *a;
    int		    l;
{
    int	i;

    Debug ("%s", s);
    for (i = 0; i < l; i++)
	Debug (" %02x", a[i] & 0xff);
    Debug ("\n");
}

#ifdef notdef			/* not used */
XdmPrintKey (s, k)
    char	    *s;
    XdmAuthKeyRec   *k;
{
    XdmPrintDataHex (s, (char *) k->data, 8);
}
#endif

#ifdef XDMCP
XdmPrintArray8Hex (s, a)
    char	*s;
    ARRAY8Ptr	a;
{
    XdmPrintDataHex (s, (char *) a->data, a->length);
}
#endif

XdmInitAuth (name_len, name)
    unsigned short  name_len;
    char	    *name;
{
    if (name_len > 256)
	name_len = 256;
    auth_name_len = name_len;
    bcopy (name, auth_name, name_len);
}

/*
 * Generate authorization for XDM-AUTHORIZATION-1 
 *
 * When being used with XDMCP, 8 bytes are generated for the session key
 * (sigma), as the random number (rho) is already shared between xdm and
 * the server.  Otherwise, we'll prepend a random number to pass in the file
 * between xdm and the server (16 bytes total)
 */

Xauth *
XdmGetAuthHelper (namelen, name, includeRho)
    unsigned short  namelen;
    char	    *name;
    int	    includeRho;
{
    Xauth   *new;
    new = (Xauth *) malloc (sizeof (Xauth));

    if (!new)
	return (Xauth *) 0;
    new->family = FamilyWild;
    new->address_length = 0;
    new->address = 0;
    new->number_length = 0;
    new->number = 0;
    if (includeRho)
	new->data_length = 16;
    else
	new->data_length = 8;

    new->data = (char *) malloc (new->data_length);
    if (!new->data)
    {
	free ((char *) new);
	return (Xauth *) 0;
    }
    new->name = (char *) malloc (namelen);
    if (!new->name)
    {
	free ((char *) new->data);
	free ((char *) new);
	return (Xauth *) 0;
    }
    bcopy (name, (char *)new->name, namelen);
    new->name_length = namelen;
    GenerateAuthorization ((char *)new->data, new->data_length);
    /*
     * set the first byte of the session key to zero as it
     * is a DES key and only uses 56 bits
     */
    ((char *)new->data)[new->data_length - 8] = '\0';
    XdmPrintDataHex ("Local server auth", (char *)new->data, new->data_length);
    return new;
}

Xauth *
XdmGetAuth (namelen, name)
    unsigned short  namelen;
    char	    *name;
{
    return XdmGetAuthHelper (namelen, name, TRUE);
}

#ifdef XDMCP

XdmGetXdmcpAuth (pdpy,authorizationNameLen, authorizationName)
    struct protoDisplay	*pdpy;
    unsigned short	authorizationNameLen;
    char		*authorizationName;
{
    Xauth   *fileauth, *xdmcpauth;

    if (pdpy->fileAuthorization && pdpy->xdmcpAuthorization)
	return;
    xdmcpauth = XdmGetAuthHelper (authorizationNameLen, authorizationName, FALSE);
    if (!xdmcpauth)
	return;
    fileauth = (Xauth *) malloc (sizeof (Xauth));
    if (!fileauth)
    {
	XauDisposeAuth(xdmcpauth);
	return;
    }
    /* build the file auth from the XDMCP auth */
    *fileauth = *xdmcpauth;
    fileauth->name = malloc (xdmcpauth->name_length);
    fileauth->data = malloc (16);
    fileauth->data_length = 16;
    if (!fileauth->name || !fileauth->data)
    {
	XauDisposeAuth (xdmcpauth);
	if (fileauth->name)
	    free ((char *) fileauth->name);
	if (fileauth->data)
	    free ((char *) fileauth->data);
	free ((char *) fileauth);
	return;
    }
    /*
     * for the file authorization, prepend the random number (rho)
     * which is simply the number we've been passing back and
     * forth via XDMCP
     */
    bcopy (xdmcpauth->name, fileauth->name, xdmcpauth->name_length);
    bcopy (pdpy->authenticationData.data, fileauth->data, 8);
    bcopy (xdmcpauth->data, fileauth->data + 8, 8);
    XdmPrintDataHex ("Accept packet auth", xdmcpauth->data, xdmcpauth->data_length);
    XdmPrintDataHex ("Auth file auth", fileauth->data, fileauth->data_length);
    /* encrypt the session key for its trip back to the server */
    XdmcpWrap (xdmcpauth->data, &pdpy->key, xdmcpauth->data, 8);
    pdpy->fileAuthorization = fileauth;
    pdpy->xdmcpAuthorization = xdmcpauth;
}

#define atox(c)	('0' <= c && c <= '9' ? c - '0' : \
		 'a' <= c && c <= 'f' ? c - 'a' + 10 : \
		 'A' <= c && c <= 'F' ? c - 'A' + 10 : -1)

static
HexToBinary (key)
    char    *key;
{
    char    *out, *in;
    int	    top, bottom;

    in = key + 2;
    out= key;
    while (in[0] && in[1])
    {
	top = atox(in[0]);
	if (top == -1)
	    return 0;
	bottom = atox(in[1]);
	if (bottom == -1)
	    return 0;
	*out++ = (top << 4) | bottom;
	in += 2;
    }
    if (in[0])
	return 0;
    *out++ = '\0';
    return 1;
}

/*
 * Search the Keys file for the entry matching this display.  This
 * routine accepts either plain ascii strings for keys, or hex-encoded numbers
 */

XdmGetKey (pdpy, displayID)
    struct protoDisplay	*pdpy;
    ARRAY8Ptr		displayID;
{
    FILE    *keys;
    char    line[1024], id[1024], key[1024];
    int	    keylen;

    Debug ("Lookup key for %*.*s\n", displayID->length, displayID->length, displayID->data);
    keys = fopen (keyFile, "r");
    if (!keys)
	return FALSE;
    while (fgets (line, sizeof (line) -  1, keys))
    {
	if (line[0] == '#' || sscanf (line, "%s %s", id, key) != 2)
	    continue;
	Debug ("Key entry \"%s\" \"%s\"\n", id, key);
	if (strlen (id) == displayID->length &&
	    !strncmp (id, (char *)displayID->data, displayID->length))
	{
	    if (!strncmp (key, "0x", 2) || !strncmp (key, "0X", 2))
		if (!HexToBinary (key))
		    break;
	    keylen = strlen (key);
	    while (keylen < 7)
		key[keylen++] = '\0';
	    pdpy->key.data[0] = '\0';
	    bcopy (key, pdpy->key.data + 1, 7);
	    fclose (keys);
	    return TRUE;
	}
    }
    fclose (keys);
    return FALSE;
}

/*ARGSUSED*/
XdmCheckAuthentication (pdpy, displayID, authenticationName, authenticationData)
    struct protoDisplay	*pdpy;
    ARRAY8Ptr		displayID, authenticationName, authenticationData;
{
    XdmAuthKeyPtr   incoming;

    if (!XdmGetKey (pdpy, displayID))
	return FALSE;
    if (authenticationData->length != 8)
	return FALSE;
    XdmcpUnwrap (authenticationData->data, &pdpy->key,
		  authenticationData->data, 8);
    XdmPrintArray8Hex ("Request packet auth", authenticationData);
    if (!XdmcpCopyARRAY8(authenticationData, &pdpy->authenticationData))
	return FALSE;
    incoming = (XdmAuthKeyPtr) authenticationData->data;
    XdmcpIncrementKey (incoming);
    XdmcpWrap (authenticationData->data, &pdpy->key,
		  authenticationData->data, 8);
    return TRUE;
}

#endif /* XDMCP */
#endif /* HASXDMAUTH (covering the entire file) */
