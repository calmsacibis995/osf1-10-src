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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/cmplrs/prof_header.h,v 4.2.11.2 1993/11/18 01:26:36 Linda_Wilson Exp $ */
/* Definition of the header for a "mon.out" profile data file, or for */
/* a "bbcounts/bbaddrs" pair of files.				      */

#ifndef _PROF_HEADER_H_
#define _PROF_HEADER_H_

#define BBADDRS_MAGIC 0x0f0e0010
#define BBCOUNTS_MAGIC 0x0f0e0a11
#define ICOUNTS_MAGIC 0x0f0e0012

#define ADDHASH(v, w) \
	((v) = ((v) << 5) ^ ((v) >> (32-5)) ^ (w))

#define MAGIC 0x0f0e0000
#define HAS_PC_SAMPLES 1
#define HAS_INV_COUNTS 2
#define HAS_BB_COUNTS 4
#define HAS_INT_SAMPLES 8	/* PC-samples are unsigned ints rather
				   than unsigned shorts */
struct prof_header {
   /* Lower and upper limits of pc values for pc-sampling */
   unsigned long low_pc;
   unsigned long high_pc;
   /* A magic number which tells "mprof" what data to expect in the
     profile output file, using the "define"s above */
   int p_opt_value;
   /* Size of the pc-sampling array, bytes */
   int pc_buf_size;
   /* Size of bb-counting array, bytes. Set to 0 in mon.out files */
   int count_buf_size;
   };

#define BB_SCALE 2	/* bb array is always half the size of text segment */
#define SAMPLE_PERIOD 10.0e-3 /* interval between pc samples in seconds */

/*

A "mon.out" file consists of:

   struct prof_header the_header;
   unsigned short pc_buffer[the_header.pc_buf_size];
   unsigned int count_buffer[the_header.count_buf_size];

   The count_buffer is optional and not generated for programs compiled -p.

A "bbaddrs" file consists of:

   unsigned int magic = BBADDRS_MAGIC;
   unsigned int hash;
   unsigned int addrs[n + 1];

A "bbcounts" file consists of:

   unsigned int magic = BBCOUNTS_MAGIC;
   unsigned int hash;
   unsigned long count[n];

If the executable contains .init and .fini sections in addition to .text,
pixie and prof treat them as one continuous .text section.

To compute the hash, apply ADDHASH(hash, word) to each word of the text
section of the pre-pixie executable file. The addrs array gives the start
of each basic block, expressed as a word offset from the beginning of the
text. These offsets appear in increasing order.  The last element of the
array is the size of the text in words. The count array gives the number
of times the corresponding basic block was executed.

If any basic-block address in the addrs array is zero, then the
corresponding element of the count array is a branch-taken count.

*/
#endif /* _PROF_HEADER_H_ */
