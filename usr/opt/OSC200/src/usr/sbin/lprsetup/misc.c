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
static char *sccsid  =  "@(#)$RCSfile: misc.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1994/01/05 15:52:38 $";
#endif

/*
 * Rewritten by John Allen Painter
 * with previous work done by:
 *     Mike Ardehali
 *     Adrian Thoms (thoms@wessex)
 *     Adrian Thoms (for Daren Seymour)
 *     DJG
 */

#include <stdio.h>
#include <strings.h>
#include <sys/ioctl.h>
#include "lprsetup.h"
#include "argstrings.h"
#include "lprsetup_msg.h"

/*
 * Message catalog support 
 */
#include <ctype.h>
#include "lprsetup_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LPRSETUP,n,s)
#ifdef	SEC_BASE
#define	MSGSTR_SEC(n,s)	catgets(catd,MS_LPRSETUP_SEC,n,s)
#endif

/*
 *  Prototypes and externals 
 */
FILE *Popen();
extern char yes_char;
extern char no_char;

/*
 * SynonymMatch - returns a pointer to the first
 * occurrance of the synonym in the list, it returns NULL
 * if it can not find one or if synonym is longer than list.
 */
char *SynonymMatch (char *list, char *synonym)
{
#define SEPARATORS  "|:"       /* separators in synonym "list" */
#define BLANK      " "         /* identifies end of synonym list */

    char *token, *strtok();
    char tmplist[BUFLEN];

    strcpy (tmplist, list);    /* because strtok corrupts string */
    if (strlen (synonym) <= strlen (tmplist)) 
    { /* get first token in list */
        token = strtok (tmplist, SEPARATORS);
        while (token != NULL) 
        {
            if (index (token, BLANK) == 0) 
            { /* its a legal synonym */
                if (strcmp (token, synonym) == 0) 
                    return (token);
            }
            else 
            {  /* its the last entry, which doesn't count */
                return (NULL);
            }
            /* Get next token */
            token = strtok (0, SEPARATORS);
        }
    }
    return (NULL);
}
            
/*
 *  CommentOrEmpty - Returns true if the line passed is a
 *  comment or an empty line, false otherwise. The line is
 *  from the /etc/printcap file.
 */

int CommentOrEmpty (char line[])
{
    int      i;

    if (line[0] == '#') { /* the line is a comment */
        return (1);
    }
    else { /* its empty or something is on it */
        for (i = 0; line[i] != '\0'; i++ ) {
            if (line[i] != ' ' && line[i] != '\t') {
                if (line[i] == '\n')        /* the line is empty */
                    return (1);
                else                        /* somethings on it  */
                    return (0);
            }
        }
    }
}

/*
 * YesNo - get a yes or no answer
 * Returns TRUE=(yes), FALSE=(no)
 */
int YesNo (int def_ans)
{
    int     ans, done;
        
    done = FALSE;
    ans = 0;
    while (NOT done)
    {
        ans = getcmd ();
        switch (ans)
        {
          case NOREPLY: 
            done = TRUE;
            if (def_ans == yes_char)
                ans = TRUE;
            else
            if (def_ans == no_char)
                ans = FALSE;
            break;
          case YES: 
            done = TRUE;
            ans = TRUE;
            break;
          case NO: 
          case QUIT: 
            done = TRUE;
            ans = FALSE;
            break;
          case HELP: 
          default: 
            printf (MSGSTR(MISC_0,"\nPlease answer yes or no  [%c] "), 
                    def_ans);
        }
    }
    return (ans);
}

/*
 * UseDefault
 *       returns TRUE if NULL entered (meaning use default) or
 *       FALSE after reading text to use in place of default
 */
int UseDefault (char *str, char *def, int i)
{
/* 
 *  i is the index into table of current symbol name 
 *  which is used for more specific help messages 
 */
    extern char symbolname[];
    extern struct table tab[];
    int    done;
    
    done = FALSE;
    while (!done)
    { /* loop until done */
        printf (" [%s] ?  ", def);
        switch (getsymbol ())
        {
          default:      /* this should never match! */
          case GOT_SYMBOL: 
            strcpy (str, symbolname);
            return (FALSE);
            break;
          case NO:
            tab[i].used = NO;
            return (TRUE);
            break;
          case QUIT: 
            return (QUIT);
            break;
          case HELP: 
            realhelp(i); /* more specific help */
            /* a general help message */
            PrintHelp (MSGSTR(LPRSETUP_HELP_7,h_default),TRUE);
            printf (MSGSTR(MISC_1,"\nEnter a value for '%s'? "), 
                    tab[i].name, def);
            break;
          case NOREPLY: 
            if (str != def)
                strcpy (str, def);
            return (TRUE);
            break;
        }
    }
}

/*
 * realhelp
 *       provides specific help for a given
 *       symbol index.
 */
realhelp(int i)
{
    extern struct table tab[];

    switch(i) 
    {
      case H_af:
        PrintHelp(MSGSTR(OPTION_HELP_af,h_af),TRUE);
        break;

      case H_br:
        PrintHelp(MSGSTR(OPTION_HELP_br,h_br),TRUE);
        break;

      case H_cf:
        PrintHelp(MSGSTR(OPTION_HELP_cf,h_cf),TRUE);
        break;

      case H_ct:
        PrintHelp(MSGSTR(OPTION_HELP_ct,h_ct),TRUE);
        break;

      case H_df:
        PrintHelp(MSGSTR(OPTION_HELP_df,h_df),TRUE);
        break;

      case H_dn:
        PrintHelp(MSGSTR(OPTION_HELP_dn,h_dn),TRUE);
        break;

      case H_du:
        PrintHelp(MSGSTR(OPTION_HELP_du,h_du),TRUE);
        break;

      case H_fc:
        PrintHelp(MSGSTR(OPTION_HELP_fc,h_fc),TRUE);
        break;

      case H_ff:
        PrintHelp(MSGSTR(OPTION_HELP_ff,h_ff),TRUE);
        break;

      case H_fo:
        PrintHelp(MSGSTR(OPTION_HELP_fo,h_fo),TRUE);
        break;
 
      case H_fs:
        PrintHelp(MSGSTR(OPTION_HELP_fs_A,h_fs),TRUE);
        PrintHelp(MSGSTR(OPTION_HELP_fs_B,""),FALSE);
        break;

      case H_gf:
        PrintHelp(MSGSTR(OPTION_HELP_gf,h_gf),TRUE);
        break;

      case H_ic:
        PrintHelp(MSGSTR(OPTION_HELP_ic,h_ic),TRUE);
        break;
 
      case H_if:
        PrintHelp(MSGSTR(OPTION_HELP_if,h_if),TRUE);
        break;

      case H_lf:
        PrintHelp(MSGSTR(OPTION_HELP_lf,h_lf),TRUE);
        break;

      case H_lo:
        PrintHelp(MSGSTR(OPTION_HELP_lo,h_lo),TRUE);
        break;

      case H_lp:
        PrintHelp(MSGSTR(OPTION_HELP_lp,h_lp),TRUE);
        break;

      case H_mc:
        PrintHelp(MSGSTR(OPTION_HELP_mc,h_mc),TRUE);
        break;

      case H_mx:
        PrintHelp(MSGSTR(OPTION_HELP_mx,h_mx),TRUE);
        break;

      case H_nf:
        PrintHelp(MSGSTR(OPTION_HELP_nf,h_nf),TRUE);
        break;

      case H_of:
        PrintHelp(MSGSTR(OPTION_HELP_of_A,h_of),TRUE);
        PrintHelp(MSGSTR(OPTION_HELP_of_B,""),FALSE);
        break;

/*** 001 - gray *************************************
      case H_op:
        PrintHelp(MSGSTR(OPTION_HELP_op,h_op),TRUE);
        break;

      case H_os:
        PrintHelp(MSGSTR(OPTION_HELP_os,h_os),TRUE);
        break;
*****************************************************/

      case H_pl:
        PrintHelp(MSGSTR(OPTION_HELP_pl,h_pl),TRUE);
        break;

      case H_pp:
        PrintHelp(MSGSTR(OPTION_HELP_pp,h_pp),TRUE);
        break;

      case H_ps:
        PrintHelp(MSGSTR(OPTION_HELP_ps,h_ps),TRUE);
        break;
 
      case H_pw:
        PrintHelp(MSGSTR(OPTION_HELP_pw,h_pw),TRUE);
        break;

      case H_px:
        PrintHelp(MSGSTR(OPTION_HELP_px,h_px),TRUE);
        break;

      case H_py:
        PrintHelp(MSGSTR(OPTION_HELP_py,h_py),TRUE);
        break;

      case H_rf:
        PrintHelp(MSGSTR(OPTION_HELP_rf,h_rf),TRUE);
        break;

      case H_rm:
        PrintHelp(MSGSTR(OPTION_HELP_rm,h_rm),TRUE);
        break;

      case H_rp:
        PrintHelp(MSGSTR(OPTION_HELP_rp,h_rp),TRUE);
        break;

      case H_rs:
        PrintHelp(MSGSTR(OPTION_HELP_rs,h_rs),TRUE);
        break;

      case H_rw:
        PrintHelp(MSGSTR(OPTION_HELP_rw,h_rw),TRUE);
        break;

      case H_sb:
        PrintHelp(MSGSTR(OPTION_HELP_sb,h_sb),TRUE);
        break;

      case H_sc:
        PrintHelp(MSGSTR(OPTION_HELP_sc,h_sc),TRUE);
        break;

      case H_sd:
        PrintHelp(MSGSTR(OPTION_HELP_sd,h_sd),TRUE);
        break;

      case H_sf:
        PrintHelp(MSGSTR(OPTION_HELP_sf,h_sf),TRUE);
        break;

      case H_sh:
        PrintHelp(MSGSTR(OPTION_HELP_sh,h_sh),TRUE);
        break;

      case H_st:
        PrintHelp(MSGSTR(OPTION_HELP_st,h_st),TRUE);
        break;

      case H_tf:
        PrintHelp(MSGSTR(OPTION_HELP_tf,h_tf),TRUE);
        break;

      case H_tr:
        PrintHelp(MSGSTR(OPTION_HELP_tr,h_tr),TRUE);
        break;

/*** 001 - gray *************************************
      case H_ts:
        PrintHelp(MSGSTR(OPTION_HELP_ts,h_ts),TRUE);
        break;
*****************************************************/

      case H_uv:
        PrintHelp(MSGSTR(OPTION_HELP_uv,h_uv),TRUE);
        break;
 
      case H_vf:
        PrintHelp(MSGSTR(OPTION_HELP_vf,h_vf),TRUE);
        break;

      case H_xc:
        PrintHelp(MSGSTR(OPTION_HELP_xc,h_xc),TRUE);
        break;

      case H_xf:
        PrintHelp(MSGSTR(OPTION_HELP_xf,h_xf),TRUE);
        break;

      case H_xs:
        PrintHelp(MSGSTR(OPTION_HELP_xs_A,h_xs),TRUE);
        PrintHelp(MSGSTR(OPTION_HELP_xs_B,""),FALSE);
        break;

      case H_Da:
        PrintHelp(MSGSTR(OPTION_HELP_Da,h_Da),TRUE);
        break;

      case H_Dl:
        PrintHelp(MSGSTR(OPTION_HELP_Dl,h_Dl),TRUE);
        break;

      case H_It:
        PrintHelp(MSGSTR(OPTION_HELP_It,h_It),TRUE);
        break;

      case H_Lf:
        PrintHelp(MSGSTR(OPTION_HELP_Lf,h_Lf),TRUE);
        break;

      case H_Lu:
        PrintHelp(MSGSTR(OPTION_HELP_Lu,h_Lu),TRUE);
        break;

      case H_Ml:
        PrintHelp(MSGSTR(OPTION_HELP_Ml,h_Ml),TRUE);
        break;

      case H_Nu:
        PrintHelp(MSGSTR(OPTION_HELP_Nu,h_Nu),TRUE);
        break;

      case H_Or:
        PrintHelp(MSGSTR(OPTION_HELP_Or,h_Or),TRUE);
        break;

      case H_Ot:
        PrintHelp(MSGSTR(OPTION_HELP_Ot,h_Ot),TRUE);
        break;

      case H_Ps:
        PrintHelp(MSGSTR(OPTION_HELP_Ps,h_Ps),TRUE);
        break;

      case H_Sd:
        PrintHelp(MSGSTR(OPTION_HELP_Sd,h_Sd),TRUE);
        break;

      case H_Si:
        PrintHelp(MSGSTR(OPTION_HELP_Si,h_Si),TRUE);
        break;

      case H_Ss:
        PrintHelp(MSGSTR(OPTION_HELP_Ss,h_Ss),TRUE);
        break;

      case H_Ul:
        PrintHelp(MSGSTR(OPTION_HELP_Ul,h_Ul),TRUE);
        break;

      case H_Xf:
        PrintHelp(MSGSTR(OPTION_HELP_Xf,h_Xf),TRUE);
        break;

      default:
        printf (MSGSTR(MISC_2, "Sorry, no specific help is available for symbol'%s'\n"),
                tab[i].name);
        break;
    }
}

/*
 * MapLowerCase
 *     maps the given string into lower-case.
 */
int MapLowerCase (char *b)
{
    char *p = b;

    while (*p)
    {
        if (isascii (*p) && isupper (*p))
            *p = tolower (*p);
        p++;
    }
    return (OK);
}

int HasBadChars (char *b)
{
    while (*b)
    {
        if ((NOT isalpha (*b)) && (NOT isdigit (*b)) && (*b != '_'))
            return (TRUE);
        b++;
    }
    return (FALSE);
}

/*
 *  Print the symbol table
 *  print whole table, or just the 'used' symbols 
 */
int Print (int flag)
{
    extern struct table tab[];
    extern char pnum[];
    extern char ptype[];
    int     i, j;

    printf (MSGSTR(MISC_3,"\n\tPrinter #%s \n\t----------"),pnum);
    if (strlen(pnum) > 1)
        printf(MSGSTR(MISC_4,"-"));             /* add one for printers numbered 10...99 */
    printf (MSGSTR(MISC_5A,"\nSymbol "));
    if (flag == ALL)
        printf (MSGSTR(MISC_6A,"used "));

    printf (MSGSTR(MISC_7A," type  value\n"));
    printf (MSGSTR(MISC_5B,"------ "));
    if (flag == ALL)
        printf (MSGSTR(MISC_6B,"---- "));  /* under 'used' */

    printf (MSGSTR(MISC_7B," ----  -----\n"));

    /*
     * for each symbol, print name, type, used, value 
     */
    for (i=0; tab[i].name != 0; i++)
    {
         /* don't print it, if not being used now */
        if ((flag == USED) && (tab[i].used == NO))
            continue;

        printf ("  %s   ", tab[i].name);
        if (flag == ALL)
            printf ("%s", tab[i].used == YES ? MSGSTR(MISC_8A,"YES  ") : MSGSTR(MISC_8B," NO  "));

        switch (tab[i].stype)
        {
          case BOOL: 
            printf (" BOOL ");
            break;
          case INT: 
            printf (" INT  ");
            break;
          case STR: 
            printf (" STR  ");
            break;
          default: 
            printf (MSGSTR(MISC_8C," ???    ??????\n"));
            continue;       /* get next symbol */
        }
 
        if ((tab[i].nvalue != 0) && (tab[i].nvalue != '\0')) 
        {
            printf ("  %s", tab[i].nvalue);
        }
        else if ((tab[i].svalue != 0) && (tab[i].svalue != '\0')) 
        {
            printf ("  %s", tab[i].svalue);
        }
        printf("\n");           /* end the line */
    }
    return (OK);
}

/*
 * Verified
 *       print the current printcap data, ask if it is OK, and return
 *       TRUE if it is OK, otherwise false.      
 */
int Verified ()
{
    extern char pnum[];
    int yn;
/*
 *  Clear all waiting input and output chars.
 *  Actually we just want to clear any waiting input chars so
 *  we have a chance to see the values before confirming them.
 *  We have to sleep a second to let waiting output chars print.
 */
    sleep (1);
    ioctl (0, TIOCFLUSH, 0);

    Print (USED);               /* print values being used in current
                                   configuration  */

    printf (MSGSTR(MISC_9,"\nAre these the final values for printer %s ? [y] "), pnum);
    fflush (stdout);
        yn = yes_char;
    if (YesNo (yn) == TRUE) 
    {
        printf("\n");
        return (TRUE);
    }
    else 
    {
        printf("\n");
        return (FALSE);
    }
}

/*
 *  DoSymbol - adds/modifies symbols.
 */
int DoSymbol ()
{
    extern struct table tab[];          /* default printer table        */
    extern char symbolname[];           /* getcmd result                */
    extern char oldfilter[];            /* print filter before modify   */
    extern char ptype[];                /* for checking on 'af' use     */
    extern char isnotused[];            /* "...feature is not used in LP11... */
    char     newval[LEN];               /* new value entered            */
    char *addr, *curval;                /* malloc and current value     */
    int     i, done = FALSE;
    int yn;

    /* 
     * find the symbol, print current value, and
     * ask for the new value, or initial value,
     * if any.
     */
    if (strlen (symbolname) > 2)
    {
        printf (MSGSTR(MISC_10,"\nSymbol name '%s' is too long!\n"), symbolname);
        return(ERROR);
    }

    /* symbolname contains the line just read from stdin */
    for (i = 0; tab[i].name != 0; i++)
    {
        if (strcmp (tab[i].name, symbolname) == 0)
        {
            curval = tab[i].nvalue ? tab[i].nvalue : tab[i].svalue;
            break;
        }
    }
    if (tab[i].name == 0)
    {
        printf(MSGSTR(MISC_11,
               "\nSymbol '%s' not found.  Use the 'list' command for a\nlist of all of the symbols and their defaults.\n"),
                symbolname);
        return(ERROR);
    }

    /*
     * got symbol, now prompt for new value
     */
    do
    {
        printf (MSGSTR(MISC_12,"\nEnter a new value for symbol '%s'?  [%s] "),tab[i].name, curval);
        switch (getsymbol() )
        {
          case QUIT: 
            return (QUIT);
            break;
          case HELP: 
            realhelp(i); /* more specific help */
            PrintHelp (MSGSTR(LPRSETUP_HELP_7,h_default),TRUE);
            break;
          case NO:          /* easily turn this parameter off, ie. set used */
            tab[i].used = NO;
            done = TRUE;
            break;
          case NOREPLY: 
            if ((strcmp (curval, "none") != 0))
                tab[i].used = YES;
            done = TRUE;
            break;
          case GOT_SYMBOL: 
          default:
            strcpy (newval, symbolname);
            printf (MSGSTR(MISC_13,"\nNew '%s' is '%s', is this correct?  [y] "), tab[i].name, newval);
            yn = yes_char;
            if (YesNo (yn) == TRUE)
            {
                if (validate(i, newval) < 0)        /* check if valid */
                    continue;

                if (strcmp (newval, curval) != 0)
                {
                    if ((addr = (char *) malloc (strlen (newval) + 1)) == NULL)
                    {
                        printf (MSGSTR(LPRSETUP_55,"\nmalloc: cannot get space for symbol '%s'.\n"),
                                tab[i].name);
                        return(ERROR);
                    }
                    tab[i].nvalue = addr;
		    /* 001 - gray */
		    if (strcmp(tab[i].name, "ct") == 0 && strcmp(newval, "lat") == 0)
			strcpy (tab[i].nvalue, "LAT");
		    else
                        strcpy (tab[i].nvalue, newval);
                }
                tab[i].used = YES;
                done = TRUE;
            }
            break;
        }
    } while (NOT done);
    return (OK);
}

/*
 * invalid postscript argument - print error messege + valid args
 */

static void invalid_arg(char *opt, int opt_num)
{
    struct arg_pair *arg_list;
    int width=0;

    get_args(opt_num,&arg_list);
    printf(MSGSTR(MISC_14,"\nSorry, the value of '%s' must be one of: "), opt);

    while (arg_list->arg) 
    {
        printf("%s ", arg_list->arg);
        arg_list++;
    }
    printf("\n");
}
        
/*
 * validate: check that 
 *      EITHER 
 *              entered value = "none" to remove entry
 *      OR
 *              int's are all digits
 *              baud rates are legal 
 *              booleans are on/off
 *              directory names must all start with '/' 
 *                      (except "lp" for remote should be "null"
 *                       or start with a "@/" for tcp/ip connections )
 *
 * returns -1 if bad, else 0.
 */
int validate(int i, char *value1)
{
    extern struct table tab[];
    int retval = OK;    /* return value from this routine */
    int k;                      /* loop counter */
    char value[LEN];    /* value of the symbol just entered */
    char dummy[LEN];

    /*
     * save the symbol value locally
     */
    if ((strcmp("none",value1)==0) 
        || (strcmp ("no", value1) == 0)) 
    {   /* "none" = delete this entry */
        tab[i].nun = 1;
        tab[i].used = NO;
        strcpy(value1,"");
        retval = OK;
        return (retval);
    }

    if (value1 != NULL)
        strcpy(value, value1);
    else
        strcpy(value, "");              /* probably can't happen */

    switch (tab[i].stype)
    {
      case BOOL: 
      /* Booleans can only be on or off */
        if (!(!strcmp("on", value)
            || (! strcmp("ON", value))
            || (! strcmp("off", value))
            || (! strcmp("OFF", value))))
        {
            printf(MSGSTR(MISC_15,"\nSorry, boolean symbol '%s' can only be 'on' or 'off'\n"), tab[i].name);
            retval=BAD;
        }
        break;

      case INT: 
      /* check first that we have all digits */

        for (k = 0; value[k]!='\0'; k++)
        {
            if (! isdigit(value[k]))
            {
                printf(MSGSTR(MISC_16,"\nSorry, integer symbol '%s' must contain only digits (0 - 9).\n"), tab[i].name);
                retval=BAD;
                break;
            }
        }
      
        /*
         * See if BR was specified and check if it is valid
         * only if we haven't encountered an error yet.
         */
        if ((strcmp("br", tab[i].name) == 0) && (retval != BAD))
        {
            switch(atoi(value))
            {
              case 0:
              case 50:
              case 75:
              case 110:
              case 134:
              case 150:
              case 200:
              case 300:
              case 600:
              case 1200:
              case 1800:
              case 2400:
              case 4800:
              case 9600:
              case 19200:
              case 38400:   
              case 57600:
                break; /* baud rate OK */
      
              default:
                printf(MSGSTR(MISC_17,"\nSorry, illegal baudrate: %s\n\nAvailable baud rates are:\n"), value);
                printf("\t   0\t  134\t   600\t  4800\n");
                printf("\t  50\t  150\t  1200\t  9600\n");
                printf("\t  75\t  200\t  1800\t 19200\n");
                printf("\t 110\t  300\t  2400\t 38400\n");
                printf("\t 57600\n");
                retval = BAD;
                break;
            }
        }
        break;
      
      case STR: 
      /* check if name is special and must start with '/' */
        if (!strcmp ("af", tab[i].name)      /* accounting file */
           || (!strcmp ("cf", tab[i].name))  /* cifplot filter */
           || (!strcmp ("df", tab[i].name))  /* TeX DVI filter */
           || (!strcmp ("dn", tab[i].name))  /* daemon name */
           || (!strcmp ("gf", tab[i].name))  /* plot filter */
           || (!strcmp ("lf", tab[i].name))  /* logfile */
           || (!strcmp ("lp", tab[i].name))  /* device name */
           || (!strcmp ("nf", tab[i].name))  /* ditroff filter */
           || (!strcmp ("rf", tab[i].name))  /* FORTRAN filter */
           || (!strcmp ("sd", tab[i].name))  /* spool directory */
           || (!strcmp ("tf", tab[i].name))  /* troff filter */
           || (!strcmp ("vf", tab[i].name))  /* raster filter */
           || (!strcmp ("xf", tab[i].name))) /* passthru filter */
        {
            if (value[0] != '/') 
            {
                printf(MSGSTR(MISC_18,"\nSorry, the value of symbol '%s' must begin with '/'.\n"),tab[i].name);
                retval=BAD;
            }
            break;
        }
      
        /* check arguments to postscript options */
        if (!strcmp("It", tab[i].name))
            if (check_arg(value, as_input_trays, dummy) != 0)
            {
                invalid_arg("It", as_input_trays);
                retval=BAD;
            }
        if (!strcmp("Ml", tab[i].name))
            if (check_arg(value, as_messages, dummy) != 0)
            {
                invalid_arg("Ml", as_messages);
                retval=BAD;
            }
        if (!strcmp("Or", tab[i].name))
            if (check_arg(value, as_orientations, dummy) != 0)
            {
                invalid_arg("Or", as_orientations);
                retval=BAD;
            }
        if (!strcmp("Ot", tab[i].name))
            if (check_arg(value, as_output_trays, dummy) != 0)
            {
                invalid_arg("Ot", as_output_trays);
                retval=BAD;
            }
        if (!strcmp("Ps", tab[i].name))
            if (check_arg(value, as_page_sizes, dummy) != 0)
            {
                invalid_arg("Ps", as_page_sizes);
                retval=BAD;
            }
        if (!strcmp("Sd", tab[i].name))
            if (check_arg(value, as_page_sizes, dummy) != 0)
            {
                invalid_arg("Sd", as_page_sizes);
                retval=BAD;
            }
        if (!strcmp("Si", tab[i].name))
            if (check_arg(value, as_sides, dummy) != 0)
            {
                invalid_arg("Si", as_sides);
                retval=BAD;
            }
        if (!strcmp("Ss", tab[i].name))
            if (check_arg(value, as_page_sizes, dummy) != 0)
            {
                invalid_arg("Ss", as_page_sizes);
                retval=BAD;
            }
        break;
      
      default: 
        printf (MSGSTR(MISC_19,"lprsetup: bad type %d for symbol %s.\n"),tab[i].stype,tab[i].name);
        retval=BAD;
      
    } 
    return(retval);
}
/*
 *  Read a line and return the symbol; only knows about
 *  quit and help, but none of the other commands.
 */
int getsymbol ()
{
    extern struct cmdtyp cmdtyp[];
    extern char symbolname[];

    int     i, length, retval = GOT_SYMBOL;
    register char  *q;
    char    line[BUF_LINE];     /* input line */
    char    line2[BUF_LINE];    /* saved version of the input line */

    if (fgets (line, BUF_LINE, stdin) == NULL) 
    {
        printf ("\n");  /* EOF (^D) */
        return (QUIT);
    }

    if (line[0] == '\n') 
    {
        return (NOREPLY);
    }
    if (line[strlen (line) - 1] == '\n') 
    {
        line[strlen (line) - 1] = NULL;
    }
    for (q = line; isspace (*q); q++) 
        ; /* skip spaces - all work done in 'for'  */
    strcpy(line2, line);        /* save original entry, including caps */
    MapLowerCase(line);
    length = strlen(line);
    for (i = 0; cmdtyp[i].cmd_id != EO_LIST ; i++) 
    {
        if (strncmp(cmdtyp[i].cmd_name, line, length) == 0) 
        {
            retval = cmdtyp[i].cmd_id;
            break;
        }
    }

    strcpy(symbolname, line2);  /* save symbol name globaly */
    if ((retval == HELP) || (retval == QUIT)
        || (retval == PRINTER_INFO) || (retval == NO))
    {
        return(retval);         /* return command id only if quit or help */
    } 
    else 
    {
        return (GOT_SYMBOL);    /* else return a symbol */
    }
}

/*
 *  Read a line, decode the command, return command id
 *  This routine knows about the full command set.
 */
int getcmd ()
{
    extern struct cmdtyp cmdtyp[];
    extern char symbolname[];

    int     i, length;
    register char  *q;          /* parse character pointer */
    char    line[BUF_LINE];     /* input line */
    char    tline[BUF_LINE];    /* tmp copy of input line */

    if (fgets (line, BUF_LINE, stdin) == NULL)
    {                           /* EOF (^D) */
        printf ("\n");
        return (QUIT);
    }

    if (line[0] == '\n')
        return (NOREPLY);

    if (line[strlen (line) - 1] == '\n')
        line[strlen (line) - 1] = NULL;

    for (q = line; isspace (*q); q++)/* strip leading blanks */
        ;/* required null statement */
    strcpy (tline, line);
    MapLowerCase (line);

    length = strlen (line);

    /* check against commands in table command */
    for (i = 0 ; cmdtyp[i].cmd_id != EO_LIST ; i++)
    {/* look at the command table until match or EO_LIST is the command id */
        if (strncmp(cmdtyp[i].cmd_name, line, length) == 0)
        {/* return the command id of a valid command */
            return (cmdtyp[i].cmd_id);/* command id */
        }
    }

    /* Not a command so it defaults to being classed as a symbol */
    strcpy (symbolname, tline); /* save symbol name globaly */
    return (GOT_SYMBOL);
}

leave (int status)
{
    exit (status);
}

/*
 *  free nvalue when done
 */
freemem ()
{
    extern struct table tab[];
    int i;

    for (i = 0; tab[i].name != 0; ++i)
    if (tab[i].nvalue != 0)
    free (tab[i].nvalue);
}

/*
 *      finish printer setup
 *      notify user of success
 */
int setupdone()
{
    printf(MSGSTR(MISC_20A,"\nSet up activity is complete for this printer.\n"));
    printf(MSGSTR(MISC_20B,"Verify that the printer works properly by using\n"));
    printf(MSGSTR(MISC_20C,"the lpr(1) command to send files to the printer.\n"));

    return(OK);
}

FILE* 
Popen(char *s1, char *s2)
{
    FILE    *fp;

    close(1);
    fp = popen(s1, s2);
    dup2(3, 1);
    return(fp);
}

/*
 *  Determines if pname is in the buf
 *  needed by DeleteEntry
 */

int findpname (char pname[], char buf[])
{
    int   i,j,done;
    char  *c;

    i = strlen(pname);

    if ((strncmp (pname, buf, i) == 0) && ((buf[i] == ':') || (buf[i] == '|'))) 
    {
        return (TRUE);
    }
    else 
    {
        j = i;
        done = FALSE;
        while (!done) 
        {
            while ((buf[j] != '|') && (buf[j] != ':') && (buf[j] != ' ')) 
                j++;
            if ((buf[j] != ':') && (buf[j] != ' ')) 
            {
                j++;
                c = &buf[j];
                if ((strncmp (pname, c, i) == 0) 
                    && ((buf[i+j] == '|') || (buf[i+j] == ':'))) 
                {
                    return (TRUE);
                }
            }
            else 
            {
                return (FALSE);
            }
        }
    }
}

/*
 *  PrintHelp - Prints the requested help information,
 *              it stops every 23 lines.
 */
int PrintHelp (char* hlpmsg, int start)
{
/*  display the help message screen by screen.  Reset line count displayed
 *  so far if start is TRUE
 */
    int    done, pos;
    char   line[BUF_HELP];
    static int linecnt;

    pos = 0;
    done = FALSE;
    if (start) linecnt = 0;

    GetRows();    /* Do it every time in case window is resized */
    while (sgetline (hlpmsg, line, &pos) && !done) 
    {
        linecnt ++;
        printf ("%s\n", line);
        if (linecnt == rows) 
        {
            printf (MSGSTR(MISC_21,"\nPress 'RETURN' to continue or 'quit RETURN' to quit: "));
            switch (getcmd()) 
            {
              case QUIT:
              case GOT_SYMBOL:
                done = TRUE;
                break;
              case NOREPLY:
              default:
                linecnt = 0;
                break;
            }
        }
    }
    return (OK);
}

/*
 *  sgetline - returns in line the next line in buf, reads until a \n is found.
 *  the \n is changed to a \0. Returns true if a line can be read. False if not.
 */

int sgetline (char *buf, char *str, int *pos)
{
    int   i;

    if (buf[*pos] == '\0') 
    {
        str[0] = '\0';
        return (0);
    }
    else 
    {
        for (i = 0; (str[i] = buf[*pos]) != '\n'; i++, (*pos)++);
        str[i] = '\0';
        (*pos)++;
        return (1);
    }
}

/*
 * GetRows - sets the global variable "rows" to contain the number of rows in
 *           the output window.
 */

int GetRows ()
{
    struct winsize win;

    rows = DEFAULTROWS;
    if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1) 
    {
        if (win.ws_row > 0)
            rows = win.ws_row - 2;
        else
            rows = DEFAULTROWS;
    }
    return (OK);
}

/*
 *  replace "/n" with '/n'
 */
int break_string(char *str)
{
    int pos_old;
    int pos_new = 0;
    int length  = strlen(str);

    for(pos_old = 0 ; pos_old < (length-1) ; /* pos_old will inc in loop */)
    {/* scan the string ... */
        /* check all '\' to see if followed by 'n'. */
        if (str[pos_old] != '\\' || str[pos_old+1] != 'n')
        { /* No? copy character as is */
            str[pos_new++] = str[pos_old++]; 
        }
        else
        { /* Yes? replace with '\n', and skip ahead one extra char */
            str[pos_new++] = '\n';
            pos_old += 2;
        }
    }
    /* terminate the new string */
    str[pos_new] = '\0';
    return (OK);
}

/*
 *  end of misc.c
 */
