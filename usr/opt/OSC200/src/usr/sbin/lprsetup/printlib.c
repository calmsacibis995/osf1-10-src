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
static char *sccsid  =  "@(#)$RCSfile: printlib.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/10/13 14:38:26 $";
#endif

#include <stdio.h>
#ifndef BUF_SIZE
#define BUF_SIZE        4096
#endif

#define MAXHOP  32      /* max number of tc= indirections */

/*
 * Message catalog support 
 */
#include <ctype.h>
#include "lprsetup_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LPRSETUP,n,s)
#ifdef  SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_LPRSETUP_SEC,n,s)
#endif

/*
 *  printlib - routines for dealing with the printer data base
 */

/*
 * Testing - move 'define' outside of comment
#define LOCAL
 */

#ifdef LOCAL
#define E_PRINTCAP "printcap"
#define D_PRINTCAP "lprsetup.dat"
#else
#define E_PRINTCAP "/etc/printcap"
#define D_PRINTCAP "/etc/lprsetup.dat"
#endif 

static char *entry_buf;
static char *defaults_buf;
static int hopcount = 0; /* detect infinite loops in printcap */

/*
 *  prototypes of routines externally accessed
 */
int  pgetent(char *capbuf_ptr, char *name, int use_data); /* get an entry  */
int  pnchktc(int use_data);                  /* check for indirection      */
int  pnamatch(char *nam_ptr, int use_data);  /* match an entries name      */
int  pgetnum(char *id, int use_data);        /* get a numeric entrys value */
int  pgetflag(char *id, int use_data);       /* get a boolean flags value  */
char *pgetstr(char *id, char **area, int use_data);/* get entrys string    */

/*
 *  prototypes of local routines
 */
static char *pdecode(register char *str, char **area);
static char *pskip(char *capbuf_ptr);

/*
 * Get an entry for printer entry in buffer capbuf_ptr,
 * from the printcap file.  Parse is very rudimentary;
 * we just notice escaped newlines.
 */
int pgetent(char *capbuf_ptr, char *name, int use_data)
{
    char *cp;
    int c;
    int i = 0, cnt = 0;
    char ibuf[BUF_SIZE];
    char *cp2;
    int pf;

    pf = 0;
    if (use_data)
    {
        entry_buf = capbuf_ptr;
        pf = open(E_PRINTCAP, 0);
    }
    else
    {
        defaults_buf = capbuf_ptr;
        pf = open(D_PRINTCAP, 0);
    }

    if (pf < 0)
        return (-1);
    for (;;) 
    {
        cp = capbuf_ptr;
        for (;;) 
        {
            if (i == cnt) 
            {
                cnt = read(pf, ibuf, BUF_SIZE);
                if (cnt <= 0) 
                {
                    close(pf);
                    return (0);
                }
                i = 0;
            }
            c = ibuf[i++];
            if (c == '\n') 
            {
                if (cp > capbuf_ptr && cp[-1] == '\\')
                {
                    cp--;
                    continue;
                }
                break;
            }
#ifndef UNM
            if (cp >= capbuf_ptr+BUF_SIZE) 
#else   UNM
            if (cp == capbuf_ptr+BUF_SIZE)
#endif  UNM
            {
                printf(MSGSTR(PRINTLIB_1,"printcap entry too long\n"));
                break;
            } 
            else
            {
                *cp++ = c;
            }
        }
        *cp = 0;

/*
 * The real work for the match.
 */
        if (pnamatch(name, use_data)) 
        {
            close(pf);
            return(pnchktc(use_data));
        }
    }
}

/*
 * pnchktc: check the last entry, see if it's tc=xxx. If so,
 * recursively find xxx and append that entry (minus the names)
 * to take the place of the tc=xxx entry. This allows printcap
 * entries to say "like an HP2621 but doesn't turn on the labels".
 * Note that this works because of the left to right scan.
 */
int pnchktc(int use_data)
{
    register char *p, *q;
    char tcname[32];    /* name of similar printer */
    char tcbuf[BUF_SIZE];
    char *holdentry_buf = use_data ? entry_buf : defaults_buf;
    char *buf_ptr = use_data ? entry_buf : defaults_buf;
    int l;

    p = buf_ptr + strlen(buf_ptr) - 2;  /* before the last colon */
    while (*--p != ':')
    {/* back through the buffer until we find a colon */
        if (p<buf_ptr) 
        {/* there needs be at least one field so this shouldn't happen */
            printf(MSGSTR(PRINTLIB_2,"Bad printcap entry\n"));
            return (0);
        }
    }
    p++;
    /* p now points to beginning of last field */
    /* check if last entry was 'tc' */
    if (p[0] != 't' || p[1] != 'c')
    { /* if not get out of this routine */
        return(1);
    }
    /* get the name to scan for */
    strcpy(tcname,p+3);
    q = tcname;
    /* scan till the end of the field (or end of entry) */
    while (q && *q != ':')
        q++;
    /* terminate the string */
    *q = 0;
    if (++hopcount > MAXHOP) 
    {/* recursion is beyond a reasonable amount */
        printf(MSGSTR(PRINTLIB_3,"Infinite tc= loop in /etc/printcap\n"));
        return (0);
    }
    /* find the entry referenced in the 'tc' symbol */
    if (pgetent(tcbuf, tcname, use_data) != 1)
    { /* could not find it, scat out of here */
        return(0);
    }
    /* scan forwards to skip entry name */
    for (q=tcbuf; *q != ':'; q++)
        ; /* null statement because the for expression does all the work */
    /* see if the indirect reference can be added to the current entry */
    l = p - holdentry_buf + strlen(q);
    if (l > BUF_SIZE) 
    { /* if the indirect reference causes overflow, truncate it */
        printf(MSGSTR(PRINTLIB_1,"printcap entry too long\n"));
        q[BUF_SIZE - (p-buf_ptr)] = 0;
    }
    /* add the indirect entry to the end of the current entry */
    strcpy(p, q+1);
    /* restore the buffer pointer to the correct value */
    if (use_data)
    {
        entry_buf = holdentry_buf;
    }
    else
    {
        defaults_buf = holdentry_buf;
    }
    return(1);
}

/*
 * pnamatch deals with name matching.  The first field of the printcap
 * entry is a sequence of names separated by |'s, so we compare
 * against each such name.  The normal : terminator after the last
 * name (before the first field) stops us.
 */
int pnamatch(char *nam_ptr, int use_data)
{
    register char *Nam_Ptr, *Buf_Ptr;

    Buf_Ptr = use_data ? entry_buf : defaults_buf;
    if (*Buf_Ptr == '#')
        return(0);
    for (;;) 
    {
        for (Nam_Ptr = nam_ptr; *Nam_Ptr && *Buf_Ptr == *Nam_Ptr; 
             Buf_Ptr++, Nam_Ptr++)
            continue;
        if (*Nam_Ptr == 0 && (*Buf_Ptr == '|' || *Buf_Ptr == ':' || *Buf_Ptr == 0))
            return (1);
        while (*Buf_Ptr && *Buf_Ptr != ':' && *Buf_Ptr != '|')
            Buf_Ptr++;
        if (*Buf_Ptr == 0 || *Buf_Ptr == ':')
            return (0);
        Buf_Ptr++;
    }
}

/*
 * Skip to the next field.  Notice that this is very dumb, not
 * knowing about \: escapes or any such.  If necessary, :'s can be put
 * into the printcap or lprsetup.dat file in octal.
 */
static char *
pskip(char *buf_ptr)
{
    while (*buf_ptr && *buf_ptr != ':')
        buf_ptr++;
    if (*buf_ptr == ':')
        buf_ptr++;
    return (buf_ptr);
}

/*
 * Return the (numeric) option id.
 * Numeric options look like
 *      li#80
 * i.e. the option string is separated from the numeric value by
 * a # character.  If the option is not found we return -1.
 * Note that we handle octal numbers beginning with 0.
 */
int pgetnum(char *id, int use_data)
{
    int i, base;
    char *capbuf_ptr = use_data ? entry_buf : defaults_buf;

    for (;;) 
    {
        capbuf_ptr = pskip(capbuf_ptr);
        if (*capbuf_ptr == 0)
            return (-1);
        if (*capbuf_ptr++ != id[0] || *capbuf_ptr == 0 || 
            *capbuf_ptr++ != id[1])
            continue;
        if (*capbuf_ptr == '@')
            return(-1);
        if (*capbuf_ptr != '#')
            continue;
        capbuf_ptr++;
            base = 10;
        if (*capbuf_ptr == '0')
            base = 8;
        i = 0;
        while (isdigit(*capbuf_ptr))
            i *= base, i += *capbuf_ptr++ - '0';
        return (i);
    }
}

/*
 * Handle a flag option.
 * Flag options are given "naked", i.e. followed by a : or the end
 * of the buffer.  Return 1 if we find the option, or 0 if it is
 * not given.
 */
int pgetflag(char *id, int use_data)
{
    char *capbuf_ptr = use_data ? entry_buf : defaults_buf;

    for (;;) 
    {
        capbuf_ptr = pskip(capbuf_ptr);
        if (!*capbuf_ptr)
            return (0);
        if (*capbuf_ptr++ == id[0] && *capbuf_ptr != 0 && *capbuf_ptr++ == id[1]) 
        {
            if (!*capbuf_ptr || *capbuf_ptr == ':')
                return (1);
            else if (*capbuf_ptr == '@')
                return(0);
        }
    }
}

/*
 * Get a string valued option.
 * These are given as
 *      cl=^Z
 * Not much decoding is done on the strings, and the strings are
 * placed in area, which is a ref parameter which is updated.
 * No checking on area overflow.
 */
char *
pgetstr(char *id, char **area, int use_data)
{
    char *capbuf_ptr = use_data ? entry_buf : defaults_buf;

    for (;;) 
    {
        capbuf_ptr = pskip(capbuf_ptr);
        if (!*capbuf_ptr)
            return (0);
        if (*capbuf_ptr++ != id[0] || *capbuf_ptr == 0 || 
            *capbuf_ptr++ != id[1])
            continue;
        if (*capbuf_ptr == '@')
            return(0);
        if (*capbuf_ptr != '=')
            continue;
        capbuf_ptr++;

        return(pdecode(capbuf_ptr,area));
    }
}

/*
 * pdecode does the grunge work to decode the
 * string capability escapes.
 */
static char *
pdecode(register char *str, char **area)
{
    char *cp;
    int c;
    char *dp;
    int i;

    cp = *area;
    while ((c = *str++) && c != ':') 
    {
        *cp++ = c;
    }

    *cp++ = 0;
    str = *area;
    *area = cp;

    return (str);
}
