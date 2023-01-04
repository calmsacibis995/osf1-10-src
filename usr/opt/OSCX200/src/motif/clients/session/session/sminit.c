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
#include <stdio.h>
#include <string.h>
#include <sys/file.h>

#include "smdata.h"
#include "smconstants.h"
#include "smresource.h"

#include	<X11/Xlib.h>
#include	<X11/Xatom.h>
#include	<X11/Intrinsic.h>
#include	<X11/StringDefs.h>
#include	<Xm/Xm.h>
#include	<Mrm/MrmPublic.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#ifndef SS_NORMAL
#define SS_NORMAL 1
extern void PhilSave();
extern void MikeyRemove();
#endif
#include	<X11/Xresource.h>
#include	<X11/Vendor.h>
#include	<X11/keysym.h>
#include	<X11/keysymdef.h>

extern  int     sm_set_property();
extern  int     sm_change_property();
extern  int     sm_save_database();
extern  void    IconInit();

extern	int property_handler();
extern  XtEventHandler unixunidle();

XmString get_drm_message ();
void sm_put_resource();

extern char *user_resource;

#define XtNdebug	"debug"
#define XtCDebug	"Debug"
#define XtNsecurity	"security"
#define XtCSecurity	"Security"
#define XtNwindowManagerName	"windowManagerName"
#define XtCWindowManagerName	"WindowManagerName"
#define XtNwindowManagerNameDefault	"windowManagerName_default"
#define XtCWindowManagerNameDefault	"WindowManagerName_default"
#define XtNrootPasswd	"rootPasswd"
#define XtCRootPasswd	"RootPasswd"
#define XtNpauseSaver	"pauseSaver"
#define XtCPauseSaver	"PauseSaver"
#define XtNwaitForWM	"waitForWM"
#define XtCWaitForWM	"WaitForWM"
#define XtNpauseFile	"pauseFile"
#define XtCPauseFile	"PauseFile"

char smdb_file[256];
int ten = 10;

OptionsRec options;

static XtResource applicationResources[] = {
	{XtNdebug, XtCDebug, XtRBoolean, sizeof(Boolean),
	   XtOffsetOf(OptionsRec, session_debug), XtRString, "FALSE"},
	{XtNsecurity, XtCSecurity, XtRBoolean, sizeof(Boolean),
	   XtOffsetOf(OptionsRec, session_security), XtRString, "TRUE"},
	{XtNwindowManagerName, XtCWindowManagerName, XtRString, sizeof(char *),
	   XtOffsetOf(OptionsRec, session_wm), XtRString,
	   "System Default"},
	{XtNwindowManagerNameDefault, XtCWindowManagerNameDefault, XtRString,
	   sizeof(char *), XtOffsetOf(OptionsRec, session_default_wm),
	   XtRString, "/usr/bin/X11/mwm"},
	{XtNrootPasswd, XtCRootPasswd, XtRBoolean, sizeof(Boolean),
	   XtOffsetOf(OptionsRec, session_rootpasswd), XtRString,
	   "FALSE"},
	{XtNpauseSaver, XtCPauseSaver, XtRInt, sizeof(int),
	   XtOffsetOf(OptionsRec, session_pausesaver), XtRInt,
	   (caddr_t) &ten},
	{XtNwaitForWM, XtCWaitForWM, XtRBoolean, sizeof(Boolean),
	   XtOffsetOf(OptionsRec, session_waitforwm), XtRString,
	   "TRUE"},
	{XtNpauseFile, XtCPauseFile, XtRString, sizeof(char *),
	   XtOffsetOf(OptionsRec, session_pausefile), XtRString, ""},
};

int exposeHandler(w, client_data, event)
Widget w;
caddr_t client_data;
XEvent *event;
{
  Position x, y;
  XtTranslateCoords(smdata.toplevel, 0, 0, &x, &y);

  smsetup.x = x;
  smsetup.y = y;
  XtRemoveEventHandler(smdata.toplevel, XtAllEvents, True,
		       (XtEventHandler)exposeHandler, 0);
}

int	sm_init(argc,argv)
int argc;
char **argv;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called to initialize a session.  We need to open the display,
**	set up the icons, get the property off of the root window,
**	and set up the session according to the user's customization.
**
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
Display		*pDisplay;
unsigned	int	status, i;
XWMHints	wmhints;
Arg     arglist[10];
int	argcnt = 0;
unsigned int	mask;
int numscreens;
Visual	*type;
Screen	*the_screen;
char    *copy_resource = "Session Manager.copytitle";
XrmDatabase app_database;
char        *the_rep;
XrmValue        the_value;
char *titlestr, *iconstr;
char errout[80];

/* DRM heirarchy file */
static char *db_filename_vec[] = 
	{"DXsession.uid"};
static int db_filename_num = (sizeof db_filename_vec/sizeof db_filename_vec[0]);

smdata.resource_changed = 0;

#ifdef DOHELP
DXmInitialize();
#endif

MrmInitialize();

#ifdef DOPRINT
dxPrscInitialize();
#endif

display_id = XOpenDisplay("");
if (display_id == NULL) {
  fprintf(stderr, "Can't open display!\n");
  exit(1);
}

/* get the resource data base we will use,from the property on the
    root window */
if (sm_set_property(user_filename, display_id, 
                    &xrmdb.xrmlist[system_color_type][rdb_merge], 0) == 0)
    sm_set_property(system_generic_filename, display_id, 
  		    &xrmdb.xrmlist[system_color_type][rdb_merge], 0);

/* open the display with the toolkit.  Creates a toplevel shell
   which is the base of the applications 

   The intrinsics will now read the property off of the root window 
   and use it as resource data base */

smdata.toplevel = XtInitialize(NULL, "DXsession", NULL, 0, &argc, argv);

XtAddEventHandler(smdata.toplevel, StructureNotifyMask, True,
		  (XtEventHandler)exposeHandler, 0);
XtAddEventHandler(smdata.toplevel, EnterWindowMask, True,
		  (XtEventHandler)unixunidle, 0);

XtGetApplicationResources(smdata.toplevel, &options, applicationResources,
			  XtNumber(applicationResources), NULL, 0);

if (MrmOpenHierarchy(db_filename_num, db_filename_vec, NULL,
                     &s_DRMHierarchy) != MrmSUCCESS)
{
	fprintf(stderr, "Can\'t open DRM hierarchy");
        exit(1);
}

/* initialize some important global variables */
/* we'll make the assumptions once, and then if they change, we only
    need to change them here */
XCloseDisplay(display_id);

display_id = XtDisplay(smdata.toplevel);
screen = XDefaultScreen(display_id);
root_window = XRootWindow(display_id, screen);

/* determine if we are on a black and white or color system */
system_color_type = determine_system_color(display_id, screen);

sprintf(smdb_file, "/tmp/smdb-%s.defaults", XDisplayString(display_id));
XrmPutFileDatabase(XtDatabase(display_id), smdb_file);

xrmdb.xrmlist[system_color_type][rdb_color] = 0;
xrmdb.xrmlist[system_color_type][rdb_generic] = 0;
xrmdb.xrmlist[system_color_type][rdb_merge] = 0;

/* set up security for the workstation */
if (options.session_security) XEnableAccessControl(display_id);

/* Get the number of screens on the system */
numscreens = XScreenCount(display_id);
/* get rid of loginout structures.  Loginout exited with
    retain temporary close down mode so that the server
    would not reset.  Now the server is full of garbage.
    So, lets get rid of it */
XKillClient(display_id, AllTemporary);

/* Set up the icons */
IconInit();

XmStringGetLtoR(get_drm_message(k_sm_iconname_msg),
	def_char_set, &iconstr);

argcnt = 0;
XtSetArg(arglist[argcnt], XtNiconPixmap, smdata.icon); argcnt++;
XtSetArg(arglist[argcnt], XtNiconName, iconstr); argcnt++;
XtSetArg(arglist[argcnt], XtNallowShellResize, True); argcnt++;
XtSetArg(arglist[argcnt], XmNtitleString, get_drm_message (k_sm_copytitle_msg));
argcnt++;

app_database=XtDatabase(display_id);
XrmGetResource(app_database, copy_resource, NULL, &the_rep, &the_value);
if (the_value.size > 0) {
    XtSetArg(arglist[argcnt], XtNtitle, the_value.addr);
    argcnt++;
}
else {
    XmStringGetLtoR(get_drm_message (k_sm_copytitle_msg),
		def_char_set, &titlestr);
    XtSetArg(arglist[argcnt], XtNtitle, titlestr);
    argcnt++;
}

XtSetValues(smdata.toplevel, arglist, argcnt);

/* set up an event handler for icon/window property change */
XtAddEventHandler(smdata.toplevel, PropertyChangeMask,
		  False, (XtEventHandler)property_handler, 0 );

/* moved further up, but leaving here also for now .... A.R. */
/* determine if we are on a black and white or color system */
system_color_type = determine_system_color(display_id, screen);

the_screen = ScreenOfDisplay(display_id, screen);
type = XDefaultVisualOfScreen(the_screen);
/* save type for color selection later */
vtype = type->class;

sm_get_property();

/* update the setup structures based on the property on the root window */
updatesetup();

/* set up the keyboard and display to match users resources */
/* note that we won't do the pointer until mainloop starts.  This
    is so that the cursor will stay like a watch until the
    control panel is actually visible */
mask = mkeyboard_mask;

/* set the help key modified */
set_help_mod(display_id);
execute_keyboard((unsigned int)mask);
execute_display((unsigned int)mdisplay_mask);

return(1);
}


int sm_get_property()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get the property off of the root window.  Initialize the
**	resource databases, and set up security.  If the property
**	wasn't there for some reason, put it there and continue
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
unsigned    int	i;
Atom	return_type;
int	return_format;
unsigned    long    return_nitems, return_after;
unsigned    char    *return_prop = NULL;
unsigned    int	status;
unsigned    int j;

/* zero out the xrm data */
for (i=0; i<num_system_types; i++)
    {
    for (j=0; j<num_databases; j++)
	xrmdb.xrmlist[i][j] = 0;
    }

/* Changed 2000 to 20000 for quick 2.1 fix. */
i = XGetWindowProperty(display_id, root_window, XA_RESOURCE_MANAGER, 0, 
	20000, 0, XA_STRING, &return_type, &return_format, &return_nitems,
	&return_after, &return_prop);
if (return_prop != NULL)
    {
    /* property is already there, but we need to initialize all of
	the databases */
    xrmdb.xrmlist[system_color_type][rdb_merge] = XtDatabase(display_id);
    XFree(return_prop);
    }
else sm_use_memory();

/* Initialize the security access list */
InitServerHosts();
securitysetup.reset_listbox = 1;
}

static int create_file (file_id)
int file_id;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	If we try to use the system resoure files, and they are not
**	there, then we will create a file in sys$login with the
**	defaults from our resource table in the file.
**
**  FORMAL PARAMETERS:
**
**	file_id1 - The file id of the created file for color or bw resources
**	file_id2 - The file id for storing sm_general resources
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
unsigned	int	i;
char	*linefeed = "\n";
char	*space = " ";
char	*colan = ":";
unsigned	int	length = 0;
	
/* put out the default table */
for (i=0; i< num_elements; i++)
    {
    if(def_table[i].name == NULL)
      continue;
    if (def_table[i].name) length = strlen(def_table[i].name);

    /* write out the resource name followed by a colon */
    write(file_id, def_table[i].name, length);
    write(file_id, colan, 1);
    write(file_id, space, 1);
    
    /* now write out the value and a linefeed */
    if (def_table[i].def_value) length = strlen(def_table[i].def_value);
    write(file_id, def_table[i].def_value, length);
    write(file_id, linefeed, 1);
    }
return(1);
}



static  int     create_buf(buf_ptr)
char **buf_ptr;
{
unsigned        int     i;
unsigned        int     length = 0;
char    *buffer;
char    *linefeed = "\n";
char    *space = " ";
char    *colan = ":";

        /* put out the default table */
for (i=0; i< num_elements; i++)
        {
        if (def_table[i].name) length = length + strlen(def_table[i].name);
        if (def_table[i].def_value)
                length = length + strlen(def_table[i].def_value);
        /* one for the space and one for the linefeed, and one for the
                colan */
        length = length + 3;
        }

*buf_ptr = (char *)malloc(length);
buffer = *buf_ptr;

/* initialize first char of buffer to NULL */
*buffer = 0;

for (i=0; i< num_elements; i++)
        {
        length = ((def_table[i].name) ? strlen(def_table[i].name) : 0);
        strncpy(buffer, def_table[i].name, length);
        buffer = buffer + length;
        strncpy(buffer, colan, 1);
        buffer++;
        strncpy(buffer, space, 1);
        buffer++;
        length =
           ((def_table[i].def_value) ? strlen(def_table[i].def_value) : 0);
        strncpy(buffer, def_table[i].def_value, length);
        buffer = buffer + length;
        if (i != (num_elements - 1))
                {
                strncpy(buffer, linefeed, 1);
                buffer++;
                }
        else
                *buffer = NULL;
        }
return(1);
}

int	sm_get_resource(name, value)
char	*name;
char	*value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	name - A pointer to the resource we are looking for.  
**	value - A pointer to a string to return the value of the resource
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	Returns length of the string or 0 if there was a problem
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
char *the_rep;
XrmValue	the_value;
char	*get_ptr;

/* don't want to look for * when looking for the resource. Strip that off */
get_ptr = name;
if ((*get_ptr) == '*') get_ptr++;

/* Get the resource out of the xdefaults database */
XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge], get_ptr, 
		NULL, &the_rep, &the_value);
if (the_value.size != 0) {
    /* It was there, copy the result to return storage */
    strncpy(value, the_value.addr, the_value.size);
    value[the_value.size] = '\0';
}
return(the_value.size);
}

int sm_get_int_resource(index, intptr)
unsigned    int	index;
int intptr[4];
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its integer value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	intptr - Place to return the integer(s) that are the resource value
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
char	svalue[256];
unsigned    int	size;

/* Get the resource from the database */
size = sm_get_resource(def_table[index].name,svalue);
/* If we couldn't find the resource, then use the default value.  The
   default value will be stored in the table */
if (size == 0)
    {
    size = sm_convert_int(index, NULL, intptr);
    }
else
    {
    /* We found the resource.  Convert the string to an int */
    size = sm_convert_int(index, svalue, intptr);
    if (size == 0)
	{
	/* If we have problems converting, use the default table value */
	size = sm_convert_int(index, NULL, intptr);
	}
    }
return(1);
}

int sm_get_string_resource(index, charptr)
unsigned    int	index;
char	*charptr;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	charptr - Place to return the string that is the resource value
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	size - The size of the string returned
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
unsigned    int	size;

/* Get the resource */
size = sm_get_resource(def_table[index].name,charptr);
if (size == 0)
    {
    /* If couldn't find it, just use the default table value */
#ifdef __osf__
      strcpy(charptr, def_table[index].def_value);
      size = strlen(charptr) ;
#else
      size = strcpy(charptr, def_table[index].def_value);
#endif
    }
return(size);
}

int sm_get_any_resource(index, charptr, intptr)
unsigned    int	index;
char	*charptr;
int intptr[4];
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Look in the resource table to find the format of a particular
**	resource.  If it is integer or string, call the appropriate
**	routine and return the value as either an integer or string.
**
**  FORMAL PARAMETERS:
**
**	index - Index into the resource table of the resource we want
**	charptr - Place to return string resources
**	intptr - Place to return integer resources
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
unsigned    int	size;

/* String resources */
if (def_table[index].format == tstring)
    {
    size = sm_get_string_resource(index, charptr);
    }
/* Integer resources */
if (def_table[index].format == tint)
    {
    size = sm_get_int_resource(index, intptr);
    }
/* Two integers */
if (def_table[index].format == t2int)
    {
    size = sm_get_int_resource(index, intptr);
    }
return(1);
}

void	sm_put_resource(index, value)
unsigned    int	index;
char	*value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Store a resource value.  We maintain two databases.  One with
**	all of the resources merged together, and one with the current
**	values seperated by file they are stored in (general or color
**	specific).   Update both databases.
**
**  FORMAL PARAMETERS:
**
**	index - index into the resource table
**	value - The string value to store for the resource
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
char	*name;
unsigned int	i,numscreens,j;

/* Get the name of the resource out of the table */
name = def_table[index].name;
j = def_table[index].rdb_index;
XrmPutStringResource(&xrmdb.xrmlist[system_color_type][rdb_merge], name, value);
PhilSave( name, value, def_table[index].rdb_index);
}

void	sm_remove_resource(index)
unsigned    int	index;
{
char	*name;
unsigned int	i,numscreens,j;

/* Get the name of the resource out of the table */
name = def_table[index].name;
j = def_table[index].rdb_index;

XrmPutStringResource(&xrmdb.xrmlist[system_color_type][rdb_merge], name, "");
MikeyRemove( name, def_table[index].rdb_index);
}



void	sm_put_int_resource(index, value)
unsigned    int	index;
int value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Store a resource value.  We maintain two databases.  One with
**	all of the resources merged together, and one with the current
**	values seperated by file they are stored in (general or color
**	specific).   Update both databases.
**
**  FORMAL PARAMETERS:
**
**	index - index into the resource table
**	value - The integer value to store for the resource
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
char	astring[256];
unsigned    int	status;

/* change the integer value into a string */
status = int_to_str(value, astring, sizeof(astring));

/* Now store the string */
if (status == SS_NORMAL)
    {
    sm_put_resource(index, astring);
    }
}

int	sm_put_database()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Write out all of the databases to the correct files.   We need
**	to write the color specific files and then the general file.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	1 - everything went smoothly
**	0 - there was an error somewhere
**
**  SIDE EFFECTS:
**
**--
**/
{

    extern Boolean mergedatabase;
    if(options.dontPutDatabase) return(0);
    if(mergedatabase)
          do_merge_database(xrmdb.xrmlist[system_color_type][rdb_merge],
			    user_resource);
    else
          XrmPutFileDatabase(xrmdb.xrmlist[system_color_type][rdb_merge],
			     user_resource);
    return(1);
}

int     sm_switch_database(filename)
char    *filename;
{
unsigned        int     status;
int rm;  /* A.R.  to make things compile */
  int size, value[4];
  char svalue[256];
  XrmDatabase fdb;

fdb = XrmGetFileDatabase(smdb_file);
if (!fdb) {
  display_drm_message(0, k_resource_norestore_msg);
  return(0);
}

status = sm_set_property(filename, XtDisplay(smdata.toplevel),
                         &xrmdb.xrmlist[system_color_type][rdb_merge],   &rm);
/* A.R.  2d now */
if (!status) return(status);

/* Initialize the security access list */
InitServerHosts();
securitysetup.reset_listbox = 1;

  if (xrmdb.xrmlist[system_color_type][rdb_merge] != XtDatabase(display_id)) {
    XtFree((char *)xrmdb.xrmlist[system_color_type][rdb_merge]);
  }
  xrmdb.xrmlist[system_color_type][rdb_merge] = fdb;
  size = sm_get_any_resource(ilanguage, svalue, value);
  if (size != 0) {
    setenv("LANG=", svalue);
  }

updatesetup();
exposeHandler(NULL, NULL, NULL);
return(1);
}

int	sm_save_database()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Calls routines to store the current database in the correct
**	files.
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
int	file_id;
unsigned    int	ret_status;

if (smdata.resource_changed)
	{
	  XrmPutFileDatabase(xrmdb.xrmlist[system_color_type][rdb_merge],
			     smdb_file);
	ret_status = sm_put_database();
	if (ret_status != 1)
	    return(0);
	}

smdata.resource_changed = 0;
return(1);
}

sm_use_memory()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	For some reason, the users and the system session manager
**	default files are not usable.  We will create files with
**	the correct resource values in the user's directory and
**	display a warning that the system files are corrupt.
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
unsigned	int	status,i;
int	file_id1 = 0;
int	file_id2 = 0;
XrmDatabase tempDB;
Atom    return_type;
int     return_format;
unsigned    long    return_nitems, return_after;
unsigned char    *return_prop = NULL;
int rm;

/* display a warning to the user */
put_error(0, k_system_resource_msg);

/* for some reason there was not a good system manager default file,
   so we need to create a user's file with the default table */
file_id1 = -1;
file_id1 = creat(user_resource,0644);

if (file_id1 == -1)
	{
	/* if still problems, punt */
	put_error(0, k_resource_create_msg);
	return;
	}

/* write the databases to the files */
/* A.R.  ok to pass two but not using file_id2 .... */
status = create_file(file_id1, file_id2);
if (!status)
	{
	put_error(0, k_resource_create_msg);
	return;
	}

/* Close the files */
close(file_id1);

status = sm_set_property(user_resource, display_id, 
		&xrmdb.xrmlist[system_color_type][rdb_merge], &rm);
if (!status)
    return;

i = XGetWindowProperty(display_id, root_window,
        XA_RESOURCE_MANAGER, 0, 20000, 0, XA_STRING,
        &return_type, &return_format, &return_nitems,
        &return_after, &return_prop);

if (return_prop != NULL)
    {
    /* property is already there, but we need to initialize all of
        the databases */
              xrmdb.xrmlist[system_color_type][rdb_merge] = XrmGetStringDatabase((char *)return_prop);

              XFree(return_prop);
      }
}

int set_help_mod (d)
    Display  *d;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Set the help modifier
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
XModifierKeymap *mods = XGetModifierMapping (d);
KeySym helpSym = XStringToKeysym("Help");
KeyCode help = 0;

if (helpSym != NoSymbol) help = XKeysymToKeycode (d, helpSym);

if (help == 0 || mods == NULL ||
    (mods = XInsertModifiermapEntry(mods, help, Mod5MapIndex)) == NULL)
        XSetModifierMapping(d, mods); 

if (mods != NULL) XFreeModifiermap(mods);
}

XmString get_drm_message (msgnbr)
    int msgnbr;
{
    static XmString *table = NULL;
    caddr_t value;
    MrmCode type;
    int status;
    if (table == NULL)
      {
    	status = MrmFetchLiteral (s_DRMHierarchy, "k_sm_message_table",
		 XtDisplay(smdata.toplevel), &value, &type);
	table = (XmString *)value;
      }
    return (table[msgnbr]);
}

int	sm_get_screen_resource(name, screen_type, value)
char	*name;
unsigned    int	screen_type;
char	*value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	name - A pointer to the resource we are looking for.  
**	value - A pointer to a string to return the value of the resource
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	Returns length of the string or 0 if there was a problem
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
char *the_rep;
XrmValue	the_value;
char	*get_ptr;
int type;

/* We don't want to look for * when looking for the resource.  Strip that
   off */
get_ptr = name;
if ((*get_ptr) == '*')
    {
    get_ptr++;
    }

type = screen_type;
/* Get the resource out of the xdefaults database */
if (xrmdb.xrmlist[screen_type][rdb_merge] == 0)
    {
    type = system_color_type;
    }

XrmGetResource(xrmdb.xrmlist[type][rdb_merge], get_ptr, NULL, &the_rep, &the_value);
if (the_value.size != 0)
	{
	/* It was there, copy the result to return storage */
	strncpy(value, the_value.addr, the_value.size);
	value[the_value.size] = '\0';
	return(the_value.size);
	}
return(0);
}

int sm_get_int_screen_resource(index, screen_type, intptr)
unsigned    int	index;
unsigned    int	screen_type;
int intptr[4];
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its integer value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	intptr - Place to return the integer(s) that are the resource value
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
char	svalue[256];
unsigned    int	size;

/* Get the resource from the database */
size = sm_get_screen_resource(def_table[index].name,screen_type, svalue);
/* If we couldn't find the resource, then use the default value.  The
   default value will be stored in the table */
if (size == 0)
    {
    size = sm_convert_int(index, NULL, intptr);
    }
else
    {
    /* We found the resource.  Convert the string to an int */
    size = sm_convert_int(index, svalue, intptr);
    if (size == 0)
	{
	/* If we have problems converting, use the default table value */
	size = sm_convert_int(index, NULL, intptr);
	}
    }
return(1);
}

logErr(str)
char *str;
{
    FILE *errorfile;

    if ((errorfile = fopen("/tmp/DXsession.log", "a")) != NULL) {
    	fprintf(errorfile, "DXsession:  %s", str);
    	fflush(errorfile);
    	fclose(errorfile);
    }
}
