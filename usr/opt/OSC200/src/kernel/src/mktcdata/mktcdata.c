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
static char *rcsid = "@(#)$RCSfile: mktcdata.c,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/10/11 15:39:59 $";
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

 /* input and out file pointers for reading the tc_option_data.c files */
 FILE	*ifp, *ofp;

 /*
  * declaring this here relieves the need to declare char * at every
  * invokation of strtok()
  */
 char	*strtok();

 char *cnfg_name;
 char *search_path;
 char *getenv();


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
        if(retval = mklocaltc()) {
		mklocaltc();
	}
	
}

/*
 * routine name:	process_file()
 * description:		Process the tc_data file to build a new
 *			tc_option_data.c entry.
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
char	tc_data[132];	/* buffer to build the tc_data path and
			 * filename in
			 */
FILE	*tc_data_fp;	/* ptr to the tc_data file descriptor */
char	list_entry[1024];
			/* buffer for lines as read from NAME.list file */

char	copy[1024];	/* copy of list_entry so that strtok can be used */
char	*rtn;		/* return status from fgets() */
char	*pathname;	/* first token from list_entry is the pathname of the
			 * tc_data file
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

		bzero(tc_data, sizeof(tc_data));
		(void) strncpy (tc_data, pathname, strlen(pathname));
		(void) strcat (tc_data, "/tc_data");

		tc_data_fp = fopen (tc_data, "r");

		/* test to see if tc_data was opened
		 */
		if (tc_data_fp != NULL) {
			process_data(tc_data_fp, ofp);
		} /* endif tc_data.file open */
		bzero (tc_data, strlen(tc_data));
	    } /* endif test for blank lines */
	    rtn = fgets(list_entry,1024,list_fp);
	} /* end for loop that reads NAME.list */
    }
}

/*
 * routine name:	mklocaltc()
 * purpose:		create a new tc_option_data.c in the ../NAME directory.
 *			This version of tc_option_data.c is a copy of the 
 *			tc_option_data.c file found in data with the addition 
 *			of 3rd party driver entries.
 * assumptions:		- run in obj/pmax/kernel/conf directory
 *			- "cookie" in place in the tc_option_data.c file
 *			- all "tokens" are surrounded by white space
 */
mklocaltc()
{
char	*st;	/* status returned from fgets() */

char	tbuf[132];
int	retval = 0;

	/*
	 * build the output file name
	 * open the input and output files.
	 */
	sprintf (tbuf, "../%s/tc_option_data.c", cnfg_name);
	if (! (ifp = fopen ("../data/tc_option_data.c", "r"))) {
		perror ("../data/tc_option_data.c");
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
			if (strncmp(tbuf, "struct tc_option",16) == 0) {
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
	char rom_id[15];
	char drv_name[10];
	char intr_b4[5];
	char intr_aft[5];
	char type[5];
	char adpt_cnfg[132];
	struct adapt *tptr;
	int i;
	int init_line = 0;

	intr_b4[0] = '0';
	intr_aft[0] = '0';
	type[0] = 'C';
	adpt_cnfg[0] = '0';
	adpt_cnfg[1] = '\0';
	rom_id[0] = '\0';
	rtn = fgets (entry, 132, ifp);

	/* loop as lines are read from tc_data file
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
				if(rom_id[0] != '\0')
					fprintf(ofp, "    {\t\"%-8.8s\",\t\"%s\",\t%c,\t%c,\t'%c',\t%s},\n", 
						rom_id,
						drv_name,
						intr_b4[0],
						intr_aft[0],
						type[0],
						adpt_cnfg);
				/* Reset the default values */
				intr_b4[0] = '0';
				intr_aft[0] = '0';
				type[0] = 'C';
				adpt_cnfg[0] = '0';
				adpt_cnfg[1] = '\0';	
				rom_id[0] = '\0';
				/* Read next line */
				rtn = fgets (entry, 132, ifp);
				/* bump for loop */
				continue; 
			}
		}
		if(strncmp(entry, "ROM_ID=", 7) == 0) {
			strcpy(rom_id, &entry[7]);
		} else 
		if(strncmp(entry, "DRV_NAME=", 9) == 0) {
			strcpy(drv_name, &entry[9]);
		        for(i=0;;i++) {
				if(drv_name[i] == '\n' ||
				   drv_name[i] == '\0') {
					drv_name[i] = '\0';
					break;
				}
			}
		} else 
		if(strncmp(entry, "INTR_B4=", 8) == 0) {
			strcpy(intr_b4, &entry[8]);
		} else 
		if(strncmp(entry, "INTR_AFT=", 9) == 0) {
			strcpy(intr_aft, &entry[9]);
		} else 
		if(strncmp(entry, "TYPE=", 5) == 0) {
			strcpy(type, &entry[5]);
		} else 
		if(strncmp(entry, "ADPT_CNFG=", 10) == 0) {
			strcpy(adpt_cnfg, &entry[10]);
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
            **  (vs the ADPT_CNFG=0 called for if there is no config
            **  function by AA-PH3HA-TET1.ps, the "Guide to Writing
            **  TURBOchannel Device Drivers") Note that
            **  strcmp (adapt_cnfg, "0") is TRUE --i.e. non-zero--
            **  if adpt_config is NOT "0". Setting the adapt_flag=1
			**	forces a second pass to deal with generating 
			**	external references for an adapter. -bg
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
	if(rom_id[0] != '\0')
		fprintf(ofp, "    {\t\"%-8.8s\",\t\"%s\",\t%c,\t%c,\t'%c',\t%s},\n", 
			rom_id,
			drv_name,
			intr_b4[0],
			intr_aft[0],
			type[0],
			adpt_cnfg);

}


