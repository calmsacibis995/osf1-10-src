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
 * @(#)$RCSfile: wgamacros.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/11/22 17:35:49 $
 */

/*
 *  WGAMACROS.H
 *
 *  This module contains macros to access the QVision SVGA engine for drawing
 *
 */

#ifndef _WGAMACROS_H_
#define _WGAMACROS_H_

#include "wgaio.h"



/*
 *  This function sets up the hardware context.  It is normally called at
 *  the entry to the line drawing routine to insure that all the state is
 *  valid.  The WGAUpdateXXXX functions check to see if the hardware state
 *  is already set, and if not set it - and store the current state.
 *
 */

#define WGASETUPLINE(screen, shadows, fg, alu, planemask, style) \
{  \
  GLOBALWAIT((screen), "WGASETUPLINE");		\
  WGAUpdateCtrlReg1(shadows, EXPAND_TO_FG|BITS_PER_PIX_8 | ENAB_TRITON_MODE);  \
  WGAUpdateROP_A(shadows, mergexlate[alu]);	\
  WGAUpdatePlaneMask(shadows, planemask);	\
  WGAUpdateFGColor(shadows, fg);			\
  WGAUpdateDataPathCtrl(shadows, SRC_IS_LINE_PATTERN | ROPSELECT_ALL | PIXELMASK_ONLY); \
  outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));  \
  if (style != CapNotLast)			\
   {						\
    outpw (GC_INDEX,LINE_CMD+((RETAIN_PATTERN)<<8));  \
   }						\
  else    					\
   {						\
    outpw (GC_INDEX,LINE_CMD+((RETAIN_PATTERN | LAST_PIXEL_NULL)<<8));  \
   } \
}




/*
 *
 *  This routine emits a point to the engine.  If the first point is not
 *  flagged as valid, then it sets up the initial point and sets the valid
 *  flag to true.
 *
 */

#define WGABRESLINE(screen, valid, x1, y1, x2, y2) \
{  \
  if (!valid)  \
    {  \
      GLOBALWAIT((screen), "WGABRESLINE");  \
      outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));  \
      outpw (GC_INDEX,LINE_CMD+((RETAIN_PATTERN | LAST_PIXEL_NULL)<<8));  \
      outpi (X0_SRC_ADDR_LO, x1, y1); \
      valid = TRUE; \
    } else { \
      WGABUFFEREDWAIT("BRESLINE"); \
    } \
  outpi (X1, x2, y2);  \
}




/*
 *  This function sets up the hardware context.  It is normally called at
 *  the entry to the segment drawing routine to insure that all the state is
 *  valid.  The WGAUpdateXXXX functions check to see if the hardware state
 *  is already set, and if not set it - and store the current state.
 *
 */
#define WGASETUPSEGMENT(screen, shadows, fg, alu, planemask, style) \
{  \
  GLOBALWAIT((screen), "WGASETUPSEGMENT");     \
  WGAUpdateCtrlReg1(shadows, EXPAND_TO_FG|BITS_PER_PIX_8 | ENAB_TRITON_MODE);  \
  WGAUpdateROP_A (shadows, mergexlate[alu]);  \
  WGAUpdatePlaneMask(shadows, planemask);     \
  WGAUpdateFGColor(shadows, fg);              \
  WGAUpdateDataPathCtrl(shadows, SRC_IS_LINE_PATTERN | ROPSELECT_ALL | PIXELMASK_ONLY); \
  outpw (GC_INDEX,LINE_CMD+((LINE_RESET)<<8));  \
  if (style != CapNotLast)			\
   {						\
    outpw (GC_INDEX,LINE_CMD+((RETAIN_PATTERN | KEEP_X0_Y0)<<8));  \
   }						\
  else    					\
   {						\
    outpw (GC_INDEX,LINE_CMD+((RETAIN_PATTERN | LAST_PIXEL_NULL | KEEP_X0_Y0)<<8));  \
   } \
}




/*
 *  This routine emits a pair of segment end-points to the engine.  The 
 *  engine has already been set up by calling WGASETUPSEGMENT.
 */
#define WGABRESSEGMENT(screen, x1, y1, x2, y2) \
{  \
  GLOBALWAIT((screen), "WGABRESSEGMENT");  \
  outpi (X0_SRC_ADDR_LO, x1, y1); \
  outpi (X1, x2, y2);  \
  asm("wmb");      \
}

#endif /* _WGAMACROS_H_ */
