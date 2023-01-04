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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_TYPEDEFS.H*/
/* *24    5-SEP-1992 19:34:31 BALLENGER "Add fext_baseline to BKR_TEXT_LINE data structure"*/
/* *23   13-AUG-1992 15:11:26 GOSSELIN "updating with necessary A/OSF changes"*/
/* *22    3-AUG-1992 17:32:12 BALLENGER "Add drawing window pixmaps."*/
/* *21   14-JUL-1992 16:48:45 BALLENGER "UXC$CONVERT"*/
/* *20   14-JUL-1992 16:39:39 BALLENGER "character cell support"*/
/* *19   19-JUN-1992 20:51:19 BALLENGER "Backout previous change to min_space."*/
/* *18   19-JUN-1992 20:23:52 BALLENGER "Make min_space in BKR_FONT_DATA an unsigned int."*/
/* *17   19-MAY-1992 14:10:44 FITZELL "added fields to search context and textlines structs"*/
/* *16   29-MAR-1992 13:46:56 FITZELL "modifying global highlight struct"*/
/* *15   28-MAR-1992 17:24:24 BALLENGER "Add font support for converted postscript"*/
/* *14   19-MAR-1992 11:36:31 PARMENTER "added search and results context"*/
/* *13   19-MAR-1992 11:15:55 FITZELL "add global selection structure"*/
/* *12   14-MAR-1992 14:12:29 BALLENGER "Remove obsolete BKR_LIBRARY_SHELL definitition and update BKR_FONT_DATA"""*/
/* *11    9-MAR-1992 12:37:07 BALLENGER " Add next pointer to BKR_TEXT_ITEM_LIST structure"*/
/* *10    8-MAR-1992 19:14:03 BALLENGER " Add topic data and text line support"*/
/* *9     5-MAR-1992 14:27:02 PARMENTER "adding simple search"*/
/* *8     3-MAR-1992 17:12:38 KARDON "UCXed"*/
/* *7    12-FEB-1992 13:03:48 PARMENTER "ooops,  typo in LAUNCH_TOPIC"*/
/* *6    12-FEB-1992 12:18:31 PARMENTER "added asian support"*/
/* *5    10-JAN-1992 12:31:49 FITZELL "try adding bkr_launch_topic to bkr_window_type again"*/
/* *4    10-JAN-1992 10:46:52 FITZELL "added bke_launch_topic to Bkr_window type"*/
/* *3     1-NOV-1991 12:52:41 BALLENGER "Reintegrate memex support."*/
/* *2    17-SEP-1991 18:17:31 BALLENGER "Correct spelling/capitalization of include file names."*/
/* *1    16-SEP-1991 12:48:48 PARMENTER "type definitions"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BR_TYPEDEFS.H*/
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/

/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr*)
**
**  ABSTRACT:
**
**	Data structures for keeping track of open books and their
**      related windows at the UI level.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     09-Jul-1991
**
**  MODIFICATION HISTORY:
**
**
**--
**/

#ifndef BR_TYPEDEFS_H
#define BR_TYPEDEFS_H

/*
** INCLUDE FILES
**/
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>           /* XmString typedef */
#include "bkr_widget_values.h"

#ifdef MEMEX
#include    <lwk_def.h>
#include    <lwk_dxm_def.h>
#endif



/*
** Macro Definitions
**/

#define	BKR_MAX_LIBRARY_SHELVES     100
#define	BKR_TOPLEVEL_SHELF  	    0
#define BKR_SOURCE_NUM_COMPONENTS   2	/* icon and entry name */


/*
 *  Entry types for entry_type field 
 *
 *  Note: these values must match what is returned by bri_shelf_open.
 *  	    ie,  1 = BOOK; 2 = SHELF
 */

#define BKR_NULL_TYPE	0
#define BKR_BOOK_FILE   1
#define BKR_SHELF_FILE  2

/*
 * Data structures for keeping track of surrogate objects.
 */
#ifdef MEMEX
typedef struct _BMI_SURROGATE_LIST {
    struct _BMI_SURROGATE_LIST *next;
    lwk_surrogate surrogate;
} BMI_SURROGATE_LIST, *BMI_SURROGATE_LIST_PTR;

typedef struct _BMI_NETWORK_LIST {
    struct _BMI_NETWORK_LIST *next;
    lwk_linknet network;
} BMI_NETWORK_LIST, *BMI_NETWORK_LIST_PTR;

typedef struct _BMI_SURROGATE_TREE {
    unsigned         key;
    BMI_SURROGATE_LIST_PTR  list;
    struct _BMI_SURROGATE_TREE *less;
    struct _BMI_SURROGATE_TREE *more;
} BMI_SURROGATE_TREE, *BMI_SURROGATE_TREE_PTR;
#endif 

/*
 *  NODE data structure used to display book/shelf entries in SVN display.
 */
typedef struct _BKR_NODE
    {
    	int                 level;          /* level number of children	    */
	char	    	    *title;	    /* text title of entry	    */
    	struct _BKR_NODE    *sibling;       /* pointer to sibling or NULL   */
    	struct _BKR_NODE    *parent;        /* pointer to parent shelf      */

    	unsigned    	    entry_type;     /* 1 = BOOK; 2 = SHELF	    */
    	unsigned    	    entry_id;	    /* entry id within parent shelf */
    	union
    	{
            struct {
                int	            num_children;   /* number of children     */
                Boolean             opened;         /* children are showing   */
                struct _BKR_NODE    *children;      /* pointer to children    */
                BMD_SHELF_ID        id;             /* id from
                                                       bri_shelf_open */
                char                *path;
#ifdef MEMEX
                BMI_SURROGATE_LIST  *child_surrogates;
#endif 
            } shelf;
            struct {
                struct _BKR_SHELL   *shells;        /* list of SHELLS */
            } book;
    	} u;

#ifdef MEMEX
        BMI_SURROGATE_LIST  *surrogates;
        Boolean             highlight;   /* Tells whether to highlight this entry. */
#endif 

    } BKR_NODE, *BKR_NODE_PTR;

/*
 *  Macros
 */

    /*  
     *  ONLY shelf files are expandable entries.  Shelf entries will have
     *  a NULL children pointer until the shelf file is actually opened.
     *
     *  NOTE: a NULL children pointer doesn't mean the node isn't expandable.
     */

#define NODE_IS_EXPANDABLE( n_id )  	    	    \
    (	    	    	    	    	    	    \
    	( (n_id)->entry_type == BKR_SHELF_FILE )    \
    )



/*
 *  BKR_FONT_DATA -- per font data need by each font referenced in a book
 */

typedef struct _BKR_FONT_DATA 
{
    struct _BKR_FONT_DATA *next;
    int                  ref_count;     /* How many books are using
                                         * this font.
                                         */
    char	    	 *font_name;    /* Complete font name   	*/
    XFontStruct          *font_struct;  /* font information */
    Boolean              font_2byte;    /* specify 2-byte font or not */
    unsigned long	 min_space;     /* Minum spacing between words */

} BKR_FONT_DATA, *BKR_FONT_DATA_PTR;

/*
 *  value of font field, if the font is not able to be loaded
 */
#define	    FONT_NOT_FOUND          ((BKR_FONT_DATA_PTR)0xFFFFFFFF)


/*
 * Window data types
 */

typedef enum _BKR_WINDOW_TYPE {
    BKR_NO_TYPE          = 0,
    BKR_STANDARD_TOPIC   = 1,
    BKR_FORMAL_TOPIC     = 2,
    BKR_SELECTION        = 3,
    BKR_LIBRARY          = 4,
    BKR_LAUNCH_TOPIC	 = 5
} BKR_WINDOW_TYPE;


/*
** Data structures for dealing with text topic data
*/
typedef struct _BKR_TEXT_ITEM_LIST {
    struct _BKR_TEXT_ITEM_LIST *next;
    Boolean is2byte;
    int x;
    int n_items;
    union {
        XTextItem *items;
        XTextItem16 *item16s;
    } u;
    unsigned short *font_nums;
} BKR_TEXT_ITEM_LIST;

typedef struct _BKR_TEXT_LINE {
    struct _BKR_TEXT_LINE *next;
    struct _BKR_TEXT_LINE *prev;
    BMD_CHUNK *parent_chunk;	/* Pointer to the parent chunk for this text 
                                 * line. The chunk and its subchunks contributed 
                                 * the ftext packets that make up this text line.
                                 */
    int ftext_baseline;		/* Original baseline from the ftext fragments. 
                                 * This is used by the postscript printing code.
                                 */
    int baseline;		/* This is the adjusted baseline that is used for
                                 * display and accessing (copy) of text in the topic 
                                 * window.
                                 */
    int x;			/* Coordinates for the bounding box of the line. */
    int y;
    int width;
    int height;

    int n_bytes;		/* Size of the line */
    unsigned char *chars;       /* Characters that make up the line. */
    short *char_widths;         /* Widths of the chracters. */

    int n_item_lists;           	/* Lists of XTextItem's and/or XTextItem16's */
    BKR_TEXT_ITEM_LIST *item_lists;	/* for the line. */

    int exposed;
} BKR_TEXT_LINE, *BKR_TEXT_LINE_PTR;

typedef struct _BKR_TOPIC_DATA {
    struct _BKR_TOPIC_DATA *next;
    BKR_WINDOW_TYPE type;
    int use_count;
    BMD_OBJECT_ID page_id;
    char *title;
    int width;
    int height;
    int num_chunks;
    BMD_CHUNK *chunk_list;
    unsigned char *char_buffer;
    short *widths_buffer;
    BKR_TEXT_ITEM_LIST *text_item_lists;
    XTextItem *items;
    XTextItem16 *item16s;
    unsigned short *font_nums;
    int n_lines;
    BKR_TEXT_LINE *text_lines;
    BMD_CHUNK *graphic_data;
    BMD_CHUNK *hot_spots;
    BMD_CHUNK *extensions;
} BKR_TOPIC_DATA, *BKR_TOPIC_DATA_PTR;

/*
 *  BKR_DIR_ENTRY data structure - contains all the data for displaying a 
 *  single directory entry.
 */

typedef enum _BKR_ENTRY_TYPE
    {
    	    NO_TYPE,
    	    DIRECTORY,
    	    DIRECTORY_ENTRY
    }  BKR_ENTRY_TYPE;

typedef struct _BKR_DIR_ENTRY  {
    struct _BKR_DIR_ENTRY  *sibling;
    struct _BKR_DIR_ENTRY  *children;
    int 	    	   num_children;
    unsigned	    	   level;	 /* level number of entry     */
    char		   *title;
#ifdef MEMEX
    Boolean                highlight;
#endif 
    BKR_ENTRY_TYPE	   entry_type;
    BMD_OBJECT_ID          object_id;  	/* (directory#,entry#) pair   */
    union
    {
        struct {
            int	    	       num_entries;	/* number of entries in directory */
#ifdef MEMEX
            BMI_SURROGATE_TREE *surrogates;
#endif
        } directory;
        struct {
            BMD_OBJECT_ID  target;
        } entry;
    } u;

} BKR_DIR_ENTRY, *BKR_DIR_ENTRY_PTR;


/*
** Data Structures for keeping track of open books.
**/

typedef struct _BKR_BOOK_CTX {

    struct _BKR_BOOK_CTX *next;
    BMD_BOOK_ID          book_id;
    char                 *filename;
    char                 *filespec;
    char                 *title;
    BMD_VERSION          version;
    unsigned             timestamp[2];
    unsigned             n_pages;
    unsigned             n_chunks;
    unsigned             n_directories;
    BKR_DIR_ENTRY_PTR    directories;
    BKR_DIR_ENTRY_PTR    default_directory;
    BKR_DIR_ENTRY_PTR    toc;
    BKR_DIR_ENTRY_PTR    index;
    BMD_OBJECT_ID        first_page;
    unsigned             max_font_id;
    struct _BKR_SHELL    *shells;
    BKR_TOPIC_DATA_PTR   open_topics;
    BKR_FONT_DATA        **font_data;
#ifdef MEMEX
    Boolean              queried_chunks;
    Boolean              queried_directories;
    BMI_SURROGATE_TREE   *chunk_surrogates;
#endif 
} BKR_BOOK_CTX, *BKR_BOOK_CTX_PTR ;

typedef struct _BKR_SHELL {
    int                    n_active_windows;
    BKR_NODE               *library_node;
    BKR_BOOK_CTX           *book;
    struct _BKR_WINDOW     *selection;      
    struct _BKR_WINDOW     *default_topic;
    struct _BKR_WINDOW     *other_topics;
    struct _BKR_SHELL      *all_shells;		/* Links all shells together	 */
    struct _BKR_SHELL      *book_shells;	/* Links all shells for a book	 */
    struct _BKR_SHELL      *library_shells;	/* Links all shells for a library node */
    struct _BKR_SHELL      *client_shells;	/* Links all shells for a client */
    struct _BKR_CLIENT     *client;             /* Head of client list of shells */
} BKR_SHELL, *BKR_SHELL_PTR;

typedef struct _BKR_CLIENT {
    struct _BKR_CLIENT *next;
    Window             id;
    BKR_SHELL          *shells;
} BKR_CLIENT, *BKR_CLIENT_PTR ;

/*
 * BKR_BACK_TOPIC data structure 
 * 
 * all the information needed to re-open a topic in a single topic
 * window.  Used to store one history list entry.
 */

typedef struct _BKR_BACK_TOPIC
    {
    	struct _BKR_BACK_TOPIC	*next;      
    	BMD_OBJECT_ID   	page_id;    /* Topic id to "Go Back" to */
    	int		    	x;	    /* X pos to go back to      */
    	int		    	y;	    /* Y pos to go back to      */

    }	BKR_BACK_TOPIC, *BKR_BACK_TOPIC_PTR;

/*
 *  RESULTS context.  Keeps track of all result oriented data.  Each
 *  search_context allocates an array of these, one for each result.
 */

typedef struct _BKR_SEARCH_RESULTS
    {
	int		result_type;			/* type of result */
	char		*name;				/* name (shelf only) */
	int		file_index;			/* index into fn's */
	int		x, y;				/* x and y for res */
	unsigned	page_id;			/* page id */
	BMD_OBJECT_ID	chunk_id;			/* chunk id */
    } BKR_SEARCH_RESULTS;

typedef struct _BKR_SEARCH_CONTEXT
    {
	int			query_type;		/* type of query */

	int			n_filenames;		/* # of filenames */
	int			n_filenames_allocated;	/* # of fn's allocated*/
	char			**filenames;		/* array of fn's */

	int 			n_results;		/* # of results */
	int 			n_results_allocated;	/* # of r's allocated */
	int			search_string_length;
	BKR_SEARCH_RESULTS	*results;		/* array of results */
			
    } BKR_SEARCH_CONTEXT;



/* 
 * Definitions for handling the display of windows;
 */
typedef struct _BKR_WINDOW {
    /*
     * Generic window information.
     */
    BKR_WINDOW_TYPE    type;
    BKR_SHELL          *shell;
    Widget             appl_shell_id;
    WidgetList         widgets;
    Boolean            active;
    char               *title_bar_name;
    char               *icon_name;
    unsigned int       icon_size;
    Pixmap   	       icon_pixmap;
    Pixmap   	       iconify_pixmap;
    BKR_SEARCH_CONTEXT search;
#ifdef MEMEX
    lwk_dxm_ui         memex_ui;
    void               (*update_highlighting)();
    lwk_integer        highlighting;
    Boolean            using_default_highlighting;
    Widget             connection_menu;
#endif     
    /*
     * Type specific window information.
     */
    union {
        /*
         * Data specific to topic windows.  This is valid only if
         * type is BKR_STANDARD_TOPIC or BKR_FORMAL_TOPIC
         */
        struct {
            struct _BKR_WINDOW  *sibling;
            BMD_OBJECT_ID   page_id;        /* topic page id */
#ifdef USE_TEXT_LINES
            BKR_TOPIC_DATA  *data;
#endif 
            char	    *title;	    /* topic "title" for display */
            int		    x;		    /* of topic display in pixels */
            int		    y;		    /* of topic display in pixels */
            int     	    width;  	    /* of topic display in pixels */
            int     	    height;	    /* of topic display in pixels */
            unsigned	    num_chunks;	    /* number of display chunks */
            BMD_CHUNK	    *chunk_list;    /* address of display chunks */
            BMD_CHUNK	    *selected_chunk;
            BMD_CHUNK	    *outlined_chunk;
            BMD_CHUNK       *btn3down_popup_chunk;
            int		    internal_x;	    /* x for next graphics op */
            int		    internal_y;	    /* y for next graphics op */
            BKR_BACK_TOPIC  *back_topic;    /* History list pointer */
            Time	    mb1_up_time;
            Boolean	    mb1_is_down;
            Boolean	    mb1_double_click;
            Boolean	    grop_pending;   /* graphics operation pending */
            Boolean	    show_hot_spots; /* display hot spot bounding box */
            Boolean	    show_extensions;/* display extensions pattern */

            /* The following two pixmaps are used for turning drawing window
             * highlighting on/off as it gets/loses focus.
             */
            Pixmap          drawing_win_highlight_on;
            Pixmap          drawing_win_highlight_off;
        } topic ;
        /*
         * Data spcific to selection windows.  Valid only if type is
         * BKR_SELECTION.
         */
        struct {
            int 	    	    	num_source_entries;
            BKR_DIR_ENTRY_PTR   	btn3down_entry_node;
            XtPointer		    	*selected_entry_tags;
            int 	    	    	num_selected;
            WidgetList  	    	view_menu_dir_buttons;	/* array of push button widgets */
            int 	    	    	num_view_dir_buttons;
        } selection;
        /*
         * Data specific to library windows
         */
        struct {
            BKR_NODE_PTR	    	root_of_tree;
            XtPointer		    	*selected_entry_tags;
            int 	    	    	num_selected;
            BKR_NODE_PTR	    	btn3down_entry_node;
            BKR_NODE_PTR	    	default_node;
#ifdef MEMEX
            Boolean                     shelves_queried;
            char                        *name;
#endif 
        } library;
    } u;
} BKR_WINDOW, *BKR_WINDOW_PTR;

typedef struct _bkr_global_selection {
    BKR_WINDOW      *window;
    BMD_CHUNK	    *chunk;
    char	    *bkr_copy_buffer;
    int             bkr_copy_buffer_len;
    int             bkr_copy_buffer_size;
    BKR_TEXT_LINE   *startline;
    int             x_start;
    int             start_index;
    int		    start_width;
    BKR_TEXT_LINE   *endline;
    int             x_end;
    int             end_index;
    int             end_width;
    int             y_pos;
    int             y_pos_prev;
    int             direction;
    int		    onoff;
    Atom            atom;
    Boolean         active;
    Boolean	    enabled;
    } BKR_GLOBAL_SELECTION;


/*
 *  ENTRY_IS_EXPANDABLE 
 */

#define ENTRY_IS_EXPANDABLE( en ) \
    	    ( ( (en)->num_children > 0 ) || ( (en)->entry_type == DIRECTORY ) )

#endif 






