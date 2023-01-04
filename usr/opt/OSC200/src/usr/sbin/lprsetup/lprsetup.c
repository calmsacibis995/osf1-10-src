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
static char *sccsid  =  "@(#)$RCSfile: lprsetup.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/08 16:48:02 $";
#endif

/*
 *
 *      OSF/1 Printer Installation/Setup  Program
 *
 *      This program helps system administrators to set up
 *      printers for their system.  It guides through the
 *      steps in setting up /etc/printcap, makes all of the
 *      necessary files and directories for each printer,
 *      and insures everything necessary for successful
 *      printer operation was specified.
 *
 */
/*
 * based on lprsetup program developed by John Dustin for ULTRIX-11
 * and as modified by the following engineers:
 *  John Painter
 *  Mike Ardehali
 *  Adrian Thoms
 *  Daren Seymour 
 *  David Gray 
 *  chetal, Pradeep
 *  Dave Maxwell 
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <dec/lat/lat.h>
#include "lprsetup_msg.h" /* must be before other 'local' includes */
#include "lprsetup.h"
#include "globals.h"

time_t time();
extern int errno;
extern void perror();
static int MakeSpool(struct passwd *passwd);
static int MakeAcct(struct passwd *passwd);
static int MakeErrorlog(struct passwd *passwd);
static void initcmds();
static void UnLinkSpool(struct table *tabent, char *oldpath);
static void UnLinkFile(struct table *tabent, char *oldpath);
void UnLinkSymFile (char *symbol);
void UnLinkSpooler();

/*
 *  prototypes of routines referenced
 */
int pgetent(char *capbuf_ptr, char *name, int use_data);/* get an entry  */
int pnchktc(int use_data);                 /* check for indirection      */
int pnamatch(char *nam_ptr, int use_data); /* match an entries name      */
int pgetnum(char *id, int use_data);       /* get a numeric entrys value */
int pgetflag(char *id, int use_data);      /* get a boolean flags value  */
char *pgetstr(char *id, char **area, int use_data);/* get entrys string  */

/* debug */
int debug = 0;

/* 'local' globals, these get reused when updating default values too */
static char *pd_help;
static char helpbuf[BUF_HELP];
static char *helpbuf_ptr;

main (int argc, char *argv)
{
    int i;
    FILE *pfp;

    /* Open the internationalization message file */
    catd = catopen(MF_LPRSETUP,NL_CAT_LOCALE);

    if (argc != 1)
    { /*  no command line arguments are accepted so print the
       *  usage message if any are given
       */
        fprintf (stderr, MSGSTR(LPRSETUP_0,"\nusage: %s\n"), progname);
        exit (1);
    }

/*
 *  check prerequisites (allow for local debugging by
 *  by redifing ROOTUSER in lprsetup.h)
 */
    if (getuid () != 0 && !ROOTUSER)
    {
#ifndef LOCAL
        printf (MSGSTR(LPRSETUP_1,"\n%s: must be superuser!\n\n"), progname);
        leave (ERROR);
#endif
    }

    /* for Popen later (in misc.c) */
    catclose(catd); /* just because we need to keep the file id 'hidden' */
    for (i = 3; i < 20; i++)
        close(i);
    dup2(1, 3);

    /* Open the internationalization message file again (we just closed it) */
    catd = catopen(MF_LPRSETUP,NL_CAT_LOCALE);

    if ((pfp = fopen (PRINTCAP, "a+")) == NULL)
    {/* we could not open the file or create it if non-existant, so leave */
        perror (PRINTCAP);
        leave (ERROR);
    }
    else
    {/* if we could open/create the file, close it, then open it for read */
        close (pfp);
        if ((pfp = fopen (PRINTCAP, "r")) == NULL)
        {/* just to be sure, check access again and fail if we can't open ro */
            perror (PRINTCAP);
            leave (ERROR);
        }
    }

    printf (MSGSTR(LPRSETUP_2,"\nDigital OSF/1 Printer Setup Program\n"));

    /* initialize the command table */
    initcmds();

    /*
     * loop until done
     */
    for (;;)
    {
        /* clear changable table values */
        for (i = 0; tab[i].name != 0; ++i)
        {
            tab[i].used = NO;
            tab[i].nvalue = 0;
        }
        /*
         * clear modify flag after each time
         * through, used to keep track of when to
         * link the filter(s).
         */
        modifying = FALSE;

        printf (MSGSTR(COMMANDS_0,
                "\nCommand  < add modify delete exit view quit help >: "));

        strcpy(pname, "");
        strcpy(pnum, "");

        switch (getcmd ())
        {
          case ADD:
            DoAdd ();
            break;
          case MODIFY:
            DoModify ();
            break;
          case DELETE:
            DoDelete ();
            break;
          case QUIT:
            printf("\n");
            leave (OK);
            break;
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_0A,h_help),TRUE);
            PrintHelp (MSGSTR(LPRSETUP_HELP_0B,""),FALSE);
            PrintHelp (MSGSTR(LPRSETUP_HELP_0C,""),FALSE);
            break;
          case VIEW:
            ViewPrintcap ();
          case NOREPLY:
            break;
          default:
            printf (MSGSTR(LPRSETUP_6,
                    "\nSorry, invalid choice.  Type '?' for help.\n"));
            break;
        }
        freemem ();
    }
}

/*
 *  add new entry to printcap and create
 *  corresponding special device
 */
DoAdd ()
{
    int     done;
    int     status, printerNumber;

    /* Starting at 0 find the lowest printer number in
     * the printcap data base, this number will be used
     * for default tty's and standard file names. If
     * the number is 0, and is selected by the user then
     * the printer is assumed to be the default printer.
     */
    
    printerNumber = 0;
    printerNumber = GetLowestNumber (printerNumber);
    if (printerNumber == 0) {
        /* Check to see if "lp" currently exists */
        status = pgetent (capbuf_ptr, "lp",TRUE);
        if (status == -1)
            badfile (PRINTCAP);
        else if (status == 1)  /* it does exist */
            /* get next lowest number */
            printerNumber = GetLowestNumber (printerNumber + 1);
    }
    (void) sprintf (pnum, "%d", printerNumber);
    
    printf(MSGSTR(LPRSETUP_3,"\nAdding printer entry, type '?' for help.\n"));
 
    done = 0;
    while (! done) 
    {
        printf (MSGSTR(LPRSETUP_4,"\nEnter printer name to add [%s] : "), pnum);
        switch (getcmd ())
        {
          case NOREPLY:
            strcpy(symbolname, pnum); /* he entered the default printer # */
            /* no break! ...falls through to case GOT_SYMBOL: */
          case GOT_SYMBOL:
            status = pgetent (capbuf_ptr, symbolname,TRUE);
            if (status == -1) 
            {
                badfile(PRINTCAP);
            }
            if (status == 1)
                printf(MSGSTR(LPRSETUP_5,
                       "\nSorry, printer '%s' already exists.\n"), symbolname);
            else
                done = TRUE;
            break;
          case QUIT:
            return (QUIT);
            break;
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_1A,h_doadd),TRUE);
            PrintHelp (MSGSTR(LPRSETUP_HELP_1B,""),FALSE);
            break;
          default:
            printf (MSGSTR(LPRSETUP_6,"\nInvalid choice.  Type '?' for help.\n"));
            break;
        }
    }

    strcpy(pname, "");

    if (strcmp (pnum, symbolname) == 0) 
    {
        if (strcmp (pnum, "0") == 0)
            (void) sprintf (pname, "lp|lp0|0");
        else
            (void) sprintf (pname, "lp%s|%s", pnum, pnum);
    }
    else 
    { /* not default number */
        (void) sprintf(pname, "%s|lp%s", symbolname, pnum);
    }

    if (AddField () != QUIT)
    {
        AddComments ();
        if (AddEntry () == ERROR)
            printf(MSGSTR(LPRSETUP_7,
                   "\nError in adding printcap entry, try again.\n"));
        else if (AddDevice () == ERROR)
            printf(MSGSTR(LPRSETUP_8,
                   "\nError in associating printer files/directories.\n"));
    }
    freemem ();
    printf("\n");
    return (OK);
}

/*
 *  modify existing printcap entry
 */
DoModify ()
{
    int     done = FALSE;
    int     status;

    printf(MSGSTR(LPRSETUP_9,
           "\nModifying a printer entry, type '?' for help.\n"));
    while (!done)
    {
        modifying = TRUE;

        strcpy(pnum, "");
        strcpy(pname, "");
        strcpy(longname, "");   /* in case of previous modify */

        printf (MSGSTR(LPRSETUP_10,
          "\nEnter printer name to modify (or view to view printcap file): "));

        switch (getcmd ())
        {
          case GOT_SYMBOL:
            status = pgetent (capbuf_ptr, symbolname,TRUE);
            if (status == -1) 
            {
                badfile(PRINTCAP);
            }
            if (status == 0)
            {
                printf (MSGSTR(LPRSETUP_11,
                        "\nSorry, printer number '%s' is not in %s.\n"), 
                        symbolname, PRINTCAP);
            }
            else
            {
                strcpy(pnum, symbolname);
                strcpy(pname, symbolname);
                ModifyEntry();
                done = TRUE;    /* get back to main menu */
            }
            break;
          case VIEW:
            ViewPrintcap ();
            break;
          case QUIT:
            done = TRUE;
            break;
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_3,h_domod),TRUE);
            break;
          case NOREPLY:
            break;
          default:
            printf (MSGSTR(LPRSETUP_6,"\nInvalid choice, try again.\n"));
            break;
        }
    }
    freemem();
    return (OK);
}

/*
 *  delete existing printcap entry
 */
DoDelete ()
{
    int     done = FALSE;
    int     status;
    int yn;

    printf (MSGSTR(LPRSETUP_12,
            "\nDeleting a printer entry, type '?' for help.\n"));
    while (!done)
    {
        strcpy(pnum, "");
        strcpy(pname, "");

        printf (MSGSTR(LPRSETUP_13,
          "\nEnter printer name to delete (or view to view printcap file): "));

        switch (getcmd ())
        {
          case GOT_SYMBOL:
            status = pgetent (capbuf_ptr, symbolname,TRUE);
            if (status == -1) 
            {
                badfile(PRINTCAP);
            }
            if (status == 0)
            {
                printf (MSGSTR(LPRSETUP_14,
                        "\nCannot delete printer %s, entry not found.\n"), 
                        symbolname);
            }
            else
            {
                strcpy(pnum, symbolname);
                (void) sprintf(pname, "%s", pnum);
                done = TRUE;
            }
            break;
          case VIEW:
            ViewPrintcap ();
            break;
          case QUIT:
            return (QUIT);
            break;
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_2,h_dodel),TRUE);
            break;
          case NOREPLY:
            break;
          default:
            printf (MSGSTR(LPRSETUP_6,"\nInvalid choice, try again.\n"));
        }
    }

    /* read printcap into tab for final confirmation */
    CopyEntry ();

    Print (USED);
    printf (MSGSTR(LPRSETUP_15,"\nDelete %s, are you sure? [n]  "), pname);

    yn = no_char;
    if (YesNo (yn) == TRUE) 
    {
        DeleteEntry ();
    }
    else 
    {
        printf (MSGSTR(LPRSETUP_16,"\n%s not deleted.\n"), pname);
    }

    freemem ();
    printf("\n");
    return (OK);
}

/*******************************************
* Find and return the lowest printer number
* in the printcap data base starting from
* the number "startingNumber".   DJG#2
********************************************/
int
GetLowestNumber (int startingNumber)
{
    char        pn[ALPHANUMLEN];    /* printer number string */
    char        lppn[ALPHANUMLEN];  /* lp name           */
    int pnum_done;      /* loop flag             */
    int status;         /* return for printcap query */
    int     printerNumber;  /* lowest number found   */

    printerNumber = startingNumber;
    pnum_done = 0;
    while (! pnum_done) 
    {
        (void) sprintf (pn, "%d", printerNumber);
        status = pgetent (capbuf_ptr, pn,TRUE);
        if (status == -1)
        {
            badfile (PRINTCAP);
        }
        else if (status == 1)
        {
            printerNumber ++;
        }
        else 
        {
            (void) sprintf (lppn, "lp%d", printerNumber);
            if (pgetent (capbuf_ptr, lppn,TRUE) == 1)
                printerNumber ++;
            else
                pnum_done ++;
        }
    }
    return (printerNumber);
}
                        
/*
 *  Add comments to printcap file
 *  for printer added by DoAdd
 */
AddComments ()
{
    int    done, leave;

    printf (MSGSTR(LPRSETUP_17, "\nAdding comments to printcap file for new printer, type '?' for help.\n"));
    done = FALSE;
    leave = FALSE;
    strcpy (symbolname, "n");
    printf (MSGSTR(LPRSETUP_18,"Do you want to add comments to the printcap file [%s] ? : "), symbolname);
    while (!done) 
    {
        switch (getcmd()) 
        {
          case NOREPLY:
          case NO:
          case QUIT:
            leave = TRUE;
          case YES:
          case GOT_SYMBOL:
            done = TRUE;
            break;
          case HELP:
            PrintHelp(MSGSTR(LPRSETUP_HELP_8,h_addcmnts),TRUE);
          default: /* falls through form case HELP: */
            strcpy (symbolname, "n");
            printf (MSGSTR(LPRSETUP_18,"Do you want to add comments to the printcap file [%s] ? : "), symbolname);
            break;
        }
    }
    if (!leave) 
    {
        printf (MSGSTR(LPRSETUP_19,"Enter comments below - Press RETURN on empty line to exit\n\n"));
        done = FALSE;
        for (numcomments = 0; !done && numcomments < LINES; ) 
        {
            printf ("# ");
            switch (getcmd()) 
            {
              case NOREPLY:
              case QUIT:
                done = TRUE;
                break;
              case HELP:
                PrintHelp (MSGSTR(LPRSETUP_HELP_8,h_addcmnts),TRUE);
                break;
              default:
                sprintf(printercomments[numcomments++],
                        "# %.*s", COLUMNS-3, symbolname);
                break;
            }
        }
    }
}

                

/*
 *  get fields for DoAdd
 */
AddField ()
{
    char    buf[LEN];           /* temp buffer           */
    int     done;               /* flag                  */
    int     i,j;                /* temp index            */
    struct stat st;
    char *valu;
    int yn;
    struct table *lp_entry=NULL;

    /*
     *  clear changeable tab values
     */
    for (i = 0; tab[i].name != 0; ++i)
    {
        tab[i].used = NO;
        tab[i].nvalue = 0;
    }

    if (MatchPrinter () == QUIT)
        return (QUIT);

    if (AddSyn () == QUIT)
        return (QUIT);

    if ((strcmp (ptype, "remote") !=0) && (strcmp(ptype, "printserver") != 0)) 
    {
        do 
        {
            printf (MSGSTR(LPRSETUP_20,"\nSet device pathname 'lp'"));
            if ((pnum[0] != '\0') && (pnum[1] == '\0'))
                (void) sprintf (buf, "%s%s", "/dev/tty0", pnum);
            else
                (void) sprintf (buf, "%s%s", "/dev/tty", pnum);
        } while (SetVal ("lp", buf) < 0);
        
	printf (MSGSTR(LPRSETUP_96,"\nDo you want to capture print job accounting data ([y]|n)? "));

	yn = yes_char;
	if (YesNo(yn) == TRUE)
	{
	        do 
		{
		    printf (MSGSTR(LPRSETUP_21,"\nSet accounting file 'af'"));
		    if (pnum[0] == '0' )
		        (void) sprintf (buf, "%s", "/usr/adm/lpacct");
		    else
		        (void) sprintf (buf, "%s%s%s", "/usr/adm/lp", pnum, "acct");
		} while (SetVal ("af", buf) < 0);

		/*
		 * If accounting is desired and no input filter
		 * is specified use the specified output filter
		 * for the input filter or else no accounting
		 * data will ever be collected.
		 */

		for (i = 0; tab[i].name; i++)
		    if (strcmp(tab[i].name, "if") == 0)
		        break;

		if (tab[i].name == 0)
		{
		    printf(MSGSTR(LPRSETUP_54,"internal error: cannot find symbol %s in table\n"), "if");
		    return(BAD);
		}

		if (tab[i].used == NO)
		{
		    for (j = 0; tab[j].name; j++)
		        if (strcmp(tab[j].name, "of") == 0)
		            break;

		    if (tab[j].name == 0)
		    {
		        printf(MSGSTR(LPRSETUP_54,"internal error: cannot find symbol %s in table\n"), "if");
		        return(BAD);
		    }

		    if ((tab[i].nvalue = (char *) malloc (strlen(tab[j].nvalue) + 1)) == NULL)
		    {
		        printf(MSGSTR(LPRSETUP_45,"\nmalloc: not enough space for symbols!\n"));
		        return(ERROR);
		    }
		    strcpy(tab[i].nvalue, tab[j].nvalue);
		    tab[i].used = tab[j].used;
		}
	}
    }

    /* Spooling directory and error log file ('lf') is set  */
    /* for ALL printers,  including 'remote'      001-gray  */

    do 
    {
        printf (MSGSTR(LPRSETUP_22,"\nSet spooler directory 'sd'"));
        if (pnum[0] == '0' )
            (void) sprintf (buf, "%s", "/usr/spool/lpd");
        else
            (void) sprintf (buf, "%s%s", "/usr/spool/lpd", pnum);
    } while (SetVal ("sd", buf) < 0);

    /* if (strcmp (ptype, "remote") != 0)                   */
    /* {                                                    */
    do 
    {
        printf (MSGSTR(LPRSETUP_23,"\nSet printer error log file 'lf'"));
        if (pnum[0] == '0' )
            (void) sprintf (buf, "/usr/adm/lperr");
        else
            (void) sprintf (buf, "/usr/adm/lp%serr", pnum);
    } while (SetVal ("lf", buf) < 0);
    /* }                                                    */

    for ( i = 0; tab[i].name; ++i ) 
    {
        if ( strcmp (tab[i].name, "lp" ) == 0 ) 
        {
            lp_entry = &tab[i];
            break;
        }
    }
    if (!lp_entry) 
    {
        printf(MSGSTR(LPRSETUP_24,"\nInternal Error, no \"lp\" entry"));
        return(QUIT);
    }
    if (lp_entry->used == YES && lp_entry->nvalue != NULL) 
    {
        if (strlen(lp_entry->nvalue) > 8 &&
            strncmp(lp_entry->nvalue, "/dev/tty", 8) == 0) 
        {
            /*
             * Serial printer connection
             */
            do 
            {
                printf(MSGSTR(LPRSETUP_25,"\nSet printer connection type 'ct'"));
                (void) sprintf(buf, "%s", "dev");
            } while (SetVal("ct", buf) < 0);
            if (strcmp (buf, "LAT") == 0)	/* 002 - gray */
            {
                if (stat(lp_entry->nvalue, &st) == 0
                    && major(st.st_rdev) == LAT_MAJORDEV) 
                {
		    /* Get Baud Rate - 002 - gray */
		    do 
                    {
			printf (MSGSTR(LPRSETUP_32,"\nSet printer baud rate 'br'"));
			(void) sprintf (buf, "%s", "9600");
                    } while (SetVal ("br", buf) < 0);

		    /************************************************************** 
                     ***** 002 - gray, commented out prompts for 'ts', 'op', and
                     ***** 'os', which are not needed until the /dev/lat mechanism
                     ***** is supported. 
                     ***** 
                    int k;
                    int OP_or_OS_set=0;

                    for (k=0; k < 2; k++) 
                    {
                        if (k)
                            printf(MSGSTR(LPRSETUP_26,"\nYou must enter a value for 'ts'"));

                        do 
                        {
                            printf(MSGSTR(LPRSETUP_27,"\nSet terminal server name 'ts'"));
                            (void) sprintf(buf, "");
                        } while (SetVal ("ts", buf) < 0);

                        if (buf[0] != '\0') break;
                    }

                    for (k=0; !OP_or_OS_set && k < 2; k++) 
                    {
                        if (k)
                        printf(MSGSTR(LPRSETUP_28,"\nYou must enter a value for either 'op' or 'os'\n"));
                        do 
                        {
                            printf(MSGSTR(LPRSETUP_29,"\nSet terminal server output port 'op'"));
                            (void) sprintf(buf, "");
                        } while (SetVal ("op", buf) < 0);
                        if (buf[0] != '\0') OP_or_OS_set++;
                        do 
                        {
                            printf(MSGSTR(LPRSETUP_30,"\nSet terminal server output service 'os'"));
                            (void) sprintf(buf, "");
                        } while (SetVal ("os", buf) < 0);
                        if (buf[0] != '\0') OP_or_OS_set++;
                    }
		    ***** 002 - gray, end of commented out section.
		    **************************************************************/
                } 
                else 
                {
                    printf(MSGSTR(LPRSETUP_31,
"\nThe tty you are currently using is not a LAT configured tty. \n\
You must configure a tty for LAT use, see MAKEDEV(8) and latcp(8) \n\
for more details."));
                    return(QUIT);       
                }
            } 
            else 
            {
             /*
              * if it's a serial printer not parallel
              * include baud rate
              */
                do 
                {
                    printf (MSGSTR(LPRSETUP_32,"\nSet printer baud rate 'br'"));
                    (void) sprintf (buf, "%s", "9600");
                } while (SetVal ("br", buf) < 0);
            }
        } 
        else 
        {
            /*
             * lp field does not appear to be a tty device
             * Don't use the tty ioctl fields
             */
            static char *not_used_tab[] = { "br", "fc", "fs", "xc", "xs", 0 };
            register char **p;

            for (p = not_used_tab; *p; p++) 
            {
                for ( i = 0; tab[i].name; ++i ) 
                {
                    if ( strcmp (tab[i].name, *p) == 0 ) 
                    {
                        tab[i].used = NO;
                        break;
                    }
                }
            }
        }
    }
    if (strcmp (ptype, "printserver") == 0) 
    {
        do 
        {
           printf(MSGSTR(LPRSETUP_33,"\nSet printserver output filter 'of'\n\retype default \
exactly with NODE replaced by printserver node name\n"));
           printf(MSGSTR(LPRSETUP_34,"\nFor a printserver running TCP/IP, use 'iplpscomm' \
instead of 'lpscomm'\n"));
           sprintf(buf, "%s", "");
        } while  (SetVal ("of", buf) < 0);
    }

    if (strcmp (ptype, "remote") == 0) 
    {
     /*
      * It is remote but we still have to add
      *     :lp=:\
      * in the /etc/printcap file.
      */
        /* Find index for 'lp' */
        for ( i = 0; tab[i].name; ++i )
            if ( strcmp(tab[i].name,"lp") == 0 )
                break;
        tab[i].used = YES;
        *tab[i].svalue = 0;  /* make lp="" for the printcap */
        do 
        {
            printf (MSGSTR(LPRSETUP_35,"\nSet remote system name 'rm'"));
            (void) sprintf(buf,"");
        } while (SetVal ("rm", buf) < 0);
        do 
        {
            printf (MSGSTR(LPRSETUP_36,"\nSet remote system printer name 'rp'"));
            (void) sprintf(buf,"");
        } while (SetVal ("rp", buf) < 0);
    }

    /* modify default field values */
    printf (h_symsel);
    for (i = 0,j=0; tab[i].name != 0; ++i)
    {
        printf (" %s ", tab[i].name);
        if (j++ > 14)
        {
            printf ("\n");
            j = 0;
        }
    }
    printf ("\n");

    done = FALSE;
    for (;;)
    {
        while (!done)
        {
            printf (MSGSTR(LPRSETUP_37,"\nEnter symbol name: "));
            switch (getcmd ())
            {
              case GOT_SYMBOL:
                DoSymbol ();    /*  Don't have to special case sd, the
                                 *  spooling directory since the entry
                                 *  is being added. It still can be
                                 *  changed since it doesn't exist yet.
                                 */
                break;
              case HELP:
                PrintHelp (MSGSTR(LPRSETUP_HELP_4,h_symsel),TRUE);
                for (i = 0,j=0; tab[i].name != 0; ++i)
                {
                    printf (" %s ", tab[i].name);
                    if (j++ > 14)
                    {
                        printf ("\n");
                        j = 0;
                    }
                }
                printf ("\n");
                break;
              case NOREPLY:
                break;
              case PRINT:
                Print (USED);
                break;
              case LIST:
                Print (ALL);
                break;
              case QUIT:
                done = TRUE;
                break;
              default:
                printf (MSGSTR(LPRSETUP_6,"\nInvalid choice, try again.\n"));
                break;
            }
        }
        if (Verified () == TRUE)
        {
            break;
        }
        else
        {
            printf(MSGSTR(LPRSETUP_38,"Do you wish to continue with this entry?  [y] "));
                yn = yes_char;
            if (YesNo (yn) == FALSE) 
            {
                return(QUIT);           /* no, they wish to abort */
            }
            else
            {
                done = FALSE;
            }
        }
    }
    return (OK);
}

/*
 *  find matching printer by name and copy over fields
 *  loop through default table, find match and assign.
 */
MatchPrinter ()
{
    int  found, done; /* flags                           */
    int  length;      /* strlen return                   */
    char *addr;       /* malloc return                   */
    int  i, j, k;     /* temp indices                    */
    int  yn;          /* yes - no answer                 */
    int  gotprinter;  /* true if a printer type is found */

    /*
     *  get printer type
     */
    found = FALSE;
    while (!found)
    {
        done = FALSE;
        while (!done)
        {
            printf (MSGSTR(LPRSETUP_39,"\nFor more information on the specific printer types\nEnter `printer?'\n"));
            printf (MSGSTR(LPRSETUP_40,"\nEnter the FULL name of one of the following printer types:\n"));

            helpbuf_ptr = helpbuf;
            pgetent(datbuf_ptr, "known", FALSE);
            if(pd_help = pgetstr("PD", &helpbuf_ptr, FALSE))
            {
                break_string(pd_help);
                PrintHelp(pd_help, TRUE);
            }

            printf (MSGSTR(LPRSETUP_41,"\nor press RETURN for [unknown] : "));

            switch (getsymbol ())
            {
              case PRINTER_INFO:
                SpecificPrinter();
                break;
              case GOT_SYMBOL:
              default:
                strcpy (ptype, symbolname);
                done = TRUE;
                break;
              case HELP:
                PrintHelp (MSGSTR(LPRSETUP_HELP_6A,h_type),TRUE);
                PrintHelp (MSGSTR(LPRSETUP_HELP_6B,""),FALSE);
                break;
              case QUIT:
                return (QUIT);
                break;
              case NOREPLY:
                printf (MSGSTR(LPRSETUP_42,"\nUsing 'unknown' for printer type, OK? [ n ] "));
                yn = no_char;
                if (YesNo (yn) == TRUE) 
                {
                    done = TRUE;
                    strcpy (ptype, UNKNOWN);
                }
                break;
            }
        }

        /*
         *  get the printers default entry 
         */

        helpbuf_ptr = helpbuf;
        gotprinter = pgetent(datbuf_ptr, ptype, FALSE);
        if(gotprinter == -1)
        { /* defaults file couldn't be opened, so leave */
            perror (DATAFILE);
            leave (ERROR);
        }

        if (!gotprinter)
        {
            printf (MSGSTR(LPRSETUP_43,"\nDon't know about printer '%s'\n"), ptype);
            printf (MSGSTR(LPRSETUP_44,"\nEnter 'y' to try again, or 'n' to use 'unknown' [y]: "));
                yn = yes_char;
            if (YesNo (yn) == FALSE)
            {
                strcpy (ptype, UNKNOWN);
                found = TRUE;
            } 
            else 
            {
                found = FALSE;
            }
        }
        else 
        { /* found a printer type */
                found = TRUE;
        }
    }

    /*
     *  For every value possible see if there is a entry
     *  and update the temporary printcap entry to match 
     */

    for(k = 0 ; tab[k].name != 0 ; k++)
    {/* loop through the standard entry symbol by symbol */
        if(pd_help = pgetstr(tab[k].name,&helpbuf_ptr,FALSE))
        {/* we found a match */
            length = strlen (pd_help) + 1;
            if ((addr = (char *) malloc (length)) == NULL)
            {/* bummer dude ! */
                printf (MSGSTR(LPRSETUP_45,"\nmalloc: not enough space for symbols!\n"));
                return (ERROR);
            }
            tab[k].nvalue = addr;
            strcpy (tab[k].nvalue, pd_help);
            tab[k].used = YES;
        }
    }
    return (OK);
}

/********************************************
* get help information on a specific printer
*********************************************/
SpecificPrinter()
{
    int   done;
    int   i, j;

    done = FALSE;

    printf (MSGSTR(LPRSETUP_46,"\nEntering PRINTER TYPE help information MODE\n"));
    printf (MSGSTR(LPRSETUP_47,"\nEnter the printer type for specific information\n \
       Enter '?' for help, 'quit' to exit : "));
    while (!done) 
    {
        helpbuf_ptr = helpbuf;
        switch (getcmd()) 
        {
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_9,h_printype),TRUE);
            break;
          case QUIT:
            done = TRUE;
            break;
          case GOT_SYMBOL:
            strcpy (printertype, symbolname); /* save printer name globaly */
            pgetent(datbuf_ptr, printertype, FALSE);
            if(pd_help = pgetstr("PD",&helpbuf_ptr,FALSE))
            {
                break_string(pd_help);
                PrintHelp(pd_help, TRUE);
            }
            else
            {
                printf (MSGSTR(LPRSETUP_48, "\n\nThe printer type %s is NOT SUPPORTED - No Information\n\n"), 
                        printertype);
            }
            break;
          case NOREPLY:
          default:
            break;
        }
        if (!done) 
        {
            helpbuf_ptr = helpbuf;
            pgetent(datbuf_ptr, "known", FALSE);
            if(pd_help = pgetstr("PD",&helpbuf_ptr,FALSE))
            {
                break_string(pd_help);
                PrintHelp(pd_help, TRUE);
            }
        }
        else 
        {
            printf ("\n\n");
        }
        printf (MSGSTR(LPRSETUP_47,"\nEnter the printer type for specific information\n \
               Enter '?' for help, 'quit' to exit : "));
    }
    printf (MSGSTR(LPRSETUP_49,"\nLeaving PRINTER TYPE help information MODE\n"));
    printf (MSGSTR(LPRSETUP_50,"\n\nReturning to Selection of Printer Type for ADDING a New Printer\n\n"));
}

/*****************************
*  add synonyms
*****************************/
AddSyn ()
{
    int     done, status;

    done = FALSE;
    while (!done && strlen (pname) < BUFLEN)
    {
        printf (MSGSTR(LPRSETUP_51,"\nEnter printer synonym: "));
        switch (getcmd ())
        {
          case GOT_SYMBOL:
            status = pgetent (capbuf_ptr, symbolname,TRUE);
            if (status == -1) 
            {
                badfile(PRINTCAP);
            }
            if (status == 1)
            {
                printf (MSGSTR(LPRSETUP_52,"\nSynonym is already in use, try something else.\n"));
            }
            else
            {
                if (strlen (pname) + strlen (symbolname) > (BUFLEN - 1))
                {
                    printf (MSGSTR(LPRSETUP_53,"\nSynonym too long, truncating to %d characters.\n"), BUFLEN);
                }
                else
                {
                    strcat (pname, "|");
                    strcat (pname, symbolname);
                    if ((strchr (symbolname, '\t') != NULL) || 
                        (strchr (symbolname, ' ') != NULL))
                    { /* Then last synonym */
                        return (TRUE);
                    }
                }
            }
            break;
          case HELP:
            PrintHelp (MSGSTR(LPRSETUP_HELP_5,h_synonym),TRUE);
            break;
          case QUIT:
            return (TRUE);
            break;
          case NOREPLY:
            return (TRUE);
            break;
          default:
            printf (MSGSTR(LPRSETUP_6,"\nInvalid choice, try again.\n"));
            break;
        }
    }
    return(OK);
}

/*
 *  set default values
 *  Returns -1 on error, 0 if ok
 */
SetVal (char *val, char *buf)
/*  *val - two-letter symbol 
 *  *buf - preset value
 */ 
{
    int     i;                  /* temp index            */
    char    line[LEN];          /* temp buffer           */

    /* find val */
    for (i = 0; tab[i].name; ++i)
        if (strcmp (tab[i].name, val) == 0)
            break;

    if (tab[i].name == 0) 
    {
        printf(MSGSTR(LPRSETUP_54,"internal error: cannot find symbol %s in table\n"), val);
        return (BAD);
    }

    /*
     * Next assignment is needed for the case when the parameter was not set
     * in the template.
     * It is set here because the parameter may be switched off again in
     * the call to UseDefault.
     */
    tab[i].used = YES;

    (void) UseDefault (line, (tab[i].nvalue ? tab[i].nvalue : buf), i);

/*
 *  By ignoring the return value of UseDefault we may do a superfluous
 *  copy if the default value was held in tab[i].nvalue.
 *  The win is that we now have only one flow of control.
 */

    if (validate(i, line) < 0) 
    {
        return(BAD);
    }
    if (tab[i].nvalue != NULL) 
    {
        free(tab[i].nvalue);
        tab[i].nvalue = NULL;
    }
    if ((tab[i].nvalue = (char *) malloc (strlen (line) + 1)) == NULL) 
    {
        printf (MSGSTR(LPRSETUP_55,"\nmalloc: no space for %s\n"), tab[i].name);
        return (ERROR);
    }
    /* 002 - gray */
    if (strcmp (tab[i].name, "ct") == 0 && strcmp (line, "lat") == 0)
	strcpy (tab[i].nvalue, "LAT");
    else
	strcpy (tab[i].nvalue, line);

    /* This is so caller can examine the chosen value */
    strcpy(buf, line);

    return(OK);
}

/*
 *  view current contents of printcap file
 */
ViewPrintcap ()
{
    FILE *pfp;
    int  cnt, done;
    char line[BUF_LINE];


    GetRows ();      /* determine current size of window or screen - DJG#12 */

    if ((pfp = fopen (PRINTCAP, "r")) == NULL ) 
    {
        badfile (PRINTCAP);
    }
    cnt = 0;
    done = FALSE;
    while (!done) 
    {
        cnt ++;
        if (fgets (line, BUF_LINE, pfp) == NULL) 
        {
            done = TRUE;
        }
        if (!done) 
        {
            if (cnt <= rows) 
            {
                printf ("%s", line);
            }
            else 
            {
                printf (MSGSTR(LPRSETUP_56,"\nPress 'RETURN' to continue or 'quit RETURN' to quit: "));
                switch (getcmd()) 
                {
                  case QUIT:
                  case GOT_SYMBOL:
                    done = TRUE;
                    break;
                  case NOREPLY:
                  default:
                    printf ("%s", line);
                    cnt = 1;
                    break;
                }
            }
        }
    }
}

/*
 *  write new printcap entry to printcap file
 */
AddEntry ()
{
    FILE *ofp, *lfp;            /* output and log file pointers */
    long timeval;               /* time for log file            */
    char buf[LEN];              /* temp buffer                  */

    /* open output and log files */
    if ((ofp = fopen (PRINTCAP, "a")) == NULL) 
    {
        badfile(PRINTCAP);
    }
    if ((lfp = fopen (LOGCAP, "a")) == NULL) 
    {
        badfile(LOGCAP);
    }
    
    WriteComments (ofp);
    WriteEntry (ofp);

    /* write time stamp and entry to log */
    timeval = time(0);
    strcpy (buf, "\nAdded ");
    strcat (buf, ctime (&timeval));
    fputs (buf, lfp);

    WriteEntry (lfp);

    fclose (ofp);
    fclose (lfp);
    return (OK);
}

/*
 *  create special device, if necessary
 */
AddDevice ()
{
    struct passwd *passwd;      /* password file entry   */
    char   *device;             /* parameter value ptr   */
    int     mode;               /* chmod mode            */
    int     i;                  /* temp index            */

/*
 *  get daemon id
 */
    if ((passwd = getpwnam (DAEMON)) == 0)
    {
        printf (MSGSTR(LPRSETUP_57,"\ngetpwnam: cannot get id for %s\n"), DAEMON);
        /*perror ();*/
        leave (ERROR);
    }

    /* create spooling directory */
    MakeSpool (passwd);

    /* create accounting file */
    MakeAcct (passwd);

    /* create errorlog file */
    MakeErrorlog (passwd);

    /* inform user of completion */
    setupdone();
}

/*
 * UnLinkSpool - optionally unlink old spool directory when modifying entry
 *
 * Description:
 *      We use existing function UnLinkSpooler by faking up the entry
 *      We prompt system manager who may want to keep old spool directory
 */
static void UnLinkSpool(struct table *tabent, char *oldpath)
{
    char *newpath;
    int used;
    int yn;


    printf(MSGSTR(LPRSETUP_58,"\nChanging spool directory\n"));
    printf(MSGSTR(LPRSETUP_59,"Do you want to delete the old spool directory? [n] "));

    yn = no_char;
    if (YesNo (yn) == TRUE) 
    {

        /* save current values */
        newpath = tabent->nvalue;
        used = tabent->used;

        /* Fake up for deletion */
        tabent->nvalue = oldpath;
        tabent->used = YES;

        /* Use existing function to unlink spool directory */
        UnLinkSpooler();        /* uses value stored in tabent */

        /* restore current values */
        tabent->nvalue = newpath;
        tabent->used = used;
    }
    return;
}

/*
 * UnLinkFile - optionally unlink old account or log file
 *      when modifying printcap entry
 *
 * Description:
 *      We use existing function UnLinkSymFile by faking up the entry
 *      We prompt system manager who may want to keep the old files
 *      which may be shared with other queues.
 */
static void UnLinkFile(struct table *tabent, char *oldpath)
{
    char *newpath;
    int used;

    /* save current values */
    newpath = tabent->nvalue;
    used = tabent->used;

    /* Fake up for deletion */
    tabent->nvalue = oldpath;
    tabent->used = YES;

    /* Use existing function to unlink file */
    UnLinkSymFile(tabent->name);

    /* restore current values */
    tabent->nvalue = newpath;
    tabent->used = used;

    return;
}

/*
 *  Modify selected entry
 *  If the entry for sd, lf, or af is 
 *  changed delete the old files
 */
ModifyEntry ()
{
    struct passwd *passwd;      /* passwd entry ptr              */
    int    done;                /* flag                          */
    FILE   *ifp, *ofp;          /* input/ouput file pointers     */
    char   keyname[LEN];        /* match name                    */
    char   buf[BUFLEN];         /* read/write buffer             */
    char   oldvalue[MAXPATH];   /* old value of af, lf, or sd    */
    int    i,j;                 /* temp index                    */
    int    old_used;            /* were these previously in use */
    int    old_and_new_different;
    int    yn;
    int    retval;
    enum which_symbol_e 
    {
        any_other_symbol,
        sd_symbol,
        lf_symbol,
        af_symbol
    } which_symbol;

    static struct delete_and_create_fns 
    {
        void (*delete_fn)(); 
        int  (*create_fn)();
    } fn_tab[] = 
      {
        {NULL,        NULL},
        {UnLinkSpool, MakeSpool},
        {UnLinkFile,  MakeErrorlog},
        {UnLinkFile,  MakeAcct}
      };

    /* get daemon id */
    if ((passwd = getpwnam (DAEMON)) == 0)
    {
        printf (MSGSTR(LPRSETUP_60,"\ngetpwnam: cannot find %s uid in passwd file.\n"), DAEMON);
        leave (ERROR);
    }

    /* modify fields */
    CopyEntry ();       /* affects longname */

    for (;;)
    {
        done = FALSE;
        printf(MSGSTR(LPRSETUP_61,"\nEnter the name of the symbol you wish to change.\nEnter 'p' \
to print the current values, 'l' to list\n all printcap values or 'q' to quit.\n"));
        while (!done)
        {
            old_used = NO;
            old_and_new_different = 0;
            which_symbol = any_other_symbol;

            printf (MSGSTR(LPRSETUP_62,"\nEnter symbol name:  "));
            switch (getcmd ())
            {
                case GOT_SYMBOL:
                    if ((strcmp("sd", symbolname) == 0  
                         && (which_symbol = sd_symbol)) ||
                        (strcmp("lf", symbolname) == 0 
                         && (which_symbol = lf_symbol)) ||
                        (strcmp("af", symbolname) == 0 
                         && (which_symbol = af_symbol))) 
                    {
                        for (i = 0; tab[i].name != 0; i++) 
                        {
                            if (strcmp (tab[i].name, symbolname) == 0) 
                            {
                                if ((old_used = tab[i].used) == YES) 
                                {
                                    strcpy (oldvalue,(tab[i].nvalue ? tab[i].nvalue : tab[i].svalue));
                                }
                                break;
                            }
                        }
                        /*
                         * No point in processing symbol if it wasn't found
                         * in the table
                         */
                        if (!tab[i].name) continue;
                    }

                    DoSymbol ();

                    /**** NOTE: i still points to the correct table entry ****/

                    switch(which_symbol) 
                    {
                      case sd_symbol:
                      case lf_symbol:
                      case af_symbol:
                        if (old_used == YES && tab[i].used == YES) 
                        {
                            old_and_new_different = 
                                strcmp(oldvalue, 
                                       (tab[i].nvalue ? tab[i].nvalue : 
                                        tab[i].svalue));
                        }
                        if (old_used == YES && (tab[i].used == NO 
                            || old_and_new_different)) 
                        {
                            fn_tab[(int)which_symbol].delete_fn(&tab[i], oldvalue);
                        }
                        if (!(old_used == YES && tab[i].used == YES &&
                              !old_and_new_different)) 
                        {
                            /*
                             * Always call create function unless there was no
                             * change. 
                             * This is inefficient, but makes use of
                             * warning message in MakeSpool if spool
                             * directory switched off.
                             */
                            fn_tab[(int)which_symbol].create_fn(passwd);
                        }
                        break;
                      case any_other_symbol:
                      default:
                        break;
                    }
                    break;
                case HELP:
                    PrintHelp (MSGSTR(LPRSETUP_HELP_4,h_symsel),TRUE);
                        for (i = 0,j=0; tab[i].name != 0; ++i)
                        {
                            printf (" %s ", tab[i].name);
                            if (j++ > 14)
                            {
                                printf ("\n");
                                j = 0;
                            }
                        }
                    printf ("\n");
                    break;
                case NOREPLY:
                    break;
                case PRINT:
                    Print (USED);
                    break;
                case LIST:
                    Print (ALL);
                    break;
                case QUIT:
                    done = TRUE;
                    break;
                default:
                    printf (MSGSTR(LPRSETUP_6,"\nInvalid choice, try again.\n"));
                    break;
            }
        }
        if (Verified() == TRUE)
        {
            break;
        }
        else
        {
            printf(MSGSTR(LPRSETUP_38,"Do you wish to continue with this entry?  [y] "));
                yn = yes_char;
            if (YesNo (yn) == FALSE) 
            {
                return(QUIT);   /* no, they wish to abort, although at this
                        point, the return value (from here) is not checked.
                        We just return early without actually do anything. */
            }
            else
            {
                done = FALSE;   /* not done yet */
            }
        }
    }

    /* save pname for match and longname to rewrite PNAME changed to PNUM */
    strcpy (keyname, pnum);   /* can search for any name, like "lp2" or "two" */
    strcpy (pname, longname); /* new pname contains the entire first line */

    /* open original and copy */
    if ((ifp = fopen (PRINTCAP, "r")) == NULL) 
    {
        badfile(PRINTCAP);
    }

    if ((ofp = fopen (COPYCAP, "w")) == NULL) 
    {
        badfile(COPYCAP);
    }

    /* copy printcap to copy until entry */
    while (fgets (buf, BUFLEN, ifp) != 0) 
    {
        if (CommentOrEmpty (buf))
        {
            fputs (buf, ofp);
        }
        else 
        {
            if (strpbrk (buf, ":|\\") != 0) 
            {
                if (SynonymMatch (buf, keyname) == 0) 
                { /* wrong entry so just copy */
                    fputs (buf, ofp);
                    while (fgets (buf, BUFLEN, ifp) != 0) 
                    {
                        fputs (buf, ofp);
                        if (buf[strlen(buf) - 2] != '\\')
                            break;
                    }
                }
                else 
                { /* right entry so replace old entry with modified one */
                    while (fgets (buf, BUFLEN, ifp) != 0) 
                    {
                        if (buf[strlen(buf) - 2] != '\\') 
                        {
                            WriteEntry (ofp);
                            break;
                        }
                    }
                }
            }
            else /* not sure what it is, but we better copy it */
                fputs (buf, ofp);
        }
    }

    fclose (ofp);
    fclose (ifp);

    /* mv new file to old file */
    if (rename (COPYCAP, PRINTCAP) < 0)
    {
        printf (MSGSTR(LPRSETUP_63,"\nCannot rename %s to %s (errno = %d).\n"),
                COPYCAP, PRINTCAP, errno);
        /* don't know what best to do here...*/
    }

    /* inform user of completion */
    setupdone();
    return(OK); /* the return value isn't checked (yet) but to be safe ... */
}


/*
 *  delete existing printcap entry
 */
DeleteEntry ()
{
    FILE *ifp, *ofp, *lfp;      /* input/output file pointers   */
    char    buf[LEN];           /* read/write buffer            */
    long    timeval;            /* time in seconds for logfile  */
    char    tempfile[LEN];      /* file name buffer             */

    /* open original, copy, and log */
    if ((ifp = fopen (PRINTCAP, "r")) == NULL) 
    {
        badfile(PRINTCAP);
    }

    (void) sprintf (tempfile, "%s%d", COPYCAP, getpid());
    if ((ofp = fopen (tempfile, "w")) == NULL)
    {
        printf(MSGSTR(LPRSETUP_64,"\nCannot open intermediate file: %s.\n"), tempfile);
        perror (tempfile);
        leave (ERROR);
    }

    if ((lfp = fopen (LOGCAP, "a")) == NULL) 
    {
        badfile(LOGCAP);
    }

    timeval = time(0);
    strcpy (buf, "\nDeleted ");
    strcat (buf, ctime(&timeval));
    fputs (buf, lfp);

    /* copy printcap to copy until next entry */
    while (fgets (buf, BUFLEN, ifp) != 0)
    {
        if (!findpname (pname, buf))
        {
            fputs (buf, ofp);
        }
        else
        {
            fputs (buf, lfp);
            while ((fgets (buf, BUFLEN, ifp) != 0) && 
                   (buf[strlen(buf) - 2] != ':'))
            {
                    fputs (buf, lfp);
            }
            /* write line with colon */
            fputs (buf, lfp);
        }
    }

    if (rename (tempfile, PRINTCAP) < 0) 
    {
        printf(MSGSTR(LPRSETUP_63,"\nCannot rename %s to %s (errno=%d).\n"),
                tempfile, PRINTCAP, errno);
    }
    fclose (ofp);
    fclose (ifp);
    fclose (lfp);

    UnLinkSpooler();           /* Delete Spooling directory  DJG#5 */
    UnLinkSymFile("af");       /* Delete Accounting file     DJG#10*/
    UnLinkSymFile("lf");       /* Delete Error log file      DJG#10*/

    return(OK);
}

/*
 *  copy entry into tab
 */
CopyEntry ()
{
    char    line[LEN];          /* read buffer           */
    char    tmpline[LEN];       /* temporary buffer      */
    char   *lineptr;            /* read buffer ptr       */
    char   *ptr;                /* temp pointer          */
    int     num;                /* pgetnum return        */
    int     i;                  /* temp index            */

    char s[ALPHANUMLEN];
    char *p;
    int status;
    
    strcpy(s, pnum);
    status = pgetent(capbuf_ptr, s,TRUE);       
    if (status == -1) 
    {
        badfile(PRINTCAP);
    }

    /* save names for rewrite */
    if ((ptr = (char *)index(capbuf_ptr, ':')) != 0) 
    {
        strncpy (longname, capbuf_ptr, ptr - capbuf_ptr);
    }
    longname[ptr - capbuf_ptr] = '\0';

    /* loop thru table, changing values where appropriate */
    for (i = 0; tab[i].name != 0; ++i)
    {
        switch (tab[i].stype)
        {
            case BOOL:
                
                if (pgetflag (tab[i].name, TRUE) == TRUE)
                {
                    if ((tab[i].nvalue = (char *) malloc (strlen ("on") + 1)) 
                        == NULL)
                    {
                        printf (MSGSTR(LPRSETUP_55,"\nCannot malloc space for %s\n"), tab[i].name);
                    }
                    else
                    {
                        strcpy (tab[i].nvalue, "on");
                        tab[i].used = YES;
                    }
                }
                break;
            case INT:
                if ((num = pgetnum (tab[i].name, TRUE)) >= 0)
                {
                    /* fc, fs, xc, xs are in octal, all others are decimal */
                    if ((strcmp(tab[i].name, "fc") == 0) ||
                        (strcmp(tab[i].name, "fs") == 0) ||
                        (strcmp(tab[i].name, "xc") == 0) ||
                        (strcmp(tab[i].name, "xs") == 0)) 
                    {
                        (void) sprintf (tmpline, "%o", num);
                        strcpy(line, "0");      /* put the zero out in front */
                        strcat(line, tmpline);
                    } 
                    else 
                    {
                        (void) sprintf (line, "%d", num);
                    }
                    if ((tab[i].nvalue = (char *) malloc (strlen (line) + 1)) 
                        == NULL)
                    {
                        printf (MSGSTR(LPRSETUP_55,"\nCannot malloc space for %s\n"), tab[i].name);
                    }
                    else
                    {
                        strcpy (tab[i].nvalue, line);
                        tab[i].used = YES;
                    }
                }
                break;
            case STR:
                lineptr = line;
                if (pgetstr (tab[i].name, &lineptr, TRUE) != NULL)
                {
                    *lineptr = 0;
                    if ((tab[i].nvalue = (char *) malloc (strlen (line) + 1)) 
                        == NULL)
                    {
                        printf (MSGSTR(LPRSETUP_55,"\nCannot malloc space for %s\n"), tab[i].name);
                    }
                    else
                    {
                        strcpy(tab[i].nvalue, line);
                        tab[i].used = YES;
                    }
                }
                break;
            default:
                printf (MSGSTR(LPRSETUP_65,"\nBad type (%d) for %s\n"), tab[i].stype, tab[i].name);
                break;
        }
    }

    return(OK);
}

/*
 *  write comments for next entry
 */
WriteComments (FILE *fp)
{
    int  i;
    
    for (i = 0; i < numcomments; i++) 
    {
        fprintf (fp, "%s\n", printercomments[i]);
    }
}

/*
 *  write single entry to file
 */
WriteEntry (FILE *fp)
{
    int     i;                  /* temp index                    */
    char   *curval;             /* pointer to current value      */
    char localname[LEN];        /* output filter, lp2 or whatever */

    fprintf (fp, "%s:", pname); /* here, pname is really the longname */
    for (i = 0; tab[i].name != 0; ++i)
    {
        if (tab[i].used == YES)
        {
            curval = (tab[i].nvalue ? tab[i].nvalue : tab[i].svalue);
            switch (tab[i].stype)
            {
                case BOOL:
                    if (strcmp ("on", curval) == 0)
                        fprintf (fp, "\\\n\t:%s:", tab[i].name);
                    break;
                case INT:
                    if (strcmp ("none", curval) != 0)
                        fprintf (fp, "\\\n\t:%s#%s:", tab[i].name, curval);
                    break;
                case STR:
                    fprintf (fp, "\\\n\t:%s=%s:", tab[i].name, curval);
                    break;
                default:
                    printf (MSGSTR(LPRSETUP_65,"\nbad type (%d) for %s\n"), tab[i].stype, tab[i].name);
                    break;
            }
        }
    }
    fprintf (fp, "\n");
    return(OK);
}

/*
 *  create accounting file 
 */
static int 
MakeAcct (struct passwd *passwd)
/* *passwd - password file entry */
{
    char   *acct;               /* parameter value ptr   */
    int    i, j;                /* temp index   */
    int    done;
    char   intermediate[MAXPATH];   /* intermediate directories */
    struct stat sb;
    int filedescriptor;

    for (i = 0; tab[i].name != 0; ++i)
        if (strcmp (tab[i].name, "af") == 0)
            break;

    if (tab[i].name == 0)
    {
        printf (MSGSTR(LPRSETUP_66,"\nCannot find accounting file entry in table!\nNo accounting file created.\n"));
        return (ERROR);
    }
    if (tab[i].used != YES) 
    {
        /* No account file specified */
        return (OK);
    }
    acct = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    if (stat(acct, &sb) < 0) 
    {
        if (errno == ENOENT)
        {/*
          * Accounting file does not exist, make any necessary
          * intermediate directories as needed
          */
            done = 0;
            j = 1;
            while (!done) 
            {
                while (acct[j] != '/' && acct[j] != '\0') j++;
                if (acct[j] == '\0') break; /* Create file */
                strncpy (intermediate, acct, j);
                intermediate[j++] = '\0';

                if (stat(intermediate, &sb) < 0) 
                {
                    if (mkdir(intermediate, DIRMODE) == -1) 
                    {
                        printf (MSGSTR(LPRSETUP_67,"\nCannot make intermediate accounting directory: %s\n"), 
                                intermediate);
                        perror (intermediate);
                        return (ERROR);
                    }
                    if (safechmod (intermediate, 00755) == -1)  /* incase root umask is not 022 */
                    {
                        printf (MSGSTR(LPRSETUP_68,"\nCannot chmod %s to mode 0755\n"), intermediate);
                        perror (intermediate);
                        return (ERROR);
                    }
                } 
                else 
                {
                    /* this portion of the accounting file path exists... */
                    if ((sb.st_mode&S_IFMT) != S_IFDIR) 
                    {
                        printf (MSGSTR(LPRSETUP_69,"\nIntermediate account directory: %s exists but it is not a directory!\n"), 
                                intermediate);
                        return (ERROR);
                    }
                }
            }

            /* Create actual accounting file */
            filedescriptor=open(acct,O_CREAT,00644);
            if (filedescriptor == -1)
            {
                printf(MSGSTR(LPRSETUP_70,"\nCannot create accounting file %s\n"),acct);
                perror (acct);
                return (ERROR);
            }
/* Unknown why this is commented out - 
            if (safechmod (acct, 00644) == -1)
            {
                printf (MSGSTR(LPRSETUP_71,"\nCannot chmod %s to mode 0644\n"), acct);
                perror (acct);
                close(filedescriptor);
                return (ERROR);
            }
*/
            if (safechown (acct, passwd -> pw_uid, passwd -> pw_gid) == -1)
            {
                printf (MSGSTR(LPRSETUP_72,"\nCannot chown %s to (%o/%o)\n"), acct, passwd->pw_uid, passwd->pw_gid);
                perror (acct);
                close(filedescriptor);
                return (ERROR);
            }
            close(filedescriptor);
            return(OK);
        }
        else
        {
            printf(MSGSTR(LPRSETUP_70,"\nCannot create accounting file %s\n"),acct);
            perror (acct);
            return (ERROR);
        }
    }
    else
    {
        printf(MSGSTR(LPRSETUP_73,"\nFile with same name as accounting file %s already exists\n"),acct);
        return (ERROR);
    }
}

/*
 *  create error log file if it doesn't already exist
 */
static int 
MakeErrorlog (struct passwd *passwd)
/* *passwd - password file entry */
{
    char   *errlog;             /* parameter value ptr   */
    int    i, j;                /* temp index   */
    int    done;
    char   intermediate[MAXPATH];   /* intermediate directories */
    struct stat sb;
    int filedescriptor;

    for (i = 0; tab[i].name != 0; ++i)
        if (strcmp (tab[i].name, "lf") == 0)
            break;

    if (tab[i].name == 0)
    {
        printf (MSGSTR(LPRSETUP_74,"\nCannot find error log file entry in table!\nNo error log file created.\n"));
        return (ERROR);
    }
    if (tab[i].used != YES) 
    {
        /* No error log file specified */
        return (OK);
    }
    errlog = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    if (stat(errlog, &sb) < 0) 
    {
        if (errno == ENOENT)
        {
           /*
            * Error Log file does not exist, make any necessary
            * intermediate directories as needed  DJG#1
            */
            done = 0;
            j = 1;
            while (!done) 
            {
                while (errlog[j] != '/' && errlog[j] != '\0') j++;
                if (errlog[j] == '\0') break; /* Create file */
                strncpy (intermediate, errlog, j);
                intermediate[j++] = '\0';

                if (stat(intermediate, &sb) < 0) 
                {
                    if (mkdir(intermediate, DIRMODE) == -1) 
                    {
                        printf (MSGSTR(LPRSETUP_75,"\nCannot make intermediate error log directory: %s\n"), intermediate);
                        perror (intermediate);
                        return (ERROR);
                    }
                    if (safechmod (intermediate, 00755) == -1)  /* incase root umask is not 022 */
                    {
                        printf (MSGSTR(LPRSETUP_68,"\nCannot chmod %s to mode 0755\n"), intermediate);
                        perror (intermediate);
                        return (ERROR);
                    }
                } 
                else 
                {
                    /* this portion of the error log file path exists... */
                    if ((sb.st_mode&S_IFMT) != S_IFDIR) 
                    {
                        printf (MSGSTR(LPRSETUP_76,"\nIntermediate error log directory: %s exists but is not a directory!\n"),
                                intermediate);
                        return (ERROR);
                    }
                }
            }

            /* Create error log file */
            filedescriptor=open(errlog,O_CREAT,00644);
            if (filedescriptor == -1)
            {
                printf(MSGSTR(LPRSETUP_77,"\nCannot create error log file %s\n"),errlog);
                perror (errlog);
                return (ERROR);
            }
/* Unknown why this section of the code is commented out - 
            if (safechmod (errlog, 00644) == -1)
            {
                printf (MSGSTR(LPRSETUP_71,"\nCannot chmod %s to mode 0644\n"), errlog);
                perror (errlog);
                close(filedescriptor);
                return (ERROR);
            }
*/
            if (safechown (errlog, passwd -> pw_uid, passwd -> pw_gid) == -1)
            {
                printf (MSGSTR(LPRSETUP_72,"\nCannot chown %s to (%o/%o)\n"), errlog, passwd->pw_uid, passwd->pw_gid);
                perror (errlog);
                close(filedescriptor);
                return (ERROR);
            }
            close(filedescriptor);
            return(OK);
        }
    }
    else
    {
        if ((sb.st_mode & S_IFMT) == S_IFREG) 
        {
            printf(MSGSTR(LPRSETUP_78,"\nWarning: regular file with same name as error log file %s already exists.\n"),errlog);
            return (OK);
        }
        if ((sb.st_mode & S_IFMT) == S_IFLNK)
        {
            printf(MSGSTR(LPRSETUP_79,"\nWarning: symbolic link with same name as error log file %s already exists.\n\
This may result in printers sharing an error log file.\n"),
                    errlog);
            return (OK);
        }
    }
    printf(MSGSTR(LPRSETUP_80,"\nInvalid name for error log file %s\n"),errlog);
    printf(MSGSTR(LPRSETUP_81,"File with same name as error log file %s already exists,\nbut it \
is not a regular file or a symbolic link.\n"),errlog);
    printf(MSGSTR(LPRSETUP_82,"Status information word S_IFMT = %d\n"),sb.st_mode&S_IFMT);
    return(ERROR);
}

/*
 *  create spooling directory
 */
static int 
MakeSpool (struct passwd *passwd)
/* *passwd - password file entry */
{
    char   *spool;              /* parameter value ptr   */
    int    i;                   /* temp index   */
    int    j;                   /* spool index  */
    struct stat sb;
    int    done;                /* loop flag    */
    char   intermediate[MAXPATH];   /* to create intermediate directories */

    for (i = 0; tab[i].name != 0; ++i)
        if (strcmp (tab[i].name, "sd") == 0)
            break;

    if (tab[i].name == 0)
    {
        printf (MSGSTR(LPRSETUP_83,"\nCannot find spooler directory entry in table!\nNo spooling directory created.\n"));
        return (ERROR);
    }
    if (tab[i].used != YES) 
    {
        /*
         * If spool directory is not specified in the printcap entry,
         * lpd will use the default directory
         */
        printf (MSGSTR(LPRSETUP_84,"Warning: queue will use default spool directory [/usr/spool/lpd]\n"));
        return (OK);
    }
    spool = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    /*
     * Try creating intermediate directories if they do not exist.
     * If stat fails, and then cannot mkdir, so exit. DJG#1
     */

    done = 0;
    j = 1;
    while (!done) 
    {
        while (spool[j] != '/' && spool[j] != '\0') j++;
        if (spool[j] == '\0') done++;
        strncpy (intermediate, spool, j);
        intermediate[j++] = '\0';

        if (stat(intermediate, &sb) < 0) 
        {
            if (mkdir(intermediate, DIRMODE) == -1) 
            {
                printf (MSGSTR(LPRSETUP_85,"\nCannot make spooling directory path: %s\n"), intermediate);
                perror (intermediate);
                return (ERROR);
            }
            if (safechmod (intermediate, 00755) == -1)  
            {/* incase root umask is not 022 */
                printf (MSGSTR(LPRSETUP_68,"\nCannot chmod %s to mode 0755\n"), intermediate);
                perror (intermediate);
                return (ERROR);
            }
        } 
        else 
        {
            /* this portion of the spooling directory exists... */
            if ((sb.st_mode&S_IFMT) != S_IFDIR) 
            {
                printf (MSGSTR(LPRSETUP_86,"\nSpooling directory path: %s already exists but it is not a directory!\n"),
                        intermediate);
                
                return (ERROR);
            }
        }
    }

    if (safechown (spool, passwd -> pw_uid, passwd -> pw_gid) == -1)
    {
        printf (MSGSTR(LPRSETUP_72,"\nCannot chown %s to (%o/%o)\n"), spool, passwd->pw_uid,
            passwd->pw_gid);
        perror (spool);
        return (ERROR);
    }
    return(OK);
}

/*
 *  unlink spooler directory - removes all files in spool 
 *  directory first
 *  for OSF/1 reference to structure direct changed to dirent
 */
void UnLinkSpooler()
{
    int    i;
    char  *spooler;
    char  unfile[MAXPATH];
    DIR    *dptr, *opendir();
    struct dirent *dp, *readdir();
    struct stat sb;

    for (i = 0; tab[i].name != 0; ++i)
        if (strcmp (tab[i].name, "sd") == 0)
            break;

    if (tab[i].name == 0)       /* can't find 'sd' symbol */
        return;         /* spool directory is just left laying around */

    spooler = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;

    if (stat(spooler, &sb) == 0) 
    {
        dptr = opendir (spooler);
        for (dp = readdir (dptr); dp != NULL; dp = readdir(dptr)) 
        {
            if ((strcmp (dp->d_name, ".") != 0) && 
                (strcmp (dp->d_name, "..") != 0)) 
            {
                (void) sprintf (unfile, "%s/%s", spooler, dp->d_name);
                if (safeunlink (unfile) != 0)
                    printf (MSGSTR(LPRSETUP_87,"Couldn't unlink spooler file %s\n"), unfile);
            }
        }

        if (safermdir(spooler) < 0) {
            printf(MSGSTR(LPRSETUP_88,"couldn't unlink old spooler directory (%s)\n"), spooler);
        }
        else 
        {
            printf(MSGSTR(LPRSETUP_89,"Removed spooling directory: %s\n"), spooler);
        }
    }
    else 
    {  /* There is a problem */
        if (errno == ENOENT) 
        {
            printf(MSGSTR(LPRSETUP_90,"\nSpooling directory %s does not exist\n"),spooler);
        }
        else 
        {
            printf (MSGSTR(LPRSETUP_91,"\nCan not unlink spooling directory %s because of errno %d\n"),
                    spooler, errno);
        }
        perror (spooler);
    }
    return;
}

/*
 * UnLinkSymFile - Removes the file associated with the specified symbol.
 */
void UnLinkSymFile (char *symbol)
{
    int    i;
    char *filename;
    int yn;


    for (i = 0; tab[i].name != 0; i++) 
    {
        if (strcmp (tab[i].name, symbol) == 0) 
        {
            if (tab[i].used) 
            {
                filename = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;
                printf(MSGSTR(LPRSETUP_92,"Do you want to delete '%s' file '%s' [n] "), 
                        symbol, filename);
                yn = no_char;
                if (YesNo (yn) == TRUE) 
                {
                    if (safeunlink(filename) != 0)
                    printf (MSGSTR(LPRSETUP_93,"Couldn't unlink :%s: file: %s\n"), symbol, filename);
                    else
                    printf (MSGSTR(LPRSETUP_94,"Deleted file: %s\n"), filename);
                }
            }
            break;
        }
    }
    return;
}

/*
 * badfile: print "cannot open
 * <filename>", and exit(1).
 */
badfile(char *s)
{
        printf(MSGSTR(LPRSETUP_95,"\nCannot open %s\n"), s);
        perror(s);
        leave(ERROR);
}

/*
 * initcmds: initializes the command strings from the internationalization
 *           (message) catalog. This allows the commands to be changed to
 *           meet requirements for a foriegn language.
 */
static void initcmds()
{/* All of the data structures touched are global */

    char *ind_e, *cmd_buf;
    int ind_cmd = 0;

/*
 *  initialize the command strings
 */

/* help command */
    /* get the message file string for the command */
    cmd_buf = MSGSTR(COMMANDS_1,"? help HELP \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {/* step through the aliases until done or no more space is left */
        /* copy the alias into the command table */
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        /* define the action of the alias */
        cmdtyp[ind_cmd++].cmd_id = HELP;
        /* set the pointer after the current alias */
        cmd_buf = ind_e + 1;
    }

/* add command */
    cmd_buf = MSGSTR(COMMANDS_2,"add ADD \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = ADD;
        cmd_buf = ind_e + 1;
    }

/* delete command */
    cmd_buf = MSGSTR(COMMANDS_3,"delete DELETE \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = DELETE;
        cmd_buf = ind_e + 1;
    }

/* exit command */
    cmd_buf = MSGSTR(COMMANDS_4,"exit EXIT quit QUIT \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = QUIT;
        cmd_buf = ind_e + 1;
    }

/* list command */
    cmd_buf = MSGSTR(COMMANDS_5,"list LIST \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = LIST;
        cmd_buf = ind_e + 1;
    }

/* modify command */
    cmd_buf = MSGSTR(COMMANDS_6,"modify MODIFY \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = MODIFY;
        cmd_buf = ind_e + 1;
    }

/* print command */
    cmd_buf = MSGSTR(COMMANDS_7,"print PRINT \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = PRINT;
        cmd_buf = ind_e + 1;
    }

/* view command */
    cmd_buf = MSGSTR(COMMANDS_8,"view VIEW \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = VIEW;
        cmd_buf = ind_e + 1;
    }

/* printer? command */
    cmd_buf = MSGSTR(COMMANDS_9,"printer? PRINTER? \n");
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = PRINTER_INFO;
        cmd_buf = ind_e + 1;
    }

/* yes command */
    cmd_buf = MSGSTR(COMMANDS_10,"yes YES \n");
    yes_char = cmd_buf[0]; /* snag the 'default' affirmative */
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = YES;
        cmd_buf = ind_e + 1;
    }

/* no command */
    cmd_buf = MSGSTR(COMMANDS_11,"no NO none NONE \n");
    no_char = cmd_buf[0]; /* snag the 'default' negative */
    while((ind_e = (char *)index(cmd_buf,' ')) && (ind_cmd < MAX_CMDS))
    {
        strncpy(cmdtyp[ind_cmd].cmd_name,cmd_buf,ind_e-cmd_buf);
        cmdtyp[ind_cmd++].cmd_id = NO;
        cmd_buf = ind_e + 1;
    }

/* 
 * terminate the list 
 */
    if (ind_cmd < MAX_CMDS)
    {/* if not out of space, make the next command alias the end of list */ 
        cmdtyp[ind_cmd].cmd_name[0] = '\0';
        cmdtyp[ind_cmd].cmd_id = EO_LIST;
    }
    /* force the last command to be the end of list 'just in case' */ 
    cmdtyp[MAX_CMDS-1].cmd_name[0] = '\0';
    cmdtyp[MAX_CMDS-1].cmd_id = EO_LIST;

    return;
}

/*
 * end of lprsetup.c
 */
