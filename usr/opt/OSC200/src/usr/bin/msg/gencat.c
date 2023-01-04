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
static char rcsid[] = "@(#)$RCSfile: gencat.c,v $ $Revision: 4.3.11.7 $ (DEC) $Date: 1993/10/18 14:22:02 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: (CMDMSG) Message Catalogue Facilities
 *
 * FUNCTIONS: main, bump_msg, set_quote, set_message, store_msg, set_set,
 *            set_len, delset, get_text, write_msg, msg_comp, load_cat,
 *            open_source, rmalloc 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 *  1.28  com/cmd/msg/gencat.c, cmdmsg, bos320, 9130320k 7/24/91 15:17:37
 */
/*                                                                   
 * EXTERNAL PROCEDURES CALLED: standard library functions
 */

#include <stdio.h>
#include <stdlib.h>
#include "catio.h"
#include <sys/types.h>
#include <sys/errno.h>
#ifndef _BLD
#include <sys/mman.h>
#endif /* _BLD */
#include <fcntl.h>
#include <sys/file.h>
#include <ctype.h>
#include <langinfo.h>
#include <locale.h>
#include <nl_types.h>
#ifndef _BLD
#include "msgfac_msg.h" /* include file for message texts */
#endif

#define MAXMSG 32
#define isaoct(c) (c >= '0' && c <= '7')

#define MSG(n,s)	catgets(catderr, MS_GENCAT, n, s)

#ifdef _BLD
#   define iswblank(wc)		isspace(wc)
#else /* _BLD */
#   ifndef iswblank
#	define iswblank(wc)      iswctype(wc, wctype("blank")) /* not defined by X/Open */
#   endif /* iswblank */
#endif /* _BLD */

#define SLOP	20

/***************************************************************
 * defines for the 'bootstrap' (no NLS/MSG) build version
***************************************************************/
#ifdef _BLD
#define catgets(a,b,c,s)      s
extern int opterr;
#endif /* _BLD */
/*******************end if boot strap stuff***************/



wchar_t		quote;			/*---- current quote character ----*/
unsigned short	set = 0;		/*---- current set number  ----*/
unsigned short	msglen = NL_TEXTMAX;	/*---- current msglen ----*/
int		current = -1;		/*---- current _message index into 
                                               emsg[] ----*/
int 		msgmax  = 0;		/*---- current dimension of emsg[] ---*/
struct _message	*emsg;			/*---- array of _message structs 
                                               (holds all _messages --*/
nl_catd	catderr;			/* gencat error catalog descriptor */


void set_quote(char *line) ;
void get_message(char *line, FILE *file) ;
void store_msg(struct _message *msg) ;
void set_set(char *line) ;
void set_len(char *line) ;
void delset(char *line) ;
int get_text(char *source, char *target, FILE *file) ;
void write_msg(struct _message msg[], FILE *file) ;
int msg_comp(struct _message *a, struct _message *b) ;
void load_cat(char *tcat) ;
FILE *open_source(char *file) ;
void *rmalloc(int n) ;
char *skip_to_nwhite(char *p) ;
char *skip_to_white(char *p) ;



/*
 * NAME: main 
 *                                                                    
 * FUNCTION: Parses the arguments, reads the input stream and
 *           drives the rest of the program.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 0 on success, 1 on failure.
 *
 */  

main(int argc, char *argv[], char *envp[])
{
	char 	*target,		/*---- Target file name ----*/
		line[LINE_MAX+1+SLOP],	/*---- current line of text ----*/
 		*p; 			/*---- dummy string variable ----*/
	FILE	*sf,			/*---- source stream ----*/
		*tf;			/*---- target stream ----*/
	int file_no;
	int badopt;
	
/*______________________________________________________________________
	Check the input arguments, open the input and output files
  ______________________________________________________________________ */
#ifndef _BLD
	setlocale(LC_ALL,"");
	catderr = catopen(MF_MSGFAC, NL_CAT_LOCALE);
#endif

	opterr = badopt = 0;
	if (getopt(argc, argv, "") != EOF)
		badopt++;

	target = argv[optind++];

	if (badopt || target == NULL)	/*---- die if no target cat specified ----*/
		die( MSG(M_MSG_0, "Usage: gencat <target_cat> [descfile ...]") );

	if (argv[optind] == NULL || strcmp(argv[optind], "-") == 0) {
		sf = stdin;	/*---- read from stdin ----*/
		if(argv[optind] != NULL)
		 	 optind++;       /* QAR 15297 */
	} else {
		sf = (FILE *)open_source(argv[optind++]);
	}


	/*-- Load any existing catalog into memory,
	     unless stdout is specified.  --*/
	if (strcmp(target, "-") != 0)
		load_cat(target);
	
	do {

	    	fgets(line,LINE_MAX+1,sf); 

		while (!feof(sf)) {	/*- read through the input and 
                                            branch on any keywords -*/
			if (!memcmp(line,"",1))
				;
			else if (!memcmp(line,"\n",1))
				;
			else if (!memcmp(line,"$quote",strlen("$quote"))) {
				set_quote(line);
			}
			else if (!memcmp(line,"$delset",strlen("$delset"))){
				delset(line);
			}
			else if (!memcmp(line,"$set",strlen("$set"))) {
				set_set(line);
			}
			else if (!memcmp(line,"$len",strlen("$len"))) {
				set_len(line);
			}
			else if (!memcmp(line,"$ ",2) || !memcmp(line,"$\t",2)||
    				 !memcmp(line,"$\n",2))
				;	       /*----  check for comment  ---*/
			else {
				p = line;
				p = skip_to_nwhite(p);
				if (isdigit(*p)) {
					if (set == 0) {
						set = NL_SETD;
					}
					get_message(p,sf);		
				}
				else if (!memcmp(line," ",1))
					;
				else if (!memcmp(line,"\t",1))
					;
				else  { 
					fprintf(stderr, MSG(M_MSG_19, "gencat: Symbolic message identifier used:\n \t %s"),line);
					exit (1);
				}
			}
		        fgets(line,LINE_MAX+1,sf);
		}
	} while ( argv[optind] != NULL &&
		  (sf = (FILE *) ( strcmp(argv[optind], "-") ?
			open_source(argv[optind]) : stdin )) && ++optind);

	if (current == -1) 
		die(MSG(M_NOMSG, "No messages defined in source file."));
	if (strcmp(target, "-") == 0) {
		tf = stdout;
	} else {
		tf = fopen(target,"w");
	}
	if (!tf)
		die(MSG(M_MSG_1, "gencat: Unable to open target file."));
	write_msg(emsg,tf);	/*---- Write the output ----*/

	fclose(sf);
	fclose(tf);

	exit(0);
}

/*
 * NAME: bump_msg
 *
 * FUNCTION: Increments the current _message pointer.
 *           Checks for room in emsg. If there is not enough, it 
 *           calls realloc().
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS:  void
 */


void bump_msg()	/*----  incements the current _message pointer, 
			checks for room in emsg[], if there 
			is not enough it will realloc() ----*/
{
	extern struct _message 	*emsg;
	extern int 		current;
	extern int 		msgmax;

	if (current >= msgmax - 2) {
		register int i;

		if (msgmax > 0) {	/*---- if emsg exists ----*/
			msgmax += MAXMSG;
			if (!(emsg = (struct _message *)realloc(emsg,msgmax * 
                            sizeof (struct _message))))
				die( MSG(M_MSG_2, "gencat:  Unable to realloc().") );
		}
		else {			/*---- if this is the first time ----*/
			msgmax += MAXMSG;
			if (!(emsg = rmalloc(msgmax *
                            sizeof (struct _message))))
				die( MSG(M_MSG_2, "gencat:  Unable to realloc().") );
		}
		for (i = current + 1 ; i < msgmax ; i++) {
 			/*-- set up the new _messages --*/
			emsg[i]._text = FALSE;
			emsg[i]._set = emsg[i]._msg = emsg[i]._old = FALSE;
		}
	}
	current++;	/*---- bump current ----*/
}

/*
 * NAME: set_quote
 *
 * FUNCTION: Reset the current quote character.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void set_quote(char *line)
	/*---- line: input line (must include a $quote ----*/

{
	int	len;		/* length of quote character */

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	len = mbtowc(&quote, line, strlen(line));
	if (len < 0)
		die( MSG(M_MSG_EILSEQ, "gencat: Invalid character encountered.") );
	else if (len == 0)
		quote = 0;
	else if (len == 1)
		quote = (*line == '\n') ? 0 : (unsigned char)*line;
}

/*
 * NAME: get_message()
 *
 * FUNCTION: Gets the _message starting on the current line
 *           and store the resulting _message structure in emsg[].
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void get_message(char *line, FILE *file)

	/*---- line: Line the where the _message begins ----*/
	/*---- file: File it came from (in case of a continuation) ----*/

{
	char 			ttxt[NL_TEXTMAX+SLOP]; /* place to store the text */
	struct _message 	msg;		  /* _message we are getting */
	static struct _message	omsg;		  /* old _message(order check)*/
	static char		started = 'N';	  /* is there an old _message?*/
	int  			i,j;

	sscanf(line,"%u",&i);
	if (i < 1 || i > NL_MSGMAX) {
		fprintf(stderr, MSG(M_MSG_19, "gencat: Symbolic message identifier used:\n \t %s"),line);
		exit (1);
	}
	msg._msg = i;
	if (get_text(line,ttxt,file) == -1)  {
	 	for (i=0; i<=current && emsg[i]._text; i++) {
			if (emsg[i]._set == set && emsg[i]._msg == msg._msg) {
				free(emsg[i]._text);
				for (j=i; j<current; j++) {
					emsg[j] = emsg[j+1];
				}
				emsg[j]._text = FALSE;
				current--;
				break;
			}
		}
		return;
	}
	if (strlen(ttxt) > msglen)  {
		fprintf (stderr, MSG(M_MSG_3,
			"gencat: Message text is longer than $len value.\n \t %s\n"),line);
		exit (1);
	}
 	msg._text = rmalloc(strlen(ttxt) + 1);
	strcpy(msg._text,ttxt);
	msg._set = set;

	if (started == 'Y' && msg_comp(&msg,&omsg) <= 0) {
		fprintf(stderr, MSG(M_ORDER,
                        "gencat:  The message numbers/sets became out of \
order just after:\n msg:  %d,  set %d\n %s"), omsg._msg,omsg._set, omsg._text);
		exit(1);
	}
	omsg = msg;
	started = 'Y';
	store_msg(&msg);	/*---- Store the _message (used to replace 
                                       old ones) ----*/
}

/*
 * NAME: store_msg
 *
 * FUNCTION: Insterts a _message into emsg[]. Overwrites an existing
 *           _message if the catalog is being updated and there is a 
 *           duplicate _message in the old version of the catalog.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void store_msg(struct _message *msg)

	/*---- mesg: _message to be inserted in emsg ----*/

{
	extern struct _message 	*emsg;
	extern int		current,
				msgmax;
	int 	i;

/*______________________________________________________________________
	Search to see if there is a duplicate in the old _messages
  ______________________________________________________________________*/

	for (i = 0 ; i <= current ; i++) {
		if (!msg_comp(msg,&emsg[i]))
			break;
	}
	if (i <= current) {
		emsg[i] = *msg;		/* If there is an old one, replace it */
	}
	else {
		bump_msg();		/*---- else add a new one ----*/
		emsg[current] = *msg;
	}
}
			

/*
 * NAME: set_set
 *
 * FUNCTION: Sets the current set number and stores the value in the
 *           global variable 'set'.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void set_set(char *line)

	/*---- line: line with $set n command  ----*/

{
	int n;

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	if (!isdigit(line[0])) {
		fprintf(stderr, MSG(M_MSG_18, "gencat: Symbolic set identifier used. \n \t %s \n"),line);
	        exit (1);
	}
	sscanf(line,"%d",&n);
	if (n < SETMIN || n > SETMAX) {
		fprintf(stderr, MSG(M_MSG_5, "gencat:  Invalid set number. \n \t %s\n"),line);
 		exit (1);
	}
	set = (unsigned short)n;
}



/*
 * NAME: set_len
 *
 * FUNCTION: Sets the current len number and stores the value in the
 *           global variable 'msglen'.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */



void set_len(char *line)

	/*---- line: line with $len n command  ----*/

{
	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	
        if (isdigit(*line))
            sscanf(line,"%hu",&msglen);
        else
            msglen = NL_TEXTMAX;

}


/*
 * NAME: delset
 *
 * FUNCTION: Delete an existing set of _messages from emsg[].
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void delset(char *line)

 	/*----line: line with $delset n command	        ----*/

{
	extern struct _message 	*emsg;
	extern int		current,
				msgmax;
	unsigned short 		dset;	/*---- set to be deleted ----*/
	int			i;	/*- Misc counter(s) used for loops -*/
	int			n;

	line = skip_to_white(line);
	line = skip_to_nwhite(line);
	sscanf(line,"%d",&n);	/*---- get set to be removed ----*/
	if (n < SETMIN || n > SETMAX) {
                fprintf(stderr, MSG(M_MSG_5, "gencat  Invalid set number. \n \t %s\n"),line);
                exit (1);
        }
	dset = (unsigned short)n;


/*______________________________________________________________________
	Shuffle the _messages to delete any existing sets
  ______________________________________________________________________*/

	for (i = 0 ; i <= current && emsg[i]._text ; i++) {
		if (emsg[i]._set == dset) {
			int j;
			free(emsg[i]._text);
			for (j = i ; j < current ; j++) {
				emsg[j] = emsg[j + 1];
			}
			emsg[j]._text = FALSE;
			current--;
			i--;
		}	
	}
}
	

		
/*
 * NAME: get_text
 *
 * FUNCTION: Assembles a string of _message text which has been stored in the 
 *           gencat (._msg file) format.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 0 if  successful.
 *	    -1 if source is an empty string which has to be deleted from
 *          the soruce.
 */

int get_text(char *source, char *target, FILE *file)

	/*---- source: source string ----*/
	/*---- target: target string ----*/
	/*---- file: file (used for multi-line _messages) ----*/

{
	char 	quoted = FALSE,
		*base,
		*next,
		*targetbase;
	int j;
	int	len;
	wchar_t	wc;

	base = source;
	targetbase = target;
	source = skip_to_white(source);
	next = source;
	next = skip_to_nwhite(next);
	len = mbtowc(&wc, next, strlen(next));
	if (len < 0) {
		die( MSG(M_MSG_EILSEQ, "gencat: Invalid character encountered.") );
	}
	if (wc == quote) {
		quoted = TRUE;
		source = next + len;
	}
	else if (*source == ' ' || *source == '\t')
		source++;
	else if (*source == '\n')     /* source is an empty string */
		return(-1);
	while (*source && target - targetbase <= NL_TEXTMAX) {
		len = mbtowc(&wc, source, strlen(source));
		if (len < 0) {
			die( MSG(M_MSG_EILSEQ, "gencat: Invalid character encountered.") );
		}
		if (wc == '\\') {	/*---- Process backslash codes ----*/
			source++;
			if (isaoct(*source)) {	/*---- Octal number ----*/
				int octal;
				sscanf(source,"%3o",&octal);
				*target++ = octal;
				for (octal = 0 ; octal < 3 && isaoct(*(source +
                                     octal)) ; octal++)
					;
				source += octal;
			}
			else {
				switch (*source) {
					case 'n': {
						*target++ = '\n';
						source++;
						break;
					}
					case 't': {	/*---- tab ----*/
						*target++ = '\t';
						source++;
						break;
					}
					case 'r': {	/*---- return ----*/
						*target++ = '\r';
						source++;
						break;
					}
					case 'b': {	/*---- backspace ----*/
						*target++ = '\b';
						source++;
						break;
					}
					case 'f': {	/*---- form feed ----*/
						*target++ = '\f';
						source++;
						break;
					}
					case 'v': {	/*--- vertical tab ---*/
						*target++ = '\v';
						source++;
						break;
					}
					case 'x': {	/*--- hex number (two 
                                                         or four digits) ---*/
						int 	hex,
							hexlen = 0;
						source++;
						while (isxdigit(*(source +
                                                       hexlen)) && (hexlen < 4))
							hexlen++;
						switch (hexlen) {
						case 4:
							sscanf(source,"%2x",
                                                               &hex);
							*target++ = hex;
							source += 2;
							/* FALLTHROUGH */
						case 2:
							sscanf(source,"%2x",
                                                               &hex);
							*target++ = hex;
							source += 2;
							break;
						default:
						 	fprintf(stderr,
MSG(M_MSG_7, "Bad hex len (the length of a hex number must be either two or four digits.)\n \t %s\n"),base);
							exit (1);
						}
						break;
					}
					case '\n': {	/*-- continuation --*/
						source = base;
						fgets(source,NL_TEXTMAX,file);
						break;
					}
					default: {
						len = mblen(source, MB_CUR_MAX);
						if (len < 0)
							len = 1;
						do
							*target++ = *source++;
						while (--len > 0);
					}
				}
			}
		}
		else if (quoted && wc == quote) 
			break;
		else if (wc == '\n')
			if (quoted) {
				fprintf(stderr, MSG(M_MSG_8, "gencat:  Unexpected newline within quotes. \n \t %s \n"),base);
				exit (1);
			}
			else
				break; 
		else {
			do
				*target++ = *source++;
			while (--len > 0);
		}
		if (!(target - targetbase <= NL_TEXTMAX)) {
			fprintf (stderr, MSG(M_MSG_9, "gencat:  Message string longer than NL_TEXTMAX.\n \t %s \n"),base);
			exit (1);
		}
	}
	if (!*source) { 
		fprintf(stderr, MSG(M_MSG_10, "gencat: Unexpected end of string, (no newline or end of quotes) \n \t %s\n"),base);
		exit (1);
	}
	*target = '\0';
	return (0);
}


/*
 * NAME: write_msg
 *
 * FUNCTION:  Converts emsg[] into a format suitable for fast access and write
 *            the result to the target file (file).
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void write_msg(struct _message msg[], FILE *file)

	/*---- mesg: _message array to be written 
	      (this is actually equal to emsg[]) ---*/
	/*---- file: File to write msg[] to ----*/

{
	unsigned short	i,j;		/*---- Misc counter(s) used for loops */
	int 		total_sets;	/*---- total sets used ----*/
	int 		total_msgs;	/*---- total _messages in msg[] ----*/
	int 		header_size = sizeof(struct _header);
     					/*---- header size ----*/
	int 		msg_offset;	/*---- place to write the text of the
   					       next  _message ----*/
	struct _msgptr 	mp;		/*---- structure used to accellerate 
     				  	       the _message retrieval ----*/
	struct _header	hd;		/*---- _header record of the .cat file*/
	char		*codeset;	/*---- name of present codeset ----*/

/*______________________________________________________________________
	Use qsort to sort msg[] by _message within set
  ______________________________________________________________________*/

	for (i = 0 ; msg[i]._text ; i++) 
		;
	qsort ((void *)msg,(size_t)i, sizeof(struct _message),(int (*)())msg_comp); 

/*______________________________________________________________________
	Set up:
		total_sets,
		total_msgs,
		setmax
  ______________________________________________________________________*/

	for (i = 0 , total_sets = 0 , hd._setmax = 0 ; msg[i]._text ; i++) {
		if (!i || msg[i]._set != msg[i - 1]._set)
			total_sets++;
		if (msg[i]._set > hd._setmax)
			hd._setmax = msg[i]._set;
	}

	total_msgs = i;

	msg_offset = total_msgs * sizeof(struct _msgptr) + 	
	 	     /*- base of the _message text -*/
	 	     sizeof(struct _header) + 
		     total_sets * 2 * sizeof(unsigned short);
	hd._magic = CAT_MAGIC;
	hd._n_sets = total_sets;
	
	/*---- mark catalog with name of present codeset ----*/
#ifdef _BLD
	codeset = "ASCII";
#else  /* _BLD */
	codeset = nl_langinfo(CODESET);
	if (!codeset) codeset = "Unnamed";
#endif /* _BLD */
	strncpy(hd._filler, codeset, sizeof(hd._filler));
	hd._filler[sizeof(hd._filler)-1] = '\0';

	fwrite(&hd,header_size,1,file);	
	/*---- write the header to the file ----*/

	for (i = 0 ; i < total_msgs ; i++) {
	/*---- write the index table to the file ----*/

		if (!i || msg[i]._set != msg[i - 1]._set) {
		/*---- when the set changes ----*/

			fwrite(&msg[i]._set, sizeof(msg[i]._set), 1, file);	
			/*---- set number ----*/

			for (j = 0 ; j + i < total_msgs ; j++) {
				if (msg[i + j]._set != msg[i]._set) 
					break;
			}

			fwrite(&j, sizeof(short),1,file);	
			/*---- number of _messages ----*/
		}
		mp._msgno = msg[i]._msg;	
                /*---- write an 'mp' for each _message -----*/

		mp._msglen = strlen(msg[i]._text);
		mp._offset = msg_offset;
		fwrite(&mp,sizeof(mp),1,file);
		msg_offset += mp._msglen + 1;
	}
	if(file != stdout)  /* can't do this check if writing to stdout */
	       if (ftell(file) != total_msgs * sizeof(struct _msgptr) + 
				sizeof(struct _header) + 
				total_sets * 2 * sizeof(unsigned short))
		die( MSG(M_MSG_11, "gencat:  internal error.") );
                /*---- file pointer consistancey check ----*/

	for (i = 0 ; i < total_msgs ; i++) {
		fwrite(msg[i]._text,strlen(msg[i]._text) + 1,1,file);
	}
	if(file != stdout)  /* can't do this check if writing to stdout */
	        if (ftell(file) != msg_offset)
		        die( MSG(M_MSG_12, "gencat: internal error (bad file position)") );
                     /*---- file pointer consistancey check  ----*/
}

/*
 * NAME: msg_comp
 *
 * FUNCTION: Compare _message structures and return a value which is
 *           approprite for qsort.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: 	a > b : 1
 *		a < b : -1
 *		a = b : 0
 */

int msg_comp(struct _message *a, struct _message *b)

	/*---- msg_comp: used by qsort and get_msg ----*/
	/*---- a,b: the two _messages to be compared ----*/

{
	if (a->_set != b->_set)
		return(a->_set - b->_set);
	else 
		return(a->_msg - b->_msg);
}




/*
 * NAME: load_cat
 *
 * FUNCTION: Uses catopen to open a .cat file. Reformats the catd 
 *           structure into the emsg array, and closes the .cat file.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: void
 */

void load_cat(char *tcat)

	/*---- tcat: catalog name to be loaded ----*/

{
#ifdef _BLD
        return;
#else
	nl_catd catd;		/*---- catalog descriptor ----*/
	struct _msgptr mpt;	/*---- catd style _message pointer ----*/
	int i,j;		/*---- Misc counter(s) used for loops ----*/
	char cat[PATH_MAX+1];

	if (strchr(tcat,'/')) {
		strcpy(cat,tcat);
	}
	else {
		sprintf(cat,"./%s",tcat);
	}
	

	if (access(cat,R_OK))
		return;

	catd = catopen(cat , NL_CAT_LOCALE);

	catgets(catd, 0, 0, "");	/* Kick it to cause deferred open to happen */

	if (catd == CATD_ERR || catd->_fd == FILE_UNUSED) {
		fprintf(stderr, MSG(M_MSG_13, "Unable to load specified catalog. \n \t %s \n"),tcat);
		exit (1);
 	             /*---- target cat exists, but is not a real cat ----*/
	}

/*______________________________________________________________________
	Reorder the catd structures into the emsg style[] structure
	while expanding emsg as needed.
  ______________________________________________________________________*/

	for (i = 0 ; i <= catd->_n_sets ; i++ ) {
		if (catd->_set[i]._mp == NULL)
			continue;		/* skip empty sets */
		for (j = 0 ; j <= catd->_set[i]._n_msgs ; j++) {
			if (catd->_set[i]._mp[j]._offset) {
				mpt = catd->_set[i]._mp[j];
				bump_msg();
				emsg[current]._text = rmalloc(mpt._msglen + 1);
				strncpy( emsg[current]._text,
					catd->_mem+mpt._offset, mpt._msglen+1);
				emsg[current]._set = catd->_set[i]._setno;
				emsg[current]._msg = mpt._msgno;
				emsg[current]._old = TRUE;
			}
		}
	}

	/*
	 * Before we close the catalog, we want to invalidate any resident pages
	 * because we're about to overwrite them via write(2).  This is defense
	 * against a suspected VM bug
	 */

/* msync not working on DEC OSF/1 t1.2-2 (Rev. 5) */
#ifndef __alpha
	msync((caddr_t) catd->_hd, catd->_catlen, MS_INVALIDATE|MS_SYNC);
#endif

	catclose(catd);
#endif /* _BLD */
}



/*
 * NAME: open_source
 *
 * FUNCTION: Opens a source stream.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the source stream.
 */

FILE *open_source(char *file)

{
	FILE *f;

	if (!(f = fopen(file,"r"))) {
		fprintf(stderr, MSG(M_MSG_1,
                        "Gencat: Unable to open %s\n") ,file);
		exit(1);
	}
	return(f);
}


/*
 * NAME: rmalloc
 *
 * FUNCTION: Performs a malloc with some error checking.
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the result of the malloc.
 */ 

void *
rmalloc(int n)
	/*----  n: the number of bytes to be malloc'ed  ----*/
{
	void *t;

	t =   malloc(n);
	if (!t)
		die( MSG(M_MSG_15, "Unable to malloc memory.") );
	return(t);
}


/*
 * NAME: skip_to_white
 *
 * FUNCTION: Locate the next character less than or equal to a blank
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the next blank (or lesser) character
 */ 

char *skip_to_white(char *p)
{
	int	len;		/* # bytes in next character */
	wchar_t	wc;		/* process code of next character */

	while (*p) {
		len = mbtowc(&wc, p, strlen(p));
		if (len < 0) {
			die( MSG(M_MSG_EILSEQ, "gencat: Invalid character encountered.") );
		}
		if (wc <= ' ')
			return (p);
		p += len;
	}
	return (p);
}


/*
 * NAME: skip_to_nwhite
 *
 * FUNCTION: Locate the next "nonblank" character
 *
 * EXECUTION ENVIRONMENT:
 * 	User mode.
 *
 * RETURNS: A pointer to the next nonblank character
 */ 


char *skip_to_nwhite(char *p)
{
	int	len;		/* # bytes in next character */
	wchar_t	wc;		/* process code of next character */

	while (*p) {
		len = mbtowc(&wc, p, strlen(p));
		if (len < 0) {
			die( MSG(M_MSG_EILSEQ, "gencat: Invalid character encountered.") );
		}
		if (iswblank(wc) == 0)
			return (p);
		p += len;
	}
	return (p);
}



