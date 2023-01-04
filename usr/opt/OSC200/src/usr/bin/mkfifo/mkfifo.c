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
static char rcsid[] = "@(#)$RCSfile: mkfifo.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/10/11 20:00:45 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDPOSIX) new commands required by Posix 1003.2
 *
 * FUNCTIONS: mkfifo
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.1  com/cmd/posix/mkfifo.c, bos320 12/3/90 13:31:14
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/mode.h>
#include <sys/stat.h>

#include <nl_types.h>
#include "mkfifo_msg.h"

#define MODE 	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_MKFIFO,num,str) 

mode_t convertmode(char *ms);
int who(char **ms);
int what(char **ms);
int where(char **ms);
int usage(void);

extern char *optarg;
extern int  optind;


main(int argc, char **argv)
{
	int ch;			 /* option holder */
	int errflg = 0;		 /* error indicator */
	mode_t filemode;	 /* mode of fifo to be created */
	char *modestring = NULL; /* user supplied mode of fifo */
	char *msp;

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_MKFIFO, NL_CAT_LOCALE);

	if (argc < 2)
		usage();

	while((ch = getopt(argc, argv, "m:")) != EOF)
		switch(ch) {
		case 'm':		/* Specify mode */
			modestring = optarg;
			break;

		case '?':
			usage();
			break;
		}

	if (!argv[optind]) {
		fprintf(stderr, MSGSTR(NOFILE, "mkfifo: must specify file\n"));
		usage();
	}

        /*
	 * if a mode is supplied on the command line, convert
         * it to a numerical value.  Otherwise use prw-rw-rw-.
         */
	if ((modestring != (char *)NULL) && (*modestring != '\0'))  {
		filemode = strtol(modestring, &msp, 8);
		if (msp == modestring)
			filemode = convertmode(modestring);   
	}
	else 
		filemode = MODE;


	/* 
         * For each file argument, create a fifo.  If a failure 
	 * occurs, print error message and continue processing
         * files.
         */

	for (; argv[optind]; optind++) {

		if (mkfifo(argv[optind], filemode)) {
			fprintf(stderr,"mkfifo: ");
			perror(argv[optind]);
			errflg++;
		}
	}

	exit(errflg);
}




/*
 * NAME: convertmode
 *                                                                    
 * FUNCTION: convertmode converts modes in the format of [ugoa][+-=][rwxst]
 *	     to a numerical representation.
 */  
mode_t
convertmode(char *ms)
{
	int who_val;		/* result of who():   { ugoa } */
	int what_val;		/* result of what():  { +-= } */
	int where_val;		/* result of where(): { rwxst } */

	mode_t oldmode = MODE;
	
	do 
	{
		who_val = who(&ms);
		while (what_val = what(&ms)) 
		{
			where_val = where(&ms);
			switch (what_val) 
			{
			    case '+':
				oldmode |= where_val & who_val;
				break;
			    case '-':
				oldmode &= ~(where_val & who_val);
				break;
			    case '=':
				oldmode &= ~who_val;
				oldmode |= where_val & who_val;
				break;
			}
		}
	} while (*ms++ == ',');

	if (*--ms) 
	{
		fprintf(stderr, MSGSTR(EMODE, "mkfifo: invalid mode\n"));
		exit(2);
	}

	return(oldmode);
}



/*
 * NAME: who
 *
 * FUNCTION: determine whose modes are changed.
 */
int
who(char **ms)
{
	int m = 0;

	for (;;) 
		switch (*(*ms)++) 
		{
		    case 'u':
			m |= S_ISUID | S_ISVTX | S_IRWXU;  /* user's bits */
			continue;
		    case 'g':
			m |= S_ISGID | S_IRWXG;		   /* group's bits */
			continue;
		    case 'o':
			m |= S_IRWXO;  			   /* other's bits */
			continue;
		    case 'a':
			m |= S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO; /* all */
			continue;
		    default:
			(*ms)--;
			if (m == 0)
				m = S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO; /*all*/
			return(m);
		}
}



/*
 * NAME: what
 *                                                                    
 * FUNCTION: how are the modes being changed.
 */  
int
what(char **ms)
{
	switch (**ms) 
	{
	case '+':
	case '-':
	case '=':
		return (*(*ms)++);
	}
	return(0);
}


/*
 * NAME: where
 *                                                                    
 * FUNCTION: determine where the mode needs to be changed
 */  
int
where(char **ms)
{
	int	m = 0;


	for (;;) 
		switch (*(*ms)++) 
		{
		    case 'r':
			m |= S_IRUSR | S_IRGRP | S_IROTH;  /* read permit */
			continue;
		    case 'w':
			m |= S_IWUSR | S_IWGRP | S_IWOTH;  /* write permit */
			continue;
		    case 'x':
			m |= S_IXUSR | S_IXGRP | S_IXOTH;  /* exec permit */
			continue;
		    case 's':
			m |= S_ISUID | S_ISGID;		   /* set[ug]id */
			continue;
		    case 't':
			m |= S_ISVTX;		   	   /* sticky bit */
			continue;
		    default:
			(*ms)--;
			return(m);
		}
}


usage(void)
{
	fprintf(stderr, MSGSTR(USAGE, "Usage: mkfifo [-m mode] file ...\n"));
	exit(2);
}
