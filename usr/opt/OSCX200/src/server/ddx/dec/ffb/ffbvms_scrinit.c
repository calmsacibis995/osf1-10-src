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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ffbvms_scrinit.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:18:49 $";
#endif
/*
 */
/*
 * HISTORY
 */
/*
 *  Define Alpha specific symbols
 *
 */
#ifdef ALPHA

/*
 *  Define Flamingo/Sandpiper specific stuff
 *
 */
#define FLAMINGO

/*
 *  Debugging.  Draw patterns to the frame buffer,
 *  print extra error messages to help trace where we
 *  are in the code at any given time.  Undefine these
 *  for production.
 *
 *  #define TEST_FRAME_BUFFER
 *
 */
#endif

/*
 *  This logical sets the default for the noise level.  When it's set to 1,
 *  ErrorF statements print some more-or-less interesting things to help in
 *  debugging errors.
 *
 */
#ifdef NOISY
#define NOISE_LEVEL 1
#else
#define NOISE_LEVEL 0
#endif

/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1991, 1992 BY                      *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
*/
/** Stolen from sfbvms_scrinit.c 
**   AUTHORS:
**
**      Fred Kleinsorge
**--
**/

#include "misc.h"
#include "X.h"
#include "scrnintstr.h"
#include "decwdef.h"
#include "ffb.h"

#define CSR 0
#define DAC 1

static char smartLogical[] =	"DECW$SERVER_SMART_FRAME_BUFFER";

extern cfbvms_InitHX();
extern cfbvms_InitDAC();

extern Bool ffb8ScreenInit();
extern Bool ffb32ScreenInit();

extern vmsSaveScreen();

extern cfbvmsQueryBestSize();

extern cfbvmsStoreColors();
extern cfbvmsInstallColormap();
extern cfbvmsUninstallColormap();
extern cfbvmsListInstalledColormaps();

#ifndef DVI$K_TC_FFB
#define DVI$K_TC_FFB    31 
#endif

/*
 *  Runtime check to see if we should display something...
 *
 */
static int noisy = NOISE_LEVEL;

/*
 *  Default screen resolution.
 *
 */
#define DENSITY 100


Bool
ffbvmsScreenInit(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;
    char **argv;
{
    int framewidth, screenwidth, screenheight, total_bytes,
        density, use_device_driver, onscreen_addr, length, status,
	CSR_addr;
    FFBInfo ffbInformation;

    unsigned char *pLUT;		/* Used to format the LUT */
    int bitmap_addr;			/* frame buffer address */

    struct dvi_struct DVI;		/* DVI structure */
    struct dvi_struct *pDvi = &DVI;	/* device info block pointer */

   /*
    *  If the mode is not SMART then fall back to the CFB server.
    *
    */
    if (!vmsGetMode(index, smartLogical, TRUE))
	return cfbvmsScreenInit(index, pScreen, argc, argv);

   /*
    *  Make sure that these are initialized to zero the first time
    *
    */
    vmsInitDeviceData(index);

    use_device_driver = vmsGetDriverMode(index);		/* use device driver ? */
    density = vmsGetDensity(index, DENSITY);			/* screen resolution */

    ErrorF("\n");
    ErrorF("ffbvmsScreenInit: Init FFB screen number %d\n", index);

    if (use_device_driver)
      {
	if (!vmsInitDevice(index, pDvi, DVI$K_TC_FFB))
	    return FALSE;
      }

#ifdef FLAMINGO

    else
      {
       /*
	*  Non-Driver DVI code for Flamingo ***ONLY***  **NOT** the
	*  sandpiper!!!
	*
	*/
	pDvi->dvi$w_onscreen_width     = 1284;		/* Default to 1284 x 1024 */
	pDvi->dvi$w_onscreen_height    = 1024;
	pDvi->dvi$w_video_page_count   = 4096;		/* Size of frame buffer */
	pDvi->dvi$l_video_starting_pfn = 0xF1100;	/* PFN of the frame buffer */
	pDvi->dvi$l_lego_p0_mask       = 0xF1080;	/* PFN of base CSR */
	pDvi->dvi$w_lego_p0            = 0;
	pDvi->dvi$w_onscreen_y_origin  = 0;		/* No console */
	pDvi->dvi$w_onscreen_x_origin  = 0;
	pDvi->dvi$b_bits_per_rgb	=8;		/* force 8 bit */
      }

    if (!vmsMapIOSpace(index, pDvi->dvi$l_lego_p0_mask + 0x80, CSR, &CSR_addr, noisy))
	return FALSE;


#endif	/* ifdef FLAMINGO */

   /*
    *  Calculate page offset to screen start
    *  and length of screen memory in pages.
    *  Just map visible screen area.
    */
    framewidth   = pDvi->dvi$w_onscreen_width;		/* Y step value */
    screenwidth  = pDvi->dvi$w_onscreen_width;
    screenheight = pDvi->dvi$w_onscreen_height;

    if (noisy)
	ErrorF("ffbvmsScreenInit: FB Width %d, Screen X %d, Screen Y %d, density %d\n",
		framewidth, screenwidth, screenheight, density);

   /*
    *  The FFB uses ALL of the frame buffer memory...
    *
    *  This code maps 16mb of I/O space per frame buffer * 5 aliases 
    *  == 80 mB of I/O space. This allows the code to use alias frame buffer
    *  addresses.  So, we will ignore the actual size of the frame buffer
    *  passed to us.
    *
    *  The frame buffer is
    *
    */

    length = 0x28000;	/* in pagelets.   80Mb/512 */

    vmsMapFrameBuffer(	index,					/* screen # */
			pDvi->dvi$l_video_starting_pfn,         /*  - 0x100, starting PFN of memory */
			&bitmap_addr,				/* starting FB address */
			&length,				/* amount of memory to map (pagelets) */
			(framewidth * screenheight) / 512,	/* actual FB size */
			0x28000,				/* max memory available */
			noisy);					/* print msgs? */

   /*
    *  Ignore the first 2mb
    *
    */

    /* bitmap_addr += 0x200000; */

   /*
    *  Offset to the start of onscreen memory from the start of video memory
    *
    */
    onscreen_addr = bitmap_addr + 4096;

    total_bytes = (pDvi->dvi$w_video_page_count * 512) - 
		(pDvi->dvi$w_onscreen_width * pDvi->dvi$w_onscreen_y_origin +
		pDvi->dvi$w_onscreen_x_origin);

    if (noisy) ErrorF("ffbvmsScreenInit: VRAM %d, PFN ^x%X, pages %d, FB Addr %d\n",
		bitmap_addr, pDvi->dvi$l_video_starting_pfn,
		length, onscreen_addr );

   /*
    * We need to set up the ffbInfo structure for use in the 
    *  ffbScreenInit routine.
    */

    ffbInformation = (FFBInfo)xalloc(sizeof(FFBInfoRec));  
    ffbInformation->option_base = CSR_addr - 0x100000;
    ffbInformation->fb_alias_increment = (vm_offset_t) CYCLE_FB_INC;
    
    vmsSetCursorRoutines(pScreen, use_device_driver);

    pScreen->SaveScreen = vmsSaveScreen;
    pScreen->QueryBestSize = cfbvmsQueryBestSize;

    pScreen->StoreColors = cfbvmsStoreColors;

    pScreen->InstallColormap = cfbvmsInstallColormap;
    pScreen->UninstallColormap = cfbvmsUninstallColormap;
    pScreen->ListInstalledColormaps = cfbvmsListInstalledColormaps;

#ifdef TEST_FRAME_BUFFER

 /*
  *  According to the Flamingo MacroCoders manual, dense I/O space doesn't
  *  work like main memory when doing unaligned longword access.  However,
  *  the documentation is wrong.  This is a frame buffer pattern test that
  *  simply writes a set of bitpatterns to the screen and then reads them
  *  back to see if they are the same.  The visible effect is horizontal
  *  bars on the screen.  This test can be disabled for production, but it
  *  is handy when doing initial testing to see that the FB is good.
  *
  */
    vmsTestFrameBuffer(bitmap_addr, length);

#endif	/* TEST_FRAME_BUFFER */

   /*
    *  Define the default visual class
    *
    */
    vmsSetDefaultVisualClass(index, PseudoColor, GrayScale);

   /*
    *  Call the FFB screen initialization
    *
    */
    if (noisy) ErrorF("ffbvmsScreenInit: SMART MODE - Call the FFB screen init\n");

    if (pDvi->dvi$b_bits_per_rgb == 8){
        status = ffb8ScreenInit(
			    pScreen,			/* per screen structure */
			    onscreen_addr,		/* Screen address */
			    screenwidth,		/* Screen width */
			    screenheight,		/* Screen height */
			    density,			/* Y DPI */
			    density,			/* Y DPI */
			    framewidth,			/* Width of the framebuffer */
			    0,			        /* unused */
			    8,				/* depth */
			    ffbInformation,		/* FFB info */
			    total_bytes);		/* Total bytes available */
    } else {
        status = ffb32ScreenInit(
			    pScreen,			/* per screen structure */
			    onscreen_addr,		/* Screen address */
			    screenwidth,		/* Screen width */
			    screenheight,		/* Screen height */
			    density,			/* Y DPI */
			    density,			/* Y DPI */
			    framewidth,			/* Width of the framebuffer */
			    0,			        /* unused */
			    32,				/* depth */
			    ffbInformation,		/* FFB info */
			    total_bytes);		/* Total bytes available */
    }
    if (status)
      {
       /*
	*  Create the default colormap
	*
	*/
	if (noisy) ErrorF("ffbvmsScreenInit: Call the Default Colormap Creation\n");

	if (cfbCreateDefColormap (pScreen))
	  {
	    if (noisy) ErrorF("ffbvmsScreenInit: Exit from init with success\n");
	    return TRUE;
	  }
	else
	  ErrorF("ffbvmsScreenInit: ERROR from Default Colormap create\n");
      }

    ErrorF("ffbvmsScreenInit: Exit with an ERROR, returns FALSE\n");
    return FALSE;

}
