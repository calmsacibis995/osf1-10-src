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
/*-
 * mibstore.h --
 *	Header file for users of the MI backing-store scheme.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *	"$XConsortium: mibstore.h,v 5.1 91/05/10 17:39:33 keith Exp $ SPRITE (Berkeley)"
 */

#ifndef _MIBSTORE_H
#define _MIBSTORE_H

typedef struct {
    void	    (*SaveAreas)(/* pPixmap, pObscured, x, y, pWin */);
    void	    (*RestoreAreas)(/* pPixmap, pExposed, x, y, pWin */);
    void	    (*SetClipmaskRgn)(/* pGC, pRegion */);
    PixmapPtr	    (*GetImagePixmap)(); /* unused */
    PixmapPtr	    (*GetSpansPixmap)(); /* unused */
} miBSFuncRec, *miBSFuncPtr;

extern void miInitBackingStore();

#endif /* _MIBSTORE_H */
