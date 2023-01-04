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
/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/*-
 * macII.h --
 *	Internal declarations for the macII ddx interface
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
 * $XConsortium: macII.h,v 1.17 91/05/14 16:25:08 rws Exp $
 */
#ifndef _MACII_H_
#define _MACII_H_

#include    <errno.h>
#include    <sys/param.h>
#include    <sys/types.h>
#include    <sys/time.h>
#include    <sys/file.h>
#include    <sys/signal.h>
#include    <sys/stropts.h>

/*
 * Under A/UX 2.0 struct video has been "Macintized" and now incorporates
 * a structure called AuxDCE which is defined once and for all in
 * /usr/include/mac. Alas the definition for AuxDCE requires wheeling in
 * lots of Mac stuff including QuickDraw. This is a headache as there are
 * a variety of clashes between X and QuickDraw (they both do windows after
 * all). So we define just what we need here and avoid pulling in the Mac
 * includes. Of course if this ever changes ...
 */

#ifndef __OSUTILS__
struct QElem {
    struct QElem *qLink;
    short qType;
    short qData[1];
};

typedef struct QElem QElem;

typedef QElem *QElemPtr;

struct QHdr {
    short qFlags;
    QElemPtr qHead;
    QElemPtr qTail;
};
#define __OSUTILS__
#endif

#ifndef __DEVICES__
struct CntrlParam {
        struct QElem *qLink;
        short qType;
        short ioTrap;
        char *ioCmdAddr;
        int (*ioCompletion)();
        short ioResult;
        char *ioNamePtr;
        short ioVRefNum;
        short ioCRefNum;
        short csCode;
        short csParam[11];
};

struct DCtlEntry {
        char **dCtlDriver;
        short dCtlFlags;
        struct QHdr dCtlQHdr;
        long dCtlPosition;
        char **dCtlStorage;
        short dCtlRefNum;
        long dCtlCurTicks;
        char *dCtlWindow;
        short dCtlDelay;
        short dCtlEMask;
        short dCtlMenu;
        char dCtlSlot;
        char dCtlSlotId;
        long dCtlDevBase;
        long reserved;
        char dCtlExtDev;
        char fillByte;
};

struct AuxDCE {
        char **dCtlDriver;
        short dCtlFlags;
        struct QHdr dCtlQHdr;
        long dCtlPosition;
        char **dCtlStorage;
        short dCtlRefNum;
        long dCtlCurTicks;
        char *dCtlWindow;
        short dCtlDelay;
        short dCtlEMask;
        short dCtlMenu;
        char dCtlSlot;
        char dCtlSlotId;
        long dCtlDevBase;
        long reserved;
        char dCtlExtDev;
        char fillByte;
};

#define __DEVICES__
#endif

#include    <sys/video.h>

#include    "X.h"
#include    "Xproto.h"
#include    "scrnintstr.h"
#include    "screenint.h"
#ifdef NEED_EVENTS
#include    "inputstr.h"
#endif /* NEED_EVENTS */
#include    "input.h"
#include    "cursorstr.h"
#include    "cursor.h"
#include    "pixmapstr.h"
#include    "pixmap.h"
#include    "windowstr.h"
#include    "gc.h"
#include    "gcstruct.h"
#include    "regionstr.h"
#include    "colormap.h"
#include    "miscstruct.h"
#include    "dix.h"
#include    "mfb.h"
#include    "mi.h"

/*
 * MAXEVENTS is the maximum number of events the mouse and keyboard functions
 * will read on a given call to their GetEvents vectors.
 */
#define MAXEVENTS 	32

/*
 * Data private to any macII keyboard.
 *	ProcessEvent processes a single event and gives it to DIX
 *	DoneEvents is called when done handling a string of keyboard
 *	    events or done handling all events.
 *	devPrivate is private to the specific keyboard.
 */
#define KEY_DETAIL(e) 	((e) & 0x7f)
#define KEY_UP(e)     	((e) & 0x80)

#define MS_LEFT 	1
#define MS_MIDDLE 	2
#define MS_RIGHT 	3

#define MOUSE_ESCAPE 	0x7e			/* from <sys/video.h> */
#define PSEUDO_MIDDLE_1 0x3b 			/* Left Arrow Key */
#define PSEUDO_RIGHT_1 	0x3c 			/* Right Arrow Key */
#define PSEUDO_MIDDLE_2 PSEUDO_MIDDLE_1		/* extra defs just in case */
#define PSEUDO_RIGHT_2 	PSEUDO_RIGHT_1

#define IS_MIDDLE_KEY(c) 			\
	((KEY_DETAIL(c) == PSEUDO_MIDDLE_1) || 	\
	 (KEY_DETAIL(c) == PSEUDO_MIDDLE_2))

#define IS_RIGHT_KEY(c) 			\
      	((KEY_DETAIL(c) == PSEUDO_RIGHT_1) || 	\
	 (KEY_DETAIL(c) == PSEUDO_RIGHT_2))

#define IS_MOUSE_KEY(c)     			\
	(IS_MIDDLE_KEY(c) || IS_RIGHT_KEY(c))


#define KBTYPE_MACII 	0

typedef struct kbPrivate {
    int	    	  type;           	/* Type of keyboard */
    void    	  (*EnqueueEvent)();	/* Function to process an event */
    int		  offset;		/* to be added to device keycodes */
    KeybdCtrl     *ctrl;                /* Current control structure (for
                                         * keyclick, bell duration, auto-
                                         * repeat, etc.) */
} KbPrivRec, *KbPrivPtr;

#define	MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */

/*
 * Data private to any macII pointer device.
 *	ProcessEvent and DoneEvents have uses similar to the
 *	    keyboard fields of the same name.
 *	pScreen is the screen the pointer is on (only valid if it is the
 *	    main pointer device).
 *	x and y are absolute coordinates on that screen (they may be negative)
 */
typedef struct ptrPrivate {
    void    	  (*EnqueueEvent)();	/* Function to process an event */
    ScreenPtr	  pScreen;  	    	/* Screen pointer is on */
} PtrPrivRec, *PtrPrivPtr;

/*
 * Frame-buffer-private info.
 *	fb  	  	pointer to the mapped image of the frame buffer. Used
 *	    	  	by the driving routines for the specific frame buffer
 *	    	  	type.
 *	slot
 *	default_depth
 *	installedMap
 *	info	  	description of the frame buffer -- type, height, depth,
 *	    	  	width, etc.
 *	fbPriv	  	Data private to the frame buffer type.
 */

#define FBTYPE_MACII 0

typedef struct video_data fbtype;

typedef struct {
    pointer 	  	fb; 	    /* Frame buffer itself */
    int			slot;
    int			default_depth;
    ColormapPtr		installedMap;
    fbtype 		info;	    /* Frame buffer characteristics */
    pointer 	  	fbPriv;	    /* Frame-buffer-dependent data */
} fbFd;

extern fbFd 	  macIIFbs[];

/*
 * Data describing each type of frame buffer. The probeProc is called to
 * see if such a device exists and to do what needs doing if it does. devName
 * is the expected name of the device in the file system. 
 */
typedef enum {
	neverProbed, probedAndSucceeded, probedAndFailed
} macIIProbeStatus;

typedef struct _macIIFbDataRec {
    Bool    (*probeProc)();	/* probe procedure for this fb */
    char    *devName;		/* device filename */
    macIIProbeStatus probeStatus;	/* TRUE if fb has been probed successfully */
} macIIFbDataRec;

extern macIIFbDataRec macIIFbData[];

/*
 * Initialization
 */
extern Bool 	  macIIScreenInit();
extern int  	  macIIOpenFrameBuffer();

extern int  	  macIICheckInput;    /* Non-zero if input is available */

extern int  	  lastEventTime;    /* Time (in ms.) of last event */
extern void 	  SetTimeSinceLastInputEvent();

#define AUTOREPEAT_INITIATE     (400)           /* milliseconds */
#define AUTOREPEAT_DELAY        (50)           /* milliseconds */
/*
 * We signal autorepeat events with the unique id AUTOREPEAT_EVENTID.
 */
#define AUTOREPEAT_EVENTID      (0x7d)          /* AutoRepeat id */

/*-
 * TVTOMILLI(tv)
 *	Given a struct timeval, convert its time into milliseconds...
 */
#define TVTOMILLI(tv)	(((tv).tv_usec/1000)+((tv).tv_sec*1000))

#define tvminus(tv, tv1, tv2) /* tv = tv1 - tv2 */ \
              if ((tv1).tv_usec < (tv2).tv_usec) { \
                      (tv1).tv_usec += 1000000; \
                      (tv1).tv_sec -= 1; \
              } \
              (tv).tv_usec = (tv1).tv_usec - (tv2).tv_usec; \
              (tv).tv_sec = (tv1).tv_sec - (tv2).tv_sec;

#define tvplus(tv, tv1, tv2)  /* tv = tv1 + tv2 */ \
              (tv).tv_sec = (tv1).tv_sec + (tv2).tv_sec; \
              (tv).tv_usec = (tv1).tv_usec + (tv2).tv_usec; \
              if ((tv).tv_usec > 1000000) { \
                      (tv).tv_usec -= 1000000; \
                      (tv).tv_sec += 1; \
              }

#define gettimeofday(time, timezone) _gettimeofday(time)

#define LookupPointerDevice()	pPointerDevice
#define LookupKeyboardDevice()	pKeyboardDevice
extern DevicePtr pPointerDevice, pKeyboardDevice;

#endif /* _MACII_H_ */
