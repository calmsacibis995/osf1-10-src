
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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: buf.c,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/07/23 14:28:50 $";
#endif
#include <stdio.h>
#define _KERNEL
#include <sys/buf.h>
#include "krash.h"

short *freetab, *hashtab;
char *freestr[] = { "   ", "LCK", "LRU", "AGE", "EMP" };
struct buf *bufaddr;		/* Start of buffer vector */
long nbuf;			/* Number of bufs */
struct bufhd *bufhash;		/* Start of bufhd vector */
int bufhsz;			/* Number of bufhds */

static char line[256], *error;

static char *help_string =
"buf - print out the buf table                                           \\\n\
    Usage : buf [ -free | -all | -stray | addresses ]                     \\\n\
 [no option] -- all buf structures in the hash lists are printed \\\n\
 -free       -- all buf structures in the free lists are printed \\\n\
 -all        -- all buf structures are printed, first by free lists, \\\n\
                then by hash table, then any strays.             \\\n\
 addresses   -- buf structures at these given addresses are printed \\\n\
";
    

FieldRec fields[] = {
  { ".av_forw", NUMBER, NULL, NULL },	/* 0 */
  { ".b_dev", NUMBER, NULL, NULL },	/* 1 */
  { ".b_blkno", NUMBER, NULL, NULL },	/* 2 */
  { ".b_bcount", NUMBER, NULL, NULL },	/* 3 */
  { ".b_bufsize", NUMBER, NULL, NULL },	/* 4 */
  { ".b_resid", NUMBER, NULL, NULL },	/* 5 */
  { ".b_vp", NUMBER, NULL, NULL },	/* 6 */
  { ".b_forw", NUMBER, NULL, NULL },	/* 7 */
  { ".av_back", NUMBER, NULL, NULL },	/* 8 */
  { ".b_flags", NUMBER, NULL, NULL },	/* 9 */
  { ".b_back", NUMBER, NULL, NULL }	/* 10 */
};

#define NUM_FIELDS (sizeof(fields)/sizeof(fields[0]))

FieldRec bufhd_fields[] = {
  { ".b_forw", NUMBER, NULL, NULL },
  { ".b_back", NUMBER, NULL, NULL },
  { ".b_flags", NUMBER, NULL, NULL }
};

static Boolean prbuf(long addr, long *next, struct buf **obp, int free_flag, char *prefix)
{
  char line[256], *error, address[12], vnode[12], forward[12], back[12];
  DataStruct buf;
  int b_flags;
  struct buf *backbuf;

  if(!cast(addr, "struct buf", &buf, &error)){
    fprintf(stderr, "Couldn't cast nextaddr to a buf:\n");
    fprintf(stderr, "%s\n", error);
    return(False);
  }    
  if(!read_field_vals(buf, fields, NUM_FIELDS)){
    field_errors(fields, NUM_FIELDS);
    return(False);
  }
  b_flags = (int) fields[9].data;
  format_addr(addr, address);
  format_addr((long) fields[6].data, vnode);
  format_addr((long) ((free_flag)? fields[7].data: fields[0].data), forward);
  format_addr((long) ((free_flag)? fields[10].data: fields[8].data), back);
  sprintf(line, "%s %s %3d %5d %7ld %5u %5u %5u %s %s %s"
	  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", address, prefix,
	  major((int) (fields[1].data)), minor((int) (fields[1].data)),
	  fields[2].data, fields[3].data, fields[4].data, fields[5].data,
	  vnode, forward, back,
	  b_flags & B_READ ? " read" : " write",
	  b_flags & B_ERROR ? " err" : "",
	  b_flags & B_BUSY ? " busy" : "", 
	  b_flags & B_PHYS ? " phys" : "",
	  b_flags & B_WANTED ? " wnt" : "",
	  b_flags & B_AGE ? " age" : "",
	  b_flags & B_ASYNC ? " async" : "",
	  b_flags & B_DELWRI ? " del" : "",
	  b_flags & B_TAPE ? " tape" : "",
	  b_flags & B_UBC ? " ubc" : "",
	  b_flags & B_DIRTY ? " dirty" : "",
	  b_flags & B_CACHE ? " cache" : "",
	  b_flags & B_INVAL ? " inval" : "",
	  b_flags & B_LOCKED ? " lck" : "",
	  b_flags & B_HEAD ? " head" : "",
	  b_flags & B_BAD ? " bad" : "",
	  b_flags & B_NOCACHE ? " nocache" : "");
  print(line);
  if (obp) {
    backbuf = (struct buf *) ((free_flag)?
			      fields[8].data:
			      fields[10].data);
    if (*obp && *obp != backbuf) {
      sprintf(line, "*** %s: back link is 0x%p, expected 0x%p", address, backbuf, *obp);
      print(line);
    }
    *obp = (struct buf *) addr;
  }
  if(next)
    *next = (long) ((free_flag)?
		    fields[0].data:
		    fields[7].data);
  return(True);
}

main(int argc, char **argv)
{
  int i;
  struct buf *bp;
  char line[256], *error, *ptr;
  Boolean allflag=False, freeflag=False, hashflag=False;

  check_args(argc, argv, help_string);
  if(argc == 1) hashflag = True;
  else for (i = 1; i < argc; i++)
      if (!strcmp(argv[i], "-free")) freeflag=1;
      else if (!strcmp(argv[i], "-all"))  allflag=1;

  if(!check_fields("struct buf", fields, NUM_FIELDS, NULL)){
    field_errors(fields, NUM_FIELDS);
    quit(1);
  }
  if(!read_sym_val("buf", NUMBER, (long *) &bufaddr, &error)){
    fprintf(stderr, "Couldn't read buf:\n");
    fprintf(stderr, "%s\n", error);
    quit(1);
  }
  print("BUF         BQ_ MAJ   MIN   BLOCK COUNT  SIZE RESID VNO         FWD         BACK        FLAGS ");
  print("=========== === === =====  ====== ===== ===== ===== =========== =========== =========== ======");

  if(allflag || hashflag || freeflag){
    if(!read_sym_val("nbuf", NUMBER, (long *) &nbuf, &error)){
      fprintf(stderr, "Couldn't read nbuf:\n%s\n", error);
      quit(1);
    }
    freetab = (short *) calloc(nbuf, sizeof(freetab[0]));
    hashtab = (short *) calloc(nbuf, sizeof(freetab[0]));
    if (!freetab || !hashtab) {
      fprintf(stderr, "Couldn't malloc %d shorts\n", nbuf);
      quit(1);
    }
    if (freeflag || allflag) {
      print_free();
      print("");
    }
    if (hashflag || allflag) {
      print_hash();
      print("");
    }
    if (allflag) {
      print_stray();
    }
  }
  else {
    argv++;
    while(*argv){
      bp = (struct buf *) strtoul(*argv, &ptr, 0);
      if(*ptr != '\0'){
	fprintf(stderr, "Couldn't parse %s to a number\n", *argv);
	quit(1);
      }
      if ((unsigned long) bp < 10000)
	bp = &bufaddr[(unsigned long) bp];
      if(!prbuf((long)bp, NULL, NULL, TRUE, "   ")) quit(1);
      argv++;
    }
  }
  quit(0);
}

/*
 * Routine to print all bufs on the four free lists.  At this writing, the
 * last one, BQ_EMPTY is never used (or perhaps rarely used).  The free list
 * each buf is on is saved in freetab, which is also used in print_hash() and
 * print_stray().
 */
print_free() {
  DataStruct bufdat, bfreedat;
  int i, bnum;
  struct buf *bp;		/* Walks down each list */
  struct buf *bfreep;		/* Steps through each free list head */
  struct buf *obp;		/* Remembers previous bp */
  struct buf *headbbp;		/* Remembers back pointer of head */

  print("Bufs by freelists (locked, LRU, age, empty):");
  if(!read_sym_addr("bfreelist", (long *) &bfreep, &error)){
    fprintf(stderr, "Couldn't read bfreelist:\n%s\n", error);
    quit(1);
  }
  for (i = 0; i < BQUEUES; i++, bfreep++) {
    obp = NULL;
    if (!prbuf((long)bfreep, (long *) &bp, &obp, TRUE, freestr[i + 1]))
      quit(1);
    headbbp = (struct buf *) fields[8].data;
    while (bp != bfreep) {
      bnum = bp - bufaddr;
      if (bnum >= nbuf) {
	fprintf(stderr, "buf addr %p is outside buf area\n", bp);
	break;
      }
      if (freetab[bnum]) {
	sprintf(line, "Dup ref for buf 0x%p, already seen on freelist %s",
		bp, freestr[freetab[bnum]]);
	print(line);
	break;
      }
      freetab[bnum] = i + 1;
      if (!prbuf((long)bp, (long *) &bp, &obp, TRUE, freestr[i + 1])) quit(1);
    }
    if (bp == bfreep && headbbp != obp) {
      sprintf(line, "*** List head 0x%p: back link is 0x%p, expected 0x%p",
	      bfreep, headbbp, obp);
      print(line);
    }
  }
}

/*
 * Routine to print all buf hash lists.  At this writing there were
 * 512 lists for only 360 bufs, so don't bother listing the bufhd index.
 * Call this after print_free() so this can print the free list the
 * buffer is on.  hashtab[] is used to track the buffers we've visited
 * so we can't go into a loop.  The data is only used to find stray
 * buffers.
 */
print_hash() {
  DataStruct bufdat, bufhddat;
  int i, bnum;
  struct buf *bp;		/* Walks down each list */
  struct buf *obp;		/* Remembers previous bp */
  struct buf *headbbp;		/* Remembers back pointer of head */

  print("Bufs on hash lists:");
  if(!read_sym_val("bufhsz", NUMBER, (long *) &bufhsz, &error)){
    fprintf(stderr, "Couldn't read bufhsz:\n%s\n", error);
    quit(1);
  }
  if(!read_sym_val("bufhash", NUMBER, (long *) &bufhash, &error)){
    fprintf(stderr, "Couldn't read bufhash:\n%s\n", error);
    quit(1);
  }
  for (i = 0; i < bufhsz; i++, bufhash++) {
    if(!cast((long) bufhash, "struct bufhd", &bufhddat, &error)){
      fprintf(stderr, "Couldn't cast bufhash to a bufhd:\n");
      fprintf(stderr, "%s\n", error);
      return(False);
    }    
    if(!read_field_vals(bufhddat, bufhd_fields, 3)){
      field_errors(fields, 3);
      return(False);
    }
    bp = (struct buf *) bufhd_fields[0].data;
    headbbp = (struct buf *) bufhd_fields[1].data;
    for (obp = (struct buf *)bufhash; bp != (struct buf *) bufhash; ) {
      bnum = bp - bufaddr;
      if (bnum >= nbuf) {
	fprintf(stderr, "buf addr %p is outside buf area\n", bp);
	break;
      }
      if (hashtab[bnum]) {
	sprintf(line, "Dup ref for buf 0x%p, already seen on bufhash[%d]",
		bp, hashtab[bnum] - 1);
	print(line);
	break;
      }
      hashtab[bnum] = i + 1;
      if(!prbuf((long)bp, (long *) &bp, &obp, FALSE, freestr[freetab[bnum]]))
	quit(1);
    }
    if (bp == (struct buf *) bufhash && headbbp != obp) {
      sprintf(line, "*** List head 0x%p: back link is 0x%p, expected 0x%p",
	      bufhash, headbbp, obp);
      print(line);
    }
  }
}

/*
 * Routine to print any buffers missed by print_free() or print_hash().
 */
print_stray() {
  int i, headflag;

  for (i = 0, headflag = FALSE; i < nbuf; i++)
    if (!hashtab[i] && !freetab[i]) {
      if (!headflag) {
	headflag = TRUE;
	print("Stray buffers not yet printed:");
      }
      prbuf((long)(bufaddr + i), NULL, NULL, TRUE, "   ");
    }
  if (!headflag)
    print("All buffers accounted for");
}
