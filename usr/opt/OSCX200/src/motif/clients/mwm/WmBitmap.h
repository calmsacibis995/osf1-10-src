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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmBitmap.h,v $ $Revision: 1.1.2.3 $ $Date: 1993/07/16 21:47:44 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Global Variables And Definitions:
 */

/*
 * Define HOURGLASS cursor
 */

#ifdef LARGECURSORS

#define time32_width 32
#define time32_height 32
#define time32_x_hot 15
#define time32_y_hot 15
static char time32_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f,
   0x8c, 0x00, 0x00, 0x31, 0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32,
   0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32, 0x4c, 0x00, 0x00, 0x32,
   0x8c, 0x00, 0x00, 0x31, 0x0c, 0x7f, 0xfe, 0x30, 0x0c, 0xfe, 0x7f, 0x30,
   0x0c, 0xfc, 0x3f, 0x30, 0x0c, 0xf8, 0x1f, 0x30, 0x0c, 0xe0, 0x07, 0x30,
   0x0c, 0x80, 0x01, 0x30, 0x0c, 0x80, 0x01, 0x30, 0x0c, 0x60, 0x06, 0x30,
   0x0c, 0x18, 0x18, 0x30, 0x0c, 0x04, 0x20, 0x30, 0x0c, 0x02, 0x40, 0x30,
   0x0c, 0x01, 0x80, 0x30, 0x8c, 0x00, 0x00, 0x31, 0x4c, 0x80, 0x01, 0x32,
   0x4c, 0xc0, 0x03, 0x32, 0x4c, 0xf0, 0x1f, 0x32, 0x4c, 0xff, 0xff, 0x32,
   0xcc, 0xff, 0xff, 0x33, 0x8c, 0xff, 0xff, 0x31, 0xfe, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00};

#define time32m_width 32
#define time32m_height 32
static char time32m_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xcf, 0x00, 0x00, 0xf3, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76,
   0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76,
   0xce, 0x00, 0x00, 0x73, 0x8e, 0x7f, 0xfe, 0x71, 0x0e, 0xff, 0xff, 0x70,
   0x0e, 0xfe, 0x7f, 0x70, 0x0e, 0xfc, 0x3f, 0x70, 0x0e, 0xf8, 0x1f, 0x70,
   0x0e, 0xe0, 0x07, 0x70, 0x0e, 0xe0, 0x07, 0x70, 0x0e, 0x78, 0x1e, 0x70,
   0x0e, 0x1c, 0x38, 0x70, 0x0e, 0x06, 0x60, 0x70, 0x0e, 0x03, 0xc0, 0x70,
   0x8e, 0x01, 0x80, 0x71, 0xce, 0x00, 0x00, 0x73, 0x6e, 0x80, 0x01, 0x76,
   0x6e, 0xc0, 0x03, 0x76, 0x6e, 0xf0, 0x1f, 0x76, 0x6e, 0xff, 0xff, 0x76,
   0xee, 0xff, 0xff, 0x77, 0xcf, 0xff, 0xff, 0xf3, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#endif /* LARGECURSORS */


#define time16_x_hot 7
#define time16_y_hot 7
#define time16_width 16
#define time16_height 16
static char time16_bits[] = {
   0x00, 0x00, 0xfe, 0x7f, 0x14, 0x28, 0x14, 0x28, 0x14, 0x28, 0x24, 0x24,
   0x44, 0x22, 0x84, 0x21, 0x84, 0x21, 0x44, 0x22, 0x24, 0x24, 0x14, 0x28,
   0x94, 0x29, 0xd4, 0x2b, 0xfe, 0x7f, 0x00, 0x00};

#define time16m_width 16
#define time16m_height 16
static char time16m_bits[] = {
   0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f,
   0xfe, 0x7f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff};



/*
 * Define DO NOT ENTER cursor
 */

#ifdef LARGECURSORS

#define noenter32_width 32
#define noenter32_height 32
#define noenter32_x_hot 15
#define noenter32_y_hot 15

static char noenter32_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xfc, 0x3f, 0x00,
   0x00, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x03,
   0xe0, 0xff, 0xff, 0x07, 0xf0, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x1f,
   0xf8, 0xff, 0xff, 0x1f, 0xfc, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0xff, 0x3f,
   0xfc, 0xff, 0xff, 0x3f, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60,
   0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x60,
   0x06, 0x00, 0x00, 0x60, 0xfc, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0xff, 0x3f,
   0xfc, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xff, 0xff, 0x1f,
   0xf0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x03,
   0x80, 0xff, 0xff, 0x01, 0x00, 0xff, 0xff, 0x00, 0x00, 0xfc, 0x3f, 0x00,
   0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};

static char noenter32m_bits[] = {
   0x00, 0xe0, 0x07, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xff, 0xff, 0x00,
   0x80, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x07,
   0xf0, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x1f, 0xfc, 0xff, 0xff, 0x3f,
   0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x7f, 0xfe, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0xff, 0x3f, 0xfc, 0xff, 0xff, 0x3f,
   0xf8, 0xff, 0xff, 0x1f, 0xf0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x07,
   0xc0, 0xff, 0xff, 0x03, 0x80, 0xff, 0xff, 0x01, 0x00, 0xff, 0xff, 0x00,
   0x00, 0xfc, 0x3f, 0x00, 0x00, 0xe0, 0x07, 0x00};

#endif /* LARGECURSORS */

#define noenter16_width 16
#define noenter16_height 16
#define noenter16_x_hot 7
#define noenter16_y_hot 7

static char noenter16_bits[] = {
   0x00, 0x00, 0xc0, 0x03, 0xf0, 0x0f, 0xf8, 0x1f, 0xfc, 0x3f, 0xfc, 0x3f,
   0xfe, 0x7f, 0x02, 0x40, 0x02, 0x40, 0xfe, 0x7f, 0xfc, 0x3f, 0xfc, 0x3f,
   0xf8, 0x1f, 0xf0, 0x0f, 0xc0, 0x03, 0x00, 0x00};

static char noenter16m_bits[] = {
   0xc0, 0x03, 0xf0, 0x0f, 0xf8, 0x1f, 0xfc, 0x3f, 0xfe, 0x7f, 0xfe, 0x7f,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f,
   0xfc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f, 0xc0, 0x03};
