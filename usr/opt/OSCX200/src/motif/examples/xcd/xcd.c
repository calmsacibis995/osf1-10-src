/******************************************************************************
*******************************************************************************
*
*   Copyright (c) 1989, 1993 by Digital Equipment Corporation
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose and without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies, and that
*   the name of Digital Equipment Corporation not be used in advertising or
*   publicity pertaining to distribution of the document or software without
*   specific, written prior permission.
*
*   Digital Equipment Corporation makes no representations about the
*   suitability of the software described herein for any purpose.  It is
*   provided "as is" without express or implied warranty.
*  
*  DEC is a registered trademark of Digital Equipment Corporation
*  DIGITAL is a registered trademark of Digital Equipment Corporation
*  X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * File:	xcd.c
 * Date:	April 4, 1991
 *
 * Description:
 *   A sample Compact Disk front panel program.
 *
 * version 2.4
 * Modified: 	December 4, 1993
 *
 * Remove Sony vendor unique commands used with RRD42, since the new
 * CD-ROM's are Toshiba drives (RRD43 & RRD44).  This required removing
 * the SetAddrFormatMsf() & GetPlaybackStatus() functions.  The playback
 * status is now obtained from the header of the current position info.
 *
 * version 2.3
 * Modified:
 * Date:	June 25,1993
 * Fixed improper formatting of remaining time as well as cleaned up the
 * button handling a bit. Ignore double clicks on the buttons and take the
 * border widths into account.
 *
 * version 2.2
 * Modified:
 * Date:	May 21,1993
 * Added CAM code to dynamically find the CD player. Also now make it
 * use the toolkit to parse command line options so that the standard
 * toolkit command line options work.
 *
 * Fixed bugs breaking xcd in the CAM subsystem: open_dev must be RDWR for
 * the volume control to work right on that bus.  Also, the CD player in
 * the CAM subsystem locks in the CD on the first open.  We compensate for
 * this in eject_device() now.  This has the side effect of making
 * xcd's eject button always work, even when the hardware eject switch is
 * locked.  I considered this a feature(!).
 *
 * version 2.1a
 * Modified: 	
 * Date:	November 11, 1992
 * 
 * added an access check to ensure that device permisions are what is
 * required for correct operations.
 *
 * version 2.1
 * Modified: 	
 * Date:	January 14, 1992
 *
 * added version argument (-version).
 * 
 * Fixed a timing bug that caused the last track to cutoff early when
 * scan mode had been used.  Fixed an obscure selection highlighting bug.
 * Added a cuter bitmap icon for the iconified display.  Put in a
 * "better" copyright notice.
 *
 * no version label
 * Modified: 	
 * Date:	January 2, 1992
 *
 * Added "scan" function to advance or reverse disk position a few
 * seconds at a time.  Re-designed the status display to now include track
 * index display and an option menu for remaining/elapsed/total.  Now get
 * select highlight background color from widget armColor resource.  Many
 * fixes to "|<<" function.  Fixed bug where entering shuffle mode with a CD
 * of over 24 tracks hung the program.  Fixed a bunch of other small but
 * annoying shuffle problems
 *
 * Modified: 	
 * Date:	December 10, 1991
 *
 * Add virtual device layer interface to view the device as having
 * idealized commands.  Added selection buttons so that a specific track
 * can be directly selected.  Also, the selection buttons show the play
 * sequence and the track now playing.  Now get default volume level,
 * select button size and color from the UID file.
 * 	
 * Changed play_track() to play all tracks between the current track
 * and the last track if we aren't in shuffle mode.  This gets rid of
 * the stop/reset cycle that interrupts play between tracks.  This is
 * especially desirable for CD's with tracks that run right into each
 * other.
 * 
 * Massive overhaul with much simpler data structures and processing.
 * Fixed many design and implementation bugs.
 * 
 */

/*
 * Notes on the RRD42 CDROM PLAYER device:
 *
 * As a point of interest, the drive thinks the first track is #1, not #0.
 * It's convenient for us if the first track is track #0 so that we
 * can use the track number to index an array of size [total_tracks].
 * For this reason, the table of contents structure always thinks the
 * current track is one less than the hardware does.  This mismatch is
 * known only to the play_track() function, which fixes the mismatch
 * at the last moment when a play command is issued to the device.
 *
 * A note from the original source:
 *   A bug in the CDROM firmware or the SCSI software hangs the CDROM if
 *   you issue a stop and play command without waiting a bit, so when
 *   going play to stop to play again we sleep a second.  (I haven't
 *   observed this myself, but maybe my CDROM firmware has been fixed -ml)
 * 
 */

#include <stdio.h>
#include <sys/file.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Mrm/MrmPublic.h>	/* Motif Toolkit */
#include "cdutil.h"		/* Device specific includes */
#include "xcd_icons.h"		/* icon bitmaps */

#ifdef USE_CAM_TO_FIND_CD_PLAYER
#include	<io/cam/dec_cam.h>
#include	<io/cam/scsi_all.h>  
#include	"camFindDev.h"
#endif

#ifndef MAX
#define MAX(x,y)    ((x) > (y) ? (x) : (y))
#endif

/*
 * Global structures
 *
 * There are two primary global structures (not inlcuding the hardware
 * structure).  These are named "status" and "toc".
 *
 * "status":
 * 	the status structure defined immediately below contains
 * 	device state relevant to this program.  The status structure is
 * 	reloaded every time we call poll_device().  poll_device() polls
 * 	the device via ioctl() and updates much of the status
 * 	structure and the some of the toc structure.
 *
 * "toc":
 * 	The table of contents structures are laid out as follows.
 *
 *	toc +-------+
 *	    | entry |----> +--------------+
 *	    | ...   |      | track_number |
 *	    | ...   |	   | track_length |
 *	    +-------+	   +--------------+
 *	    		   | track_number |
 *	    		   | track_length |
 *	    		   +--------------+
 *	    		   | 		  |
 *	    		    . . . . . . .
 *	    		   |		  |
 *	    		   +--------------+
 *
 * 	The toc structure holds info relevant to the entire CD (eg:
 * 	total_tracks, etc).  toc.entry points to allocated space
 * 	for track structures which are accessed by indexing (ie:
 * 	toc.entryi[i].track_number.  The next CD track for playing is
 * 	ALWAYS selected by incrementing the index.  The index wraps to
 * 	zero when it gets to the last valid track strcuture.
 *
 *	If the track numbers in the array are numbered sequentially,
 *	then the tracks play in sequence.  Shuffle play is implemented
 *	by numbering the valid toc.entry's in random sequence.  The
 *	tracks then play in random sequence as the array index
 *	increments in linear fashion.
 *
 */

#define	VERSION "2.4"

typedef struct _cdstat {
    int state;			/* Current state of the CDROM */

#define IDLE 		0
#define STOPPED 	1
#define PLAYING 	2
#define PAUSED 		3
#define EJECTED 	4

    int flags;			/* Flags to control the CDROM */

#define CD_EXCLUSIVE 	0x01	/* To mask out the timeout during changes */
#define CD_REPEAT_TRACK	0x02	/* Repeat the current track */
#define CD_REPEAT_DISK 	0x04	/* Repeat the whole disk */
#define CD_PREVENT 	0x08	/* Prevent removal */
#define CD_DEVICE_OPEN	0x10	/* Device is open */
#define CD_SHUFFLE	0x20	/* the play list is shuffled */

#define CD_REMAINING	0x40	/* display time remaining */
#define CD_ELAPSED	0x80	/* display time elapsed*/
#define CD_TOTAL	0x100	/* display total time only */
#define CD_TIME_MASK	0x1C0	/* timer mask bits */

#define CD_TIMED_OUT	0x200	/* device timed out during open */
  
    int current_track;		/* reported current track playing (start = 1) */
    int current_index;		/* reported current index playing */
    time_t current_seconds;	/* current seconds into the track */
    time_t current_total_seconds; /* current total seconds into the disc */
    int currentVolume;   	/* current volume setting */
	
    XtIntervalId timer;		/* To update the time each second, we set
			 	 * a timer.  timer holds the handle to allow us
			 	 * to turn off the timer before its expiration
			 	 */

    XtIntervalId scan_timer;	/* Same for scanning if active */
    enum {BACK, AHEAD, OFF} scan_direction;
    time_t scan_stride;		/* Current scanning skip distance */

#define SCAN_BITE  	950	/* ms to play each scan interval */
#define MIN_SCAN_STRIDE	5	/* Starting secs to skip per scan interval */
#define MAX_SCAN_STRIDE	20	/* Final secs to skip per scan interval */

} statusRec, *statusPtr;

statusRec status;

/*
 * table of contents structures
 */

typedef struct _tocEntry {
    int track_number;		/* Track number of this TOC entry */
    time_t track_address;	/* start adrs in secs (used for SCAN mode) */
    time_t track_length; 	/* length in secs */
} tocEntryRec, *tocEntryPtr;

typedef struct _toc {
    tocEntryPtr entry;		/* pointer to an array of tocEntryRec */
    int current;		/* the number of the last toc.entry to be played */
    int last;			/* the number of the last track on the disk */
    time_t total_time;		/* the total seconds on the CD */
    int total_tracks;		/* the total tracks on the CD */
} tocRec, *tocPtr;

tocRec toc;

/*
 * Declare space to hold Table of Contents read from CDROM hardware
 */

struct cd_toc_head_and_entries glob_toc;

static MrmHierarchy	s_MrmHierarchy;		/* MRM database hierarch id */
static char		*vec[]={"xcd.uid"}; 	/* MRM database file */

#if defined __ultrix || defined ultrix
/*
** EXTERNAL DECLARATIONS (which have no prototypes)
*/
#ifdef _NO_PROTO
extern long random();
extern void srandom();
extern int getopt();
extern char *rindex();
#else
extern long random( void );
extern void srandom( int seed );
extern int getopt(int argc, char **argv, char *optstring);
extern char *rindex(char *string, char searchfor);
#endif /* _NO_PROTO */
#endif /* ultrix */

/*
** FORWARD DECLARATIONS
*/
#ifdef _NO_PROTO
static void play_button_activate ( );
static void select_button_activate ( );
static void prev_track_button_activate ( );
static void next_track_button_activate ( );
static void scan_back_button_arm ( );
static void scan_ahead_button_arm ( );
static void scan_button_disarm ( );
static void stop_button_activate ( );
static void pause_button_activate ( );
static void shuffle_button_activate ( );
static void repeat_button_activate ( );
static void eject_button_activate ( );
static void quit_button_activate ( );
static void quit_signalled ( );
static void prevent_button_activate ( );
static void volume_slider_activate ( );
static void status_menu_activate ( );
static void create_cb ( );
static void updateStatusDisplay ( );
static void updateSelectDisplay ( );
static void lightMainButton ( );
static void lightSelectButton ( );
static void setupTOC ( );
static void clearTOC ( );
static void selectpad_resize_handler ( );
static void reparent_handler ( );
static void set_icon ( );
static int open_dev ( );
static void play_track ( );
static void next_track ( );
static void prev_track ( );
static void pause_device ( );
static void resume_device ( );
static void stop_device ( );
static void eject_device ( );
static void set_volume ( );
static void start_timer ( );
static void stop_timer ( );
static void timeout_proc ( );
static void start_scan_timer ( );
static void stop_scan_timer ( );
static void scan_timeout ( );
static void poll_device ( );
static int getTrackLength ( );

#else /* _NO_PROTO */

static void play_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void select_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void prev_track_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void next_track_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void scan_back_button_arm ( Widget widget , XtPointer tag , XtPointer callback_data );
static void scan_ahead_button_arm ( Widget widget , XtPointer tag , XtPointer callback_data );
static void scan_button_disarm ( Widget widget , XtPointer tag , XtPointer callback_data );
static void stop_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void pause_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void shuffle_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void repeat_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void eject_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void quit_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void quit_signalled ( int sig );
static void prevent_button_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void volume_slider_activate ( Widget widget , XtPointer tag , XmScaleCallbackStruct *scale );
static void status_menu_activate ( Widget widget , XtPointer tag , XtPointer callback_data );
static void create_cb ( Widget w , XtPointer tag , unsigned long *reason );
static void updateStatusDisplay ( void );
static void updateSelectDisplay ( void );
static void lightMainButton ( int buttonToLight );
static void lightSelectButton ( int buttonToLight );
static void setupTOC ( void );
static void clearTOC ( void );
static void selectpad_resize_handler ( Widget widget , XtPointer client_data , XEvent *event , Boolean *continue_to_dispatch );
static void reparent_handler ( Widget widget , XtPointer client_data , XEvent *event , Boolean *continue_to_dispatch );
static void set_icon ( void );
static int open_dev ( void );
static void play_track ( void );
static void next_track ( void );
static void prev_track ( void );
static void pause_device ( void );
static void resume_device ( void );
static void stop_device ( void );
static void eject_device ( void );
static void set_volume ( int sliderValue );
static void start_timer ( Widget w );
static void stop_timer ( void );
static void timeout_proc ( XtPointer data , XtIntervalId *t );
static void start_scan_timer ( Widget w );
static void stop_scan_timer ( void );
static void scan_timeout ( XtPointer data , XtIntervalId *t );
static void poll_device ( void );
static int getTrackLength ( int trackNum );

#endif /* _NO_PROTO */


/*
 * Set up registration structure.  These strings will be associated with
 * the corresponding routines and made visible to the uil commands.
 */

static MrmRegisterArg	regvec[] = {
    { "play_button_activate", 		(XtPointer) play_button_activate },
    { "stop_button_activate", 		(XtPointer) stop_button_activate },
    { "pause_button_activate", 		(XtPointer) pause_button_activate },
    { "prev_track_button_activate",	(XtPointer) prev_track_button_activate },
    { "next_track_button_activate", 	(XtPointer) next_track_button_activate },
    { "scan_back_button_arm", 		(XtPointer) scan_back_button_arm },
    { "scan_ahead_button_arm", 		(XtPointer) scan_ahead_button_arm },
    { "scan_button_disarm", 		(XtPointer) scan_button_disarm },
    { "shuffle_button_activate", 	(XtPointer) shuffle_button_activate },
    { "repeat_button_activate", 	(XtPointer) repeat_button_activate },
    { "eject_button_activate", 		(XtPointer) eject_button_activate },
    { "quit_button_activate", 		(XtPointer) quit_button_activate },
    { "prevent_button_activate", 	(XtPointer) prevent_button_activate },
    { "select_button_activate", 	(XtPointer) select_button_activate },
    { "volume_slider_activate", 	(XtPointer) volume_slider_activate },
    { "status_menu_activate", 		(XtPointer) status_menu_activate },
    { "create_cb", 			(XtPointer) create_cb }
};

static MrmCount		 regnum = sizeof(regvec) / sizeof(MrmRegisterArg);

/*
 * Define the indices into the widget array.  When the widgets are created,
 * the create callback will fill in the corresponding entry in the widget
 * array for use when the widget characteristics need to be changed.
 */

#define k_prevent_id 		0
#define k_stop_id 		1
#define k_play_id 		2
#define k_pause_id 		3
#define k_eject_id		4
#define k_repeat_id 		5
#define k_shuffle_id 		6
#define k_trackNum_id		7
#define k_trackTime_id		8
#define k_discTime_id		9
#define k_indexNum_id		10
#define k_selectPad_id 		11
#define k_selectButton_id 	12
#define k_volumeSlider_id 	13
#define k_timeElapsed_id	14
#define k_timeRemaining_id	15
#define k_timeTotal_id		16
#define k_timeRC_id		17

#define NWIDGET 	18	/* # of slots in widget array */

static Widget primaryWidgets[NWIDGET];

/* status widget ID's */

#define REMAINING	1
#define ELAPSED		2
#define	TOTAL		3

/* selection widget array */

static Widget selectWidgets[MAXTRACKS];
static int selectTags[MAXTRACKS];  /* ptrs to these ints passed by select cb's */

static Widget toplevel, xcdmain;

/*
 * these arrays are indexed by the widget ID numbers and are used to
 * switch from the active icon to the passive icon in the
 * lightMainButtons() routine.
 */

static char *passiveIconName[] = {
    "allowIcon",
    "passiveStopIcon",
    "passivePlayIcon",
    "passivePauseIcon",
    "passiveEjectIcon",
    "",
    "",
    ""
};  

static char *activeIconName[] = {
    "preventIcon",
    "activeStopIcon",
    "activePlayIcon",
    "activePauseIcon",
    "activeEjectIcon",
    "",
    "",
    ""
};  

/* variables used to manage size of track selection keypad */
static Dimension	selpad_spacing,
			selbtn_width,
			selbtn_height,
			selbtn_hilite,
			selbtn_shadow;
static int		initial_rows;

/************************************************************************/
/*									*/
/* Application Resources						*/
/*									*/
/************************************************************************/

/*
 * The OptionsRec structure is used to hold the resources for the 
 * application.
 */
typedef struct {
    int		volume;			/* Volume.			*/
    int		selectKeysPerRow;	/* Number of keys per row.	*/
    String	timeDisplayType;	/* Elapsed/Remaining/Total	*/
    String 	cddev;
    Boolean 	version;
    Boolean	usage;
} OptionsRec;

OptionsRec options;

/*
 * Define the resource array to pass to XtGetApplicationResources.
 */
#define Offset(field) XtOffsetOf(OptionsRec,field)

XtResource resources[] = {
    {"volume", "Volume", XtRInt, sizeof(int),
	Offset(volume), XtRImmediate, (XtPointer) 50},
    {"keysPerRow", "keysPerRow", XtRInt, sizeof(int),
	Offset(selectKeysPerRow), XtRImmediate, (XtPointer) 0},
    {"timeDisplayType", "TimeDisplayType", XtRString, sizeof(String),
	Offset(timeDisplayType), XtRImmediate, (XtPointer) "elapsed"},
    {"cd", "CD", XtRString, sizeof(String),
	Offset(cddev), XtRImmediate, (XtPointer) NULL},
    {"version","Version",XtRBoolean, sizeof(Boolean),
	Offset(version), XtRImmediate, (XtPointer) FALSE},
    {"usage","Usage",XtRBoolean, sizeof(Boolean),
	Offset(usage), XtRImmediate, (XtPointer) FALSE}
};
#undef Offset

/*
 * Set up any fallback resources here in case the user doesn't have a
 * data file for this application.  XtAppInitialize will use this.
 */
String fallback_resources[] = {
	NULL
};

/*
 * Tell the toolkit that there are some additional command line arguments
 * that can be parsed.  XtAppInitialize will use this.
 */
XrmOptionDescRec optionDesc[] = {
    {"-volume",		"*volume", 		XrmoptionSepArg, (XtPointer) NULL},
    {"-keysperrow", 	"*keysPerRow", 		XrmoptionSepArg, (XtPointer) NULL},
    {"-time", 		"*timeDisplayType", 	XrmoptionSepArg, (XtPointer) NULL},
    {"-cd",		".cd", 			XrmoptionSepArg, (XtPointer) NULL},
    {"-version",	".version", 		XrmoptionNoArg,  (XtPointer) "on"},
    {"-usage",		".usage",		XrmoptionNoArg,  (XtPointer) "on"}
};

String usage_message =
"%s: version %s\n\
  usage: -volume        specifies initial volume in range 0 through 100\n\
         -keysperrow    specifies number of track keys to display per row\n\
         -time          specifies 'remaining', 'total' or 'elapsed' display\n\
         -cd		specifies device name of CD player, such as /dev/rrz4c\n\
         -version	requests xcd to print version id then exit\n\
         -usage		requests xcd to print this usage message then exit\n\
  In addition, xcd accepts standard X Toolkit options\n";

/*
 * externals...
 */
extern char *optarg;
extern int opterr;

/*
 * global to this file
 */
static Display *display;

/* select button literals */
static Pixel highlightColorFG, highlightColorBG;
static Pixel lowlightColorFG, lowlightColorBG;

/************************************************************************
 *
 * 	main()... the buck starts here
 * 	
 ************************************************************************/

int main
#ifdef _NO_PROTO
(argc, argv)
    int		argc;
    char	**argv;
#else
(
    int 	argc,
    char **	argv
)
#endif /* _NO_PROTO */
{
    MrmCode 		class;
    XtAppContext 	app_context;
    int 		i, n;
    time_t 		salt;
    Arg 		argList[16];
    char 		*pC;
    int			timebutton;
    Dimension		min_width = 0, min_height = 0;
    Dimension		height_inc = 0, width_inc = 0,
			base_height = 0, base_width = 0,
			selpad_height = 0, selpad_width = 0,
			selpad_shadow = 0, selpad_highlight = 0,
			selpad_marginheight = 0;
    unsigned char	rctype;
    Boolean		radio;

    /* Salt away the program name for later */

    pC = rindex(argv[0], '/');
    pC = pC ? &(pC[1]) : argv[0];

    /* Initialize Mrm and the toolkit */

    MrmInitialize();		/* init the motif resource manager */

    n = 0;
    XtSetArg(argList[n], XtNallowShellResize, TRUE);	n++;
    toplevel = XtAppInitialize(&app_context,
				"Xcd",
				optionDesc, XtNumber(optionDesc),
				&argc, argv,
				fallback_resources,
				argList, n);

    XtGetApplicationResources (toplevel, (XtPointer) &options,
				resources, XtNumber(resources),
				(Arg *)NULL, 0);
    
    display = XtDisplay(toplevel);
    
    if (display == NULL) {
	fprintf(stderr, "%s:  Can't open X display\n", argv[0]);
	exit(1);
    }

    /* Initialize the status rec to all zeroes */
    BZERO(&status, sizeof(statusRec));

    if (options.usage || argc > 1) {
	printf(usage_message, pC, VERSION);
	exit(0);
    }

    if (options.version) {
	printf("%s: version %s\n", pC, VERSION);
	exit(0);
    }

    if (!options.cddev) {
	options.cddev = getenv("CDROM");
#if USE_CAM_TO_FIND_CD_PLAYER
	if (!options.cddev) {
	    camDevTbl cd;
	    if ((cd = camFindDev(ALL_DTYPE_RODIRECT))) {
		options.cddev = malloc(16);
		sprintf(options.cddev,"/dev/rrz%dc",cd[0].bus*8+cd[0].target);
		free (cd);
	    }
	}
#endif
    }

    if (options.cddev) {
	if ( access(options.cddev,R_OK|W_OK) ) {
	    fprintf(stderr, "%s: Device specified (%s) is not writable\n",
				argv[0], options.cddev);
	    fprintf(stderr, 
		"\tRead and Write permissions on this device are required\n");
	    exit(-1);
	}
	if(open_dev() == 0) {
	    status.state = IDLE;
	} else {
	    status.state = EJECTED;
	}
	AllowRemoval();
    } else {
	fprintf (stderr, "%s: Please specify a device name using the '-cd device' option\nor set the CDROM environment variable.\n", pC);
	exit (-1);
    }


    /*
     *  Define the Mrm hierarchy (only 1 file)
     */

    if (MrmOpenHierarchy(	1,		/* number of files (1) */
				vec, 	    	/* filename(s) */
				NULL,	    	/* os_ext_list */
				&s_MrmHierarchy) /* ptr to returned id */
	!= MrmSUCCESS) {
	fprintf (stderr, "can't open the UID file hierarchy\n");
    }

    /*
     * fetch the application's icon name from the UID file and set the
     * value in the toplevel widget.
     */
    
    XtSetArg(argList[0], XmNiconName, "k_iconName");
    MrmFetchSetValues(s_MrmHierarchy, toplevel, argList, 1);

    /*
     * Register the names and addresses of the callback routines so
     * that the resource manager can bind them to their names in the
     * UID file.
     */

    if (MrmRegisterNames(regvec, regnum) != MrmSUCCESS)
	fprintf(stderr, "can't register names\n");

    /*
     *  create the main widget and the kids
     */

    if (MrmFetchWidget(s_MrmHierarchy,
			"xcdMain",	/* name of main window widget */
			toplevel,	/* top widget ID from Xtoolkit */
			&xcdmain,
			&class)
	!= MrmSUCCESS) {
	fprintf(stderr, "can't fetch interface\n");
    }

    /*
     * Initialize display values, etc.
     */

    status.flags &= ~(CD_REPEAT_DISK | CD_REPEAT_TRACK | CD_PREVENT);

    XtSetArg(argList[0], XmNlabelPixmap, "repeatOffIcon");
    MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[k_repeat_id],
			argList, 1);
    
    status.flags &= ~CD_SHUFFLE;

    XtSetArg(argList[0], XmNlabelPixmap, "serialPlayIcon");
    MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[k_shuffle_id],
			argList, 1);

    /*
     *  Set the default volume level.  If you don't initialize it
     *  it reverts to the maximum level (ouch!).
     */

    XtSetArg(argList[0], XmNvalue, status.currentVolume = options.volume);
    XtSetValues(primaryWidgets[k_volumeSlider_id], argList, 1);

    /*
     * get the selection button colors
     */
    
    n = 0;
    XtSetArg(argList[n], XmNforeground, &lowlightColorFG);	n++;
    XtSetArg(argList[n], XmNbackground, &lowlightColorBG);	n++;
    XtSetArg(argList[n], XmNarmColor, &highlightColorBG);	n++;
    XtGetValues(primaryWidgets[k_selectButton_id], argList, n);
    
    highlightColorFG = lowlightColorFG;
    
    /*
     * set the time display defaults
     */
    if (strcmp(options.timeDisplayType,"remaining") == 0) {
	status.flags |= CD_REMAINING;
	timebutton = k_timeRemaining_id;
    } else if (strcmp(options.timeDisplayType,"total") == 0) {
	status.flags |= CD_TOTAL;
	timebutton = k_timeTotal_id;
    } else {
    	status.flags |= CD_ELAPSED;			/* default */
	timebutton = k_timeElapsed_id;
    }

/*
 * The UIL code allows the time display to be either an XmRadioBox or an
 * XmOptionMenu.  In order to set the initial display correctly, we need
 * to know which.
 */
    XtSetArg(argList[0], XmNrowColumnType, &rctype);
    XtSetArg(argList[1], XmNradioBehavior, &radio);
    XtGetValues(primaryWidgets[k_timeRC_id], argList, 2);
    if (rctype == XmWORK_AREA && radio) {
	XtSetArg(argList[0], XmNset, True);
	XtSetValues(primaryWidgets[timebutton], argList, 1);
    } else if (rctype == XmMENU_OPTION) {
	XtSetArg(argList[0], XmNmenuHistory, primaryWidgets[timebutton]);
	XtSetValues(primaryWidgets[k_timeRC_id], argList, 1);
    }

    /*
     * Make the toplevel widget "manage" the main window (or whatever the
     * the UIL defines as the topmost widget).  This will cause it to be
     * "realized" when the toplevel widget is "realized"
     */

    XtManageChild(xcdmain);

    /* Setup event handler for Reparent notify events */
    XtAddEventHandler(toplevel, StructureNotifyMask, False, reparent_handler, None);

    /* If user left it to us to figure out how many buttons per row, add a
     * resize handler
     */
    if (options.selectKeysPerRow == 0) {
	XtAddEventHandler(primaryWidgets[k_selectPad_id],
		StructureNotifyMask, False, selectpad_resize_handler, None);
    }

    /*
     * Realize the toplevel widget.  This will cause the entire "managed"
     * widget hierarchy to be displayed
     */
    XtRealizeWidget(toplevel);

    /*
     * At this point, the select pad has a single button.  Get the width and
     * height of the main window.  Tell the window manager not to let the
     * user shrink the window any smaller than this.
     */
    n = 0;
    XtSetArg(argList[n], XmNheight, &min_height); n++;
    XtSetArg(argList[n], XmNwidth, &min_width); n++;
    XtGetValues(toplevel, argList, n);

    /*
     * Get values we need from the select pad and the button
     * in order to handle resizes.  The following calculations are
     * based on the assumption that the height of the function button box
     * and the status area are taller than the height of the one select
     * button.
     */
    n = 0;
    XtSetArg(argList[n], XmNspacing, &selpad_spacing); n++;
    XtSetArg(argList[n], XmNheight, &selpad_height); n++;
    XtSetArg(argList[n], XmNwidth, &selpad_width); n++;
    XtSetArg(argList[n], XmNshadowThickness, &selpad_shadow); n++;
    XtSetArg(argList[n], XmNhighlightThickness, &selpad_highlight); n++;
    XtSetArg(argList[n], XmNmarginHeight, &selpad_marginheight); n++;
    XtGetValues(primaryWidgets[k_selectPad_id], argList, n);

    n = 0;
    XtSetArg(argList[n], XmNheight, &selbtn_height); n++;
    XtSetArg(argList[n], XmNwidth, &selbtn_width); n++;
    XtGetValues(primaryWidgets[k_selectButton_id], argList, n);

    height_inc = selbtn_height + selpad_spacing;
    width_inc = selbtn_width + selpad_spacing;
    base_height = min_height - selpad_height +
			selbtn_height + 2*selpad_shadow +
			2*selpad_highlight + 2*selpad_marginheight;
    base_width = (min_width % width_inc) + selbtn_width ;
    initial_rows = (min_height - base_height) / height_inc;
    if ((min_height - base_height) % height_inc)
	initial_rows++;
    n = 0;
    XtSetArg(argList[n], XmNminHeight, min_height); n++;
    XtSetArg(argList[n], XmNminWidth, min_width); n++;
    XtSetArg(argList[n], XmNbaseHeight, base_height); n++;
    XtSetArg(argList[n], XmNbaseWidth, base_width); n++;
    XtSetArg(argList[n], XmNheightInc, height_inc); n++;
    XtSetArg(argList[n], XmNwidthInc, width_inc); n++;
    XtSetValues(toplevel, argList, n);

    /* Now that min height and width are set, it's OK to unmanage the
     * dummy pushbutton we use as a template.
     */
    XtUnmanageChild(primaryWidgets[k_selectButton_id]);

    /* Set the appropriately-sized icon pixmap */
    set_icon();

    /*
     * init the random number generator in case the user shuffles
     */
    salt = time((time_t *)0);

    srandom((int)salt);

    /*
     * set up the display for initial dipiction
     */
    
    toc.entry = (tocEntryPtr) malloc(MAXTRACKS * sizeof(tocEntryRec));

    poll_device();	/* toc.entry[] must be malloc'd b4 u get here */

    updateStatusDisplay(); 

    switch (status.state) {
	case PLAYING:
	case PAUSED:

	    setupTOC();
	    lightMainButton(k_play_id);
	    lightSelectButton(status.current_track - 1);
	    start_timer(toplevel);
	    break;

	case IDLE:
	 
	    stop_device();

	case STOPPED:

	    setupTOC();
	    lightMainButton(k_stop_id);	 
	    set_volume(status.currentVolume);
	    break;
	 
	case EJECTED:

	    clearTOC();
	    break;
	 
	default:
	    break;
    }

    XtManageChild(primaryWidgets[k_selectPad_id]);

    /*
     * catch interrupt, hangup, and quit so we can unlock the front panel
     */
    
    (void) signal(SIGINT, quit_signalled);
    (void) signal(SIGHUP, quit_signalled);
    (void) signal(SIGQUIT, quit_signalled);
    
    /*
     * Loop and process events
     */

    XtAppMainLoop(app_context);

    /* UNREACHABLE */

    return(0);
    
} /* main */

/*************************************************************************
 *************************************************************************
 *************************************************************************
 *
 * 	MOTIF CALLBACKS...
 *
 *************************************************************************
 *************************************************************************
 *************************************************************************/

/************************************************************************
 *
 * 	play_button_activate()... If a disk is inserted, play it.
 *
 ************************************************************************
 *
 * If the disk is paused, issue a resume command, otherwise restart
 * play at the current track.  If the drive was stopped or previously
 * ejected, rebuild the table of contents.  (This restores default
 * order after a shuffle).
 */

static void play_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case PAUSED:
	    resume_device();
	    break;

	case EJECTED:
	case IDLE:
	case STOPPED:

	/*
	 * PLAY might be pressed after a new CD was inserted so we
	 * need to call poll_device() to get the new TOC.
	 */

	    poll_device();

	    if (status.state == EJECTED)	/* still ejected? */
		return;
	
	    setupTOC();
	    play_track();
	    break;
	
	case PLAYING:
	default:
	    return;
    }
    
} /* play_button_activate */

/************************************************************************
 *
 *	select_button_activate()...
 *
 ************************************************************************/

static void select_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case IDLE:
	case EJECTED:

	    setupTOC();
	    break;
	
	case PAUSED:
	case STOPPED:
	case PLAYING:
	    break;

	default:
	    return;
    }
	  
    toc.current = *(int *)tag;
    play_track();

} /* select_button_activate */

/************************************************************************
 *
 * prev_track_button_activate()... play previous track (or current track)
 *
 ***********************************************************************/
 
static void prev_track_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case EJECTED:
	case IDLE:
	
	    /* handle a newly inserted disk */

	    poll_device();

	    if (status.state == EJECTED)	/* still ejected? */
		return;
	
	    setupTOC();
	    break;
	
	case STOPPED:
	case PLAYING:
	    break;

	case PAUSED:
	default:
	    return;
    }
	
    prev_track();
    
} /* prev_track_button_activate */

/************************************************************************
 *
 * 	next_track_button_activate()... play the next track
 *
 ************************************************************************/

static void next_track_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case EJECTED:
	case IDLE:

	    /* handle a newly inserted disk */

	    poll_device();

	    if (status.state == EJECTED)	/* still ejected? */
		return;
	
	    setupTOC();
	    play_track();
	    break;
	
	case STOPPED:
	case PLAYING:
	    next_track();
	    break;
	
	case PAUSED:
	default:
	    return;
    }

} /* next_track_button_activate */

/************************************************************************
 *
 * scan_back_button_arm()... enter scanning backward state
 *
 ***********************************************************************/
 
static void scan_back_button_arm
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case PLAYING:
	    stop_timer();		/* stop the usual timeout */
	    status.scan_direction = BACK;
	    status.scan_stride = MIN_SCAN_STRIDE;
	    start_scan_timer(widget);
	    break;

	case EJECTED:
	case IDLE:
	case STOPPED:
	case PAUSED:
	default:
	    return;
    }
    
} /* scan_back_button_arm */

/************************************************************************
 *
 * scan_ahead_button_arm()... enter scanning ahead state
 *
 ***********************************************************************/
 
static void scan_ahead_button_arm
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case PLAYING:
	    stop_timer();		/* stop the usual timeout */
	    status.scan_direction = AHEAD;
	    status.scan_stride = MIN_SCAN_STRIDE;
	    start_scan_timer(widget);
	    break;

	case EJECTED:
	case IDLE:
	case STOPPED:
	case PAUSED:
	default:
	    return;
    }
    
} /* scan_ahead_button_arm */

/************************************************************************
 *
 * scan_button_disarm()... exit scanning state
 *
 ***********************************************************************/
 
static void scan_button_disarm
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case IDLE:
	case PLAYING:
	    stop_scan_timer();
	    status.scan_direction = OFF;
	    status.scan_stride = MIN_SCAN_STRIDE;
	    start_timer(widget);		/* resume the usual timer */
	
	    poll_device();			/* get new latest numbers... */
	    updateStatusDisplay();		/* ...and print them */
	    break;

	case EJECTED:
	case STOPPED:
	case PAUSED:
	default:
	    return;
    }
    
} /* scan_button_disarm */

/************************************************************************
 *
 * 	stop_button_activate()... stop the CD
 *
 ************************************************************************/

static void stop_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    poll_device();
  
    switch (status.state) {
	case PLAYING:
	case PAUSED:

	    stop_device();
	    break;
	
	case IDLE:
	    setupTOC();
	    stop_device();
	    break;

	case EJECTED:
	case STOPPED:
	default:
	    return;
    }
	
} /* stop_button_activate */

/************************************************************************
 *
 * 	pause_button_activate()... pause if currently playing, else resume
 *
 ************************************************************************/

static void pause_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    switch (status.state) {
	case PLAYING:
	    pause_device();
	    break;
	
	case PAUSED:
	    resume_device();
	    break;
	
	case EJECTED:
	case IDLE:
	case STOPPED:
	default:
	    return;
    }
    
} /* pause_button_activate */

/************************************************************************
 *
 * 	shuffle_button_activate()... shuffle the play list and start play
 *
 ************************************************************************/

static void shuffle_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    Arg argList[16];

    switch (status.state) {
	case IDLE:
	case PLAYING:
	case STOPPED:

	    if (status.flags & CD_SHUFFLE) {
		status.flags &= ~CD_SHUFFLE;
		XtSetArg(argList[0], XmNlabelPixmap, "serialPlayIcon");
	    } else {
		status.flags |= CD_SHUFFLE;
		XtSetArg(argList[0], XmNlabelPixmap, "shufflePlayIcon");
	    }

	    /* update the widget to use the new icon */
	
	    MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[k_shuffle_id],
				argList, 1);

	    setupTOC();
	    play_track();
	    break;
	
	case PAUSED:
	case EJECTED:
	default:
	    return;
    }

} /* shuffle_button_activate */

/************************************************************************
 *
 * 	repeat_button_activate()... handle repeat state changes
 *
 ************************************************************************/

static void repeat_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    Arg argList[16];

    if (status.flags & CD_REPEAT_TRACK) {
	status.flags &= ~(CD_REPEAT_DISK | CD_REPEAT_TRACK);
	XtSetArg(argList[0], XmNlabelPixmap, "repeatOffIcon");
    } else if (status.flags & CD_REPEAT_DISK) {
	status.flags &= ~CD_REPEAT_DISK;
	status.flags |= CD_REPEAT_TRACK;
	XtSetArg(argList[0], XmNlabelPixmap, "repeatTrackIcon");
    } else {
	status.flags |= CD_REPEAT_DISK;
	XtSetArg(argList[0], XmNlabelPixmap, "repeatDiskIcon");
    }
    
    MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[k_repeat_id], argList, 1);

} /* repeat_button_activate */

/************************************************************************
 *
 * 	eject_button_activate()... eject the CD
 *
 ************************************************************************/

static void eject_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    poll_device();

    if (status.state == EJECTED || status.flags & CD_PREVENT)
	return;
    
    eject_device();

} /* eject_button_activate */

/************************************************************************
 *
 * 	quit_button_activate()...  bye!
 *
 ************************************************************************/

static void quit_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    stop_button_activate(widget, tag, callback_data);
    quit_signalled(SIGQUIT);
}

/************************************************************************
 *
 * 	quit_signalled()... bye!
 *
 ************************************************************************/

static void quit_signalled
#ifdef _NO_PROTO
(sig)
    int	sig;
#else
(
    int	sig
)
#endif /* _NO_PROTO */
{
    AllowRemoval();
    exit(0);
}

/************************************************************************
 *
 *	prevent_button_activate()... toggle prevent state
 *
 ***********************************************************************
 *
 * Either allow or prevent removal of the caddy.  Also update the 
 * Icon on the button to show the current state.
 */

static void prevent_button_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    Arg argList[1];

    if (status.flags & CD_PREVENT) {
	status.flags &= ~CD_PREVENT;
	AllowRemoval();
	XtSetArg(argList[0], XmNlabelPixmap, passiveIconName[k_prevent_id]);
    } else {
	status.flags |= CD_PREVENT;
	PreventRemoval();
	XtSetArg(argList[0], XmNlabelPixmap, activeIconName[k_prevent_id]);
    }

    MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[k_prevent_id],
		     argList, 1);

} /* prevent_button_activate */

/************************************************************************
 *
 * volume_slider_activate()... change the volume level
 *
 ************************************************************************/

static void volume_slider_activate
#ifdef _NO_PROTO
(widget, tag, scale_callback_data)
    Widget	widget;
    XtPointer	tag;
    XmScaleCallbackStruct *scale_callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XmScaleCallbackStruct *scale_callback_data
)
#endif /* _NO_PROTO */
{
    set_volume(scale_callback_data->value);
}

/************************************************************************
 *
 *	status_menu_activate()... status option selection
 *
 ************************************************************************/

static void status_menu_activate
#ifdef _NO_PROTO
(widget, tag, callback_data)
    Widget	widget;
    XtPointer	tag;
    XtPointer	callback_data;
#else
(
    Widget	widget,
    XtPointer	tag,
    XtPointer	callback_data
)
#endif /* _NO_PROTO */
{
    char	*id = (char *)tag;
    status.flags &= ~(CD_TIME_MASK);

    switch (*id) {
	case REMAINING:
	    status.flags |= CD_REMAINING;
	    break;
	
	case ELAPSED:
	    status.flags |= CD_ELAPSED;
	    break;

	case TOTAL:
	    status.flags |= CD_TOTAL;
	    break;
	
	default:
	    break;
    }

    updateStatusDisplay();
    
} /* status_menu_activate */

/************************************************************************
 *
 *	create_cb()... called when widgets startup
 *
 ************************************************************************/

static void create_cb
#ifdef _NO_PROTO
(widget, tag, reason)
    Widget	widget;
    XtPointer	tag;
    unsigned long *reason;
#else
(
    Widget	widget,
    XtPointer	tag,
    unsigned long *reason
)
#endif /* _NO_PROTO */
{
    int id = *(int *)tag;
    if (id < NWIDGET)
	primaryWidgets[id] = widget;
}

/***********************************************************************
 ***********************************************************************
 ***********************************************************************
 *
 * 	UTILITY ROUTINES...
 *
 ***********************************************************************
 ***********************************************************************
 ***********************************************************************/

/************************************************************************
 *
 * 	updateStatusDisplay()... update the changing time numbers
 *
 ************************************************************************
 *
 * All of the states being serviced below make this routine a disgusting
 * mess.  The object of the mess is to update the running track/index and
 * time displays only when we have to (ie: when they change).  We go to this
 * trouble because when you update a time label the display has a tendency
 * to blink, which is damn annoying.
 */

#define printLabel(string, widget) \
    xstr = XmStringCreate(string, XmSTRING_ISO8859_1); \
    XtSetArg(argList[0], XmNlabelString, xstr); \
    XtSetValues(widget, argList, 1); \
    XmStringFree(xstr)

static void updateStatusDisplay()
{
    static int lastIndex = 0;
    static int lastTrack = 0;
    static int lastFlags = 0;
    
    char disc[64], track[64], trackNum[64], indexNum[64];
    XmString xstr;
    Arg argList[16];
    
    status.flags |= CD_EXCLUSIVE; 	/* no interruptions... */

    /*
     * if the disc is stopped, put the totals on the board
     */

    if (status.state == PLAYING ||  status.state == PAUSED) {
	/*
	 * update the status display's time and track info
	 */
	
	switch (status.flags & CD_TIME_MASK) {
	    case CD_REMAINING:
		sprintf(track, "%02d:%02d",
			(toc.entry[toc.current].track_length -
			 status.current_seconds) /60,
			(toc.entry[toc.current].track_length -
			 status.current_seconds) %60);
		sprintf(disc, "%02d:%02d",
			(toc.total_time - status.current_total_seconds)/60,
			(toc.total_time - status.current_total_seconds)%60);
		break;

	    case CD_ELAPSED:
		sprintf(track, "%02d:%02d", status.current_seconds/60,
			status.current_seconds%60);
		sprintf(disc, "%02d:%02d", status.current_total_seconds/60,
			status.current_total_seconds%60);
		break;	
	
	    case CD_TOTAL:
		sprintf(track, "%02d:%02d",
			toc.entry[toc.current].track_length/60,
			toc.entry[toc.current].track_length%60);
		sprintf(disc, "%02d:%02d",
			toc.total_time/60,
			toc.total_time%60);
		break;
	}

	/*
	 * update the running times if we're in REMAINING or ELAPSED mode,
	 * OR if this is the first time through in TOTAL mode, OR if
	 * TOTAL mode and the track number just changed.
	 */

	if (status.flags & CD_TOTAL) {
	    if (!(lastFlags & CD_TOTAL)) {
		printLabel(track, primaryWidgets[k_trackTime_id]);
		printLabel(disc, primaryWidgets[k_discTime_id]);
	    } else if (lastTrack != toc.entry[toc.current].track_number + 1) {
		printLabel(track, primaryWidgets[k_trackTime_id]);
	    }
	}

	if (status.flags & (CD_REMAINING | CD_ELAPSED)) {
	    /*
	     * If in SHUFFLE mode, then disc time remaining or elapsed
	     * makes no sense, so display total disc time instead.
	     */
	    
	    if (status.flags & CD_SHUFFLE) {
		if (!(lastFlags & CD_SHUFFLE)) {
		    sprintf(disc, "%02d:%02d", toc.total_time/60,
			    toc.total_time%60);
		    printLabel(disc, primaryWidgets[k_discTime_id]);
		}
		printLabel(track, primaryWidgets[k_trackTime_id]);
	    } else {
		printLabel(track, primaryWidgets[k_trackTime_id]);
		printLabel(disc, primaryWidgets[k_discTime_id]);
	    }
	}

	/*
	 * if the track number/index changed, update it
	 */

	if (lastTrack != toc.entry[toc.current].track_number + 1) {
	    sprintf(trackNum, "%02d", toc.entry[toc.current].track_number + 1);
	    printLabel(trackNum, primaryWidgets[k_trackNum_id]);
	    printLabel(track, primaryWidgets[k_trackTime_id]);
	}

	if (lastIndex != status.current_index) {
	    sprintf(indexNum, "%02d", status.current_index);
	    printLabel(indexNum, primaryWidgets[k_indexNum_id]);
	}

	lastFlags = status.flags;
	lastIndex = status.current_index;
	lastTrack = toc.entry[toc.current].track_number + 1;
    }

    /*
     * if the device isn't in run mode, then display the totals or nothing.
     * When the device is stopped this code is not continually called
     * so the display won't blink, therefore we don't bother to check
     * if the below display work is redundant.
     */
    
    else {
	if (status.state == STOPPED || status.state == IDLE) {
	    sprintf(disc, "%02d:%02d", toc.total_time/60, toc.total_time%60);
	} else {
	    strcpy(disc, "00:00");
	}

	strcpy(track, "00:00");
	strcpy(trackNum, "00");
	strcpy(indexNum, "00");

	printLabel(trackNum, primaryWidgets[k_trackNum_id]);
	printLabel(indexNum, primaryWidgets[k_indexNum_id]);
	printLabel(track, primaryWidgets[k_trackTime_id]);
	printLabel(disc, primaryWidgets[k_discTime_id]);

	lastTrack = 0;
	lastIndex = 0;
	lastFlags = status.flags;
    }

    status.flags &= ~CD_EXCLUSIVE;
    
} /* updateStatusDisplay */

/**********************************************************************
 *
 * 	updateSelectDisplay()... 
 * 	
 **********************************************************************
 *
 * relabel the select buttons
 * 
 * If there's now a different number of tracks than the last time we
 * did this, the disk must've changed.  If there aren't enough select
 * button widgets to meet demand, we make some more.
 */

static void updateSelectDisplay()
{
    static int buttonCount = 0;
    static Boolean initial_rows_set = False;
    MrmCode class;
    Arg argList[1];
    XmString xstr;
    char buf[16];
    int i, n, np, x, y, height, width, newrows;

    /*
     * display the new button picture
     */

    if (toc.total_tracks == 0) {
	return;
    }
    
    /*
     * If we don't now have as many buttons as total_tracks, I guess the
     * disk changed.  Go make more buttons.
     */

    if (toc.total_tracks > buttonCount) {
	for (i = buttonCount; i < toc.total_tracks; i++) {
	    /* create the new widget by copying selectButtons */

	    sprintf(buf, "selectButton%d", i+1);
	    selectWidgets[i] = 0;

	    /* create the new widget by copying selectButtonTemplate */
	    MrmFetchWidgetOverride(s_MrmHierarchy,
				   "selectButtonTemplate",
				   primaryWidgets[k_selectPad_id],
				   buf,
				   argList, 0,
				   &selectWidgets[i], &class);

	    /*
	    * setup the the new widget's callback to pass a pointer
	    * to its button number at activate time
	    */
	    
	    selectTags[i] = i;

	    XtAddCallback(selectWidgets[i], XmNactivateCallback,
			  select_button_activate, (XtPointer) &selectTags[i]);

	    buttonCount++;
	}
    }
    
    for (i = 0; i < toc.total_tracks; i++) {
	/* label each button with the number of the track to be played */
	sprintf(buf, "%d", toc.entry[i].track_number + 1);
	xstr = XmStringCreate(buf, XmSTRING_ISO8859_1);
	XtSetArg(argList[0], XmNlabelString, xstr);
	XtSetValues(selectWidgets[i], argList, 1);
	XmStringFree(xstr);
    }

    if (options.selectKeysPerRow > 0) {
	newrows = toc.total_tracks / options.selectKeysPerRow;
	if (toc.total_tracks % options.selectKeysPerRow)
	    newrows++;			/* one more for leftovers */
    } else {
	newrows = initial_rows;
    }

    if (newrows != initial_rows || !initial_rows_set) {
	initial_rows = newrows;
	XtSetArg(argList[0], XmNnumColumns, initial_rows); 
	XtSetValues(primaryWidgets[k_selectPad_id], argList, 1);
	initial_rows_set = True;
    }

    XtManageChildren( selectWidgets, toc.total_tracks );
   
    if (buttonCount > toc.total_tracks)
	XtUnmanageChildren( &selectWidgets[toc.total_tracks],
			    buttonCount - toc.total_tracks );

} /* updateSelectDisplay */

/**********************************************************************
 *
 * 	lightMainButton()...
 *
 **********************************************************************
 *
 * Highlight the main button passed in.  Unhighlight the last button
 * that was passed in, if any.
 *
 * By convention, the static int "buttonLit" contains the widget id of
 * the last button we lit up.  It contains -1 if no buttons are currently
 * lit.
 *
 * The calling convention is that an input value of -1 causes the
 * currently lit up track (if any) to be unhighlighted.
 */

static void lightMainButton
#ifdef _NO_PROTO
(buttonToLight)
    int	buttonToLight;
#else
(
    int	buttonToLight
)
#endif /* _NO_PROTO */
{
    static int buttonLit = -1;
    Arg argList[16];
    int n;

    /*
     * if the button to be lit is already lit up, we got nothing to do!
     */
    
    if (buttonToLight == buttonLit) {
	return;
    }

    /*
     * if there's a button currently highlighted, shut it off.
     */

    if (buttonLit != -1) {
	XtSetArg(argList[0], XmNlabelPixmap, passiveIconName[buttonLit]);
	MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[buttonLit],
			  argList, 1);

	buttonLit = -1;
    }

    /*
     * if there's a button to be highlighted, light it up.
     */
    
    if (buttonToLight != -1) {
	XtSetArg(argList[0], XmNlabelPixmap, activeIconName[buttonToLight]);
	MrmFetchSetValues(s_MrmHierarchy, primaryWidgets[buttonToLight],
			     argList, 1);

	buttonLit = buttonToLight;
    }
    
} /* lightMainButton */

/**********************************************************************
 *
 * 	lightSelectButton()...
 * 	
 **********************************************************************
 *
 * Highlight the selection button that corresponds to the currently
 * selected track.  Unlight the previous selection, if there is one.
 *
 * By convention, the static int "buttonLit" contains the number
 * of the last track we lit up.  It contains -1 if no tracks are
 * currently lit.
 *
 * The calling convention is that an input value of -1 causes the
 * currently lit up track (if any) to be unhighlighted.
 */

static void lightSelectButton
#ifdef _NO_PROTO
(buttonToLight)
    int buttonToLight;
#else
(
    int buttonToLight
)
#endif /* _NO_PROTO */
{
    static int buttonLit = -1;
    Arg argList[16];
    int n;

    if (buttonToLight == buttonLit) {
	return;
    }
	
    /*
     * if there's a track currently highlighted, shut it off.
     */
    if (buttonLit != -1) {
	n = 0;
	XtSetArg(argList[n], XmNbackground, lowlightColorBG); n++;
	XtSetArg(argList[n], XmNforeground, lowlightColorFG); n++;

	if (selectWidgets[buttonLit])
	    XtSetValues(selectWidgets[buttonLit], argList, n);

	buttonLit = -1;
    }

    /*
     * if there's a track to be highlighted, light it up.
     */
    
    if (buttonToLight != -1) {
	n = 0;
	XtSetArg(argList[n], XmNforeground, highlightColorFG); n++;
	XtSetArg(argList[n], XmNbackground, highlightColorBG); n++;

	if (selectWidgets[buttonToLight])
	    XtSetValues(selectWidgets[buttonToLight], argList, n);

	buttonLit = buttonToLight;
    }

} /* lightSelectButton */

/************************************************************************
 *
 * 	setupTOC()... build the table of contents
 *
 ************************************************************************
 *
 * Build a table of contents from hardware information.  The hardware
 * information must have been gathered during poll_device(), so be sure
 * poll_device() gets called before you come here.
 *
 * The table of contents is built in random order if shuffle is active,
 * sequential order otherwise.
 *
 * This routine also updates the select and status displays according
 * to the new TOC info.
 */

#define CD_DIRECTORY_TIME 2

static void setupTOC()
{
    int i, j, n;

    if (status.state == EJECTED)
	return;

    /*
    * build the toc entries for either shuffle or serial order
    */
    
    toc.total_time = CD_DIRECTORY_TIME;
    
    if (status.flags & CD_SHUFFLE) {
	long shuffleTrack;
	int gotNum[MAXTRACKS];

	BZERO(gotNum, MAXTRACKS * sizeof(int));
    	i = 0;
	
	while (i < toc.total_tracks) {
	    shuffleTrack = random() % toc.total_tracks;
	
	    if (gotNum[shuffleTrack] == 0) {
		gotNum[shuffleTrack] = -1;

		toc.entry[i].track_number = shuffleTrack;
		toc.entry[i].track_length = getTrackLength(shuffleTrack);
		toc.total_time += toc.entry[i].track_length;

		i++;	/* only increment when we find a new track */
	    }
	}
    } else {
	for (i = 0; i < toc.total_tracks; i++) {
	    toc.entry[i].track_number = i;
	    toc.entry[i].track_length = getTrackLength(i);
	    toc.total_time += toc.entry[i].track_length;
	}
    }

    /*
     * Compute and record the start address of each track in seconds.
     * We need this to know when the "scan" function crosses tracks.
     */

    n = CD_DIRECTORY_TIME;
    
    for (i = 0; i < toc.total_tracks; i++) {
	for (j = 0; j < toc.total_tracks; j++) {
	    if (toc.entry[j].track_number == i) {
		toc.entry[j].track_address = n;
		n += toc.entry[j].track_length;
		continue;
	    }
	}
    }

    toc.current = 0;

    updateSelectDisplay();	/* unmap the select buttons */
    
} /* setupTOC */

/************************************************************************
 *
 * 	clearTOC()... zero out the table of contents
 *
 ************************************************************************/

static void clearTOC()
{
    BZERO(toc.entry, MAXTRACKS * sizeof(tocEntryRec));
    
    toc.current = 0;
    toc.last = 0;
    toc.total_tracks = 0;
    toc.total_time = 0;

} /* clearTOC */


static void selectpad_resize_handler
#ifdef _NO_PROTO
(widget, client_data, event, continue_to_dispatch)
    Widget	widget;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*continue_to_dispatch;
#else
(
    Widget	widget,
    XtPointer	client_data,
    XEvent	*event,
    Boolean	*continue_to_dispatch
)
#endif /* _NO_PROTO */
{
    XConfigureEvent	*cevent = (XConfigureEvent *)event;
    int			n, nrows;
    Arg 		argList[1];

    if ( (event->type != ConfigureNotify) || (toc.total_tracks == 0) )
	return;

    n = 1;
    while (n <= toc.total_tracks) {
	if ( ((n*selbtn_width) + ((n-1)*selpad_spacing)) >
		cevent->width ) {
	    n--;
	    break;
	} else {
	    n++;
	}
    }

    nrows = toc.total_tracks / n;
    if (toc.total_tracks % n) nrows++;
    if (nrows != initial_rows) {
	initial_rows = nrows;
    	XtSetArg(argList[0], XmNnumColumns, nrows);
    	XtSetValues(primaryWidgets[k_selectPad_id], argList, 1);
    }
}

/*
 *  Define the application icon image bitmap.
 */
/************************************************************************
 *
 * 	set_icon()... 	set the icon pixmap according to what the window
 *			manager wants us to do.
 *
 ************************************************************************/

static Pixmap 	cd_icon_pixmap = NULL;
static int	current_icon_width = 0;

static void reparent_handler
#ifdef _NO_PROTO
(widget, client_data, event, continue_to_dispatch)
    Widget	widget;
    XtPointer	client_data;
    XEvent	*event;
    Boolean	*continue_to_dispatch;
#else
(
    Widget	widget,
    XtPointer	client_data,
    XEvent	*event,
    Boolean	*continue_to_dispatch
)
#endif /* _NO_PROTO */
{
    XReparentEvent      *reparent = (XReparentEvent *) &event->xreparent;
    if (event->type != ReparentNotify)
	return;
    if (reparent->parent == XDefaultRootWindow(display))
	return;
    set_icon();
}

static void set_icon()
{
    Window 	window;
    Arg 	argList[1];
    XIconSize	*size_list_return;
    int	      	count_return;
    Status	status;
    Boolean	made_new_pixmap;

    window = XtWindow(toplevel);

    /* See what the window manager says to do. */
    status = XGetIconSizes(display,
			   RootWindowOfScreen(XtScreen(toplevel)),
			   &size_list_return,
			   &count_return);
    if (status == 0)			/* wm has not set any sizes - punt */
	return;

    /* find largest of our icons which is <= max */
    if ( ( cd_large_icon_width < size_list_return->max_width) &&
        ( cd_large_icon_height < size_list_return->max_height) ) {
	if (current_icon_width == cd_large_icon_width)
	    made_new_pixmap = False;
	else {
	    if (cd_icon_pixmap != NULL)
		XFreePixmap(display, cd_icon_pixmap);
	    cd_icon_pixmap = XCreateBitmapFromData(display, window,
				    cd_large_icon_bits,
				    cd_large_icon_width,
				    cd_large_icon_height);
	    current_icon_width = cd_large_icon_width;
	    made_new_pixmap = True;
	}
    } else {
	if( ( cd_medium_icon_width < size_list_return->max_width) &&
            ( cd_medium_icon_height < size_list_return->max_height) ) {
	    if (current_icon_width == cd_medium_icon_width)
		made_new_pixmap = False;
	    else {
		if (cd_icon_pixmap != NULL)
		    XFreePixmap(display, cd_icon_pixmap);
		cd_icon_pixmap = XCreateBitmapFromData(display, window,
					cd_medium_icon_bits,
				        cd_medium_icon_width,
				        cd_medium_icon_height);
		current_icon_width = cd_medium_icon_width;
		made_new_pixmap = True;
	    }
	} else {
	    if( ( cd_small_icon_width < size_list_return->max_width) &&
	        ( cd_small_icon_height < size_list_return->max_height) ) {
		if (current_icon_width == cd_small_icon_width)
		    made_new_pixmap = False;
		else {
		    if (cd_icon_pixmap != NULL)
			XFreePixmap(display, cd_icon_pixmap);
		    cd_icon_pixmap = XCreateBitmapFromData(display, window,
				           cd_small_icon_bits,
				           cd_small_icon_width,
				           cd_small_icon_height);
		    current_icon_width = cd_small_icon_width;
		    made_new_pixmap = True;
		}
	    }
	}
    }

    if (made_new_pixmap) {
	XtSetArg(argList[0], XmNiconPixmap, cd_icon_pixmap);
	XtSetValues(toplevel, argList, 1);
    }
    
} /* set_icon */

/************************************************************************
 *
 * open_dev()	Open the physical device
 *
 ************************************************************************
 *
 * The open will not complete until the CD is inserted into the drive.
 * This routine is provided to allow the program to be started before the
 * CD is inserted and later allow the open to complete.
 *
 * Returns:
 * 	0 = success
 *	-1 = failure
 */

static int open_dev()
{
    int fd = 0;

    /* Attempt to open the CD-ROM device */

    if (options.cddev) {
	if ((status.flags & CD_DEVICE_OPEN) == 0) {
	    /* if we've tried opening within the last 5 seconds, skip it */
	    if (((status.flags & CD_TIMED_OUT) != 0) && 
		((time(NULL) - status.current_seconds) < 5)) {
		return(-1);
	    } else {
		if ((fd = open(options.cddev, O_RDWR)) >= 0) {
		    set_fd(fd); 	/* Set the fd for the util routines */
		    status.flags &= ~CD_TIMED_OUT;
		    status.current_seconds = 0;
		    status.flags |= CD_DEVICE_OPEN;
		    return(0);
		} else if (errno == ETIMEDOUT || errno == EIO) {
		    status.flags |= CD_TIMED_OUT;
		    status.current_seconds = time(NULL);
		}
	    }
	}
    }
    return(-1);
    
} /* open_dev */

/***********************************************************************
 ***********************************************************************
 ***********************************************************************
 *
 * 	DEVICE LAYER...
 *
 ***********************************************************************
 ***********************************************************************
 ***********************************************************************
 *
 * This layer presents a virtual device interface to the rest of the
 * program.  The routines here provide abstractions that are hardware-like
 * in that they are atoimic functions, yet they may not be implemented in
 * the hardware.  In short, this interface presents an idealized view of
 * device command/control functions.
 *
 * The routines are:
 *
 * play_track() - begin play at a specific track
 * stop_device() - stop the play
 * pause_device() - pause the play
 * resume_play() - resume play after a pause command
 * next_track() - play the track following the currently playing one
 * prev_track() - play the track preceeding the currently playing one
 * eject_device() - barf out the CD
 * poll_device() - update the status and toc structs from device info
 * set_volume() - set (scale) the volume level
 *
 * Button highlighting for the main buttons and the selection buttons
 * is done in this layer.
 *
 */

/************************************************************************
 *
 *	play_track()... play the selected track
 *
 ***********************************************************************/

static void play_track()
{
    status.flags |= CD_EXCLUSIVE;
    status.state = PLAYING;
    
    if (status.flags & CD_SHUFFLE) {
	PlayTrack(toc.entry[toc.current].track_number + 1,
		  toc.entry[toc.current].track_number + 1);
    } else {
	PlayTrack(toc.entry[toc.current].track_number + 1,
		  toc.entry[toc.last].track_number + 1);
    }	
    
    /*
     * reset volume after every play command or else it defaults to the
     * maximum value (ouch!)
     */

    set_volume(status.currentVolume);

    /*
     * update the lights
     */
    
    lightMainButton(k_play_id);
    lightSelectButton(toc.current);

    poll_device();    /* get new numbers for the status struct */

    updateStatusDisplay();
    
    if (status.timer == 0)
	start_timer(primaryWidgets[k_play_id]);

    /*
     * after each new disk insertion, the device forgets that it was locked.
     * Since it's now possible to eject a locked disk using sthe front
     * panel button, we need to relock when a disk is re-inserted.
     */
    
    if (status.flags & CD_PREVENT)
	PreventRemoval();

    status.flags &= ~CD_EXCLUSIVE;
    
} /* play_track */

/************************************************************************
 *
 * 	next_track()... select the next CD track
 *
 ************************************************************************/

static void next_track() 
{
    if (toc.current < toc.last)
	toc.current++;
    else
	toc.current = 0;
    
    play_track();
    
} /* next_track */

/************************************************************************
 *
 * 	prev_track()... select the previous track
 *
 ************************************************************************
 *
 * On a real CD player, the 1st time you punch " |<< " the current
 * track starts over.  If you quickly punch it again, the previous track
 * plays and so on.  To emulate that function, if the track has
 * played more than 2 seconds we assume you are punching " |<< " the 1st
 * time in the current command sequence and restart play at the current
 * track.  Else, we play the previous track.
 */

static void prev_track()
{
    if (status.current_seconds < 2) {
	if (toc.current)
	    --toc.current;
	else
	    toc.current = toc.last;
    }
    
    play_track();
    
} /* prev_track */

/************************************************************************
 *
 * 	pause_device()... pause the CD in play, if any
 *
 ************************************************************************/

static void pause_device() 
{
    status.flags |= CD_EXCLUSIVE;
    
    stop_timer();
    lightMainButton(k_pause_id);

    PausePlay();

    status.state = PAUSED;
    status.flags &= ~CD_EXCLUSIVE;
  
} /* pause_device */

/************************************************************************
 *
 * 	resume_device()... start playing agin following a pause
 *
 ************************************************************************/

static void resume_device() 
{
    if (status.state == PAUSED) {
	status.flags |= CD_EXCLUSIVE;

	if (status.timer == 0)
	    start_timer(primaryWidgets[k_pause_id]);

	status.state = PLAYING;
	lightMainButton(k_play_id);

	ResumePlay();

	status.flags &= ~CD_EXCLUSIVE;
    }
  
} /* resume_device */

/************************************************************************
 *
 * 	stop_device()... stop the player
 *
 ************************************************************************/

static void stop_device() 
{
    status.flags |= CD_EXCLUSIVE;

    status.state = STOPPED;
    toc.current = 0;

    status.current_track = 1;
    status.current_seconds = 0;
    status.current_index = 0;

    lightMainButton(k_stop_id);
    lightSelectButton(-1);

    StopUnit();

    stop_timer();

    updateStatusDisplay();
    updateSelectDisplay();

    /*
    * after each new disk insertion, the device forgets that it was locked.
    * Since it's now possible to eject a locked disk using sthe front
    * panel button, we need to relock when a disk is re-inserted.
    */
    
    if (status.flags & CD_PREVENT)
	PreventRemoval();

    status.flags &= ~CD_EXCLUSIVE;

} /* stop_device */

/************************************************************************
 *
 * 	eject_device()... barf up a CD
 *
 ************************************************************************/

static void eject_device() 
{
    status.flags |= CD_EXCLUSIVE;

    status.state = EJECTED;

    status.current_track = 1;
    status.current_seconds = 0;
    status.current_index = 0;

    clearTOC();

    lightMainButton(k_eject_id);
    lightSelectButton(-1);

    updateSelectDisplay();	/* unmap the select buttons */
    updateStatusDisplay();
    
    stop_timer();

    EjectUnit();

    status.flags &= ~CD_EXCLUSIVE;

} /* eject_device */

/********************************************************************
 *
 * 	set_volume()... device independant volume setting
 *
 ********************************************************************
 *
 * the input volume value from the volumeSlider widget is in the range
 * 0-100.  The hardware supports volume settings 0-255 so we scale the
 * input values to the device specific range.
 *
 * Also, there are a few inherent problems with the hardware's volume scale.
 * First, in most cases, volume settings below 128 are barely audible, so
 * the hardware's effective effective "real" volume range is from 128 to 255,
 * even though the full 0-255 range is legal. Second, volume deltas appear
 * non-linear to the human ear (ie: your ear thinks a volume change of 5 units
 * is less than the same 5 unit change heard at high volume levels).
 *
 * We solve both these problems by scaling volume slider values of 1-50 to
 * actual values of 128-219 while scaling volume slider values of 51-100 to
 * a smaller range of 220-255.  Zero always scales to zero.  Not the slickest
 * scaling algorithm, but it works.
 */

static void set_volume
#ifdef _NO_PROTO
(sliderValue)
    int sliderValue;
#else
(
    int sliderValue
)
#endif /* _NO_PROTO */
{
    int scaledValue;
    float percent;

    status.currentVolume = sliderValue;
    scaledValue = sliderValue;
    
    if (sliderValue) {
	if (sliderValue <= 50) {
	    percent = scaledValue / (float)50;
	    scaledValue = (percent * 91) + 128;
	} else {
	    percent = (sliderValue - 50) / (float)50;
	    scaledValue = (percent * 35) + 220;
	}
    }
    
    /* same volume for right and left */

    status.flags |= CD_EXCLUSIVE;

    SetVolume(scaledValue, scaledValue);

    status.flags &= ~CD_EXCLUSIVE;

} /* set_volume */

/************************************************************************
 *
 * 	start_timer()... start the timer
 *
 ************************************************************************/

static void start_timer
#ifdef _NO_PROTO
(w)
    Widget w;
#else
(
    Widget w
)
#endif /* _NO_PROTO */
{
    status.timer = XtAppAddTimeOut(XtWidgetToApplicationContext(w), 
			950, timeout_proc, w);
}

/************************************************************************
 *
 *	stop_timer()... stop the timer
 *
 ************************************************************************/

static void stop_timer()
{
    if (status.timer != 0)
	XtRemoveTimeOut(status.timer);

    status.timer = 0;
    
} /* stop_timer */

/************************************************************************
 *
 * 	timeout_proc()... timeout procedure
 *
 ************************************************************************
 *
 * This routine is called every second.
 *
 * It updates the highlighting of the select buttons to display the
 * current track, the numbers in the status display window, and handles
 * the various repeat modes.
 *
 * NOTE:  When the caddy is ejected, the timeout is stopped to avoid
 * 	  polling the CDROM.  When the CDROM is polled during it's
 * 	  ejected state, an error message is sent to the log file, which
 * 	  should be avoided.  The timeout is started again when a play
 * 	  command is executed.
 */

static void timeout_proc
#ifdef _NO_PROTO
(data, t)
    XtPointer data;
    XtIntervalId *t;
#else
(
    XtPointer data,
    XtIntervalId *t
)
#endif /* _NO_PROTO */
{
    Widget w = (Widget)data;
    int i;

    if ((status.flags & CD_EXCLUSIVE) == 0) {
	poll_device();
	updateStatusDisplay();
		    
	switch (status.state) {
	    case IDLE:

		/*
		 * if the player is idle we've either finished the disk or
		 * we're in shuffle mode and between tracks.  Handle the
		 * various repeat modes.
		 */
	    
		if (toc.current >= toc.last) {
		    if (status.flags & CD_REPEAT_DISK) {
			next_track();
		    } else if (status.flags & CD_REPEAT_TRACK) {
			play_track();
		    } else {
			stop_device();
			return;
		    }
		} else {
		    if (status.flags & CD_REPEAT_TRACK)
			play_track();
		    else
			next_track();
		}

		break;
	    
	    case PLAYING:
	    
		/*
		 * if not in shuffle mode and...
		 * if the track now playing no longer matches the the
		 * track that the TOC thinks is playing...
		 */

		if (!(status.flags & CD_SHUFFLE)) {
		    if (toc.entry[toc.current].track_number !=
				status.current_track-1) {
			if (status.flags & CD_REPEAT_TRACK) {
			    play_track();
			} else {
			    /*
			     * fix toc.current and the select button
			     * highlighting to reflect new reality.
			     */
			    for (i = 0; i < toc.total_tracks; i++) {
				if (toc.entry[i].track_number ==
						status.current_track-1)
				    toc.current = i;
			    }
			    lightSelectButton(toc.current);
			}
		    }
		}
		break;
	    
	    case STOPPED:
	    case EJECTED:
		return;
		
	    default:
		break;
	}
    }

    /*
     * set the timer to fire again.
     */

    status.timer = XtAppAddTimeOut(XtWidgetToApplicationContext(w), 
				   950, timeout_proc, w);
    
} /* timeout_proc */

/***********************************************************************
 *
 *	start_scan_timer()...
 *
 **********************************************************************/

static void start_scan_timer
#ifdef _NO_PROTO
(w)
    Widget w;
#else
(
    Widget w
)
#endif /* _NO_PROTO */
{
    status.scan_timer = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
					SCAN_BITE, scan_timeout, w);

} /* start_scan_timer */

/***********************************************************************
 *
 *	stop_scan_timer()...
 *
 **********************************************************************/

static void stop_scan_timer()
{
    if (status.scan_timer != 0)
	XtRemoveTimeOut(status.scan_timer);
    
    status.scan_timer = 0;

} /* stop_scan_timer */

/***********************************************************************
 *
 *	scan_timout()...
 *
 **********************************************************************
 *
 * This routine implements the audio cues in scan mode.  You enter
 * scan mode by holding down a scan button (">>" or "<<") and exit
 * when you release the button.  When you press a scan button a
 * "scan_timer" is started which causes this routine to get called
 * every "SCAN_BITE" milliseconds.
 *
 * When this routine runs, it restarts play at a point "status.scan_stride"
 * seconds from the current position.  Play then continues until the timer
 * fires and this routine runs again, restarting play at the next position.
 *
 * There is a crude accelerator that causes the stride to get one
 * second larger up to MAX_SCAN_STRIDE seconds, each time this routine runs.
 *
 */

static void scan_timeout
#ifdef _NO_PROTO
(data, t)
    XtPointer	data;
    XtIntervalId *t;
#else
(
    XtPointer	data,
    XtIntervalId *t
)
#endif /* _NO_PROTO */
{
    Widget w = (Widget)data;
    union cd_address startTime, endTime;
    time_t seconds;
    int newTrack = FALSE;
    
    start_scan_timer(w);
    
    if ((status.flags & CD_EXCLUSIVE) == 0) {
	poll_device();

	/*
	 * compute the absolute address of the new play point expressed in
	 * seconds.  This calculation accounts for the possibility that
	 * we're in shuffle mode with its non-contiguous tracks.
	 */

	startTime.msf.f_units = 0;
	
	switch(status.scan_direction) {
	    case BACK:
	     
		if (status.current_total_seconds - status.scan_stride <
			toc.entry[toc.current].track_address) {
		    if (toc.current)
			--toc.current;
		    else
			toc.current = toc.last;
		
		    seconds = (toc.entry[toc.current].track_address +
				toc.entry[toc.current].track_length) -
				(status.scan_stride - status.current_seconds);

		    newTrack = TRUE;
		} else {
		    seconds = status.current_total_seconds - status.scan_stride;
		}

		startTime.msf.m_units = seconds / 60;
		startTime.msf.s_units = seconds % 60;
		break;

	    case AHEAD:

		if ((status.current_seconds + status.scan_stride) >=
			toc.entry[toc.current].track_length) {
		    seconds = (status.current_seconds + status.scan_stride) - 
				toc.entry[toc.current].track_length;
		
		    if (toc.current == toc.last)
			toc.current = 0;
		    else
			toc.current++;

		    seconds += toc.entry[toc.current].track_address;

		    newTrack = TRUE;
		} else {
		    seconds = status.current_total_seconds + status.scan_stride;
		}

		startTime.msf.m_units = seconds / 60;
		startTime.msf.s_units = seconds % 60;
		break;

	    case OFF:
		return;
	}

	/*
	 * set play end interval. (we reissue the play command in the
	 * button_disarm callback).
	 */
	
	endTime.msf.f_units = 0;

	if (status.flags & CD_SHUFFLE) {
	    seconds = (toc.entry[toc.current].track_address +
		      toc.entry[toc.current].track_length);
	} else {
	    seconds = toc.total_time;
	}
	    
	endTime.msf.s_units = seconds % 60;
	endTime.msf.m_units = seconds / 60;

	PlayTime(startTime, endTime);

	/*
	 * If you don't set volume, it defaults to the wrong value
	 */

	set_volume(status.currentVolume);	

	/*
	 * Primitive scan speed acceleration; the longer you hold down
	 * the scan button, the more music we jump over between samples.
	 */

	if (status.scan_stride < MAX_SCAN_STRIDE)
	    status.scan_stride += 2;

	if (newTrack) {
	    lightSelectButton(toc.current);
	    poll_device();	/* get new numbers for display update */
	}
    }

    updateStatusDisplay();

} /* scan_timeout */

/************************************************************************
 *
 *	poll_device()... read status from the hardware
 *
 ************************************************************************
 *
 * This routine put status info from the hardware into the global
 * "status" structure.
 *
 * It also loads toc.total_tracks and toc.last
 */

static void poll_device()
{
    struct cd_subc_information sci;
    struct cd_subc_header *sch;
    struct cd_subc_position *scp;
    struct cd_toc_header *th;
    struct cd_toc_entry *tocentp;     /* Pointer to array of entries	*/
    union cd_address *abaddr, *caddr;
    int csecaddr, absecaddr;
    int i;
    
    sch = &sci.sci_header; 
    scp = &sci.sci_scp;  	
    abaddr = &scp->scp_absaddr;

    /*
     * If the device is not currently open, try the open again
     */

    if ((status.flags & CD_DEVICE_OPEN) == 0) {
	if (open_dev()) {
	    status.state = EJECTED;
	    clearTOC();
	    return;
	}
	status.flags |= CD_DEVICE_OPEN;
    }
    
    tocentp = glob_toc.cdte;
    th = &glob_toc.cdth;
    
    /*
     * poll the device for status and update out own status info.
     */
    
    if (GetPlayPosition(&sci)) {
	/* The disk must have been removed */

	status.state = EJECTED;
	status.current_track = 1;
	status.current_seconds = 0;
	status.current_index = 0;

	clearTOC();
	lightMainButton(k_eject_id);
	lightSelectButton(-1);
	updateSelectDisplay();	/* unmap the select buttons */
	stop_timer();
	return;
    }

    status.current_track = scp->scp_track_number;
    status.current_index = scp->scp_index_number;

    if(GetTOC(&glob_toc) == 0) {
	toc.total_tracks = (th->th_ending_track - th->th_starting_track) + 1;
	toc.last = toc.total_tracks - 1;
    } else {
	stop_device();
	clearTOC();
    }
    
    /*
     * if the device is in play mode...
     */

    if (sch->sh_audio_status == AS_PLAY_IN_PROGRESS) {
	status.state = PLAYING;

	/*
	 * calculate the number of seconds we're into the current track...
	 */
	
	for (i = 0, caddr = 0; i < toc.total_tracks; i++) {
	    if (tocentp[i].te_track_number == status.current_track) { 
		caddr = &tocentp[i].te_absaddr;
		break;
	    }
	}

	/*
	 * caddr is address of beginning of current track.
	 * abaddr is address of current position.
	 */

	if (caddr != 0) {
	    csecaddr = (caddr->msf.m_units * 60) + caddr->msf.s_units;
	    absecaddr = (abaddr->msf.m_units * 60) + abaddr->msf.s_units;
	    status.current_seconds = absecaddr - csecaddr;
	    status.current_total_seconds = absecaddr;
	}
    } else {
	status.state = IDLE;
    }

} /* poll_device */

/************************************************************************
 *
 * 	getTrackLength()... get the length of a specific track #
 *
 ************************************************************************
 *
 * This routine reads the hardware structure so poll_device() must have
 * been called sometime after the disk was inserted but before you get
 * here. 
 */

static int getTrackLength
#ifdef _NO_PROTO
(trackNum)
    int trackNum;
#else
(
    int trackNum
)
#endif /* _NO_PROTO */
{
    struct cd_toc_entry *toc_entry;	/* Pointer to array of entries	*/
    union cd_address *thisaddrp, *nextaddrp;	
    int secs;

    toc_entry = glob_toc.cdte;
    
    thisaddrp = &toc_entry[trackNum].te_absaddr;
    nextaddrp = &toc_entry[trackNum + 1].te_absaddr;

    /*
     * using the absolute address of the current track and the next
     * track, calculate the number of seconds in the current track.
     */

    secs = ((nextaddrp->msf.m_units * 60) + nextaddrp->msf.s_units) - 
         ((thisaddrp->msf.m_units * 60) + thisaddrp->msf.s_units);

    return(secs);
    
} /* getTrackLength */

