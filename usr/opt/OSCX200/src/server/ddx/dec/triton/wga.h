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
 * @(#)$RCSfile: wga.h,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:35:20 $
 */

#ifndef _WGA_H_
#define _WGA_H_

/***************************************************************************
 * Defines
 ***************************************************************************/

#define GC_INDEX            0x3CE      /* Index and Data Registers */
#define GC_DATA             0x3CF
#define SEQ_INDEX           0x3C4
#define SEQ_DATA            0x3C5
#define CRTC_INDEX          0x3D4
#define CRTC_DATA           0x3D5
#define ATTR_INDEX          0x3C0
#define ATTR_DATA           0x3C0
#define MISC_OUTPUT         0x3C2


#define CTRL_REG_1         0x63CA      /* Datapath Registers */
#define DATAPATH_CTRL        0x5A
#define GC_FG_COLOR          0x43
#define GC_BG_COLOR          0x44
#define SEQ_PIXEL_WR_MSK     0x02
#define SEQ_MEM_MODE	     0x04
#define GC_PLANE_WR_MSK      0x08
#define ROP_A              0x33C7
#define ROP_0              0x33C5
#define ROP_1              0x33C4
#define ROP_2              0x33C3
#define ROP_3              0x33C2
#define DATA_ROTATE          0x03
#define READ_CTRL            0x41

#define X0_SRC_ADDR_LO     0x63C0      /* BitBLT Registers */
#define Y0_SRC_ADDR_HI     0x63C2
#define DEST_ADDR_LO       0x63CC
#define DEST_ADDR_HI       0x63CE
#define BITMAP_WIDTH       0x23C2
#define BITMAP_HEIGHT      0x23C4
#define BITMAP_HWR         0x23C8
#define SRC_PITCH          0x23CA
#define DEST_PITCH         0x23CE
#define BLT_CMD_0          0x33CE
#define BLT_CMD_1          0x33CF
#define PREG_0             0x33CA
#define PREG_1             0x33CB
#define PREG_2             0x33CC
#define PREG_3             0x33CD
#define PREG_4             0x33CA
#define PREG_5             0x33CB
#define PREG_6             0x33CC
#define PREG_7             0x33CD

#define VER_NUM_REG         0x0C            // addded by ecr 
#define EXT_VER_NUM_REG     0x0D            // addded by ecr 
#define ENV_REG_0           0x0F            /* added by ecr */
#define BLT_CONFIG          0x10
#define CONFIG_STATE        0x52         /* LO, HI is at 0x53 */
#define BIOS_DATA           0x54
#define DATAPATH_CONTROL    0x5A

#define LOCK_KEY_QVISION    0x05
#define EXT_COLOR_MODE      0x01

#define BLT_ENABLE          0x08       /* BLT_CONFIG values - ecr */
#define RESET_BLT           0x40

#define X1                 0x83CC      /* Line Draw Registers */
#define Y1                 0x83CE
#define LINE_PATTERN       0x83C0
#define PATTERN_POINTER	     0x61
#define PATTERN_END          0x62
#define LINE_CMD             0x60
#define LINE_PIX_CNT         0x64
#define LINE_ERR_TERM        0x66
#define SIGN_CODES           0x63
#define K1_CONST             0x68
#define K2_CONST             0x6A

#define PALETTE_WRITE       0x3C8      /* DAC registers */
#define PALETTE_READ        0x3C7
#define PALETTE_DATA        0x3C9
#define CURSOR_WRITE	    0x3c8
#define CURSOR_DATA	    0x13c7
#define CO_COLOR_WRITE     0x83C8
#define CO_COLOR_DATA      0x83C9
#define DAC_CMD_0          0x83C6
#define DAC_CMD_1          0x13C8
#define DAC_CMD_2          0x13C9
#define PIXEL_MASK	    0x03C6
#define PALETTE_STATE	   0x03C7
#define VDAC_STATE	   0x13C6
#define CURSOR_X_LO	   0x93C8
#define CURSOR_X_HI	   0x93C9
#define CURSOR_Y_LO	   0x93C6
#define CURSOR_Y_HI	   0x93C7

#define PAGE_REG_0           0x45      /* Control Registers */
#define PAGE_REG_1           0x46
#define HI_ADDR_MAP          0x48
#define ENV_REG_1            0x50
#define VIRT_CTRLR_SEL     0x83C4

#define PACKED_PIXEL_VIEW    0x00      /* CTRL_REG_1 values */
#define PLANAR_VIEW          0x08
#define EXPAND_TO_FG         0x10
#define EXPAND_TO_BG         0x18
#define BITS_PER_PIX_4       0x00
#define BITS_PER_PIX_8       0x02
#define BITS_PER_PIX_16      0x04
#define BITS_PER_PIX_32      0x06
#define ENAB_TRITON_MODE     0x01

#define ROPSELECT_NO_ROPS              0x00      /* DATAPATH_CTRL values */
#define ROPSELECT_PRIMARY_ONLY         0x40
#define ROPSELECT_ALL_EXCPT_PRIMARY    0x80
#define ROPSELECT_ALL                  0xc0
#define PIXELMASK_ONLY                 0x00
#define PIXELMASK_AND_SRC_DATA         0x10
#define PIXELMASK_AND_CPU_DATA         0x20
#define PIXELMASK_AND_SCRN_LATCHES     0x30
#define PLANARMASK_ONLY                0x00
#define PLANARMASK_NONE_0XFF           0x04
#define PLANARMASK_AND_CPU_DATA        0x08
#define PLANARMASK_AND_SCRN_LATCHES    0x0c
#define SRC_IS_CPU_DATA                0x00
#define SRC_IS_SCRN_LATCHES            0x01
#define SRC_IS_PATTERN_REGS            0x02
#define SRC_IS_LINE_PATTERN            0x03

#define SOURCE_DATA         0x0C       /* ROP values */
#define DEST_DATA           0x0A

#define START_BLT            0x01      /* BLT_CMD_0 values */
#define NO_BYTE_SWAP         0x00
#define BYTE_SWAP            0x20
#define FORWARD              0x00
#define BACKWARD             0x40
#define WRAP                 0x00
#define NO_WRAP              0x80

#define LIN_SRC_ADDR         0x00      /* BLT_CMD_1 values */
#define XY_SRC_ADDR          0x40
#define LIN_DEST_ADDR        0x00
#define XY_DEST_ADDR         0x80

#define START_LINE           0x01      /* LINE_CMD values */
#define CALC_ONLY            0x02
#define NO_CALC_ONLY         0x00
#define LAST_PIXEL_NULL	     0x04
#define NO_LAST_PIXEL_NULL   0x00
#define KEEP_X0_Y0           0x08
#define NO_KEEP_X0_Y0        0x00
#define RETAIN_PATTERN	     0x10
#define NO_RETAIN_PATTERN    0x00
#define REVERSIBLE_LINE	     0x20
#define NO_REVERSIBLE_LINE   0x00
#define AXIAL_WHEN_0	     0x40
#define NO_AXIAL_WHEN_0	     0x00
#define LINE_RESET           0x80

#define BUFFER_BUSY_BIT      0x80      /* CTRL_REG_1 bit */
#define GLOBAL_BUSY_BIT      0x40

#define SS_BIT               0x01      /* BLT_CMD_0 bit */

#define START_BIT            0x01      /* LINE_CMD bit */

#define X_MAJOR			0	/* Sign (Octant) Code bits */
#define Y_MAJOR			1
#define SIGN_Y_POS		0
#define SIGN_Y_NEG		0x02
#define SIGN_X_POS		0
#define SIGN_X_NEG		0x04

#ifndef VMS
#define TRUE                    1      /* Misc. */
#define FALSE                   0
#endif

#define NO_ROTATE            0x00
#define NO_MASK              0xFF
#define MAX_SCANLINE_DWORDS   256

#define TESTS_PASSED            0      /* TritonPOST() defines */
#define ASIC_FAILURE            1
#define SETMODE_FAILURE         2
#define MEMORY_FAILURE          3
#define DAC_FAILURE             4

#define MODE_32                 0      /* SetExtMode() defines */
#define MODE_37                 1
#define MODE_38                 2
#define MODE_3B                 3
#define MODE_3C                 4
#define MODE_3E                 5
#define MODE_4D                 6
#define MODE_4E                 7
#define MON_CLASS_CNT           4
#define MODE_CNT                8
#define SEQ_CNT                 5
#define CRTC_CNT               25
#define ATTR_CNT               20
#define GRFX_CNT                9

#define MDA                  0x00      /* GetAdapterInfo() defines */
#define CGA                  0x01
#define VGA                  0x02
#define ACCELERATED_VGA      0x03
#define ACCELERATED_VGA_132  0x04
#define ADVANCED_VGA         0x05
#define PORTABLE_ADV_VGA     0x06
#define TRITON_ISA           0x07
#define TRITON_EISA          0x08
#define NONE               0xFFFF


typedef struct {
  int	CtrlReg1;
  int	FGColor;
  int	BGColor;
  int	PlaneMask;
  int	PixelMask;
  int	Rop_A;
  int	DataPathCtrl;
} wgaShadowRegRec, *wgaShadowRegPtr;


#ifndef VMS

#define GLOBALWAIT(screen, a) \
  {  							\
    int __cntr = 0;					\
    while (inp( CTRL_REG_1) & GLOBAL_BUSY_BIT) {	\
      usleep(10);					\
      if (__cntr++ > 5000){				\
	fprintf(stderr,"BLT engine hung in %s GLOBALWAIT; resetting...\n",a); \
	resetBlt();					\
      }							\
    }							\
    if ((screen) != vgaScreenActive) {			\
      outpz(VIRT_CTRLR_SEL, screen);			\
      vgaScreenActive = screen;				\
    }							\
  }

#define WGABUFFEREDWAIT(a)				\
  {							\
    int __cntr = 0;					\
    while (inp( CTRL_REG_1) & BUFFER_BUSY_BIT) {	\
      usleep(10);					\
      if (__cntr++ > 5000){				\
	fprintf(stderr,"BLT engine hung in %s BUFFERWAIT; resetting...\n",a); \
	resetBlt();					\
      }							\
    }							\
  }


/*	externs		*/

/* ALU to MERGE translation table */
extern unsigned char mergexlate[];

#else

#define GLOBALWAIT(a)					\
  {							\
   if (inp( CTRL_REG_1) & GLOBAL_BUSY_BIT)		\
    {							\
     int spin, cntr = 0, cr1d = inp( CTRL_REG_1);	\
     while (cr1d & GLOBAL_BUSY_BIT)			\
      { 						\
       for (spin = 20; spin > 0; spin -= 1)		\
	{						\
         cr1d = inp( CTRL_REG_1);			\
	 if (!(cr1d & GLOBAL_BUSY_BIT)) spin = 0;	\
	}   						\
       if (cr1d & GLOBAL_BUSY_BIT)			\
	{						\
         usleep(10);					\
         if (cntr++ > 500)				\
          {						\
  	   ErrorF("Hung BLT Engine from GLOBALWAIT in %s\n", a); \
	   ErrorF("ctrl reg 1 = %X, resetting engine...\n", cr1d); \
	   resetBlt();					\
	   cntr = 0;					\
	  }						\
	}						\
      }							\
    }							\
  }


/*
 *  The usleep function is costly on VMS, so we will
 *  do a spin loop first.  I will look at some type of
 *  user mode timedwait logic later on.  Currently, a
 *  usleep function requires two VMS system calls -
 *  and the process loses control, and then needs to be
 *  rescheduled.
 *
 */

#define WGABUFFEREDWAIT(a)				\
  {							\
   if (inp( CTRL_REG_1) & BUFFER_BUSY_BIT)		\
    {							\
     int spin, cntr = 0, cr1d = inp( CTRL_REG_1);	\
     while (cr1d & BUFFER_BUSY_BIT)			\
      { 						\
       for (spin = 20; spin > 0; spin -= 1)		\
	{						\
         cr1d = inp( CTRL_REG_1);			\
	 if (!(cr1d & BUFFER_BUSY_BIT)) spin = 0;	\
	}   						\
       if (cr1d & BUFFER_BUSY_BIT)			\
	{						\
         usleep(10);					\
         if (cntr++ > 500)				\
          {						\
  	   ErrorF("Hung BLT Engine from WGABUFFEREDWAIT in %s\n", a); \
	   ErrorF("ctrl reg 1 = %X, resetting engine...\n", cr1d); \
	   resetBlt();					\
	   cntr = 0;					\
	  }						\
	}						\
      }							\
    }							\
  }

/*	externs		*/

/* ALU to MERGE translation table */
extern unsigned int mergexlate[];

#endif

#endif /* _WGA_H_ */
