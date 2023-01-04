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
#if defined(OSF1) || defined(__osf__)
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Id: cardmain.c,v 1.1.4.4 1993/10/19 19:55:41 Susan_Ng Exp $";
#endif
#endif
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

/*
**++
**  MODULE NAME:
**	cardmain.c
**
**  FACILITY:
**      OOTB Cardfiler
**
**  ABSTRACT:
**	Currently contains most of the routines for the Cardfiler.
**
**  AUTHORS:
**      Paul Reilly, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     27-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/

#include "cardglobaldefs.h"
#include <stdio.h>
#include <stdlib.h>

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>
#include <Xm/FileSB.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <DXm/DXmHelpB.h>
#include <DXm/DXmSvn.h>
#include <DXm/DXmCSText.h>
#include <DXm/DXmCSTextP.h>
#include <X11/DECWmHints.h>
#include <DXm/DECspecific.h>
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#include <dwi18n_lib.h>
#ifdef VMS
#include <fab.h>
#include <nam.h>
#else
#include <pwd.h>
#include <unistd.h>
#include <strings.h>
#ifndef NO_HYPERHELP
#include <DXm/bkr_api.h>
#endif
#endif

#ifdef VMS
static void find_file();
#else
static char *createHomeDirSpec();
#endif

/* routine to test if the XUI WM is running. */
extern Boolean DXIsXUIWMRunning();

/* routines used for supporting multiple size icon. */
void set_icons();
static void set_icons_on_shell();
static void set_iconify_pixmap();
static char *get_icon_index_name();
Pixmap fetch_bitmap_icon();

static void tracking_help();
void displaycardbyid();
void svn_make_entry_visible();
void clearstoreaction();
void clearaction();
void exitsaveaction();
void exitaction();
void enterfnameaction();
void deleteaction();
void mergeaction();
void open_caution_action();
void retrieveaction();
void readgraphic();
void readgraphicaction();
void renameindex();
int saveaction();
void expand_filespec();
void DisplayHelp();

extern int display_message();
extern int setscroll();
extern void gray_no_cards();
extern void displaycardfromindex();

extern Cursor WorkCursorCreate();
extern void WorkCursorDisplay();
extern void WorkCursorRemove();

static char CARDFILER_75_bits[] = {
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x54, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfe, 0xff, 0xa8, 0x02, 0x55, 0x55,
   0x55, 0x00, 0x00, 0x00, 0x57, 0x55, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xff,
   0xff, 0xff, 0xab, 0xaa, 0xa8, 0x02, 0x55, 0x55, 0xd5, 0x55, 0x55, 0x05,
   0x00, 0x40, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0xff, 0xa7,
   0xa8, 0x02, 0x55, 0x55, 0x01, 0x00, 0x00, 0x5c, 0x55, 0x45, 0x54, 0x05,
   0xaa, 0xaa, 0xfe, 0xff, 0xff, 0xaf, 0xaa, 0xa2, 0xa8, 0x02, 0x55, 0x55,
   0x57, 0x55, 0x15, 0x00, 0x00, 0x45, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xca, 0xff, 0x3f, 0xa2, 0xa8, 0x02, 0x55, 0x15, 0x00, 0x00, 0x60, 0x55,
   0x15, 0x45, 0x54, 0x05, 0xaa, 0xea, 0xff, 0xff, 0xbf, 0xaa, 0x2a, 0xa2,
   0xa8, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55, 0x15, 0x45, 0x54, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2, 0xa8, 0x02, 0x55, 0x75,
   0x55, 0x55, 0x55, 0x55, 0x15, 0x45, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x2a, 0xa2, 0xa8, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55,
   0x15, 0x45, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2,
   0xa8, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55, 0x15, 0x45, 0x54, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2, 0xa8, 0x02, 0x55, 0x75,
   0x55, 0x55, 0x55, 0x55, 0x15, 0x45, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x2a, 0xa2, 0xa8, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55,
   0x15, 0x45, 0x54, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2,
   0xa8, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55, 0x15, 0x45, 0x40, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2, 0x80, 0x02, 0x55, 0x75,
   0x55, 0x55, 0x55, 0x55, 0x15, 0x05, 0x14, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x2a, 0x02, 0x9a, 0x02, 0x55, 0x75, 0x55, 0x55, 0x55, 0x55,
   0x15, 0x45, 0x1d, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0xa2,
   0x8e, 0x02, 0x55, 0x75, 0x55, 0x57, 0x55, 0x75, 0x15, 0x45, 0x17, 0x05,
   0xaa, 0xaa, 0xaa, 0xaf, 0xaa, 0xfa, 0x2a, 0xa0, 0x8b, 0x02, 0x55, 0x75,
   0xd5, 0x53, 0x55, 0x3d, 0x15, 0xd0, 0x45, 0x05, 0xaa, 0xaa, 0xea, 0xa9,
   0xaa, 0x9e, 0x2a, 0xea, 0xa2, 0x02, 0x55, 0x75, 0xf5, 0x54, 0x55, 0x4f,
   0x15, 0x75, 0x51, 0x05, 0xaa, 0x2a, 0x78, 0x00, 0x80, 0x07, 0x00, 0xba,
   0xa8, 0x02, 0x55, 0x15, 0x3c, 0x00, 0xc0, 0x03, 0x00, 0x5d, 0x54, 0x05,
   0xaa, 0xba, 0x9e, 0xaa, 0xea, 0xa9, 0xaa, 0x2e, 0xaa, 0x02, 0x55, 0x5d,
   0x4f, 0x55, 0xf5, 0x54, 0x55, 0x17, 0x55, 0x05, 0xaa, 0xae, 0xae, 0xaa,
   0xea, 0xaa, 0xaa, 0x8b, 0xaa, 0x02, 0x55, 0x57, 0x4f, 0x55, 0xf5, 0x54,
   0xd5, 0x45, 0x55, 0x05, 0xaa, 0xab, 0xae, 0xaa, 0xea, 0xaa, 0xea, 0xa2,
   0xaa, 0x02, 0xd5, 0x55, 0x4f, 0x55, 0xf5, 0x54, 0x75, 0x51, 0x55, 0x05,
   0xea, 0xaa, 0xae, 0xaa, 0xea, 0xaa, 0xba, 0xa8, 0xaa, 0x02, 0x75, 0x55,
   0x4f, 0x55, 0xf5, 0x54, 0x5d, 0x54, 0x55, 0x05, 0xba, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x2e, 0xaa, 0xaa, 0x02, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x17, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x8a, 0xaa,
   0xaa, 0x02, 0x5d, 0x55, 0x55, 0x55, 0x55, 0x55, 0x45, 0x55, 0x55, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa2, 0xaa, 0xaa, 0x02, 0x05, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x50, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02};

static char CARDFILER_50_bits[] = {
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x01, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x01, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0xa0, 0x02, 0x55, 0x55, 0x00, 0x00, 0xfe,
   0x57, 0x01, 0xaa, 0xaa, 0xff, 0xff, 0xab, 0xa2, 0x02, 0x55, 0x55, 0x55,
   0x15, 0x00, 0x54, 0x01, 0xaa, 0x0a, 0x00, 0xc0, 0xff, 0xa2, 0x02, 0x55,
   0xf5, 0xff, 0x7f, 0x55, 0x54, 0x01, 0xaa, 0xaa, 0xaa, 0x02, 0x80, 0xa2,
   0x02, 0x55, 0x05, 0x00, 0xf8, 0x5f, 0x54, 0x01, 0xaa, 0xfe, 0xff, 0xaf,
   0x8a, 0xa2, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x01, 0xaa, 0xae,
   0xaa, 0xaa, 0x8a, 0xa2, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x01,
   0xaa, 0xae, 0xaa, 0xaa, 0x8a, 0xa2, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x54, 0x01, 0xaa, 0xae, 0xaa, 0xaa, 0x8a, 0xa2, 0x02, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x54, 0x01, 0xaa, 0xae, 0xaa, 0xaa, 0x8a, 0xa2, 0x02, 0x55,
   0x55, 0x55, 0x55, 0x55, 0x04, 0x01, 0xaa, 0xae, 0xaa, 0xaa, 0x8a, 0xa2,
   0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x30, 0x01, 0xaa, 0xae, 0xaa, 0xaa,
   0x8a, 0xb8, 0x02, 0x55, 0x55, 0x57, 0x75, 0x55, 0x1c, 0x01, 0xaa, 0xae,
   0xab, 0xba, 0x0a, 0x8e, 0x02, 0x55, 0xc5, 0x01, 0x1c, 0x00, 0x47, 0x01,
   0xaa, 0xe2, 0x00, 0x0e, 0x80, 0xa3, 0x02, 0x55, 0x77, 0x54, 0x47, 0xd5,
   0x51, 0x01, 0xaa, 0x6b, 0xaa, 0xa6, 0xea, 0xa8, 0x02, 0xd5, 0x75, 0x55,
   0x57, 0x75, 0x54, 0x01, 0xea, 0x6a, 0xaa, 0xa6, 0x3a, 0xaa, 0x02, 0x75,
   0x75, 0x55, 0x57, 0x1d, 0x55, 0x01, 0xea, 0xff, 0xff, 0xff, 0x8f, 0xaa,
   0x02, 0x75, 0x55, 0x55, 0x55, 0x45, 0x55, 0x01, 0xaa, 0xaa, 0xaa, 0xaa,
   0xa2, 0xaa, 0x02, 0x15, 0x00, 0x00, 0x00, 0x50, 0x55, 0x01, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x01};

static char CARDFILER_32_bits[] = {
   0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa,
   0x55, 0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x05, 0x40,
   0xaa, 0xaa, 0xf2, 0x9f, 0x55, 0x01, 0x50, 0x55, 0xaa, 0xaa, 0xfc, 0x87,
   0x55, 0x00, 0x54, 0x55, 0xaa, 0x2a, 0xff, 0x81, 0x15, 0x00, 0x55, 0x55,
   0xfa, 0xff, 0xab, 0x80, 0x55, 0x55, 0x55, 0x55, 0xba, 0xaa, 0xaa, 0x80,
   0x55, 0x55, 0x55, 0x55, 0xba, 0xaa, 0xaa, 0x80, 0x55, 0x55, 0x55, 0x55,
   0xba, 0xaa, 0xaa, 0xa0, 0x55, 0x5d, 0x5d, 0x55, 0xba, 0xae, 0xae, 0x88,
   0x15, 0x07, 0x07, 0x45, 0xba, 0xab, 0x53, 0xa3, 0xdd, 0xd5, 0xa9, 0x51,
   0xae, 0xa9, 0xd5, 0xa8, 0xd7, 0xd5, 0x69, 0x54, 0xaa, 0xa9, 0x35, 0xaa,
   0xff, 0xff, 0x1f, 0x55, 0x00, 0x00, 0x80, 0xaa, 0x55, 0x55, 0x55, 0x55,
   0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x55, 0x55, 0x55};

static char CARDFILER_17_bits[] = {
   0x00, 0x00, 0x00, 0xfe, 0xff, 0x00, 0xfe, 0xff, 0x00, 0xfe, 0xff, 0x00,
   0xfe, 0xf8, 0x00, 0xfe, 0xf3, 0x00, 0x3e, 0x84, 0x00, 0x1e, 0xf8, 0x00,
   0xfe, 0x81, 0x00, 0x06, 0x82, 0x00, 0x02, 0xfc, 0x00, 0x02, 0x80, 0x00,
   0x02, 0x80, 0x00, 0x42, 0x88, 0x00, 0x42, 0x88, 0x00, 0xfe, 0xff, 0x00,
   0x00, 0x00, 0x00};

Boolean parent_realized = FALSE;
Boolean use_c_sort = FALSE;
Boolean full_path_name = FALSE;
Boolean show_timer_results = FALSE;
Boolean timer_went_off = FALSE;
Display *dpy;
XtAppContext app_context;
XtIntervalId button_timerid = NULL;
Time lasttime;

unsigned int highest_card_id_used = 0;
Dimension highlight_icon_width, highlight_icon_height;
Pixmap memex_highlight_icon;
Pixmap icon_pixmap;
Pixmap sm_icon_pixmap;

XrmDatabase user_database = NULL;
XrmDatabase system_database = NULL;
XrmDatabase merged_database;

FILE *tempfile;

MrmHierarchy card_hierarchy;		/* DRM database hierarch id */

#ifndef NO_XNLS
Locale language_id;
#endif

/* All of the widget ids. */
DXmCSTextWidget valuewindow;
DXmCSTextWidget index_dialog_text = NULL, card_index_dialog_text = NULL;
DXmCSTextWidget goto_text = NULL, find_text = NULL;
DXmCSTextWidget card_goto_text = NULL, card_find_text = NULL;

Widget indexparent, cardparent;
Widget indexworkarea;
Widget listbox, svnlist, card_help_widget, help_widget;
Widget print_widget_id = NULL;
Widget card_print_widget_id = NULL;
Widget buttonbox;

XmMainWindowWidget indexmainwindow, cardmainwindow;

XmBulletinBoardWidget cardworkarea;
/*Widget cardworkarea;*/
XmBulletinBoardWidget index_dialog = NULL, card_index_dialog = NULL;
XmBulletinBoardWidget goto_dialog = NULL, find_dialog = NULL;
XmBulletinBoardWidget card_goto_dialog = NULL, card_find_dialog = NULL;

XmFileSelectionBoxWidget file_select_dialog;
XmFileSelectionBoxWidget card_file_select_dialog;

XmMessageBoxWidget exit_dialog = NULL, clear_dialog = NULL,
  enter_fname_dialog = NULL;
XmMessageBoxWidget open_caution = NULL, delete_dialog = NULL;
XmMessageBoxWidget message_dialog = NULL, card_message_dialog = NULL;
XmMessageBoxWidget error_message_dialog = NULL;

XmPushButtonWidget print_button, print_as_button;
XmPushButtonWidget undelete_button, restore_button;
XmPushButtonWidget goto_button = NULL, card_goto_button = NULL;
XmPushButtonWidget find_button = NULL, find_next_button = NULL;
XmPushButtonWidget card_find_button = NULL, card_find_next_button = NULL;
XmPushButtonWidget undo_button, cut_button, copy_button;
XmPushButtonWidget paste_button, select_graphic_button,
  deselect_graphic_button;
XmPushButtonWidget bb_close;
XmArrowButtonWidget bb_button1, bb_button2;
/* Some global variables used for various reasons. */

GC pixmap_gc = NULL;			/* Graphic context for put image */
GC bitmap_gc = NULL;			/* Graphic context for bitmap draws  */
GC bitmap_clear_gc;			/* Graphic context for bitmap clears */
Pixel bitmap_fg;			/* Foreground pixel value */
Pixel bitmap_bg;			/* Background pixel value */
Pixmap bitmap_pm = NULL;		/* The bitmap pixmap */
XImage *bitmap_image = NULL;		/* The bitmap image */

Cursor watch_cursor = NULL;		/* The watch cursor for long funcs */
Cursor card_watch_cursor = NULL;	/* The watch cursor for long funcs */

int find_position = 0;			/* Position to start when FINDing */
char find_target[FIND_LENGTH + 1] = "";	/* string used in find */
char goto_target[INDEX_LENGTH + 1] = "";
					/* string used in goto */

char selection_buff[TEXT_LENGTH + 1] = "";
					/* selected text */
char global_fname[MAX_FILE_LEN];	/* name of open file */
char bitmap_dir[MAX_FILE_LEN];		/* bitmap directory mask */
char card_dir[MAX_FILE_LEN];		/* card directory mask */

int item_id;				/* The clipboard item id */
char text[TEXT_LENGTH + 1];		/* text to be displayed in card */
char bitmap[BITMAP_SIZE];		/* bitmap to be displayed in card */
Dimension bmp_height = 0;		/* height of bitmap to be displayed */
Dimension bmp_width = 0;		/* width of bitmap to be displayed */
int bmp_bytes = 0;			/* bytes/line of bitmap         */

Dimension bitmap_pm_wid = 300;		/* current height of bitmap pixmap */
Dimension bitmap_pm_hyt = 300;		/* current width of bitmap pixmap  */

Dimension prev_bmp_height = 0;		/* height of bitmap of prev card */
Dimension prev_bmp_width = 0;		/* width of bitmap of prev card  */

Dimension prev_width = 0;		/* Text Width of last resize    */
Dimension prev_height = 0;		/* Text Height of last resize   */

Dimension total_prev_width = 0;		/* Total Width of last resize   */
Dimension total_prev_height = 0;	/* Total Height of last resize  */

int card_mapped = FALSE;		/* Card mapped state */
int first_time_in = TRUE;		/* The first time the card is mapped */
int card_deleted = FALSE;		/* Has a card been deleted state */
int file_changed = FALSE;		/* Has the file changed state */
int file_changed_temp = FALSE;		/* Has the file changed state */
int bitmap_changed = FALSE;		/* Has the bitmap changed */

int pending_open = FALSE;		/* Used for cases where we have to */
int pending_link = FALSE;		/* save a file before doing a  */
int pending_link_visit = FALSE;		/* linkworks operation */

int FromIndex;
Boolean PrintFromIndex;
int FromCreateCard;
int has_file_name = FALSE;		/* Boolean for file name        */
/*int JustSavedUnnamed = FALSE;*/	/* Boolean for saving unnamed   */
Boolean has_bmp_dir = FALSE;		/* Boolean for bitmap dir mask  */
Boolean has_card_dir = FALSE;		/* Boolean for card dir mask    */
Boolean GrabFocus = FALSE;		/* Grabbing focus on Expose     */
Boolean CardDeIconified = TRUE;		/* Card Visible                 */
Boolean IconState = TRUE;		/* Card Visible                 */
Boolean JustCutPasted = FALSE;		/* A cut or paste was just done */

int undotype = 0;			/* Type of operation to UNDO */
int undoposition = 0;			/* Position for UNDO operation */
char undostring[TEXT_LENGTH + 1];	/* String for UNDO operation */
char undo_bitmap[BITMAP_SIZE];		/* bitmap CUT from card */
Dimension undo_bmp_height = 0;		/* height of CUT bitmap */
Dimension undo_bmp_width = 0;		/* width of CUT bitmap */

card_pointer tag_array[2000];		/* array of pointers to cards, used */
XmScrolledWindowWidget s_valuewindow;
XmFontList svnfontlist;			/* fontlist used for the SVN text. */
int index_count = 1;			/* A counter for the list box   */
int numcards = 0;			/* Number of cards in file */
int oldselect = 0;			/* Number of last selected index */
int find_type = 0;			/* Type for find next   */
int file_select_type = 0;		/* The type of FS widget used   */

card_instance onecard;
card_pointer addcard();
card_pointer first, last, card_displayed;
					/* various pointers to cards */
card_pointer currentselection;		/* currently selected card. */

struct card_contents_rec temp_card1, deleted_card;
					/* contents of temporary and the last
					 * deleted card. */
XmString printfilename[1];
unsigned int delete_flag = DXmSUPPRESS_DELETE_FILE;
XmString us_supplied_data_syntax[1];
char cardfiler[MAX_FILE_LEN];
char card_name[MAX_FILE_LEN];
char filename[MAX_FILE_LEN];
char default_name[MAX_FILE_LEN];
char *defaults_name;
char *system_defaults_name;
char print_title[MAX_FILE_LEN];
char print_format[128];
char clip_label[MAX_FILE_LEN];
char language_str[30];
Cardinal status;

#ifdef VMS
char temp_file_name[] = "sys$scratch:cardfiler%x.tmp\0";
					/* temp file name */
char printfile_buf[] = "sys$scratch:dwcard_print%x.tmp\0";
					/* Print temp name */
#else
char temp_file_name[] = "cardfiler%x.tmp\0";
					/* temp file name */
char printfile_buf[] = "dxcard_print%x.tmp\0";
					/* Print temp name */
#endif

char pid_temp_file[MAX_FILE_LEN];
char pid_print_file[MAX_FILE_LEN];

#ifndef NO_HYPERHELP
Opaque hyper_help_context;
#endif

#ifdef VMS
static readonly char ctx_stm[] = "ctx=stm", rec_udf[] = "rfm=udf",
  shr_val[] = "shr=get";
#endif

#ifdef I18N_MULTIBYTE
static unsigned int card_index_max_byte_len=40;
#endif /* I18N_MULTIBYTE */
/* Define the callback lists identifying our callback routines. */
extern void print_ok_cb();		/* receives control when OK finally
					 * selected  */
extern void print_cancel_cb();		/* receives control when CANCEL
					 * selected. */

#ifdef VMS
static char uid_filespec[] = {"DECW$CARDFILER"};
#else
static char APPL_CLASS[] = {"DXcardfiler"};
#endif

static char uid_default[] = {"DECW$SYSTEM_DEFAULTS:.UID"};

static MrmOsOpenParam os_ext_list;
static MrmOsOpenParamPtr os_ext_list_ptr = &os_ext_list;

/*
**++
**  ROUTINE NAME: main
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**      argc - argument count
**      argv - argument list
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
/******************************The MAIN PROGRAM******************************/

main(argc, argv)
    int argc;
    char **argv;
{
    int i;
    int resize_flag = FALSE;
    int xtnumber;
    int pid;
    Arg args[5];
    char card_icon[40];
    char *file_array[1];
    char base_defaults_name[MAX_FILE_LEN];
    char *status_str;
    char *get_type;
    FILE *fp;
    XrmValue get_value;
    Pixel foreground, background;
    long cs_status, byte_count;

#ifndef VMS
    /* get_uid_filename looks up the environment variable UIDDIR, and attempts
     * to find a uid file with the correct name in it. */
    char *def_name, *uiddir, *uid_name;
    int uiddir_len, uid_fd;

    os_ext_list.nam_flg.clobber_flg = TRUE;

    file_array[0] =
      XtMalloc(strlen("/usr/lib/X11/uid/.uid") + strlen(CLASS_NAME) + 1);
/*
    sprintf (file_array [0], "/usr/lib/X11/uid/%s.uid", CLASS_NAME);
*/
    sprintf(file_array[0], "%s", CLASS_NAME);

    if ((uiddir = (char *) getenv("UIDDIR")) != NULL) {
	uiddir_len = strlen(uiddir);
	if ((def_name = rindex(file_array[0], '/')) == NULL) {
	    def_name = file_array[0];
	} else
	    def_name++;
	uiddir_len += strlen(def_name) + 2;
					/* 1 for '/', 1 for '\0' */
	uid_name = (char *) XtMalloc(uiddir_len * sizeof(char));
	strcpy(uid_name, uiddir);
	strcat(uid_name, "/");
	strcat(uid_name, def_name);
	if (access(uid_name, R_OK) != 0) {
	    XtFree(uid_name);
	} else {
	    XtFree(file_array[0]);
	    file_array[0] = uid_name;
	}
    }
#else
    file_array[0] = uid_filespec;
    os_ext_list.nam_flg.related_nam = 0;
#endif
    os_ext_list.version = MrmOsOpenParamVersion;
    os_ext_list.default_fname = (char *) uid_default;

    /* Check for a resize in the arglist */

    for (i = 1; i < argc; i++)
	if (myindex(argv[i], '=') != NULL) {
	    resize_flag = TRUE;
	    i = argc;
	}

#ifdef R5_XLIB
    /* XtSetLanguageProc() should be called before XtAppInitialize() */
    XtSetLanguageProc(NULL, NULL, NULL);
#endif /* R5_XLIB */

    MrmInitialize();
    DXmInitialize();

#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    XmRepTypeInstallTearOffModelConverter();
#endif

    /* Initialize the display and the toolkit */
    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

    dpy = XtOpenDisplay(app_context, NULL, APPL_NAME, CLASS_NAME, NULL, 0,
      &argc, argv);

    if (dpy == 0)
	/* exit with error */
	XtAppErrorMsg(app_context, "invalidDisplay", "XtOpenDisplay",
	  "XtToolkitError", "Can't Open display", (String *) NULL,
	  (Cardinal *) NULL);

    indexparent = XtAppCreateShell(APPL_NAME, CLASS_NAME,
      applicationShellWidgetClass, dpy, NULL, 0);

#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    if (MrmOpenHierarchyPerDisplay(
      dpy, 1, file_array, &os_ext_list_ptr, &card_hierarchy) != MrmSUCCESS)
	exit(ERROR_STATUS);
#else
    if (MrmOpenHierarchy(1, file_array, &os_ext_list_ptr, &card_hierarchy) !=
      MrmSUCCESS) {
	exit(ERROR_STATUS);
    }
#endif

    card_get_uil_string(card_hierarchy, "CARD_APPLICATION_TITLE", cardfiler);
    card_get_uil_string(card_hierarchy, "CARD_NAME_TITLE", card_name);
    card_get_uil_string(card_hierarchy, "CARD_FILENAME_DEF", filename);
    card_get_uil_string(card_hierarchy, "CARD_FILENAME_DEF", default_name);
    card_get_uil_string(card_hierarchy, "CARD_PRINT_TITLE", print_title);
    card_get_uil_string(card_hierarchy, "CARD_CLIPBOARD_LABEL", clip_label);

#ifdef VMS
    card_get_uil_string(card_hierarchy, "CARD_VMS_DEFAULTS_NAME",
      base_defaults_name);
    defaults_name = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(defaults_name, base_defaults_name);
    card_get_uil_string(card_hierarchy, "CARD_VMS_SYSTEM_DEF_NAME",
      base_defaults_name);
#else
    card_get_uil_string(card_hierarchy, "CARD_UNIX_DEFAULTS_NAME",
      base_defaults_name);
    defaults_name = createHomeDirSpec(base_defaults_name);
    card_get_uil_string(card_hierarchy, "CARD_UNIX_SYSTEM_DEF_NAME",
      base_defaults_name);
#endif
    system_defaults_name = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(system_defaults_name, base_defaults_name);

#ifdef SYNCH
    XSynchronize(XtDisplay(indexparent), 1);
#endif

    merged_database = XtDatabase(dpy);

    status = XrmGetResource(merged_database,
					/* Database. */
      xrm_language, 			/* Resource's ASCIZ name. */
      xrc_language, 			/* Resource's ASCIZ class. */
      &get_type, 			/* Resource's type (out). */
      &get_value);			/* Address to return value. */
    if (status != NULL)
	strcpy(language_str, get_value. addr);
    else
	strcpy(language_str, "");

    status = XrmGetResource(merged_database, xrm_c_sort, xrc_c_sort, &get_type,
      &get_value);
    if (status != NULL) {
	use_c_sort = atoi(get_value. addr);
    }

    status = XrmGetResource(merged_database, xrm_full_path_name, xrc_full_path_name, &get_type,
      &get_value);
    if (status != NULL) {
	full_path_name = atoi(get_value. addr);
    }

    status = XrmGetResource(merged_database, xrm_show_timings,
      xrc_show_timings, &get_type, &get_value);
    if (status != NULL)
	show_timer_results = atoi(get_value. addr);

#ifdef NO_XNLS
    use_c_sort = TRUE;
#else
    /* Load the locale from XNLS, use the language in the defaults file or let
     * setlocale read it if it is not in the defaults file. */

    if (!DWI18n_IsXnlLangISOLatin1())
	status_str =
	  xnl_winsetlocale(LC_ALL, "en_US.88591", &language_id, dpy);
    else if (strcmp(language_str, "") == 0) {
	status_str = xnl_winsetlocale(0, 0, &language_id, dpy);
	if (status_str == 0) {
	    card_serror(BadLanguage, WARNING);
	    status_str =
	      xnl_winsetlocale(LC_ALL, "en_US.88591", &language_id, dpy);
	}
    } else
	status_str = xnl_winsetlocale(LC_ALL, language_str, &language_id, dpy);

    if (status_str == 0) {
	card_serror(BadLocale, FATAL);
    }
#endif

    /* Add PID to the end of the cardfiler and print tmp file. */
    pid = getpid();
    sprintf(pid_temp_file, temp_file_name, pid);
    sprintf(pid_print_file, printfile_buf, pid);

    /* Load the file name syntax fields */
    printfilename[0] =
      XmStringCreate(pid_print_file, XmSTRING_DEFAULT_CHARSET);

    /* get "Text" string from uil, then convert it to XmString */
    card_get_uil_string(card_hierarchy, "PrintFormat", print_format);
    us_supplied_data_syntax[0] =
      DXmCvtFCtoCS(print_format, &byte_count, &cs_status);

    /* Make the windows to be used in the overall display */
    makewindows();

    /* Initialize the card pointers. */
    initcardpointers();
    update_titlebar();

    /* Add an event handler to track Reparent notify events for the index
     * window. */
    XtAddEventHandler(indexparent, StructureNotifyMask, False,
      set_icons_on_shell, None);

    /* Add an event handler to track Reparent notify events for the card
     * window. */
    XtAddEventHandler(cardparent, StructureNotifyMask, False,
      set_icons_on_shell, None);

    /* Set the icon pixmap by calling set_icons() */
    set_icons(indexparent, card_hierarchy, FALSE);
    set_icons(cardparent, card_hierarchy, FALSE);

    /* Open the temporary file */
    if ((tempfile = fopen(pid_temp_file, WRITE_BINARY)) == NULL) {
	display_message("TempFileError");
    /* exit(ERROR_STATUS); */
    }

    /* Fetch the Memex highlight icon pixmap from UIL, using the foreground and
     * background color pixels from the SVN. */
    xtnumber = 0;
    XtSetArg(args[xtnumber], XmNforeground, &foreground);
    xtnumber++;
    XtSetArg(args[xtnumber], XmNbackground, &background);
    xtnumber++;
    XtGetValues(svnlist, args, xtnumber);

    if (MrmFetchIconLiteral(card_hierarchy, "IconHighlighted", XtScreen(
      indexparent), dpy, foreground, background, &memex_highlight_icon) !=
      MrmSUCCESS) {
	exit(ERROR_STATUS);
    }

    /* allow resizing */
    xtnumber = 0;
    XtSetArg(args[xtnumber], XtNallowShellResize, TRUE);
    xtnumber++;
    XtSetValues(indexparent, args, xtnumber);

    /* Realize all the widgets */
    XtRealizeWidget(indexparent);

    /* MEMEX - Initialize HIS and Create the Connection Menu in the Index
     * Window */
    CreateDwUi();

    /* If a card database filename was supplied in the command line, load  it.
     */
    if (argc > 1)
	retrieveaction(argv[1]);

    /* Set parent_realized = TRUE so subsequence fetch_widget() calls will get
     * Watch Cursor. */
    parent_realized = TRUE;

    /* Get and dispatch events */
    XtAppMainLoop(app_context);
}

/*
**++
**  ROUTINE NAME: NoFunction
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int NoFunction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
#ifdef DEBUG
    fprintf(stderr, "No Function.\n");
#endif
}

/*
**++
**  ROUTINE NAME: noaction
**
**  FUNCTIONAL DESCRIPTION:
**		Used for cancel buttons. Place holder and dummy routine.
**              It does nothing. <------- It's a lie, don't believe him.
**
**  FORMAL PARAMETERS:
**		param - passed and not used, therefore routine can be used for
**			any needed function that is not yet implemented.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void noaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    lasttime = XtLastTimestampProcessed(dpy);

    if (!FromIndex)
	set_focus_now(cardparent);
    else
	set_focus_now(indexparent);

    XtUnmanageChild(widget);
}

/*
**++
**  ROUTINE NAME: manage_and_focus
**
**  FUNCTIONAL DESCRIPTION:
**		Manages a widget and sets the input focus to it.
**
**  FORMAL PARAMETERS:
**		w -	widget to be managed.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
manage_and_focus(w)
    Widget w;
{
    XtManageChild(w);
    set_focus(w);
}

/*
**++
**  ROUTINE NAME: set_focus
**
**  FUNCTIONAL DESCRIPTION:
**	Sets the input focus flag the next time the widget is refreshed.
**
**  FORMAL PARAMETERS:
**	w -	widget to be focused
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	GrabFocus
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
set_focus(w)
    Widget w;
{
    if (CardDeIconified)
	GrabFocus = TRUE;
}

/*
**++
**  ROUTINE NAME: set_focus_now
**
**  FUNCTIONAL DESCRIPTION:
**		Sets the input focus on a widget
**
**  FORMAL PARAMETERS:
**		w -	widget to be focused
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
set_focus_now(w)
    Widget w;
{
    Boolean status;
    XtWidgetGeometry request;
    XtGeometryResult result;

#ifdef DEBUG
    printf("set_focus_now called\n");
#endif
    if (w == cardparent) {

	if ((card_mapped) && (CardDeIconified)) {
#ifdef DEBUG
	    printf("Setting focus to cardparent\n");
#endif
	    XSetInputFocus(dpy, XtWindow(w), RevertToParent, lasttime);
	    request.request_mode = CWStackMode;
	    request.stack_mode = Above;
	    result = XtMakeGeometryRequest(w, &request, NULL);
	}

    } else {
#ifdef DEBUG
	printf("Setting focus to indexparent\n");
#endif
	XSetInputFocus(dpy, XtWindow(w), RevertToParent, lasttime);
	request.request_mode = CWStackMode;
	request.stack_mode = Above;
	result = XtMakeGeometryRequest(w, &request, NULL);
    }
/*
	status = XmProcessTraversal (w, XmTRAVERSE_CURRENT);
*/

/*  Replaced by the above call.
    Time CT;
    if ((card_mapped) && (CardDeIconified)) {
      CT = CurrentTime;
      (*w ->core.widget_class->core_class.accept_focus) (w, &CT);
    }
*/
}

/*
**++
**  ROUTINE NAME: on_context_activate_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void on_context_activate_proc(w, tag, reason, name, parent)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
    String name;
    Widget parent;
{
    /* Call DXm Help on context to handle everything. */
    DXmHelpOnContext(indexparent, FALSE);
}

/*
**++
**  ROUTINE NAME: tracking_help
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void tracking_help()
{
    Widget track_widget;
    Cursor cursor;

    track_widget = NULL;

    cursor = XCreateFontCursor(dpy, XC_question_arrow);

    track_widget = XmTrackingLocate(indexparent, cursor, FALSE);

    if (track_widget != 0) {
	if (XtHasCallbacks(track_widget, XmNhelpCallback) == XtCallbackHasSome)
	    XtCallCallbacks(track_widget, XmNhelpCallback, NULL);
    } else {
	XtUngrabPointer(indexparent, CurrentTime);
    }
}

/*
**++
**  ROUTINE NAME: cardhelp
**
**  FUNCTIONAL DESCRIPTION:
**		Puts up the card help.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void cardhelp(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    XmString Frame;

    Frame = (XmString) tag;
    DisplayHelp(Frame, FALSE);
}

/*
**++
**  ROUTINE NAME: indexhelp
**
**  FUNCTIONAL DESCRIPTION:
**		Puts up the card help.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void indexhelp(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    XmString Frame;

    Frame = (XmString) tag;
    DisplayHelp(Frame, TRUE);
}

/*
**++
**  ROUTINE NAME: DisplayHelp
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DisplayHelp(Frame, FromIndex)
    XmString Frame;
    Boolean FromIndex;
{
#ifndef NO_HYPERHELP			/* Hyperhelp */

    if (!hyper_help_context) {
	WorkCursorDisplay();
	DXmHelpSystemOpen(&hyper_help_context, indexparent, CARD_HELP,
	  display_message, NoCardHelp);
	WorkCursorRemove();
    }

    WorkCursorDisplay();
    DXmHelpSystemDisplay(hyper_help_context, CARD_HELP, "Topic", Frame,
      display_message, NoCardHelp);
    WorkCursorRemove();

#else					/* Motif Help Widget */

    Arg args[2];
    int ac=0;
    MrmType *dummy_class;
    XmString help_lib_name_cs;
    long cs_status, byte_count;

    help_lib_name_cs = DXmCvtFCtoCS("cardfiler", &byte_count, &cs_status);
    XtSetArg(args[ac], DXmNlibrarySpec, help_lib_name_cs);
    ac++;

    if (help_widget == 0) {
	WorkCursorDisplay();
	if (MrmFetchWidgetOverride(card_hierarchy,
					/* hierarchy id                 */
	  "main_help", 			/* Index of widget to fetch     */
	  indexparent, 			/* Parent of widget             */
	  NULL, 			/* Override name                */
	  args, 			/* Override args                */
	  ac, 				/* Override args count          */
	  &help_widget, 		/* Widget id                    */
	  &dummy_class) != MrmSUCCESS)	/* Widget Class                 */
	    card_serror(NoCardHelp, WARNING);

	if (MrmFetchWidgetOverride(card_hierarchy,
					/* hierarchy id                 */
	  "card_main_help", 		/* Index of widget to fetch     */
	  cardparent, 			/* Parent of widget             */
	  NULL, 			/* Override name                */
	  args, 			/* Override args                */
	  ac, 				/* Override args count          */
	  &card_help_widget, 		/* Widget id                    */
	  &dummy_class) != MrmSUCCESS)	/* Widget Class                 */
	    card_serror(NoCardHelp, WARNING);
	WorkCursorRemove();
    }

    if ((help_widget != 0) && (card_help_widget != 0)) {
	ac = 0;
	XtSetArg(args[ac], DXmNfirstTopic, Frame);
	ac++;
	if (FromIndex) {
	    XtSetValues(help_widget, args, ac);
	    XtManageChild(help_widget);
	} else {
	    XtSetValues(card_help_widget, args, ac);
	    XtManageChild(card_help_widget);
	}
    }
#endif
}

/*
**++
**  ROUTINE NAME: help_done_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void help_done_proc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (*tag == 0)
	XtUnmanageChild(help_widget);
    else
	XtUnmanageChild(card_help_widget);
}

/*
**++
**  ROUTINE NAME: MakePixmap
**
**  FUNCTIONAL DESCRIPTION:
**              Routine to make a pixmap
**
**  FORMAL PARAMETERS:
**		None used.
**
**  IMPLICIT INPUTS:
**		NONE
**
**  IMPLICIT OUTPUTS:
**		NONE
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static Pixmap MakePixmap(dpy, root, data, width, height)
    Display *dpy;
    Drawable root;
    short *data;
    Dimension width, height;
{
    Pixmap pid;
    unsigned long ScreenNumber;

    ScreenNumber = XDefaultScreen(dpy);

    pid =
      XCreatePixmapFromBitmapData(dpy, root, (char*)data, (Dimension) width,
      (Dimension) height, (unsigned long) BlackPixel(dpy, ScreenNumber),
      (unsigned long) WhitePixel(dpy, ScreenNumber), 1);
/*  The following line removed for Motif Icon.  */
/*		(unsigned int) DefaultDepth (dpy, ScreenNumber)); */
    return (pid);
}

/*
**++
**  ROUTINE NAME: selectline
**
**  FUNCTIONAL DESCRIPTION:
**	This routine selects the index card from the LISTBOX
**	and displays the contents of the card in the Card Window.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void selectline(index)
    int index;
{
    card_pointer card, FindCardByIndex();

    displaycardfromindex(index);
    card = FindCardByIndex(index);
    svn_make_entry_visible(card);

}

/*
**++
**  ROUTINE NAME: gotoaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure searches for the card with the user-specified
**	string in the index field. It then makes that card the current
**	card.
**
**  FORMAL PARAMETERS:
**	tag - used to pass the window id of the text widget.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void gotoaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer current, start;
    int found, ok;
    char *target, *temp;
    long byte_count, cvt_status;

    if (widget == NULL)
	ok = TRUE;
    else {
	ok = (int) *tag;
	lasttime = XtLastTimestampProcessed(dpy);
    }

    if (ok) {
	if (currentselection != NULL) {
	    WorkCursorDisplay();
	    found = FALSE;
	    find_type = 1;
	    XtSetSensitive((Widget)find_next_button, TRUE);
	    if (card_find_next_button != NULL)
		XtSetSensitive((Widget)card_find_next_button, TRUE);
	    current = start = currentselection->next;
	    if (FromIndex) {
		XmString cs_ptr;

		cs_ptr = DXmCSTextGetString(goto_text);
		if (cs_ptr == NULL) {
		    target = XtMalloc(1);
		    target[0] = '\0';
		} else
		    target = DXmCvtCStoFC(cs_ptr, &byte_count, &cvt_status);
		XmStringFree(cs_ptr);
	    } else {
		XmString cs_ptr;

		cs_ptr = DXmCSTextGetString(card_goto_text);
		if (cs_ptr == NULL) {
		    target = XtMalloc(1);
		    target[0] = '\0';
		} else
		    target = DXmCvtCStoFC(cs_ptr, &byte_count, &cvt_status);
		XmStringFree(cs_ptr);
	    }
	    strcpy(goto_target, target);
	    XtFree(target);

	    while ((!found) && (current != NULL)) {
		if ((current != last) &&
		  (search(current->index, goto_target) != -1)) {
		    XmString cs_ptr;

		    found = TRUE;
		    getcard(current);
		    card_displayed = current;
		    DXmCSTextClearSelection(valuewindow);
		    cs_ptr = DXmCSTextGetString(valuewindow);
		    if (cs_ptr == NULL) {
			temp = XtMalloc(1);
			temp[0] = '\0';
		    } else
			temp = DXmCvtCStoFC(cs_ptr, &byte_count, &cvt_status);
		    DXmCSTextSetCursorPosition(valuewindow,
		      DWI18n_CharCount(temp));
		    XmStringFree(cs_ptr);
		    XtFree(temp);
		    displaycard();

		    if (currentselection != current) {
			find_position = 0;
			currentselection = current;
			svn_make_entry_visible(currentselection);
		    }
		} else {
		    if (current->next == NULL)
			current = first;
		    else
			current = current->next;
		}

		if (current == start)
		    break;		/* get out of while when come back to
					 * start */
	    }
	    WorkCursorRemove();
	}

	if (found == FALSE)
	    display_message("StringNotFound");
    }

    if (widget != NULL)
	if (FromIndex) {
	    set_focus_now(indexparent);
	    XtUnmanageChild((Widget)goto_dialog);
	} else {
	    set_focus_now(cardparent);
	    XtUnmanageChild((Widget)card_goto_dialog);
	}
}

/*
**++
**  ROUTINE NAME: displaycardbyid
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure searches for the card with the specified unique
**	id. It then displays the card and makes that card the current card .
**
**  FORMAL PARAMETERS:
**	card_id - unique id of the card to be displayed.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void displaycardbyid(card_id)
    unsigned int card_id;
{
    card_pointer current, start;
    int found;
    char *target, *temp;
    int ok;

    WorkCursorDisplay();
    found = FALSE;
    current = start = first;
    while ((!found) && (current != NULL)) {
	if ((current != last) && (current->card_id == card_id)) {
	    found = TRUE;
	    getcard(current);
	    card_displayed = current;
	    displaycard();

	    if (currentselection != current) {
		find_position = 0;
		currentselection = current;
		svn_make_entry_visible(currentselection);
	    }
	} else {
	    if (current->next == NULL)
		current = first;
	    else
		current = current->next;
	}

	if (current == start)
	    break;			/* get out of while when come back to
					 * start */
    }
    WorkCursorRemove();

    if (found == FALSE)
	display_message("StringNotFound");

}

/*
**++
**  ROUTINE NAME: gotoproc
**
**  FUNCTIONAL DESCRIPTION:
**	This routine puts up the dialog box to prompt for the 'goto' string.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int gotoproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (*tag == 0)
	FromIndex = TRUE;
    else
	FromIndex = FALSE;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (currentselection != NULL) {
	if (goto_dialog == NULL) {
	    goto_dialog =
	      (XmBulletinBoardWidget) fetch_widget("goto_dialog", indexparent);

	    card_goto_dialog = (XmBulletinBoardWidget) fetch_widget(
	      "card_goto_dialog", cardparent);
	} else {
	    if (FromIndex) {
		XmString cs_ptr;
		long byte_count, cvt_status;

		cs_ptr = DXmCvtFCtoCS(goto_target, &byte_count, &cvt_status);
		DXmCSTextSetString(goto_text, cs_ptr);
		DXmCSTextSetSelection(goto_text, 0,
		  DWI18n_CharCount(goto_target), CurrentTime);
		XmStringFree(cs_ptr);
	    } else {
		XmString cs_ptr;
		long byte_count, cvt_status;

		cs_ptr = DXmCvtFCtoCS(goto_target, &byte_count, &cvt_status);
		DXmCSTextSetString(card_goto_text, cs_ptr);
		DXmCSTextSetSelection(card_goto_text, 0,
		  DWI18n_CharCount(goto_target), CurrentTime);
		XmStringFree(cs_ptr);
	    }
	}

	if (FromIndex) {
	    XtManageChild((Widget)goto_dialog);
	} else {
	    XtManageChild((Widget)card_goto_dialog);
	}
    } else
	display_message("NoCardsError");
}

/*
**++
**  ROUTINE NAME: findaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure searches for a particular string in the card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void findaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int found;
    int ok = (int) *tag;
    char *target;
    long byte_count, cvt_status;
    XmString cs_target;
    card_pointer start;

    lasttime = XtLastTimestampProcessed(dpy);

    if (ok) {
	WorkCursorDisplay();
	if (currentselection != NULL) {
	    found = FALSE;
	    find_type = 2;
	    start = currentselection;
	    if (FromIndex) {
		cs_target = DXmCSTextGetString(find_text);
		if (cs_target == NULL) {
		    target = XtMalloc(1);
		    target[0] = '\0';
		} else
		    target = DXmCvtCStoFC(cs_target, &byte_count, &cvt_status);
		XmStringFree(cs_target);
	    } else {
		cs_target = DXmCSTextGetString(card_find_text);
		if (cs_target == NULL) {
		    target = XtMalloc(1);
		    target[0] = '\0';
		} else
		    target = DXmCvtCStoFC(cs_target, &byte_count, &cvt_status);
		XmStringFree(cs_target);
	    }
	    strcpy(find_target, target);
	    XtFree(target);
	    findit(start);
	    XtSetSensitive((Widget)find_next_button, TRUE);
	    if (card_find_next_button != NULL)
		XtSetSensitive((Widget)card_find_next_button, TRUE);
	}
	WorkCursorRemove();
    }

    if (FromIndex) {
	set_focus_now(indexparent);
	XtUnmanageChild((Widget)find_dialog);
    } else {
	set_focus_now(cardparent);
	XtUnmanageChild((Widget)card_find_dialog);
    }
}

/*
**++
**  ROUTINE NAME: findit
**
**  FUNCTIONAL DESCRIPTION:
**	This is the generic find procedure - it's used for both FIND and
**	FIND NEXT.
**
**  FORMAL PARAMETERS:
**	start - the place to start when searching for the string.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
findit(start)
    card_pointer start;
{
    card_pointer current;
    int found = FALSE, back_to_beginning = FALSE;
    int temp_position = 0, xtnumber;
    int first_pos, last_pos;
    int char_inc;
    char find_text[FIND_LENGTH + 1], find_index[INDEX_LENGTH + 1];
    char *count_text;
    Arg changeargs[10];

    current = start;
    readonecard(current, &temp_card1);

    while ((back_to_beginning == FALSE) && (found == FALSE)) {
	if ((find_target[0] == NULL) && (find_position != 0))
	    temp_position = -1;
	else
	    temp_position =
	      search(&temp_card1.text[find_position], find_target);
	if ((current != last) && (temp_position != -1)) {
	    /* To align word boundary */
	    char_inc =
#ifdef I18N_BUG_FIX
	      DWI18n_ByteLengthOfChar(&temp_card1.text[find_position +
				temp_position]);
#else
	      DWI18n_ByteLengthOfChar(&temp_card1.text[find_position]);
#endif /* I18N_BUG_FIX */
	    find_position = find_position + char_inc + temp_position;
	    found = TRUE;
	    if (card_displayed != current) {
		getcard(current);
		card_displayed = current;
		displaycard();
		if (currentselection != current) {
		    currentselection = current;
		    svn_make_entry_visible(currentselection);
		}
	    }

	    count_text = XtMalloc(find_position - char_inc + 1);
	    memcpy(count_text, &temp_card1.text[0], find_position - char_inc);
	    count_text[find_position - char_inc] = '\0';
	    first_pos = DWI18n_CharCount(count_text);
	    last_pos = first_pos + DWI18n_CharCount(find_target);
	    XtFree(count_text);

	    xtnumber = 0;

	    XtSetArg(changeargs[xtnumber], XmNcursorPosition, last_pos);
	    xtnumber++;
	    XtSetValues((Widget)valuewindow, changeargs, xtnumber);
	    DXmCSTextClearSelection(valuewindow);
	    DXmCSTextSetSelection(valuewindow, first_pos, last_pos,
	      CurrentTime);
	} else {
	    if (current->next == last) {
		current = first;
		readonecard(current, &temp_card1);
	    } else {
		current = current->next;
		readonecard(current, &temp_card1);
	    }
	    find_position = 0;
	    if (current == start)
		back_to_beginning = TRUE;
	}
    }

    if (found == FALSE) 
	display_message("StringNotFound");

}

/*
**++
**  ROUTINE NAME: findproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure maps the dialog box to prompt for the find target
**	string.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int findproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    XmString cs_ptr;
    long byte_count, cvt_status;

    if (*tag == 0)
	FromIndex = TRUE;
    else
	FromIndex = FALSE;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (currentselection != NULL) {
	if (find_dialog == NULL) {
	    find_dialog =
	      (XmBulletinBoardWidget) fetch_widget("find_dialog", indexparent);

	    card_find_dialog = (XmBulletinBoardWidget) fetch_widget(
	      "card_find_dialog", cardparent);
	} else {
	    if (FromIndex) {
		cs_ptr = DXmCvtFCtoCS(find_target, &byte_count, &cvt_status);
		DXmCSTextSetString(find_text, cs_ptr);
		DXmCSTextSetSelection(find_text, 0,
		  DWI18n_CharCount(find_target), CurrentTime);
		XmStringFree(cs_ptr);
	    } else {
		cs_ptr = DXmCvtFCtoCS(find_target, &byte_count, &cvt_status);
		DXmCSTextSetString(card_find_text, cs_ptr);
		DXmCSTextSetSelection(card_find_text, 0,
		  DWI18n_CharCount(find_target), CurrentTime);
		XmStringFree(cs_ptr);
	    }
	}

	if (FromIndex) {
	    XtManageChild((Widget)find_dialog);
	} else {
	    XtManageChild((Widget)card_find_dialog);
	}
	find_target[0] = '\0';

    } else
	display_message("NoCardsError");
}

/*
**++
**  ROUTINE NAME: findnextproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure searches for the find target again.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int findnextproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (find_type == 1)
	gotoaction(NULL, NULL, NULL);
    else if (currentselection == NULL)
	display_message("NoCardsError");
    else {
	WorkCursorDisplay();
	findit(currentselection);
	WorkCursorRemove();
    }
}

/*
**++
**  ROUTINE NAME: LoadListBox
**
**  FUNCTIONAL DESCRIPTION:
**	Find the place where the card belongs (routine for this must change).
**	Then call insert card to actually insert the card into the linked
**	list.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
LoadListBox()
{
    int num_tags = 0;
    card_pointer curcard, newcard;

    curcard = first;

    while (curcard != last) {
	tag_array[num_tags] = curcard;
	num_tags++;

	/* loading the list box with empty svn, set svngetentrycalled field to
	 * false. Otherwise SVN will ACCVIO. */
	curcard->svngetentrycalled = FALSE;
	curcard->index_number = index_count;
	curcard = curcard->next;
	index_count++;
    }
    DXmSvnAddEntries(svnlist, 0, num_tags, 0, tag_array, TRUE);
    SetHighlighting(TRUE);		/* MEMEX */
}

/*
**++
**  ROUTINE NAME: insertcard
**
**  FUNCTIONAL DESCRIPTION:
**	Create an element for and insert card just before current card
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
card_pointer insertcard(curcard, index, fp, offset, index_number)
    struct card *curcard;
    char index[];
    FILE *fp;
    int offset;
    int index_number;
{
    struct card *newcard;

    if ((newcard = (card_pointer) XtMalloc(sizeof(card_instance))) == NULL) {
	display_message("NoMoreMemory");
	/* exit(ERROR_STATUS); */
    } else {
	if (curcard->last != NULL)	/* curcard isn't the first card */
	    curcard->last->next = newcard;
	else
	    first = newcard;		/* curcard is the first card */

	newcard->next = curcard;	/* update all the pointers to insert */
	strcpy(newcard->index, index);	/* the card into the linked list */
	newcard->fp = fp;
	newcard->offset = offset;
	newcard->orig_fp = NULL;
	newcard->orig_offset = NULL;
	newcard->last = curcard->last;
	curcard->last = newcard;
	newcard->index_number = index_number;
#ifndef NO_MEMEX
	newcard->surrogates = 0;	/* MEMEX */
#endif
	newcard->highlighted = FALSE;	/* MEMEX */
	newcard->svngetentrycalled = FALSE;

	return (newcard);
    }
}

/*
**++
**  ROUTINE NAME: addcard
**
**  FUNCTIONAL DESCRIPTION:
**	Find the place where the card belongs (routine for this must change).
**	Then call insert card to actually insert the card into the linked
**	list.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
card_pointer addcard(index)
    char index[];
{
    card_pointer curcard, newcard;
    int inserted;

    inserted = FALSE;
    curcard = first;

    if (last->last != NULL)
	if (comparethem(last->last->index, index) <= 0) {
	    /* insert it at the end */
	    newcard = insertcard(last, index, NULL, NULL, index_count);
	    inserted = TRUE;
	}

    while (!inserted) {
	if (curcard == last) {
	    /* the card is inserted here */
	    newcard = insertcard(curcard, index, NULL, NULL, index_count);
	    inserted = TRUE;
	} else if (comparethem(curcard->index, index) >= 0) {
	    /* the card is inserted here */
	    newcard = insertcard(curcard, index, NULL, NULL, index_count);
	    inserted = TRUE;
	}
	curcard = curcard->next;
    }
    index_count++;
    return (newcard);
}

/*
**++
**  ROUTINE NAME: previousproc
**
**  FUNCTIONAL DESCRIPTION:
**	Make the previous card the new current card. If the current card is
**	the first card, then the last card becomes the current card. In
**	other words, the list appears cyclic (although it actually isn't).
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
previousproc(widget, tag, reason)
    Widget widget;
    int *tag;
    XmAnyCallbackStruct *reason;
{
    int new_index;

    WorkCursorDisplay();
    if (card_displayed == first)
	new_index = last->last->index_number;
    else
	new_index = card_displayed->last->index_number;

    selectline(new_index);
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: nextproc
**
**  FUNCTIONAL DESCRIPTION:
**	Makes the next card the current card. This acts cyclic also (see
**	previousproc routine.)
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
nextproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int new_index;

    WorkCursorDisplay();
    if (card_displayed->next == last)
	new_index = first->index_number;
    else
	new_index = card_displayed->next->index_number;

    selectline(new_index);
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: closeproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the cardwindow.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
closeproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    lasttime = XtLastTimestampProcessed(dpy);
/*  set_focus_now (indexparent); */
    check_and_save(card_displayed);
    closeaction();
}

/*
**++
**  ROUTINE NAME: closeaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the cardwindow.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
closeaction()
{
    CursorRemove(cardparent, cardparent);
    card_displayed = NULL;
    card_mapped = FALSE;
    CardDeIconified = FALSE;
    GrabFocus = FALSE;
    XtUnmapWidget(cardparent);
}

/*
**++
**  ROUTINE NAME: restoreproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure restores the card to what it was when read in
**	originally. If it was added since read in, it will be left unchanged.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int restoreproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer deleteme, newcard;
    int offset;
    char buff1[INDEX_LENGTH + 1], buff2[TEXT_LENGTH + 1], *buff3;
    int rem, save_width;


    deleteme = card_displayed;
    currentselection = card_displayed;
    if (currentselection->orig_fp != NULL) {
	WorkCursorDisplay();

	/* if this was read in */
	currentselection->fp = currentselection->orig_fp;
	currentselection->offset = currentselection->orig_offset;

	/* Read in the old values and then make them the global ones. */
	readonecard(currentselection, &temp_card1);
	strcpy(text, temp_card1.text);
	bmp_height = temp_card1.bitmap_height;
	bmp_width = temp_card1.bitmap_width;

	rem = bmp_width % 32;
	if (rem == 0)
	    save_width = bmp_width;
	else
	    save_width = bmp_width - rem + 32;

	mybcopy(temp_card1.bitmap, bitmap, (bmp_height * save_width) / 8);
	displaycard();
	WorkCursorRemove();

    } else
	display_message("CardNotReadError");
}

/*
**++
**  ROUTINE NAME: addproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure does the prompting for adding a new card. It then
**	calls addcard which finds the spot to insert the card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int addproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer newcard;
    char buff1[INDEX_LENGTH + 1];
    int offset;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (*tag == 0)
	FromIndex = TRUE;
    else
	FromIndex = FALSE;

    renameindex(NULL, NULL, NULL);
}

/*
**++
**  ROUTINE NAME: opencard
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure opens the currently selected card.
**
**  FORMAL PARAMETERS:
**	tag - not used
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void opencard(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char *temp;

    if (currentselection != NULL) {
	XmString cs_temp;
	long byte_count, cvt_status;

	cs_temp = DXmCSTextGetString(valuewindow);
	if (cs_temp == NULL) {
	    temp = XtMalloc(1);
	    temp[0] = '\0';
	} else
	    temp = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
	DXmCSTextSetCursorPosition(valuewindow, DWI18n_CharCount(temp));
	XtFree(temp);
	DXmCSTextClearSelection(valuewindow);
	getcard(currentselection);
	card_displayed = currentselection;
	displaycard();
    }

}

/*
**++
**  ROUTINE NAME: AddCard
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void AddCard(card_to_add)
    card_pointer card_to_add;
{

    int after_entry;

/* Get the entry number of the card before this one.
   If this was the first card in the list where the previous
   pointer is to Last, then add card after entry number 0.
*/

    if (card_to_add->last == last)
	after_entry = 0;
    else
	after_entry = DXmSvnGetEntryNumber(svnlist, card_to_add->last);

/* Add the entry and make it visible in window. */
    tag_array[0] = card_to_add;
    DXmSvnAddEntries(svnlist, after_entry, 1, 0, tag_array, TRUE);
    svn_make_entry_visible(card_to_add);
}

/*
**++
**  ROUTINE NAME: svn_make_entry_visible
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure makes the passed entry visible and selected.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void svn_make_entry_visible(card)
    card_pointer card;
{
    int entry_number;
    unsigned int ent_array, comp_array, tag_array;

    entry_number = DXmSvnGetEntryNumber(svnlist, card);

    if (entry_number != 0) {

	/* Add the entry and make it visible in window. */
	DXmSvnDisableDisplay(svnlist);

	/* DXmSvnGetSelections (svnlist, &ent_array, &comp_array, &tag_array,
	 * 1); */
	DXmSvnClearSelections(svnlist);
	DXmSvnPositionDisplay(svnlist, entry_number, DXmSvnKpositionMiddle);
	DXmSvnSelectEntry(svnlist, entry_number);
	DXmSvnEnableDisplay(svnlist);
    }
}

/*
**++
**  ROUTINE NAME: duplicate
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure duplicates the current card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int duplicate(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer newcard;
    char *temp_str;
    int offset;

    if (card_displayed != NULL) {
	XmString cs_temp;
	long byte_count, cvt_status;

	WorkCursorDisplay();
	check_and_save(card_displayed);
	cs_temp = DXmCSTextGetString(valuewindow);
	if (cs_temp == NULL) {
	    temp_str = XtMalloc(1);
	    temp_str[0] = '\0';
	} else
	    temp_str = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
	strcpy(text, temp_str);
	XtFree(temp_str);

	/* Create new card. */
	readonecard(card_displayed, &temp_card1);
	bmp_height = temp_card1.bitmap_height;
	bmp_width = temp_card1.bitmap_width;
	newcard = addcard(card_displayed->index);

	/* increment the highest id and assigned it to the card. */
	highest_card_id_used++;
	newcard->card_id = highest_card_id_used;
	numcards++;
	if (numcards == 1)
	    gray_no_cards();
	currentselection = card_displayed = newcard;

	AddCard(card_displayed);

	SetHighlighting(TRUE);		/* MEMEX */

	if (card_mapped != FALSE)
	    displaycard();
	WorkCursorRemove();
    }
}

/*
**++
**  ROUTINE NAME: delcard
**
**  FUNCTIONAL DESCRIPTION:
**	Delete the specified card from the linked list
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
delcard(curcard)
    struct card *curcard;
{
    int entry_number;

    if (curcard != NULL) {

	/* check for first and last card */
	if (curcard == first)
	    first = curcard->next;
	if (curcard == last)
	    last = curcard->last;

	/* make sure the pointers aren't null and update */
	if (curcard->last != NULL)
	    curcard->last->next = curcard->next;
	if (curcard->next != NULL)
	    curcard->next->last = curcard->last;

	/* Free the memory allocated for this card */
	curcard->next = NULL;
	curcard->last = NULL;

	FreeSurrogates(curcard);	/* MEMEX */

	XtFree((char*)curcard);

	/* Get the entry number and call SvnDeleteEntries to remove it. */
	entry_number = DXmSvnGetEntryNumber(svnlist, curcard);
	DXmSvnDeleteEntries(svnlist, entry_number - 1, 1);
    }
}

/*
**++
**  ROUTINE NAME: clearfunc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure maps the dialog box which asks if you want to save
**	all of the cards before proceeding.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
clearfunc()
{
    int nreason;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if ((numcards > 0) && (file_changed == TRUE)) {
	if (clear_dialog == NULL)
	    clear_dialog =
	      (XmMessageBoxWidget) fetch_widget("clear_dialog", indexparent);
	XtManageChild((Widget)clear_dialog);
    } else if (numcards > 0) {
	nreason = XmCR_CANCEL;
	clearaction(NULL, NULL, &nreason);
    }
}

/*
**++
**  ROUTINE NAME: clearaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure deletes all the cards in the cardfile.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void clearaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer current, temp;
    int res;

    res = reason->reason;

    lasttime = XtLastTimestampProcessed(dpy);

    if (widget != NULL) {
	set_focus_now(indexparent);
	XtUnmanageChild((Widget)clear_dialog);
    }

    switch (res) {
	case XmCR_OK:
	    clearsaveaction(NULL);
	    break;

	case XmCR_CANCEL:
	    current = first;
	    while (numcards > 0) {
		temp = current->next;
		delcard(current);
		current = temp;
		numcards--;
	    }
	    gray_no_cards();

	    initcardpointers();
	    update_titlebar();
	    strcpy(filename, default_name);
	    if (card_mapped == TRUE) {
		closeaction();
	    }
	    card_displayed = NULL;
	    currentselection = NULL;
	    file_changed = FALSE;

	    /* Delete all entries beginning from entry 0. */
	    DXmSvnDisableDisplay(svnlist);
	    DXmSvnDeleteEntries(svnlist, 0, numcards);
	    DXmSvnEnableDisplay(svnlist);
	    break;
	default:
	    break;
    }
}

/*
**++
**  ROUTINE NAME: file_select_action
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the clear caution box, stores the data and
**	clears the cardfiler.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void file_select_action(widget, tag, cbstruct)
    Widget widget;
    XtPointer *tag;
    XmFileSelectionBoxCallbackStruct *cbstruct;
{
    char *fname, *dirmask;
    long byte_count, cvt_status;

    lasttime = XtLastTimestampProcessed(dpy);
    set_focus_now(indexparent);
    XtUnmanageChild(widget);

    if (cbstruct->reason == XmCR_OK) {

	dirmask = DXmCvtCStoFC(cbstruct->mask, &byte_count, &cvt_status);

	if (file_select_type == READGRAPHIC) {
	    strcpy(bitmap_dir, dirmask);
	    has_bmp_dir = TRUE;
	} else {
	    strcpy(card_dir, dirmask);
	    has_card_dir = TRUE;
	}

	/* XtFree(dirmask); */

	fname = DXmCvtCStoFC(cbstruct->value, &byte_count, &cvt_status);

	switch (file_select_type) {
	    case CLEARSTORE:
		clearstoreaction(fname);
		break;

	    case EXITSAVE:
		exitsaveaction(fname);
		break;

	    case MERGE:
		mergeaction(fname);
		break;

	    case READGRAPHIC:
		readgraphicaction(fname);
		break;

	    case RETRIEVE:
		retrieveaction(fname);
		break;

	    case SAVE:
		saveaction(fname);
		break;

	    case ENTERFNAME:
		/* This case is used by Linkworks only.
		 * set pending_link_visit flag and carry on */
		pending_link_visit = TRUE;
		saveaction(fname);
		break;

	    default:
		break;
	}				/* end switch */
	/* XtFree(fname); */
    }					/* end reason = OK */
      else {				/* reason = cancel */
	pending_open = FALSE;		/* cancel all pending */
	pending_link = FALSE;		/* operations */
        pending_link_visit = FALSE;
	if (file_select_type == RETRIEVE) {
	    file_changed = file_changed_temp;
					/* reset to correct state */
	    file_changed_temp = FALSE;
	}
    }					/* end reason = cancel */
}

/*
**++
**  ROUTINE NAME: card_file_select_action
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the clear caution box, stores the data and
**	clears the cardfiler.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void card_file_select_action(widget, tag, cbstruct)
    Widget widget;
    XtPointer *tag;
    XmFileSelectionBoxCallbackStruct *cbstruct;
{
    char *fname, *dirmask;
    long byte_count, cvt_status;

    lasttime = XtLastTimestampProcessed(dpy);
    set_focus_now(cardparent);
    XtUnmanageChild(widget);

    WorkCursorDisplay();

    if (cbstruct->reason == XmCR_OK) {

	dirmask = DXmCvtCStoFC(cbstruct->mask, &byte_count, &cvt_status);

	if (file_select_type == READGRAPHIC) {
	    strcpy(bitmap_dir, dirmask);
	    has_bmp_dir = TRUE;
	} else {
	    strcpy(card_dir, dirmask);
	    has_card_dir = TRUE;
	}
	/* XtFree(dirmask); */

	fname = DXmCvtCStoFC(cbstruct->value, &byte_count, &cvt_status);

	if (file_select_type == READGRAPHIC)
	    readgraphicaction(fname);

    } else 
	/* Cancel button pressed */
	if (file_select_type == RETRIEVE) {
	    file_changed = file_changed_temp;
	    file_changed_temp = FALSE;
	}

    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: clearstoreaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure stores the data and clears the cardfiler.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void clearstoreaction(fname)
    char *fname;
{
    FILE *fp;
    int reason;

    fp = fopen(fname, WRITE_BINARY);

    if (fp == NULL)
	display_message("OpenFileError");
    else {
	if (SaveAllCards(fp)) {
	    reason = XmCR_CANCEL;
	    clearaction(NULL, NULL, &reason);
	    file_changed = FALSE;
	}
    }
    XtFree(fname);
}

/*
**++
**  ROUTINE NAME: clearsaveaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure calls the appropriate procedure to either save the
**	cards or to prompt for a filename and then save the cards.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
clearsaveaction(widget, tag, reason)
    Widget widget;
    int *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    int nreason;
    XmString CS;

    if (!has_file_name) {
	if (card_displayed != NULL)
	    check_and_save(card_displayed);
	file_select_type = CLEARSTORE;
	XtSetArg(arg_list[ac], XmNdialogTitle, "SaveTitle");
	ac++;
	XtSetArg(arg_list[ac], XmNvalue, "CardValue");
	ac++;
	XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
	ac++;
	if (file_select_dialog == NULL)
	    file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
	      "file_select_dialog", indexparent);
	MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list, ac);
	if (has_card_dir) {
	    CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
	    XtSetArg(arg_list[0], XmNdirMask, CS);
	    XtSetValues((Widget)file_select_dialog, arg_list, 1);
	    XmStringFree(CS);
	}
	XtManageChild((Widget)file_select_dialog);
    } else {
	if (saveaction(NULL)) {
	    nreason = XmCR_CANCEL;
	    clearaction(NULL, NULL, &nreason);
	    file_changed = FALSE;
	}
    }
}

/*
**++
**  ROUTINE NAME: mergeproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure merges a card file with the current card file.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mergeproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    XmString CS;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    file_select_type = MERGE;
    XtSetArg(arg_list[ac], XmNdialogTitle, "MergeTitle");
    ac++;
    XtSetArg(arg_list[ac], XmNvalue, "CardValue");
    ac++;
    XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
    ac++;
    if (file_select_dialog == NULL)
	file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
	  "file_select_dialog", indexparent);
    MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list, ac);
    if (has_card_dir) {
	CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
	XtSetArg(arg_list[0], XmNdirMask, CS);
	XtSetValues((Widget)file_select_dialog, arg_list, 1);
	XmStringFree(CS);
    }
    XtManageChild((Widget)file_select_dialog);
}

/*
**++
**  ROUTINE NAME: update_global_fname
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure updates the global_fname variable. It checks to make
**	sure that no version number is part of it. If there is a version number,
**	it is removed from the global_fname so that storage can be done
**	correctly without prompting the user for a file name without a version
**	number.
**
**  FORMAL PARAMETERS:
**	fname	-	file name to be checked and then placed in the
**			global_fname variable without a version number.
**
**  IMPLICIT INPUTS:
**	global_fname - 	the global_fname used when exiting or saving without
**			user interaction.
**
**  IMPLICIT OUTPUTS:
**	global_fname - 	the global_fname used when exiting or saving without
**			user interaction.
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
update_global_fname(fname)
    char *fname;
{
    char *temp, templabel[MAX_FILE_LEN];
    char label[MAX_FILE_LEN];
    Arg new_args[10];
    int xtnumber;
    XmString cs_title;
    long byte_count, cvt_status;

#ifdef VMS
#define FSCN$_NAME 6
#define FSCN$_VERSION 8

    char *version_number;

    struct {
	unsigned short length1;		/* length of field found */
	unsigned short itemcode1;	/* item to look for */
	unsigned int address1;		/* address of item found */
	unsigned short length2;		/* length of field found */
	unsigned short itemcode2;	/* item to look for */
	unsigned int address2;		/* address of item found */
	int terminator;			/* terminator for descriptor */
    } item_list_2;

    struct dsc$descriptor_s filename;
    int status;

/* Set up the item list to describe what we want from SYS$FILESCAN */

    item_list_2.length1 = 0;
    item_list_2.itemcode1 = FSCN$_VERSION;
    item_list_2.address1 = 0;
    item_list_2.length2 = 0;
    item_list_2.itemcode2 = FSCN$_NAME;
    item_list_2.address2 = 0;
    item_list_2.terminator = 0;

/* Set up the descriptor for the filename to be passed to SYS$FILESCAN */

    filename.dsc$w_length = strlen(fname);
    filename.dsc$b_class = DSC$K_CLASS_S;
    filename.dsc$b_dtype = DSC$K_DTYPE_T;
    filename.dsc$a_pointer = fname;

    if (((status = SYS$FILESCAN(&filename, &item_list_2, 0)) & 1) == 1) {
	if (item_list_2.length1 != 0) {	/* If version number is found */
	    version_number = item_list_2.address1;
	    *version_number = '\0';	/* End at version number */
	}
	if (item_list_2.length2 != 0)
	    strcpy(templabel, item_list_2.address2);
					/* Just want file name */
    }

#else
    /* get filename only, no path */
    strcpy(templabel, fname);
    temp = strrchr(templabel,'/');
    if (temp != 0) {
	strcpy(templabel, temp+1);
    }
#endif

    has_file_name = TRUE;
    expand_filespec(global_fname, fname);

    strcpy(label, cardfiler);
    if (full_path_name)
	strcat(label, global_fname);
    else
	strcat(label, templabel);
    cs_title = DXmCvtFCtoCS(label, &byte_count, &cvt_status);
    DWI18n_SetTitle(indexparent, cs_title);
    DWI18n_SetIconName(indexparent, cs_title);
    XmStringFree(cs_title);
}

/*
**++
**  ROUTINE NAME: expand_filespec
**
**  FUNCTIONAL DESCRIPTION:
**      Expands an input filename into a full filespec.
**
**  FORMAL PARAMETERS:
**      out_fname :  Buffer to get the expanded filespec.
**      in_fname  :  Buffer containing the filename to expand.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void expand_filespec(out_fname, in_fname)
    char *out_fname, *in_fname;
{
#ifdef VMS
    int status;
    struct FAB fab;
    struct NAM nam;
    char file_name[MAX_FILE_LEN];

    /* Allocate and initialize the FAB and NAM with default values */

    fab = cc$rms_fab;
    nam = cc$rms_nam;

    /* Set the appropriate non-default fields */

    fab.fab$l_fna = in_fname;
    fab.fab$b_fns = strlen(in_fname);

/*
    fab.fab$l_dna = _DefaultFileName;
    fab.fab$b_dns = strlen(_DefaultFileName);
*/

    fab.fab$l_nam = &nam;

    nam.nam$l_esa = file_name;
    nam.nam$b_ess = MAX_FILE_LEN - 1;

    /***  Parse the file */

    status = sys$parse(&fab);

    if (!(status & 1)) {
	int vector[3];

	vector[0] = 2;
	vector[1] = fab.fab$l_sts;
	vector[2] = fab.fab$l_stv;

	sys$putmsg(vector);
    }

    /* Get the Expanded file name */
    file_name[nam.nam$b_esl] = 0;
    strcpy(out_fname, file_name);

#else
    char cwd[256];

    if (in_fname[0] != '/') {
	if ((char *) getcwd(cwd, 256) != NULL) {
	    strcpy(out_fname, cwd);
	    strcat(out_fname, "/");
	    strcat(out_fname, in_fname);
	} else
	    strcpy(out_fname, in_fname);
    } else
	strcpy(out_fname, in_fname);

#endif
}

/*
**++
**  ROUTINE NAME: mergeaction
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure does the merging of the files.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mergeaction(fname)
    char *fname;
{
    FILE *fp;

    fp = fopen(fname, READ_BINARY);

    if (fp == NULL)
	display_message("OpenFileError");
    else {
	strcpy(filename, fname);

	WorkCursorDisplay();
	/* Delete all entries beginning from entry 0. */
	DXmSvnDeleteEntries(svnlist, 0, numcards);
	readcards(fp, TRUE);
	LoadListBox();

	file_changed = TRUE;

	if (card_mapped == TRUE)
	    displaycard();
	else {
	    currentselection = first;
	    svn_make_entry_visible(currentselection);
	    card_displayed = NULL;
	}
	WorkCursorRemove();
    }
    XtFree(fname);
}

/*
**++
**  ROUTINE NAME: readgraphicproc
**
**  FUNCTIONAL DESCRIPTION:
**		Reads an X bitmap file.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/

void readgraphicproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if ((bmp_height == 0) && (bmp_width == 0)) {
	deleteaction(NULL, NULL, NULL);
    } else {
	if (delete_dialog == NULL)
	    delete_dialog =
	      (XmMessageBoxWidget) fetch_widget("delete_dialog", cardparent);
	XtManageChild((Widget)delete_dialog);
    }
}

/*
**++
**  ROUTINE NAME: readgraphicaction
**
**  FUNCTIONAL DESCRIPTION:
**		Reads in the bitmap from the file and then calls displaycard
**		to display it.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void readgraphicaction(fname)
    char *fname;
{
    int status;
    int i, number;
    char *temp_str;
    FILE *fp;

    fp = fopen(fname, READ_BINARY);

    if (fp == NULL)
	display_message("OpenFileError");

    else {
	fclose(fp);
	WorkCursorDisplay();
	status = card_Read_DDIF(fname);

	if (status != OK_STATUS)
	    switch (status) {
		case BMP_TOOLARGE:
		    display_message("GraphicTooLarge");
		    break;

		case NOT_VALID_TYPE:
		    display_message("GraphicMultiPlane");
		    break;

		case DDIF_NOGRAPHIC:
		    display_message("DDIFNoGraphic");
		    break;
		default:
		    display_message("DDIFReadError");
		    break;
	    }
	else {
	    XmString cs_temp;
	    long byte_count, cvt_status;

	    cs_temp = DXmCSTextGetString(valuewindow);
	    if (cs_temp == NULL) {
		temp_str = XtMalloc(1);
		temp_str[0] = '\0';
	    } else
		temp_str = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	    XmStringFree(cs_temp);
	    strcpy(text, temp_str);
	    XtFree(temp_str);

	    bitmap_changed = TRUE;

	    displaycard();
	}
	WorkCursorRemove();
    }
    XtFree(fname);
}

/*
**++
**  ROUTINE NAME: delproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure maps the dialog box asking for confirmation before
**	deleting the card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void delproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    struct card *deleteme;
    int rem, save_width;
    int new_index;

    if ((numcards > 0) && (card_displayed != NULL)) {

	WorkCursorDisplay();
	/* Copy deleted card to storage for an undelete. */
	deleteme = card_displayed;
	strcpy(deleted_card.index, deleteme->index);
	strcpy(deleted_card.text, text);
	deleted_card.card_id = deleteme->card_id;
	deleted_card.reserved = deleteme->reserved;

	if ((bmp_height != 0) && (bmp_width != 0)) {
	    rem = bmp_width % 32;
	    if (rem == 0)
		save_width = bmp_width;
	    else
		save_width = bmp_width - rem + 32;
	    mybcopy(bitmap, deleted_card.bitmap,
	      (bmp_height * save_width) / 8);
	}

	deleted_card.bitmap_height = bmp_height;
	deleted_card.bitmap_width = bmp_width;

	XtSetSensitive((Widget)undelete_button, TRUE);
	card_deleted = TRUE;

	if (deleteme->next == last)
	    new_index = first->index_number;
	else
	    new_index = deleteme->next->index_number;

	if (deleteme == currentselection)
	    currentselection = card_displayed;

	delcard(deleteme);
	numcards--;
	if (numcards == 0)
	    gray_no_cards();
	if (numcards == 0) {
	    if (card_mapped == TRUE) {
		closeaction();
	    }
	    file_changed = FALSE;
	    currentselection = NULL;

	    /* Fix bug when deleting last card, new_index should be 0 */
	    new_index = 0;
	}
	file_changed = TRUE;

	card_displayed = NULL;

	selectline(new_index);
	WorkCursorRemove();

    } else
	display_message("NoCardSelectedError");

}

/*
**++
**  ROUTINE NAME: undeleteproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure "undeletes" the last deleted card. If the card
**	was already restored or no card had been deleted then it
**	beeps at you.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
undeleteproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer restored;
    int offset;
    int rem, save_width;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (card_deleted == FALSE)
	display_message("NoDeletedCardError");
    else {
	WorkCursorDisplay();
	XtSetSensitive((Widget)undelete_button, FALSE);
	restored = addcard(deleted_card.index);

	/* restore the card_id and the reserved field. */
	restored->card_id = deleted_card.card_id;
	restored->reserved = deleted_card.reserved;

	bmp_height = deleted_card.bitmap_height;
	bmp_width = deleted_card.bitmap_width;

	if ((bmp_width != 0) && (bmp_height != 0)) {
	    rem = bmp_width % 32;
	    if (rem == 0)
		save_width = bmp_width;
	    else
		save_width = bmp_width - rem + 32;

	    mybcopy(deleted_card.bitmap, bitmap,
	      (bmp_height * save_width) / 8);
	}
	saveonecard(restored, &deleted_card, FALSE);

	strcpy(deleted_card.index, "");
	strcpy(deleted_card.text, "");

	deleted_card.bitmap_height = 0;
	deleted_card.bitmap_width = 0;

	card_deleted = FALSE;
	numcards++;
	if (numcards == 1)
	    gray_no_cards();

	readonecard(restored, &temp_card1);
	bmp_height = temp_card1.bitmap_height;
	bmp_width = temp_card1.bitmap_width;
	strcpy(text, temp_card1.text);

	AddCard(restored);
	SetHighlighting(TRUE);		/* MEMEX */
	displaycardfromindex(restored->index_number);
	WorkCursorRemove();
    }

}

/*
**++
**  ROUTINE NAME: deleteaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the graphics delete dialog and takes action
**	according to what the user wants to do
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void deleteaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int ac = 0;
    Arg arg_list[4];
    long cvtlength, cvtstatus;
    XmString CS;

    if (widget != NULL) {
	lasttime = XtLastTimestampProcessed(dpy);
	set_focus_now(cardparent);
	XtUnmanageChild(widget);
    }

    if ((widget == NULL) || (reason->reason == XmCR_OK)) {
	file_select_type = READGRAPHIC;
	XtSetArg(arg_list[ac], XmNdialogTitle, "ReadGraphicTitle");
	ac++;
	XtSetArg(arg_list[ac], XmNvalue, "GraphicValue");
	ac++;
	if (!has_bmp_dir) {
	    XtSetArg(arg_list[ac], XmNdirMask, "GraphicDirMask");
	    ac++;
	}
	if (card_file_select_dialog == NULL)
	    card_file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
	      "card_file_select_dialog", cardparent);
	MrmFetchSetValues(card_hierarchy, card_file_select_dialog, arg_list,
	  ac);
	if (has_bmp_dir) {
	    CS = DXmCvtFCtoCS(bitmap_dir, &cvtlength, &cvtstatus);
	    XtSetArg(arg_list[0], XmNdirMask, CS);
	    XtSetValues((Widget)card_file_select_dialog, arg_list, 1);
	    XmStringFree(CS);
	}
	XtManageChild((Widget)card_file_select_dialog);
    }
}

/*
**++
**  ROUTINE NAME: exitsave
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure unmaps the exit dialog box and
**	takes the action desired by the user.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void exitsave(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    Arg arg_list[4];
    int ac = 0;
    long cvtlength, cvtstatus;
    XmString CS;

    lasttime = XtLastTimestampProcessed(dpy);
    set_focus_now(indexparent);
    XtUnmanageChild((Widget)exit_dialog);

    switch (reason->reason) {
	case XmCR_OK:
	    if (!has_file_name) {
		file_select_type = EXITSAVE;
		XtSetArg(arg_list[ac], XmNdialogTitle, "SaveTitle");
		ac++;
		XtSetArg(arg_list[ac], XmNvalue, "CardValue");
		ac++;
		XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
		ac++;
		if (file_select_dialog == NULL)
		    file_select_dialog =
		      (XmFileSelectionBoxWidget) fetch_widget(
		      "file_select_dialog", indexparent);
		MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list,
		  ac);
		if (has_card_dir) {
		    CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
		    XtSetArg(arg_list[0], XmNdirMask, CS);
		    XtSetValues((Widget)file_select_dialog, arg_list, 1);
		    XmStringFree(CS);
		}
		XtManageChild((Widget)file_select_dialog);
	    } else {
		if (saveaction(NULL))
		    exitaction();
	    }
	    break;

	case XmCR_CANCEL:
	    break;

	default: 			/* user answered NO! don't save */
	    exitaction();
	    break;
    }
}

/*
**++
**  ROUTINE NAME: exitsaveaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure stores the cards before exiting.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void exitsaveaction(fname)
    char *fname;
{
    FILE *fp;

    fp = fopen(fname, WRITE_BINARY);

    if (fp == NULL)
	display_message("OpenFileError");
    else {
	if (SaveAllCards(fp))
	    exitaction();
    }
    XtFree(fname);
}

/*
**++
**  ROUTINE NAME: exitproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure maps the dialog box maps asking if the user wants 
**	to save the data before exiting if there are cards. Otherwise
**	it just calls exitaction.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void exitproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if ((numcards > 0) && (file_changed == TRUE)) {
	if (exit_dialog == NULL)
	    exit_dialog =
	      (XmMessageBoxWidget) fetch_widget("exit_dialog", indexparent);
	XtManageChild((Widget)exit_dialog);
    } else
	exitaction();
}

/*
**++
**  ROUTINE NAME: exitaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is to handle exiting. It closes the temporary file
**	and then deletes it, the display is closed and then the program
**	exits.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void exitaction()
{

    fclose(tempfile);

/* Delete the temporary file. Need both Ultrix and VMS ways. */
#ifdef VMS
    delete(pid_temp_file);
#else
    unlink(pid_temp_file);
#endif
#ifndef NO_HYPERHELP
    if (hyper_help_context)
	DXmHelpSystemClose(hyper_help_context, display_message,
	  "Help Close Error");
#endif

    MMXexit();				/* MEMEX */
    XCloseDisplay(dpy);
    exit(OK_STATUS);
}

/*
**++
**  ROUTINE NAME: renameindex
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure calls the procedures to rename on the card index.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void renameindex(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char buff[10];
    Arg arg_list[2];
    XmString cs_buff;
    long cvt_status, byte_count;
    MrmCode type;

    WorkCursorDisplay();
    if (widget == NULL)
	FromCreateCard = TRUE;
    else {
	FromCreateCard = FALSE;
	FromIndex = FALSE;
    }

    strcpy(buff, "");

    if (index_dialog == NULL) {
	index_dialog =
	  (XmBulletinBoardWidget) fetch_widget("index_dialog", indexparent);

	card_index_dialog = (XmBulletinBoardWidget) fetch_widget(
	  "card_index_dialog", cardparent);
    } else {
	if (FromIndex) {
	    if (FromCreateCard) {
		cs_buff = DXmCvtFCtoCS(buff, &byte_count, &cvt_status);
		DXmCSTextSetString(index_dialog_text, cs_buff);
		XmStringFree(cs_buff);
	    } else {
		cs_buff = DXmCvtFCtoCS(card_displayed->index, &byte_count,
		  &cvt_status);
		DXmCSTextSetString(index_dialog_text, cs_buff);
		DXmCSTextSetSelection(index_dialog_text, 0,
		  DWI18n_CharCount(card_displayed->index), CurrentTime);
		XmStringFree(cs_buff);
	    }
	} else {
	    if (FromCreateCard) {
		cs_buff = DXmCvtFCtoCS(buff, &byte_count, &cvt_status);
		DXmCSTextSetString(card_index_dialog_text, cs_buff);
		XmStringFree(cs_buff);
	    } else {
		cs_buff = DXmCvtFCtoCS(card_displayed->index, &byte_count,
		  &cvt_status);
		DXmCSTextSetString(card_index_dialog_text, cs_buff);
		DXmCSTextSetSelection(card_index_dialog_text, 0,
		  DWI18n_CharCount(card_displayed->index), CurrentTime);
		XmStringFree(cs_buff);
	    }
	}
    }

    if (FromCreateCard)
	MrmFetchLiteral(card_hierarchy, "CreateTitle", XtDisplay(indexparent),
	  &cs_buff, &type);
    else
	MrmFetchLiteral(card_hierarchy, "RenameTitle", XtDisplay(indexparent),
	  &cs_buff, &type);

    WorkCursorRemove();
    if (FromIndex) {
	DWI18n_SetTitle(index_dialog, cs_buff);
	XmStringFree(cs_buff);
	XtManageChild((Widget)index_dialog);
    } else {
	DWI18n_SetTitle(card_index_dialog, cs_buff);
	XmStringFree(cs_buff);
	XtManageChild((Widget)card_index_dialog);
    }
}

/*
**++
**  ROUTINE NAME: renameaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure removes the create/rename dialog box and puts
**	the new index field on the card after it has been edited
**	and confirmed.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void renameaction(widget, tag, reason)
    Widget widget;
    int *tag;
    XmAnyCallbackStruct *reason;
{
    card_pointer deleteme;
    char *buff, *temp_str;
    int offset;
    long byte_count, cvt_status;

    lasttime = XtLastTimestampProcessed(dpy);

    if (FromIndex)
	XtUnmanageChild((Widget)index_dialog);
    else
	XtUnmanageChild((Widget)card_index_dialog);

    if (*tag == 0) 			/* cancel */
	return;

/*  Commented out to workaround Motif 1.2 problem
    WorkCursorDisplay();
*/
    if (card_displayed != NULL)
	check_and_save(card_displayed);

    {
	XmString cs_temp;

	cs_temp = DXmCSTextGetString(valuewindow);
	if (cs_temp == NULL) {
	    temp_str = XtMalloc(1);
	    temp_str[0] = '\0';
	} else
	    temp_str = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
    }
    strcpy(text, temp_str);
    XtFree(temp_str);


    if (FromIndex) {
	XmString cs_temp;

	cs_temp = DXmCSTextGetString(index_dialog_text);
	if (cs_temp == NULL) {
	    buff = XtMalloc(1);
	    buff[0] = '\0';
	} else
	    buff = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
#ifdef I18N_MULTIBYTE
/* For non-Asian locales, the maximum length of the card index is
 * controlled by the XmNmaxLength resource for CSText widget. 
 * This is defined to be 40 characters in the UIL file. However, 
 * for Asian locales, where it is possible to intermix 1, 2 and 4
 * byte characters, the XmNmaxLength resource is not sufficient. 
 * An XmNmaxLength number of CHARACTERS entered can easily exceed 40 bytes!
 * This is really a defiency in CSText since it does not support a 
 * max byte length resource. It's important to enforce the 40 byte
 * limit since other carfiler processing depends on it and it 
 * makes the cardfiler databases portable across locales. 
 */
	if (strlen(buff) > card_index_max_byte_len) {
	   char *ptr = buff;
	   char *tmp;

	   display_message("CARDNameExceedMaxLen");
	   while (ptr - buff <= card_index_max_byte_len) { 
		tmp = ptr;
		DWI18n_CharIncrement(&ptr);
	   }
	   *tmp = '\0';
	}
#endif /* I18N_MULTIBYTE */		
    } else {
	XmString cs_temp;

	cs_temp = DXmCSTextGetString(card_index_dialog_text);
	if (cs_temp == NULL) {
	    buff = XtMalloc(1);
	    buff[0] = '\0';
	} else
	    buff = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
#ifdef I18N_MULTIBYTE
/* For non-Asian locales, the maximum length of the card index is
 * controlled by the XmNmaxLength resource for CSText widget. 
 * This is defined to be 40 characters in the UIL file. However, 
 * for Asian locales, where it is possible to intermix 1, 2 and 4
 * byte characters, the XmNmaxLength resource is not sufficient. 
 * An XmNmaxLength number of CHARACTERS entered can easily exceed 40 bytes!
 * This is really a defiency in CSText since it does not support a 
 * max byte length resource. It's important to enforce the 40 byte
 * limit since other carfiler processing depends on it and it 
 * makes the cardfiler databases portable across locales. 
 */
	if (strlen(buff) > card_index_max_byte_len) {
	   char *ptr = buff;
	   char *tmp;

	   display_message("CARDNameExceedMaxLen");
	   while (ptr - buff <= card_index_max_byte_len) { 
		tmp = ptr;
		DWI18n_CharIncrement(&ptr);
	   }
	   *tmp = '\0';
	}
#endif /* I18N_MULTIBYTE */		
    }
    if (FromCreateCard) {
	strcpy(text, "");
	bmp_height = 0;
	bmp_width = 0;
	bitmap_changed = FALSE;
	undotype = CARDFILER_NO_UNDO;
	numcards++;
	if (numcards == 1)
	    gray_no_cards();
	XtSetSensitive((Widget)restore_button, FALSE);
    }

    /* save the cardpoint, might need to be used for deletion. */
    deleteme = card_displayed;

    card_displayed = addcard(buff);

    /* For rename operation, copy the ID and reserved field before deleting the
     * old card. If not, then it is a new card creation, assign an ID */
    if (!FromCreateCard) {
	card_displayed->card_id = deleteme->card_id;
	card_displayed->reserved = deleteme->reserved;
	delcard(deleteme);
    } else {
	highest_card_id_used++;
	card_displayed->card_id = highest_card_id_used;
    }
    currentselection = card_displayed;

    displaycard();
    file_changed = TRUE;
    XtFree(buff);

    AddCard(card_displayed);
    SetHighlighting(TRUE);		/* MEMEX */
/*  Commented out to workaround Motif 1.2 problem
    WorkCursorRemove();
*/

}

/*
**++
**  ROUTINE NAME: getcard
**
**  FUNCTIONAL DESCRIPTION:
**	This function takes care of checking to see if the current card has
**   	changed. If so, it saves it in the temporary file. It then reads in
**	the newly requested card.
**
**  FORMAL PARAMETERS:
**	thisone - a pointer to the card to be retrieved.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
getcard(thisone)
    card_pointer thisone;
{
    int rem, save_width;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    readonecard(thisone, &temp_card1);
    bitmap_changed = FALSE;
    bmp_height = temp_card1.bitmap_height;
    bmp_width = temp_card1.bitmap_width;

    if ((temp_card1.bitmap_height != 0) && (temp_card1.bitmap_width != 0)) {
	rem = bmp_width % 32;
	if (rem == 0)
	    save_width = bmp_width;
	else
	    save_width = bmp_width - rem + 32;

	mybcopy(temp_card1.bitmap, bitmap, (bmp_height * save_width) / 8);
    }
    strcpy(text, temp_card1.text);

    if (thisone->orig_fp != NULL)
	XtSetSensitive((Widget)restore_button, TRUE);
    else
	XtSetSensitive((Widget)restore_button, FALSE);

    undotype = CARDFILER_NO_UNDO;
}

/*
**++
**  ROUTINE NAME: open_caution_action
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure calls the procedures to read in the cards.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void open_caution_action(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    XmString CS;
    Boolean AskForFileName = TRUE;

    lasttime = XtLastTimestampProcessed(dpy);
    set_focus_now(indexparent);

    /* save 'file_changed' in case the user cancels operation */
    file_changed_temp = file_changed;

    if (pending_link)
	AskForFileName = FALSE;
    if (widget != NULL) {
	XtUnmanageChild((Widget)open_caution);

	if (reason->reason == XmCR_OK) {
	    pending_open = TRUE;
	    storecards();
	} else if (reason->reason == XmCR_CANCEL) {
	    /* Cancel button pressed */
	    pending_open = FALSE;
	    pending_link = FALSE;
	    AskForFileName = FALSE;
	    file_changed = file_changed_temp;
	} else {
	    /* User answered no, so don't save the cards and get newfile */
	    if (pending_link) {
		pending_link = FALSE;
		file_changed = FALSE;
		display_from_link();
	    }

	    if (AskForFileName) {
		file_changed = FALSE;
		file_select_type = RETRIEVE;
		XtSetArg(arg_list[ac], XmNdialogTitle, "OpenTitle");
		ac++;
		XtSetArg(arg_list[ac], XmNvalue, "CardValue");
		ac++;
		XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
		ac++;
		if (file_select_dialog == NULL)
		    file_select_dialog =
		      (XmFileSelectionBoxWidget) fetch_widget(
		      "file_select_dialog", indexparent);
		MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list,
		  ac);
		if (has_card_dir) {
		    CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
		    XtSetArg(arg_list[0], XmNdirMask, CS);
		    XtSetValues((Widget)file_select_dialog, arg_list, 1);
		    XmStringFree(CS);
		}
		XtManageChild((Widget)file_select_dialog);
	    }				/* end askforfilename */
	}				/* end if (reason != ok) */
    }					/* end if (widget != NULL) */
}

/*
**++
**  ROUTINE NAME: enterfnameaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure called by the enter_fname_dialog, which was brought
**	up by memex.  If the user enter OK, it bring up the file_select_dialog
**	for the user to enter a filename.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void enterfnameaction(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    XmString CS;

    lasttime = XtLastTimestampProcessed(dpy);
    set_focus_now(indexparent);
    if (widget != NULL) {
	XtUnmanageChild(widget);

	if (reason->reason == XmCR_OK) {

	    file_select_type = ENTERFNAME;
	    XtSetArg(arg_list[ac], XmNdialogTitle, "SaveTitle");
	    ac++;
	    XtSetArg(arg_list[ac], XmNvalue, "CardValue");
	    ac++;
	    XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
	    ac++;
	    if (file_select_dialog == NULL)
		file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
		  "file_select_dialog", indexparent);
	    MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list,
	      ac);
	    if (has_card_dir) {
		CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
		XtSetArg(arg_list[0], XmNdirMask, CS);
		XtSetValues((Widget)file_select_dialog, arg_list, 1);
		XmStringFree(CS);
	    }
	    XtManageChild((Widget)file_select_dialog);
	}
    }
}

/*
**++
**  ROUTINE NAME: retrievecards
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure maps the dialog box for the retrieval of the cards.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int retrievecards(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    XmString CS;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    /* save file changed status */
    file_changed_temp = file_changed;
    if (file_changed == TRUE) {
	if (open_caution == NULL)
	    open_caution =
	      (XmMessageBoxWidget) fetch_widget("open_caution", indexparent);
	XtManageChild((Widget)open_caution);
    } else {
	file_select_type = RETRIEVE;
	XtSetArg(arg_list[ac], XmNdialogTitle, "OpenTitle");
	ac++;
	XtSetArg(arg_list[ac], XmNvalue, "CardValue");
	ac++;
	XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
	ac++;
	if (file_select_dialog == NULL)
	    file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
	      "file_select_dialog", indexparent);
	MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list, ac);
	if (has_card_dir) {
	    CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
	    XtSetArg(arg_list[0], XmNdirMask, CS);
	    XtSetValues((Widget)file_select_dialog, arg_list, 1);
	    XmStringFree(CS);
	}
	XtManageChild((Widget)file_select_dialog);
    }
}

/*
**++
**  ROUTINE NAME: retrieveaction
**
**  FUNCTIONAL DESCRIPTION:
**	Get the cards from where ever they are currently residing.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void retrieveaction(fname)
    char *fname;
{
    struct card *current;
    FILE *fp;
    int error = FALSE;
    int new_file = FALSE;

    if (strcmp(fname, global_fname) == 0)
					/* MEMEX */
	return;				/* MEMEX */

    if ((fp = fopen(fname, READ_BINARY, "shr=upd")) == NULL) {
	if ((fp = fopen(fname, WRITE_BINARY)) == NULL) {
	    error = TRUE;
	    display_message("OpenFileError");
	} else {
	    fclose(fp);
	    new_file = TRUE;
#ifdef VMS
	    delete(fname);
#else
	    unlink(fname);
#endif
	    display_message("FileNotFound");
	}
    }

    if (!error) {
	strcpy(filename, fname);
	if (numcards > 0)
	    clearfunc(NULL, NULL, NULL);
	current = first = last = &onecard;
	current->last = NULL;
	current->next = NULL;
	strcpy(current->index, "");
	strcpy(text, "");
	if (!new_file) {
            if (readcards(fp, FALSE)) {
                error = TRUE;
            } else {
                strcpy(filename, fname);
                update_global_fname(fname);     /* MEMEX */
                LoadListBox();
            }
        } /* newfile */
    } /* !error */

    if (!error) {
	if (card_mapped) {
	    currentselection = card_displayed = first;
	    displaycard();
	} else {
	    currentselection = first;
	    svn_make_entry_visible(currentselection);
	    card_displayed = NULL;
	}
    } 
}

/*
**++
**  ROUTINE NAME: saveaction
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is for storing the cards
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int saveaction(fname)
    char *fname;
{
    FILE *fp;
    int error = FALSE;

    if (fname != NULL) {
	fp = fopen(fname, WRITE_BINARY);
	if (fp == NULL) {
	    error = TRUE;
	    display_message("OpenFileError");
	}
    }					/* end (fname != NULL) */
      else {
	fp = fopen(global_fname, WRITE_BINARY);
	if (fp == NULL) {
	    error = TRUE;
	    display_message("OpenFileError");
	}
    }					/* end (fname = NULL) */

    if (!error) {
	if (SaveAllCards(fp)) {
	    if (fname != NULL) {
		strcpy(filename, fname);
		update_global_fname(fname);
	    }
	    file_changed = FALSE;
	    bitmap_changed = FALSE;
	    XtSetSensitive((Widget)restore_button, TRUE);
	} else
	    error = TRUE;
    }

    if (error) {
	pending_link = FALSE;
	pending_open = FALSE;
        pending_link_visit = FALSE;
    }

    /* check if we wanted to visit a link */
    if (pending_link_visit) {
	pending_link_visit = FALSE;
	RedoLink ();
    }

    /* check if there was an incoming linkworks request */
    if (pending_link) {
	pending_link = FALSE;
	pending_open = FALSE;
	display_from_link();
    }

    /* check for a pending open request */
    if (pending_open) {
	pending_open = FALSE;
	retrievecards(NULL, NULL, NULL);
    }
    XtFree(fname);
    return(!error);
}

/*
**++
**  ROUTINE NAME: storecards
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure stores the cards with the current file name.
**	If a filename has not yet been selected, storeascards is called
**	to put up a dialog box and get one.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int storecards(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if (!has_file_name) {
	storeascards(NULL, NULL, NULL);
    } else
	return(saveaction(NULL));

}

/*
**++
**  ROUTINE NAME: storeascards
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure puts up a dialog box which gets a filename and
**	which will then call a procedure to store the cards.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int storeascards(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    long cvtlength, cvtstatus;
    Arg arg_list[4];
    int ac = 0;
    XmString CS;

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    file_select_type = SAVE;
    XtSetArg(arg_list[ac], XmNdialogTitle, "SaveTitle");
    ac++;
    XtSetArg(arg_list[ac], XmNvalue, "CardValue");
    ac++;
    XtSetArg(arg_list[ac], XmNdirMask, "CardDirMask");
    ac++;
    if (file_select_dialog == NULL)
	file_select_dialog = (XmFileSelectionBoxWidget) fetch_widget(
	  "file_select_dialog", indexparent);
    MrmFetchSetValues(card_hierarchy, file_select_dialog, arg_list, ac);
    if (has_card_dir) {
	CS = DXmCvtFCtoCS(card_dir, &cvtlength, &cvtstatus);
	XtSetArg(arg_list[0], XmNdirMask, CS);
	XtSetValues((Widget)file_select_dialog, arg_list, 1);
	XmStringFree(CS);
    }
    XtManageChild((Widget)file_select_dialog);
}

/*
**++
**  ROUTINE NAME: initcardpointers
**
**  FUNCTIONAL DESCRIPTION:
**	Initialize the card pointers to nulls.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
initcardpointers()
{
    first = last = &onecard;
    first->last = NULL;
    first->next = NULL;
    strcpy(first->index, "");
    strcpy(text, "");
    strcpy(deleted_card.index, "");
    strcpy(deleted_card.text, "");
#ifndef NO_FILE_CHANGE
    /* No current selection or card_displayed and the file hasn't changed yet.
     */
    card_displayed = NULL;
    currentselection = NULL;
    file_changed = FALSE;
#endif
}

update_titlebar()
{
    char label[MAX_FILE_LEN];
    XmString cs_title;
    long byte_count, cvt_status;

    /* Set up the label for the index window. */

    strcpy(label, cardfiler);
    strcat(label, default_name);
    cs_title = DXmCvtFCtoCS(label, &byte_count, &cvt_status);
    DWI18n_SetTitle(indexparent, cs_title);
    DWI18n_SetIconName(indexparent, cs_title);
    XmStringFree(cs_title);
}

/*
**++
**  ROUTINE NAME: SaveAllCards
**
**  FUNCTIONAL DESCRIPTION:
**	Save's cards with the Ultrix hack for saving over a name
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int SaveAllCards(fp)
    FILE *fp;
{
int	status;

    WorkCursorDisplay();
    status = savecards(fp);
    WorkCursorRemove();
    if (!status)
	display_error("SaveFileError");
    fclose(fp);
    return(status);
}

/*
**++
**  ROUTINE NAME: card_get_uil_string
**
**  FUNCTIONAL DESCRIPTION:
**	Fetches a UIL string using DRM.  The UI utility routine cannot
**	be used since a context block does not exist at this point.
**
**  FORMAL PARAMETERS:
**	NONE
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
card_get_uil_string(hierarchy, name_str, buffer)
    MrmHierarchy hierarchy;
    char *name_str;
    char *buffer;
{
    Cardinal status;
    caddr_t *value;
    MrmCode *type;

    /* Now get the literal using DRM. */
    status = MrmFetchLiteral(hierarchy, name_str, dpy, &value, &type);

    strcpy(buffer, value);
    XtFree((char*)value);
}

#ifndef VMS
/*
**++
**  ROUTINE NAME: createHomeDirSpec
**
**  FUNCTIONAL DESCRIPTION:
**	Returns a file spec for the named file in the users home directory
**	space needs to be freed with XtFree
**
**  FORMAL PARAMETERS:
**	char *simple;		File name of the desired file in the users
**				home directory
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static char *createHomeDirSpec(simple)
    char *simple;
{
    char *dir = NULL;
    char *spec;
    char *user;
    struct passwd *pw;
    int count = 0;

    while (count < 3) {
	switch (count) {
	    case 0:
		dir = (char *) getenv("HOME");
		break;

	    case 1:
		if ((user = (char *) getenv("USER")) == NULL) {
		    break;
		}
		if ((pw = getpwnam(user)) == NULL) {
		    break;
		}
		dir = pw->pw_dir;
		break;

	    case 2:
		if ((pw = getpwuid(getuid())) == NULL) {
		    break;
		}
		dir = pw->pw_dir;
		break;
	}
	count++;
	if (dir == NULL)
	    continue;
	if (!access(dir, F_OK))
	    break;
    }

    spec = (char *) XtMalloc(strlen(dir) + strlen(simple) + 2);
    strcpy(spec, dir);
    strcat(spec, "/");
    strcat(spec, simple);
    return (spec);
}
#endif

/*
**++
**  ROUTINE NAME: set_icons_on_shell
**
**  FUNCTIONAL DESCRIPTION:
**	Callback routine which sets the icon pixmaps for Reparenting
**	window managers.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void set_icons_on_shell(shell, user_data, event)
    Widget shell;
    caddr_t user_data;			/* unused */
    XEvent *event;
{
    XIconSize *size_list;
    int num_sizes;
    Display *dpy = XtDisplay(shell);
    Window root_window = XDefaultRootWindow(dpy);
    XReparentEvent *reparent = (XReparentEvent *) &event->xreparent;

    if (event->type == MapNotify) {
	if (shell != indexparent) {
	    XSetInputFocus(dpy, XtWindow(shell), RevertToParent, CurrentTime);
	}
    }

    if (event->type != ReparentNotify)
	return;

    /* Ignore reparents back to the root window. */
    if (reparent->parent == root_window)
	return;

    /*  Set the icons for this shell. */
    if (!XGetIconSizes(dpy, root_window, &size_list, &num_sizes))
	return;
    else {
	XFree(size_list);
	set_icons(shell, card_hierarchy, TRUE);
    }
}

/*
**++
**  ROUTINE NAME: set_icons
**
**  FUNCTIONAL DESCRIPTION:
**  	Sets the icon and iconify pixmaps for the given shell widget.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void set_icons(shell, hierarchy_id, check)
    Widget shell;
    MrmHierarchy hierarchy_id;
    Boolean check;
{
    Display *dpy = XtDisplay(shell);
    Screen *scr = XtScreen(shell);
    int ac;
    Arg args[5];
    unsigned int icon_size;
    char *icon_name;
    static char *shell_icon_sizes[] = {"75", "50", "32", "17"};
    static int num_sizes = XtNumber(shell_icon_sizes);
    static unsigned int current_icon_size = 0;
    static Pixmap icon_pixmap = 0;
    static Pixmap iconify_pixmap = 0;

    /* Determine the icon pixmap name and size to fetch. */
    icon_name = get_icon_index_name(dpy, "ICON_PIXMAP", &icon_size,
      shell_icon_sizes, num_sizes);
    if (icon_name != NULL) {

	/* If the icon sizes are different we need to free the current ones,
	 * and re-fetch new icons.  We assume that re-fetching new  icons is an
	 * infrequent operation, so we don't cache the old icons. */
	if ((current_icon_size != 0)	/* Icon exists. */
	  &&(current_icon_size != icon_size))
					/* New icon needed. */
	  {
	    if (icon_pixmap)
		XFreePixmap(dpy, icon_pixmap);
	    if ((iconify_pixmap) && (iconify_pixmap != icon_pixmap))
		XFreePixmap(dpy, iconify_pixmap);
	    icon_pixmap = 0;
	    iconify_pixmap = 0;
	    current_icon_size = 0;
	}
	if (current_icon_size == 0) {
	    current_icon_size = icon_size;
	    icon_pixmap = fetch_bitmap_icon(dpy, scr, icon_size);
	}
	XtFree(icon_name);
	icon_name = NULL;
    } else				/* Can't get icon sizes */
	return;

    /* Fetch the iconify pixmap for compatibility with the XUI window manager.
     */
    if (DXIsXUIWMRunning(shell, check)) {
	if (icon_size == 17)		/* Don't fetch icon twice */
	    iconify_pixmap = icon_pixmap;
	else if (icon_size > 17)
	    iconify_pixmap = fetch_bitmap_icon(dpy, scr, 17);
    }

    /* Set the icon pixmap on the shell. */
    if (icon_pixmap) {
	ac = 0;
	XtSetArg(args[ac], XtNiconPixmap, icon_pixmap);
	ac++;
	XtSetValues(shell, args, ac);
    }

    /* Set the iconify pixmap for the XUI window manager */
    if ((iconify_pixmap) && (XtIsRealized(shell)))
	set_iconify_pixmap(shell, iconify_pixmap);
}

/*
**++
**  ROUTINE NAME: fetch_bitmap_icon
**
**  FUNCTIONAL DESCRIPTION:
**	From the icon_size requested, create a pixmap from the icon bitmap
**	data corresponded to the size requested.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
Pixmap fetch_bitmap_icon(dpy, scr, icon_size)
    Display *dpy;
    Screen *scr;
    int icon_size;
{
    int status;
    Pixmap pixmap_rtn;
    Window root = XDefaultRootWindow(dpy);

    switch (icon_size) {
	case 75:
	    pixmap_rtn =
	      MakePixmap(dpy, root, CARDFILER_75_bits, icon_size, icon_size);
	    break;

	case 50:
	    pixmap_rtn =
	      MakePixmap(dpy, root, CARDFILER_50_bits, icon_size, icon_size);
	    break;

	case 32:
	    pixmap_rtn =
	      MakePixmap(dpy, root, CARDFILER_32_bits, icon_size, icon_size);
	    break;

	case 17:
	    pixmap_rtn =
	      MakePixmap(dpy, root, CARDFILER_17_bits, icon_size, icon_size);
	    break;

    }
    return (Pixmap) pixmap_rtn;
}

/*
**++
**  ROUTINE NAME: get_icon_index_name
**
**  FUNCTIONAL DESCRIPTION:
**	Finds the largest icon supported by the window manager and returns
**	a string which represents that icon in UIL.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static char *get_icon_index_name(dpy, root_index_name, icon_size_rtn,
  supported_icon_sizes, num_supported_sizes)
    Display *dpy;
    char *root_index_name;
    unsigned int *icon_size_rtn;
    char **supported_icon_sizes;
    int num_supported_sizes;
{
    XIconSize *size_list;
    int num_sizes;
    int cursize;
    int i;
    char *icon_index = NULL;
    int icon_size;
    char *icon_size_ptr;
    Boolean found_icon_size = False;

    *icon_size_rtn = 0;			/* Initial value */

    if (!XGetIconSizes(dpy, XDefaultRootWindow(dpy), &size_list, &num_sizes))
	return (NULL);

    /* Find the largest icon supported by the window manager. */
    cursize = 0;
    for (i = 1; i < num_sizes; i++) {
	if ((size_list[i].max_width >= size_list[cursize].max_width) &&
	  (size_list[i].max_height >= size_list[cursize].max_height))
	    cursize = i;
    }
    if ((size_list[cursize].max_width <= 0) ||
      (size_list[cursize].max_height <= 0)) {
	XFree(size_list);
	return (NULL);
    }

    /* Find the largest supported icon. */
    for (i = 0; i < num_supported_sizes; i++) {
	icon_size = atoi(supported_icon_sizes[i]);
	if ((icon_size <= size_list[cursize].max_width) &&
	  (icon_size <= size_list[cursize].max_height)) {
	    icon_size_ptr = supported_icon_sizes[i];
	    found_icon_size = TRUE;
	    break;
	}
    }
    XFree(size_list);

    /*  Build the icon index name       format: root_index_name + "_" +
     * icon_size_ptr + "X" + icon_size_ptr */
    if (found_icon_size) {
	icon_index = (char *) XtMalloc(strlen(root_index_name) + sizeof("_") +
	  (2 * sizeof(icon_size_ptr)) + 1);
					/* for \0 char */
	strcpy(icon_index, root_index_name);
	strcat(icon_index, "_");
	strcat(icon_index, icon_size_ptr);
	strcat(icon_index, "X");
	strcat(icon_index, icon_size_ptr);

	*icon_size_rtn = (unsigned int) icon_size;
    }
    return (icon_index);
}

/*
**++
**  ROUTINE NAME: set_iconify_pixmap
**
**  FUNCTIONAL DESCRIPTION:
**  	Sets the iconify pixmap in the DEC_WM_HINTS property for the
**  	given shell widget.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void set_iconify_pixmap(toplevel, iconify_pixmap)
    Widget toplevel;
    Pixmap iconify_pixmap;
{
    Pixmap		spix;
    Atom		dwm_atom;
    DECWmHintsRec	dwm_hint;
    int			status;

    /*
    ** Check for XUI.
    */
    if (!DXIsXUIWMRunning (toplevel, False)) return;

    /*
    ** Look for dxwm and do iconify pixmap if possible.
    */
    dwm_atom = XmInternAtom (XtDisplay(toplevel), "DEC_WM_HINTS", True);
    if (dwm_atom != None)
    {

	spix = iconify_pixmap;

	dwm_hint.value_mask = DECWmIconifyPixmapMask;
	dwm_hint.iconify_pixmap = spix;
	XChangeProperty
	(
	    XtDisplay(toplevel),
	    XtWindow (toplevel),
	    dwm_atom,
	    dwm_atom,
	    32,
	    PropModeReplace,
	    (unsigned char *)&dwm_hint,
	    9
	);
    }
}

#ifdef VMS
/*
**++
**  ROUTINE NAME: find_file
**
**  FUNCTIONAL DESCRIPTION:
** 	Looks for the specified file.  Returns failure on no find.
** 	Regardless of return status, the result of the file parse is returned
** 	(if requested).
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
/*
 * NOTE: LIB$FIND_FILE has problems with NODE access.  As long as the file
 * parses properly, a success is returned!  Got to find a way around this!.
 *
 *	file_len		: int		: read	: value
 *				: (IN) Length of filename.
 *
 *	file_name		: char []	: read	: ref
 *				: (IN) Filename string.
 *
 *	[def_len]		: int		: read	: value
 *				: (IN) Length of default filename.
 *
 *	[def_name]		: char []	: read	: ref
 *				: (IN) Default filename string.
 *
 *	[in_exp_len]		: int		: read	: value
 *				: (IN) Length of "exp_name" buffer.
 *
 *	[exp_name]		: ASCIZ char [256] : write : ref
 *				: (OUT) Expanded filename string.
 *
 *	[out_exp_len]		: int		: write	: ref
 *				: (OUT) Actual length of expanded string.
 *
 *	[flags]
 *				: (IN) Flags (EICTL$UTL$M_FIND_):
 *				:   STRIP_VERS	- Strips off the file version
 *				:		  number.
 *
 *-----------------------------------------------------------------------------
 *
 * value:
 *	SUC	- file was found.
 *	other	- file was not found.
 *
 * errors:
 *
 *-----------------------------------------------------------------------------
 */

static void find_file(file_len, file_name, def_len, def_name, in_exp_len,
  exp_name, out_exp_len, flags)
    int file_len;
    char *file_name;
    int def_len;
    char *def_name;
    int in_exp_len;
    char *exp_name;
    int *out_exp_len;
    int flags;

{
    VMSDSC file_dsc, def_dsc, exp_dsc;
    long find_ctx, status, status2;
    char *strchr(), dummy_buf[255 + 1], *cptr;
    int len;

    /* Set up descriptors */

    $INIT_VMSDSC(file_dsc, file_len, file_name);
    $INIT_VMSDSC(def_dsc, def_len, def_name);

    if (in_exp_len == 0)
	$INIT_VMSDSC(exp_dsc, sizeof(dummy_buf) - 1, dummy_buf);
    else {

	/* Make sure buffer is not > 255 characters */
	len = (in_exp_len > 255 ? 255 : in_exp_len);
	$INIT_VMSDSC(exp_dsc, len, exp_name);
    }

    /* Make sure there is at least one blank in the string */
    cptr = exp_dsc .dsc$a_pointer;
    cptr[0] = ' ';

    /* Search for the filename */

    find_ctx = 0;
    status = LIB$FIND_FILE(&file_dsc, 	/* (IN) File spec */
      &exp_dsc, 			/* (OUT) Result spec */
      &find_ctx, 			/* (IN/OUT) Wildcard context */
      &def_dsc, 			/* (IN) Default spec */
      0, 				/* (IN) Related spec */
      &status2, 			/* (OUT) Secondary status value */
      &1);				/* (IN) User flags */

    /* And cleanup... */

    LIB$FIND_FILE_END(&find_ctx);

    /* Find the length of the file name */

    if (in_exp_len != 0) {
	char *cptr;

	/* Strip off the version number, if request */
	if (flags) {

	    /* There may not be a semicolon */
	    cptr = strchr(exp_name, ';');
	    if (cptr == 0)
		len = strchr(exp_name, ' ') - exp_name;
	    else
		len = cptr - exp_name;
	} else
	    len = strchr(exp_name, ' ') - exp_name;

	exp_name[len] = '\0';

	/* Return actual length, if requested */
	if (out_exp_len != 0)
	    (*out_exp_len) = len;
    }
}
#endif
