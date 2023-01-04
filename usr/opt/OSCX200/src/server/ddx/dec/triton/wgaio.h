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
 * @(#)$RCSfile: wgaio.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/11/22 17:35:42 $
 */

#ifndef _WGAIO_H_
#define _WGAIO_H_


/*
 *	This header file includes the macros which write to and read from the
 *	Triton device registers.  It also includes macros to provide shadowing
 *	of selected registers so that they are not written-to if the new
 *	contents are unchanged from the prior.
 */


#define INLINE_IO_ROUTINES

#define SWIZZLE_ADDRESS(a) (a<<7)
    
#define SWIZZLE_DATA(d,a)     \
        (((int)(d)) << ((((unsigned long)(a)) & 3) * 8))

#define UNSWIZZLE_DATA(d,a)   \
        (((int)(d)) >> ((((unsigned long)(a)) & 3) * 8))

#ifdef VMS

#include <builtins.h>

#define _memory_barrier __MB
#define asm(a) { _memory_barrier(); }

/*
 *  This actually may not be broken.  It appears that
 *  for multi-screen we may use the controller select
 *  abilities, and they will all share the same addresses
 *
 */
#define FRAME_BUFFER_ADDRESS onscreen_addr[0]
#define CSR_ADDRESS QVGA_addr[0]

extern int32 QVGA_addr[MAXSCREENS*2],
             QVGA_guard0[MAXSCREENS*2],
             QVGA_guard1[MAXSCREENS*2],
             onscreen_addr[MAXSCREENS];
#else

/*
 *  This is completely untested for OSF.  In JENSENIO the
 *  inp/outp logic would need to be ifdef'ed.
 *
 */

#include <c_asm.h>
#define _memory_barrier() asm("wmb")

#define FRAME_BUFFER_ADDRESS GetFrameAddress(0)
#define CSR_ADDRESS GetVgaAddress(0)

#endif

/*
 *  The following routines replace the code in QVGAVMS_IO (or JENSENIO)
 *  when the INLINE_IO_ROUTINES symbol is defined.  This makes the HW
 *  access an inline function.  It does this by hook or crook, including
 *  making outp, outpz, outpw, and outpwz MACRO's.
 *
 */

#ifdef INLINE_IO_ROUTINES

#define outp(__r1,__d1) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 _memory_barrier();	\
 tmp = (CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r1)); \
 rP  = (unsigned int *)tmp; \
 *rP = SWIZZLE_DATA(__d1,__r1); \
}

#define outpz(__r1,__d1) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 tmp = (CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r1)); \
 rP  = (unsigned int *)tmp; \
 *rP = SWIZZLE_DATA(__d1,__r1); \
}

#define outpw(__r2,__d2) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 _memory_barrier();	\
 tmp = ((CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r2)) + 0x20); \
 rP  = (unsigned int *)tmp; \
 *rP = SWIZZLE_DATA(__d2,__r2); \
}

#define outpwz(__r2,__d2) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 tmp = ((CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r2)) + 0x20); \
 rP  = (unsigned int *)tmp; \
 *rP = SWIZZLE_DATA(__d2,__r2); \
}

#define outpi(__r3,__d3,__e3) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 _memory_barrier();	\
 tmp = ((CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r3)) + 0x60); \
 rP  = (unsigned int *)tmp; \
 *rP = (__d3 | ((__e3) << 16));	\
}

#define outpiz(__r3,__d3,__e3) \
{ \
 char *tmp;		\
 unsigned int *rP;	\
 tmp = ((CSR_ADDRESS + (char *)SWIZZLE_ADDRESS(__r3)) + 0x60); \
 rP  = (unsigned int *)tmp; \
 *rP = (__d3 | ((__e3) << 16));	\
}




#ifdef VMS
/*
 *	usleep() compatibility function
 *
 */
static int
usleep(usec)
unsigned usec;
{
	int Delta[2];

	/*
	 *	Calculate the correct VMS Delta time
	 */
	Delta[0] = -(10*usec);
	Delta[1] = -1;
	/*
	 *	Schedule a wakeup
	 */
	SYS$SCHDWK(0, 0, Delta, 0);
	/*
	 *	Hibernate
	 */
	SYS$HIBER();
	/*
	 *	Done
	 */
}
#endif /* usleep for VMS */
#endif /* INLINE_IO_ROUTINES */



#define WGAUpdateCtrlReg1(shadows, a) {  \
  if ((a) != (shadows)->CtrlReg1)  \
    {  \
      (shadows)->CtrlReg1 = ((a) & 0x1F);  \
      outp (CTRL_REG_1, (a));  \
    }  \
};

#define WGAUpdateCtrlReg1z(shadows, a) {  \
  if ((a) != (shadows)->CtrlReg1)   \
    {  \
      (shadows)->CtrlReg1 = ((a) & 0x1F);  \
      outpz (CTRL_REG_1, (a));  \
    }  \
};

#define WGAReadCtrlReg1(shadows, d)  \
{  \
    { \
      d = inp(CTRL_REG_1);  \
      (shadows)->CtrlReg1 = d & 0x1F;  \
    }  \
};

#define WGAUpdateFGColor(shadows, a) {  \
  if ((a) != (shadows)->FGColor)  \
    {  \
      (shadows)->FGColor = (a);  \
      outpw (GC_INDEX, GC_FG_COLOR+((a)<<8));  \
    }  \
};

#define WGAUpdateFGColorz(shadows, a) {  \
  if ((a) != (shadows)->FGColor)  \
    {  \
      (shadows)->FGColor = (a);  \
      outpwz (GC_INDEX, GC_FG_COLOR+((a)<<8));  \
    }  \
};

#define WGAReadFGColor(shadows, d)  \
{  \
    { \
      outp(GC_INDEX, GC_FG_COLOR);  \
      d = inp(GC_DATA); \
      (shadows)->FGColor = d & 0xFF;  \
    }  \
};

#define WGAUpdateBGColor(shadows, a) {  \
  if ((a) != (shadows)->BGColor)  \
    {  \
      (shadows)->BGColor = (a);  \
      outpw (GC_INDEX, GC_BG_COLOR+((a)<<8));  \
    }  \
};

#define WGAUpdateBGColorz(shadows, a) {  \
  if ((a) != (shadows)->BGColor)  \
    {  \
      (shadows)->BGColor = (a);  \
      outpwz (GC_INDEX, GC_BG_COLOR+((a)<<8));  \
    }  \
};

#define WGAReadBGColor(shadows, d)  \
{  \
    { \
      outp(GC_INDEX, GC_BG_COLOR);  \
      d = inp(GC_DATA); \
      (shadows)->BGColor = d & 0xFF;  \
    }  \
};

#define WGAUpdatePlaneMask(shadows, a) {  \
  if ((a) != (shadows)->PlaneMask)  \
    {  \
      (shadows)->PlaneMask = (a);  \
      outpw (GC_INDEX, GC_PLANE_WR_MSK+((a&0xFF)<<8));  \
    }  \
};

#define WGAUpdatePlaneMaskz(shadows, a) {  \
  if ((a) != (shadows)->PlaneMask)  \
    {  \
      (shadows)->PlaneMask = (a);  \
      outpwz (GC_INDEX, GC_PLANE_WR_MSK+((a&0xFF)<<8));  \
    }  \
};

#define WGAReadPlaneMask(shadows, d)  \
{  \
    { \
      outp(GC_INDEX, GC_PLANE_WR_MSK);  \
      d = inp(GC_DATA); \
      (shadows)->PlaneMask = d & 0xFF;  \
    }  \
};


#define WGAUpdateROP_A(shadows, a) {  \
  if ((a) != (shadows)->Rop_A)  \
    {  \
      (shadows)->Rop_A = (a);  \
      outp (ROP_A,(a));  \
    }  \
};

#define WGAUpdateROP_Az(shadows, a) {  \
  if ((a) != (shadows)->Rop_A)  \
    {  \
      (shadows)->Rop_A = (a);  \
      outpz (ROP_A, (a));  \
    }  \
};

#define WGAReadROP_A(shadows, d)  \
{  \
    { \
      d = inp(ROP_A); \
      (shadows)->Rop_A = d & 0xFF;  \
    }  \
};

#define WGAUpdatePixelMask(shadows, a) {  \
  if ((a) != (shadows)->PixelMask)  \
    {  \
      (shadows)->PixelMask = (a);  \
      outpw (SEQ_INDEX, SEQ_PIXEL_WR_MSK+((a&0xFF)<<8));  \
    }  \
};

#define WGAUpdatePixekMaskz(shadows, a) {  \
  if ((a) != (shadows)->PixelMask)  \
    {  \
      (shadows)->PixelMask = (a);  \
      outpwz (SEQ_INDEX, SEQ_PIXEL_WR_MSK+((a&0xFF)<<8));  \
    }  \
};

#define WGAReadPixelMask(shadows, d)  \
{  \
    { \
      outp(SEQ_INDEX, SEQ_PIXEL_WR_MSK);  \
      d = inp(SEQ_DATA); \
      (shadows)->PixelMask = d & 0xFF;  \
    }  \
};

#define WGAUpdateDataPathCtrl(shadows, a) {  \
  if ((a) != (shadows)->DataPathCtrl)  \
    {  \
      (shadows)->DataPathCtrl = (a);  \
      outpw (GC_INDEX, DATAPATH_CTRL+((a&0xFF)<<8));  \
    }  \
};

#define WGAUpdateDataPathCtrlz(shadows, a) {  \
  if ((a) != (shadows)->DataPathCtrl)  \
    {  \
      (shadows)->DataPathCtrl = (a);  \
      outpwz (GC_INDEX, DATAPATH_CTRL+((a&0xFF)<<8));  \
    }  \
};

#define WGAReadDataPathCtrl(shadows, d)  \
{  \
    { \
      outp(GC_INDEX, DATAPATH_CTRL);  \
      d = inp(GC_DATA); \
      (shadows)->DataPathCtrl = d & 0xFF;  \
    }  \
};

#endif  /* _WGAIO_H_ */
