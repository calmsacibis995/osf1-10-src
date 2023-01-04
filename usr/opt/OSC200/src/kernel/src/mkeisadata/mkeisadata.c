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
static char *rcsid = "@(#)$RCSfile: mkeisadata.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/09 21:54:41 $";
#endif

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>	/* for toupper() -bg  */

struct adapt {
	char adapt_routine[132];
	struct adapt *next;
};

int adapt_flag = 0;
int pass = 0;
struct adapt *adapt_extern = 0;

 /* input and out file pointers for reading the eisa_option_data.c files */
 FILE	*ifp, *ofp;

 /*
  * declaring this here relieves the need to declare char * at every
  * invokation of strtok()
  */
 char	*strtok();

 char *cnfg_name;
 char *search_path;
 char *getenv();

/* Definitions */

#define	IDNAMELEN	7
#define	NAMELEN		8
#define	FUNCLEN		89
#define COMLEN		24
#define CNFGLEN		131

/*
 * routine name:	main
 * description:		main control routine, process environ variables
 * input:		none
 * output:		char *
 * assumptions:
 * notes:		
 *			
 *			
 *			
 */
main(argc, argv, envp)
	int argc;
	char **argv;
	char **envp;
{

	int retval;

	cnfg_name = getenv("CONFIG_NAME");
	if (cnfg_name == NULL) {
		printf("No configuration name\n");
		exit(1);
	}
        if(retval = mklocaleisa()) {
		mklocaleisa();
	}
	
}

/*
 * routine name:	process_file()
 * description:		Process the eisa_data file to build a new
 *			eisa_option_data.c entry.
 * input:
 * output:
 * assumptions:
 * notes:
 */
process_file(ofp)
FILE *ofp;
{

char	List[132];	/* buffer to build the NAME.list filename in */
FILE	*list_fp;	/* ptr to NAME.list file descriptor */
char	eisa_data[132];	/* buffer to build the eisa_data path and
			 * filename in
			 */
FILE	*eisa_data_fp;	/* ptr to the eisa_data file descriptor */
char	list_entry[1024];
			/* buffer for lines as read from NAME.list file */

char	copy[1024];	/* copy of list_entry so that strtok can be used */
char	*rtn;		/* return status from fgets() */
char	*pathname;	/* first token from list_entry is the pathname of the
			 * eisa_data file
			 */
int	i;

    /* open the NAME.list file, if it exists
     */
    sprintf (List, "./%s.list", cnfg_name);
    list_fp = fopen (List, "r");

    /* test to see if the NAME.list was successfully opened.  If it was, then
     * get the first line.
     */
    if (list_fp != 0) {
	rtn = fgets (list_entry, 1024, list_fp);
		
	/* loop as lines are read from NAME.list
	 */
	for ( ; rtn != (char *) 0; ) {

	    /* OK now we have a line from the cnfg_name.list file which
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

		/* build the path and filename, and open the file
		 */

		bzero(eisa_data, sizeof(eisa_data));
		(void) strncpy (eisa_data, pathname, strlen(pathname));
		(void) strcat (eisa_data, "/eisa_data");

		eisa_data_fp = fopen (eisa_data, "r");

		/* test to see if eisa_data was opened
		 */
		if (eisa_data_fp != NULL) {
			process_data(eisa_data_fp, ofp);
		} /* endif eisa_data.file open */
		bzero (eisa_data, strlen(eisa_data));
	    } /* endif test for blank lines */
	    rtn = fgets(list_entry,1024,list_fp);
	} /* end for loop that reads NAME.list */
    }
}

/*
 * routine name:	mklocaleisa()
 * purpose:		create a new eisa_option_data.c in the ../NAME directory.
 *			This version of eisa_option_data.c is a copy of the 
 *			eisa_option_data.c file found in data with the addition 
 *			of 3rd party driver entries.
 * assumptions:		- run in obj/pmax/kernel/conf directory
 *			- "cookie" in place in the eisa_option_data.c file
 *			- all "tokens" are surrounded by white space
 */
mklocaleisa()
{
char	*st;	/* status returned from fgets() */

char	tbuf[132];
int	retval = 0;

	/*
	 * build the output file name
	 * open the input and output files.
	 */
	sprintf (tbuf, "../%s/eisa_option_data.c", cnfg_name);
	if (! (ifp = fopen ("../data/eisa_option_data.c", "r"))) {
		perror ("../data/eisa_option_data.c");
		exit (1);
	}
	if (! (ofp = fopen (tbuf, "w"))) {
		perror (tbuf);
		exit (1);
	}

	/*
	 * blindly copy lines until cookie is reached. 
	 */
	while ((st=fgets (tbuf, 132, ifp)) != 0) {
		if (adapt_flag) {
			if (strncmp(tbuf, "struct	 eisa_option",19) == 0) {
				process_adapt_extern(ofp);
				adapt_flag = 0;
			}
		}
		if (*tbuf == '%') {
			if (strncmp (tbuf, "%%%", 3) == 0) {
				break;
			}
		} else {
			fprintf (ofp, "%s", tbuf);
		}
	}

        /*
	 * Process one more line to get the end of comment
	 */
        if((st=fgets (tbuf, 132, ifp)) != 0) {
		fprintf (ofp, "\tAutomatically added entries\n");
		fprintf (ofp, "%s", tbuf);
	}
	process_file(ofp);

        if(adapt_flag && !pass) {
		lseek(ofp, 0, SEEK_SET);
		fclose(ifp);
		retval = 1;
		pass = 1;
	}
	/*
	 * blindly copy rest of file
	 */
	while ((st=fgets (tbuf, 132, ifp)) != 0) {
		fprintf (ofp, "%s", tbuf);
	}
        fclose(ofp);
        return(retval);

}


/*
 * routine name:	process_adapt_extern()
 * description:		
 *			
 *			
 * input:
 * output:
 * assumptions:
 * notes:
 */
process_adapt_extern(ofp)
FILE *ofp;
{
struct adapt *adapt_ptr;

	adapt_ptr = adapt_extern;

	while(adapt_ptr) {
        /*
        **  if there is an adapter configuration function
        **  to declare as an external reference, do it -bg
        */
        if (strcmp(adapt_ptr, "0")) /* true if string is NOT "0" */
			fprintf(ofp, "extern\tint\t%s();\n", adapt_ptr);
		adapt_ptr = adapt_ptr->next;
	}
	fprintf(ofp, "\n");
}
/*
 * routine name:	process_data()
 * description:		
 *			
 *			
 * input:
 * output:
 * assumptions:
 * notes:
 */
process_data(ifp, ofp)
FILE *ifp, *ofp;
{
	char *rtn;
	char entry[132];
	char board_id[IDNAMELEN+1];
	char function[FUNCLEN+1];
	char comment[COMLEN+1];
	char drv_name[NAMELEN+1];
	char intr_b4[5];
	char intr_aft[5];
	char type[5];
	char adpt_cnfg[CNFGLEN+1];
	struct adapt *tptr;
	int i;
	int init_line = 0;

	intr_b4[0] = '0';
	intr_aft[0] = '0';
	type[0] = 'C';
	adpt_cnfg[0] = '0';
	adpt_cnfg[1] = '\0';
	board_id[0] = '\0';
	function[0] = '\0';
	comment[0] = '\0';
	rtn = fgets (entry, 132, ifp);

	/* loop as lines are read from eisa_data file
	 */
	for ( ; rtn != (char *) 0; init_line++ ) {
		if( strncmp(rtn, "#Entry", strlen("#Entry")) == 0)
		{
			/* If the first line is a new "Entry" skip this line
			 * and read the first real data line.  We can only get
			 * into this code on finding the string "#Entry" as the
			 * first 6 chars in the line.
			 */
			if(init_line == 0)
			{
				rtn = fgets (entry, 132, ifp);
				continue; /* bump for loop */
			} else {

			/* If a new entry mark is found, print out 
			 * the collected data
			 * from the last entry; skip this 
			 * new_entry line; and start
			 * processing this new entry's data.
			 */
				if(board_id[0] != '\0')
					fprintf(ofp, "    { \"%-7.7s\", \"%s\",\t\"%s\",\t%c,\t%c,   '%c',    %s},    /* %s */\n", 
						board_id,
						function,
						drv_name,
						intr_b4[0],
						intr_aft[0],
						type[0],
						adpt_cnfg,
						comment);
				/* Reset the default values */
				intr_b4[0] = '0';
				intr_aft[0] = '0';
				type[0] = 'C';
				adpt_cnfg[0] = '0';
				adpt_cnfg[1] = '\0';	
				board_id[0] = '\0';
				function[0] = '\0';
				comment[0] = '\0';
				/* Read next line */
				rtn = fgets (entry, 132, ifp);
				/* bump for loop */
				continue; 
			}
		}
		if(strncmp(entry, "BOARD_ID=", 9) == 0) {
			strncpy(board_id, &entry[9], IDNAMELEN);
		        for(i=0; i < IDNAMELEN ;i++) {
				if(board_id[i] == '\n' ||
				   board_id[i] == '\0') {
					board_id[i] = '\0';
					break;
				}
			}
			board_id[IDNAMELEN] = '\0';
		} else 
		if(strncmp(entry, "FUNCTION=", 9) == 0) {
			strncpy(function, &entry[9], FUNCLEN);
		        for(i=0;i < FUNCLEN ;i++) {
				if(function[i] == '\n' ||
				   function[i] == '\0') {
					function[i] = '\0';
					break;
				}
			}
			board_id[FUNCLEN] = '\0';
		} else 
		if(strncmp(entry, "DRV_NAME=", 9) == 0) {
			strncpy(drv_name, &entry[9], NAMELEN);
		        for(i=0;i < NAMELEN ;i++) {
				if(drv_name[i] == '\n' ||
				   drv_name[i] == '\0') {
					drv_name[i] = '\0';
					break;
				}
			}
			board_id[NAMELEN] = '\0';
		} else 
		if(strncmp(entry, "INTR_B4=", 8) == 0) {
			strncpy(intr_b4, &entry[8], 2);
		} else 
		if(strncmp(entry, "INTR_AFT=", 9) == 0) {
			strcpy(intr_aft, &entry[9], 2);
		} else 
		if(strncmp(entry, "TYPE=", 5) == 0) {
			strncpy(type, &entry[5], 2);
		} else 
		if(strncmp(entry, "COMMENT=", 8) == 0) {
			strncpy(comment, &entry[8], COMLEN);
		        for(i=0;i<15;i++) {
				if(comment[i] == '\n' ||
				   comment[i] == '\0') {
					comment[i] = '\0';
					break;
				}
			}
		} else 
		if(strncmp(entry, "ADPT_CNFG=", 10) == 0) {
			strncpy(adpt_cnfg, &entry[10], CNFGLEN);
		        for(i=0;;i++) {
				if(adpt_cnfg[i] == '\n' ||
				   adpt_cnfg[i] == '\0') {
					adpt_cnfg[i] = '\0';
					break;
				}
			}
            /*
            **  check to see whether this is an adapter, and
            **  whether there is a config function specified
            **  Setting the adapt_flag=1 forces a second pass 
	    **  to deal with generating external references 
	    **  for an adapter.
            */
			if ((toupper(*type) == 'A') && strcmp (adpt_cnfg, "0"))
				adapt_flag = 1;
			tptr = (struct adapt *) malloc(sizeof(struct adapt));
			if (tptr == (struct adapt *)NULL) {
				exit(1);
			}
			strcpy(tptr, adpt_cnfg);
			tptr->next = adapt_extern;
			adapt_extern = tptr;
		}

		rtn = fgets (entry, 132, ifp);

	}
	if(board_id[0] != '\0')
		fprintf(ofp, "    { \"%-7.7s\", \"%s\",\t\"%s\",\t%c,\t%c,   '%c',    %s},    /* %s */\n", 
			board_id,
			function,
			drv_name,
			intr_b4[0],
			intr_aft[0],
			type[0],
			adpt_cnfg,
			comment);

}


