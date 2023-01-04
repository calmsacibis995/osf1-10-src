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
 * $XConsortium: omronInit.c,v 1.1 91/06/29 13:48:57 xguest Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "omron.h"
#include "omronFb.h"
#include "omronKbd.h"
#include "omronMouse.h"

OmronFbInfo	omron_fb_info;	

int omronScreenIndex;
static unsigned long omronGeneration;

static Bool omronScreenInit();
static Bool omronGetFbInfo();
static void omronSetConsoleMode();
static void omronResetConsoleMode();

#ifndef luna88k
# ifdef uniosu
#  ifndef luna2
static OmronFbProc omron_fb_proc_2 = {
	omronFsBmCreate,  omronFsBmInit,  omronFsBmGiveUp
};
#  endif
# endif /* uniosu */
static OmronFbProc omron_fb_proc_1 = {
	omronDtBmCreate,  omronDtBmInit,  omronDtBmGiveUp
};
#else /* luna88k */
static OmronFbProc omron_fb_proc_1 = {
	omron88kBmCreate, omron88kBmInit, omron88kBmGiveUp
};
#endif /* luna88k */


int
InitOutput(pScreenInfo, argc, argv)
ScreenInfo 	  *pScreenInfo;
int     	  argc;
char    	  **argv;
{
static PixmapFormatRec  MonoFormats = {
	    1, 1, BITMAP_SCANLINE_PAD,  /* 1-bit deep */
	};
static Bool omronFbInfo 	= FALSE;
static Bool omronDevsCreate = FALSE;
	
	if (!omronFbInfo) {
		if (omronGetFbInfo(&omron_fb_info) != TRUE) 
			FatalError("Can't get framebuffer information.\n");
		omronFbInfo = TRUE;
	}

	pScreenInfo->imageByteOrder     = IMAGE_BYTE_ORDER;
	pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	pScreenInfo->bitmapScanlinePad  = BITMAP_SCANLINE_PAD;
	pScreenInfo->bitmapBitOrder     = BITMAP_BIT_ORDER;

	pScreenInfo->numPixmapFormats = 1;
	pScreenInfo->formats[0]       = MonoFormats;
	
	if (!omronDevsCreate) {
		omronSetConsoleMode();

		if (!(* omron_fb_info.func->CreateProc)(&omron_fb_info)) {
			FatalError("Can't create framebuffer.\n");
		}
		omronDevsCreate = TRUE;
	}

	if(AddScreen(omronScreenInit, argc, argv) < 0) {
		FatalError("Can't add screen\n");
	}
}


static Bool
omronScreenInit(screenIndex, pScreen, argc, argv)
int		screenIndex;
ScreenPtr	pScreen;
int		argc;
char 		**argv;
{
	if (omronGeneration != serverGeneration) {
		if((omronScreenIndex = AllocateScreenPrivateIndex()) <0) {
			ErrorF("AllocateScreenPrivateIndex error.\n");
			return FALSE;
		}
		omronGeneration = serverGeneration;
	}
	pScreen->devPrivates[omronScreenIndex].ptr = (pointer)&omron_fb_info;

	return((* omron_fb_info.func->InitProc)(screenIndex, pScreen, argc, argv));
}


int
InitInput(argc, argv)
    int    argc;
    char   **argv;
{
    DevicePtr p, k;

#ifndef UNUSE_DRV_TIME
	omronInitEventPrvRec();
#endif
    
    p = AddInputDevice(omronMouseProc, TRUE);
    k = AddInputDevice(omronKbdProc, TRUE);

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);

    miRegisterPointerDevice(screenInfo.screens[0], p);

    if (mieqInit (k, p) != TRUE) {
		FatalError("Enqueue init error.\n");
	}

	omronSetIoHandler(omronEnqueueEvents);

#ifndef UNUSE_SIGIO_SIGNAL
	signal(SIGIO, omronSigIOHandler);
#else
    if(RegisterBlockAndWakeupHandlers(NoopDDA, omronWakeupProc, (pointer)0) != TRUE) {
		FatalError("Can't register WakeupHandler\n");
	 }
#endif
}


static Bool
omronGetFbInfo(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	int machine_type;

	if ( fb_type )
	{
		if (strcmp(fb_type, "DT_BM") == 0) {
			omron_fb_info->fb_type = DT_BM;
			omron_fb_info->func  = &omron_fb_proc_1;
		} else if (strcmp(fb_type, "DT_BM8") == 0) {
			omron_fb_info->fb_type = DT_BM8;
			omron_fb_info->func  = &omron_fb_proc_1;
		} else if (strcmp(fb_type, "DT_PLASMA") == 0) {
			omron_fb_info->fb_type = DT_PLASMA;
			omron_fb_info->func  = &omron_fb_proc_1;
		} else if (strcmp(fb_type, "FS_BM") == 0) {	
			omron_fb_info->fb_type = FS_BM;
#if (defined(uniosu) && (! defined(luna2)))
			omron_fb_info->func  = &omron_fb_proc_2;
#endif
		} else if (strcmp(fb_type, "DS_BM") == 0) {
			omron_fb_info->fb_type = DS_BM; 
		} else {
			ErrorF("Unknown framebuffer type.\n");
			return (FALSE);
		}

		if (omron_fb_info->fb_type != DT_PLASMA) {
			omron_fb_info->scr_width = SCREEN_WIDTH;  
			omron_fb_info->scr_height = SCREEN_HEIGHT;  
		} else {
			omron_fb_info->scr_width = PLASMA_SCREEN_WIDTH;  
			omron_fb_info->scr_height = PLASMA_SCREEN_HEIGHT;  
		}

		omron_fb_info->fb_width = FB_WIDTH;  
		omron_fb_info->fb_height = FB_HEIGHT;  
		omron_fb_info->fb_depth = 1;
		return (TRUE);
	}

#ifdef	uniosu
	if (sys9100(S91MACHTYPE,&machine_type) < 0) {
		Error("sys9100 error.");
		return (FALSE);
	}
#else
	if (sysomron(S91MACHTYPE,&machine_type) < 0) {
		Error("sysomron error.");
		return (FALSE);
	}
#endif

	switch(machine_type & (MACH_MACH | MACH_GRAPHIC_BOARD)) {
	case MACH_DT | MACH_BM : /* we call LUNA */ 
		omron_fb_info->fb_type = DT_BM;
		omron_fb_info->func  = &omron_fb_proc_1;
		break;

	case MACH_DT | MACH_PLASMA  : /* LUNA support plasma display */  
		omron_fb_info->fb_type = DT_PLASMA;
		omron_fb_info->func  = &omron_fb_proc_1;
		break;

	case MACH_DS | MACH_BM : /* we call Mr. */
		omron_fb_info->fb_type = DS_BM;
		break;

	case MACH_FS | MACH_BM : /* we call M90 */
		omron_fb_info->fb_type = FS_BM;
#if (defined(uniosu) && (! defined(luna2)))
		omron_fb_info->func  = &omron_fb_proc_2;
#endif
		break;
	default : 
		ErrorF("Can't support machine type. (0x%x)\n",machine_type);
		return (FALSE);
	}

	switch(machine_type & MACH_PLANE){
	case MACH_1_PLANE :
		omron_fb_info->fb_depth = 1;
		break;

	case MACH_4_PLANE :
		omron_fb_info->fb_depth = 4;
		break;

	case MACH_8_PLANE :
		omron_fb_info->fb_depth = 8;
		if(omron_fb_info->fb_type == DT_BM) {
			omron_fb_info->fb_type = DT_BM8;
		}
		break;

	default :
		ErrorF("Can't support plane number. (0x%x)\n", machine_type); 
		return (FALSE);
	}

	if (omron_fb_info->fb_type != DT_PLASMA) {
		omron_fb_info->scr_width  = SCREEN_WIDTH;  
		omron_fb_info->scr_height = SCREEN_HEIGHT;  
	} else {
		omron_fb_info->scr_width  = PLASMA_SCREEN_WIDTH;  
		omron_fb_info->scr_height = PLASMA_SCREEN_HEIGHT;  
	}

	if( scrw )
		omron_fb_info->scr_width  = scrw;
	if( scrh )
		omron_fb_info->scr_height = scrh;

	omron_fb_info->fb_width = FB_WIDTH;  
	omron_fb_info->fb_height = FB_HEIGHT;  

	return(TRUE);
}


static int  NonConsole;

static void
omronSetConsoleMode()
{
	extern	char *ttyname();
	char *p = ttyname(2);
	register int i; 

	if (p && strcmp(p,"/dev/console")!=0) {
		NonConsole = TRUE;
		return;
	} 

	i = fcntl(2, F_GETFL, 0); 
	if(i >=0)
		i = fcntl(2, F_SETFL, i | FNDELAY);
	if (i < 0) {
		FatalError("InitOutput:on console can't put stderr in non-block mode.\n");
	}
}

static void
omronResetConsoleMode()
{
	register int i;

	if ( NonConsole ) {
		return;
	}
	i = fcntl(2, F_GETFL, 0);
	if (i >= 0)
		(void)fcntl(2, F_SETFL, i & ~FNDELAY);
}


void
AbortDDX()
{
	omronResetConsoleMode();
}


void
ddxGiveUp()
{
	omronKbdGiveUp();
	omronMouseGiveUp();
	if (omron_fb_info.func->GiveUpProc != NULL)
		(*omron_fb_info.func->GiveUpProc)(&omron_fb_info);
	omronResetConsoleMode();
}
