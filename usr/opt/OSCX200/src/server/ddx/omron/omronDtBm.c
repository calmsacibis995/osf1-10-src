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
 * $XConsortium: omronDtBm.c,v 1.1 91/06/29 13:48:53 xguest Exp $
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

#ifndef luna88k
#include "omron.h" 

#include "omronFb.h" 

/*
**	color palette 
*/
struct pal{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

static struct pal paldata[256] = {
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0}
};

static struct pal palinit[256] = {
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0},
	{~0,~0,~0}, {0,0,0}
};

static struct pal palsave[256];

static int	  pagesize;

static Bool omronDtBmPalCreate();
static void omronDtBmPalInit();
static void omronDtBmGetPal();
static void omronDtBmSetPal();

Bool
omronDtBmCreate(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	DtBmMapPtr dt_bm_reg = (DtBmMapPtr)0xb1000000;	/* frame buffer i/o port */

	/* Set transparent Mapping. */
#ifdef uniosu
	if(sys9100(S91TPTRE, 0x80, 0x3f, 0) < 0) {
		return FALSE;
	}
#else
	if(sysomron(S91TPTRE, 0x80, 0x3f, 0) < 0) {
		return FALSE;
	}
#endif

	omron_fb_info->plane = (char *)0xb10c0008;	


	omron_fb_info->fbmap = (char *)dt_bm_reg;
	omron_fb_info->refresh_reg = &(dt_bm_reg->refresh.reg);

	if (!omronDtBmPalCreate(omron_fb_info)) {
		return FALSE;
	}

	if (omron_fb_info->fb_type == DT_PLASMA)
		*(omron_fb_info->refresh_reg) = 0x100000;
	else if (omron_fb_info->fb_type == DT_BM8)
		*(omron_fb_info->refresh_reg) = 0xff07f7e6;
	else
		*(omron_fb_info->refresh_reg) = 0xff07f7e5;

	return TRUE;
}


static Bool
omronDtBmSaveScreen(pScreen, on)
ScreenPtr     pScreen;
Bool          on;
{
static int omronScreenIsSaved = FALSE;
	struct pal savepal[256]; 	
	OmronFbInfoPtr pFbInfo =
		(OmronFbInfoPtr)pScreen->devPrivates[omronScreenIndex].ptr;

	if (on != SCREEN_SAVER_ON) {
		omronSetLastEventTime();
		if(omronScreenIsSaved == TRUE) {
			omronDtBmSetPal(pFbInfo, palsave);
			omronScreenIsSaved = FALSE;
		}
		return TRUE;
	}

	bzero(savepal, sizeof(savepal));
	omronDtBmSetPal(pFbInfo, savepal);
	omronScreenIsSaved = TRUE;
	return TRUE;
}

Bool
omronDtBmInit(index, pScreen, argc, argv)
    int           index;
    ScreenPtr     pScreen;
    int           argc;
    char          **argv;
{
	OmronFbInfoPtr pFbInfo;
	extern miPointerScreenFuncRec  omronPointerScreenFuncs;
	
	pFbInfo = (OmronFbInfoPtr) pScreen->devPrivates[omronScreenIndex].ptr;

	omronDtBmPalInit(pFbInfo);

	if (!monitorResolution) {
		if (pFbInfo->fb_type == DT_PLASMA)
			monitorResolution = PLASMA_TV_RESOLUTION;
		else
			monitorResolution = MONO_TV_RESOLUTION; 
	}

	if(!mfbScreenInit(pScreen,(pointer)pFbInfo->plane,
			pFbInfo->scr_width,pFbInfo->scr_height,
			monitorResolution,monitorResolution,pFbInfo->fb_width)){
		ErrorF("mfbScreenInit error.\n");
		return FALSE;
	}

   	pScreen->whitePixel = 0;
   	pScreen->blackPixel = 1;

	miDCInitialize (pScreen, &omronPointerScreenFuncs);

	mfbCreateDefColormap(pScreen);

	pScreen->SaveScreen = omronDtBmSaveScreen;

	omronDtBmSaveScreen(pScreen, SCREEN_SAVER_FORCER);
	
	return TRUE;

}

static void
omronDtBmPalClose(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	union palette_bm8 *dt_bm8_pal;

	omronDtBmSetPal(omron_fb_info, paldata);

	if(omron_fb_info->fb_type == DT_BM8) {
		dt_bm8_pal = (union palette_bm8 *)(omron_fb_info->palmap);
		dt_bm8_pal->reg.addr = 0x4L;
		dt_bm8_pal->reg.command = 0x1L;
	} else if(omron_fb_info->fb_type == DT_PLASMA) {
		*(omron_fb_info->refresh_reg) = 0;
	}

	if (munmap(omron_fb_info->palmap, omron_fb_info->palmapsize) < 0) {
		Error("Can't munmap palette.");
	}

	free(omron_fb_info->palmap);
	(void)close(omron_fb_info->palfd);
}


static Bool
omronDtBmPalCreate(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	union palette_bm8 *dt_bm8_pal;

	if ((omron_fb_info->palfd = open("/dev/palette",2)) < 0) {
		Error("Can't open /dev/palette");
		return FALSE;
	}

#ifndef uniosu
	pagesize = getpagesize();
#endif

	if(omron_fb_info->fb_type == DT_BM8) {	/* BM8 */
#ifndef uniosu
		omron_fb_info->palmapsize =
			(sizeof(union palette_bm8) + (pagesize - 1)) & ~(pagesize - 1);
#else
		omron_fb_info->palmapsize = sizeof(union palette_bm8);
#endif
	} else {	/* BM */
#ifndef uniosu
		omron_fb_info->palmapsize =
			(sizeof(union palette) + (pagesize - 1)) & ~(pagesize - 1);
#else
		omron_fb_info->palmapsize = sizeof(union palette);
#endif
	}

	omron_fb_info->palmap = valloc(omron_fb_info->palmapsize);

	if(omron_fb_info->palmap  == (char *) NULL) {
		Error("Can't allocate palette.");
		return FALSE;	
	}

	if (mmap(omron_fb_info->palmap, omron_fb_info->palmapsize,
			 (PROT_WRITE | PROT_READ),MAP_SHARED, omron_fb_info->palfd, 0) < 0) { 
		Error("Can't mmap palette.");
		free(omron_fb_info->palmap);
		return FALSE;
	}	

	if(ioctl(omron_fb_info->palfd, PLTIOCUND, 0) < 0) {
		Error("ioctl PLTIOCUND error.");
		return FALSE;
	}

	if(omron_fb_info->fb_type == DT_BM8) {	/* BM8 */
		dt_bm8_pal = (union palette_bm8 *)(omron_fb_info->palmap);
		dt_bm8_pal->reg.addr    = 0x04;
		dt_bm8_pal->reg.command = (1 << omron_fb_info->fb_depth) - 1;
		dt_bm8_pal->reg.addr    = 0x06;
		dt_bm8_pal->reg.command = 0x40;
	}

	omronDtBmGetPal(omron_fb_info, paldata);

	return TRUE; 
}


static void
omronDtBmPalInit(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	bcopy(palinit, palsave, sizeof(palinit));
	omronDtBmSetPal(omron_fb_info, palsave);
}


static void
omronDtBmSetPal(omron_fb_info, pal)
OmronFbInfoPtr omron_fb_info;
struct pal *pal;
{
	union palette	   *dt_bm_pal;
	union palette_bm8  *dt_bm8_pal;
	register int       i, j;
	unsigned char      *palp;

	if(omron_fb_info->fb_type == DT_BM8) {	/* BM8 */
		dt_bm8_pal = (union palette_bm8 *)(omron_fb_info->palmap);
		for (i = 0; i < 256; i++) {
			palp = &pal++->red;
			dt_bm8_pal->reg.addr = i;
			for (j = 0; j < 3; j++) {
				dt_bm8_pal->reg.coldata = *palp++;
			}
		}
	} else {
		dt_bm_pal = (union palette *)(omron_fb_info->palmap);
		/* restore palette data */
		for (i = 0; i < 16; i++) {
			palp = &pal++->red;
			dt_bm_pal->reg.addr = i << 4;
			for (j = 0; j < 3; j++)
				dt_bm_pal->reg.coldata = *palp++;
		}
	}
}


static void
omronDtBmGetPal(omron_fb_info, pal)
OmronFbInfoPtr omron_fb_info;
struct pal *pal;
{
	union palette	   *dt_bm_pal;
	union palette_bm8  *dt_bm8_pal;
	register	int    i,j;
	unsigned char    *palp;

	if(omron_fb_info->fb_type == DT_BM8) {	/* BM8 */
		dt_bm8_pal = (union palette_bm8 *)(omron_fb_info->palmap);

		/* save the CPU ROM palette setting */
		for (i = 0; i < 256; i++) {
			palp = &pal++->red;
			dt_bm8_pal->reg.addr = i;
			for (j = 0; j < 3; j++) {
				*palp++ = dt_bm8_pal->reg.coldata;
			}
		}
	} else {
		dt_bm_pal = (union palette *)(omron_fb_info->palmap);

		/* save the CPU ROM palette setting */
		for (i = 0; i < 16; i++) {
			palp = &pal++->red;
			dt_bm_pal->reg.addr = i << 4;
			for (j = 0; j < 3; j++) {
				*palp++ = dt_bm_pal->reg.coldata;
			}
		}
	}
}


static void
omronDtBmFbClear(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	register int *f;
	register int nw,height;
	int nwidth,nlwidth,nlwExtra;

	f = (int *)omron_fb_info->plane;
	nlwidth  = (omron_fb_info->fb_width)>>5;
	nwidth   = (omron_fb_info->scr_width)>>5;
	nlwExtra = nlwidth - nwidth;
	height =omron_fb_info->scr_height;

	while(height--) {
		nw = nwidth;
		while(nw--)
			*f++ = 0;
		f += nlwExtra;
	}
}

void
omronDtBmGiveUp(omron_fb_info)
OmronFbInfoPtr omron_fb_info;
{
	omronDtBmFbClear(omron_fb_info);
	omronDtBmPalClose(omron_fb_info);
}
/*
#endif
*/
#endif /* !luna88k */
