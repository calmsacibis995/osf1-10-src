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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/bitmaps.h,v 1.1.2.3 92/04/02 18:24:59 Russ_Kuhn Exp $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)bitmaps.h	3.3 90/05/16":
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:        bitmaps.h
 **
 **   Description: This file contains a set of predefines bitmaps
 **		   which are used by the image caching functions.
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _bitmaps_h
#define _bitmaps_h

#ifdef DEC_MOTIF_EXTENSION
#if defined(mips) || defined (SUN)	    /* mips c doesn't accept const */
static unsigned char bitmaps [20][32] =
#else
static const unsigned char bitmaps [20][32] =
#endif /* mips */
#else /* not DEC_MOTIF_EXTENSION */
static unsigned char bitmaps [20][32] =
#endif /* DEC_MOTIF_EXTENSION */
{
   {  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*  Solid Background  */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  },

   {  0x88, 0x88, 0x22, 0x22, 0x88, 0x88, 0x22, 0x22,	/*  25 percent  */
      0x88, 0x88, 0x22, 0x22, 0x88, 0x88, 0x22, 0x22,
      0x88, 0x88, 0x22, 0x22, 0x88, 0x88, 0x22, 0x22,
      0x88, 0x88, 0x22, 0x22, 0x88, 0x88, 0x22, 0x22  },

   {  0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA,	/*  50 percent  */
      0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA,
      0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA,
      0x55, 0x55, 0xAA, 0xAA, 0x55, 0x55, 0xAA, 0xAA  },

   {  0x55, 0x55, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF,	/*  75 percent  */
      0x55, 0x55, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF,
      0x55, 0x55, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF,
      0x55, 0x55, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF  },

   {  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,	/*  Vertical  */
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55  },

   {  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,	/*  Horizontal  */ 
      0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
      0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 
      0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00  },

   {  0x77, 0x77, 0xbb, 0xbb, 0xdd, 0xdd, 0xee, 0xee,	/*  Slant Left  */
      0x77, 0x77, 0xbb, 0xbb, 0xdd, 0xdd, 0xee, 0xee, 
      0x77, 0x77, 0xbb, 0xbb, 0xdd, 0xdd, 0xee, 0xee, 
      0x77, 0x77, 0xbb, 0xbb, 0xdd, 0xdd, 0xee, 0xee  },

   {  0xee, 0xee, 0xdd, 0xdd, 0xbb, 0xbb, 0x77, 0x77,	/*  Slant Right  */
      0xee, 0xee, 0xdd, 0xdd, 0xbb, 0xbb, 0x77, 0x77, 
      0xee, 0xee, 0xdd, 0xdd, 0xbb, 0xbb, 0x77, 0x77, 
      0xee, 0xee, 0xdd, 0xdd, 0xbb, 0xbb, 0x77, 0x77  },

   {  						
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /*  Default Cascade  */ 
     0x00, 0x03, 0x00, 0x06, 0x00, 0x0c, 0x00, 0x18,  
     0xff, 0x3f, 0x00, 0x18, 0x00, 0x0c, 0x00, 0x06,  
     0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   },

#ifdef DEC_MOTIF_RTOL
   {  						
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /*  Default RtoL Cascade */ 
     0xc0, 0x00, 0x60, 0x00, 0x30, 0x00, 0x18, 0x00,
     0xfc, 0xff, 0x18, 0x00, 0x30, 0x00, 0x60, 0x00,
     0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   },
#endif DEC_MOTIF_RTOL
   {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* Default CheckMark */
      0x00, 0x00, 0x00, 0x60, 0x00, 0x30, 0x00, 0x18, 
      0x00, 0x0c, 0x08, 0x06, 0x18, 0x03, 0xb0, 0x01, 
      0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  }, 
   {  						
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /*  Default menu dash */ 
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
     0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   },
};


static char * bitmap_name_set[] =
{
   "background",
   "25_foreground",
   "50_foreground",
   "75_foreground",
   "vertical",
   "horizontal",
   "slant_right",
   "slant_left",
   "menu_cascade",
#ifdef DEC_MOTIF_RTOL
   "menu_cascade_rtol",
#endif DEC_MOTIF_RTOL
   "menu_checkmark",
   "menu_dash"
};
#endif /* _bitmaps_h */
/* DON'T ADD STUFF AFTER THIS #endif */
