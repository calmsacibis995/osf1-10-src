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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: ch.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/02 18:39:48 $";
#endif
/*
 * HISTORY
 */
/*
 * HISTORY
 * $OSF_Log:	ch.c,v $
 * Revision 1.1.1.1  93/01/07  08:45:00  devrcs
 *  *** OSF1_1_2B07 version ***
 * 
 * Revision 1.1.2.2  1992/08/24  18:15:50  tom
 * 	New more for POSIX.2/XPG4.
 * 	[1992/08/24  17:30:02  tom]
 *
 * $OSF_EndLog$
 */
/*
 * Copyright (c) 1988 Mark Nudleman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)ch.c	5.11 (Berkeley) 6/21/92";
#endif /* not lint */

/*
 * Low level character input from the input file.
 * We use these special purpose routines which optimize moving
 * both forward and backward from the current read pointer.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "less.h"

int file = -1;		/* File descriptor of the input file */

/*
 * The input file is read in blocks of characters.  For single-byte
 * locales, "character" means a byte; for multi-byte locales, it means a
 * wchar_t.
 *
 * The routines in this module are used by the rest of the program to
 * move around the input file, character by character.  Conceptually,
 * positions are in terms of character positions in the file, not byte
 * offsets (although for single-byte locales, the two are the same).
 *
 * The blocks of characters read from the file are kept on an LRU chain.
 * Each block has room for BUFLEN characters.  Given a character position,
 * it's easy to find the block containing the character, as well as the
 * index of the character within the block:
 *
 *     block = pos / BUFLEN;
 *     index = pos % BUFLEN;
 *
 * We may be told to move back to a position whose block has been pushed
 * off the end of the LRU chain.  In that case, we need to reread the
 * block, and to do that, we need the byte offset of the start of the
 * block.  For single-byte locales, that offset is just 'block * BUFLEN'
 * because each character corresponds to one byte in the input file.
 * For multi-byte locales though, the offset can't be calculated as
 * easily because each character may be represented in the file by a
 * sequence of bytes.  So for multi-byte locales, we keep track of our
 * byte offset as we read through the file, and we save it whenever we
 * start a new block.  Those offsets are stored in a list, block_offsets,
 * indexed by block number.
 *
 * We may also by asked the byte offset of any (character) position.
 * For single-byte locales, byte offset and position are the same thing.
 * But for multi-byte locales they aren't.  For multi-byte locales, we
 * need to save the byte offset of each character.  We save the offsets
 * along with the characters in the buf structure.  To save space, the
 * character offsets are stored as deltas from their block's offset.  It
 * all works something like this:
 *
 *     wc = pblock->data.mb[index].wc;
 *
 *     block_offset = block_offsets[block];
 *     delta_offset = pblock->data.mb[index].offset;
 *     offset = block_offset + delta_offset;
 */

#define MB_BUFSIZ	(BUFSIZ / sizeof(wchar_and_offset))
#define BUFLEN		(mbcodeset ? MB_BUFSIZ : BUFSIZ)

typedef struct {
	wchar_t wc;	/* character */
	int offset;	/* add to block's byte offset to get full offset */
} wchar_and_offset;

/*
 * Pool of buffers holding the most recently used blocks of the input file.
 */
struct buf {
	struct buf *next, *prev;
	long block;
	int datasize;
	union {
		unsigned char sb[BUFSIZ*2]; /* double length for escaping */
		wchar_and_offset mb[MB_BUFSIZ*2];
	} data;
};
int nbufs;

/*
 * The buffer pool is kept as a doubly-linked circular list, in order from
 * most- to least-recently used.  The circular list is anchored by buf_anchor.
 */
#define	END_OF_CHAIN	((struct buf *)&buf_anchor)
#define	buf_head	buf_anchor.next
#define	buf_tail	buf_anchor.prev

static struct {
	struct buf *next, *prev;
} buf_anchor = { END_OF_CHAIN, END_OF_CHAIN };

extern int ispipe, cbufs, sigs;
extern int mb_cur_max;
extern int mbcodeset;

/*
 * Current position in file.
 * Stored as a block number and an offset into the block.
 */
static long ch_block;
static int ch_offset;
/*
 * End of input positions
 */
static long eoi_block = -1;
static int eoi_offset = -1;

/* Length of file, needed if input is a pipe. */
static off_t ch_fsize;

/* Number of bytes read, if input is standard input (a pipe). */
static off_t last_piped_pos;

static wint_t	fch_get(void);
static int	read_more_mb(struct buf *, off_t);
static int	read_more_sb(struct buf *, off_t);
static void	init_block_offsets(int);
static off_t	get_block_offset(long);
static void	add_block_offset(long, off_t);
static int	buffered(long);

/*
 * Get the character pointed to by the read pointer from this block.
 */
#define current_character(bp) \
	(mbcodeset ? \
	    (bp)->data.mb[ch_offset].wc: \
	    (bp)->data.sb[ch_offset])

/*
 * Get the character pointed to by the read pointer.  ch_get() is a macro
 * which is more efficient to call than fch_get (the function), in the usual
 * case that the block desired is at the head of the chain.
 */
#define	ch_get() \
	((buf_head->block == ch_block && \
	    ch_offset < buf_head->datasize) ? \
	    current_character(buf_head) : fch_get())

static wint_t
fch_get(void)
{
	register struct buf *bp;
	int n;
	off_t pos;

	/* look for a buffer holding the desired block. */

	for (bp = buf_head;  bp != END_OF_CHAIN;  bp = bp->next)
		if (bp->block == ch_block) {
			if (ch_offset >= bp->datasize)
				/*
				 * Need more data in this buffer.
				 */
				goto read_more;
			/*
			 * On a pipe, we don't sort the buffers LRU
			 * because this can cause gaps in the buffers.
			 * For example, suppose we've got 12 1K buffers,
			 * and a 15K input stream.  If we read the first 12K
			 * sequentially, then jump to line 1, then jump to
			 * the end, the buffers have blocks 0,4,5,6,..,14.
			 * If we then jump to line 1 again and try to
			 * read sequentially, we're out of luck when we
			 * get to block 1 (we'd get the "pipe error" below).
			 * To avoid this, we only sort buffers on a pipe
			 * when we actually READ the data, not when we
			 * find it already buffered.
			 */
			if (ispipe)
				return(current_character(bp));
			goto found;
		}

	/*
	 * Block is not in a buffer.  Take the least recently used buffer
	 * and read the desired block into it.  If the LRU buffer has data
	 * in it, and input is a pipe, then try to allocate a new buffer first.
	 */
	if (ispipe && buf_tail->block != (long)(-1))
		(void)ch_addbuf(1);
	bp = buf_tail;
	bp->block = ch_block;
	bp->datasize = 0;

read_more:
	if ((ch_block == eoi_block) && (ch_offset == eoi_offset))
		return(EOI);

	pos = (ch_block * BUFLEN) + bp->datasize;
	if (ispipe) {
		/*
		 * The data requested should be immediately after
		 * the last data read from the pipe.
		 */
		if (pos != last_piped_pos) {
			error(MSGSTR(PIPE_ERR, "pipe error"));
			quit();
		}
	}

	n = mbcodeset ? read_more_mb(bp, pos) : read_more_sb(bp, pos);
	if (n == READ_INTR)
		return (EOI);
	if (n < 0) {
		error("read error");
		quit();
	}

	bp->datasize += n;
	if (ispipe)
		last_piped_pos += n;

	/*
	 * Remember end-of-input, can't put a marker in our data
	 * because we are using all 8 bits...
	 */
	if (n == 0) {
		eoi_block = ch_block;
		eoi_offset = bp->datasize;
	}

found:
	if (buf_head != bp) {
		/*
		 * Move the buffer to the head of the buffer chain.
		 * This orders the buffer chain, most- to least-recently used.
		 */
		bp->next->prev = bp->prev;
		bp->prev->next = bp->next;

		bp->next = buf_head;
		bp->prev = END_OF_CHAIN;
		buf_head->prev = bp;
		buf_head = bp;
	}

	if (ch_offset >= bp->datasize)
		/*
		 * After all that, we still don't have enough data.
		 * Go back and try again.
		 */
		goto read_more;

	return(current_character(bp));
}

/*
 * When a multi-byte block is first read, its byte offset is saved in
 * this list.  Then if the block is paged out, it can be read back in
 * easily.
 */
static off_t *block_offsets;
static long num_block_offsets;
static long max_block_offsets;

/*
 * Initialize the list of byte offsets.
 */
static void
init_block_offsets(int keep)
{
	if (!keep)
		num_block_offsets = 0;

	if (num_block_offsets == 0)
		add_block_offset(0, 0);
}

/*
 * Get the byte offset of a multi-byte block.
 */
static off_t
get_block_offset(long block)
{
	if ((block < 0) || (block >= num_block_offsets)) {
		/*
		 * XXX This shouldn't happen.  Need an error message.
		 */
		quit();
	}

	return(block_offsets[block]);
}

/*
 * Add the byte offset of a multi-byte block to the list.
 */
static void
add_block_offset(long block, off_t offset)
{
	size_t size;

	if (block >= max_block_offsets) {
		max_block_offsets += 16;
		size = max_block_offsets * sizeof(*block_offsets);
		block_offsets = realloc(block_offsets, size);
		if (block_offsets == NULL) {
			error(MSGSTR(NOMEM, "cannot allocate memory"));
			quit();
		}
	}

	block_offsets[block] = offset;
	num_block_offsets++;
}

/*
 * Read into a multi-byte block.  Return number of characters read. 
 */
static int
read_more_mb(struct buf *bp, off_t pos)
{
	off_t block_offset;
	int delta_offset;
	off_t offset;
	static int n;
	static struct{
		unsigned char *bufptr;
		int bufcnt;
		unsigned char buf[MB_BUFSIZ];
	} rbuf = {rbuf.buf, 0};
	int datasize;
	static int len;
	static unsigned char carry_buf[MB_LEN_MAX];
	static int idx;

	block_offset = get_block_offset(bp->block);
	delta_offset = bp->datasize ? bp->data.mb[bp->datasize].offset : 0;
	offset = block_offset + delta_offset;

	if(!ispipe && delta_offset==0){
		rbuf.bufptr = rbuf.buf;
		rbuf.bufcnt = 0;
		idx = 0;
	}
	/*
	 * Read the block.
	 * The buffer rbuf.buf is statically kept in this routine.
	 * We call iread() only when no more data is available
	 * in the buffer. carry_buf[] is used to carry the fractions
	 * of multibyte characters to the next buffer.
	 * Otherwise mbtowc() will fail to convert.
	 */
	if(rbuf.bufptr >= rbuf.buf && rbuf.bufcnt == 0){
		(void)lseek(file, offset+idx, SEEK_SET);
		if(idx)
			strncpy(rbuf.buf, carry_buf, idx);

		n = iread(file, rbuf.buf+idx, MB_BUFSIZ-idx);
		rbuf.bufcnt = n + idx;
	}

	if ((n < 0) || (n == READ_INTR))
		return(n);
	else {
		n += idx;
		idx = 0;
	}
	if (n == 0) {
		ch_fsize = offset;
		return(0);
	}

	datasize = bp->datasize;

	while ((n > 0) && (datasize < MB_BUFSIZ)) {

		len = mbtowc(&bp->data.mb[datasize].wc, rbuf.bufptr, n);

		if (len < 0) {
			/*
			 * Couldn't get a valid wide character out of the
			 * remaining bytes.  If there were enough bytes
			 * there, we're stuck at an invalid multi-byte
			 * sequence (EILSEQ), so quit.  If there weren't
			 * enough bytes, but we're at the start of the
			 * buffer, the file must end with an invalid
			 * sequence, so also quit.  Otherwise, pass for
			 * now; the next call to us will try again.
			 */
			if((n >= mb_cur_max) || (rbuf.bufptr == rbuf.buf)){
				perror("more");
				quit();
			}
			else {
				for(idx=0; idx < n; idx++)
					carry_buf[idx] = rbuf.bufptr[idx];
				rbuf.bufptr += n;
				break;
			}
		}

		/* Don't throw away "NULL" code for -z option */
		if( ( bp->data.mb[datasize].wc == 0) && (len == 0) ){
			rbuf.bufptr++;
			n--; 
		}
		else{
			rbuf.bufptr += len;
			n -= len;
		}

		/*
		 * XXX Still need to translate \r\n to \n and escape
		 * escape characters, just like in read_more_sb().
		 */
		
		/*
		 * Save the offset of the next character.
		 */

		if(!idx)
			/* Don't throw away "NULL" code for -z option */
			if( ( bp->data.mb[datasize].wc == 0) && (len == 0) )
				bp->data.mb[++datasize].offset = (delta_offset += 1);
			else
				bp->data.mb[++datasize].offset = (delta_offset += len);
	}
	offset = block_offset + delta_offset;	
	if (datasize >= MB_BUFSIZ)
		add_block_offset(bp->block + 1, offset);

	if(rbuf.bufptr >= &rbuf.buf[MB_BUFSIZ]
	   || rbuf.bufcnt < MB_BUFSIZ && n == 0){
		rbuf.bufptr = rbuf.buf;
		rbuf.bufcnt = 0;
	}
	return(datasize - bp->datasize);
}

/*
 * Read into a single-byte block.  Return number of characters read. 
 */
static int
read_more_sb(struct buf *bp, off_t pos)
{
	extern int bs_mode;
	register int n, ch;
	register unsigned char *p, *t;
	unsigned char buf[BUFSIZ];

	(void)lseek(file, pos, SEEK_SET);

	/*
	 * Read the block.
	 * If we read less than a full block, we just return the
	 * partial block and pick up the rest next time.
	 */
	n = iread(file, buf, BUFSIZ - bp->datasize);
	if ((n < 0) || (n == READ_INTR))
		return(n);
	if (n == 0) {
		ch_fsize = pos;
		return(0);
	}

	p = &buf[0];

	/*
	 * Translate \r\n sequences to \n if -u flag not set.
	 * Escape all ESC_CHAR's in the input stream
	 */
	if (!bs_mode) {
		for (t = &bp->data.sb[bp->datasize]; --n >= 0; p++) {
			ch = *p;
			if (ch == '\r' && n && p[1] == '\n') {
				++p;
				*t++ = '\n';
			}
			else if (ch == ESC_CHAR) {
				*t++ = ESC_CHAR;
				*t++ = ch;
			}
			else
				*t++ = ch;
		}
	}
	else {
	/*
	 * just take care of the ESC_CHAR in the input stream
	 */
		for(t = &bp->data.sb[bp->datasize]; --n >= 0; p++) {
			if (*p == ESC_CHAR)
				*t++ = ESC_CHAR;
			*t++ = *p;
		}
		
	}

	n = t - &bp->data.sb[bp->datasize];

	return(n);
}

/*
 * Determine if a specific block is currently in one of the buffers.
 */
static int
buffered(long block)
{
	register struct buf *bp;

	for (bp = buf_head; bp != END_OF_CHAIN; bp = bp->next)
		if (bp->block == block)
			return(1);
	return(0);
}

/*
 * Seek to a specified character position in the file.
 * Return 0 if successful, non-zero if can't seek there.
 */
int
ch_seek(register off_t pos)
{
	long new_block;

	new_block = pos / BUFLEN;
	if (!ispipe || pos == last_piped_pos || buffered(new_block)) {
		/*
		 * Set read pointer.
		 */
		ch_block = new_block;
		ch_offset = pos % BUFLEN;
		return(0);
	}
	return(1);
}

/*
 * Seek to a specified byte offset in the file.
 * Return 0 if successful, non-zero if can't seek there.
 */
int
ch_seek_byte(register off_t offset)
{
	/*
	 * Character positions and byte offsets are identical only in
	 * single-byte locales.
	 */
	return(mbcodeset ? 1 : ch_seek(offset));
}

/*
 * Seek to the end of the file.
 */
int
ch_end_seek(void)
{
	if (!ispipe && !mbcodeset)
		return(ch_seek(ch_length()));

	/*
	 * Do it the slow way: read till end of data.
	 */
	while (ch_forw_get() != EOI)
		if (sigs)
			return(1);
	return(0);
}

/*
 * Seek to the beginning of the file, or as close to it as we can get.
 * We may not be able to seek there if input is a pipe and the
 * beginning of the pipe is no longer buffered.
 */
int
ch_beg_seek(void)
{
	register struct buf *bp, *firstbp;

	/*
	 * Try a plain ch_seek first.
	 */
	if (ch_seek((off_t)0) == 0)
		return(0);

	/*
	 * Can't get to position 0.
	 * Look thru the buffers for the one closest to position 0.
	 */
	firstbp = bp = buf_head;
	if (bp == END_OF_CHAIN)
		return(1);
	while ((bp = bp->next) != END_OF_CHAIN)
		if (bp->block < firstbp->block)
			firstbp = bp;
	ch_block = firstbp->block;
	ch_offset = 0;
	return(0);
}

/*
 * Return the length of the file, if known.
 */
off_t
ch_length(void)
{
	if (ispipe)
		return(ch_fsize);
	return((off_t)(lseek(file, (off_t)0, SEEK_END)));
}

/*
 * Return the current position in the file.
 */
off_t
ch_tell(void)
{
	return(ch_block * BUFLEN + ch_offset);
}

/*
 * Map a position to a byte offset.
 */
off_t
ch_byte(off_t pos)
{
	long block;
	int index;
	int delta;
	struct buf *bp;
	off_t offset;

	if (mbcodeset) {
		block = pos / MB_BUFSIZ;
		index = pos % MB_BUFSIZ;

		if (index == 0) {
			/*
			 * This covers the case where we've just finished
			 * one block, but haven't read the next one yet.
			 */
			delta = 0;
		}
		else {
			/* look for a buffer holding the desired block. */
			for (bp = buf_head; bp != END_OF_CHAIN; bp = bp->next)
				if (bp->block == block)
					break;

			if ((bp == END_OF_CHAIN) || (index > bp->datasize)) {
				/*
				 * XXX This should never happen, right?
				 */
				quit();
			}

			delta = bp->data.mb[index].offset;
		}

		offset = get_block_offset(block) + delta;
	}
	else {
		offset = pos;
	}
	return(offset);
}

/*
 * Get the current char and post-increment the read pointer.
 */
wint_t
ch_forw_get(void)
{
	register wint_t c;

	c = ch_get();
	if (c != EOI && ++ch_offset >= BUFLEN) {
		ch_offset = 0;
		++ch_block;
	}
	return(c);
}

/*
 * Pre-decrement the read pointer and get the new current char.
 */
wint_t
ch_back_get(void)
{
	if (--ch_offset < 0) {
		if (ch_block <= 0 || (ispipe && !buffered(ch_block-1))) {
			ch_offset = 0;
			return(EOI);
		}
		ch_offset = BUFLEN - 1;
		ch_block--;
	}
	return(ch_get());
}

/*
 * Allocate buffers.
 * Caller wants us to have a total of at least want_nbufs buffers.
 * keep==1 means keep the data in the current buffers;
 * otherwise discard the old data.
 */
void
ch_init(int want_nbufs, int keep)
{
	register struct buf *bp;
	char message[80];

	cbufs = nbufs;
	if (nbufs < want_nbufs && ch_addbuf(want_nbufs - nbufs)) {
		/*
		 * Cannot allocate enough buffers.
		 * If we don't have ANY, then quit.
		 * Otherwise, just report the error and return.
		 */
		(void)sprintf(message, MSGSTR(NOBUFF, "cannot allocate %d buffers"),
		    want_nbufs - nbufs);
		error(message);
		if (nbufs == 0)
			quit();
		return;
	}

	if (mbcodeset)
		init_block_offsets(keep);

	if (keep)
		return;

	/*
	 * We don't want to keep the old data,
	 * so initialize all the buffers now.
	 */
	for (bp = buf_head;  bp != END_OF_CHAIN;  bp = bp->next)
		bp->block = (long)(-1);
	last_piped_pos = (off_t)0;
	ch_fsize = NULL_POSITION;
	eoi_block = -1;
	eoi_offset = -1;
	(void)ch_seek((off_t)0);
}

/*
 * Allocate some new buffers.
 * The buffers are added to the tail of the buffer chain.
 */
int
ch_addbuf(int nnew)
{
	register struct buf *bp;
	register struct buf *newbufs;

	/*
	 * We don't have enough buffers.  
	 * Allocate some new ones.
	 */
	newbufs = (struct buf *)calloc((u_int)nnew, sizeof(struct buf));
	if (newbufs == NULL)
		return(1);

	/*
	 * Initialize the new buffers and link them together.
	 * Link them all onto the tail of the buffer list.
	 */
	nbufs += nnew;
	cbufs = nbufs;
	for (bp = &newbufs[0];  bp < &newbufs[nnew];  bp++) {
		bp->next = bp + 1;
		bp->prev = bp - 1;
		bp->block = (long)(-1);
	}
	newbufs[nnew-1].next = END_OF_CHAIN;
	newbufs[0].prev = buf_tail;
	buf_tail->next = &newbufs[0];
	buf_tail = &newbufs[nnew-1];
	return(0);
}
