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
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/***********************************************************************
 *
 * $XConsortium: iconmgr.h,v 1.11 89/12/10 17:47:02 jim Exp $
 *
 * Icon Manager includes
 *
 * 09-Mar-89 Tom LaStrange		File Created
 *
 ***********************************************************************/

#ifndef _ICONMGR_
#define _ICONMGR_

typedef struct WList
{
    struct WList *next;
    struct WList *prev;
    struct TwmWindow *twm;
    struct IconMgr *iconmgr;
    Window w;
    Window icon;
    int x, y, width, height;
    int row, col;
    int me;
    Pixel fore, back, highlight;
    unsigned top, bottom;
    short active;
    short down;
} WList;

typedef struct IconMgr
{
    struct IconMgr *next;		/* pointer to the next icon manager */
    struct IconMgr *prev;		/* pointer to the previous icon mgr */
    struct IconMgr *lasti;		/* pointer to the last icon mgr */
    struct WList *first;		/* first window in the list */
    struct WList *last;			/* last window in the list */
    struct WList *active;		/* the active entry */
    TwmWindow *twm_win;			/* back pointer to the new parent */
    struct ScreenInfo *scr;		/* the screen this thing is on */
    Window w;				/* this icon manager window */
    char *geometry;			/* geometry string */
    char *name;
    char *icon_name;
    int x, y, width, height;
    int columns, cur_rows, cur_columns;
    int count;
} IconMgr;

extern int iconmgr_textx;
extern WList *DownIconManager;

extern void CreateIconManagers();
extern IconMgr *AllocateIconManager();
extern void MoveIconManager();
extern void JumpIconManager();
extern WList *AddIconManager();
extern void InsertInIconManager();
extern void RemoveFromIconManager();
extern void RemoveIconManager();
extern void ActiveIconManager();
extern void NotActiveIconManager();
extern void DrawIconManagerBorder();
extern void SortIconManager();
extern void PackIconManager();


#endif /* _ICONMGR_ */
