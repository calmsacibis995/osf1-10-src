/**
*	.TITLE	REGLOB -- Regis Global Storage
*	.IDENT	/1.003/							   2
*/
/* #module <module name> "X0.0" */
/*
 *  Title:	reglob.h
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
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
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Alfred von Campe    03-May-1993     V1.2/BL2
 *      - Use threads on all Alpha platforms now (OpenVMS AXP and DEC OSF/1).
 *
 *  Dave Doucette	07-Apr-1993	V1.2/BL2
 *	- Added ReGIS structure elements to support rubber banding cursor.
 *
 *  Eric Osman		21-Jul-1992	VXT V12
 *	- Add timer slot so we can get rid of timer when regis is reset or
 *	  when DECterm is going away.
 *
 *  Eric Osman		11-June-1992	V3.1/BL6
 *	- Add thread support for regis
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Changed private #include file names to all lower case.
 *      - Removed superfluous typedef.
 *
 *  Bob Messenger	31-Jul-1990	X3.0-6
 *	- Put back #ifdef mips, which was mistakenly removed in X3.0-5.
 *
 *  Bob Messenger       17-Jul-1990     X3.0-5
 *      Merge in Toshi Tanimoto's changes to support Asian terminals -
 *	- definitions of character sets
 *
 *  Bob Messenger	 8-Aug-1989	X2.0-18
 *	- Add spx_inverted, to fix the bug where inverting the shading pattern
 *	  doesn't force the shading pixmap to be redrawn.
 *
 *  Bob Messenger	28-May-1989	X2.0-13
 *	- Add screen_erase_gc
 *	- Add sh_glyph_changed, which tells G65_END_FILLED_FIGURE that
 *	  a loaded glyph has changed, so it has to redraw the shading pixmap
 *	  even if the shading character code hasn't changed.  This is a hack;
 *	  we should really store the entire shading character rather than
 *	  just the character code.
 *
 *  Bob Messenger	11-May-1989	X2.0-10
 *	_ Split up Gidis display size into hard (physical) and soft (logical)
 *	  versions, to allow global symmetry to work without gaps between
 *	  characters.
 *	- Eliminate x_hard_pos, y_hard_pos, foreground_mono and
 *	  background_mono.
 *
 *  Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Removed black_pixel, white_pixel, and default_*ground_* (moved
 *	  to w->common) and cm_data.
 *
 *  Bob Messenger	 1-Apr-1989	X2.0-5
 *	- Don't include both decterm.h and dectermp.h
 *
 *  Bob Messenger	31-Mar-1989	X2.0-5
 *	- Took out foreground_color and background_color as part of shared
 *	  colormap support (they'll be fetched as needed).
*
*  RDM001	27-Aug-1987	R Messenger
* Modified for use with DECterm
*
*  004	26 Jan 1985	/RDM
* Converted to generic form for use with terminal manager
*
*  003   8 May 1984	/RDM
* In order to make Regis reentrant, put all variables previously declared
* in this file into a structure, RS.
*
*  002  17-NOV-1983	/RFD
* Converted the MACRO-11 code (original form) into C code.
*
*  001  16 Sep 1983	/AFV						   2
* Add MG_BEING_DEFINED to fix a problem with macrograph			   3
* definitions being interrupted with <ESC> \ <ESC> P 1 p		   2
*									   2
*  000   1 Jul 1983	/AFV
* Initial Setup
**/

/*
 * Determine what kind of linkage we'll use.  We'd like to use the same on all
 * platforms, but alas, as of 6/92, OSF threads isn't available everywhere
 * yet.
 *
 * Current choices and reasons:
 *
 *	Platform	choice	reason
 *	--------	------	------
 *	Sun		LWP	Lightweight processes package is available but
 *				OSF threads is not yet available.
 *
 *	Alpha/OpenVMS	OSF	OSF threads seems to work fine on Alpha VMS
 *
 *	VAX/VMS		asmbly	OSF threads isn't available until VMS 5.5.
 *				Use assembly language linkage until all
 *				customers are at least at 5.5.
 *
 *	VAX/VXT		asmbly	I doubt anyone has ported threads to VXT
 *				(says Osman on 7-Jun-1993)
 *
 *	MIPS		asmbly	OSF threads seems to be having trouble on
 *				OSF MIPS systems, perhaps because of how
 *				dxterm uses "fork" for user process.  Once
 *				this is worked out, we can use OSF threads,
 *				but for now we use assembly language.
 */

#if defined(sun)
#define USE_LWP yes
#else
#if defined(ALPHA) || defined(__alpha)
#define USE_PTHREADS yes
#else
#define USE_REGIS_ASSEMBLY_CODE yes
#endif
#endif

#include "dectermp.h"
#include "reguseful.h"

#ifdef USE_PTHREADS
#include "pthread.h"
#endif

#ifdef USE_LWP
#include "lwp/lwp.h"
#endif

#define REGSTKSIZ 5000

struct defer_struct
    {
    int op;
    int p1, p2, p3, p4, p5;
    };

#define DEFER_SCROLL_VERTICAL 1
#define DEFER_SCROLL_HORIZONTAL 2
#define DEFER_ERASE 3

#define MAX_DEFERRED_OPERATIONS 100

/**
*  General
**/
struct regis_cntx							/*3*/
{									/*3*/
/**
* context for subregis
**/

#ifndef USE_REGIS_ASSEMBLY_CODE
char		thread_exists_flag;
				/* 1 if thread exists */
#endif

#ifdef USE_PTHREADS
pthread_mutex_t	mutex;		/* lock so only one thread runs at a time */
pthread_cond_t	bell;		/* bell between regis and rest of decterm */
pthread_t	thread;		/* handle on regis's thread */
char		new_data_flag;	/* flag saying there's new data for regis */
#endif

#ifdef USE_LWP
thread_t        thread;        /* handle on regis thread */
thread_t        main_thread;   /* handle on main thread */
#endif

char *bufpos;			/* position in input buffer */
char *taskfp;
char *tasksp;
#ifdef mips
char *taskra;			/* return address */
#endif
void (*regspc)();
char *regssp;
char *regsfp;
char *linkage;			/* This points to place in the regis
				   stack that holds the address of the previous
				   frame, which previous frame is in the
				   non-regis stack.  Code in SUBREGIS uses this
				   to modify the referenced place on the regis
				   stack whenever we are returning to regis.
				   Such modification is necessary, since the
				   place in the non-regis stack might no longer
				   be a stack frame.  Such will be the case
				   if the non-regis code totally returned
				   from routine associated with the referenced
				   frame after the last time we were in regis.

				   Three reasons to do this:

				1)	The VMS condition-handling code
					relies on an intact chain of frames
					in order to find condition handlers,
					such as decterm_error_handler.

				2)	The FAKE_VM tool requires an intact
					chain of frames.

				3)	The VMS debugger requires an intact
					chain of frames.
				*/
#ifdef mips
char *regsra;			/* return address */
#endif
char regstack[REGSTKSIZ];

int	first_process_me;
int	character;
int	gid_xy_valid;
int	gid_x;
int	gid_y;
int	gid_alphabet;
int	err_code;
int	err_char;
int	tmp_write_flag;

/**
* cursor
**/
int	cs_alphabet;
int	cs_index;
int	cs_x_size;
int	cs_y_size;
int	cs_x_rband;
int	cs_y_rband;
Boolean	cs_rband_active;
unsigned int
	cs_width_rband;
unsigned int
	cs_height_rband;
void  (*cursor_motion_handler)();
void  (*cursor_cleanup_handler)();
GC	cs_gc_rband;

/**
*  Modes (not part of writing modes)
**/
int	md_background_color;

/**
*  Writing modes  (those which must be saved and restored)
**/
/* The following line is MACOR-11 code nested in C comment delimeters. */
/* cur.state::	*/
/**
* CAUTION:  it is assumed by the functions contained in file regis4.c that 
*  md_writing_color is the first variable of the current state block.
**/
int	md_writing_color;
int	md_plane_mask;
int	md_mode;
int	md_negative_writing;
int	pt_register;
int	pt_size;
int	pt_multiplier;
int	sh_mode;
int	sh_alphabet;
int	sh_char;
int	sh_width;
int	sh_height;
int	sh_x;
int	sh_y;
int	pix_multiplier_vector;

/* The following line is MACOR-11 code nested in C comment delimeters. */
/* state.size EQL <.-cur.state> / 2 */
/**
* NOTE:  This is also used in file ReGIS4.C in a #define statement as
*	  state_size and therefore must be updated if this is changed.
**/
/* The following line is MACOR-11 code nested in C comment delimeters. */
/*svd.state::	.blkw	state.size */
/**
* CAUTION:  it is assumed by the functions contained in file regis4.c that 
*  s_md_writing_color is the first variable of the saved state block.
**/
int	s_mdwriting_color;
int	s_mdplane_mask;
int	s_mdmode;
int	s_mdnegative_writing;
int	s_ptregister;
int	s_ptsize;
int	s_ptmultiplier;
int	s_shmode;
int	s_shalphabet;
int	s_shchar;
int	s_shwidth;
int	s_shheight;
int	s_shx;
int	s_shy;
int	s_pix_multiplier_vector;

/**
* text state
**/
/* The following line is MACOR-11 code nested in C comment delimeters. */
/* tx.cur.state::	*/
int	tx_alphabet;
int	tx_oblique;
int	tx_direction;
int	tx_x_unit_size;
int	tx_y_unit_size;
int	tx_x_display_size;
int	tx_y_display_size;
int	tx_x_begin;
int	tx_y_begin;
int	tx_x_escapement;
int	tx_y_escapement;

/* The following line is MACOR-11 code nested in C comment delimeters. */
/* tx.state.size EQL <.-tx.cur.state> / 2 */
/**
*  NOTE:  This is also used in file ReGIS2.C in a #define statement as
*	   tx_state_size and therefore must be updated if this is changed.
**/
/* The following line is MACOR-11 code nested in C comment delimeters. */
/* tx.svd.state:: */

int	s_txalphabet;
int	s_txoblique;
int	s_txdirection;
int	s_txx_unit_size;
int	s_txy_unit_size;
int	s_6txx_display_size;
int	s_5txy_display_size;
int	s_4txx_begin;
int	s_3txy_begin;
int	s_2txx_escapement;
int	s_1txy_escapement;

/**
* standard text cell sizes -- must be in the order following (for	   3
* gidis REQUEST_STANDARD_SIZE instruction				   3
**/
int	st_x_unit_size;
int	st_y_unit_size;
int	st_x_display_size;
int	st_y_display_size;

/**
*  Position stack
**/
	/* position stack size */
#define	PS_MAX	16

int	ps_size;
int	ps_count;
int	ps_bits;
int	ps_x[PS_MAX];
int	ps_y[PS_MAX];

/**
*  Screen Addressing
**/
int	sa_2_power;
int	sa_10_power;
int	sa_ulx;
int	sa_uly;
int	sa_x_extent;
int	sa_y_extent;

/**
*  Hard copy
**/
int	hd_x_offset;
int	hd_y_offset;
int	hd_bits;
int	hd_mask;

/**
*  Coordinate passing
**/
int	coord_relflags;
int	rel_x_coord;
int	rel_y_coord;
int	x_crd;
int	y_crd;

/**
*  Macrographs
**/
	/*	 total_number_of_macrographs	*/
#define	MG_NUM		26
	/*	 TOTAL2_MACROGRAPH_MEMORY	*/
#define	MG_SIZE		5000

int	mg_top_one;
int	mg_total_stored_length;
int	mg_count_defined_mgs;
int	mg_being_defined;
int	mg_lengths[MG_NUM];
char		*mg_begin[MG_NUM];
char		mg_memory[MG_NUM + MG_SIZE];

/**
*  Scanner
**/
char		*sc_pointer;
char		*sc_stack[MG_NUM + 1];
char		sc_string[2];
int	sc_source;
int	sc_macrograph[MG_NUM + 1];
int	sc_need_to_flush;
int	sc_current_opcode;/* Current opcode -- if NOT ZERO */
					/*  must do an END_LIST before */
					/*  the FLUSH_BUFFERS and send */
					/*  this word after */
/**
*  Alphabet memory
**/
	/*	 TOTAL3_NUMBER_OF_ALPHABETS	*/
#define	AL_NUM		16
	/*	 total_chars_per_alphabet	*/
#define	AL_SIZE		10

int	ld_alphabet;
int	al_height[AL_NUM];
int	al_extent[AL_NUM];
int	al_width[AL_NUM];
int	al_length[AL_NUM];
char		al_name[AL_NUM * AL_SIZE];

/**
*  Curve state
**/
int	cv_closed;
int	cv_check_pt;
int	cv_in_progress;
int	cv_px[3];
int	cv_py[3];
int	cv_qx[4];
int	cv_qy[4];

/* this should be removed when interface is cleaned up */	/* RDM001 */

#define BUF_GID_SIZE 10
int temp_gid_buffer[BUF_GID_SIZE];

/* Taken from renumber.c */

#define ACCUM_BLOCK_SIZE 4
int actual_accum_block[ACCUM_BLOCK_SIZE];
int *accum_block;

/* State needed by gidisx.c */

Display *display;	/* display context */
DECtermData *widget;	/* widget context pointer */
Window window;		/* window ID */
Drawable bs_pixmap;	/* pixmap for backing store */
int bs_width, bs_height;	/* size of backing store */
GC gc;			/* graphics context */
GC bs_gc;
GC text_gc;		/* graphics context for drawing text with stipple */
GC bs_text_gc;
GC text_clear_gc;	/* graphics context for pre-clearing text in
			   overlay mode on multi-plane systems */
GC bs_text_clear_gc;
GC text_background_gc;	/* graphics context for drawing character backgrounds */
GC bs_text_background_gc;
GC screen_erase_gc;	/* graphics context for erasing the screen */
GC bs_screen_erase_gc;
GC shade_gc;		/* graphics context for filling polygons */
GC bs_shade_gc;

int x_soft_pos, y_soft_pos;
int numpoints;
int window_width, window_height;
int output_ids_width, output_ids_height, x_offset, y_offset;
float x_scale, y_scale;
char dash_list[18];
int dashno, gid_pattern, gid_multiplier;
int writing_mode;
int gid_x_hard_unit_size, gid_y_hard_unit_size;
int gid_x_soft_display_size, gid_y_soft_display_size;
int gid_x_hard_display_size, gid_y_hard_display_size;
int gid_x_soft_escapement, gid_y_soft_escapement;
int gid_x_escapement, gid_y_escapement;
int gid_direction, gid_oblique;

int gid_sh_alphabet, gid_sh_character, gid_sh_pattern, gid_sh_multiplier,
  gid_sh_width, gid_sh_height, sh_glyph_changed;

#define REG_MAX_POINTS 256

XPoint point_list[ REG_MAX_POINTS ];

Drawable load_font_pixmap;
GC load_font_gc;
char *font_glyphs[TOTAL3_NUMBER_OF_ALPHABETS][ 256 ];

int char_width, char_height, char_ascent;

Drawable tpx_pixmap;
GC tpx_gc;
int tpx_width, tpx_height, tpx_x_origin, tpx_y_origin;

Drawable spx_pixmap;
GC spx_gc;
int spx_width, spx_height, spx_alphabet, spx_character;
Boolean spx_inverted;

int plane_mask, dynamic_plane_mask, all_plane_mask, function;

int num_planes;
unsigned long screen_background_mono;
int foreground_index, background_index;

int gc_foreground_color, gc_background_color, gc_function,
  gc_line_style, gc_pattern, gc_multiplier, gc_plane_mask;

int text_gc_foreground_color, text_gc_function, text_gc_plane_mask;
int text_gc_background_color;

int text_background_gc_foreground, text_background_gc_function,
  text_background_gc_plane_mask;

int screen_erase_gc_foreground;

int shade_gc_foreground_color, shade_gc_background_color,
  shade_gc_function, shade_gc_fill_style, shade_gc_plane_mask;

Boolean initialized, input_pending, (*text_button_handler)();
int input_mode;

int defer_count;
struct defer_struct defer_queue[ MAX_DEFERRED_OPERATIONS ];

Boolean color_monitor, paused;
char *paused_data;

XtIntervalId timer;

/* available ReGIS character sets in Kanji ReGIS */
#define REG_ASCII		0
#define REG_LINE_DRAWING	1
#define REG_SUPPLEMENTAL	2
#define REG_JIS_ROMAN		3
#define REG_JIS_KATAKANA	4
#define REG_DEC_KANJI		5

/* working data for Kanji ReGIS emulation */
char kanji_regis;		/* Kanji ReGIS mode enable */
int max1_alph_cell_height;	/* maximum cell height */
int max2_alph_cell_width;	/* maximum cell width */
int g_char_set[2];		/* character sets in use */
XFontStruct *g_text_font[2];	/* fonts in use */
unsigned char char_stack;	/* stack for multi-byte char parsing */
char char_stack_empty;		/* stack empty flag */

};								/*3*/

#define RSTRUCT rstruct

#ifdef VXT_DECTERM
globalref struct regis_cntx *rstruct;
#else
externalref struct regis_cntx *rstruct;
#endif
