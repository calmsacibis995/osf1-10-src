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
/*
  Copyright (c) Digital Equipment Corporation, 1987, 1988, 1989, 1990
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.
*/
 

/* PROGRAM DESCRIPTION: 
 * 
 *	This is an X11 program to play the puzzle game.
 *	Puzzle is simple number scremble game where a series of numbers are
 *	randomly place in a square with one blank.  The objective is to reorder
 *	the numbers.
 * 
 * AUTHORS: 
 * 
 *	NFF
 *
 * CREATION DATE:	9-Jun-1987
 *
 * Modfication Date:
 *
 *	Nov,1990  (ASP)	V3.0 - converting to Motif.
 * 
 *			C H A N G E   L O G
 *
 *    Date     |   Name  | Description
 * ------------+---------+-----------------------------------------------------
 * %[change_entry]%
 */
#include "puzzledefs.h"
#include "puzzleexterns.h"
#include "DECspecific.h"

#ifdef UNIX
#include <Mrm/MrmPublic.h>
#include <X11/cursorfont.h>
#include <Xt/decw$cursor.h>
#include <unistd.h>
#else
#include "MrmPublic.h"
#include "cursorfont.h"
#include "decw$cursor.h"
#endif


/*++
**  Forward declarations
**--
*/                                         
extern void 	exit ();
extern void 	NewGame ();
extern void 	makemove ();
extern int	outoforder ();

static void 	popup_procedure ();
static void	key_pressed ();
static void 	tile_hit_proc ();
static void 	Redraw ();

Boolean		test_intersection ();
void 		ResizePuzzle ();

void 		PlaceTiles ();

void 		CreateMain ();
void 		CreateTiles ();
void 		ManageTiles ();
void 		UnmanageTiles ();
static void 	change_num_tiles ();
static void 	puzzle_serror ();
static void	remove_message_box_child ();

static unsigned int	create_level_proc ();
static unsigned int	create_ok_proc ();
static unsigned int	create_reverse_proc ();
static unsigned int	create_work_proc ();

static unsigned int	exit_proc ();
static unsigned int	help_done_proc ();
static unsigned int	main_help_proc ();
static void		on_context_activate_proc ();
static void		tracking_help();
static unsigned int	message_done_proc ();
static unsigned int	new_game_proc ();
static unsigned int	restore_settings_proc ();
static unsigned int	save_settings_proc ();
static unsigned int	settings_cancel_proc ();
static unsigned int	settings_ok_proc ();
static unsigned int	settings_proc ();
static void		Help ();
void			DisplayHelp ();
static unsigned int	save_current_settings ();
/* I18N change */
long                     byte_count, cvt_status;
/* end I18N change */
extern void 	DumpWidgets ();

#ifdef VMS
Opaque			hyper_help_context;
#endif


/* Used for displaying of the Watch Cursor. */
Cursor watch_cursor = NULL;
Cursor WorkCursorCreate ();
void WorkCursorDisplay ();
void WorkCursorRemove ();



#ifdef UNIX
#else
static	char uid_filespec [] = {"DECW$PUZZLE"};
#endif

static	char uid_default [] = {"DECW$SYSTEM_DEFAULTS:.UID"};

static MrmOsOpenParam    os_ext_list;
static MrmOsOpenParamPtr os_ext_list_ptr = &os_ext_list; 

#define success	1


/*
 * the extra stuff we want to add to the main window syntax
 *
 * 
 * 
 */                              
static char main_translation_table [] =
	"<Btn3Down>:		CallMenu()\n\
	 <KeyPress>KP_0:		key_pressed(0)\n\
	 <KeyPress>KP_1:		key_pressed(1)\n\
	 <KeyPress>KP_2:		key_pressed(2)\n\
	 <KeyPress>KP_3:		key_pressed(3)\n\
	 <KeyPress>KP_4:		key_pressed(4)\n\
	 <KeyPress>KP_5:		key_pressed(5)\n\
	 <KeyPress>KP_6:		key_pressed(6)\n\
	 <KeyPress>KP_7:		key_pressed(7)\n\
	 <KeyPress>KP_8:		key_pressed(8)\n\
	 <KeyPress>KP_9:		key_pressed(9)\n\
	 <KeyPress>0:		key_pressed(0)\n\
	 <KeyPress>1:		key_pressed(1)\n\
	 <KeyPress>2:		key_pressed(2)\n\
	 <KeyPress>3:		key_pressed(3)\n\
	 <KeyPress>4:		key_pressed(4)\n\
	 <KeyPress>5:		key_pressed(5)\n\
	 <KeyPress>6:		key_pressed(6)\n\
	 <KeyPress>7:		key_pressed(7)\n\
	 <KeyPress>8:		key_pressed(8)\n\
	 <KeyPress>9:		key_pressed(9)\n\
	 <KeyPress>Return:		key_pressed(return)\n\
	 <KeyPress>osfActivate:		key_pressed(return)\n\
	 <KeyPress>osfDelete: 		key_pressed(cancel)\n\
	 <KeyPress>osfBackSpace:		key_pressed(cancel)\n\
   	 @Help<Btn1Down>:	Help()\n\
	 ~@Help<Btn1Down>:	move_tile()\n\
	 <Expose>:		Redraw()";
                          
/*
 * now establish an action binding table which associates the above
 * procedure names (ascii) with actual addresses
 */
static XtActionsRec our_action_table [] =
    {
        {"CallMenu",		(XtActionProc)popup_procedure},
	{"key_pressed",		(XtActionProc)key_pressed},
        {"move_tile",		(XtActionProc)tile_hit_proc},
        {"Redraw",		(XtActionProc)Redraw},
    	{"Help",		(XtActionProc)Help},
        {NULL, NULL}
    };



/*
 *  This routine creates a "wait" cursor
 */

Cursor WorkCursorCreate (wid)
    Widget  wid;        /* Widget to be used to create the cursor.  The fields
                           used are the display and the colormap.  Any widget
                           with the same display and colormap can be used
                           later with this cursor */
{
/*
 *  Create a wait cursor
 */
    Cursor cursor;
    Font font;
    XColor fcolor, bcolor, dummy;
    int status;

    font = XLoadFont (XtDisplay (wid), "DECw$Cursor");
    status = XAllocNamedColor (XtDisplay (wid),
            wid->core.colormap, "Black", &fcolor, &dummy) ;
    status = XAllocNamedColor (XtDisplay (wid),
            wid->core.colormap, "White", &bcolor, &dummy) ;
    cursor = XCreateGlyphCursor (XtDisplay (wid),
            font, font, decw$c_wait_cursor, decw$c_wait_cursor + 1,
            &fcolor, &bcolor);
    return cursor;
}

/*
 *  This routine displays a cursor a watch cursor in the Clock
 *  window.
 */
void WorkCursorDisplay ()
{


  /* Display the watch cursor in the clock window       */
    if (watch_cursor == NULL) {
      watch_cursor = DXmCreateCursor (toplevel, decw$c_wait_cursor);
    }

   XDefineCursor (XtDisplay(toplevel), XtWindow(toplevel), watch_cursor);

/*
 *  Use XtAddGrab to enable toolkit filtering of events
 */
    XtAddGrab (toplevel , TRUE, FALSE);
    XFlush (XtDisplay(toplevel));
}


/*
 * Return to the normal cursor in both windows
 */
void WorkCursorRemove ()
{
    XUndefineCursor (XtDisplay(toplevel), XtWindow(toplevel));
/*
 *  Use XtRemoveGrab to disable toolkit filtering of events
 */
    XtRemoveGrab (toplevel);
}


/*  This routine removes the corresponding child from a Message Box. */
static void remove_message_box_child (w, child)
    Widget		w;
    unsigned char	child;

{
    XmMessageBoxWidget	mb;

    mb = (XmMessageBoxWidget) w;
    XtUnmanageChild (XmMessageBoxGetChild (mb, child));

}


void PlaceTiles ()
{
int		i, row, col;
Dimension	x, y;

    for (i = 0; i < num_tiles; i++) {
      ac = 0;
      tile [i]. x = (tile [i]. col*tile_width) + left_space;
      tile [i]. y = (tile [i]. row*tile_height) + top_space;
    }
} /* PlaceTiles () */


static	MrmRegisterArg	regvec [] = { 
		{"create_level_proc", (caddr_t)create_level_proc},
		{"create_work_proc", (caddr_t)create_work_proc},
		{"exit_proc", (caddr_t)exit_proc},
		{"help_done_proc", (caddr_t)help_done_proc},
		{"main_help_proc", (caddr_t)main_help_proc},
		{"message_done_proc", (caddr_t)message_done_proc},
		{"new_game_proc", (caddr_t)new_game_proc},
		{"on_context_activate_proc", (caddr_t)on_context_activate_proc},
		{"restore_settings_proc", (caddr_t)restore_settings_proc},
		{"save_settings_proc", (caddr_t)save_settings_proc},
		{"settings_cancel_proc", (caddr_t)settings_cancel_proc},
		{"settings_ok_proc", (caddr_t)settings_ok_proc},
		{"settings_proc", (caddr_t)settings_proc}
	};


void FetchHeierchy ()
{
    MrmType		*dummy_class;
    char		* file_array[1];
    FILE		*fp;

#ifdef UNIX
    extern char *getenv();

  /* get_uid_filename looks up the environment variable UIDDIR, and attempts
     to find a uid file with the correct name in it. */
    char *def_name, *uiddir, *uid_name;
    int uiddir_len, uid_fd;

    /* untested on ultrix */
    os_ext_list.nam_flg.clobber_flg = TRUE;

    file_array [0] = XtMalloc( strlen("/usr/lib/X11/uid/.uid") +
                                  strlen (CLASS_NAME) + 1 );

/*
    sprintf (file_array [0], "/usr/lib/X11/uid/%s.uid", CLASS_NAME);
*/
    sprintf (file_array [0], "%s", CLASS_NAME);

    if ((uiddir = (char *)getenv("UIDDIR")) != NULL) {
	    uiddir_len = strlen(uiddir);
	    if ((def_name = rindex(file_array [0], '/')) == NULL){
		    def_name = file_array [0];
	    }
	    else def_name++;
	    uiddir_len += strlen(def_name) + 2; /* 1 for '/', 1 for '\0' */
	    uid_name = (char *) XtMalloc(uiddir_len * sizeof(char));
	    strcpy(uid_name, uiddir);
	    strcat(uid_name, "/");
	    strcat(uid_name, def_name);
	    if(access(uid_name, R_OK) != 0) {
		    XtFree(uid_name);
	    } else {
		    XtFree(file_array [0]);
		    file_array [0] = uid_name;
	    }
    }
#else
    file_array [0] = uid_filespec;
    os_ext_list.nam_flg.related_nam = 0; 
#endif

    os_ext_list.version = MrmOsOpenParamVersion;
    os_ext_list.default_fname = (char *) uid_default;

  /* Define the DRM "hierarchy" */
    if (MrmOpenHierarchy (1,		/* number of files	*/
			file_array,		/* files     	    	*/
			&os_ext_list_ptr,	/* os_ext_list 		*/
			&puzzle_hierarchy)	/* ptr to returned id   */
			!= MrmSUCCESS)
			    puzzle_serror (NoPuzzleHierarchy, FATAL);
}

void CreateMain ()
{
    MrmType		*dummy_class;
    MrmCount		regnum;
/*
 *    	Register our callback routines so that the resource manager can 
 *    	resolve them at widget-creation time.
 */


    regnum = sizeof (regvec)/sizeof (MrmRegisterArg);
    MrmRegisterNames (regvec, regnum);

  /* make our action table */
    XtAddActions (our_action_table, XtNumber (our_action_table));

    if (MrmFetchWidget (
		puzzle_hierarchy,
		"puzzle_main_window",
		toplevel,
		& main_window,
		& dummy_class) != MrmSUCCESS) {
	puzzle_serror  (NoPuzzleMain, FATAL);
    }

/*
 *  Make "main_window" and all it's children appear on the screen, ready to
 *  interact with.
 */
    XtManageChild (main_window);
}



void CreateGC ()
{
    Pixel		fg, bg;
    XGCValues		gcv;
    unsigned long	GCVsettings;
    XmFontList		FontList;
    XmFontContext	context;
    XmStringCharSet	charset;

    win = XtWindow (workarea);
    ac = 0;
    XtSetArg (args [ac], XmNforeground, & fg); ac++;
    XtSetArg (args [ac], XmNbackground, & bg); ac++;
    XtSetArg (args [ac], XmNtextFontList, &FontList); ac++;
    XtGetValues (workarea, args, ac);

    gcv. foreground = fg;
    gcv. background = bg;

    if (FontList != 0) {
      XmFontListInitFontContext (&context, FontList);
      XmFontListGetNextFont (context, &charset, &font);
      XtFree (charset);
      XmFontListFreeFontContext (context);
/*  This line was replaced by the above lines.
      font = FontList -> font;
*/
    }
    else {
      font = XLoadQueryFont (dpy,
		"-Adobe-Times-Bold-R-Normal--*-180-*-*-P-*-ISO8859-1");
      if (font == 0) {
      font = XLoadQueryFont (dpy, "fixed");
      }
    }

    GCVsettings = GCForeground | GCBackground | GCFont;
    lineheight = font -> max_bounds.ascent + font -> max_bounds.descent;
    gcv. font = font -> fid;

    gcdraw = XCreateGC (dpy, win, GCVsettings, &gcv);
/*
    gcv. foreground = bg;
    gcv. background = fg;
*/
    gcline = XCreateGC (dpy, win, GCVsettings, &gcv);
    gcv. foreground = bg;
    gcv. background = fg;
    gcreverse = XCreateGC (dpy, win, GCVsettings, &gcv);
}



void CreateTiles (w)
Widget w;
{
XmBulletinBoardWidget	pw = (XmBulletinBoardWidget) w; 
int		i, row, col;
char		str [10];

    row = 0; col = 0;

    for (i = 0; i < MAX_TILES; i++) {
      tile [i]. row = row; tile [i]. col = col;
      if ((col = (col+1) % num_across) == 0) row++;
    }

}

/*
 * routine to called from the translation manager when a key is pressed
 */
static void key_pressed (w, event, params, num_params)
    Widget	w;
    XEvent	*event;
    char	**params;
    int		*num_params;
{
    XKeyPressedEvent *key_event = (XKeyPressedEvent *) event;
    int num = *num_params;
    char str [10];
    int digit_enter;

    memcpy (str, params [0], 10);

    if ((strcmp (str, "0") >= 0) && 
	(strcmp (str, "9") <= 0))  {
      digit_enter = atoi (str);
      Number_Entered = (Number_Entered * 10) + digit_enter;
      }
    else if (strcmp (str, "return") == 0){
	   makemove (Number_Entered - 1);
     	   if (!outoforder ()) { GameOver = TRUE; SolvePuzzle (); }
	   }
	 else 
	   Number_Entered = 0;

}                                      


static void tile_hit_proc (w, event, params, num_params)
    Widget   w;
    XEvent  *event;
    char   **params;
    int	     num_params;
{
   int			i, tile_num = -i;
   int			row, col;
   XButtonPressedEvent	*but_event = (XButtonPressedEvent *) event;

   if (!GameOver) {
     row = but_event -> y / tile_height;
     col = but_event -> x / tile_width;
     tile_num = -1;
     for (i=0; i < num_tiles; i++)
       if ((tile [i]. row == row) && (tile [i]. col == col)) tile_num = i;
     if (tile_num >= 0)
       makemove (tile_num);
     if (!outoforder ()) { GameOver = TRUE; SolvePuzzle (); }
   }
}

static void popup_procedure (w, event, params, num_params)
    Widget   w;
    XEvent  *event;
    char   **params;
    int	     num_params;
{
     XmMenuPosition (opt_popup_menu, event);
     XtManageChild (opt_popup_menu);
}                                                                      


void ResizePuzzle (new_width, new_height)
Dimension   	new_width, new_height;
{
int		i;
Dimension	reduced_w, reduced_h;
Dimension     	height_left, width_left;
    
    width = new_width; height = new_height;
    tile_width = width / num_across;
    tile_height = height / num_across;
    height_left = height - (tile_height * num_across);
    width_left = width - (tile_width * num_across);
    bottom_space = Half (height_left);
    top_space = height_left - bottom_space;
    right_space = Half (width_left);
    left_space = width_left - right_space;

    reduced_w = (tile_width / REDUCE_FACTOR) + 1;
    if (reduced_w < MIN_INDENT) reduced_w = MIN_INDENT;
    real_tile_width = tile_width - reduced_w;

    reduced_h = (tile_height / REDUCE_FACTOR) + 1;
    if (reduced_h < MIN_INDENT) reduced_h = MIN_INDENT;
    real_tile_height = tile_height - reduced_h;

    PlaceTiles ();
}

           
static void Redraw (w, event, params, num_params)	/* An expose event */
    Widget   w;
    XEvent  *event;
    char   **params;
    int	     num_params;
{                            
    MrmType		*dummy_class;
    XExposeEvent  	*exp_event = (XExposeEvent  *) event;
    Dimension		cur_width, cur_height;
    int			i;

    if (opt_popup_menu == 0)
      if (MrmFetchWidget (
	puzzle_hierarchy,
	"opt_popup_menu",
	workarea,
	& opt_popup_menu,
	& dummy_class) != MrmSUCCESS) {
	puzzle_serror (NoPuzzlePopup, FATAL);
        }

    if (workarea == NULL) return;

    cur_width = XtWidth (w); cur_height = XtHeight (w);

    if ((cur_width != (width)) || (cur_height != (height)))
      ResizePuzzle (cur_width, cur_height);

    DrawWindow (exp_event -> x, exp_event -> y,
		exp_event -> width, exp_event -> height);

}


/*
*/
DrawWindow (exp_x, exp_y, exp_width, exp_height)
    Position 	exp_x, exp_y;
    Dimension	exp_width, exp_height;
{
    int		i;

    rc = 0; lc = 0;
    for (i = 0; i < num_tiles; i++)
      if ((test_intersection (i, exp_x, exp_y,
		exp_width, exp_height)) || (from_newgame))
        drawtile (i, FALSE);

    from_newgame = FALSE;

    rects [rc]. x = empty_col*tile_width + left_space;
    rects [rc]. y = empty_row*tile_height + top_space;
    rects [rc]. width = tile_width;
    rects [rc]. height = tile_height;
    rc++;

    rects [rc]. x = 0; rects [rc]. y = 0;
    rects [rc]. width = width; rects [rc]. height = top_space;
    rc++;

    rects [rc]. x = 0; rects [rc]. y = 0;
    rects [rc]. width = left_space; rects [rc]. height = height;
    rc++;

    rects [rc]. x = width - right_space; rects [rc]. y = 0;
    rects [rc]. width = right_space; rects [rc]. height = height;
    rc++;

    rects [rc]. x = 0; rects [rc]. y = height - bottom_space;
    rects [rc]. width = width; rects [rc]. height = bottom_space;
    rc++;

    XFillRectangles (dpy, win, gcline, rects, rc);
    XDrawSegments (dpy, win, gcline, segs, lc);
}


/*
 * test_intersection (n, x, y, w, h) determines if tile n
 * intersects the square bound by x, y, w, h
 *                    
 * Input:	 n - the number of the tile
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
Boolean test_intersection (n, x, y, w, h)
    int		n;
    Position	x, y;
    Dimension	w, h;
{
    Position	x1, y1, x2, y2;

    Boolean	result = TRUE;
    Boolean	x_int = TRUE;
    Boolean	y_int = TRUE;

    x1 = tile [n]. x; x2 = x1 + tile_width;
    y1 = tile [n]. y; y2 = y1 + tile_height;

    if (x1 > x)
      if ((x + w) < x1) x_int = FALSE;
      else x_int = TRUE;
    else if (x2 < x) x_int = FALSE;
      else x_int = TRUE;

    if (y1 > y)
      if ((y + h) < y1) y_int = FALSE;
      else y_int = TRUE;
    else if (y2 < y) y_int = FALSE;
      else y_int = TRUE;

    if (x_int && y_int) result = TRUE;
    else result = FALSE;

    return (result);
}


/*
 * drawtile (n) refreshes tile number n.
 *                    
 * Input:	 n - the number of the tile
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
drawtile (n, single)
int n;
int single;
{
    char 		str [5];
    Position 		xp, yp, xoff, yoff;
    Dimension 		width, indent_w, indent_h, adj_height, adj_width;
    XPoint 		TriArray [4];
    XCharStruct		overall;		/* Return extent values */
    unsigned int        direction = NULL;       /* Return direction value */
    unsigned int        ascent = NULL;          /* Return ascent value */
    unsigned int        descent = NULL;         /* Return descent value */

    indent_w = real_tile_width / FACTOR_3D;
    indent_h = real_tile_height / FACTOR_3D;
    if (indent_w < MIN_INDENT) indent_w = MIN_INDENT;
    if (indent_h < MIN_INDENT) indent_h = MIN_INDENT;
   
    adj_height = real_tile_height - indent_h;
    adj_width = real_tile_width - indent_w;

    xoff = tile [n]. x; yoff = tile [n]. y;

    if (single) { lc = 0; rc = 0; }
    if (single)
      XFillRectangle (dpy, win, gcreverse, xoff, yoff,
					    tile_width, tile_height);
    segs [lc]. x1 = xoff; segs [lc]. y1 = yoff + adj_height;
    segs [lc]. x2 = xoff + adj_width; segs [lc]. y2 = yoff + adj_height;
    lc++;

    segs [lc]. x1 = xoff + adj_width; segs [lc]. y1 = yoff;
    segs [lc]. x2 = xoff + adj_width; segs [lc]. y2 = yoff + adj_height;
    lc++;

    segs [lc]. x1 = xoff + adj_width; segs [lc]. y1 = yoff + adj_height;
    segs [lc]. x2 = xoff + real_tile_width;
    segs [lc]. y2 = yoff + real_tile_height;
    lc++;

    TriArray [0]. x = xoff; TriArray [0]. y = yoff + real_tile_height;
    TriArray [1]. x = 0; TriArray [1]. y = -indent_h;
    TriArray [2]. x = indent_w; TriArray [2]. y = indent_h;
    TriArray [3]. x = 0; TriArray [3]. y = 0;
    XFillPolygon (dpy, win, gcdraw, TriArray, 4, Convex, CoordModePrevious);

    TriArray [0]. x = xoff + real_tile_width; TriArray [0]. y = yoff;
    TriArray [1]. x = 0; TriArray [1]. y = indent_h;
    TriArray [2]. x = -indent_w; TriArray [2]. y = -indent_h;
    TriArray [3]. x = indent_w; TriArray [3]. y = 0;
    XFillPolygon (dpy, win, gcdraw, TriArray, 4, Convex, CoordModePrevious);

    rects [rc]. x = xoff; rects [rc]. y = yoff + real_tile_height;
    rects [rc]. width = tile_width;
    rects [rc]. height = tile_height - real_tile_height;
    rc++;

    rects [rc]. x = xoff + real_tile_width; rects [rc]. y = yoff;
    rects [rc]. width = tile_width - real_tile_width;
    rects [rc]. height = tile_height;
    rc++;

    sprintf (str, "%d", n+1);

    XTextExtents (font, str, strlen (str), &direction,
			&ascent, &descent, &overall);

    xp = (real_tile_width - overall.width - indent_w)/2 + tile [n]. x;
    yp = (real_tile_height + lineheight - indent_h)/2 + tile [n]. y;

    XDrawString (dpy, win, gcdraw, xp, yp, str, strlen(str));

    if (single) {
      XFillRectangles (dpy, win, gcline, rects, rc);
      XDrawSegments (dpy, win, gcline, segs, lc);
   }
} /* drawtile (n) */

           
SolvePuzzle ()
{
    MrmType			*dummy_class;
    char			lresult [200];
    char			*xy;
    XmString		res, CS;
    int				char_set, dont_care;
    XmStringContext 	context;
    Boolean			status = TRUE;    

    InfoMessage = FALSE;

    if (Messages_widget == 0) {
      if (MrmFetchWidget (
	puzzle_hierarchy,
	"message",
	workarea,        
	& Messages_widget,
	& dummy_class) != MrmSUCCESS) {
	puzzle_serror  (NoMessageWidget, FATAL);
	}

/*    Remove the Cancel and Help Buttons from the Message Box.  */
      remove_message_box_child (Messages_widget, XmDIALOG_CANCEL_BUTTON);
      remove_message_box_child (Messages_widget, XmDIALOG_HELP_BUTTON);


      XtSetArg (args [0], XmNmessageString, &res);
      XtGetValues (Messages_widget, args, 1);

      
      XmStringInitContext (&context, res);
      while (status) {     
        status = XmStringGetNextSegment (context, &xy, &char_set,
		 &dont_care, &dont_care, &dont_care);
        if (status) {
	    strcat (result, xy);
/*	    strcat (result, "\n");   /* insert NEWLINE string. */
	}
      }
      XmStringFreeContext (context);
    }

    sprintf (lresult, result, moves, clicks);
/* I18N change */
/*  CS = XmStringCreateLtoR (lresult, XmSTRING_DEFAULT_CHARSET); */
    CS = DXmCvtFCtoCS (lresult, &byte_count, &cvt_status);
/* end I18N change */

    XtSetArg (args [0], XmNmessageString, CS);
    XtSetValues (Messages_widget, args, 1);
    XtFree (CS);

    XtManageChild (Messages_widget);
    pop_result = XtMakeGeometryRequest (Messages_widget, & pop_request,
					& pop_reply);
}



static unsigned int exit_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{
/*
   DumpWidgets (w);
*/
#ifdef VMS
   if (hyper_help_context) 
	DXmHelpSystemClose(
      	  hyper_help_context,
      	  puzzle_serror,
      	  "Help Close Error");
#endif
   exit (OK_STATUS);
}


static unsigned int main_help_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{       
    XmString	Frame;

    Frame = (XmString) tag;
    DisplayHelp (Frame);

}


static void on_context_activate_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{       


/* Change to use the DXm Help on context routine. */
    
    DXmHelpOnContext (toplevel, FALSE);

}


static void tracking_help()

{
     Widget     track_widget;
     Cursor     cursor;

     track_widget = NULL;

     cursor = XCreateFontCursor(dpy, XC_question_arrow);

     track_widget = XmTrackingLocate(toplevel, cursor, FALSE);

     if (track_widget != 0)
        {
        if (XtHasCallbacks(track_widget, XmNhelpCallback) == XtCallbackHasSome)
            XtCallCallbacks(track_widget, XmNhelpCallback, NULL);
        }
    else
        {
        XtUngrabPointer(toplevel, CurrentTime);
        }

}


/*---------------------------------------------------*/
/* help(w)                                           */
/*                                                   */
/* this routine will be called from the widget's     */
/* main event handler via the translation manager.   */
/*---------------------------------------------------*/

static void Help (w, ev, params, num_params)
Widget w;
XEvent *ev;
char **params;
int num_params;
{
    XmString	Frame;

    Frame = XmStringCreate("Overview", XmSTRING_DEFAULT_CHARSET);

    DisplayHelp (Frame);
}

void DisplayHelp (Frame)
XmString	Frame;
{
#ifdef VMS
      /* Hyperhelp */
    if (!hyper_help_context) {
      WorkCursorDisplay ();
      DXmHelpSystemOpen (
	&hyper_help_context,            
	toplevel,   			
	PUZZLE_HELP,
	puzzle_serror,
	NoPuzzleHelp);
      WorkCursorRemove ();
    }

      WorkCursorDisplay ();
      DXmHelpSystemDisplay (
	hyper_help_context,            
	PUZZLE_HELP,
	"Topic",
	Frame,
	puzzle_serror,
	NoPuzzleHelp);
      WorkCursorRemove ();
#else
    int			ac;
    Arg			args [2];
    XmString	help_lib_name_cs;
    MrmType		*dummy_class;

/*
    help_lib_name_cs = XmStringCreate("/usr/lib/help/puzzle", XmSTRING_DEFAULT_CHARSET);
*/
    help_lib_name_cs = XmStringCreate("puzzle", XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg (args [ac], DXmNlibrarySpec, help_lib_name_cs); ac++;

    if (help_message_widget == 0) {
      
      WorkCursorDisplay ();
      if (MrmFetchWidgetOverride (                                    
	puzzle_hierarchy,		/* hierarchy id			*/
        "main_help",			/* Index of widget to fetch	*/
	workarea,			/* Parent of widget		*/
	NULL,				/* Override name		*/
	args,				/* Override args		*/
	ac,				/* Override args count		*/
	& help_message_widget,		/* Widget id			*/
	& dummy_class) != MrmSUCCESS)	/* Widget Class			*/
		puzzle_serror  (NoPuzzleHelp, ERROR);
      WorkCursorRemove ();
    }

    if (help_message_widget != 0) {
      ac = 0;
      XtSetArg (args [ac], DXmNfirstTopic, Frame); ac++;
      XtSetValues (help_message_widget, args, ac);
      XtManageChild (help_message_widget);
      pop_result = XtMakeGeometryRequest (help_message_widget, & pop_request,
					& pop_reply);
    }
#endif
}


static unsigned int new_game_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{
  NewGame ();
}

static unsigned int help_done_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{
    XtUnmanageChild (w);
}

static unsigned int message_done_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{
    XtUnmanageChild (w);
    if (!InfoMessage) {
      NewGame ();
      DrawWindow (0, 0, width, height);
    }
}


static unsigned int settings_cancel_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;

{
  XmScaleSetValue (level_scale, GameLevel);

  XtUnmanageChild (settings_widget);
}

static unsigned int settings_ok_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;

{
    int new_num_tiles;

    XmScaleGetValue (level_scale, & new_num_tiles);

    XtUnmanageChild (settings_widget);
    if (reversed != oldorder) NewGame ();

    change_num_tiles (new_num_tiles);

    DrawWindow (0, 0, width, height);
}


static unsigned int save_settings_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String	    	name;
    Widget		parent;
{
    Position	x, y;
    Dimension	width, height;

    width = XtWidth (toplevel);
    height = XtHeight (toplevel);
    XtTranslateCoords (toplevel, 0, 0, &x, &y);
    x = x - XtBorderWidth (toplevel);
    y = y - XtBorderWidth (toplevel);

    save_current_settings (x, y, width, height);
}

static unsigned int save_current_settings (x, y, width, height)
    Position	x, y;
    Dimension	width, height;
{
    XrmValue	put_value;
    char	level_value [10];
    char	geometry_value [20];

    if (user_database == NULL)
      user_database = XrmGetFileDatabase (defaults_name);

    sprintf (geometry_value, "%dx%d+%d+%d", width, height, x, y);
    sprintf (level_value, "%d", GameLevel);

    put_value. addr = level_value;
    put_value. size = strlen (level_value) + 1;

    XrmPutResource (& user_database, xrm_level_name, XtRString, & put_value);

    put_value. addr = geometry_value;
    put_value. size = strlen (geometry_value) + 1;

    XrmPutResource (& user_database, xrm_geometry_name, XtRString, & put_value);

    XrmPutFileDatabase (user_database, defaults_name);
}

static unsigned int restore_settings_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String	    	name;
    Widget		parent;

{
    Arg		args [10];
    int		ac;
    int		status, geometry_mask;
    Position	x, y;
    Dimension	width, height;
    int		intx, inty;
    int		intwidth, intheight;
    char	* get_type;
    XrmValue	get_value;
    int		new_num_tiles;

    if (system_database == NULL)
      system_database = XrmGetFileDatabase (system_defaults_name);

    intx = 100; inty = 150;
    intwidth = 200; intheight = 200;
    new_num_tiles = 4; oldlevel = GameLevel;
    status = XrmGetResource (
	system_database,			/* Database. */
	xrm_geometry_name, 			/* Resource's ASCIZ name. */
	xrm_geometry_class,			/* Resource's ASCIZ class. */
	& get_type,				/* Resource's type (out). */
	& get_value);				/* Address to return value. */
    if (status != NULL) {
      geometry_mask = XParseGeometry (get_value. addr,
	&intx, &inty, &intwidth, &intheight);
    }

    status = XrmGetResource (system_database, xrm_level_name, xrm_level_class,
	& get_type, & get_value);
    if (status != NULL) new_num_tiles = atoi (get_value. addr);

    if (intx < 0)
      intx = XWidthOfScreen (XDefaultScreenOfDisplay (dpy)) + intx - intwidth;
    if (inty < 0)
      inty = XHeightOfScreen (XDefaultScreenOfDisplay (dpy)) + inty - intheight;
    if (intwidth < 100) intwidth = 100;
    if (intheight < 100) intheight = 100;

    x = (Position) intx; y = (Position) inty;
    width = (Dimension) intwidth; height = (Dimension) intheight;

    change_num_tiles (new_num_tiles);

    ac = 0;
    XtSetArg (args [ac], XmNx, x); ac++;
    XtSetArg (args [ac], XmNy, y); ac++;
    XtSetValues (toplevel, args, ac);
    ac = 0;
    XtSetArg (args [ac], XmNwidth, width); ac++;
    XtSetArg (args [ac], XmNheight, height); ac++;
    XtSetValues (main_window, args, ac);

}

static unsigned int settings_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String	    	name;
    Widget		parent;

{
    oldorder = reversed; oldlevel = GameLevel;
    XtManageChild (settings_widget);
    pop_result = XtMakeGeometryRequest (settings_widget, & pop_request,
					& pop_reply);
}

static unsigned int create_level_proc (w, tag, reason, name, parent)
    Widget		*w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;
{
    int new_num_tiles;

    level_scale = (XmScaleWidget) w;
    XmScaleGetValue (level_scale, & new_num_tiles);

    change_num_tiles (new_num_tiles);
}


static void change_num_tiles (new_num_tiles)
    int new_num_tiles;
{
    GameLevel = new_num_tiles;

    if (GameLevel < 3) GameLevel = oldlevel;
    if (GameLevel > MAX_ACROSS) GameLevel = oldlevel;

    if (GameLevel != oldlevel) {
      num_across = GameLevel;
      num_tiles = (num_across * num_across) - 1;
      ResizePuzzle (width, height);
      NewGame ();
    }
    XmScaleSetValue (level_scale, GameLevel);
}

static unsigned int create_work_proc (w, tag, reason, name, parent)
    Widget		w;
    caddr_t		*tag;
    unsigned int	*reason;
    String		name;
    Widget		parent;

{
    MrmType		*dummy_class;
    XtTranslations  	main_translation;

    main_translation = XtParseTranslationTable (main_translation_table);
    workarea = (XmBulletinBoardWidget) w;

    XtSetArg (args [0], XmNtranslations, main_translation);
    XtSetValues (workarea, args, 1);

    if (MrmFetchWidget ( 
	puzzle_hierarchy,
	"settings_dialog_box",
	workarea,        
	& settings_widget,
	& dummy_class) != MrmSUCCESS) {
	puzzle_serror (NoPuzzleSettings, FATAL);
  	}

    reversed = 0;
    XmScaleGetValue (level_scale, & GameLevel);

    if (GameLevel < 3) GameLevel = oldlevel;
    if (GameLevel > MAX_ACROSS) GameLevel = oldlevel;
    XmScaleSetValue (level_scale, GameLevel);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This is the error routine.  Right now, it's a stub that just returns.
**
**  FORMAL PARAMETERS:
**
**	see routine definition
**
**--
**/

static void puzzle_serror (problem_string, level)
char	*problem_string;
int	level;
{
   fprintf (stderr, problem_string);
   if (level == FATAL) exit (ERROR_STATUS);
}
