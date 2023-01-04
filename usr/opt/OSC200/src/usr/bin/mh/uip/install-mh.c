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
/* install-mh.c - initialize the MH environment */
/* 
**Revision History

Sept-23, 1991  Aju John DEC ULTRIX/OSF Engineering   AJ02

By default, MH scan  shows the date in DDMMYY format instead 
of MMDDYY format. So, a change is  made to install-mh.c
to have the format in a file in the MH directory under $HOME.
Also, .mh_profile should have an entry for the format file.
All modifications are between IFNDEF DDMMYY and ENDIF DDMMYY.
To shut off all effects of this modification, define DDMMYY
and rebuild this program.
(OSF-QAR 1337)


Sept-09, 1991 Aju John DEC ULTRIX/OSF Engineering   AJ01

Dash-munging problem while forwarding messages using  mh is
avoided with this revision. However, this disables the burst 
feature of mh, but preserves the dashes.
If BURST is defined, the  burst feature will be re-enabled, but
the dash-munging problem will be present in forwarded messages.
A new file is created in the MH directory (mhl.noformat) to prevent 
line wrapping while forwarding messages. See explanation of BURST 
below the revision history.

Submit approvals to AJ01 and AJ02 were obtained together. So,
SCCS version 3.2 and 3.3 are essentially the same.

** Revision history ends

*/



#ifndef BURST
#define WIDTH "10000"
/*
   Modify WIDTH to the required value, to enable line-wrapping at
   that column position. (Default wrap-around is at col 10000.)
*/
#endif
/*
 * BURST: mh  has a feature called "burst" to separate multiple messages
   in a forwarded mail. Each message is separated initially by a line of
   "-"s beginning at column 0. Forw command in mh modifies this line by 
   pre-pending an extra "-" and a blank before lines that begin with a 
   "-" at col zero. It looks awkward to users and is referred to as the
   dash-munging problem. To overcome this, an extra line needs to be 
   added to .mh_profile. if BURST is defined, this extra line 
   "forw: -nodash -format" is added, but burst command will not work.
   To get the original behavior of mh, BURST should not be defined.
   A file called mhl.noformat will also be created in the MH directory,
   if BURST is not  defined.
*/

#ifndef	lint
static char ident[] = "@(#)$Id: install-mh.c,v 4.3.3.3 1992/02/18 17:53:51 Adam_Elkins Exp $";
#endif	lint

#include "../h/mh.h"
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/*  */

static char *message[] = {
    "Prior to using MH, it is necessary to have a file in your login",
    "directory (%s) named %s which contains information",
    "to direct certain MH operations.  The only item which is required",
    "is the path to use for all MH folder operations.  The suggested MH",
    "path for you is %s/Mail...",
    NULL
};


static char   *geta ();

struct passwd  *getpwuid ();

/*  */

/* ARGSUSED */

main (argc, argv)
int     argc;
char  **argv;
{
    int     autof,
	    i;
    char   *cp,
           *path;
    struct node *np;
    struct passwd *pw;
    struct stat st;

    FILE   *in,
	   *out;
#ifndef BURST
    static char formatfile[]="mhl.noformat";
    FILE *fmtfile;
#endif BURST
#ifndef DDMMYY
    static char scanfile[]="scan.time";
    FILE *scn;
#endif DDMMYY

    invo_name = r1bindex (argv[0], '/');

#ifdef	COMPAT
    if (argc == 2 && strcmp (argv[1], "-compat") == 0) {
	context = "/dev/null";	/* hack past m_getdefs() */
	
	m_getdefs ();
	for (np = m_defs; np; np = np -> n_next)
	    if (uleq (pfolder, np -> n_name)
		    || ssequal ("atr-", np -> n_name)
		    || ssequal ("cur-", np -> n_name))
		np -> n_context = 1;

	ctxpath = getcpy (m_maildir (context = "context"));
	ctxflags |= CTXMOD;
	m_update ();

	if ((out = fopen (defpath, "w")) == NULL)
	    adios (defpath, "unable to write");
	for (np = m_defs; np; np = np -> n_next)
	    if (!np -> n_context)
		fprintf (out, "%s: %s\n", np -> n_name, np -> n_field);
#ifndef BURST /* AJ 01 */
    fprintf (out, "%s: %s %s %s\n", "forw", "-filter", formatfile, "-nodash");
    defpath = concat (mypath, "/", path, "/", formatfile, NULLCP);
    if (stat (defpath, &st) != NOTOK)
      adios (NULLCP, "A file called %s exists. \n \
             Rename it and re-run this program.\n \
             Also delete  ~/.mh_profile file.\n", defpath);
    if ((fmtfile = fopen (defpath, "w")) == NULL)
	adios (defpath, "unable to write");
    fprintf(fmtfile,"extras:nocomponent\n:\nbody:nocomponent,width=%s\n",WIDTH);
    (void) fclose(fmtfile);
#endif BURST

#ifndef DDMMYY /* AJ 02 */
    fprintf (out, "%s: %s %s \n", "scan", "-form", scanfile);
    defpath = concat (mypath, "/", path, "/", scanfile, NULLCP);
    if (stat (defpath, &st) != NOTOK)
      adios (NULLCP, "A file called %s exists. \n \
             Rename it and re-run this program.\n \
             Also delete  ~/.mh_profile file.\n", defpath);
    if ((scn = fopen (defpath, "w")) == NULL)
    	adios (defpath, "unable to write scan format file\n");
    fprintf(scn,"\%4(msg)\%<(cur)+\%| \%>\%<{replied}-\%|\%<{encrypted}E\%| \%>\%>\\\n");
    fprintf(scn,"%%02(mon{date})/%%02(mday{date}) \\\n");
    fprintf(scn,"%%02(hour{date}):%%02(min{date}) \\\n");
    fprintf(scn,"%%<{date} %%|*%%>\\\n");
    fprintf(scn,"%%<(mymbox{from})To:%%14(friendly{to})%%|%%17(friendly{from})%%> \\\n");
    fprintf(scn,"%%{subject}%%<{body}<<%%{body}%%> \n");
    (void) fclose(scn);
#endif DDMMYY

	(void) fclose (out);
	done (0);
    }
#endif	COMPAT

    autof = (argc == 2 && strcmp (argv[1], "-auto") == 0);
    if (mypath == NULL) {	/* straight from m_getdefs... */
	if (mypath = getenv ("HOME"))
	    mypath = getcpy (mypath);
	else
	    if ((pw = getpwuid (getuid ())) == NULL
		    || pw -> pw_dir == NULL
		    || *pw -> pw_dir == NULL)
		adios (NULLCP, "no HOME envariable");
	    else
		mypath = getcpy (pw -> pw_dir);
	if ((cp = mypath + strlen (mypath) - 1) > mypath && *cp == '/')
	    *cp = NULL;
    }
    defpath = concat (mypath, "/", mh_profile, NULLCP);

    if (stat (defpath, &st) != NOTOK)
	if (autof)
	    adios (NULLCP, "invocation error");
	else
	    adios (NULLCP,
		    "You already have an MH profile, use an editor to modify it");

    if (!autof && gans ("Do you want help? ", anoyes)) {
	(void) putchar ('\n');
	for (i = 0; message[i]; i++) {
	    printf (message[i], mypath, mh_profile);
	    (void) putchar ('\n');
	}
	(void) putchar ('\n');
    }

/*  */

    cp = concat (mypath, "/", "Mail", NULLCP);
    if (stat (cp, &st) != NOTOK) {
	if ((st.st_mode & S_IFMT) == S_IFDIR) {
	    cp = concat ("You already have the standard MH directory \"",
		    cp, "\".\nDo you want to use it for MH? ", NULLCP);
	    if (gans (cp, anoyes))
		path = "Mail";
	    else
		goto query;
	}
	else
	    goto query;
    }
    else {
	if (autof)
	    printf ("I'm going to create the standard MH path for you.\n");
	else
	    cp = concat ("Do you want the standard MH path \"",
		    mypath, "/", "Mail\"? ", NULLCP);
	if (autof || gans (cp, anoyes))
	    path = "Mail";
	else {
    query:  ;
	    if (gans ("Do you want a path below your login directory? ",
			anoyes)) {
		printf ("What is the path?  %s/", mypath);
		path = geta ();
	    }
	    else {
		printf ("What is the whole path?  /");
		path = concat ("/", geta (), NULLCP);
	    }
	}
    }

    (void) chdir (mypath);
    if (chdir (path) == NOTOK) {
	cp = concat ("\"", path, "\" doesn't exist; Create it? ", NULLCP);
	if (autof || gans (cp, anoyes))
	    if (makedir (path) == 0)
		adios (NULLCP, "unable to create %s", path);
    }
    else
	printf ("[Using existing directory]\n");

/*  */

    np = m_defs = (struct node *) malloc (sizeof *np);
    if (np == NULL)
	adios (NULLCP, "unable to allocate profile storage");
    np -> n_name = getcpy ("Path");
    np -> n_field = getcpy (path);
    np -> n_context = 0;
    np -> n_next = NULL;

    if (in = fopen (mh_defaults, "r")) {
	m_readefs (&np -> n_next, in, mh_defaults, 0);
	(void) fclose (in);
    }

    ctxpath = getcpy (m_maildir (context = "context"));
    m_replace (pfolder, defalt);
    m_update ();

    if ((out = fopen (defpath, "w")) == NULL)
	adios (defpath, "unable to write");
    for (np = m_defs; np; np = np -> n_next)
	if (!np -> n_context)
	    fprintf (out, "%s: %s\n", np -> n_name, np -> n_field);
#ifndef BURST /* AJ 01 */
    fprintf (out, "%s: %s %s %s\n", "forw", "-filter", formatfile, "-nodash");
    defpath = concat (mypath, "/", path, "/", formatfile, NULLCP);
    if (stat (defpath, &st) != NOTOK)
      adios (NULLCP, "A file called %s exists. \n \
             Rename it and re-run this program.\n \
             Also delete  ~/.mh_profile file.\n", defpath);
    if ((fmtfile = fopen (defpath, "w")) == NULL)
	adios (defpath, "unable to write");
   fprintf(fmtfile,"extras:nocomponent\n:\nbody:nocomponent,width=%s\n",WIDTH);
    (void) fclose(fmtfile);
#endif BURST

#ifndef DDMMYY /* AJ 02 */
    fprintf (out, "%s: %s %s \n", "scan", "-form", scanfile);
    defpath = concat (mypath, "/", path, "/", scanfile, NULLCP);
    if (stat (defpath, &st) != NOTOK)
      adios (NULLCP, "A file called %s exists. \n \
             Rename it and re-run this program.\n \
             Also delete  ~/.mh_profile file.\n", defpath);
    if ((scn = fopen (defpath, "w")) == NULL)
    	adios (defpath, "unable to write scan format file\n");
    fprintf(scn,"%%4(msg)%%<(cur)+%%| %%>%%<{replied}-%%|%%<{encrypted}E%%| %%>%%>\\\n");
    fprintf(scn,"%%02(mon{date})/%%02(mday{date}) \\\n");
    fprintf(scn,"%%02(hour{date}):%%02(min{date}) \\\n");
    fprintf(scn,"%%<{date} %%|*%%>\\\n");
    fprintf(scn,"%%<(mymbox{from})To:%%14(friendly{to})%%|%%17(friendly{from})%%> \\\n");
    fprintf(scn,"%%{subject}%%<{body}<<%%{body}%%> \n");
    (void) fclose(scn);
#endif DDMMYY


    (void) fclose (out);

    done (0);
}

/*  */

static char *geta () {
    register char  *cp;
    static char line[BUFSIZ];

    (void) fflush (stdout);
    if (fgets (line, sizeof line, stdin) == NULL)
	done (1);
    if (cp = index (line, '\n'))
	*cp = NULL;
    return line;
}
