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
#endif

#include	<stdio.h>
#include	<string.h>
#include	"config.h"
#include	<../../sys/conf.h>

/* src/kernel/src/config is the starting point for the following
 * relative pathname
 */
#include	"../../../usr/include/AFdefs.h"
#include	<assert.h>
#include	"mklocalconf.h"

/* input and out file pointers for reading the conf.c files */
FILE	*ifp, *ofp;

/* used as a temporary buffer */
char	tbuf[LINE_SIZE];

/*
 * declaring this here relieves the need to declare char * at every
 * invokation of strtok()
 */
char	*strtok();

/* obuf is first used as the buffer of lines of text as they were read
 * from the template conf.c file.  Then it is used as a place to build new
 * lines for new [bc]devsw table entries for the devices described in the
 * stanza.static file.
 * oi always points to the next line to be read filled in this buffer.
 * note:  have obuf be a statically declared buffer limits the size of the
 *	  conf.c file which can be parsed.  If there is too much code between
 *	  the two devsw[] tables in the io/common/conf.c file then the assert
 *	  in get_char_buf() will fail.  Since it is to late in the development
 *	  cycle of Silver, we will live with this restriction.
 */
char	obuf[BUF_LINES][LINE_SIZE];
int	oi = 0;

/* since strtok is used to examine lines from conf.c, and strtok
 * inserts null ('\0') characters into the string, token_buf is used to
 * hold copies of lines from obuf for strtok to work on.
 */
char	token_buf[TOKEN_LINES][LINE_SIZE];
char	*token[TOKENS];

/* head of the linked lists of Entry's.  One for the bdevsw table and one 
 * for the cdevsw table.
 */
Entry	*bdev_head;
Entry	*cdev_head;

/* head of linked list of new device information.  This is all the
 * information which will be required to generate the bassign_table for the
 * conf.c file
 */
New_devsw	*new_devsw_head;

/* number of entries in the bdevsw and cdevsw tables.  */
int	bdev_size;
int	cdev_size;

/* HACK ALERT!!!!!
 * NCisshift is in /lib/libc.a on an OSF system, but not on an ULTRIX
 * system.  So, in order for config to be able to link on an ULTRIX
 * system, this routine needs to be defined here.
 * Someday, when all of DECs builds are done on OSF machines, this
 * routine definition may be deleted.
 */
NCisshift() { return (0); }

/*
 * routine name:	get_char_buf()
 * description:		return a pointer to a character buffer.
 *			insure that there is another buffer available.
 * input:		none
 * output:		char *
 * assumptions:
 * notes:		in retrospect, I wish I had used malloc here.
 *			but some of the code which uses obuf depends on
 *			the fact that the lines are stored in order in
 *			obuf.  Someday I would like to "fix" this.
 */
char *
get_char_buf()
{
	assert (oi <= BUF_LINES);
	return (obuf[oi++]); 
}

/*
 * routine name:	get_char_buf2()
 * description:		return a pointer to a character buffer.
 * input:		none
 * output:		char *
 * assumptions:
 * notes:		This is a partial answer to my desire to have
 *			used malloc in get_char_buf().  Since
 *			get_char_buf() is called as the stanza file is
 *			parsed, and as new devsw table entries are
 *			built, it would have taken very few drivers to
 *			have used up all available space.  So, by only
 *			using obuf[] for the parsing of conf.c, and use
 *			malloc for everything else, there should always
 *			be enough buffer space.
 */
char *
get_char_buf2()
{
char	*ret;
	ret = (char *) malloc (LINE_SIZE);
	assert (ret);
	return (ret);
}

/*
 * routine name:	get_next_entry()
 * description:		return a pointer to a structure of type Entry
 * input:		none
 * output:		Entry *
 * assumptions:
 * notes:
 */
Entry	*
get_next_entry()
{
Entry	*ret;

	ret = (Entry *) malloc (sizeof(Entry));
	assert (ret);
	return (ret);
}

/*
 * routine name:	find_make_specific_empty()
 * description:		search for an Entry in the appropriate devsw
 *			table.  The search is based on the major
 *			number.  If the Entry exists and is free,
 *			then return a pointer to it.  If the Entry
 *			does not exist, then create it.
 * input:		search_flag
 *				BDEV_FLAG - work on the bdevsw table
 *				CDEV_FLAG - work on the cdevsw table
 *			num
 *				major number to be searched for
 * output:		Entry *
 *				NULL - matching Entry was found, but
 *				       it was not FREE, it was TAKEN
 *				non-NULL
 *				     - pointer to the matching Entry
 *				       which was found or created
 * assumptions:
 * notes:		- creation of a new Entry may require creating filler
 *			  Entry's.
 *			- future work:  this routine could call
 *			  add_empty_devsw().  It does not now because
 *			  it would add_empty_devsw() does not currently
 *			  return the pointer to the new Entry.  This
 *			  would be a simple enough change, but I don't
 *			  have the time right now.
 *			- future work:  call find_specific_devsw()
 */
Entry *
find_make_specific_empty (search_flag, num)
int	search_flag;
int	num;
{
Entry	*entry;		/* for the first loop, this is the Entry being
			 * examined.
			 * for the second loop, this is the FOOTER Entry.
			 */
Entry	*new_entry;
Entry	*prev_entry;
int	new_num;
int	continue_search;

	/* get the pointer to the head of the appropriate linked list */
	entry = ((search_flag == BDEV_FLAG) ? bdev_head : cdev_head);

	/* some sanity checks */
	assert ((search_flag == BDEV_FLAG) || (search_flag == CDEV_FLAG));
	assert (entry != NULL);
	assert (entry->flag == HEADER);
	assert (entry->nxt != NULL);

	/* search through the linked list looking for an entry with a
	 * matching major number.  At the end of this loop, entry will
	 * either point to an entry with the desired major number, or
	 * it will point to the end of the linked list.
	 * FUTURE:  make use of find_specific_devsw().  This would
	 * require find_specific_devsw() to also return the pointer to
	 * the previous entry.
	 */
	prev_entry = entry;
	entry = entry->nxt;
	continue_search = TRUE;
	while (continue_search) {
		if (entry->flag == FOOTER) {
			continue_search = FALSE;
		} else {
			if (entry->emajor == num) {
				continue_search = FALSE;
			} else {
				prev_entry = entry; 
				entry = entry->nxt;
			}
		}
	}

	/* if entry points to the end of the list, then the list did
	 * not contain the desired major number.  In this case, a new
	 * Entry needs to be added to the end of the list until an
	 * Entry with a matching major number is added.
	*/
	if (entry->flag == FOOTER) {
		/* the Entry's in the list have consecutive major numbers */
		new_num = prev_entry->emajor;

		/* loop, adding new Entry's and filling each new Entry with
		 * the default values.
		 */
		while (new_num < num) {
			new_num++;
			new_entry = get_next_entry();
			fill_default (search_flag, new_entry, new_num);
			new_entry->flag = FREE;
			new_entry->emajor = new_num;
			new_entry->nxt = entry;
			prev_entry->nxt = new_entry;
			prev_entry = new_entry;
		}
		/* entry still points to the FOOTER.  the major element
		 * of this entry is used to record the next available
		 * major number
		 */
		entry->emajor = num + 1;

		/* since entry is used for the return value to the caller,
		 * make it point to the new entry.
		 */
		entry = new_entry;

		/* bookkeeping:  recalculate the size of this devsw table
		 */
		switch (search_flag) {
	    	    case BDEV_FLAG:
			bdev_size = num + 1;
			break;
	    	    case CDEV_FLAG:
			cdev_size = num + 1;
			break;
		}
	}

	/* test to see if a FREE entry was found */
	if (entry->flag == FREE) {
		return (entry);
	} else {
		return (NULL);
	}
}

/*
 * routine name:	add_empty_devsw()
 * description:		add an empty devsw Entry to the end of the linked
 *			list.  fill this Entry with the default values.
 * input:		flag - BDEV_FLAG or CDEV_FLAG
 *				BDEV_FLAG - work on the bdevsw table
 *				CDEV_FLAG - work on the cdevsw table
 * output:		major number of the new Entry.
 * assumptions:
 * notes:		future work:  modify to return a pointer to
 *			Entry.  Since the Entry contains the major
 *			number, then this purpose would be accomplished
 *			too.
 */
int
add_empty_devsw (flag)
int	flag;
{
Entry	*e,*el,*en;
int	mj;

	/* get the pointer to the appropriate linked list */
	e = ((flag == BDEV_FLAG) ? bdev_head : cdev_head);

	/* sanity checks */
	assert ((flag == BDEV_FLAG) || (flag == CDEV_FLAG));
	assert (e);

	/* find end of list */
	while (e->flag != FOOTER) {
		el = e;
		e = e->nxt;
	}

	/* create the new entry and fill it in with the default values */
	mj = e->emajor;
	en = get_next_entry();
	fill_default (flag, en, mj);
	en->flag = FREE;
	en->emajor = mj;
	en->nxt = e;
	el->nxt = en;
	e->emajor = mj+1;

	/* calculate the new size fo the devsw table */
	switch (flag) {
	    case BDEV_FLAG:
		bdev_size = mj+1;
		break;
	    case CDEV_FLAG:
		cdev_size = mj+1;
		break;
	}

	return (mj);
}

/*
 * routine name:	get_next_line()
 * description:		return a pointer to a structure of type Line
 * input:		none
 * output:		Line *
 * assumptions:
 * notes:
 */
Line	*
get_next_line()
{
Line	*ret;

	ret = (Line *) malloc (sizeof (Line));
	assert (ret);
	return (ret);
}

/*
 * routine name:	replace_devsw()
 * description:		replace the current lines of this Entry with
 *			the new values in the New_devsw structure.
 *			Copy the major number from the Entry to the
 *			New_devsw and mark the Entry as TAKEN.
 * input:		flag - BDEV_FLAG or CDEV_FLAG
 *			d    - pointer to the New_devsw structure which
 *			       contains the information necessary to
 *			       generate the new devsw table entry.
 *			e    - pointer to the Entry which contains the
 *			       empty devsw table lines.  this Entry
 *			       also contains the major number which
 *			       needs to be copied into the New_devsw.
 * output:		none
 * assumptions:         - there are at least two text lines used for each entry
 *			  in the devsw table - thus the assertion that e->lines
 *			  and e->lines->nxt are both non-NULL.  The code may or
 *			  not fail if this assumption is not valid, but entries
 *			  of less than 2 lines probably indicate a botched
 *			  io/common/conf.c file.
 * notes:		future work:  since config_name is a required
 *			field, there is always some information on
 *			which to base a comment.
 */
replace_devsw (flag, d, e)
int		flag;
New_devsw	*d;
Entry		*e;
{
Line	*l;
char	comment[LINE_SIZE];

	/* get pointer to linked list of Line's.  The buf entry of
	 * these Line's are the ascii strings which are the devsw table
	 * entry for the conf.c file.
	 */
	l = e->lines;

	/* some sanity checks. */
	assert ((flag == BDEV_FLAG) || (flag == CDEV_FLAG));
	assert (l != NULL);
	assert (l->nxt != NULL);

	/* first line is actually white space between devsw table entries */
	sprintf (l->buf, dev_line1);
	l = l->nxt;

	/* use the text for the stanza entries config_name and
	 * description field as the comment for this devsw table
	 * entry.  If neither of these are present, then use a default
	 * value.  
	 * FUTURE:  Since the config_name is a required field, this is
	 * not necessary.  This should be cleaned up.
	 */
	if ((d->config_name) || (d->description)) {
		if (d->description) {
			sprintf (comment, "%s", d->description);
		} else {
			sprintf (comment, "");
		}
		if (d->config_name) {
			sprintf (comment, "%s (%s)", comment, d->config_name);
		}
		sprintf (l->buf, dev_line2, comment);
	} else {
		sprintf (l->buf, dev_line2, empty_comment);
	}

	/* the next lines are the remainder of the devsw entry.  these
	 * differ between the bdevsw table and the cdevsw table.  the current
	 * lines in the devsw entry will be over written.  If not enough lines
	 * already exist, then get new one(s).
	 * Also, since the New_devsw structure is used to generate the
	 * Bassign_table for the conf.c file, it is necessary to record
	 * the major number in the New_devsw.  This is a convenient
	 * place to do that.
	 */
	if (l->nxt == NULL) {
		l->nxt = get_next_line();
		l->nxt->buf = get_char_buf2 (LINE_SIZE);
	}
	l = l->nxt;
	assert (l != NULL);
	switch (flag) {
	    case BDEV_FLAG:
		sprintf (l->buf, bdev_line3, d->bdevsw[0], d->bdevsw[1], 
		    d->bdevsw[2], d->bdevsw[3], e->emajor);
		if (l->nxt) {
			l = l->nxt;
		} else {
			l->nxt = get_next_line();
			l = l->nxt;
			l->buf = get_char_buf2 (LINE_SIZE);
		}
		sprintf (l->buf, bdev_line4, d->bdevsw[4], d->bdevsw[5], 
		    d->bdevsw[6], d->bdevsw[7]);
		d->bmajor_assign = e->emajor;
		break;
	    case CDEV_FLAG:
		sprintf (l->buf, cdev_line3, d->cdevsw[0], d->cdevsw[1], 
		    d->cdevsw[2], d->cdevsw[3], e->emajor);
		if (l->nxt == NULL) {
			l->nxt = get_next_line();
			l->nxt->buf = get_char_buf2 (LINE_SIZE);
		}
		l = l->nxt;
		assert (l != NULL);
		sprintf (l->buf, cdev_line4, d->cdevsw[4], d->cdevsw[5], 
		    d->cdevsw[6], d->cdevsw[7]);
		if (l->nxt) {
			l = l->nxt;
		} else {
			l->nxt = get_next_line();
			l = l->nxt;
			l->buf = get_char_buf2 (LINE_SIZE);
		}

		/* The DDI/DKI kernel hook adds two extra fields to the
		   cdevsw structure.  See the declaration of cdevsw
		   in src/kernel/sys/conf.h for details.
		*/
		sprintf (l->buf, cdev_line5, d->cdevsw[8], d->cdevsw[9],
		    d->cdevsw[10], d->cdevsw[11], d->cdevsw[12]);

		d->cmajor_assign = e->emajor;
		break;
	}

	l->nxt = NULL;

	/* mark this Entry as TAKEN */
	e->flag = TAKEN;
}

/*
 * routine name:	find_specific_devsw()
 * description:		find Entry with matching major number.
 * input:		flag - BDEV_FLAG or CDEV_FLAG
 *			n    - major number to search for
 * output:		ptr to Entry with matching major number
 * assumptions:		major number n already exists in the devsw table.
 *			This was assured my a previous call to one of the 
 *			find_make...() routines.
 * notes:
 */
Entry *
find_specific_devsw (flag, n)
int	flag;
int	n;
{
Entry *e;

	/* ptr to head of appropriate linked list of Entry */
	e = ((flag == BDEV_FLAG) ? bdev_head : cdev_head);

	/* some sanity checks */
	assert ((flag == BDEV_FLAG) || (flag == CDEV_FLAG));
	assert (e->flag == HEADER);
	assert (e->nxt != NULL);

	/* loop until find major number */
	e = e->nxt;
	while (e->emajor != n) {
		e = e->nxt;
		assert (e != NULL);
	}

	/* e points to Entry with matching major number */
	return (e);
}

/*
 * routine name:	find_next_empty_major()
 * description:		find an Entry which is not TAKEN.  This may
 *			either be a FREE Entry or it may be the end of
 *			the linked list of Entry.
 * input:		flag - BDEV_FLAG or CDEV_FLAG
 *			num  - major number to search for
 * output:		major number of the next empty or to-be-created
 *			Entry
 * assumptions:
 * notes:		The major variable of the FOOTER Entry contains
 *			the major number of the next available major
 *			number if the devsw table needs to grow.
 */
find_next_empty_major (flag, num)
int	flag;
int	num;
{
int	n;
Entry	*e;

	/* get pointer to next Entry */
	e = find_specific_devsw (flag, num+1);

	/* loop until a FREE Entry is found, or the end of the list is
	 * reached
	 */
	while ((e->flag != FOOTER) && (e->flag != FREE)) {
		e=e->nxt;
	}

	/* return the major number of the next empty Entry */
	return (e->emajor);
}

/*
 * routine name:	find_make_empty_match()
 * description:		find empty Entry's in the bdevsw and cdevsw
 *			linked list of Entry's.  If a specific major
 *			number is requested then you only get one
 *			chance.  If any major number will due, then it
 *			may be necessary to increase the size of one or
 *			both lists.
 * input:		*bentry - pointer to a pointer
 *			*centry - pointer to a pointer
 * output:		*bentry - ultimately points to the Entry of the
 *			          bdevsw linked list entry of the
 *			          matched pair of Entry's
 *			*centry - ultimately points to the Entry of the
 *			          cdevsw linked list entry of the
 *			          matched pair of Entry's
 *			num     - major number to be searched for
 * assumptions:
 * notes:		Since the desire here was to return two Entry
 *			pointers to the caller, this routine uses
 *			**Entry.  This allows two values to be returned
 *			without having to create a special structure.
 *			This is a bit convoluted, but it works.  The
 *			calling sequence would then be:
 *				find_make_empty_match (&b, &c, n);
 *					Entry	*b;
 *					Entry	*c;
 *					int	n;
 */
find_make_empty_match (bentry, centry, num)
Entry	**bentry;
Entry	**centry;
int	num;
{
int	bnum, cnum;

	/* check to see if a specific major number is required
	 */
	if (num >=0) {
	    /* get pointers to the Entry with major number 'num' from
	     * both the bdevsw Entry list and the cdevsw Entry list.
	     * If a NULL pointer is returned from either of these calls
	     * than the specified Entry was TAKEN.
	     */
	    *bentry = find_make_specific_empty (BDEV_FLAG, num);
	    *centry = find_make_specific_empty (CDEV_FLAG, num);

	    /* if either Entry was TAKEN, then invalidate both pointers
	     * before return to caller
	     */
	    if (! (*bentry && *centry)) {
		*bentry = *centry = NULL;
	    }

	} else {
	    /* get pointers to the first FREE entry in each Entry list.
	     */
	    bnum = cnum = -1;
	    bnum = find_next_empty_major (BDEV_FLAG, bnum);
	    cnum = find_next_empty_major (CDEV_FLAG, cnum);

	    /* loop looking for matching Entry's.  exit the loop if
	     * matching entries are found, or if the end of either
	     * linked list is reached.
	     */
	    while ((bnum != cnum) && (bnum != bdev_size)
				  && (cnum != cdev_size)) {
		if (bnum < cnum) {
		    bnum = find_next_empty_major (BDEV_FLAG, bnum);
		} /* else (cnum < bnum) */ {
		    cnum = find_next_empty_major (CDEV_FLAG, cnum);
		}
	    }

	    /* at this point, either bnum == cnum, or the end of one of
	     * the linked lists was reached.  if either or both ends
	     * were reached, then force the two numbers to match.
	     */
	    while (bnum > cnum) {
		cnum = add_empty_devsw (CDEV_FLAG);
	    }
	    while (cnum > bnum) {
		bnum = add_empty_devsw (BDEV_FLAG);
	    }

	    /* sanity check */
	    assert (bnum == cnum);

	    /* fill in the pointers for return to caller */
	    *bentry = find_make_specific_empty (BDEV_FLAG, bnum);
	    *centry = find_make_specific_empty (CDEV_FLAG, cnum);
	}
}

/*
 * routine name:	find_make_random_empty()
 * description:		find the next FREE Entry in the 
 * input:		flag - BDEV_FLAG or CDEV_FLAG
 *				BDEV_FLAG - work on the bdevsw table
 *				CDEV_FLAG - work on the cdevsw table
 * output:		pointer to a FREE entry in one of the devsw linked
 *			list of Entry's.
 * assumptions:
 * notes:		future work:  add_empty_devsw should be changed
 *			to return the pointer to the Entry, which would
 *			contain the major number.  This would alleviate
 *			the need to call find_make_specific_empty().
 */
Entry *
find_make_random_empty (flag)
int	flag;
{
int	num;
Entry *entry;
int	size;

	assert ((flag == BDEV_FLAG) || (flag == CDEV_FLAG));

	/* get size of appropriate devsw linked list of Entry's */
	size = ((flag == BDEV_FLAG) ? bdev_size : cdev_size);

	/* find the major number of the first FREE entry in the list */
	num = find_next_empty_major (flag, -1);

	/* if the major number returned matches the size fo the table,
	 * then create a FREE entry at the end of the list
	 */
	if (num == size) {
		num = add_empty_devsw (flag);
	}
	entry = find_make_specific_empty (flag, num);
	assert (entry != NULL);

	return (entry);
}

/*
 * routine name:	print_devsw()
 * description:		print the devsw table to the new conf.c with the
 *			additional Entry's generated by mklocalconf()
 * input:		e - pointer to the head of one of the linked list of
 *			    Entry's
 *			flag - 
 *				BDEV_FLAG - print the bdevsw table
 *				CDEV_FLAG - print the cdevsw table
 * output:
 * assumptions:
 * notes:
 */
print_devsw (e, flag)
Entry	*e;
int	flag;
{
Line		*line;
New_devsw	*bs;
int		i;

	/* sanity check */
	assert (e);

	/* linked list of lines which make up the HEADER */
	line = e->lines;

	/* more sanity checks */
	assert (e->flag == HEADER);
	assert (line);

	/* loop to print the HEADER.  This is a speicial case because
	 * the new routines refered to within the table need to be
	 * declared before the table is started.  Since the beginning
	 * comment of the cookie has alread been printed to ofp, we
	 * need to find the place between the comment and the beginning
	 * of the table.  Yes, this is awkward, but such is life!
	 * the names of the routines are found in the New_devsw linked
	 * list.  Only print out the ones which are specified in the
	 * stanza file and that require definition according to the
	 * stanza_xdevsw_defs table.
	 */
	while (line) {
	    fprintf (ofp, "%s", line->buf);
	    if (strncmp (line->buf, "*/", 2) == 0) {
	    	switch (flag) {
	    	  case BDEV_FLAG:
	    	    for (bs=new_devsw_head; bs; bs=bs->nxt) {
	    	    	if (bs->flag & BDEV_ONLY) {
	    	    	    for (i=0; i<8; i++) {
	    	    	    	if ((strcmp (bs->bdevsw[i], default_bdevsw[i]))
				    && stanza_bdevsw_defs[i]
				    && (strcmp (bs->bdevsw[i], "0"))
				    && (strcmp (bs->bdevsw[i], "NULL"))) {
	    	    	    	    fprintf (ofp, "int\t%s();\n", 
				      bs->bdevsw[i]);
	    	    	    	}
	    	    	    }
	    	    	}
	    	    }
	    	    break;
	    	  case CDEV_FLAG:
	    	    for (bs=new_devsw_head; bs; bs=bs->nxt) {
	    	    	if (bs->flag & CDEV_ONLY) {
	    	    	    for (i=0; i<CMAX; i++)
			    {
	    	    	    	if ((strcmp (bs->cdevsw[i], default_cdevsw[i]))
				    && stanza_cdevsw_defs[i]
				    && (strcmp (bs->cdevsw[i], "0"))
				    && (strcmp (bs->cdevsw[i], "NULL"))) {
	    	    	    	    fprintf(ofp, "int\t%s();\n", bs->cdevsw[i]);
			    }
	    	    	    }
	    	    	}
	    	    }
	    	    break;
	    	}
	    }
	    line = line->nxt;
	}

	/* the rest of the lines may be blindly written out */
	e = e->nxt;
	while (e) {
		line = e->lines;
		while (line) {
			fprintf (ofp, "%s", line->buf);
			line = line->nxt;
		}
		e = e->nxt;
	}
}

/*
 * routine name:	assign_specific()
 * description:		traverse the New_devsw linked list looking for
 *			device pairs which require matching major
 *			numbers in the bdevsw and cdevsw table or
 *			New_devsw's which reqire a specific major
 *			number.  Then locate or create the matching
 *			Entry's filling them in with the info found in
 *			the New_devsw.
 * input:		none
 * output:		none
 * assumptions:
 * notes:
 */
assign_specific ()
{
New_devsw	*bs;
Entry	*bentry, *centry;
		/* Entry pointers used to search for or traverse
		 * Entry's in the devsw linked list of Entry's
		 */

    bentry = centry = NULL;
    /* loop through the New_devsw linked list */
    for (bs=new_devsw_head; bs; bs=bs->nxt) {

	/* look first for a New_devsw which has both bdevsw and cdevsw values
	 * and which require that the major numbers match.
	 */
	if (((bs->flag & BDEV_CDEV) == BDEV_CDEV) && (bs->req == SAME)) {

	    /* look to see if the preferences match.  This could mean
	     * that they both don't care (Any) what major number gets
	     * assigned, but it must match.
	     */
	    if (bs->bmajor_pref == bs->cmajor_pref) {
		find_make_empty_match (&bentry, &centry, bs->bmajor_pref);

	    /* if either of the preferences is "Any" (-1) then use the
	     * other preference as the required major number.
	     */
	    } else if (bs->bmajor_pref == -1) {
		find_make_empty_match (&bentry, &centry, bs->cmajor_pref);
	    } else if (bs->cmajor_pref == -1) {
		find_make_empty_match (&bentry, &centry, bs->bmajor_pref);

	    /* both have a preference, and a requirement of Same, but
	     * the preferences don't match.
	     */
	    }

	    /* sanity check */
	    if ((bentry == NULL) || (centry == NULL)) {
		fprintf (stderr,
		  "Device_Block_Major != Device_Char_Major in stanza.static for entry %s\n", bs->config_name);
		exit(1);
	    }

	    /* replace the emtpy Entry with the contents of New_devsw
	     */
	    replace_devsw (BDEV_FLAG, bs, bentry);
	    replace_devsw (CDEV_FLAG, bs, centry);
	}

	/* now look for New_devsw's that have a requried bdevsw major number
	 */
	if ((bs->flag & BDEV_ONLY) && (bs->bmajor_pref >= 0) &&
	    (bs->req != SAME) ) {
	    bentry = find_make_specific_empty (BDEV_FLAG, bs->bmajor_pref);
	    if (! bentry) {
		fprintf (stderr, "Major number %d requested in stanza.static for entry %s is TAKEN\n", bs->bmajor_pref, bs->config_name);
		exit (1);
	    }
	    replace_devsw (BDEV_FLAG, bs, bentry);
	}

	/* now look for New_devsw's that have a requried cdevsw major number
	 */
	if ((bs->flag & CDEV_ONLY) && (bs->cmajor_pref >= 0) && 
	    (bs->req != SAME) ) {
	    centry = find_make_specific_empty (CDEV_FLAG, bs->cmajor_pref);
	    if (! centry ) {
		fprintf (stderr, "Major number %d requested in stanza.static for entry %s is TAKEN\n", bs->cmajor_pref, bs->config_name);
		exit (1);
	    }
	    replace_devsw (CDEV_FLAG, bs, centry);
	}
    }
}

/*
 * routine name:	assign_random()
 * description:		traverse the New_devsw linked list looking for
 *			New_devsw's that can be assigned any major
 *			number and which have no requirements.  Then
 *			find or create Entry's in the appropriate Entry
 *			linked list and fill in the Entry with the info
 *			found in New_devsw.
 * input:		none
 * output:		none
 * assumptions:
 * notes:
 */
assign_random()
{
New_devsw	*bs;
Entry	*bentry, *centry;
		/* Entry pointers used to search for or traverse
		 * Entry's in the devsw linked list of Entry's
		 */

	for (bs=new_devsw_head; bs; bs=bs->nxt) {
		if (bs->flag & BDEV_ONLY) {
			if ((bs->req == NONE) && (bs->bmajor_pref == -1)) {
				bentry = find_make_random_empty (BDEV_FLAG);
				replace_devsw (BDEV_FLAG, bs, bentry);
			} 
		}
		if (bs->flag & CDEV_ONLY) {
			if ((bs->req == NONE) && (bs->cmajor_pref == -1)) {
				centry = find_make_random_empty (CDEV_FLAG);
				replace_devsw (CDEV_FLAG, bs, centry);
			} 
		}
	}
}

/*
 * routine name:	fill_default()
 * description:		allocate space for and fill the lines of an 
 *			empty Entry.
 * input:		flag
 *				BDEV_FLAG - work on the bdevsw table
 *				CDEV_FLAG - work on the cdevsw table
 *			entry - pointer to the Entry being worked on.
 *			num   - major number of the Entry being worked on.
 * output:
 * assumptions:
 * notes:		future work:  the num parameter is superfluous,
 *			since a little coding smarts would have the
 *			major number in the Entry before the call to
 *			fill_default.
 */
fill_default (flag, entry, num)
int	flag;
Entry	*entry;
int	num;
{
Line	*line1, *line2, *line3, *line4, *line5;

	/* sanity check */
	assert ((flag == BDEV_FLAG) || (flag == CDEV_FLAG));

	/* allocate space for and build linked list of Line's for this entry
	 */
	line1 = get_next_line();
	line2 = get_next_line();
	line3 = get_next_line();
	line4 = get_next_line();
	line1->buf = get_char_buf2 (LINE_SIZE);
	line2->buf = get_char_buf2 (LINE_SIZE);
	line3->buf = get_char_buf2 (LINE_SIZE);
	line4->buf = get_char_buf2 (LINE_SIZE);
	entry->lines = line1;
	line1->nxt = line2;
	line2->nxt = line3;
	line3->nxt = line4;
	line4->nxt = NULL;

	/* fill in first two lines.  This is white space and a generic
	 * comment.
	 */
	sprintf (line1->buf, dev_line1);
	sprintf (line2->buf, dev_line2, empty_comment);

	/* the next two or three lines differ for bdevsw and cdevsw entries
	 */
	switch (flag) {
		case BDEV_FLAG:
			sprintf (line3->buf, bdev_line3,
			    default_bdevsw[0], default_bdevsw[1],
			    default_bdevsw[2], default_bdevsw[3], num);
			sprintf (line4->buf, bdev_line4,
			    default_bdevsw[4], default_bdevsw[5],
			    default_bdevsw[6], default_bdevsw[7]);
			break;
		case CDEV_FLAG:
			line5 = get_next_line();
			line4->nxt = line5;
			line5->buf = get_char_buf2 (LINE_SIZE);
			sprintf (line3->buf, cdev_line3,
			    default_cdevsw[0], default_cdevsw[1],
			    default_cdevsw[2], default_cdevsw[3], num);
			sprintf (line4->buf, cdev_line4,
			    default_cdevsw[4], default_cdevsw[5],
			    default_cdevsw[6], default_cdevsw[7]);
		    /* The DDI/DKI kernel hook adds two extra fields to the
		       cdevsw structure.  See the declaration of cdevsw
		       in src/kernel/sys/conf.h for details.
		    */
			sprintf (line5->buf, cdev_line5,
			    default_cdevsw[8], default_cdevsw[9],
			    default_cdevsw[10], default_cdevsw[11],
			    default_cdevsw[12]);
			break;
	}
}

/*
 * routine name:	copy_to_token_buf()
 * description:		copy the some lines from the obuf[] to the
 *			token_buf.  This will allow strtok to be used
 *			without inserting NULL characters into the
 *			obuf[].
 * input:		s - starting index to copy lines from obuf[]
 * output:
 * assumptions:		this is one of the places where it is assumed
 *			that the lines in obuf are in the same order as
 *			read
 * notes:
 */
copy_to_token_buf (s)
int	s;
{
int	i;
	for (i=0; i<TOKEN_LINES; i++) {
		strcpy (token_buf[i], obuf[s+i]);
	}
}

/*
 * routine name:	all_digit
 * description:		determine if a string is all digits
 * input:		pointer to a string
 * output:		boolean - True if the string is all digits
 * assumptions:
 * notes:
 */
all_digit (s)
char	*s;
{
	for (; ((*s != '\0') && (isdigit(*s))); s++) ;
	if (*s == '\0')
		return (TRUE);
	else
		return (FALSE);
}

/*
 * routine name:	add_to_entry_list()
 * description:		create a new Entry at the linked list.  put in the
 *			flg passed in, and calculate the major number.  Also
 *			create, link in and fill in the linked list of Line's.  
 * input:		entry - pointer to the Entry which is to precede the
 *			        Entry being added.
 *			start - index of obuf[] to start copy from.
 *			end   - index of obuf[] to end copy.
 *			flg   - value to be copied to the flag field of the
 *			        Entry.
 *			
 * output:
 * assumptions:		- Entry being added is at the end of the list.  
 * 			- This is one of the places where it is assumed
 *			  that the lines in obuf are in the same order as
 *			  read
 * notes:		future work: ?? maybe this and the
 *			add_empty_devsw do such similar things that
 *			they need to be merged?  just a thought - too
 *			busy to look at it now - may later.  For now it
 *			is working, so it should be left alone.
 */
Entry	*
add_to_entry_list (entry, start, end, flg)
Entry	*entry;
int	start;
int	end;
int	flg;
{
int	i;
Line	*line;
int	mj;

	entry->nxt = get_next_entry();
	mj = entry->emajor;
	entry = entry->nxt;
	entry->nxt = 0;
	entry->flag = flg;

/* account for the factor that it might be more than one entry in
 * #if-endif case
 */
        if (flg > 1)
        entry->emajor = mj + flg;
        else
	entry->emajor = ++mj;

	entry->lines = line = get_next_line();
	for (i=start; i<end; i++) {
		line->buf = obuf[i];
		line->nxt = get_next_line();
		line = line->nxt;
	}
	line->buf = obuf[end];
	line->nxt = 0;
	return (entry);
}

/*
 * routine name:	process_devsw()
 * description:		parse devsw table, and create the linked list of
 *			Entry's which reflect what has been found.
 * input:
 * output:
 * assumptions:		- when this routine is called for parsing the
 *			  bdevsw table, it is the first time anything is
 *			  put into obuf[], therefore oi is 0.  This is
 *			  crude.
 *			- this is one of the places where it is assumed
 *			  that the lines in obuf are in the same order as
 *			  read
 * notes:		future work: obuf should be replaced with calls
 *			to malloc(), but there is no time to make that
 *			change.
 */
process_devsw ()
{
int	b, e;	/* indexes into obuf[].  mark the beginning and end of the
		 * devsw table being parsed.
		 */
Line	*line;	/* element of linked list of Line's which are part of an
		 * Entry as it is being created.
		 */
int	eflag;	/* identifies the Entry which is being parsed.  may have the
		 * following values:
		 *	HEADER
		 *	TAKEN
		 *	FREE
		 *	FOOTER
		 */
Entry	*entry;	/* pointer to the Entry being constructed. */
int	pflag;	/* BDEV_FLAG or CDEV_FLAG, depending on which devsw table is
		 * being parsed.
		 */
int	i;

	/* set up indexes and determine which devsw is being worked on
	 */
	b = e = oi;
	if (b) {
		pflag = CDEV_FLAG;
	} else {
		pflag = BDEV_FLAG;
	}

	/* read the devsw into memeory.  stop reading at the next cookie.
	 */
	read_devsw();

	/* make sure that this is a valid beginning of a devsw table 
	 */
	check_begin_devsw (b, &e, pflag);

	/* set up the head Entry for this devsw
	 */
	switch (pflag) {
		case BDEV_FLAG:
			bdev_head = entry = get_next_entry();
			bdev_head->flag = HEADER;
			bdev_head->nxt = 0;
			bdev_head->emajor = (-1);
			bdev_head->lines = get_next_line();
			break;
		case CDEV_FLAG:
			cdev_head = entry = get_next_entry();
			cdev_head->flag = HEADER;
			cdev_head->nxt = 0;
			cdev_head->emajor = (-1);
			cdev_head->lines = get_next_line();
			break;
	}

	/* copy in the Line's for the HEADER
	 */
	line = entry->lines;
	for (i=b; i<(e-1); i++) {
		line->buf = obuf[i];
		line->nxt = get_next_line();
		line = line->nxt;
	}
	line->buf = obuf[e-1];
	line->nxt = 0;

	/* loop to read in and build the rest of the Entry's.  stop when
	 * an Entry of type FOOTER is found.
	 */
	b = e;
	while ((eflag = get_entry(b, &e, pflag)) != FOOTER) {
		assert (eflag != HEADER);
		entry = add_to_entry_list (entry, b, e, eflag);
		b = e + 1;
	}

	/* make sure that this is a valid end of devsw table and copy
	 * the Entry to the end of the list
	 */
	e++;
	check_end_devsw (e, &e, pflag);
	entry = add_to_entry_list (entry, b, e, FOOTER);

	/* record the size of this table */
	switch (pflag) {
		case BDEV_FLAG:
			bdev_size = entry->emajor;
			break;
		case CDEV_FLAG:
			cdev_size = entry->emajor;
			break;
	}
}

/*
 * routine name:	check_end_devsw()
 * description:		examine the contents of obuf[start] -
 *			obuf[end].  the purpose is to insure that this
 *			is a valid end of a devsw table.  If it is
 *			invalid, then exit()
 * input:		start - index into obuf where examination begins
 *			*end   - index into obuf where examination ended
 *			process_flag - BDEV_FLAG or CDEV_FLAG
 * output:		*end   - index into obuf where examination ended
 * assumptions:		this is one of the places where it is assumed
 *			that the lines in obuf are in the same order as
 *			read
 * notes:
 */
check_end_devsw (start, end, pflag)
int	start;
int	*end;
int	pflag;
{
int	i, t, tb;

	assert ((pflag == BDEV_FLAG) || (pflag == CDEV_FLAG));
	/*
	 * first, check that this is the start cookie
	 */
	copy_to_token_buf(start);

	/*
	 * first token should be the comment begin
	 */
	token[0] = strtok (token_buf[0], " \t\n");
	if (strcmp (token[0], "/*")) {
		fprintf (stderr, "devsw cookie mismatch\n");
		exit (1);
	}

	/* isolate the next few tokens
	 */
	tb = 1;
	t = 0;
	while (t<3) {
		token[t] = strtok (0, " \t\n\*");
		if (token[t] == NULL) {
			while (token[t] == NULL) {
				token[t] = strtok (token_buf[tb++], " \t\n\*");
				(*end)++;
			}
		}
		t++;
	}

	/* check for the end of table cookie.
	 */
	switch (pflag) {
		case BDEV_FLAG:
			if ((strncmp (token[0], "%%%", 3)) ||
			    (strcmp  (token[1], "END"))  ||
			    (strcmp  (token[2], "BDEVSW")) ) {
				fprintf (stderr, "bdevsw cookie mismatch\n");
				exit (1);
			}
			break;
		case CDEV_FLAG:
			if ((strncmp (token[0], "%%%", 3)) ||
			    (strcmp  (token[1], "END"))  ||
			    (strcmp  (token[2], "CDEVSW")) ) {
				fprintf (stderr, "cdevsw cookie mismatch\n");
				exit (1);
			}
			break;
	}

	/* move end pointer to end of cookie comment */
	while (strncmp (obuf[*end], "*/", 2) != 0) {
		(*end)++;
	}
}

/*
 * routine name:	pass_ifdef()
 * description:		find the end of the if{,def}-endif clause and record
 *			it's obuf[] index.
 * input:		start - first line of the if-endif clause
 *			*end - pointer to be returned
 * output:		*end - pointer to line within obuf[] which contains
 *			the #endif token.
 * assumptions:		this is one of the places where it is assumed
 *			that the lines in obuf are in the same order as
 *			read
 * notes:		future work:  maybe this should return the index
 *			rather, than play with pointers.
 *                      keep count of start bracket "{" within #if and #else
 *                      and return that count.
 */
pass_ifdef (start, end)
int	start;
int	*end;
{
int	tb;
int	not_found;
int	nif, nendif;
int     bracket_cnt = 0;
int     pass_else = FALSE;


	tb = 0;
	not_found = 1;
	nif = nendif = 0;
	while (not_found) {
		if (token[0] && (strncmp (token[0], "#if", 3) == 0))
			nif++;

                if (token[0] && (strcmp (token[0], "#else") == 0))
                        pass_else = TRUE;

                if (!pass_else)
                  if (token[0] && (strcmp (token[0], STBK) == 0))
                      bracket_cnt++;


		if (token[0] && (strcmp (token[0], "#endif") == 0)) {
			if (++nendif == nif) {
				not_found = 0;
				break;
			}
		}
		(*end)++;
		if (++tb == TOKEN_LINES) {
			copy_to_token_buf (*end);
			tb = 0;
		}
		token[0] = strtok (token_buf[tb], " \t\n");
	}
       return (bracket_cnt);
}

/*
 * routine name:	get_entry()
 * description:		look for a specific number of tokens from then
 *			next few lines in obuf[].  Examine these tokens
 *			and decide if this entry is TAKEN, FREE or a
 *			FOOTER.
 * input:
 * output:		Entry flag.  One of the following values:
 *				TAKEN
 *				FREE
 *				FOOTER
 *			*end - index into obuf[] where last token was found.
 * assumptions:		- assume a FREE Entry is one whose tokens consist
 *			  entirely of "0", "nodev" and "nulldev".  This
 *			  is crude, but for now it is sufficient
 *			- this is one of the places where it is assumed
 *			  that the lines in obuf are in the same order as
 *			  read
 * notes:
 */
int
get_entry (start, end, pflag)
int	start;
int	*end;
int	pflag;
{
int	in_comment;
int	l;
int	not_done;
int	t;
int	tb;
int	ret;
int	num_tokens;

	/* sanity check */
	assert ((pflag == BDEV_FLAG) || (pflag == CDEV_FLAG));

	/* get number of tokens to read from obuf[]
	 */
	switch (pflag) {
		case BDEV_FLAG:
			num_tokens = 7;
			break;
		case CDEV_FLAG:
			/* The DDI/DKI kernel hook addes two extra fields to
			   the cdevsw structure, see the declaration of
			   cdevsw in src/kernel/sys/conf.h for details */
			num_tokens = 12;
			break;
	}

	/* set up for loop */
	/* ret is set to an invalid value.  if it is still invalid at
	 * the end of the loop, then the Entry was neither a FOOTER nor
	 * was it within an if-endif clause.
	 */

        ret = INVALID;
	*end = start;
	copy_to_token_buf (start);
	t = tb = 0;
	token[t] = strtok (token_buf[tb], " \t\n,;");
	in_comment = FALSE;
	not_done = TRUE;

	/* loop until num_tokens have been extracted
	 */
	while (not_done) {
		/* pointer to current token is NULL if all tokens from this
		 * line have been extracted.
		 */
		if (token[t] == 0) {
			(*end)++;
			if (t == 0) {
				copy_to_token_buf (*end);
				tb = 0;
			} else {
				tb++;
				assert (tb < TOKEN_LINES);
			}
			token[t] = strtok (token_buf[tb], " \t\n,;");

		/* examine the token
		 */
		} else {
			/* pass over beginning brackets
			 */
			if (strcmp (token[t], STBK) == 0) {
				;

			/* if the first character of the token is an
			 * end bracket and this is the first token of
			 * this Entry to be examined, then the devsw
			 * has been completely parsed and this is a
			 * FOOTER.  note:  it could be the end bracket
			 * which marks the end of the Entry.
			 */
			} else if (strcmp (token[t], EDBK) == 0) {
				if (t == 0) {
					not_done = FALSE;
					ret = FOOTER;
				}

			/* is a comment being parsed at the moment? 
			 */
			} else if (in_comment) {
				if (strcmp (token[t], "*/") == 0) {
					in_comment = FALSE;
				}

			/* be real dumb about ifdef's.  only account
			 * for one Entry within an if-endif clause
			 * and mark it TAKEN.
			 */
                        /* modify to return the number of entries
                         * between #if and #else
                         */

			} else if ((token[t] == token_buf[tb]) &&
				   (strncmp (token[t], "#if", 3) == 0)) {
                                ret = pass_ifdef (start, end);
                                if (ret == 1)
                                   ret = TAKEN;
				not_done = FALSE;

			/* look for the beginning of a comment
			 */
			} else if ((token[t][0] == '/') &&
			           (token[t][1] == '*')) {
				l = strlen (token[t]);
				if ((token[t][l-2] != '*') ||
				    (token[t][l-1] != '/')) {
					in_comment = TRUE;
				}

			/* this must be a token, or rather an entry in the
			 * devsw table Entry.  increment the token
			 * counter and keep going.
			 */
			} else {
				if (t < num_tokens) {
					t++;
				} else if (t == num_tokens) {
					not_done = FALSE;
				}
			}

			assert (t < TOKENS);

			/* get the next token
			 */
			token[t] = strtok (0, " \t\n,;");
		}

		/* check to see if enough tokens have been extracted
		 */
		if (not_done) {
			if (t > num_tokens) {
				not_done = FALSE;
			}
		}
	}

	/* if ret is still invalid, then the Entry was neither a FOOTER, nor
	 * was it ifdef'd.  So, it needs to be examined to see if it is
	 * FREE or TAKEN.
	 */
        if (ret <= INVALID) {
		ret = 0;
		for (l=0; ((l<num_tokens)&&(ret==l)); l++) {
			if (strcmp (token[l], "0") == 0) {
				ret++;
			} else if (strcmp (token[l], "nodev") == 0) {
				ret++;
			} else if (strcmp (token[l], "nulldev") == 0) {
				ret++;
			}
		}
		if (ret == num_tokens) {
			ret = FREE;
		} else {
			ret = TAKEN;
		}
	}
	return (ret);
}

/*
 * routine name:	check_begin_devsw()
 * description:		examine the contents of obuf[start] -
 *			obuf[end].  the purpose is to insure that this
 *			is a valid begin of a devsw table.  If it is
 *			invalid, then exit()
 * input:		start - index into obuf to start looking at
 *			*end   - index into obuf where examination ended
 *			pflag - BDEV_FLAG or CDEV_FLAG
 * output:		*end   - index into obuf where examination ended
 * assumptions:		this is one of the places where it is assumed
 *			that the lines in obuf are in the same order as
 *			read
 * notes:		crude, but effective.  
 */
check_begin_devsw (start, end, pflag)
int	start;
int	*end;
int	pflag;
{
int	i;
int	t;
int	tb;

	assert ((pflag == BDEV_FLAG) || (pflag == CDEV_FLAG));
	/*
	 * first, check that this is the start cookie
	 */
	copy_to_token_buf(start);
	for (i=0; i<TOKENS; i++) {
		token[i] = NULL;
	}

	/* get first three tokens.
	 */
	t = 0;
	tb = 0;
	while ((t<3) || (token[t] == NULL)) {
		if (token[t] == NULL) {
			token[t] = strtok (token_buf[tb++], " \t\n\*");
			(*end)++;
		}  else {
			token[++t] = strtok (0, " \t\n\*");
		}
	}

	/* make sure the first three tokens are valid
	 */
	switch (pflag) {
		case BDEV_FLAG:
			if ((strncmp (token[0], "%%%", 3)) ||
			    (strcmp  (token[1], "BEGIN"))  ||
			    (strcmp  (token[2], "BDEVSW")) ) {
				fprintf (stderr, "bdevsw cookie mismatch\n");
				exit (1);
			}
			break;
		case CDEV_FLAG:
			if ((strncmp (token[0], "%%%", 3)) ||
			    (strcmp  (token[1], "BEGIN"))  ||
			    (strcmp  (token[2], "CDEVSW")) ) {
				fprintf (stderr, "cdevsw cookie mismatch\n");
				exit (1);
			}
			break;
	}

	/*
	 * assume next token is not null; parse to end of comment
	 */
	token[0] = strtok (0, " \t\n");
	while (token[0]) {
		if (strcmp (token[0], "\*\/") == 0) {
			break;
		}
		token[0] = strtok (0, " \t\n");
		if (token[0] == NULL) {
			if (tb == TOKEN_LINES) {
				fprintf (stderr, "too many lines in cookie\n");
				exit (1);
			} else {
				token[0] = strtok (token_buf[tb++], " \t\n");
				(*end)++;
			}
		}
	}

	/*
	 * next six tokens should make up the declaration of the devsw table
	 */
	for (t=0; t<6; t++) {
		token[t] = strtok (0, " \t\n\[\]");
		while (token[t] == NULL) {
			if (tb == TOKEN_LINES) {
				fprintf (stderr,
				    "more than %d lines for header\n");
				exit (1);
				tb = 0;
			}
			token[t] = strtok (token_buf[tb++], " \t\n\[\]");
			(*end)++;
		}
	}

	/* check that the declaration is correct
	 */
	switch (pflag) {
		case BDEV_FLAG:
			if ( (strcmp (token[0], "struct"))     ||
	     		(strcmp (token[1], "bdevsw"))     ||
	     		(strcmp (token[2], "bdevsw"))     ||
	     		(strcmp (token[3], "MAX_BDEVSW")) ||
	     		(strcmp (token[4], "="))          ||
	     		(strcmp (token[5], STBK))          ) {
				fprintf (stderr,
				    "parsing error at beginning of bdevsw\n");
				exit (1);
			}
			break;
		case CDEV_FLAG:
			if ( (strcmp (token[0], "struct"))     ||
	     		(strcmp (token[1], "cdevsw"))     ||
	     		(strcmp (token[2], "cdevsw"))     ||
	     		(strcmp (token[3], "MAX_CDEVSW")) ||
	     		(strcmp (token[4], "="))          ||
	     		(strcmp (token[5], STBK))          ) {
				fprintf (stderr,
				    "parsing error at beginning of cdevsw\n");
				exit (1);
			}
			break;
	}
}

/*
 * routine name:	read_devsw()
 * description:		read in the lines from the devsw.  stop reading when
 *			the next "cookie" line is found.  
 * input:
 * output:
 * assumptions:		parsing and validation will be done by process_devsw()
 * notes:		At some future date, this routine should be rewritten
 *			to avoid obuf[].  In other words, do not use
 *			get_char_buf().  Instead, get_char_buf2() should be
 *			used.
 */
read_devsw ()
{
int	i;

	i=0;
	strcpy (get_char_buf(), tbuf);
	while (i<2) {
		if (strncmp (tbuf, "%%%", 3) == 0) {
			i++;
		}
		if (fgets (tbuf, LINE_SIZE, ifp) == 0) {
			fprintf (stderr, "EOF on conf.c\n");
			exit (1);
		}
		strcpy (get_char_buf(), tbuf);
	}
}

/*
 * routine name:	process_stanza()
 * description:		use the AF library to extract information from
 *			the stanza.static file.  Use this informtion to
 *			build and fill a New_devsw structure.
 * input:
 * output:
 * assumptions:
 * notes:
 */
process_stanza()
{
char	*req;		/* pointer to the contents of the Requirement field */
char	*pref;		/* ptr to the contents of the Preference field */
char	*name, *name2;	/* ptrs to the minor number and filename
			 * fields.  These are read in pairs since they
			 * must be recorded in pairs
			 */
ATTR_t	attr, attr2;	/* ptr to AF attribute.  Need pair for minor/filename
			 * pair testing
			 */
ENT_t	ent;		/* ptr to AF ent */
AFILE_t	afile;		/* FILE ptr, and other stuff, pointing to
			 * Stanza.static file
			 */

char	List[LINE_SIZE];
			/* buffer to build the NAME.list filename in */

FILE	*list_fp;	/* ptr to NAME.list file descriptor */

char	Stanza[LINE_SIZE];
			/* buffer to build the stanza.static path and
			 * filename in
			 */
FILE	*stan_fp;	/* ptr to the stanza.static file descriptor */
char	list_entry[1024];
			/* buffer for lines as read from NAME.list file */

char	copy[1024];	/* copy of list_entry so that strtok can be used */
char	*rtn;		/* return status from fgets() */
char	*pathname;	/* first token from list_entry is the pathname of the
			 * stanza.static file
			 */
int	ret;		/* return flag - TRUE if there was anything valid in
			 * the stanza.static file
			 */
Minor	*min_ptr;	/* pointer to the Minor structure which will become
			 * part of the Bassign_table for the New_devsw
			 */
New_devsw	*nd;	/* pointer to the New_devsw which is being added to
			 * the linked list of New_devsw.  This will
			 * record all the information which has been
			 * extracted from the stanza.static file
			 */
Bassign_table	*ac;	/* pointer to the Bassign_table which is being built
			 * for this New_devsw
			 */
int	min1, min2;	/* minimum and maximum range found while parsing the
			 * Device_Block_Minor and Device_Char_Minor stanza
			 * fields.
			 */
int	minrange;	/* flag to indicate we are in the middle of expanding
			 * a range for the Device_Block_Minor and
			 * Device_Char_Minor fields.
			 */
char	dev1, dev2;	/* minimum and maximum range found while parsing the
			 * Device_Block_Files and Device_Char_Files stanza
			 * fields.
			 */
int	devrange;	/* flag to indicate we are in the middle of expanding
			 * a range for the Device_Block_Files and
			 * Device_Char_Files fields.
			 */
char	*range;		/* pointer used to find range field within the string
			 * read from the static.stanza file.
			 */
char	basename[ANAMELEN];
			/* basename of special device files, used while
			 * building names for ranges of file names.
			 */
int	i;

    minrange = devrange = FALSE;

    /* open the NAME.list file, if it exists
     */
    sprintf (List, "./%s.list", PREFIX);
    list_fp = VPATHopen (List, "r");
    ret = FALSE;

    /* test to see if the NAME.list was successfully opened.  If it was, then
     * get the first line.
     */
    if (list_fp != 0) {
	rtn = fgets (list_entry, 1024, list_fp);

	/* loop as lines are read from NAME.list
	 */
	for ( ; rtn != (char *) 0; ) {

	    /* OK now we have a line from the PREFIX.list file which
	     * should point us to an area that MAY contain a files file
	     * and config file entries for use when building THIS
	     * kernel.
	     */

	    (void) strcpy(copy,list_entry);

	    /* Test for blank lines and comment lines */
	    if ( ((strlen (copy)-1) != (strspn (copy, " \t")))
                            && (copy[0] != '#')) {

		/* get first entry from colon separated string.  Then
		 * remove the \n in the string
		 */
		pathname = strtok(copy,":\n");

		/* build the path and filename, and open the file.
		 * Make sure the target char buffer is filled with null
		 * characters.
		 */

		bzero (Stanza, (sizeof(char) * LINE_SIZE) );
		(void) strncpy (Stanza, pathname, strlen(pathname));
		(void) strcat (Stanza, "/stanza.static");

		stan_fp = fopen (Stanza, "r");

		/* test to see if stanza.static was opened
		 */
		if (stan_fp != NULL) {

		    /* tell the AF routines to open and initialize it's
		     * structures
		     */
		    afile = (AFILE_t) AFopen (Stanza, 1024, 75);

		    /* look for an entry in the stanza.static file
		     */
		    ent = (ENT_t) AFnxtent (afile);

		    while (ent) {

		      /* get the config_name, this is a required field
		       */
		      attr = (ATTR_t) AFgetatr (ent, module_config_name);

		      if (attr) {

			/* create the new New_devsw.  If this is the
			 * head of the New_devsw linked list, then set
			 * the return flag
			 */
			if (new_devsw_head) {
				nd->nxt = (New_devsw *)
				    malloc (sizeof (New_devsw));
				nd = nd->nxt;
			} else {
				new_devsw_head = (New_devsw *) 
				    malloc (sizeof (New_devsw));
				nd = new_devsw_head;
				ret = TRUE;
			}

			/* store the config_name
			 */
			nd->config_name = get_char_buf2 (LINE_SIZE);
			strcpy (nd->config_name, AFgetval (attr));
			nd->nxt = 0;

			/* now look for the Description
			 */
			attr = (ATTR_t) AFgetatr (ent, subsystem_description);
			if (attr) {
			    nd->description = get_char_buf2 (LINE_SIZE);
			    strcpy (nd->description, 
				AFgetval (attr));
			}

			/* loop to look for each of the possible block
			 * driver routines.  For each possible routine
			 * store either the contents of the field from
			 * the stanza.static file, or the default (null)
			 * entry.
			 */
			for (i=0; i<BMAX; i++) {
			    attr = (ATTR_t) AFgetatr (ent, stanza_bdevsw[i]);
			    if (attr == 0) {
				strcpy (nd->bdevsw[i],
				    default_bdevsw[i]);
			    } else {
				name = (char *) AFgetval (attr);
				strcpy (nd->bdevsw[i], name);
				nd->flag |= BDEV_ONLY;
			    }
			}

			/* get the block major number preference
			 */
			attr = (ATTR_t) AFgetatr (ent, bmajor_preference);
			if (attr == 0) {
			    nd->bmajor_pref = ANY;
			} else {
			    pref = (char *) AFgetval (attr);
			    if (strcmp (pref, Any) == 0) {
				nd->bmajor_pref = ANY;
			    } else if (all_digit (pref)) {
				nd->bmajor_pref = atoi(pref);
			    } else {
				nd->bmajor_pref = ANY;
			    }
			}
			nd->bmajor_assign = ANY;

			/* loop to look for each of the possible char
			 * driver routines.  For each possible routine
			 * store either the contents of the field from
			 * the stanza.static file, or the default (null)
			 * entry.
			 */
			for (i=0; i<CMAX; i++) {
			    attr = (ATTR_t) AFgetatr (ent, stanza_cdevsw[i]);
			    if (attr == 0) {
				strcpy (nd->cdevsw[i], 
				    default_cdevsw[i]);
			    } else {
				name = (char *) AFgetval (attr);
				strcpy (nd->cdevsw[i], name);
				nd->flag |= CDEV_ONLY;
			    }
			}

			/* get the character major number preference
			 */
			attr = (ATTR_t) AFgetatr (ent, cmajor_preference);
			if (attr == 0) {
			    nd->cmajor_pref = ANY;
			} else {
			    pref = (char *) AFgetval (attr);
			    if (strcmp (pref, Any) == 0) {
				nd->cmajor_pref = ANY;
			    } else if (all_digit (pref)) {
				nd->cmajor_pref = atoi(pref);
			    } else {
				nd->cmajor_pref = ANY;
			    }
			}
			nd->cmajor_assign = ANY;

			/* get the major number requirements
			 */
			attr = (ATTR_t) AFgetatr (ent, major_require);
			if (attr == 0) {
			    nd->req = NONE;
			} else {
			    req = (char *) AFgetval (attr);
			    if (strcmp (req, Same) == 0) {
				nd->req = SAME;
			    } else {
				nd->req = NONE;
			    }
			}

			/* if there was either block or character
			 * routines, then set up the bassign_table
			 */
			if (nd->flag) {
			    ac = (Bassign_table *) 
				malloc (sizeof(Bassign_table));
			    nd->assign = ac;

			    /* extract directory info from stanza.static.
			     * build directory path and store it in the
			     * bassign_table.  block and character
			     * devices may have differnet directory names
			     * for their special files.
			     */
			    attr = (ATTR_t) AFgetatr (ent, device_dir);
			    if (attr == 0) {
			        sprintf (ac->bdir, "%s", 
				    device_dir_default);
			        sprintf (ac->cdir, "%s", 
				    device_dir_default);
			    } else {
			        name = AFgetval (attr);
			        if (strlen(name)) {
				    sprintf (ac->bdir, "%s", name);
				    sprintf (ac->cdir, "%s", name);
			        } else {
			            sprintf (ac->bdir, "%s", 
					device_dir_default);
			            sprintf (ac->cdir, "%s", 
					device_dir_default);
			        }
			    }
			    attr = (ATTR_t) AFgetatr (ent, device_b_subdir);
			    if (attr == 0) {
			        attr = (ATTR_t) AFgetatr (ent, device_subdir);
			    }
			    if (attr == 0) {
			        sprintf (ac->bdir, "%s/%s", 
				    ac->bdir, device_subdir_default);
			    } else {
			        name = AFgetval (attr);
			        if (strlen(name)) {
			            sprintf (ac->bdir, "%s/%s", 
					ac->bdir, name);
			        } else {
			            sprintf (ac->bdir, "%s/%s", 
					ac->bdir, 
					device_subdir_default);
			        }
			    }
			    attr = (ATTR_t) AFgetatr (ent, device_c_subdir);
			    if (attr == 0) {
			        attr = (ATTR_t) AFgetatr (ent, device_subdir);
			    }
			    if (attr == 0) {
			        sprintf (ac->cdir, "%s/%s", 
				    ac->cdir, device_subdir_default);
			    } else {
			        name = AFgetval (attr);
			        if (strlen(name)) {
			            sprintf (ac->cdir, "%s/%s", 
					ac->cdir, name);
			        } else {
			            sprintf (ac->cdir, "%s/%s", 
					ac->cdir, 
					device_subdir_default);
			        }
			    }

			    /* now extract the minor number and device
			     * filename infor for both block and
			     * character devices.  make sure that minor
			     * number and device filenames are in pairs.
			     */
			    attr = (ATTR_t) AFgetatr (ent, block_minor);
			    attr2 = (ATTR_t) AFgetatr (ent, block_files);
			    if (attr && attr2) {
			        name = AFgetval (attr);
			        name2 = AFgetval (attr2);
			        while (name && name2) {
				    min_ptr = ac->bminor;
				    if (min_ptr) {
				        while (min_ptr->nxt) {
					    min_ptr = min_ptr->nxt;
				        }
				        min_ptr->nxt = (Minor *)
					    malloc (sizeof(Minor));
				        min_ptr = min_ptr->nxt;
				    } else {
				        ac->bminor = (Minor *)
					    malloc (sizeof(Minor));
				        min_ptr = ac->bminor;
				    }
			            min_ptr->nxt = NULL;

				    /* if not already expanding a minor
				     * number range, then check for range of
				     * minor numbers
				     */

				    if ((! minrange) && (*name == '[')) {

					/* we are in a new range.  Get the
					 * minimum and maximum values, record
					 * the first value and set the minrange
					 * flag
					 */

					minrange = sscanf 
					    (name, "[%d-%d", &min1, &min2);
					min_ptr->num = min1;
					if ((minrange == 2) && (min1 < min2)) {
					    minrange = TRUE;
					} else {
					    minrange = FALSE;
					}

				    } else if (minrange) {

					/* we are in the middle of parsing a
					 * range.  Record the next value and
					 * then check for the end of the
					 * range.
					 */
					min1 ++;
					min_ptr->num = min1;
					if (min1 >= min2) {
					    minrange = FALSE;
					}

				    } else {

					/* there is no range.  just record
					 * the number.
					 */
					min_ptr->num = atoi(name);
				    }

				    /* if we are not currently expanding a
				     * range, then get the next minor number in
				     * the stanza entry.
				     */

				    if (! minrange) {
			                name = AFnxtval (attr);
				    }

				    /* now on to the device file names.
				     * If we are not currently expanding a
				     * range, then ....
				     */
				    if (! devrange) {

					/* .... then look to see if we are
					 * now looking at a range.  This
					 * forloop will exit early if a range
					 * is found.  leaving the devrange flag
					 * set.
					 */
					for (devrange=strlen(name2); (devrange 
					     && (name2[devrange] != ']')); 
					     devrange--) ;

					if (devrange) {

					    /* separate out the basename for
					     * the filenames, and then record
					     * the minimum and maximum of the
					     * range values.  range values may
					     * be pairs of digits, pairs of
					     * lower case alpha characters, or
					     * pairs of upper case alpha
					     * characters.
					     * Also, set the devrange flag.
					     */
					    range = strchr (name2, '[');
					    devrange = 
						strlen(name2)-strlen(range);
					    strncpy (basename, name2, devrange);
					    basename[devrange] = '\0';
					    devrange = sscanf (range, 
						"[%c-%c]", &dev1, &dev2);
					    if (devrange == 2) {
						if ((isdigit(dev1) && 
						     isdigit(dev2)     ) || 
						    (islower(dev1) && 
						     islower(dev2)     ) || 
						    (isupper(dev1) && 
						     isupper(dev2)     )) {
						    devrange = TRUE;
						} else {
						    devrange = FALSE;
						}
					    } else {
						devrange = FALSE;
					    }
					}
				    }

				    if (devrange) {
					/* we are expanding a range.  the
					 * file name is based on the basename
					 * and the mimimum value.
					 */
					sprintf (min_ptr->dname, "%s%c", 
					    basename, dev1);

					/* increment the mimimum value and
					 * see if the range has been fully
					 * expanded.  set devrange needed.
					 */
					dev1++;
					if (dev1 > dev2) {
					    devrange = FALSE;
					}

				    } else {
					/* we are not expanding a range, just
					 * copy in the filename.
					 */
					strcpy (min_ptr->dname, name2);
				    }

				    /* only get the next filename if we are
				     * not in the middle of expanding a range.
				     */
				    if (! devrange) {
					name2 = AFnxtval (attr2);
				    }
			        }
			    }
			    /* If no minor number or special device files */
			    /* specified then mark this entry to not have */
			    /* an entry in assign table.		  */
			    if (!attr && !attr2) {
				    nd->flag |= NO_BDEV;

			    }
			    /* Check to make sure we didn't have an uneven */
			    /* number of name/number pairs.		   */
			    else {
				    if ((name && ~(long)name2) || 
					 (~(long)name && name2)) {
					    fprintf(stderr, "Unmatched minor name/number pairs in %s for device %s\n", Stanza, nd->config_name);
					    exit(1);
				    }
			    }

			    attr = (ATTR_t) AFgetatr (ent, char_minor);
			    attr2 = (ATTR_t) AFgetatr (ent, char_files);
			    if (attr && attr2) {
			        name = AFgetval (attr);
			        name2 = AFgetval (attr2);
			        while (name && name2) {
				    min_ptr = ac->cminor;
				    if (min_ptr) {
				        while (min_ptr->nxt) {
					    min_ptr = min_ptr->nxt;
				        }
				        min_ptr->nxt = (Minor *)
					    malloc (sizeof(Minor));
				        min_ptr = min_ptr->nxt;
				    } else {
				        ac->cminor = (Minor *)
					    malloc (sizeof(Minor));
				        min_ptr = ac->cminor;
				    }
			            min_ptr->nxt = NULL;

				    /* if not already expanding a minor
				     * number range, then check for range of
				     * minor numbers
				     */

				    if ((! minrange) && (*name == '[')) {

					/* we are in a new range.  Get the
					 * minimum and maximum values, record
					 * the first value and set the minrange
					 * flag
					 */

					minrange = sscanf 
					    (name, "[%d-%d", &min1, &min2);
					min_ptr->num = min1;
					if ((minrange == 2) && (min1 < min2)) {
					    minrange = TRUE;
					} else {
					    minrange = FALSE;
					}

				    } else if (minrange) {

					/* we are in the middle of parsing a
					 * range.  Record the next value and
					 * then check for the end of the
					 * range.
					 */
					min1 ++;
					min_ptr->num = min1;
					if (min1 >= min2) {
					    minrange = FALSE;
					}

				    } else {

					/* there is no range.  just record
					 * the number.
					 */
					min_ptr->num = atoi(name);
				    }

				    /* if we are not currently expanding a
				     * range, then get the next minor number in
				     * the stanza entry.
				     */

				    if (! minrange) {
			                name = AFnxtval (attr);
				    }

				    /* now on to the device file names.
				     * If we are not currently expanding a
				     * range, then ....
				     */
				    if (! devrange) {

					/* .... then look to see if we are
					 * now looking at a range.  This
					 * forloop will exit early if a range
					 * is found.  leaving the devrange flag
					 * set.
					 */
					for (devrange=strlen(name2); (devrange 
					     && (name2[devrange] != ']')); 
					     devrange--) ;

					if (devrange) {

					    /* separate out the basename for
					     * the filenames, and then record
					     * the minimum and maximum of the
					     * range values.  range values may
					     * be pairs of digits, pairs of
					     * lower case alpha characters, or
					     * pairs of upper case alpha
					     * characters.
					     * Also, set the devrange flag.
					     */
					    range = strchr (name2, '[');
					    devrange = 
						strlen(name2)-strlen(range);
					    strncpy (basename, name2, devrange);
					    basename[devrange] = '\0';
					    devrange = sscanf (range, 
						"[%c-%c]", &dev1, &dev2);
					    if (devrange == 2) {
						if ((isdigit(dev1) && 
						     isdigit(dev2)     ) || 
						    (islower(dev1) && 
						     islower(dev2)     ) || 
						    (isupper(dev1) && 
						     isupper(dev2)     )) {
						    devrange = TRUE;
						} else {
						    devrange = FALSE;
						}
					    } else {
						devrange = FALSE;
					    }
					}
				    }

				    if (devrange) {
					/* we are expanding a range.  the
					 * file name is based on the basename
					 * and the mimimum value.
					 */
					sprintf (min_ptr->dname, "%s%c", 
					    basename, dev1);

					/* increment the mimimum value and
					 * see if the range has been fully
					 * expanded.  set devrange needed.
					 */
					dev1++;
					if (dev1 > dev2) {
					    devrange = FALSE;
					}

				    } else {
					/* we are not expanding a range, just
					 * copy in the filename.
					 */
					strcpy (min_ptr->dname, name2);
				    }

				    /* only get the next filename if we are
				     * not in the middle of expanding a range.
				     */
				    if (! devrange) {
					name2 = AFnxtval (attr2);
				    }
			        }
			    }
			    /* If no minor number or special device files */
			    /* specified then mark this entry to not have */
			    /* an entry in assign table.		  */
			    if (!attr && !attr2) {
				    nd->flag |= NO_CDEV;

			    }

			    /* Check to make sure we didn't have an uneven */
			    /* number of name/number pairs.		   */
			    else {
				    if ((name && ~(long)name2) || 
					(~(long)name && name2)) {
					   fprintf(stderr, "Unmatched minor name/number pairs in %s for device %s\n", Stanza, nd->config_name);
					    exit(1);
				    }
			    }

			    /* now get the UID, GID and protection
			     * information
			     */
		
			    attr = (ATTR_t) AFgetatr (ent, device_user);
			    if (attr == 0) {
			        ac->user.flag = NUMERIC;
			        ac->user.value.num = 
				    device_user_default;
			    } else {
			        name = AFgetval (attr);
			        if (all_digit (name)) {
				    ac->user.flag = NUMERIC;
				    ac->user.value.num = atoi(name);
			        } else {
				    ac->user.flag = ALPHA;
				    strcpy (ac->user.value.alphabetic, name);
			        }
			    }
			    attr = (ATTR_t) AFgetatr (ent, device_group);
			    if (attr == 0) {
			        ac->group.flag = NUMERIC;
			        ac->group.value.num = 
				    device_group_default;
			    } else {
			        name = AFgetval (attr);
			        if (all_digit (name)) {
				    ac->group.flag = NUMERIC;
				    ac->group.value.num = atoi(name);
			        } else {
				    ac->group.flag = ALPHA;
				    strcpy (ac->group.value.alphabetic, name);
			        }
			    }
			    attr = (ATTR_t) AFgetatr (ent, device_mode);
			    if (attr == 0) {
			        ac->mode = device_mode_default;
			    } else {
			        name = AFgetval (attr);
			        if (all_digit (name)) {
			            ac->mode = 0;
			            for (i=0; i<strlen(name); i++) {
				        ac->mode = ac->mode * 8
					    + (name[i] - '0');
			            }
			        } else {
				    ac->mode = device_mode_default;
			        }
			    }
			} /* endif need for bassign_table within New_devsw */
		      } /* endif test for config_name */
		      ent = (ENT_t) AFnxtent (afile);
		    } /* end loop to read AF entries */
		} /* endif stanza.file open */
	    } /* endif test for blank lines */
	    rtn = fgets(list_entry,1024,list_fp);
	} /* end for loop that reads NAME.list */
    }
    return (ret);
}

/* routine name:	print_assign_table()
 * purpose:		to write the Assign_table to the end of conf.c
 * input:
 * output:
 * assumptions:
 * notes:
 */

print_assign_table()
{
New_devsw	*bs;
Bassign_table	*a;
Minor		*m;
char		*mn;
char		*dv;
char		*new;
int		i;
int		D;
int		len;

	/* make a guess about how big these arrays should be.  the variable
	 * len will keep track of how big it is.
	 */
	mn = (char *) malloc (ANAMELEN);
	assert (mn);
	dv = (char *) malloc (ANAMELEN);
	assert (dv);
	len = ANAMELEN;

	/* generate a NULL pointer if there were no New_devsw's
	 */
	if (! new_devsw_head) {
	    fprintf (ofp, "\nAssign_table\t*assign_ptr = NULL;\n");

	} else {
	    /* first, loop to generate all the minor and dev_name arrays
	     */
	    fprintf (ofp, "\n");
	    for (bs=new_devsw_head; bs; bs=bs->nxt) {
		a = bs->assign;
		if ((bs->flag & BDEV_ONLY) && ((bs->flag & NO_BDEV) == 0)) {
			sprintf (mn, "int\t%s%db_minor[] = {", bs->config_name, 
			    bs->bmajor_assign);
			sprintf (dv, "char\t%s%db_dev_name[][ANAMELEN] = {", 
			    bs->config_name, bs->bmajor_assign);
			for (m=a->bminor; m; m=m->nxt) {
				/* build the two character arrays, checking
				 * that they don't outgrow their allocated 
				 * lengths.
				 */
				D = strlen(dv) + strlen (m->dname) + 6;
				if (D >= len) {
					len += ANAMELEN;
					mn = (char *) realloc (mn, len);
					assert (mn);
					dv = (char *) realloc (dv, len);
					assert (dv);
					strcat (mn, "\n");
					strcat (dv, "\n");
				}
				sprintf (mn, "%s %d%c", mn, m->num,
				    ((m->nxt)?',':'\0'));
				sprintf (dv, "%s \"%s\"%c", dv, m->dname,
				    ((m->nxt)?',':'\0'));
			}
			fprintf (ofp, "%s , -1};\n", mn);
			fprintf (ofp, "%s , 0};\n", dv);
		}

		/* free and reallocate these to the (possibly) shorter length 
		 * so that the lines which are written to conf.c don't get too
		 * long.
		 */
		free (mn);
		free (dv);
		mn = (char *) malloc (ANAMELEN);
		assert (mn);
		dv = (char *) malloc (ANAMELEN);
		assert (dv);
		len = ANAMELEN;
		
		if ((bs->flag & CDEV_ONLY) && ((bs->flag & NO_CDEV) == 0) ) {
			sprintf (mn, "int\t%s%dc_minor[] = {", bs->config_name, 
			    bs->cmajor_assign);
			sprintf (dv, "char\t%s%dc_dev_name[][ANAMELEN] = {", 
			    bs->config_name, bs->cmajor_assign);
			for (m=a->cminor; m; m=m->nxt) {
				/* build the two character arrays, checking
				 * that they don't outgrow their allocated 
				 * lengths.
				 */
				D = strlen(dv) + strlen (m->dname) + 6;
				if (D >= len) {
					len += ANAMELEN;
					mn = (char *) realloc (mn, len);
					assert (mn);
					dv = (char *) realloc (dv, len);
					assert (dv);
					strcat (mn, "\n");
					strcat (dv, "\n");
				}
				sprintf (mn, "%s %d%c", mn, m->num,
				    ((m->nxt)?',':'\0'));
				sprintf (dv, "%s \"%s\"%c", dv, m->dname,
				    ((m->nxt)?',':'\0'));
			}
			fprintf (ofp, "%s , -1};\n", mn);
			fprintf (ofp, "%s , 0};\n", dv);
		}
	    }
	    fprintf (ofp, "\nAssign_table\tassign_table[] = {\n");
	    for (bs=new_devsw_head,i=0; bs; bs=bs->nxt,i++) {
		a = bs->assign;
		if ((bs->flag & BDEV_ONLY) && ((bs->flag & NO_BDEV) ==0) ) {
			fprintf (ofp, "\t{\t\t/* assign_table[%d] */\n", i);
			fprintf (ofp, "\t\"%s\",\t/* config name */\n",
			    bs->config_name);
			fprintf (ofp, "\t\"b\",\t/* block/char device */\n");
			fprintf (ofp, "\t0%o,\t/* protection mode */\n", 
			    a->mode);
			if (a->user.flag == ALPHA) {
			    fprintf (ofp, "\t\"%s\",\t/* user name */\n", 
				a->user.value.alphabetic);
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				device_user_default);
			} else {
			    fprintf (ofp, "\t\"%s\",\t/* user name */\n", "");
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				a->user.value.num);
			}
			if (a->group.flag == ALPHA) {
			    fprintf (ofp, "\t\"%s\",\t/* group name */\n", 
				a->group.value.alphabetic);
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				device_group_default);
			} else {
			    fprintf (ofp, "\t\"%s\",\t/* group name */\n", "");
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				a->group.value.num);
			}
			fprintf (ofp, "\t\"%s\",\t/* directory name */\n",
			    a->bdir);
			fprintf (ofp, "\t%d,\t/* major number */\n",
			    bs->bmajor_assign);
			fprintf (ofp, 
			   "\t%s%db_minor,\t/* minor number array */\n", 
			   bs->config_name, bs->bmajor_assign);
			fprintf (ofp, "\t(char *) %s%db_dev_name\t",
			   bs->config_name, bs->bmajor_assign);
			fprintf (ofp, "/* device filename array */\n");
			fprintf (ofp, "\t},\n");
		}
		if ((bs->flag & CDEV_ONLY) && ((bs->flag & NO_CDEV) ==0)) {
			fprintf (ofp, "\t{\t\t/* assign_table[%d] */\n", i);
			fprintf (ofp, "\t\"%s\",\t/* config name */\n",
			    bs->config_name);
			fprintf (ofp, "\t\"c\",\t/* block/char device */\n");
			fprintf (ofp, "\t0%o,\t/* protection mode */\n", 
			    a->mode);
			if (a->user.flag == ALPHA) {
			    fprintf (ofp, "\t\"%s\",\t/* user name */\n", 
				a->user.value.alphabetic);
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				device_user_default);
			} else {
			    fprintf (ofp, "\t\"%s\",\t/* user name */\n", "");
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				a->user.value.num);
			}
			if (a->group.flag == ALPHA) {
			    fprintf (ofp, "\t\"%s\",\t/* group name */\n", 
				a->group.value.alphabetic);
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				device_group_default);
			} else {
			    fprintf (ofp, "\t\"%s\",\t/* group name */\n", "");
			    fprintf (ofp, "\t%d,\t/* uid */\n", 
				a->group.value.num);
			}
			fprintf (ofp, "\t\"%s\",\t/* directory name */\n",
			    a->cdir);
			fprintf (ofp, "\t%d,\t/* major number */\n",
			    bs->cmajor_assign);
			fprintf (ofp, 
			   "\t%s%dc_minor,\t/* minor number array */\n", 
			   bs->config_name, bs->cmajor_assign);
			fprintf (ofp, "\t(char *) %s%dc_dev_name\t",
			   bs->config_name, bs->cmajor_assign);
			fprintf (ofp, "/* device filename array */\n");
			fprintf (ofp, "\t},\n");
		}
	    }
	    /* null entry at end of assign_table.  since config_name is
	     * a required field, it will never be a null string for a
	     * valid driver
	     */
	    fprintf (ofp, "\t{ 0 }\n");
	    fprintf (ofp, "};\n");
	    fprintf (ofp, "\nAssign_table\t*assign_ptr = assign_table;\n");
	}
	free (mn);
	free (dv);
}

/*
 * routine name:	mklocalconf()
 * purpose:		create a new conf.c in the ../NAME directory.
 *			This version of conf.c is a copy of the conf.c
 *			file found in io/common with the addition of
 *			3rd party driver entries to the bdevsw and
 *			cdevsw tables.
 * assumptions:		- run in obj/pmax/kernel/conf directory
 *			- "cookies" are in place at the beginning and end of 
 *			  the bdevsw and cdevsw tables.
 *			- all "tokens" are surrounded by white space
 * notes:		This routine should be rewritten to avoid use of
 *			get_char_buf() which uses obuf[].  By using
 *			get_char_buf2(), malloc would be used.  But, since
 *			mklocalconf() assumes a certain ordering of lines which
 *			get_char_buf() provides, it is too risky to change at
 *			this late date in the Silver development.
 */
mklocalconf()
{
char	*st;	/* status returned from fgets() */
int	begin_between, end_between;
		/* indexes into obuf[], used to record where the lines
		 * between the two devsw tables reside within the
		 * obuf[].
		 */
Entry	*bentry, *centry;
		/* Entry pointers used to search for or traverse
		 * Entry's in the devsw linked list of Entry's
		 */
New_devsw	*bs;
Bassign_table	*a;
Minor		*m;
int	assign_max;
int	max;
int	namelen;
int	i;

	/*
	 * build the output file name
	 * open the input and output files.
	 */
	sprintf (tbuf, "../%s/conf.c", PREFIX);
	if (! (ifp = fopen ("../io/common/conf.c", "r"))) {
		perror ("../io/common/conf.c");
		exit (1);
	}
	if (! (ofp = fopen (tbuf, "w"))) {
		perror (tbuf);
		exit (1);
	}

	/*
	 * blindly copy lines until first cookie is reached. 
	 */
	while ((st=fgets (tbuf, LINE_SIZE, ifp)) != 0) {
		if (*tbuf == '%') {
			if (strncmp (tbuf, "%%%", 3) == 0) {
				break;
			}
		} else {
			fprintf (ofp, "%s", tbuf);
		}
	}

	/*
	 * process the bdevsw table.  it will be saved in memory.
	 */
	assert (st != NULL);
	process_devsw ();

	/*
	 * blindly copy what is between the two tables.  keep this in
	 * memory for now.  it will be written out after the new
	 * entries to bdevsw and cdevsw are added.
	 */
	begin_between = oi;
	while ((st=fgets (tbuf, LINE_SIZE, ifp)) != 0) {
		if (*tbuf == '%') {
			if (strncmp (tbuf, "%%%", 3) == 0) {
				break;
			}
		} else {
			strcpy (get_char_buf(), tbuf);
		}
	}
	end_between = oi;

	/*
	 * process the cdevsw table.  it will be saved in memory.
	 */
	assert (st != NULL);
	process_devsw ();

	/*
	 * call process_stanza() to  read the NAME.list and the
	 * stanza.static files to get info required to build the new
	 * entries for bdevsw and/or cdevsw
	 */

	if (process_stanza ()) {
		assign_specific ();
		assign_random ();
	}

	/* at this point the Entry linked list has all the Entry's from
	 * and original conf.c and all the new Entry's which were
	 * required by the stanza.static file.  It is now time to print
	 * out the devsw tables, the stuff inbetween and the new
	 * Bassign_table
	 */

	/* first, print out the bdevsw table */
	print_devsw (bdev_head, BDEV_FLAG);

	/* now the "stuff" inbetween */
	for (i=begin_between; i<end_between; i++) {
		fprintf (ofp, "%s", obuf[i]);
	}

	/* next, print out the cdevsw table */
	print_devsw (cdev_head, CDEV_FLAG);

	/* and now, blindly copy what is left of conf.c
	 */
	while ((st=fgets (tbuf, LINE_SIZE, ifp)) != 0) {
		fprintf (ofp, "%s", tbuf);
	}

	/* all that's left now is to generate the Bassign_table for conf.c
	 */
	print_assign_table ();

	fclose (ifp);
	fclose (ofp);
}
