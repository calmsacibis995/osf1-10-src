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
#ifndef _TMDBLBUF
#define _TMDBLBUF

#include "input.h"	/* for OtherClientsPtr */
#include "dixstruct.h"  /* for TimeStamp */
#include "region.h"

/*
 * per-Buffer data
 */
 
typedef struct _Buffers	*BuffersPtr;

#define SameClient(obj,client) \
	(CLIENT_BITS((obj)->resource) == (client)->clientAsMask)
#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])
#define bClient(b)   (clients[CLIENT_ID(b->id)])

#define ValidEventMasks (ExposureMask|MultibufferClobberNotifyMask|MultibufferUpdateNotifyMask)

typedef struct _Buffer {
    BuffersPtr pBuffers;  /* associated window data */
    Mask	    eventMask;	    /* BufferClobberNotifyMask|ExposureMask|BufferUpdateNotifyMask */
    Mask	    otherEventMask; /* mask of all other clients event masks */
    OtherClientsPtr otherClients;
    int		    number;	    /* index into array */
    int		    side;	    /* alwys Mono */
    int		    clobber;	    /* Unclobbered, PartiallyClobbered, FullClobbered */
    XID	    id;	    /* associated pixmap */
} BufferRec, *BufferPtr;

/*
 * per-window data
 */

typedef struct _Buffers {
    WindowPtr	pWindow;		/* associated window */
    int		numBuffer;		/* count of buffers */
    int		displayedBuffer;	/* currently active buffer */
    int		updateAction;		/* Undefined, Background, Untouched, Copied */
    int		updateHint;		/* Frequent, Intermittent, Static */
    int		windowMode;		/* always Mono */

    TimeStamp	lastUpdate;		/* time of last update */

    unsigned short	width, height;	/* last known window size */
    short		x, y;		/* for static gravity */

    PixmapPtr		pPixmap;	/* associated pixmap */
    BufferPtr	buffers;
    RegionPtr	valid_update_region;	/* region of front buffer that can be
					 * SAVE_UNDER'd and restored correctly.
					 * Any attempts to restore a portion
					 * of the front buffer from backing
					 * store that is not subsumed by this
					 * region will be discarded, and expose
					 * events will be generated instead.
					 * This region is reset to the viewable
					 * regions of the front buffer whenever
					 * a buffer update is performed.
					 */
    
} BuffersRec;


/* Actually, I don't think we're supposed to be able to look at this.
 * But it's not declared static, and we need to.  So we will. So there. */
/* extern long *checkForInput[2];    Already declared in dix.h */

#endif
