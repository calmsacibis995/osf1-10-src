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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/main.c,v 1.1.4.2 1993/06/25 18:33:11 Paul_Henderson Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include "smdata.h"
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>

extern OptionsRec options;

/*ifdef ONE_DOT_TWO */
Boolean sm_inited = FALSE;

#ifdef HYPERHELP
extern void help_error();
#endif

printenv ()
{
	extern char **environ;
	int index;
	char **tmp;

	tmp = environ;

	for(index = 0; environ[index] ; index++)
	   fprintf (stderr, "%d - %s\n", index, tmp[index]);
}

setenv(var, value)
char *var, *value;
{
	extern char **environ;
	int index;
	static Boolean first = TRUE;

	if (first) {
		char **tmp;
		first = FALSE;
		for(index = 0; environ[index] ; index++);
		tmp = (char **) XtMalloc((index+1) * sizeof(char *));
		for(index = 0; environ[index] ; index++)
		  tmp[index] = environ[index];
		tmp[index] = environ[index];
		if (options.session_debug) fprintf (stderr, "%s\n",tmp[index]);
		environ = tmp;
	}

	for (index = 0; environ [index] != NULL;) {
	    if (strncmp (environ [index], var, strlen (var)) == 0) {
		environ[index] = XtMalloc(strlen(var) + strlen(value) + 1);
		strcpy (environ [index], var);
		strcat (environ [index], value);
		return;
	    }
	    index ++;
	}

	if ((environ = (char **)XtRealloc((char *)environ,
					  sizeof(char *) * (index+2)))
	     == NULL)
		exit(0);

	environ [index] = (char *) XtMalloc (strlen (var) + strlen (value) + 1);
	strcpy (environ [index], var);
	strcat (environ [index], value);
	environ [++index] = NULL;
}


static void decwtk_errs(message)
String *message;
{
	fprintf(stderr, "X Toolkit Error: %s\n", (char *)message);
	exit(1);
}


#ifdef COMBINE
session_main(argc, argv)
#else
main(argc, argv)
#endif
char *argv[];
int argc;
{
	int status, i;
	int fds[2];
	void reaper();
	char *display = NULL;
	char *display_arg = NULL;
	int doneWithCopyright = FALSE;
	XEvent event;

	smdata.err_window_id = 0;
	smdata.caution_id = 0;
	if(argc > 1) {
		for(i = 1; i < argc ; i++)
		  if(index(argv[i], ':') != NULL) {
			  display_arg = display = argv[i];
			  break;
		  }
	}
        if(getpgrp(0) == 0 || getpgrp(0) == 1 ) setpgrp(0, getpid());

	/*
	 * Fetch the user's auth information immediately, for use by pause.
	 * Ignore the return value from checkpass, since all that is
	 * desired is the side effect of caching this information.
	 * Change the effective uid so that it is no longer root, and
	 * change the effective gid so that it is no longer authread.
	 * The root access is needed to read /etc/srvtab, and
	 * the group access is needed to read /etc/auth.
	 *
	 * N.B.  If checkpass fails to retrieve the auth information
	 * for any reason (e.g., kerberos timeout), the user may not
	 * be able to unpause the screen.  The reason for this is that
	 * this process drops its setuid and setgid privileges after
	 * the first call to checkpass, so later calls to checkpass
	 * may not be able to retrieve the auth information.  The
	 * session manager should refuse to pause the screen if this
	 * is the case, but currently it does not do this.
	 */
	/* checkpass will return an error status, 
	 * but at this point we don't care
	 */
	
	(void)root_checkpass("", &status);
	(void)checkpass(getuid(),"", &status);
	(void)seteuid(getuid());
	(void)setegid(getgid());

        signal(SIGPIPE, SIG_IGN);
        signal(SIGCHLD, reaper);

 	pipe(fds);

	XtSetErrorHandler((XtErrorHandler)decwtk_errs);
	if (display_arg) setenv("DISPLAY=", display_arg);
	if (!sm_init(argc, argv)) exit(1);

	if (DisplayString(XtDisplay(smdata.toplevel)) == NULL
	    || strcmp(DisplayString(XtDisplay(smdata.toplevel)), "") == 0)
	{
     		if(display != NULL) setenv("DISPLAY=", display);
		else setenv("DISPLAY=", ":0"); /* hack hack */
	}
	else setenv("DISPLAY=", DisplayString(XtDisplay(smdata.toplevel)));
	display = getenv("DISPLAY");

	get_pty_console(display_arg != NULL ? display_arg : display);

	doinit(smdata.toplevel);
	put_property(XtDisplay(smdata.toplevel));

	/* force all stdout and stderr messages to the console logger */
	StartXcons();

	/* Moved start of wm to here to fix CLD with mwm involving error messages */
	startwm();
	if (options.session_waitforwm) waitforwm(smdata.toplevel);

	/* create the control panel */
	smattr_get_values();

	if (!create_panel()) exit(1);
        control_window = XtWindow(smdata.toplevel);

	doMotd();
	if (options.session_debug) printenv (); 
	AutoStart();

	/* leave in for compatibility A.R.  */
	for (i=0; i<smsetup.terms; i++) start_terminal();
	for (i=0; i<smsetup.vues; i++) start_vue();

	/* this needs to be taken out when startupfilename
	   resource becomes redundant
	*/
	
	DoStartUp();

	execute_pointer((unsigned int)mpt_mask);

#ifdef HYPERHELP
	DXmHelpSystemOpen(&help_context, smdata.toplevel, dxsession_help,
			  help_error, "Help System Error");
#endif

	/* get events forever.  We get out by the end_session button */
	while (1) {			
    	    XtNextEvent (&event);

    	    if ((!doneWithCopyright) && ((event.type == ButtonPress)
	         || (event.type == KeyPress))) 
            {
	        takeDownCopyright ();
	        doneWithCopyright = 1;
            }
            XtDispatchEvent (&event);
        }
}

void    display_message(status, other_text)
unsigned        int     status;
char    *other_text;
{
	if (other_text != NULL) {
		if (other_text[strlen(other_text) -1] != '\n') 
			fprintf(stdout, "%s\n", other_text);
		else fprintf(stdout, "%s", other_text);
		fflush(stdout);
        }
}

static int	takeDownCopyright()
{
Arg     arglist[1];

XtSetArg(arglist[0], XmNtitleString, get_drm_message (k_sm_title_msg));
XtSetValues(smdata.toplevel, arglist, 1);
}

doMotd()
{
    FILE   *fd;
    int    c;

    /* don't want motd */
    if (access(".hushlogin", R_OK) == 0) return;

    if ((fd = fopen("/etc/motd", "r")) == NULL) return;

    while ((c=getc(fd)) != EOF) putc (c, stdout);
    fclose(fd);
    fflush(stdout);
}

StartXcons()
{
	FILE *xconsFptr;
	char comm[512];

	sprintf(comm, "/usr/bin/X11/dxconsole -geometry =+%d+%d", 
		smsetup.x + 30, smsetup.y + 80);

	if ((xconsFptr = popen(comm, "w")) != NULL) {;
		dup2(fileno(xconsFptr), 1);
		dup2(fileno(xconsFptr), 2);
	}
}

int
pause_session(w, arg, reason) 
Widget *w;
caddr_t arg;
Widget *reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the pause screen if this is the first time, otherwise
**	just pop it up.  Grab the keyboard and pointer for security.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    char *new_text;
    char *text_ptr;
    char *resource;    
    int size;
    int i;
    int j;
    char 	*pause_session_rep;
    XrmValue	pause_session_value;
    char 	*pause_text_rep;
    XrmValue	pause_text_value;
    char	*root_passwd_rep;
    XrmValue	root_passwd_value;
    char format[] = "%s -xrm '*PauseLabel.labelString: %s' -xrm '*rootPasswd: %s'";
    int	value[4];

   XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge],
		  "DXsession.pauseSession", NULL, 
		  &pause_session_rep, &pause_session_value);
   XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge],
		  "DXsession.pause_text", NULL, 
		  &pause_text_rep, &pause_text_value);
   XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge],
		  "DXsession.rootPasswd", NULL, 
		  &root_passwd_rep, &root_passwd_value);
  
   /* Take care of the case where the user included an apostrophe in text */

   i = strlen(pause_text_value.addr);
   for (text_ptr = pause_text_value.addr, j = 0; i; i--, text_ptr++)
       if (*text_ptr == '\'')
	   j++;
   size = pause_text_value.size + (3 * j) + 1;
   new_text = XtMalloc(size);

   size += sizeof format + pause_session_value.size + root_passwd_value.size;
   resource = XtMalloc(size);

   if ((resource == NULL) || (new_text == NULL))
     {
       fprintf(stderr, "Unable to allocate storage for pause command\n");
       return(NULL);
     }

   strcpy(new_text, pause_text_value.addr);
   
   /* Replace the apostrophes with the escaped 047 equivalent */
   
   while (j) {
       text_ptr = strrchr(new_text, '\'');
       memmove(text_ptr + 4, text_ptr + 1, strlen(text_ptr));
       memcpy(text_ptr, "\\047", 4);
       j--;
   }

   sprintf(resource, format, pause_session_value.addr,
	   pause_text_value.addr, root_passwd_value.addr);
   fprintf(stderr, "Executing resource %s \n", resource);
   sprintf(resource, format, pause_session_value.addr, new_text, 
	   root_passwd_value.addr);
   doexecstr(resource, 0, TRUE);
   fprintf(stderr, "Executed pause resource \n");
   XtFree(resource);
   XtFree(new_text);
}

