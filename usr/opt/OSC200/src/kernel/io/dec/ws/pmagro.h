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
 *	@(#)$RCSfile: pmagro.h,v $ $Revision: 1.2.4.4 $ (DEC) $Date: 1992/07/09 10:10:06 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *    pmagro.h: Definitions associated with RasterOps Frame Buffer OEM.
 */
#ifndef pmagro_DEFINED
#define pmagro_DEFINED 1
#include <sys/types.h>
#include <io/dec/ws/bt463.h>
#include <io/dec/ws/bt431.h>

/*
 *    Offsets in the Turbo Card address space of various components
 */
#define TCO_CURSOR0       0x040010  /* Cursor 0. */
#define TCO_DUTY_CYCLE    0x040020  /* Duty Cycle register*/

#define TCO_EIGHT_BIT     0x200000  /* Eight bit frame buffer. */
#define TCO_FB_REGISTERS  0x040000  /* Frame buffer device registers. */
#define TCO_MAP_REGISTER  0x040030  /* vid on, int ena, int pend,seg ena,seg */
#define TCO_PIP_REGISTERS 0x0800C0  /* Pip device registers. */
#define TCO_SELECTION     0x100000  /* Selection memory. */
#define TCO_TRUE_COLOR    0x400000  /* True color frame buffer. */
#define TCO_VIDEO_ENABLE  0xA00000  /* Video enable memory. */
#define TCO_3MAX_MAP_AREA 0x200000  /* Area in 3MAX slot which is mapped. */

typedef volatile u_char Reg;
typedef struct {
	u_int regs[4];
} Bt431;

/*
 *    Frame buffer device register area layout:
 */
typedef struct {
#ifdef __alpha
/* adjustments for SPARSE address space accesses */
    struct bt463    bt463;        /* bt 463 registers mapped one per word. */
    struct bt431  cursor0;      /* bt431 cursor 0 registers (1 per word). */
    struct bt431  cursor1;      /* Bt431 cursor 1 registers (1 per word). */
    volatile unsigned int fb_control;   /* Frame buffer control register. */
#else /* __alpha */
    struct bt463    bt463;        /* bt 463 registers mapped one per word. */
    struct bt431  cursor0;      /* bt431 cursor 0 registers (1 per word). */
    struct bt431  cursor1;      /* Bt431 cursor 1 registers (1 per word). */
    Reg      fb_control;   /* Frame buffer control register. */
	char	 padd[3];	  /* ... */
#endif /* __alpha */
}  Fb_Device_Regs;

#define TC1_FBC_VIDEO_ON  0x80
#define TC1_FBC_INT_ENA	  0x40
#define TC1_FBC_INT_PEND  0x20
#define TC1_FBC_SEG_ENA	  0x10
#define TC1_FBC_SEG	  0x0f

/*
 *    Picture in a picture device register layout:
 */
typedef struct {
#ifdef __alpha
/* adjustments for SPARSE address space accesses */
    volatile unsigned int    	control_status; 	/* 800C0: pip control/status reg 0. */
    volatile unsigned int	control_status1; 	/* 800C4: pip control/status reg 1. */
    unsigned int		fill1[14];		/* 800C8 - 800FF Unused. */
    volatile unsigned int	x_source_start_hi;  	/* 80100: video source start
							   (high). */
    volatile unsigned int	x_source_start_low;	/* 80104: video source start
							 (low). */
    volatile unsigned int	x_source_end_hi;	/* 80108: video source end
						 	(high). */
    volatile unsigned int	x_source_end_low;	/* 8010C: video source end
							 (low). */
    volatile unsigned int	x_source_scale_hi;	/* 80110: video source scale
							 (high). */
    volatile unsigned int	x_source_scale_low;	/* 80114: video source scale
							 (low). */
    unsigned int		fill2[2];		/* 80118 - 8011F Unused. */
    volatile unsigned int	y_source_start_hi;	/* 80120: video source start
							 (high). */
    volatile unsigned int	y_source_start_low;	/* 80124: video source start
							 (low). */
    volatile unsigned int	y_source_end_hi;	/* 80128: video source end
							 (high). */
    volatile unsigned int	y_source_end_low;	/* 8012C: video source end
							 (low). */
    volatile unsigned int	y_source_scale_hi;	/* 80130: video source scale
							 (high). */
    volatile unsigned int	y_source_scale_low;	/* 80134: video source scale
							 (low). */
    volatile unsigned int	fb_pitch_hi;		/* 80138: frame buffer pitch
							 (high). */
    volatile unsigned int	fb_pitch_low;		/* 8013C: frame buffer pitch
							 (low). */
    unsigned int		fill3[1];		/* 80140 - 80143 Unused. */

    volatile unsigned int	start_offset_hi;	/* 80144: pip starting offset
							 (high). */
    volatile unsigned int	start_offset_mid;	/* 80148: pip starting offset
							 (mid). */
    volatile unsigned int	start_offset_low;	/* 8014C: pip starting offset
								 (low). */
    volatile unsigned int	control_status2;	/* 80150: pip control/status
								 reg 2. */
#else /* __alpha */
    volatile unsigned char    control_status; 	/* 800C0: pip control/status reg 0. */
    char			pad1[3];			/* ... */
    volatile unsigned char	control_status1; /* 800C4: pip control/status reg 1. */
    char			pad2[3];			/* ... */
    unsigned char			fill1[56];	/* 800C8 - 800FF Unused. */

    volatile unsigned char	x_source_start_hi;  /* 80100: video source start
							 (high). */
    unsigned char			pad3[3];			/* ... */
    volatile unsigned char	x_source_start_low;	/* 80104: video source start
							 (low). */
    unsigned char			pad4[3];			/* ... */
    volatile unsigned char	x_source_end_hi;	/* 80108: video source end
						 	(high). */
    unsigned char			pad5[3];			/* ... */
    volatile unsigned char	x_source_end_low;	/* 8010C: video source end
							 (low). */
    unsigned char			pad6[3];			/* ... */
    volatile unsigned char	x_source_scale_hi;	/* 80110: video source scale
							 (high). */
    unsigned char			pad7[3];			/* ... */
    volatile unsigned char	x_source_scale_low;	/* 80114: video source scale
							 (low). */
    unsigned char			pad8[3];			/* ... */
    unsigned char			fill2[8];	/* 80118 - 8011F Unused. */

    volatile unsigned char	y_source_start_hi;	/* 80120: video source start
							 (high). */
    unsigned char			pad9[3];			/* ... */
    volatile unsigned char	y_source_start_low;	/* 80124: video source start
							 (low). */
    unsigned char			pad10[3];			/* ... */
    volatile unsigned char	y_source_end_hi;	/* 80128: video source end
							 (high). */
    unsigned char			pad11[3];			/* ... */
    volatile unsigned char	y_source_end_low;	/* 8012C: video source end
							 (low). */
    unsigned char			pad12[3];			/* ... */
    volatile unsigned char	y_source_scale_hi;	/* 80130: video source scale
							 (high). */
    unsigned char			pad13[3];			/* ... */
    volatile unsigned char	y_source_scale_low;	/* 80134: video source scale
							 (low). */
    unsigned char			pad14[3];			/* ... */
    volatile unsigned char	fb_pitch_hi;		/* 80138: frame buffer pitch
							 (high). */
    unsigned char			pad15[3];			/* ... */
    volatile unsigned char	fb_pitch_low;		/* 8013C: frame buffer pitch
							 (low). */
    unsigned char			pad16[3];			/* ... */
    unsigned char			fill3[4];	/* 80140 - 80143 Unused. */

    volatile unsigned char	start_offset_hi;	/* 80144: pip starting offset
							 (high). */
    unsigned char			pad17[3];			/* ... */
    volatile unsigned char	start_offset_mid;	/* 80148: pip starting offset
							 (mid). */
    unsigned char			pad18[3];			/* ... */
    volatile unsigned char	start_offset_low;	/* 8014C: pip starting offset
								 (low). */
    unsigned char			pad19[3];			/* ... */
    volatile unsigned char	control_status2;	/* 80150: pip control/status
								 reg 2. */
    char			pad20[3];			/* ... */
#endif /* __alpha */
}   Pip_Device_Regs;

/*    
 *	Pip Control Status Register 0 definitions:
 */
#define TC1_CSR0_PIP_ONE_SHOT 0x40 /* Set to 1 to do a one-shot frame capture*/

#define TC1_CSR0_PIP_IS_ON    	  0x40	/* = 1 on read: pip is turned on. 
					(H/W calls this PIP_BUSY)*/

#define TC1_CSR0_PIP_IS_ACTIVE      0x20	/* = 1 on read: pip is actively
					 generating images. (H/W PIP_ON)*/

#define TC1_CSR0_TURN_PIP_ON      0x20	/* Set to 1 to make pip active, set to
					 0 to stop. */

#define TC1_CSR0_PIP_INSTALLED      0x10 /* = 1 if pip present in system. */

#define TC1_CSR0_SOURCE_TYPE_MASK 0x03    /* Mask to get video source value. */

#define TC1_CSR0_COMPOSITE_SOURCE 0x00   /* ... video source is composite 
						(1 wire.) */

#define TC1_CSR0_S_VIDEO_SOURCE      0x01	/* ... video source is s-video
						 (2 wire.) */
#define TC1_CSR0_RGB_SOURCE    	  0x02	/* ... video source is rgb (3 wire.) */

/*    Control Status Register 1 definitions:
 */
#define TC1_CSR1_I2C_DATA    	  0x80	/* I2C bus interface serial data bit.*/
#define TC1_CSR1_I2C_CLOCK    	  0x40	/* I2C bus interface clock bit. */
#define TC1_CSR1_FIELD_ONLY    	  0x10	/* = 1 if compose image with only 1
					field (1/2 size.) */
#define TC1_CSR1_NO_PIP		  0x02  /* = 1 if there is no pip present. */
#define TC1_CSR1_INPUT_CONNECTED  0x01  /* = 1 if pip input connected. */

/*    
 *	Control Status Register 2 definitions:
 */
#define TC1_CSR2_ALTERNATE_PLL      0x01 /* = 1 to use VCR PLL (slow timing.)*/
#define TC1_CSR2_COUNT_DOWN  0x02  /* = 1 if address counts down (horizontal
						 flip.) */
#define TC1_CSR2_FIELD_INVERT     0x04    /* = 1 to show odd field before even
						 field. */

/*
 *	Combinations of fields in the device register area.
 */
#define Start_Offset ( ((pregs->x_start_offset_hi & 0xff) << 16) | \
	((pregs->start_offset_mid & 0xff) << 8) | \
	(pregs->start_offset_low & 0xff) )

#define X_Source_End ( ((pregs->x_source_end_hi & 0xff) << 8) | \
	(pregs->x_source_end_low & 0xff) )
#define X_Source_Scale ( ((pregs->x_source_scale_hi & 0xff) << 8) | \
	(pregs->x_source_scale_low & 0xff) )
#define X_Source_Start ( ((pregs->x_source_start_hi & 0xff) << 8) | \
	(pregs->x_source_start_low & 0xff) )

#define Y_Source_End ( ((pregs->y_source_end_hi & 0xff) << 8) | \
	(pregs->y_source_end_low & 0xff) )
#define Y_Source_Scale ( ((pregs->y_source_scale_hi & 0xff) << 8) | \
	(pregs->y_source_scale_low & 0xff) )
#define Y_Source_Start ( ((pregs->y_source_start_hi & 0xff) << 8) | \
	(pregs->y_source_start_low & 0xff) )

/*    
 *	I-squared C bus device addresses and command definitions:
 */
#define I2C_ADDR_EEPROM    (u_int)0xa0	/* Address: eeprom to store
					 configuration. */

/*************************************************************************
 *                                                                        
 *  Layout of EEPROM Storage and Related Structures:                     
 *
 *                                                                             
 *
 ***********************************************************************
 */
typedef struct {                /* PIP Initialization record: */ 
    u_char      timing_mode;    /* ... timing mode specification. */ 
     
    u_char      brightness;     /* ... digital to analog converter brightness
						 value. */ 
    u_char      contrast;       /* ... digital to analog converter contrast
						 value. */ 
    u_char      saturation;     /* ... digital to analog converter saturation
						 value. */ 
    u_char      hue;            /* ... digital to analog converter hue value.*/ 
     
    u_char		fill;			/* ... *** UNUSED *** */
     
    u_short     x_source_start; /* ... starting pixel of signal in the x
						 direction. */
    u_short     x_source_end;   /* ... ending pixel of signal in the x
						 direction. */
    u_short     y_source_start; /* ... starting pixel of signal in the y
					 	direction. */
    u_short     y_source_end;   /* ... ending pixel of signal in the y
						 direction. */
     
    u_char  rgb_brightness;  /* ... digital to analog converter rgb 
						brightness. */
    u_char		rgb_contrast;	/* ... digital to analog converter rgb
						 brightness. */
    
}   Pip_Init_Record;   
 
#define EEPROM_FACTORY -1    /* Use factory timing default (i.e., location 0 
						in eeprom). */
#define EEPROM_NTSC 0       /* Default timing mode is NTSC. */
#define EEPROM_PAL  1       /* Default timing mode is PAL. */
#define EEPROM_NUM_MODES EEPROM_PAL+1

typedef struct {                    		/* EEPROM storage layout: */
    u_char          default_mode;  	   /* ... Default mode selection. */
    u_char			fill[15];	/* ... *** UNUSED *** */
    Pip_Init_Record mode[EEPROM_NUM_MODES]; /* ... NTSC, PAL timing mode
						 defaults. */
}   EEPROM_Record;


extern int pmagro_attach();
extern void pmagro_interrupt();
extern void pmagro_bt463_enable_interrupt();
extern void pmagro_bt431_enable_interrupt();
extern caddr_t pmagro_bt431_init_closure();
extern caddr_t pmagro_bt463_init_closure();
extern int pmagro_recolor_cursor();
extern int pmagro_video_on();
extern int pmagro_video_off();
extern int pmagro_map_screen();
extern int pmagro_load_cursor();
extern int pmagro_ioctl();


typedef struct {
        short screen;
        short command;
} rop_wait_ioctl;

#define RO_MAP    _IOWR('w',  (42|IOC_S), int)

#define PIP_WAIT    _IOW ('w', (0|IOC_S), rop_wait_ioctl)
#define PIP_WAIT_OFF 1
#define PIP_WAIT_ON 2

#endif /* !pmagro_DEFINED */
