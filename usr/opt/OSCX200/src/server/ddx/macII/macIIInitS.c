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
 * macIIInit.c --
 *	Initialization functions for screen/keyboard/mouse, etc.
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
 *
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#include    "macII.h"
#include    <servermd.h>
#include    "dixstruct.h"
#include    "dix.h"
#include    "opaque.h"
#include    "mipointer.h"
#include    <compat.h>

extern int macIIMouseProc();
extern void macIIKbdProc();
extern int macIIKbdSetUp();

extern Bool macIIMonoProbe();
extern Bool macIIMonoInit();

extern Bool macIIColorInit();

extern Bool macIISlotProbe();
extern void ProcessInputEvents();

extern void SetInputCheck();

static int video_find();
static int get_video_data();
static int get_all_video_data();

DevicePtr pPointerDevice, pKeyboardDevice;

static int autoRepeatHandlersInstalled; /* FALSE each time InitOutput called */

/*-
 *-----------------------------------------------------------------------
 * SigIOHandler --
 *	Signal handler for SIGIO - input is available.
 *
 * Results:
 *	SigIO is set - ProcessInputEvents() will be called soon.
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static void
SigIOHandler(sig, code, scp)
    int		code;
    int		sig;
    struct sigcontext *scp;
{
    macIIEnqueueEvents ();
}

macIIFbDataRec macIIFbData[] = {
    macIISlotProbe,  	"slot 0",	    neverProbed,
    macIISlotProbe,  	"slot 1",	    neverProbed,
    macIISlotProbe,  	"slot 2",	    neverProbed,
    macIISlotProbe,  	"slot 3",	    neverProbed,
    macIISlotProbe,  	"slot 4",	    neverProbed,
    macIISlotProbe,  	"slot 5",	    neverProbed,
    macIISlotProbe,  	"slot 6",	    neverProbed,
    macIISlotProbe,  	"slot 7",	    neverProbed,
    macIISlotProbe,  	"slot 8",	    neverProbed,
    macIISlotProbe,  	"slot 9",	    neverProbed,
    macIISlotProbe,  	"slot A",	    neverProbed,
    macIISlotProbe,  	"slot B",	    neverProbed,
    macIISlotProbe,  	"slot C",	    neverProbed,
    macIISlotProbe,  	"slot D",	    neverProbed,
    macIISlotProbe,  	"slot E",	    neverProbed,
    /*
     * The following entry provides support for A/UX 1.0 where no
     * slot manager calls were available. After failing to probe
     * all the slots above, InitOutput falls through to this entry
     * identifying a single monochrome screen. It must be last in
     * this table!
     */
     macIIMonoProbe, 	"/dev/console",	    neverProbed,
};

/*
 * NUMSCREENS is the number of supported frame buffers (i.e. the number of
 * structures in macIIFbData which have an actual probeProc).
 */
#define NUMSCREENS (sizeof(macIIFbData)/sizeof(macIIFbData[0]))
#define NUMDEVICES 2

fbFd	macIIFbs[NUMSCREENS];  /* Space for descriptors of open frame buffers */

static PixmapFormatRec	formats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep */
};
#define NUMFORMATS	(sizeof formats)/(sizeof formats[0])

#define NUMSLOTS 16
struct video *video_index[NUMSLOTS];     /* how to find it by slot number */
static struct video video[NUMSLOTS];    /* their attributes */

#define NUMMODES	7		/* 1,2,4,8,16,24,32 */
static struct legal_mode_struct {	/* only 1 and 8 bits are supported */
    int depth;
    int legal;
} legal_modes[NUMMODES] = {{1,1},{2,0},{4,0},{8,1},{16,0},{24,0},{32,0}};

static int screen_depth[MAXSCREENS] = {0,0,0};

static struct video_mode_struct{
	int depth[NUMMODES];
        int mode[NUMMODES];   
	struct video_data  info[NUMMODES];
} video_modes[NUMSLOTS] = {
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
	{{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}};

/*-
 *-----------------------------------------------------------------------
 * InitOutput --
 *	Initialize screenInfo for all actually accessible framebuffers.
 *
 * Results:
 *	screenInfo init proc field set
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */

InitOutput(pScreenInfo, argc, argv)
    ScreenInfo 	  *pScreenInfo;
    int     	  argc;
    char    	  **argv;
{
    int     	  i, index, ac = argc;
    char	  **av = argv;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i=0; i< NUMFORMATS; i++)
    {
        pScreenInfo->formats[i] = formats[i];
    }

    autoRepeatHandlersInstalled = FALSE;

    ParseDepths(argc, argv);

#define SLOT_LO 0x09
    for (i = SLOT_LO, index = 0; i < NUMSCREENS - 1; i++) {
	if ((* macIIFbData[i].probeProc) (pScreenInfo, index, i, argc, argv)) {
	    /* This display exists OK */
	    index++;
	} 
    }

    if (index == 0) {
	if (macIIMonoProbe(pScreenInfo, index, NUMSCREENS - 1, argc, argv)) {
	    index++;
	} 
    }
    if (index == 0)
	FatalError("Can't find any displays\n");

    pScreenInfo->numScreens = index;
}

ParseDepths(argc, argv)
    int		argc;
    char 	**argv;
{
#define SCREEN_LO 	0
#define SCREEN_HI 	(MAXSCREENS - 1)
#define EQSTRING(s1,s2)	(!(strcmp((s1), (s2))))

    int i;
    int j;
    int screen = -1;
    int depth = -1;
    int supported_depth = 0;

    i = 0;
    while (i < argc) {
	if (EQSTRING(argv[i], "-screen")) {
	    if (i <= argc - 4) {
	        screen = atoi(argv[i+1]);
                if ((screen < SCREEN_LO) || (screen > SCREEN_HI)) {
		    ErrorF("Invalid screen number %d; a legal number is %d < screen < %d\n", 
		        screen, SCREEN_LO, SCREEN_HI);
                }

		if (!EQSTRING(argv[i+2], "-depth")) {
		    goto usage;
		}

	        /* is this a depth supported by the server? */
		depth = atoi(argv[i+3]);
		for (j = 0; j < NUMMODES; j++) {
		    if ((legal_modes[j].depth == depth) &&
			(legal_modes[j].legal)) {
			screen_depth[screen] = depth;
			supported_depth = 1;
			break;
		    }
		}
		if (!supported_depth) {
		    ErrorF("Invalid screen depth specified\n", depth);
		}
	    }
	    else {
usage:
		ErrorF("Usage: -screen screennum -depth depthnum\n");
	    }
	    i += 4;
	}
	else {
            i++;
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * InitInput --
 *	Initialize all supported input devices...what else is there
 *	besides pointer and keyboard?
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Two DeviceRec's are allocated and registered as the system pointer
 *	and keyboard devices.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
InitInput(argc, argv)
    int     	  argc;
    char    	  **argv;
{
    DevicePtr p, k;
    static int  zero = 0;
    
    p = AddInputDevice(macIIMouseProc, TRUE);
    k = AddInputDevice(macIIKbdProc, TRUE);

    pPointerDevice = p;
    pKeyboardDevice = k;

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
    miRegisterPointerDevice(screenInfo.screens[0], p);

    setcompat (getcompat() | COMPAT_BSDSIGNALS);
    if (!mieqInit (k, p))
	return FALSE;
    signal(SIGIO, SigIOHandler);
    SetTimeSinceLastInputEvent ();
}

/*-
 *-----------------------------------------------------------------------
 * macIISlotProbe --
 *	Attempt to find and initialize a framebuffer. 
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIISlotProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         i, j, oldNumScreens;
    int		depth;
    int		depth_supported;
    char 	*video_physaddr;
    static char *video_virtaddr = (char *)(120 * 1024 * 1024);
    static long csTable[] =
	{
	    0x0000ffff, 0xffffffff,
	    0x00000000, 0x00000000
	};

    static struct VDEntryRecord vde =
	{
	    (char *) csTable,
	    0,              /* start = 0 */
	    1               /* count = 1 (2 entries) */
	};
    struct VDPgInfo vdp;
    struct CntrlParam pb;
    struct strioctl ctl; /* Streams ioctl control structure */
    int mode_index;
    int fd;


    if (macIIFbData[fbNum].probeStatus == probedAndFailed) {
	return FALSE;
    }

    if (macIIFbData[fbNum].probeStatus == neverProbed) {

	if ((depth = video_find(fbNum)) < 0) {
		macIIFbData[fbNum].probeStatus = probedAndFailed;
		return FALSE;
	}

	/* this slot is a video screen */

	/*
	 * we need to ensure that the video board supports a depth that the 
	 * server supports before physing in the board.
	 */
	depth_supported = 0;
	for (i = 0; i < NUMMODES; i++) {
	    for (j = 0; j < NUMMODES; j++) {
		if ((video_modes[fbNum].depth[i] == legal_modes[j].depth) &&
		        (legal_modes[j].legal)) {
		    depth_supported = 1;
		}
	    }
	}
	if (!depth_supported) {
	    macIIFbData[fbNum].probeStatus = probedAndFailed;
	    ErrorF("Video board in slot %d does not offer a supported depth\n",
		(fbNum - SLOT_LO));
	    return FALSE;
	}

#ifdef VIDEO_MAP_SLOT
	{
		struct video_map_slot vmap;
		struct strioctl ctl; /* Streams ioctl control structure */

		/* map frame buffer to next 16MB segment boundary above 128M */
		video_virtaddr = video_virtaddr + (16 * 1024 * 1024); 
		vmap.map_slotnum = fbNum;
	        vmap.map_physnum = index;
        	vmap.map_virtaddr = video_virtaddr;

		ctl.ic_cmd = VIDEO_MAP_SLOT;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(vmap);
		ctl.ic_dp = (char *)&vmap;
		fd = open("/dev/console", O_RDWR, 0);
		if (fd < 0) {
		    FatalError ("could not open /dev/console");
		} 
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_MAP_SLOT failed");
			(void) close (fd);
			return (FALSE);
		}
		(void) close(fd);
	}
#else
	video_physaddr = video[fbNum].video_base;
	video_virtaddr = video_virtaddr + (16 * 1024 * 1024);

	/*
	 * Note that a unique reference number is provided as the
	 * first argument to phys. School of hard knocks ...
	 */
	ErrorF("macIIInitS.c -- need to take out temp phys() hack.\n");
	if (phys(index, video_virtaddr, 16*1024*1024, video_physaddr) == -1) {
		FatalError ("phys failed, server must run suid root");
		return (FALSE);
	}
#endif /* VIDEO_MAP_SLOT */

    	macIIFbs[index].fb = (pointer)(video_virtaddr + 
		video[fbNum].video_data.v_baseoffset); 

	macIIFbs[index].slot = fbNum;
	macIIFbs[index].installedMap = NULL;

	/* set things up for default mode */
	macIIFbs[index].default_depth = depth;
        macIIFbs[index].info = video[fbNum].video_data;

	/* check if user asks for a specific depth */
        if (screen_depth[index]) {
	    int i;
	    for (i = 0; i < NUMMODES; i++) {
		if (video_modes[fbNum].depth[i] == screen_depth[index]) {
		    macIIFbs[index].default_depth = screen_depth[index];
	            macIIFbs[index].info = video_modes[fbNum].info[i];
		    break;
		}
	    }
	    if (i == NUMMODES) {
		ErrorF("Screen %d does not support %d bits per pixel\n",
		    index, screen_depth[index]);
	    }
	}

	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following. 
     */

    oldNumScreens = pScreenInfo->numScreens;

    {
    /* poke the video board */
	ctl.ic_cmd = VIDEO_CONTROL;
	ctl.ic_timout = -1;
	ctl.ic_len = sizeof(pb);
	ctl.ic_dp = (char *)&pb;

	fd = open("/dev/console", O_RDWR, 0);
	if (fd < 0) {
	    FatalError ("could not open /dev/console");
	} 
	for (mode_index = 0; mode_index < NUMMODES; ) {
	    if (video_modes[fbNum].depth[mode_index] == 
		    macIIFbs[index].default_depth) {
		break;
	    }
	    mode_index++;
	}

#define noQueueBit 0x0200
#define SetMode 0x2
#define SetEntries 0x3 

	vdp.csMode = video_modes[fbNum].mode[mode_index];
	vdp.csData = 0;
	vdp.csPage = 0;
	vdp.csBaseAddr = (char *) NULL;

	pb.qType = fbNum;
	pb.ioTrap = noQueueBit;
	pb.ioCmdAddr = (char *) -1;
	pb.csCode = SetMode;
	* (char **) pb.csParam = (char *) &vdp;

	if (ioctl(fd, I_STR, &ctl) == -1) {
		FatalError ("ioctl I_STR VIDEO_CONTROL failed");
		(void) close (fd);
		return (FALSE);
	}

	(void) close (fd);
	if (pb.qType != 0) {
		FatalError ("ioctl I_STR VIDEO_CONTROL CMD failed");
		return (FALSE);
	}

    }

    macIIBlackScreen(index);

    switch (macIIFbs[index].default_depth) {
	case 1:
	    /* install the black & white color map */
	    fd = open("/dev/console", O_RDWR, 0);
	    if (fd < 0) {
		FatalError ("could not open /dev/console");
	    } 
	    pb.qType = fbNum;
	    pb.ioTrap = noQueueBit;
	    pb.ioCmdAddr = (char *) -1;
	    pb.csCode = SetEntries;
	    * (char **) pb.csParam = (char *) &vde;

	    if (ioctl(fd, I_STR, &ctl) == -1) {
		FatalError ("ioctl I_STR VIDEO_CONTROL failed");
		(void) close (fd);
		return(FALSE);
	    }  
	    (void) close (fd);

    	    i = AddScreen(macIIMonoInit, argc, argv);
	    break;

	case 8:
    	    i = AddScreen(macIIColorInit, argc, argv);
	    break;

	default:
	    FatalError("Encountered bogus depth: %d", fbNum);
	    break;
    }

    return (i >= oldNumScreens);
}

static int
video_find(slot)
int slot;
{
	register struct video *vp;
	register struct video_data *vdp;
	int depth;

	vp = &video[slot];


	/*
	 *	If it isn't a video card, ignore it.  Otherwise, get
	 *		the driver and video parameter block from the
	 *		slot ROM.
	 */
	
	vp->video_base = (char *)(0xf0000000 | (slot<<24));
	vp->dce.dCtlSlot = slot;
	vp->dce.dCtlDevBase = (long) vp->video_base;
	vp->dce.dCtlExtDev = 0;
	if ((depth = get_video_data(vp)) < 0)
		return(-1);
	vdp = &vp->video_data;
	vp->video_mem_x = 8*vdp->v_rowbytes;
	vp->video_mem_y = vdp->v_bottom - vdp->v_top;
	vp->video_scr_x = vdp->v_right - vdp->v_left;
	vp->video_scr_y = vdp->v_bottom - vdp->v_top;
	vp->video_addr = vp->video_base + vdp->v_baseoffset;
	video_index[slot] = vp;
	vp->video_slot = slot;

	return(depth);
}

/*
    This routine uses the Slot Manager to find a video devices default
mode and corresponding video parameter block.  It returns zero upon
success and a slot manager error code upon failure.  This code is
pretty much stolen from the Monitors Desk Accessory.
    We search through the list of video parameter blocks for the one
with the smallest bits/pixel.  For most devices, this will be the
first on in the list.
    This routine fills in the video_data and video_def_mode fields of the
video structure.  It also fills in the dCtlSlotId field of the dce which
is a magic number understood by only a few people on earth.  These people
have gained this knowledge only by promising to remove their tongues.
*/

#include "sys/slotmgr.h"
static int noSlotMgr;

static void
SigSYSHandler(sig, code, scp)
    int		code;
    int		sig;
    struct sigcontext *scp;
{
    noSlotMgr++;
}

static int get_video_data(vp)
register struct video *vp;
{
	int depth = 1024;
	int default_mode = 0x80;/* video modes normally default to 0x80 */
	int err;		/* last slot manager result */
	int success = 0;	/* assume failure */
	struct SpBlock pb;
	struct video_data *vd;
	caddr_t slotModesPointer;
	int nextMode;

	pb.spSlot = vp->dce.dCtlSlot;
	pb.spID = 0;
	pb.spCategory = 3;	/* catDisplay */
	pb.spCType = 1;		/* typeVideo */
	pb.spDrvrSW = 1;	/* drSwApple */
	pb.spTBMask = 1;

	noSlotMgr = 0;
	signal(SIGSYS, SigSYSHandler);
	err = slotmanager(_sNextTypesRsrc,&pb);
	signal(SIGSYS, SIG_DFL);
	if (noSlotMgr) return(-1);

	if (err == 0 && pb.spSlot != vp->dce.dCtlSlot)
	    err = smNoMoresRsrcs;
	else if (err == 0) {
		vp->dce.dCtlSlotId = pb.spID;
		slotModesPointer = pb.spsPointer;
		for (nextMode = 0x80; depth != 1 && !err; nextMode++) {
			pb.spID = nextMode;
			pb.spsPointer = slotModesPointer;
			err = slotmanager(_sFindStruct,&pb);
			if (err == 0) {
				pb.spID = 1;	/* mVidParams */
				pb.spResult = 
				    (long)malloc(sizeof(struct video_data));
				err = slotmanager(_sGetBlock,&pb);
				if (err == 0) {
					vd = (struct video_data *) pb.spResult;
					if (vd->v_pixelsize < depth) {
						depth = vd->v_pixelsize;
						default_mode = nextMode;
						vp->video_data = *vd;
						success = 1;
					}
					else free(pb.spResult);
				}
				else free(pb.spResult);
			}
		}
	}
	vp->video_def_mode = default_mode;
	get_all_video_data(vp);
	return success? depth: -abs(err);
}

static int get_all_video_data(vp)
register struct video *vp;
{
	int depth = 1024;
	int default_mode = 0x80;/* video modes normally default to 0x80 */
	int err;		/* last slot manager result */
	int success = 0;	/* assume failure */
	struct SpBlock pb;
	struct video_data *vd;
	caddr_t slotModesPointer;
	int nextMode;
	int i;
	int slot;

	pb.spSlot = vp->dce.dCtlSlot;
	pb.spID = 0;
	pb.spCategory = 3;	/* catDisplay */
	pb.spCType = 1;		/* typeVideo */
	pb.spDrvrSW = 1;	/* drSwApple */
	pb.spTBMask = 1;

        slot = vp->dce.dCtlSlot;

	noSlotMgr = 0;
	signal(SIGSYS, SigSYSHandler);
	err = slotmanager(_sNextTypesRsrc,&pb);
	signal(SIGSYS, SIG_DFL);
	if (noSlotMgr) return(-1);

	if (err == 0 && pb.spSlot != vp->dce.dCtlSlot)
	    err = smNoMoresRsrcs;
	else if (err == 0) {
	    vp->dce.dCtlSlotId = pb.spID;
	    slotModesPointer = pb.spsPointer;
	    for (nextMode = 0x80, i=0; ((!err) && (i < NUMMODES)); nextMode++) {
		pb.spID = nextMode;
		pb.spsPointer = slotModesPointer;
		err = slotmanager(_sFindStruct,&pb);
		if (err == 0) {
		    pb.spID = 1;	/* mVidParams */
		    pb.spResult = (long)(&video_modes[slot].info[i]);
		    err = slotmanager(_sGetBlock,&pb);
		    if (err == 0) {
		        vd = (struct video_data *) pb.spResult;
		        video_modes[slot].depth[i] = vd->v_pixelsize;
		        video_modes[slot].mode[i] = nextMode;
			i++;
		    }
		}
	    }
	}
	return 0;
}

/*-
 *-------------------------------------------------------------%---------
 * macIIScreenInit --
 *	Things which must be done for all types of frame buffers...
 *	Should be called last of all.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The graphics context for the screen is created. The CreateGC,
 *	CreateWindow and ChangeWindowAttributes vectors are changed in
 *	the screen structure.
 *
 *	Both a BlockHandler and a WakeupHandler are installed for the
 *	first screen.  Together, these handlers implement autorepeat
 *	keystrokes on the macII.
 *
 *-----------------------------------------------------------------------
 */
Bool
macIIScreenInit (pScreen)
    ScreenPtr	  pScreen;
{
    extern void   macIIBlockHandler();
    extern void   macIIWakeupHandler();
    static ScreenPtr autoRepeatScreen;
    extern miPointerScreenFuncRec   macIIPointerCursorFuncs;

    /*
     *	Block/Unblock handlers
     */
    if (autoRepeatHandlersInstalled == FALSE) {
	autoRepeatScreen = pScreen;
	autoRepeatHandlersInstalled = TRUE;
    }

    if (pScreen == autoRepeatScreen) {
        pScreen->BlockHandler = macIIBlockHandler;
        pScreen->WakeupHandler = macIIWakeupHandler;
    }

    miDCInitialize (pScreen, &macIIPointerCursorFuncs);

    return TRUE; 
}

/*-
 *-----------------------------------------------------------------------
 * macIIOpenFrameBuffer --
 *	Open a frame buffer  through the /dev/console interface.
 *
 * Results:
 *	The fd of the framebuffer.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIOpenFrameBuffer(expect, pfbType, index, fbNum, argc, argv)
    int	    	  expect;   	/* The expected type of framebuffer */
    fbtype 	  *pfbType; 	/* Place to store the fb info */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  index;    	/* Screen index */
    int	    	  argc;	    	/* Command-line arguments... */
    char	  **argv;   	/* ... */
{
    int           fd = -1;	    	/* Descriptor to device */
    struct strioctl ctl;

    fd = open("/dev/console", O_RDWR, 0);
    if (fd < 0) {
	return (-1);
    } 

    ctl.ic_cmd = VIDEO_DATA;
    ctl.ic_timout = -1;
    ctl.ic_len = sizeof(fbtype);
    ctl.ic_dp = (char *)pfbType;
    if (ioctl(fd, I_STR, &ctl) < 0) {
        FatalError("Failed to ioctl I_STR VIDEO_DATA.\n");
	(void) close(fd);
        return(!Success);
    }

    return (fd);
}

/*-
 *-----------------------------------------------------------------------
 * macIIBlackScreen --
 *    Fill a frame buffer with the black pixel.
 *
 * Results:
 *    None
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
int
macIIBlackScreen(index)
      int index;
{
    fbFd *pf;
    register unsigned char* fb;
    register int fbinc, line, lw;
    register unsigned int *fbt;

    pf = &macIIFbs[index];
    fb = pf->fb; /* Assumed longword aligned! */

    switch (pf->info.v_pixelsize) {
    case 1:
    {
      fbinc = pf->info.v_rowbytes;
        for (line = pf->info.v_top; line < pf->info.v_bottom; line++) {
          fbt = (unsigned int *)fb;
          lw = ((pf->info.v_right - pf->info.v_left) + 31) >> 5;
          do {
              *fbt++ = 0xffffffff;
          } while (--lw);
          fb += fbinc;
      }
      break;
    }
    case 8:
    {
      fbinc = pf->info.v_rowbytes;
        for (line = pf->info.v_top; line < pf->info.v_bottom; line++) {
          fbt = (unsigned int *)fb;
          lw = ((pf->info.v_right - pf->info.v_left) + 3) >> 2;
          do {
              *fbt++ = 0x01010101;
          } while (--lw);
          fb += fbinc;
      }
      break;
    }
    default:
      ErrorF("Bad depth in macIIBlackScreen.");
      break;
    }
}

void
AbortDDX()
{
    extern int consoleFd, devosmFd;

    if (devosmFd > 0) close(devosmFd);
    if (consoleFd > 0) {
	macIIKbdSetUp(consoleFd, FALSE); /* Must NOT FatalError() anywhere! */
        close(consoleFd);
	consoleFd = 0;
    }
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
}

int
ddxProcessArgument (argc, argv, i)
    int       argc;
    char *argv[];
    int       i;
{
    if ( strcmp ( argv[i], "-screen" ) == 0)
	return 4;
    else return 0;
}

void
ddxUseMsg()
{
    ErrorF("-screen # -depth #      run screen (0-5) at depth {1,8}\n");
}

void
MessageF(s)
char *s;
{
	ErrorF(s);
}

/* AUX version of ffs does bits in the wrong order, silly them */

int
ffs(mask)
unsigned int	mask;
{
    register i;

    if ( ! mask ) return 0;
    i = 1;
    while (! (mask & 1)) {
	i++;
	mask = mask >> 1;
    }
    return i;
}
