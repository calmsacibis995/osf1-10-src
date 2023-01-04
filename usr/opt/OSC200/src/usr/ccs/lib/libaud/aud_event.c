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
static char *rcsid = "@(#)$RCSfile: aud_event.c,v $ $Revision: 1.1.6.6 $ (DEC) $Date: 1993/08/02 19:58:27 $";
#endif

#include <sys/audit.h>
#include <stdio.h>

#define SITE_EVENTS     "/etc/sec/site_events"
#define TERMINATOR      ';'
#define EVENT_ALIASES   "/etc/sec/event_aliases"


#define FSCAN_SITE_EVENTS(fp,fmt,eventptr,eventnum,delmtr) \
    { \
        if ( fscanf ( fp, fmt, eventptr, eventnum, delmtr ) == EOF ) { \
            fclose(fp); \
            return(-1); \
        } \
    }


/* get indx'th alias name from EVENT_ALIASES list */
aud_aliasent ( indx, buf, len )
int indx;
char *buf;
int len;
{
    FILE *fp;
    char alias[AUD_MAXEVENT_LEN];
    char ch;
    int i, j;

    /* sanity check */
    if ( indx < 0 ) return(-1);

    /* open alias list */
    if ( (fp = fopen ( EVENT_ALIASES, "r" )) == NULL )
        return(-1);


    /* get specified alias name */
    for ( ; indx >= 0; indx-- ) {

        /* read " alias : " */
        *alias = '\0';
        if ( (j = fscanf ( fp, " %[^#: \t]%*1s", alias )) == EOF ) {
            fclose ( fp );
            return(-1);
        }
        if ( j == 0 ) indx++;   /* no alias found */

        /* read rest of entry " ... {\n,\}" */
        ch = '\0';
        if ( fscanf ( fp, "%*[^\n\\]%c", &ch ) == EOF ) {
            fclose ( fp );
            return(-1);
        }

        /* and continuation lines - read "\n ... {\n,\}" */
        for ( ; ch == '\\'; ) {
            ch = '\0';
            if ( fscanf ( fp, "%*[\n]%*[^\n\\]%c", &ch ) == EOF ) {
                fclose ( fp );
                return(-1);
            }
        }
    }

    strncpy ( buf, alias, len );
    fclose ( fp );

    return(0);
}


/* get indx'th event for given alias in EVENT_ALIASES list */
aud_alias_event ( name, indx, buf, len )
char const *name;
int indx;
char *buf;
int len;
{
    FILE *fp;
    char alias[AUD_MAXEVENT_LEN];
    char event[AUD_MAXEVENT_LEN];
    char str[2];
    char ch;
    int i;

    /* sanity check */
    if ( indx < 0 ) return(-1);

    /* open alias list */
    if ( (fp = fopen ( EVENT_ALIASES, "r" )) == NULL )
        return(-1);


    /* find matching alias */
    for ( ;; ) {

        /* read " alias : " */
        *alias = '\0';
        if ( fscanf ( fp, " %[^#: \t]%*1s", alias ) == EOF ) {
            fclose ( fp );
            return(-1);
        }

        /* check for match */
        if ( strcmp ( alias, name ) == 0 ) {
            /* check for continuation line */
            if ( fscanf ( fp, " %[\\]%*[\n]", &ch ) == EOF ) {
                fclose ( fp );
                return(-1);
            }
            break;
        }

        /* read rest of entry " ... {\n,\}" */
        ch = '\0';
        if ( fscanf ( fp, "%*[^\n\\]%c", &ch ) == EOF ) {
            fclose ( fp );
            return(-1);
        }

        /* and continuation lines - read "\n ... {\n,\}" */
        for ( ; ch == '\\'; ) {
            ch = '\0';
            if ( fscanf ( fp, "%*[\n]%*[^\n\\]%c", &ch ) == EOF ) {
                fclose ( fp );
                return(-1);
            }
        }
    }


    /* got match - get specified event name */
    for ( ; indx >= 0; indx-- ) {
        *event = *str = '\0';

        /* check for event name in "'s */
        if ( fscanf ( fp, " %[\"]", str ) == 1 ) {
            if ( fscanf ( fp, "%[^\"]%*[\"]", event ) == EOF ) {
                fclose ( fp );
                return(-1);
            }
        }

        /* just event name */
        else if ( fscanf ( fp, " %[^ \t\n\\]", event ) == EOF ) {
            fclose ( fp );
            return(-1);
        }

        /* read [blanks, tabs], '\' or '\n' */
        if ( fscanf ( fp, "%*[ \t]" ) == EOF
        || fscanf ( fp, "%[\n\\]", str ) == EOF ) {
            fclose ( fp );
            return(-1);
        }

        /* end-of-line, continuation line checks */
        if ( *str == '\\' ) fscanf ( fp, " %*[\n]" );
        else if ( *str == '\n' && indx ) {
            fclose ( fp );
            return(-1);
        }
    }

    strncpy ( buf, event, len );
    fclose ( fp );

    return(0);
}


/* given event/subevent, load names into eventp/subeventp; return 0/-1 */
int aud_sitevent ( event, subevent, eventp, subeventp )
int event, subevent;
char *eventp, *subeventp;
{
    FILE *fp;
    unsigned int ev_num;
    char del[2];
    char buf[AUD_MAXEVENT_LEN];

    if ( (fp = fopen ( SITE_EVENTS, "r" )) == NULL ) return(-1);

    /* find event */
    for ( ev_num = -1; ev_num != event; ) {
        FSCAN_SITE_EVENTS ( fp, "%s%u%1s", eventp, &ev_num, del );
        if ( ev_num != event )
            for ( ; del[0] != TERMINATOR; )
                FSCAN_SITE_EVENTS ( fp, "%127s%u%1s", buf, &ev_num, del );
    }

    /* find subevent */
    for ( ev_num = -1; ev_num != subevent; ) {
        FSCAN_SITE_EVENTS ( fp, "%s%u%1s", subeventp, &ev_num, del );
        if ( ev_num != subevent && del[0] == TERMINATOR ) {
            fclose(fp);
            return(-1);
        }
    }

    fclose(fp);
    return(0);
}


/* given event/subevent names, load numbers into ev_nump/subev_nump; return 0/-1 */
int aud_sitevent_num ( eventp, subeventp, ev_nump, subev_nump )
char const *eventp, *subeventp;
int *ev_nump, *subev_nump;
{
    FILE *fp;
    char eventp_l[AUD_MAXEVENT_LEN];
    char del[2];

    if ( (fp = fopen ( SITE_EVENTS, "r" )) == NULL ) return(-1);

    /* find event */
    do {
        FSCAN_SITE_EVENTS ( fp, "%127s%u%1s", eventp_l, ev_nump, del );
        if ( strcmp ( eventp, eventp_l ) )
            for ( ; del[0] != TERMINATOR; )
                FSCAN_SITE_EVENTS ( fp, "%127s%u%1s", eventp_l, subev_nump, del );
    } while ( strcmp ( eventp, eventp_l ) );

    /* find subevent */
    if ( subeventp == '\0' ) *subev_nump = -1;
    else do {
        FSCAN_SITE_EVENTS ( fp, "%127s%u%1s", eventp_l, subev_nump, del );
        if ( strcmp ( subeventp, eventp_l ) && del[0] == TERMINATOR ) {
            fclose(fp);
            return(-1);
        }
    } while ( strcmp ( subeventp, eventp_l ) );

    fclose(fp);
    return(0);
}
