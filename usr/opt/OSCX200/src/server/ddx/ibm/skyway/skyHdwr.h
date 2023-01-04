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
 * $XConsortium: skyHdwr.h,v 1.3 91/07/16 13:17:42 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/*
 * skyHdwr.h - hardware constants
 */

#ifndef SKYHDWR_H
#define SKYHDWR_H

/*
 * System addresses of bounds of adapter data and program areas
 */

extern unsigned long    SKYWAY_OFFSET[]         ;       /* micro channel */
extern unsigned long    SKYWAY_VRAM_START[]     ;       /* micro channel */
extern unsigned long    IOREG[]         ;               /* micro channel */
extern unsigned long    COPREG[]        ;               /* micro channel */
extern unsigned long    SKYWAY_DMA0[]   ;
extern unsigned long    SKYWAY_DMA1[]   ;
extern unsigned long    SKYWAY_DMA2[]   ;
extern unsigned long    SKYWAY_DMA3[]   ;
extern unsigned long    SKYWAY_TILEOFFSET[]   ;

#define SKYWAY_VRAM_END         0x140000                /* VRAM SIZE */
#define SKYWAY_MASKMAP_START    0x800000
#define SKYWAY_COP_START        0x400000
#define SKYWAY_WIDTH            1280
#define SKYWAY_HEIGHT           1024

/* Interrupt Enable Register */

#define FRAMEFLYBACK           (1 << 0)
#define COPREJECTED            (1 << 2)
#define COPCOMPLETED           (1 << 3)

/*  Mode    */

#define VGAINHIBITED            0x0
#define VGAENABLED              0x1
#define EXTVGAINHIBITED         0x2
#define EXTVGAENABLED           0x3
#define INTELINHIBITED          0x4
#define INTELENABLED            0x5
#define MOTOROLAINHIBITED       0x6
#define MOTOROLAENABLED         0x7

/*      work with Index Register        */

#define MEMORYCONF              0x00    /* Memory Configuration Register */
#define COPSAVERESTOREDATA1     0x0c    /* Save/Restore Data 1 */
#define COPSAVERESTOREDATA2     0x0d    /* Save/Restore Data 2 */

#define HORIZONTALTOTAL         0x10    /* Horizontal Total Register */
#define HORIZONTALDPYEND        0x12    /* Horizontal Dpy End Register  */
#define HORIZONTALBLANKSTART    0x14    /* Horizontal Blank Start       */
#define HORIZONTALBLANKEND      0x16    /* Horizontal Blank End         */
#define HORIZONTALPULSESTART    0x18    /* Horizontal Pulse Start       */
#define HORIZONTALPULSEEND1     0x1a    /* Horizontal Pulse End 1       */
#define HORIZONTALPULSEEND2     0x1c    /* Horizontal Pulse End 2       */
#define HORIZONTSPRITELO        0x30    /* Horizontal Sprite Lo Register */
#define HORIZONTSPRITEMI        0x31    /* Horizontal Sprite MI Register */
#define HORIZONTSPRITEHI        0x32    /* Horizontal Sprite HI Register */

#define VERTICALTOTALLO         0x20    /* Vertical Total Lo Register */
#define VERTICALTOTALHI         0x21    /* Vertical Total Hi Register */

#define VERTICALDPYENDLO        0x22    /* Vertical Dpy End Lo Register */
#define VERTICALDPYENDLHI       0x23    /* Vertical Dpy End Hi Register */
#define VERTICALBLANKSTARTLO    0x24    /* Vertical Blank Start Lo Register */
#define VERTICALBLANKSTARTHI    0x25    /* Vertical Blank Start Hi Register */
#define VERTICALBLANKENDLO      0x26    /* Vertical Blank End Lo Register */
#define VERTICALBLANKENDHI      0x27    /* Vertical Blank End Hi Register */

#define VERTICALSYNCPULSESTLO   0x28    /* Vertical Sync Pulse Start Lo */
#define VERTICALSYNCPULSESTHI   0x29    /* Vertical Sync Pulse Start Hi */
#define VERTICALSYNCPULSEPEND   0x2a    /* Vertical Sync Pulse End */
#define VERTICALLINECMPLO       0x2c    /* Vertical Line Compare Lo */
#define VERTICALLINECMPHI       0x2d    /* Vertical Line Compare Hi */

#define VERTICALSPRITESTARTLO   0x33    /* Vertical Sprite Start Lo Register */
#define VERTICALSPRITESTARTHI   0x34    /* Vertical Sprite Start Hi  Register */
#define VERTICALSPRITEPRESET    0x35    /* Vertical Sprite Start Preset */

#define SPRITECTRL              0x36    /* Sprite Control Register */
#define SPRITE0RED              0x38    /* Sprite Color 0 Red Register */
#define SPRITE0GREEN            0x39    /* Sprite Color 0 Green Register */
#define SPRITE0BLUE             0x3a    /* Sprite Color 0 Blue Register */
#define SPRITE1RED              0x3b    /* Sprite Color 1 Red Register */
#define SPRITE1GREEN            0x3c    /* Sprite Color 1 Green Register */
#define SPRITE1BLUE             0x3d    /* Sprite Color 1 Blue Register */

#define STARTADDRLO             0x40    /* Start Address Lo Register */
#define STARTADDRMI             0x41    /* Start Address Mi Register */
#define STARTADDRHI             0x42    /* Start Address Hi Register */
#define BUFFERPITCHLO           0x43    /* Buffer Pitch Lo Register */
#define BUFFERPITCHHI           0x44    /* Buffer Pitch Hi Register */

#define DPYMODE1                0x50    /* Display Mode 1 Register */
#define DPYMODE2                0x51    /* Display Mode 2 Register */
#define MONITORID               0x52    /* Monitor ID Register */
#define SYSTEMID                0x53    /* System  ID Register */
#define CLOCKFREQUENCE          0x54    /* Clock Frequency Select Register */
#define BORDERCOLOR             0x55    /* Border Color Register */
#define SPINDEXLO               0x60    /* Sprite/Palette Index Lo Register */
#define SPINDEXHI               0x61    /* Sprite/Palette Index Hi Register */
#define SPINDEXLOPF             0x62    /* S/P Index Lo Prefetch Register */
#define SPINDEXHIPF             0x63    /* S/P Index Hi Prefetch Register */
#define PALETTEMASK             0x64    /* Palette Mask Register */
#define PALETTEDATA             0x65    /* Palette Data Register */

#define PALETTESEQ              0x66    /* Palette Sequence Register */
#define PALETTERED              0x67    /* Palette Red Prefetch Register */
#define PALETTEGREEN            0x68    /* Palette Green Prefetch Register */
#define PALETTEBLUE             0x69    /* Palette Blue Prefetch Register */
#define SPRITEDATA              0x6a    /* Sprite Data Register */
#define SPRITEPF                0x6b    /* Sprite Prefetch Register */

#define MD_OFF          0x0     /* Operating Mode Register */
#define WC_OFF          0x1     /* Window Control Register */
#define INT_OFF         0x4     /* Interrupt Enable Register */
#define INS_OFF         0x5     /* Interrupt Status Register */
#define VMC_OFF         0x6     /* Virtual Memory Control Register */
#define VMS_OFF         0x7     /* Virtual Memory Status Register */
#define VMI_OFF         0x8     /* VRAM Index Register */
#define MEM_OFF         0x9     /* Memory Access Mode Register */
#define INDEX_OFF       0xa     /* IO Index Register */
#define DATA_B_OFF      0xb     /* IO Data B Register */
#define DATA_C_OFF      0xc     /* IO Data C Register */
#define DATA_D_OFF      0xd     /* IO Data D Register */
#define DATA_E_OFF      0xe     /* IO Data E Register */

#define SKYWAY_MODE_REG(index)                          \
	(*((volatile unsigned char *)(IOREG[index] + MD_OFF)))
#define SKYWAY_WINCTRL_REG(index)                       \
	(*((volatile unsigned char *)(IOREG[index] + WC_OFF)))
#define SKYWAY_INT_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + INT_OFF)))
#define SKYWAY_INS_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + INS_OFF)))
#define SKYWAY_VMC_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + VMC_OFF)))
#define SKYWAY_VMS_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + VMS_OFF)))
#define SKYWAY_VMI_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + VMI_OFF)))
#define SKYWAY_MEM_REG(index)                           \
	(*((volatile unsigned char *)(IOREG[index] + MEM_OFF)))
#define SKYWAY_BINDEX_REG(index)                        \
	(*((volatile unsigned char *)(IOREG[index] + INDEX_OFF)))
#define SKYWAY_SINDEX_REG(index)                        \
	(*((volatile unsigned short *)(IOREG[index] + INDEX_OFF)))
#define SKYWAY_DATAB_REG(index)                         \
	(*((volatile unsigned char *)(IOREG[index] + DATA_B_OFF)))
#define SKYWAY_DATAC_REG(index)                         \
	(*((volatile unsigned char *)(IOREG[index] + DATA_C_OFF)))
#define SKYWAY_DATAD_REG(index)                         \
	(*((volatile unsigned char *)(IOREG[index] + DATA_D_OFF)))
#define SKYWAY_DATAE_REG(index)                         \
	(*((volatile unsigned char *)(IOREG[index] + DATA_E_OFF)))

#define PD_OFF          0x0     /* Page Directory Register */
#define VA_OFF          0x4     /* Virtual Address Register */
#define POLL_OFF        0x9     /* new busy poll Register */
#define LA_OFF          0xe     /* State Length A Register */
#define LB_OFF          0xf     /* State Length B Register */
#define PMI_OFF         0x10    /* Pixmap Index Register */
#define PMC_OFF         0x12    /* Pixmap Control Register */
#define PMB_OFF         0x14    /* Pixmap Base Register */
#define PMH_OFF         0x18    /* Pixmap Height Register */
#define PMW_OFF         0x1a    /* Pixmap Width Register 16 bits */
#define PMF_OFF         0x1e    /* Pixmap Format Register 16 bits */
#define BME_OFF         0x20    /* Bresenham Error Register */
#define BMK1_OFF        0x24    /* Bresenham K1 Register */
#define BMK2_OFF        0x28    /* Bresenham K2 Register */
#define DRT_OFF         0x2c    /* Direction Step Register */
#define CCC_OFF         0x48    /* Color Compare Condition Register */
#define BM_OFF          0x4a    /* Foreground MIX Register */
#define FM_OFF          0x4b    /* Background MIX Register */
#define CCV_OFF         0x4c    /* Color Compare Condition Register */
#define PM_OFF          0x50    /* Plane Mask  Register */
#define CC_OFF          0x54    /* Carry Chain Register */
#define FC_OFF          0x58    /* Foreground Color Register */
#define BC_OFF          0x5c    /* Background Color Register */
#define DM2_OFF         0x60    /* Dimention 2 Register */
#define DM1_OFF         0x62    /* Dimention 1 Register */
#define MASKY_OFF       0x6c    /* MaskMap  Y offset Register */
#define MASKX_OFF       0x6e    /* MaskMap  X offset Register */
#define SRCY_OFF        0x70    /* SrcMap   Y offset Register */
#define SRCX_OFF        0x72    /* SrcMap   X offset Register */
#define PATY_OFF        0x74    /* PatMap   Y offset Register */
#define PATX_OFF        0x76    /* PatMap   X offset Register */
#define DSTY_OFF        0x78    /* DstMap   Y offset Register */
#define DSTX_OFF        0x7a    /* DstMap   X offset Register */
#define PO_OFF          0x7c    /* Pixel Operation Register   */

#define SKYWAY_PAGE_DIR_REG(index)                      \
	        (*((volatile unsigned int *)(COPREG[index] + PD_OFF)))
#define SKYWAY_VA_REG(index)                            \
	        (*((volatile unsigned int *)(COPREG[index] + VA_OFF)))
#define SKYWAY_POLL_REG(index)                          \
	        (*((volatile unsigned char *)(COPREG[index] + POLL_OFF)))
#define SKYWAY_LA_REG(index)                            \
	        (*((volatile unsigned char *)(COPREG[index] + LA_OFF)))
#define SKYWAY_LB_REG(index)                            \
	        (*((volatile unsigned char *)(COPREG[index] + LB_OFF)))

#define SKYWAY_PMI_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + PMI_OFF)))
#define SKYWAY_PMC_REG(index)                           \
	        (*((volatile unsigned char *)(COPREG[index] + PMC_OFF)))
#define SKYWAY_PMB_REG(index)                           \
	        (*((volatile unsigned char *)(COPREG[index] + PMB_OFF)))
#define SKYWAY_PMH_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + PMH_OFF)))
#define SKYWAY_PMW_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + PMW_OFF)))
#define SKYWAY_PMF_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + PMF_OFF)))
#define SKYWAY_BME_REG(index)                           \
	        (*((volatile unsigned int *)(COPREG[index] + BME_OFF)))
#define SKYWAY_BMK1_REG(index)                          \
	        (*((volatile unsigned int *)(COPREG[index] + BMK1_OFF)))
#define SKYWAY_BMK2_REG(index)                          \
	        (*((volatile unsigned int *)(COPREG[index] + BMK2_OFF)))
#define SKYWAY_DRT_REG(index)                           \
	        (*((volatile unsigned int *)(COPREG[index] + DRT_OFF)))
#define SKYWAY_CCC_REG(index)                           \
	        (*((volatile unsigned short *)(COPREG[index] + CCC_OFF)))
#define SKYWAY_BM_REG(index)                            \
	        (*((volatile unsigned char *)(COPREG[index] + BM_OFF)))
#define SKYWAY_FM_REG(index)                            \
	        (*((volatile unsigned char *)(COPREG[index] + FM_OFF)))
#define SKYWAY_CCV_REG(index)                           \
	        (*((volatile int *)(COPREG[index] + CCV_OFF)))
#define SKYWAY_PM_REG(index)                            \
	        (*((volatile int *)(COPREG[index] + PM_OFF)))
#define SKYWAY_CC_REG(index)                            \
	        (*((volatile int *)(COPREG[index] + CC_OFF)))
#define SKYWAY_FC_REG(index)                            \
	        (*((volatile int *)(COPREG[index] + FC_OFF)))
#define SKYWAY_BC_REG(index)                            \
	        (*((volatile int *)(COPREG[index] + BC_OFF)))
#define SKYWAY_DM1_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + DM1_OFF)))
#define SKYWAY_DM2_REG(index)                           \
	        (*((volatile short *)(COPREG[index] + DM2_OFF)))
#define SKYWAY_MASKY_REG(index)                         \
	        (*((volatile short *)(COPREG[index] + MASKY_OFF)))
#define SKYWAY_MASKX_REG(index)                         \
	        (*((volatile short *)(COPREG[index] + MASKX_OFF)))
#define SKYWAY_SRCY_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + SRCY_OFF)))
#define SKYWAY_SRCX_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + SRCX_OFF)))
#define SKYWAY_PATY_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + PATY_OFF)))
#define SKYWAY_PATX_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + PATX_OFF)))
#define SKYWAY_DSTY_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + DSTY_OFF)))
#define SKYWAY_DSTX_REG(index)                          \
	        (*((volatile short *)(COPREG[index] + DSTX_OFF)))
#define SKYWAY_PO_REG(index)                            \
	        (*((volatile unsigned int *)(COPREG[index] + PO_OFF)))

#define skywayWaitFifo1(index)  while(SKYWAY_PMC_REG(index) & 0x80 )
#define skywayWaitFifo2(index)  while(SKYWAY_POLL_REG(index) & 0x80 )

/* Pixel Map Index Register */

#define PixMapA   1
#define PixMapB   2
#define PixMapC   3
#define PixMapD   0  /* Mask Map */

/* Pixel Map n Format */
/* M/I Format */

#define MI0  0
#define MI1  0x8

/* Pixel Size */

#define PixSize1  0
#define PixSize2  1
#define PixSize4  2
#define PixSize8  3
#define PixSize16 4

/*  Octant fields      */

#define  DX              0x4        /* 3rd pos. from the right        */
#define  DY              0x2        /* 2nd pos. from the right        */
#define  DZ              0x1        /* 1st pos. from the right        */

/*  Logical Operations for both Foreground & Background Mix           */

#define  Mix_All_0       0x00       /* All 0's                        */
#define  Mix_SrcAndDst   0x01       /* Source And Destination         */
#define  Mix_SrcAndCDst  0x02       /* Source And ^Destination        */
#define  Mix_Src         0x03       /* Source                         */
#define  Mix_CSrcAndDst  0x04       /* ^Source And Destination        */
#define  Mix_Dst         0x05       /* Destination                    */
#define  Mix_SrcXorDst   0x06       /* Source XOR  Destination        */
#define  Mix_SrcOrDst    0x07       /* Source OR   Destination        */
#define  Mix_CSrcAndCDst 0x08       /* ^Source And ^Destination       */
#define  Mix_SrcXorCDst  0x09       /*  Source XOR ^Destination       */
#define  Mix_CDst        0x0A       /* ^Destination                   */
#define  Mix_SrcOrCDst   0x0B       /* Source  OR ^Destination        */
#define  Mix_CSrc        0x0C       /* ^Source                        */
#define  Mix_CSrcOrDst   0x0D       /* ^Source  OR  Destination       */
#define  Mix_CSrcORCDst  0x0E       /* ^Source  OR ^Destination       */
#define  Mix_All_1       0x0F       /* All 1's                        */

/*  Color Compare Condition         */

#define  Color_Cmp_True  0x0        /* Always True (disable updates)  */
#define  Color_Grt_Col   0x1        /* Dest > Col value               */
#define  Color_Equ_Col   0x2        /* Dest = Col value               */
#define  Color_Les_Col   0x3        /* Dest < Col value               */
#define  Color_Cmp_Fal   0x4        /* Always False (enable updates)  */
#define  Color_GtEq_Col  0x5        /* Dest >= Col value              */
#define  Color_NtEq_Col  0x6        /* Dest <> Col value              */
#define  Color_LsEq_Col  0x7        /* Dest <= Col value              */

#define  Plane_Mask_All  0xFFFF
#define  Carry_Mask      0x3FFF

#define POBackReg 0                /* Background color  (register)   */
#define POBackSrc 0x80             /* Source Pixel Map               */
#define POForeReg 0                /* Foreground  color (register)   */
#define POForeSrc 0x20             /* Source Pixel Map               */

/* Step */

#define POStepDSR 0x2              /* Draw & Step Read               */
#define POStepLDR 0x3              /* Line Draw   Read               */
#define POStepDSW 0x4              /* Draw & Step Write              */
#define POStepLDW 0x5              /* Line Draw   Write              */
#define POStepBlt 0x8              /* Pxblt                          */
#define POStepIBlt 0x9             /* Inverting Pxblt                */
#define POStepAFBlt 0xa            /* Area Fill Pxblt                */

/* Source */

#define POSrcA 0x1000              /* Pixel Map A                    */
#define POSrcB 0x2000              /* Pixel Map B                    */
#define POSrcC 0x3000              /* Pixel Map C                    */
#define POSrcD 0x0000              /* Mask Map  D                    */

/* Destination */

#define PODestA 0x100              /* Pixel Map A                    */
#define PODestB 0x200              /* Pixel Map B                    */
#define PODestC 0x300              /* Pixel Map C                    */
#define PODestD 0x000              /* Mask Map  D                    */

/* Pattern */

#define POPatA 0x100000            /* Pixel Map A                    */
#define POPatB 0x200000            /* Pixel Map B                    */
#define POPatC 0x300000            /* Pixel Map C                    */
#define POPatD 0x000000            /* Mask Map  D                    */

#define POPatFore 0x800000         /* Foreground (Fixed)             */
#define POPatSrc 0x900000          /* Generated from Source          */

/* Mask */

#define POMaskDis 0                /* Mask Map Disabled             */
#define POMaskBEn 0x40000000       /* Mask Map Boundary Enabled     */
#define POMaskEn  0x80000000       /* Mask Map Enabled              */

/* Drawing Mode */

#define POModeAll 0                /* Draw All Pixels                */
#define POModeLast 0x10000000      /* Draw 1s Pixel Null             */
#define POModeFirst 0x20000000     /* Draw Last Pixel Null           */
#define POModeArea 0x30000000      /* Draw Area Boundary             */

/* Direction Octant */

#define POOct0 0
#define POOct1 0x1000000
#define POOct2 0x2000000
#define POOct3 0x3000000
#define POOct4 0x4000000
#define POOct5 0x5000000
#define POOct6 0x6000000
#define POOct7 0x7000000

/* skycolor */

#define SC_INVBASEOFFSET   0x140000
#define SC_TILEOFFSET      0x0
#define SC_STIPPLEOFFSET   0x4000
#define SC_FONTOFFSET      0x6000
#define SC_MASKOFFSET      0xF00C
#define SC_DASHOFFSET      0x3700C
#define SC_WKSPACEOFFSET   0x3708C
#define SC_TRICKYOFFSET    0x5F08C
#define SC_USEROFFSET      0x5F08C

/* skymono */

#define SM_INVBASEOFFSET   0x0A0000
#define SM_TILEOFFSET      0x0
#define SM_STIPPLEOFFSET   0x2000
#define SM_FONTOFFSET      0x6000
#define SM_MASKOFFSET      0xEE0C
#define SM_DASHOFFSET      0x36E0C
#define SM_WKSPACEOFFSET   0x36E4C
#define SM_TRICKYOFFSET    0x5EE4C
#define SM_USEROFFSET      0x5EE4C

/*   CRTC REGISTERS     */

#define Display_Mode1   0x5000      /* Display Mode 1 Register        */
#define Display_Mode2   0x5100      /* Display Mode 2 Register        */

/*   hardware cursor                                                  */

#define CursLo_Plane0   0x5600      /* Cursor address low plane 0     */
#define CursLo_Plane1   0x5700      /* Cursor address low plane 1     */
#define CursHi_Plane0   0x5800      /* Cursor address high plane 0    */
#define CursHi_Plane1   0x5900      /* Cursor address high plane 0    */
#define CursImg_Plane0  0x5A00      /* Cursor image plane 0           */
#define CursImg_Plane1  0x5B00      /* Cursor image plane 1           */
#define CursIndex       0x6000      /* Cursor index                   */
#define CursData        0x6A00      /* Cursor data                    */
#define CursCntl_Plane0 0x6C00      /* Cursor control plane 0         */
#define CursCntl_Plane1 0x6D00      /* Cursor control plane 1         */

/*   colormap                                                         */

#define PaletIndex      0x6000      /* Palette Index                  */
#define PaletCntl       0x6400      /* Palette DAC control            */
#define PaletData       0x6500      /* Palette Data                   */

/*   misc stuff                                                       */

#define x_correct       0x01A8      /* x correction factor for pass 1 */
#define y_correct       0x001A      /* y correction factor for pass 1 */

#define xc_correct2     0x01AD      /* x cursor offset, color, pass 2 */
#define yc_correct2     0x001B      /* y cursor offset, color, pass 2 */
#define xm_correct2     0x0194      /* x cursor offset, mono, pass 2  */
#define ym_correct2     0x001F      /* y cursor offset, mono, pass 2  */

#define DAC_disable     0x0004
#define DAC_enable      0x0044
#define Video_disable   0x0061
#define Video_enable    0x0063
#define ColorCmd        0x38
#define CursPlaneSize   512
#define MaxCursorSize   64
#define BestCursorSize  32
#define MaxTileSize     1024
#define BestTileSize    32
#define MaxStippleSize  1024
#define BestStippleSize 32

#endif /* SKYHDWR_H */
