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
 * $XConsortium: sharedlib.c,v 1.6 91/07/31 21:12:30 keith Exp $
 * 
 * Copyright 1991 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 */

#if defined(SUNSHLIB) && !defined(SHAREDCODE)

#include "Atoms.h"

struct _AtomRec {
    char *name;
    struct _DisplayRec* head;
};

#if __STDC__ && !defined(UNIXCPP)
#define DeclareAtom(atom) \
extern struct _AtomRec __##atom; \
AtomPtr _##atom = &__##atom;
#else
#define DeclareAtom(atom) \
extern struct _AtomRec __/**/atom; \
AtomPtr _/**/atom = &__/**/atom;
#endif

DeclareAtom(XA_ATOM_PAIR)
DeclareAtom(XA_CHARACTER_POSITION)
DeclareAtom(XA_CLASS)
DeclareAtom(XA_CLIENT_WINDOW)
DeclareAtom(XA_CLIPBOARD)
DeclareAtom(XA_COMPOUND_TEXT)
DeclareAtom(XA_DECNET_ADDRESS)
DeclareAtom(XA_DELETE)
DeclareAtom(XA_FILENAME)
DeclareAtom(XA_HOSTNAME)
DeclareAtom(XA_IP_ADDRESS)
DeclareAtom(XA_LENGTH)
DeclareAtom(XA_LIST_LENGTH)
DeclareAtom(XA_NAME)
DeclareAtom(XA_NET_ADDRESS)
DeclareAtom(XA_NULL)
DeclareAtom(XA_OWNER_OS)
DeclareAtom(XA_SPAN)
DeclareAtom(XA_TARGETS)
DeclareAtom(XA_TEXT)
DeclareAtom(XA_TIMESTAMP)
DeclareAtom(XA_USER)

#endif /* SUNSHLIB */
