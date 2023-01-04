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
 * $XConsortium: omronFb.h,v 1.1 91/06/29 13:48:54 xguest Exp $
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

#ifdef	luna88k
# include <dev/fbmap.h>
#else	/*  ~luna88k */
#ifdef uniosu
# include <sys/devmap.h>
#else /* ~uniosu */
# ifdef luna2
#  include <dev/fbmap.h>
# else /* ~luna2 */
#  include <om68kdev/fbmap.h>
# endif /* luna2 */
#endif /* uniosu */
#endif /* luna88k */

typedef struct _OmronFbProc {
	int		(*CreateProc)();
	int		(*InitProc)();
	void	(*GiveUpProc)();
} OmronFbProc;        

typedef struct _OmronFbInfo {
	int		fb_type;
	int		fb_width;
	int		fb_height;
	int		fb_depth;
	int		scr_width;
	int		scr_height;
	char 	*plane;
	long	*refresh_reg;	
	int		palfd;	/* pallate */
	int		fbfd1;	/* luna-88k only */	/* /dev/fb */
	int		fbfd2;	/* luna-88k only */	/* /dev/allmapfb */
	int		palmapsize;
	char	*palmap;
	int		fbmapsize;
	char	*fbmap;;
	OmronFbProc	*func;
} OmronFbInfo, *OmronFbInfoPtr;

/*	fb_type		*/
#define DT_BM 		0
#define DT_PLASMA 	1
#define DS_BM		2
#define FS_BM		3
#define	DT_BM8		4

/* scr_width and scr_height */
/* DT_BM, DS_BM, FS_BM, DT_BM8 */
#define SCREEN_WIDTH				1280
#define SCREEN_HEIGHT				1024
/* DT_PLASMA */
#define PLASMA_SCREEN_WIDTH			1024
#define PLASMA_SCREEN_HEIGHT			768
/* fb_width and fb_height */
#define FB_WIDTH				2048
#define FB_HEIGHT				1024

#define COLOR_TV_RESOLUTION  			110 
#define MONO_TV_RESOLUTION			125   
#define PLASMA_TV_RESOLUTION			100


/*
**	color palette memory map
*/
union palette {			/* mono and 4 depth color */
	struct {
		unsigned char    addr;		/* palette addres register */
		unsigned char    coldata;	/* palette data register (R G B) */	
	} reg;
	char    pad[4096];
};

union palette_bm8 {		/* 8 depth color */
	struct {
		unsigned char	addr;		/* palette addres register */	
		unsigned char	pad11[3];	/* unuse */
		unsigned char	coldata;	/* palette data register (R G B) */
		unsigned char	pad12[3];	/* unuse */
		unsigned char	command;	/* palette control command register */
		unsigned char	pad13[3];	/* unuse */
		unsigned char	ovcdata;	/* palette overlay data (unuse) */
		unsigned char	pad14[3];	/* unuse */
	} reg;
	char    pad[4096];
};

union fs_palette {		/* 8 depth color (luna-fs)*/
	struct {
		long    addr;				/* palette addres register */
		long    control;			/* palette data register (R G B) */
		long    coldata;			/* palette control command register */
		long    ovcdata;			/* palette overlay data (unuse) */
	} reg;
	char    pad[0x1000];
};

/*
**	frame buffer memory memory map
*/
struct bm_one_data {	/* mono frame buffer */
	int         sd[1][1024][64];	/* 2048 x 1024 */
};

struct bm_four_data {	/* 4 depth color frame buffer */
	int			sd[4][1024][64];	/* 2048 x 1024 x 4 */
};

struct bm_eight_data {	/* 8 depth color frame buffer */
	int			sd[8][1024][64];	/* 2048 x 1024 x 8 */
};

/*
** luna raster operateon hardware memory map
*/
union bm_function_set {
		long	op_cont[16];	/* function set register */
		char	pad[0x40000];
};

/*
**	luna graphic display control hardware mememory map 
*/
typedef struct dt_bm_fbmap {	/* luna and luna-2 */
	union {	
		long	reg;
		char	pad2[0x40000];
	} refresh;		/* display area control register (write only) */
	union {
		long	reg;
		char	pad3[0x40000];
	} pselect;		/* plane select register (write only) */
	/* common frame buffer (write only) */
	struct bm_one_data		cbmplane;
	/* frame buffer (read/write) */
	struct bm_eight_data	bmplane;
	/* luna raster operateon hardware (common frame buffer)  */
	union bm_function_set cplane;
	/* luna raster operateon hardware */
	union bm_function_set planes[8];
} *DtBmMapPtr;

typedef struct fs_bm_fbmap {	/* luna-fs */
	/* common frame buffer (write only) */
	struct bm_one_data cbmplane;
	struct bm_one_data pad1[6];	/* unuse */
	/* luna raster operateon hardware (common frame buffer)  */
	union fs_fset {
		long    op_cont[16];
		char    pad2[0x8000];
	} cplane;
    union fs_fset pad3[6];	/* unuse */
	union {
		struct {
			long	addr;	/* acrtc address register */
			long	data;	/* acrtc data register */
		} reg;
		char	pad3[0x4000];
	} acrtc;
	union {
		long	reg;
		char	pad4[0x1000];
	} pselect;	      /* plane select register (write only) */
	union {
		long	reg;
		char	pad5[0x2000];
	} refresh;		  /* display area control register (write only) */
	/* color palette */
	union fs_palette palette;
	/* frame buffer (read/write) */
	struct bm_eight_data bmplane;
	char pad5[0x600000];	/* unuse */
	/* luna raster operateon hardware */
	union {
		long    op_cont[16];
		char    pad6[0x2000];
	} planes[8];
} *FsBmMapPtr;

typedef struct R88k_bm_fbmap {	/* luna-88k frame buffer */
	/* common frame buffer (write only) */
	struct bm_one_data		cbmplane;
	/* frame buffer (read/write) */
	struct bm_eight_data	bmplane;
	/* luna raster operateon hardware (common frame buffer)  */
	union bm_function_set 	cplane;
	/* luna raster operateon hardware */
	union bm_function_set	planes[8];
} *R88kBmMapPtr;

typedef struct R88k_bm_fbreg {	/* luna-88k control */
	union {
		long	reg;
		char	pad2[0x40000];
	} refresh;	/* display area control register (write only) */
	union {
		long	reg;
		char	pad3[0x40000];
	} pselect;	/* plane select register (write only) */
} *R88kBmRegPtr;

struct ds_bm_map {
	union {
		long	op_cont[16];
		char	pad1[0x8000];
	} planes[7];
	union {
		struct {
			long	addr;
			long	data;
		} reg;
		char	pad2[0x4000];
	} acrtc;
	union {
		long	reg;
		char	pad3[0x1000];
	} pselect;
	union {
		long	reg;
		char	pad4[0x2000];
	} refresh;
	union {
		struct {
			long	addr;
			long	control;
			long	coldata;
			long	ovcdata;
		} reg;
		char	pad5[0x1000];
	} palette;
};

#ifndef luna88k
# ifdef uniosu
#  ifndef luna2
extern Bool omronFsBmCreate();
extern Bool omronFsBmInit();
extern void omronFsBmGiveUp();
#  endif
# endif /* uniosu */
extern Bool omronDtBmCreate();
extern Bool omronDtBmInit();
extern void omronDtBmGiveUp();
#else /* luna88k */
extern Bool omron88kBmCreate();
extern Bool omron88kBmInit();
extern void omron88kBmGiveUp();
#endif /* luna88k */
