/* #module TEA_File.c "X0.0" */
/*
 *  Title:	TEA_File.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	This module contains the routines which handle the operations
 *	in the DECterm application's "File" menu.
 *
 *  Procedures contained in this module:
 *
 *  	Exported routines:
 *		file_new_cb
 *		file_open_cb
 *		file_save_cb
 *		file_saveas_cb
 *		file_revert_cb
 *		file_exit_cb
 *		file_open_fs_cb
 *		file_saveas_fs_cb
 *                      
 *	Internal routines:
 *		apply_defaults_to_filename
 *		get_DECterm_database
 *		file_open
 *
 *  Author:	Tom Porcher  10-Mar-1988
 *
 *  Modification history:
 *
 * Alfred von Campe     15-Oct-1993     BL-E
 *	- Write out entire resource database on U*IX systems.
 *
 * Alfred von Campe     01-Aug-1993     BL-D
 *	- Stub out XrmRemoveResource() for VMS as well as OSF/1.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *
 * Aston Chan		04-Mar-1993	V1.2/Bl2
 *	- Type ApplicationShellWidget is no longer available in V1.2.
 *	  Type Widget is good enough for our purpose.
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *      - Merged in changes from Tin SSB version that somehow got lost in
 *        shuffle.  Basically we fork a process that does a setuid(getuid())
 *        before writing out a resource file, since dxterm runs with suid root,
 *        and it may not have write access to a directory that is NFS mounted.
 *
 * Aston Chan		20-Aug-1992	 Post V1.1
 *	- Merge Shai's fix to make it built on UWS Motif 1.1.3.  Fix is to
 *	  change cs to XmString cs in get_DECterm_database().
 *
 * Eric Osman           11-June-1992     Sun
 *      - Add some casting to satisy C compiler
 *      - For Sun, include param.h for symbol MAXSYMLINKS
 *
 * Alfred von Campe     02-Apr-1992	Ag/BL6.2.1
 *	- Change XrmFreeDatabase() to the supported XrmDestroyDatabase().
 *      - Add XrmCopyDatabase() that will work under R5 (U*IX only for now).
 *	- Stub out XrmRemoveResource(), which doesn't seem to be needed and
 *	  is broken in R5.
 *
 * Aston Chan		29-Dec-1991	V3.1
 *	- Shai's fix.  Call adjust_title_iconName() no matter
 *	  XrmGetFileDatabase() returns NULL or not.  This fixes an AccVio
 *	  if in Hebrew environment but no default file exists.
 *
 * Aston Chan		15-Nov-1991	V3.1
 *	- Use XmStringFree() to free an XmString instead of XtFree().
 *
 * Alfred von Campe     01-Nov-1991	Hercules/1 BL5
 *	- Check for symbolic links when doing a Save [Named] Options.  Since
 *	  dxterm is installed with suid root, it was possible to overwrite
 *	  files like /etc/passwd.  Fixed for SSLRT #0107.
 *
 * Aston Chan		1-Sep-1991	Alpha
 *	- Add <> and .h to #include's.  Complained by DECC compiler Release 10.
 *
 * Eric Osman		28-May-1991	V3.0
 *	- Don't XrmFreeDatabase after XrmMergeDatabases, since merge already
 *	  freed it.
 *
 * Alfred von Campe     24-May-1991     V3.0
 *      - Free charsets returned by XmStringGetNextSegment to plug small leak.
 *
 * Alfred von Campe     04-Feb-1991     T3.0
 *      - Change free to XtFree.
 *
 * Bob Messenger	 8-Sep-1990	X3.0-7
 *	- Free memory returned by XmStringGetNextSegment.
 *
 * Bob Messenger	26-Aug-1990	X3.0-6
 *	- Make file_initialize() easier to debug by breaking it up so that only
 *	  one routine is called per statement.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	02-Jul-1990	X3.0-5
 *	- Fix bug in Motif conversion: first argument to XmStringGetNextSegment
 *	  should be context, not &context.
 *
 * Mark Woodbury	25-May-1990 X3.0-3M
 *	- Motif update
 *
 * Bob Messenger	28-May-1989	X2.0-13
 *	- Report error if saving default in non-existent directory on VMS.
 *	- Convert fprintf's to calls to log_message, so the messages can be
 *	  flushed.
 *
 * Bob Messenger	26-May-1989	X2.0-12
 * 	- Update current_rdb correctly when saving settings.
 *
 * Bob Messenger	20-May-1989	X2.0-11
 *	- Don't exit on fetch errors, just return.
 *
 * Bob Messenger	14-May-1989	X2.0-10
 *	- Convert printf calls to fprintf on stderr.
 *
 * Bob Messenger	28-Apr-1989	X2.0-8
 *	- Temporary fix for Use Saved Settings From...: don't call
 *	  DwtCSgetUniformText
 *
 * Bob Messenger	21-Apr-1989	X2.0-7
 *	- Call process_exit with a VMS status code (or 0 on Ultrix)
 *	  instead of exit(0).  This change was previously made
 *	  19-Apr-1989 for X2.0-6, but I'm making it again so I can
 *	  include the V1.1-3 changes in V2.
 *
 * Bob Messenger	20-Apr-1989	V1.1-3
 *	- Fix mips compilation errors
 *	- Use XrmPutFileDatabase instead of X$RM_PUT_FILE_DATABASE on
 *	  all non-VMS systems, not just mips.  This means DECterm can't
 *	  put up a warning box if it gets an error writing the file.
 *	- Fix file_save_cb and file_saveas_cb, by making the saved rdb
 *	  become the current_rdb.
 *
 * Bob Messenger	15-Feb-1989	X1.1-1
 *	- Ultrix compiler can't handle initialized auto arrays.
 *
 * Eric Osman		10-Nov-1988	v1.0
 *	- Put up warning window if can't write saved setup file
 *
 * Tom Porcher		20-Oct-1988	X0.5-4
 *	- Change VMS file names to DECW$TERMINAL_*.DAT.
 *
 * Tom Porcher		 7-Sep-1988	X0.5-0
 *	- copy XtGetRootDirName() into this module as get_home_dir().
 *
 * Eric Osman		01-Jul-1988	X0.4-31
 *	- Fix accvio in "use saved settings"
 *
 * Eric Osman		30-Jun-1988	X0.4-30
 *	- Fix Ultrix compiler errors
 *
 * Eric Osman (uh oh!)	21-Jun-1988	X0.4-30
 *	- Use fparse to apply filename defaults on vms, use XtGetRootDirName
 *	  on ultrix
 *
 * Tom Porcher		20-Apr-1988	X0.4-14
 *	- Changed XtFree()s of XrmDatabases to XrmFreeDatabase (defined as Xfree).
 *
 * Tom Porcher		20-Apr-1988	X0.4-10
 *	- added default file name if none specified to file_initialize().
 *	- added default path to apply_defaults_to_filename().
 *
 * Peter Sichel         11-Apr-1988     X0.4-7
 *      - changed to convert widget id's to streams instead
 *        of relying on callback tag not available from UIL.
 *
 * Tom Porcher		 6-Apr-1988	X0.4-7
 *	- Added "format" parameter to FileSelection widget.
 *
 */


#include "mx.h"

#ifdef VMS_DECTERM
globalvalue DECW$_CANT_FETCH_WIDGET;
#else
#define DECW$_CANT_FETCH_WIDGET 0
#endif

#define MAX_FILENAME_SIZE 100
#ifdef VMS_DECTERM
#define DECtermFileType        ".DAT"
#define DECtermFileNameMask    "DECW$TERMINAL_*.DAT"
#define DECtermDefaultFileName "DECW$TERMINAL_default.DAT"
#define DECtermDefaultPath "DECW$USER_DEFAULTS:"

#include <rms.h>
#else
#ifdef VXT_DECTERM
#define DECtermFileType        ".DAT"
#define DECtermFileNameMask    "DECTERM_*.DAT"
#define DECtermDefaultFileName "DECTERM_default.DAT"
#define DECtermDefaultPath "DECW$USER_DEFAULTS:"

#include "msgboxconstants.h"
#include "file.h"

#else VXT_DECTERM
#define DECtermFileType        ""
#define DECtermFileNameMask    ""
#define DECtermDefaultFileName "DXterm"
#define DECtermDefaultPath ""
#include "error.h"
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <DXm/DECspecific.h>
#endif VXT_DECTERM
#endif VMS_DECTERM

/* in Xrm_undefined.c */
#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
extern XrmDatabase      XrmCopyDatabase();
#else
static XrmDatabase	XrmCopyDatabase();
#endif
#ifdef VXT_DECTERM
extern XrmDatabase VxtrmGetAllDatabases();
extern XrmDatabase VxtrmGetDefaultSystemDatabase();
#endif VXT_DECTERM

/* in Widget_Resources.c */
extern XrmDatabase	GetWidgetTreeDatabase();
extern void		PutWidgetTreeDatabase();
extern void		GetResourcesFromDatabase();
extern void		PutResourcesIntoDatabase();

/* other TEA routines */
extern void		close_stream();
extern STREAM           *convert_widget_to_stream();

#ifdef VXT_DECTERM
#include <vxtrm.h>
#include <vxtconfig.h>
#include <vxtdadfileutil.h>
#endif VXT_DECTERM

extern void		_DECwTermSetTitle();
extern void		_DECwTermSetIconName();
static void		adjust_title_iconName();
static void		XrmRemoveResource();

/*
 * Global data
 */

#ifdef VXT_DECTERM
globalref char def_sys_db_is_set;	/*1 = defaults read from DECterm
					       widgets are put into xrm system
					       default. This shoud only be
					       done once */
#endif VXT_DECTERM
/* DRM */
globalref MrmHierarchy s_MRMHierarchy;    /* DRM database id */
globalref MrmType *dummy_class;           /* and class var */


/* from DECwCSmisc.c */

static
int DwtCSgetUniformText(cs, charset, direction, text)
XmString 	    *cs;
XmStringCharSet     charset;
XmStringDirection   direction;
char	**text;
{
XmStringContext     cont;
Boolean		    done, separator;
int		    ret_code;
char		    *stext;
XmStringCharSet     scharset;
int		    text_len, stat;
XmStringDirection   sdir;


XmStringInitContext(&cont,cs);
*text = NULL;

ret_code = TRUE;
text_len = 0;

done=FALSE;
while(!done)
    {
    stat = XmStringGetNextSegment(cont, &stext, &scharset, &sdir, &separator);
    if (stat) 
	{
	if( (charset != scharset) || (direction != sdir ))
	    ret_code = FALSE;
	else
	    text_len += strlen(stext);
	XtFree( stext );
	XtFree( scharset );
	}
    else 
	done = TRUE;
}

*text = XtMalloc(text_len+1);
**text = '\0';
done=FALSE;

XmStringInitContext (&cont,cs);
while(!done)
    {
    stat = XmStringGetNextSegment (cont, &stext, &scharset, &sdir, &separator);
    if (stat) 
	{
	strcat( *text, stext);
	XtFree( stext );
	XtFree( scharset );
	}
    else 
	done = TRUE;
}
return(ret_code);
}

/*
 * Here's a procedure to set one argument of a widget.  This routine
 * takes care of forming a compound string for you, creating the argument
 * list, doing the setarg, and freeing up the string storage.
 */
void set_one_string_value (w, item, string)
	Widget w;
	String item;
	char *string;
{
    Arg		arglist[1];
    XmString value_cs;

    value_cs =  XmStringCreate(string, XmSTRING_DEFAULT_CHARSET);

    XtSetArg( arglist[0], item, value_cs );
    XtSetValues( w, arglist, 1 );

    XmStringFree( value_cs );
}

/*
 * get_text( comp_string )
 *	Returns the text from a compound string.
 *	Returns the null string if the compound string is null or invalid.
 *	The string is allocated by this routine so it must be deallocated
 *	with XtFree().
 */
char*
get_text( cs )
    XmString cs;
{
    char        *ret_text;
    XmStringContext   context;
    char	*text;
    long	lang,rend;
    int		stat;
    Boolean	done, separator;
    XmStringCharSet     charset;
    XmStringDirection   dir;

    ret_text = NULL;

    if( XmStringInitContext(&context, cs) == TRUE) {
	ret_text = XtMalloc( 1 );
	*ret_text = '\0';
	while ( XmStringGetNextSegment( context, &text, &charset, &dir,
	    &separator )) {
	    ret_text = XtRealloc( ret_text, strlen(ret_text)+strlen(text)+1 );
	    strcat( ret_text, text );
	}
	XmStringFreeContext( context );
    }

    XtFree(charset);
    return ( ret_text ); 
}


/* Some error macros, noops for now, should be expanded to put up
 * confirmation box.  One kind just displays message, other kind appends
 * message with system error message.
 */

#define tell_error(text) 0
#define tell_reason(text,code) 0

/*
 * get_home_dir() - get home directory
 * This was XtGetRootDir().
 */
#if !defined(VMS_DECTERM) && !defined (VXT_DECTERM)
String get_home_dir(buf)
     String buf;
{
     uid_t uid;
     extern char *getenv();
     extern uid_t getuid();
     extern struct passwd *getpwuid();
     struct passwd *pw;
     register char *ptr;

     if((ptr = getenv("HOME")) != NULL) {
         (void) strcpy(buf, ptr);
	 buf += strlen(buf);
	 *buf = '/';
	 buf++;
	 *buf = '\0';
	 return buf;
     }

     if((ptr = getenv("USER")) != NULL) pw = getpwnam(ptr);
     else {
	 uid = getuid();
	 pw = getpwuid(uid);
     }
     if (pw) {
	 (void) strcpy(buf, pw->pw_dir);
	 buf += strlen(buf);
	 *buf = '/';
	 buf++;
	 *buf = '\0';
	 return buf;
     }

     return buf;
}
#endif

/*
 * apply_defaults_to_filename( filename, new_filename )
 */
static int
apply_defaults_to_filename( filename, new_filename, option_string )
    char	*filename,
    		*new_filename,
		*option_string;
{

	new_filename[0] = '\0';

#ifdef VMS_DECTERM
/*
 * Until toolkit is fixed to not insist on feeding us a string with
 * a version number, we assume the version number is there because of
 * toolkit, not because user typed it, so we remove it if this is
 * an output parse.
 */
	{
	char (*parsed_filename)[MAX_FILENAME_SIZE], *(fparse ()), *fields;

	extern noshare int fparse_sts;

	fields = 0;
	if (strcmp /* we reall want str_includes */ (option_string,
	    "ofp") == 0)
	fields = "node,device,directory,name,type";

	parsed_filename = fparse (filename, DECtermDefaultPath,
	    DECtermFileType, fields, option_string);

	if (parsed_filename == NULL)
	    {
	    if (fparse_sts == RMS$_NORMAL)
		tell_error ("Failed to parse filename, no memory.\n");
	    else tell_reason ("Failed to parse filename, system reason is:\n",
		fparse_sts);
	    return FALSE;
	    }
	else
	    {
	    strncpy (new_filename, parsed_filename, MAX_FILENAME_SIZE);
	    XtFree (parsed_filename);
	    }
	}
#else
#ifndef VXT_DECTERM
	if (strchr (filename, '/') == 0) get_home_dir(new_filename);
	else new_filename[0] = 0;
	strcat (new_filename, filename);
#endif VXT_DECTERM
#endif VMS_DECTERM
	return TRUE;
}

/*
 * get_DECterm_database( stm )
 */
static XrmDatabase
get_DECterm_database( stm )
    STREAM	*stm;
{
    XrmDatabase	rdb;

    rdb = NULL;
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
    PutResourcesIntoDatabase( stm->terminal, &rdb, stm->file.default_rdb );
    PutResourcesIntoDatabase( stm->parent, &rdb, stm->file.default_rdb );
#else
    PutResourcesIntoDatabase( stm->terminal, &rdb, NULL );
    PutResourcesIntoDatabase( stm->parent, &rdb, NULL );
#endif

    /* add other widgets we want to preserve */
               
/* Remove Shell width & height from the default database.  This is required
 * because they are 0 before the shell is realized, and using zero in
 * a SetValues will cause the toolkit to hang.
 */
    {
	char resource_name[100];
	char *title, *iconName, *fc;
	XmString cs;
	long len, status;
	Arg arglist[4];
	Atom titleEncoding, iconNameEncoding, XA_COMPOUND_TEXT =
	    XInternAtom( XtDisplay( stm->parent ), "COMPOUND_TEXT", False );
	XtSetArg( arglist[0], XtNtitle, &title );
	XtSetArg( arglist[1], XtNtitleEncoding, &titleEncoding );
	XtSetArg( arglist[2], XtNiconName, &iconName );
	XtSetArg( arglist[3], XtNiconNameEncoding, &iconNameEncoding );
	XtGetValues( stm->parent, arglist, 4 );
	if ( titleEncoding == XA_COMPOUND_TEXT ) {
	    sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
	    XrmRemoveResource( &rdb, resource_name );
	    cs = XmCvtCTToXmString( title );
	    fc = (char *) DXmCvtCStoFC( cs, &len, &status );
	    XrmPutStringResource( &rdb, resource_name, fc );
	    XmStringFree( cs );
	    XtFree( fc );
	}
	if ( iconNameEncoding == XA_COMPOUND_TEXT ) {
	    sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
	    XrmRemoveResource( &rdb, resource_name );
	    cs = XmCvtCTToXmString( iconName );
	    fc = (char *) DXmCvtCStoFC( cs, &len, &status );
	    XrmPutStringResource( &rdb, resource_name, fc );
	    XmStringFree( cs );
	    XtFree( fc );
	}
	sprintf( resource_name, "%s.width", DECTERM_APPL_CLASS );
	XrmRemoveResource( &rdb, resource_name );
	sprintf( resource_name, "%s.height", DECTERM_APPL_CLASS );
	XrmRemoveResource( &rdb, resource_name );
	sprintf( resource_name, "%s.x", DECTERM_APPL_CLASS );
	XrmRemoveResource( &rdb, resource_name );
	sprintf( resource_name, "%s.y", DECTERM_APPL_CLASS );
	XrmRemoveResource( &rdb, resource_name );
    }


    return (rdb);
}

/*
 * file_open( stm, filename, ignore_error )
 */
static
int file_open( stm, filename, ignore_error )
    STREAM	*stm;
    char	*filename;
    Boolean	ignore_error;
{
    XrmDatabase file_rdb;
    XrmDatabase new_rdb;
    char	new_filename[MAX_FILENAME_SIZE];

    if (filename != NULL && *filename != '\0') {
	if ( ! apply_defaults_to_filename( filename, new_filename, "" ) )
	    file_rdb = NULL;
	else
	    file_rdb = XrmGetFileDatabase( new_filename );
        if (file_rdb != NULL) {
	    new_rdb = XrmCopyDatabase( stm->file.default_rdb );
	    XrmMergeDatabases( file_rdb, &new_rdb );
	    PutWidgetTreeDatabase( stm->parent, new_rdb );
	    adjust_title_iconName( stm->parent, stm->terminal );
	    if (stm->file.current_rdb != NULL) {
	        XrmDestroyDatabase( stm->file.current_rdb );
	    }
	    stm->file.current_rdb = new_rdb;
	}
/* Shai, 29-dec-1991 */
/* Do adjust_title_iconName even if file_rdb is NULL. */
	else {
	    adjust_title_iconName( stm->parent, stm->terminal );
	}

	if (file_rdb != NULL || ignore_error) {
	    if (stm->file.current_filename != NULL) {
	        XtFree( stm->file.current_filename );
	    }
	    stm->file.current_filename = XtMalloc( strlen(new_filename)+1 );
	    strcpy( stm->file.current_filename, new_filename );
        } else {
	/* can't open file */
	warn_window (stm, "file_read_warning", new_filename);
	return 0;
        }
    }
    return 1;
}

/*
 * file_intialize( stm, filename )
 */
void
file_initialize( stm, filename )
    STREAM	*stm;
    char	*filename;
{
    XrmDatabase xt_database;
    XrmDatabase decterm_database;
    XrmDatabase default_rdb, file_rdb;
    char *title, *iconname, resource_name[100];

/* current_rdb == NULL means use default_rdb */

    stm->file.current_rdb = NULL;

#ifdef VXT_DECTERM
    stm->file.current_filename = NULL;
#endif VXT_DECTERM

/* Set up reference database for get_DECterm_database() */

#ifdef VXT_DECTERM

    stm->file.default_rdb = get_DECterm_database( stm );

    /* only want to call VxtrmPutDefaultSystemDatabase() once after the first
	stream has been created */

    if (def_sys_db_is_set == False)
    {
	/* "Save Options" saves the difference between what's in the
	   DECterm database versus the default system database.  If a 
	   user changes the window title and saves it, the DECterm database
	   is different from the default system database, and therefore
	   the difference is saved.  If a user brings up multiple windows
	   using the default title, each window would have a different 
	   default title.  When a user executes "Save Options", the
	   DECterm database is different from the system database, but in
	   this case, it is undesirable to save the difference.  So need
	   to do some tricks to save, restore the correct default title.

	   Here is the algorithm:
	
	   stm->default_db_title = "VXT DECterm" 
	   stm->default_title = the transport + "VXT DECterm" + a node
	   	name + the decterm window number (for example:
	   	"Lat VXT DECterm on GWEN 1")
	   The default system database contains "VXT DECterm" 
	   (stm->default_db_title).  Before a window comes up, it compares
	   the title in the DECterm widget	with "VXT DECterm", if it 
	   matches, it will put up the title in the format of 
	   "Lat VXT DECterm on GWEN 1" (stm->default_title).  When a user
	   applies "Save Options", the title in the DECterm widget is
	   compared with "Lat VXT DECterm on GWEN 1", if it's the same,
	   "VXT DECterm" is written to the system database.  This
	   technique also applies to "Restore System Options" and 
	   "Restore Options".
	*/

	default_rdb = XrmCopyDatabase( stm->file.default_rdb );
	
        title = XtNewString( stm->default_db_title );
        sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
        XrmPutStringResource( &default_rdb, resource_name, title );

        iconname = XtNewString( stm->default_db_title );
        sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
        XrmPutStringResource( &default_rdb, resource_name, iconname );

	/* attach database to VXT system database */

    	VxtrmPutDefaultSystemDatabase(default_rdb);

        /* Attaching database to the display */

        VxtDisplayInitialize( stm->display );

	if (default_rdb != NULL) 
	{
	    XrmDestroyDatabase( default_rdb );
	}

	def_sys_db_is_set = True;
    }

    xt_database = XtDatabase( stm->display );
    file_rdb = XrmCopyDatabase( xt_database );
    if (file_rdb != NULL) {
	XrmMergeDatabases( file_rdb, &stm->file.default_rdb );

	/* "Restore System Options" restore title and icon name to 
	   "VXT local DECterm", need to attach window number to it */

        sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
	save_default_title_iconname( stm, stm->file.default_rdb, resource_name,
					1 );
        sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
	save_default_title_iconname( stm, stm->file.default_rdb, resource_name,
					1 );

	PutWidgetTreeDatabase( stm->parent, stm->file.default_rdb );
        if (stm->file.current_rdb != NULL)
	    XrmDestroyDatabase( stm->file.current_rdb );
	stm->file.current_rdb = XrmCopyDatabase(stm->file.default_rdb);
    }

#else VXT_DECTERM
    xt_database = XtDatabase( stm->display );
    stm->file.default_rdb = XrmCopyDatabase( xt_database );
    decterm_database = get_DECterm_database( stm );
    XrmMergeDatabases( decterm_database, &stm->file.default_rdb );

/* Open initial configuration file */

    file_open( stm,
	filename != NULL && *filename != '\0' ?
	    filename : DECtermDefaultFileName, TRUE );
#endif VXT_DECTERM
}

/*
 * file_destroy( stm )
 *
 * deallocate all resources for this stream.
 */
void
file_destroy( stm )
    STREAM	*stm;
{
    if (stm->file.current_rdb != NULL)
	XrmDestroyDatabase( stm->file.current_rdb );

    if (stm->file.default_rdb != NULL)
	XrmDestroyDatabase( stm->file.default_rdb );

    if (stm->file.current_filename != NULL)
	XtFree( stm->file.current_filename );
}

/*
 * file_new_cb( w, tag, call_data )
 */
void
file_new_cb( w, tag, call_data )
    Widget	w;
    caddr_t	tag;
    caddr_t	call_data;
{
    STREAM	*stm;
    XrmDatabase rdb;
    char resource_name[100];

    stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
    rdb = VxtrmGetDefaultSystemDatabase();

    /* set default title name and icon name */

    sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
    XrmPutStringResource( &rdb, resource_name, stm->default_title );

    sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
    XrmPutStringResource( &rdb, resource_name, stm->default_title );

    PutWidgetTreeDatabase( stm->parent, rdb );

    if (rdb)
        XrmDestroyDatabase( rdb );

#else VXT_DECTERM    
    PutWidgetTreeDatabase( stm->parent, stm->file.default_rdb );
#endif VXT_DECTERM
    adjust_title_iconName( stm->parent, stm->terminal );
}

/*
 * file_open_fs_cb( w, tag, call_data )
 */
void
file_open_fs_cb( w, tag, call_data )
    Widget	w;
    caddr_t	*tag;
    XmFileSelectionBoxCallbackStruct
		*call_data;
{
    STREAM	*stm;
    stm = convert_widget_to_stream(w);

    if (call_data->reason == XmCR_OK) {
	char *text;
	int status;

        status = file_open( stm, (text = get_text(call_data->value)), FALSE );
	XtFree( text );
	if (status) XtUnmanageChild( w );
    }

    else XtUnmanageChild( w );
}

/*
 * set_filter_filename sets up filespec to be expanded into a list
 * of choices for opening or saving setup files.
 *
 * For VMS, we specify directory it as logical name DECtermDefaultPath.
 *
 * For Ultrix, we call XtGetRootDirName to get home directory.
 *
 * For both operating systems, we append DECtermFileNameMask.
 */
void
set_filter_filename( w)
    Widget    w;
{
    Arg		arglist[1];
    char	filename[MAX_FILENAME_SIZE];

#ifdef VMS_DECTERM
	strcpy (filename, DECtermDefaultPath);
#else
#ifndef VXT_DECTERM
	get_home_dir(filename);
#endif
#endif

	strcat (filename, DECtermFileNameMask);

	set_one_string_value (w, XmNdirMask, filename);
}

	
/*
 * file_open_cb( w, tag, call_data )
 */
void
file_open_cb( w, tag, call_data )
    Widget	w;            
    caddr_t	tag;
    caddr_t	call_data; 
{
    STREAM	*stm;

    stm = convert_widget_to_stream(w);

    /* check if we need to fetch the widget from disk */
    if (stm->file.open_fs == 0)
        {
        /* fetch file selection box from DRM */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "file_open_fs",
               stm->parent,
               & stm->file.open_fs,
               & dummy_class       
               )
           != MrmSUCCESS)
            {   
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
	set_filter_filename (stm->file.open_fs);
        }

    /* initialize widget */
    set_one_string_value (stm->file.open_fs, XmNvalue,
	stm->file.current_filename);

    XtUnmanageChild( stm->file.open_fs );
    XtManageChild( stm->file.open_fs );
}

/*
 * file_saveas_fs_cb( w, tag, call_data )
 */
void
file_saveas_fs_cb( w, tag, call_data )
    Widget	w;
    caddr_t     tag;
    XmFileSelectionBoxCallbackStruct
		*call_data;
{
    STREAM	*stm;
    XrmDatabase	rdb, new_rdb;
    int		status;
    char	filename[MAX_FILENAME_SIZE];

    stm = convert_widget_to_stream(w);

    if (call_data->reason == XmCR_OK) {
	char *text;

	status = apply_defaults_to_filename(
				    (text = get_text(call_data->value)),
				    filename, "ofp" );	/* output file parse */
	XtFree( text );
	if (!status)
	    {
	    warn_window (stm, "file_write_warning", filename);
	    return;
	    }
#if !defined (VMS_DECTERM) && !defined(VXT_DECTERM)
     /*
      * DECterm runs with effective uid root, so we need to ensure
      * the directory is writeable by the real uid before trying to
      * save database
      */
     if (directory_writable(filename))
#endif
     {
#ifdef VMS_DECTERM
	 int filename_desc[2];

	 filename_desc[0] = strlen(filename);
	 filename_desc[1] = (int) filename;;
#endif
	 if ( stm->file.current_rdb != NULL )
	    {
	    XrmDestroyDatabase( stm->file.current_rdb );
	    stm->file.current_rdb = NULL;
	    }

	 rdb = get_DECterm_database( stm );
	 if (rdb != NULL) {
#ifdef VMS_DECTERM
	    status = X$RM_PUT_FILE_DATABASE ( &rdb, filename_desc );
#else
#ifndef VXT_DECTERM

/* dxterm runs with suid root, so we can't just write out the database, since
   our home directoy may be NFS mounted, and root may not have write access
   on the NFS server.  So we need to set the uid and gid to the users' real
   uid and gid before writing out the database.  Unfortunately, we can't just
   set the uid and gid since we won't be able to get them back.  So we fork
   and do it in the child process.
 */

	{
	    pid_t pid, childpid;
	    int status;
	    extern void SYS_reapchild();

	    signal(SIGCHLD, SIG_IGN);	/* Ignore SIGCHLD signal for now */

	    if ((childpid = fork()) == -1)
	    {
		printf("Fork failed!\n");
		SysError(ERROR_FORK);
	    }
	    
	    if (childpid == 0)
	    {
	        /* Child process */

		setuid(getuid());	/* Set process uid to real uid */

		XrmPutFileDatabase( rdb, filename );

	    	exit(0);		/* Return to parent process */
	    }
	    else
	    {
		/* Parent process */

		while(wait(&status) != childpid)	/* wait for child */
		    ;
	    }

	    signal(SIGCHLD, SYS_reapchild);	/* Restore SIGCHLD signal */
	}
#endif
#endif
	 }
	 new_rdb = XrmCopyDatabase( stm->file.default_rdb );
	 XrmMergeDatabases( rdb, &new_rdb );
	 stm->file.current_rdb = new_rdb;
#ifdef VMS_DECTERM
      if (status) {
#endif
	    if (stm->file.current_filename != NULL ) {
	        XtFree( stm->file.current_filename );
	    }
	    stm->file.current_filename = XtMalloc( strlen(filename)+1 );
	    strcpy( stm->file.current_filename, filename );
	    XtUnmanageChild( w );
#ifdef VMS_DECTERM
	 } else {
	    warn_window (stm, "file_write_warning", filename);
      }
#endif
     }   
    }

    else XtUnmanageChild( w );
}


/*
 * file_saveas_cb( w, tag, call_data )
 */
void
file_saveas_cb( w, tag, call_data )
    Widget	w;
    caddr_t	tag;
    caddr_t	call_data;
{
    STREAM	*stm;

    stm = convert_widget_to_stream(w);

    /* check if we need to fetch the widget from disk */
    if (stm->file.saveas_fs == 0)
        {
        /* fetch file selection box from DRM */
        if (MrmFetchWidget
               (
               s_MRMHierarchy,
               "file_saveas_fs",
               stm->parent,
               & stm->file.saveas_fs,
               & dummy_class       
               )
           != MrmSUCCESS)
            {
            log_error( "Unable to fetch widget definitions from DRM\n");
	    return;
            }
	set_filter_filename (stm->file.saveas_fs);
        }

    /* initialize widget */
    set_one_string_value (stm->file.saveas_fs, XmNvalue,
	stm->file.current_filename);

    XtUnmanageChild( stm->file.saveas_fs );
    XtManageChild( stm->file.saveas_fs );
}                                   

/*
	   stm->default_db_title = "VXT DECterm" 
	   stm->default_title = the transport + "VXT DECterm" + a node
	   	name + the decterm window number (for example:
	   	"Lat VXT DECterm on GWEN 1")

   If default_flag == 0, this routine compares the current title and icon 
   to "Lat VXT DECterm on GWEN 1" type format ( stm->default_title ), and write
   "VXT DECterm" (stm->default_db_title) into the database.  
   If default_flag == 1, this routine compares current title and icon 
   to "VXT DECterm" (stm->default_db_title) and write "Lat VXT DECterm on 
   GWEN 1" type format ( stm->default_title ) into the database.
*/

#ifdef VXT_DECTERM

save_default_title_iconname( stm, rdb, resource_name, default_flag )
STREAM *stm;
XrmDatabase rdb;
char resource_name[100];
int default_flag;		/* 0 = write "VXT local DECterm" into the 
					database
				   1 = write "VXT local DECterm" plus window
					number into the database */
{
char *return_type;
XrmValue the_value;
char *read_default_name, 	
     *write_default_name,
     *current_name = NULL; /* current title or icon name */

    if ( XrmGetResource(rdb, resource_name, resource_name, &return_type, &the_value) 
		<= 0 )
    {
        printf("cannot find this DECterm resource \n");
	return;
    }

    if ( default_flag )
    {
        read_default_name = XtNewString( stm->default_db_title );
	write_default_name = XtNewString( stm->default_title );
    }
    else
    {
	read_default_name = XtNewString( stm->default_title );
        write_default_name = XtNewString( stm->default_db_title );
    }

    if ( (current_name = the_value.addr) == NULL )
        printf(" this DEcterm resource is NULL\n");
    else if ( !strcmp( read_default_name, current_name ) )
    {
        XrmPutStringResource( &rdb, resource_name, write_default_name );
    }
}

#endif

/*
 * file_save_cb( w, isn, call_data )
 */
void
file_save_cb( w, tag, call_data )
    Widget	w;
    caddr_t	tag;
    caddr_t	call_data;
{
    STREAM	*stm;
    int		status;
    XrmDatabase	rdb, new_rdb;
    char resource_name[100];
    int rc;

    stm = convert_widget_to_stream(w);

#ifndef VXT_DECTERM
    if (stm->file.current_filename != NULL) {
#ifndef VMS_DECTERM
     /*
      * DECterm runs with effective uid root, so we need to ensure
      * the directory is writeable by the real uid before trying to
      * save database
      */
         if (directory_writable(stm->file.current_filename))
#endif VMS_DECTERM
#endif VXT_DECTERM
         {
#ifdef VMS_DECTERM
	     int filename_desc[2];

	     filename_desc[0] = strlen(stm->file.current_filename);
	     filename_desc[1] = (int) stm->file.current_filename;
#endif
	     if ( stm->file.current_rdb != NULL )
	     {
	         XrmDestroyDatabase( stm->file.current_rdb );
	         stm->file.current_rdb = NULL;
	     }

             rdb = get_DECterm_database( stm );
	     if (rdb != NULL) {
#ifdef VMS_DECTERM
	         status = X$RM_PUT_FILE_DATABASE ( &rdb, filename_desc );
#else
#ifdef VXT_DECTERM

	    /* "Save Options" saves the difference between what's in the
		DECterm database versus the default system database.  If a 
		user changes the window title and saves it, the DECterm database
		is different from the default system database, and therefore
		the difference is saved.  If a user brings up multiple windows
		using the default title, each window would have a different 
		default title.  When a user executes "Save Options", the
		DECterm database is different from the system database, but in
		this case, it is undesirable to save the difference.  So need
		to do some tricks to save, restore the correct default title.

		Here is the algorithm:
	
		stm->default_db_title = "VXT DECterm" 
		stm->default_title = the transport + "VXT DECterm" + a node
			name + the decterm window number (for example:
			"Lat VXT DECterm on GWEN 1")
		The default system database contains "VXT DECterm" 
		(stm->default_db_title).  Before a window comes up, it compares
		the title in the DECterm widget	with "VXT DECterm", if it 
		matches, it will put up the title in the format of 
		"Lat VXT DECterm on GWEN 1" (stm->default_title).  When a user
		applies "Save Options", the title in the DECterm widget is
		compared with "Lat VXT DECterm on GWEN 1", if it's the same,
		"VXT DECterm" is written into the system database.  This
		technique also applies to "Restore System Options" and 
		"Restore Options".
	    */

                 sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
		 save_default_title_iconname( stm, rdb, resource_name, 0 );

	         sprintf( resource_name, "%s.main.terminal.defaultTitle", 
		          DECTERM_APPL_CLASS );
		 save_default_title_iconname( stm, rdb, resource_name, 0 );

                 sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
		 save_default_title_iconname( stm, rdb, resource_name, 0 );

	         sprintf( resource_name, "%s.main.terminal.defaultIconName", 
		          DECTERM_APPL_CLASS );
		 save_default_title_iconname( stm, rdb, resource_name, 0 );

	         new_rdb = XrmCopyDatabase( rdb );

		 rc = VxtrmPutCurrentDatabase (rdb, APP_DECTERM);

	        XrmMergeDatabases( new_rdb, &stm->file.default_rdb );

                /* Attaching database to the display */

                VxtDisplayInitialize( stm->display );
#else VXT_DECTERM
/* dxterm runs with suid root, so we can't just write out the database, since
   our home directoy may be NFS mounted, and root may not have write access
   on the NFS server.  So we need to set the uid and gid to the users' real
   uid and gid before writing out the database.  Unfortunately, we can't just
   set the uid and gid since we won't be able to get them back.  So we fork
   and do it in the child process.
 */

	{
	    pid_t pid, childpid;
	    int status;
	    extern void SYS_reapchild();

	    signal(SIGCHLD, SIG_IGN);	/* Ignore SIGCHLD signal for now */

	    if ((childpid = fork()) == -1)
	    {
		printf("Fork failed!\n");
		SysError(ERROR_FORK);
	    }
	    
	    if (childpid == 0)
	    {
	        /* Child process */

		setuid(getuid());	/* Set process uid to real uid */

	    	XrmPutFileDatabase( rdb, stm->file.current_filename );

	    	exit(0);		/* Return to parent process */
	    }
	    else
	    {
		/* Parent process */

		while(wait(&status) != childpid)	/* wait for child */
		    ;
	    }

	    signal(SIGCHLD, SYS_reapchild);	/* Restore SIGCHLD signal */
	}
#endif VXT_DECTERM
#endif VMS_DECTERM

#ifdef VMS_DECTERM
	         if (! status) warn_window (stm, "file_write_warning",
		     stm->file.current_filename);
#endif
	     }
	     new_rdb = XrmCopyDatabase( stm->file.default_rdb );
	     XrmMergeDatabases( rdb, &new_rdb );
	     stm->file.current_rdb = new_rdb;
         }
#ifndef VXT_DECTERM
    } 
    else {
	file_saveas_cb( w, tag, call_data );
    }
#endif VXT_DECTERM
}               

/*
 * file_revert_cb( w, tag, call_data )
 */
void
file_revert_cb( w, tag, call_data )
    Widget	w;
    caddr_t	tag;
    caddr_t	call_data;
{
    STREAM	*stm;
    char resource_name[100];

    stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
    if ( stm->file.current_rdb )
	XrmDestroyDatabase( stm->file.current_rdb );
    stm->file.current_rdb = VxtrmGetAllDatabases();


    /* If the root property contains "VXT DECterm" for title or icon name,
	need to change it to "Lat VXT DECterm on GWEN 1" type format. */

    sprintf( resource_name, "%s.title", DECTERM_APPL_CLASS );
    save_default_title_iconname( stm, stm->file.current_rdb, resource_name,
					1 );
    sprintf( resource_name, "%s.iconName", DECTERM_APPL_CLASS );
    save_default_title_iconname( stm, stm->file.current_rdb, resource_name,
					1 );
#endif VXT_DECTERM

    if (stm->file.current_rdb != NULL) {
	PutWidgetTreeDatabase( stm->parent, stm->file.current_rdb );
	adjust_title_iconName( stm->parent, stm->terminal );
    } else {
	PutWidgetTreeDatabase( stm->parent, stm->file.default_rdb );
	adjust_title_iconName( stm->parent, stm->terminal );
    }
}

/*
 * file_exit_cb( w, tag, call_data )
 */
void
file_exit_cb( w, tag, call_data )
    Widget	w;
    caddr_t	tag;
    caddr_t	call_data;
{
    STREAM	*stm;
    ISN         number;

    stm = convert_widget_to_stream(w);

    close_stream( stm );

}

static void
XrmRemoveResource(pdb, specifier)
    XrmDatabase     *pdb;
    char	    *specifier;
{
}

#ifndef VMS_DECTERM
#ifndef VXT_DECTERM
int
directory_writable( filename )
char *filename;
{
    char dirname[MAX_FILENAME_SIZE];
    char *last_slash;
    char current[MAX_FILENAME_SIZE], next[MAX_FILENAME_SIZE];
    int i;

/* First follow any symbolic links to find the real filename */

    for (i = 0, strcpy(current, filename); i < MAXSYMLINKS; i++)
    {
	if (readlink(current, next, MAX_FILENAME_SIZE) == -1)
	{
	    break;
	}
	else
	{
	    if (next[0] == '/')
	    {
		strcpy(current, next);
	    }
	    else
	    {
		char *	slash = rindex(current, '/');

		if (slash)
		{
		    slash[1] = '\0';
		    strcat(current, next);
		}
		else
		{
		    strcpy(current, next);
		}
	    }
	}
    }

    strcpy(filename, current);
    last_slash = strrchr (filename, '/');

    /* extract directory name */

    if (last_slash) { /* directory name specified */
        int dirsize = last_slash - filename;
	if (dirsize == 0) dirsize = 1;  /* root directory is a special case */
        strncpy (dirname, filename, dirsize);
        dirname[dirsize] = '\0';  /* null terminator */
    } else { /* directory defaults to . */
        dirname[0] = '.';
        dirname[1] = '\0';
    }

    /* the access system call checks for accessibility by real uid */
    return ( access( dirname, W_OK ) == 0 ? 1 : 0);
}

static XrmDatabase
XrmCopyDatabase(old)
    XrmDatabase	old;
{
    XrmDatabase	new;
    char	filename[L_tmpnam];

    tmpnam(filename);

    XrmPutFileDatabase(old, filename);
    new = XrmGetFileDatabase(filename);
    unlink(filename);
    return(new);
}
#endif VXT_DECTERM
#endif VMS_DECTERM

static void adjust_title_iconName( shell, w )
    Widget shell;
    Widget w;
{
    char *title, *iconName;
    long len, status;
    XmString cs;
    Arg arglist[2];

    XtSetArg( arglist[0], XtNtitle, &title );
    XtSetArg( arglist[1], XtNiconName, &iconName );
    XtGetValues( (Widget)shell, arglist, 2 );
    cs = DXmCvtFCtoCS( title, &len, &status );
    _DECwTermSetTitle( shell, w, cs );
    XmStringFree( cs );
    cs = DXmCvtFCtoCS( iconName, &len, &status );
    _DECwTermSetIconName( shell, w, cs );
    XmStringFree( cs );
}
