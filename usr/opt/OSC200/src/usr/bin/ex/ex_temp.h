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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: ex_temp.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 20:35:41 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDEDIT) ex_temp.h
 *
 * FUNCTION: none
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.7 com/cmd/edit/vi/ex_temp.h, cmdedit, bos320 6/5/91 23:32:05 
 * 
 * Copyright (c) 1981 Regents of the University of California
 * 
 */
/* Copyright (c) 1981 Regents of the University of California */

/*
 * The editor uses a temporary file for files being edited, in a structure
 * similar to that of ed.  The first block of the file is used for a header
 * block which guides recovery after editor/system crashes.
 * Lines are represented in core by a pointer into the temporary file which
 * is packed into 16 bits (32 on VMUNIX).  All but the low bit index the temp
 * file; the last is used by global commands.  The parameters below control
 * how much the other bits are shifted left before they index the temp file.
 * Larger shifts give more slop in the temp file but allow larger files
 * to be edited.
 *
 * The editor does not garbage collect the temporary file.  When a new
 * file is edited, the temporary file is rather discarded and a new one
 * created for the new file.  Garbage collection would be rather complicated
 * in ex because of the general undo, and in any case would require more
 * work when throwing lines away because marks would have be carefully
 * checked before reallocating temporary file space.  Said another way,
 * each time you create a new line in the temporary file you get a unique
 * number back, and this is a property used by marks.
 *
 * The following temp file parameters allow 256k bytes in the temporary
 * file.  By changing to the numbers in comments you can get 512k.
 * For VMUNIX you get more than you could ever want.
 * VMUNIX uses long (32 bit) integers giving much more
 * space in the temp file and no waste.  This doubles core
 * requirements but allows files of essentially unlimited size to be edited.
 */
#define	BLKMSK	077777
#define NMBLKS	077770

/* These constants are in terms of "characters", not bytes */
#define	LOGINCRMT 11
#define	INCRMT	  (1 << LOGINCRMT)
#define BUFBYTES  (INCRMT * sizeof(wchar_t))
#define	OFFMSK	  (INCRMT-1)
#define	LBTMSK	  (OFFMSK & ~1)
#define	OFFBTS	  LOGINCRMT
#define	SHFT	  0	/* don't know what this does */
#define	BNDRY	2

/*
 * The editor uses three buffers into the temporary file (ed uses two
 * and is very similar).  These are two read buffers and one write buffer.
 * Basically, the editor deals with the file as a sequence of INCRMT character
 * blocks.  Each block contains some number of lines (and lines
 * can run across block boundaries.
 *
 * New lines are written into the last block in the temporary file
 * which is in core as obuf.  When a line is needed which isn't in obuf,
 * then it is brought into an input buffer.  As there are two, the choice
 * is to take the buffer into which the last read (of the two) didn't go.
 * Thus this is a 2 buffer LRU replacement strategy.  Measurement
 * shows that this saves roughly 25% of the buffer reads over a one
 * input buffer strategy.  Since the editor (on our VAX over 1 week)
 * spends (spent) roughly 30% of its time in the system read routine,
 * this can be a big help.
 */
var short	hitin2;		/* Last read hit was ibuff2 not ibuff */
var short	ichang2;	/* Have actually changed ibuff2 */
var short	ichanged;	/* Have actually changed ibuff */
var short	iblock;		/* Temp file block number of ibuff (or -1) */
var short	iblock2;	/* Temp file block number of ibuff2 (or -1) */
var short	ninbuf;		/* Number useful chars left in input buffer */
var short	nleft;		/* Number usable chars left in output buffer */
var short	oblock;		/* Temp file block number of obuff (or -1) */
var int	tline;
/* largest possible line should fit into these buffers */
var wchar_t 	ibuff  [INCRMT];
var wchar_t 	ibuff2 [INCRMT];
var wchar_t 	obuff  [INCRMT];

/*
 * Structure of the descriptor block which resides
 * in the first block of the temporary file and is
 * the guiding light for crash recovery.
 *
 * As the Blocks field below implies, there are temporary file blocks
 * devoted to (some) image of the incore array of pointers into the temp
 * file.  Thus, to recover from a crash we use these indices to get the
 * line pointers back, and then use the line pointers to get the text back.
 * Except for possible lost lines due to sandbagged I/O, the entire
 * file (at the time of the last editor "sync") can be recovered from
 * the temp file.
 */

/* This definition also appears in expreserve.c... beware */
struct 	header {
	time_t	Time;			/* Time temp file last updated */
	uid_t	Uid;
	int	Flines;
	char	Savedfile[FNSIZE];	/* The current file name */
	short	Blocks[LBLKS];		/* Blocks where line pointers stashed */
}; 
var struct 	header H;

#define	uid		H.Uid
#define	flines		H.Flines
#define	savedfile	H.Savedfile
#define	blocks		H.Blocks
