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
 * @(#)$RCSfile: osvideo.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/08/02 20:37:27 $
 */

#define OT_INT10 100
#define OT_OUTB  101

typedef struct _Open
{
    struct _Open *next;
    int           type;
    union
    {
	struct
	{
	    short ax;
	    short bx;
	    short cx;
	    short dx;
	} int10;

	struct
	{
	    int           port;
	    unsigned char value;
	} outb;
    } u;
    
} OpenRec;


typedef struct _VideoChar
{
    int           class;
    OpenRec      *open;
    int           close;
    int           width;
    int           height;
    int           virt_width;
    int           virt_height;
    int           byte_width;
    int           dpi_x;
    int           dpi_y;
    int           ram_start;
    int           ram_end;
    void         *ram_addr;
} VideoCharRec;

extern VideoCharRec Video_char;

