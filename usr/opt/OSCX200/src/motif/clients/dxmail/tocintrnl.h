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
	@(#)$RCSfile: tocintrnl.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:02:09 $
*/

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */

/* Includes for modules implementing toc stuff. */

#ifndef _tocinternal_h
#define _tocinternal_h
typedef struct mm *MMPtr;

typedef enum {
    unknown, valid, invalid
} ValidType;

typedef struct _MsgHandleRec {
    Msg msg;
    int refcount;
} MsgHandleRec;

typedef struct _AnnoRec {
    char *type;			/* Type of annotation. */
    int num;			/* Number of messages to annotate. */
    MsgHandle *list;		/* Messages to annotate. */
} AnnoRec, *Anno;


typedef struct _MsgRec {
    MsgHandle	handle;		/* Back pointer to handle for this msg. */
    Toc		toc;		/* Which toc this message is in. */
    Toc		desttoc;	/* Folder to copy or move to (NULL if none) */
    FateType	fate;		/* What will be done to this message */
    XmTextPosition position;	/* Position in the scanfile for this msg. */
    char	*origbuf;	/* Original contents of this msg's scanline. */
    char	*curbuf;	/* The line to show in the toc for this msg. */
    short	length;		/* #/chars for this msg's entry in scanfile. */
    short	msgid;		/* Message id for this message. */
    Boolean	changed;	/* True iff this entry needs to be saved. */
    Boolean	visible;	/* Whether we should show this message. */
    Boolean	temporary;	/* Whether we should delete this message when
				   it is no longer visible. */
    Boolean	reapable;	/* True iff we don't need to keep this
				   composition around. */
    Scrn	*scrn;		/* Scrns showing this message (if any) */
    Cardinal	num_scrns;	/* How many scrns are currently showing msg. */
    MsgType	msgtype;	/* What kind of message (DDIF or text). */
    XmTextSource source;	/* Source (if any) containing this msg.  If
				   DDIF, then this source just contains the
				   headers. */
    MMPtr	mm;		/* Capsar MM record for this msg, if any. */
    XmTextPosition startPos;	/* Where to start the insertion point. */
    Anno	anno;		/* Info on annotations (if any). */
    Boolean	saved;
    Pixmap	buttonPixmap;	/* Pixmap in toc display */
    int		index;		/* Entry Number */
} MsgRec;

typedef struct _TocRec {
    Scrn	*scrn;		/* Scrns containing this table of contents. */
    Cardinal	num_scrns;	/* How many scrns are currently showing toc. */
    char 	*foldername;	/* Folder name for this toc */
    char	*path;		/* Full path to folder's directory. */
    char	*scanfile;	/* Full path to file containing scan. */
    Msg		curmsg;		/* Current msgid for this toc. */
    int		nummsgs;	/* How many info entries we currently have. */
    Msg		*msgs;		/* Array of pointers to info about each msg. */
    int		numsequences;	/* #/sequences defined for this folder. */
    Sequence	*seqlist;	/* Array of pointers to sequences. */
    Sequence 	viewedseq;	/* Seq currently shown (NULL == all msgs) */
    int		numselections;	/* Number of selection ranges in use. */
    int		maxselections;	/* Maximum number of selection ranges
				   available. */
    int		lastPos;	/* Last legal position */
    ValidType	validity;	/* Whether the scan file for this toc is */
				/* up to date. */
    Boolean	needsrepaint;	/* TRUE if we should repaint this toc. */
    Boolean	needscachesave;	/* TRUE if the cache needs saving. */
    Boolean	needslabelupdate;/* TRUE if the toclabel needs repainting. */
    Boolean	stopupdate;	/* Zero if refreshing; nonzero if refreshing is
				   currently inhibited. */
    Boolean	haschanged;	/* Whether it's changed in the process of */
				/* the current commit. */
    Boolean	visible;	/* True if not hidden */
    Boolean	updatepending;	/* True if update is pending */
    short	subFolderCount;	/* number of subfolders */
    char	*incfile;	/* Which file to incorporate from (if any). */
    char        *inchost;       /* Which host to incorporate from (via POP). */
    int		mailpending;	/* True if we're currently displaying
				   mail-pending info for this folder. */
    long	lastreaddate;	/* Last time we read or wrote the cache. */
    Boolean	opened;		/* SVN - True if opened */
    int		index;		/* Entry Number */
    int		level;		/* SVN level number of entries */
} TocRec;

#endif _tocinternal_h
