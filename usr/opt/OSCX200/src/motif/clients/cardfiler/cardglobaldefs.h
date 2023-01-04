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
 * Removed OSF test and illegal initialization of BuildSystemHeader
 */
/*
************************************************************************** 
**                   DIGITAL EQUIPMENT CORPORATION                      ** 
**                         CONFIDENTIAL                                 ** 
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   ** 
************************************************************************** 
*/
/* $Id: cardglobaldefs.h,v 1.1.2.3 92/12/11 08:29:55 devrcs Exp $ */
/*
**++

  Copyright (c) Digital Equipment Corporation, 
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.
 
**--
**/

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <stdio.h>
#include <Xm/Xm.h>
#include <DXm/DXmPrint.h>
#include <X11/cursorfont.h>
#include <X11/Vendor.h>
#include <Mrm/MrmAppl.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifdef VMS
#include <decw$cursor.h>
#include <ssdef>
#include <descrip>
#else
#include <X11/decwcursor.h>
#endif

#ifndef NO_XNLS
#ifdef VMS
#include <xnl$def.h>
#else
#include <xnl_def.h>
#endif
#endif

#include "carddefines.h"
#ifndef NO_MEMEX
#include <lwk_def.h>			/* MEMEX */

/* HyperInformation Support */
typedef struct _SurrogateList {
    struct _SurrogateList *next;
    lwk_surrogate surrogate;
} SurrogateList;			/* MEMEX */
#endif

/* Declare the data structures for the card and pointers to them. 
   This is mainly for the internal doubly-linked list of cards. */

typedef struct card {
    char index[INDEX_LENGTH + 1];
    struct card *next;
    struct card *last;
    FILE *fp, *orig_fp;
    int offset, orig_offset, temp_offset;
    int index_number;
    unsigned short reserved;
    unsigned int card_id;		/* MEMEX */
#ifndef NO_MEMEX
    SurrogateList *surrogates;		/* MEMEX */
#endif
    Boolean highlighted;		/* MEMEX */
    Boolean highlighted_changed;
    Boolean svngetentrycalled;
} card_instance;

typedef struct card(*card_pointer);


/* The data structure to hold all of the contents of the card. 
   This is mainly for storage and reading purposes. */
typedef struct card_contents_rec {
    char index[INDEX_LENGTH + 1];
    char text[TEXT_LENGTH + 1];
    char bitmap[BITMAP_SIZE];
    unsigned short reserved;
    unsigned int card_id;		/* MEMEX */
    Dimension bitmap_height;
    Dimension bitmap_width;
} CardContentsRec;

typedef struct card_contents_rec(*card_contents);
