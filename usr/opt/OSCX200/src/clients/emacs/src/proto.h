/*
PROTO:
alloc.c buffer.c bytecode.c dired.c emacs.c abbrev.c print.c 
casefiddle.c dispnew.c xdisp.c x11fns.c x11term.c unexmips.c
window.c undo.c indent.c keyboard.c fns.c cmds.c keymap.c 
doc.c   doprnt.c   lastfile.c    syntax.c    editfns.c lread.c      
sysdep.c macros.c  term.c data.c callint.c callproc.c minibuf.c
termcap.c cm.c filemode.c insdel.c scroll.c fileio.c eval.c   
marker.c search.c   process.c filelock.c mocklisp.c 

TODO:
tparam.c       vms-pp.c       environ.c      malloc.c       vmsfns.c
terminfo.c     vmsmap.c	      pre-crt0.c     crt0.c         
hftctl.c       xfns.c         regex.c        xmenu.c        
xterm.c        sunfns.c       alloca.c       unexshm.c      
unexfx2800.c   unexenix.c     unexec.c       unexconvex.c  
unexaix.c      unexelf.c     
*/

#ifdef _NO_EMACS_PROTO
#undef _NO_EMACS_PROTO
#endif

#ifndef _EMACS_CONFIG_H_
#include "config.h"
#endif _EMACS_CONFIG_H_

#ifndef _EMACS_LISP_H_
#include "lisp.h"
#undef NULL
#define NULL 0L
#endif _EMACS_LISP_H_


#ifdef HAVE_TIMEVAL
#ifdef HPUX
#include <time.h>
#else
#include <sys/time.h>
#endif
#endif


#ifdef X11
#include "x11term.h"
#endif X11

#ifndef _EMACS_WINDOW_H_
#include "window.h"
#endif _EMACS_WINDOW_H_
#include <sys/types.h>
#include <sys/stat.h>
#ifdef SYSV_SYSTEM_DIR
#include <dirent.h>

#else

#ifdef NONSYSTEM_DIR_LIBRARY
#include "ndir.h"
#else /* not NONSYSTEM_DIR_LIBRARY */
#include <sys/dir.h>
#endif /* not NONSYSTEM_DIR_LIBRARY */

#endif

#include <stdio.h>
#include <varargs.h>

/* proto for dispnew.c */
extern void remake_screen_structures ();
extern struct matrix *make_screen_structure ( int );
extern void free_screen_structure ( struct matrix * );
extern int line_hash_code (struct matrix *, int );
extern int line_draw_cost ( struct matrix *, int );
extern void cancel_line ( int );
extern void clear_screen_records ();
extern unsigned char *get_display_line ( int, register int );
/* extern int scroll_screen_lines (int, int, int); */
extern int scroll_screen_lines (long, long, long); 
extern void rotate_vector (char *, int, int);
extern void safe_bcopy( char *, char *, int);
extern void preserve_other_columns ( struct window * );
extern void cancel_my_columns (struct window * );
extern int direct_output_for_insert (int);
extern int direct_output_forward_char (int);
extern int update_screen ( int, int );
extern void quit_error_check ();
extern int scrolling ();
extern void update_line (int);
extern int count_blanks ( unsigned char * );
extern int count_match ( unsigned char *,  unsigned char *);
extern Lisp_Object Fopen_termscript ( Lisp_Object );
extern Lisp_Object Fset_screen_height( Lisp_Object, Lisp_Object );
extern Lisp_Object Fscreen_width();
#ifdef SIGWINCH
extern void window_change_signal ();
#endif /* SIGWINCH */
extern void do_pending_window_change ();
extern void change_screen_size (register int, register int, register int, 
			 register int, register int);

extern void change_screen_size_1 ( register int, register int, register int,
			    register int);
extern Lisp_Object Fbaud_rate();
extern Lisp_Object Fding( Lisp_Object );
extern void bell ();
extern Lisp_Object Fsleep_for( Lisp_Object );
extern int timeval_subtract( struct timeval *, struct timeval, 
			     struct timeval );
extern Lisp_Object Fsit_for( Lisp_Object, Lisp_Object );
extern void init_display ();
extern void syms_of_display ();

/* --- end  dispnew.c -------*/

/* --- protos for xdisp.c ----- */

/* note message() needs prototype
extern void message (char *m, long a1, long a2, long a3);  */
extern int display_string ( struct window *, int, unsigned char *,
     int, char, Lisp_Object, Lisp_Object);
extern long Fredraw_display();
extern void display_echo_area_contents ();
extern void redisplay ();
extern void redisplay_preserve_echo_area ();
extern void mark_window_display_accurate ( Lisp_Object, int);
extern void redisplay_all_windows ();
extern void redisplay_windows ( Lisp_Object );
extern int redisplay_window ( Lisp_Object, int );
extern void try_window ( Lisp_Object, int );
extern int try_window_id ( Lisp_Object );
extern struct position * display_text_line ( struct window *, long, long, long, int);
extern void display_mode_line ( struct window *);
extern int display_mode_element ( struct window *, int, int, int, int, int,
     Lisp_Object );
extern char * decode_mode_spec ( struct window *, char, int );
extern void syms_of_xdisp ();
extern void init_xdisp ();

/* end xdisp.c */
/* x11fns.c */

extern void check_xterm ();
extern Lisp_Object Fx_set_bell ( Lisp_Object );
extern Lisp_Object Fx_flip_color ();
extern Lisp_Object Fx_set_foreground_color ( Lisp_Object );

extern Lisp_Object Fx_set_background_color ( Lisp_Object );
extern Lisp_Object Fx_set_border_color ( Lisp_Object );
extern Lisp_Object Fx_set_cursor_color ( Lisp_Object );
extern Lisp_Object Fx_set_mouse_color ( Lisp_Object );
extern int x_set_cursor_colors ();
extern Lisp_Object Fx_color_p ();
extern Lisp_Object Fx_get_foreground_color ();
extern Lisp_Object Fx_get_background_color ();
extern Lisp_Object Fx_get_border_color ();
extern Lisp_Object Fx_get_cursor_color ();
extern Lisp_Object Fx_get_mouse_color ();
extern Lisp_Object Fx_get_default ( Lisp_Object );
extern Lisp_Object Fx_set_font ( Lisp_Object );
extern Lisp_Object Fcoordinates_in_window_p ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fx_mouse_events ();
extern Lisp_Object Fx_proc_mouse_event ();
extern Lisp_Object Fx_get_mouse_event ( Lisp_Object );
extern Lisp_Object Fx_store_cut_buffer ( Lisp_Object );
extern Lisp_Object Fx_get_cut_buffer ();
extern Lisp_Object Fx_set_border_width ( Lisp_Object );
extern Lisp_Object Fx_set_internal_border_width ( Lisp_Object );
extern Lisp_Object Fx_rebind_key ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fx_rebind_keys ( Lisp_Object, Lisp_Object );
extern int XExitWithCoreDump ();
extern Lisp_Object Fx_debug ( Lisp_Object );
extern void XRedrawDisplay ();
extern void XCleanUp ();
extern void syms_of_xfns ();

/* end x11fns.c */

/* start x11term */
#ifdef X11
extern void HLmode ( int ); 
extern void XTreassert_line_highlight ( int, int );
extern void XTchange_line_highlight ( int, int, int );
extern void XTset_terminal_modes ();
extern void XTmove_cursor ( int, int );
extern void cleanup ();
extern void XTclear_end_of_line ( int );
extern void x_clear_end_of_line ( int );
extern void XTreset_terminal_modes ();
extern void XTclear_screen ();
extern void dumpchars ( struct matrix *, int, int, int, int);
extern void updateline ( int );
extern void writechars ( char *,  char * );
static void XToutput_chars ( char *, int );
extern void XTflash ();
extern void XTfeep ();
extern int  CursorToggle ();
static void ClearCursor ();
extern void XTupdate_begin  ();
extern void_end ();
extern void dumprectangle ( int, int, int, int );
extern void XTset_terminal_window ( int );
extern void XTins_del_lines ( int, int );
static void XTcalculate_costs ( int, int *, int * );
static void XTinsert_chars ( char *, int );
static void XTdelete_chars ( int );
extern void stufflines ( int );
extern void scraplines ( int );
extern int XTread_socket ( int, char *, int );
extern char *stringFuncVal ( KeySym );
extern int internal_socket_read ( unsigned char *,  int );
extern int XExitGracefully ();
extern int XIgnoreError ();
extern void xfixscreen();
static int XT_GetDefaults ( char * );
extern int x_error_handler (  Display *, XErrorEvent * );
extern int x_io_error_handler ( Display * );
extern void x_term_init ();
static void x_init_1 ();
extern void XSetFlash ();
extern void XSetFeep ();
static XFontStruct *XT_CalcForFont ( char * );
extern int XNewFont ( char * );
extern void XFlipColor ();
static int XT_Set_Class_Hints( Window );
static int XT_Set_Command_Line ( Window );
static int XT_Set_Host ( Window );
static int XT_Set_Title ( Window );
static int XT_Set_Icon_Title ( Window );
static int XT_Set_Size_Hints ( Window, int, int, int, int, Bool, int );
static int XT_Set_Zoom_Sizes( Window );
static int XT_Set_WM_Hints ( Window );
extern void XSetWindowSize ( int, int );
static int XInitWindow ();

/* end  x11term */
#endif X11
/* start xterm.c */
#ifdef HAVE_X_WINDOWS
#ifndef X11
static HLmode ( int );
extern void XTreassert_line_highlight ( int, int );
static XTchange_line_highlight ( int, int, int );
static void XTset_terminal_modes ();
static void XTmove_cursor ( int, int );
static void cleanup ();
static void XTclear_end_of_line ( int );
static void x_clear_end_of_line ( int );
static void XTreset_terminal_modes ();
static void XTclear_screen ();
static void dumpchars ( struct matrix *, int, int, int, int  );
static void writechars ( char *, char * );
static void XToutput_chars (start, len);
static void XToutput_chars ( char *, int );
static void XTflash ();
static void flashback ();
static void XTfeep ();
extern void CursorToggle ();
static void ClearCursor ();
static void XTupdate_begin ();
static void XTupdate_end ();
extern void dumprectangle ( int, int, int, int );
static void XTset_terminal_window ( int );
extern void XTins_del_lines ( int vpos, int n);
static void XTinsert_chars ( char *, int );
static void XTdelete_chars ( int );
static void stufflines ( int );
static void scraplines ( int );
extern void XTread_socket ( int, char *, int );
extern void refreshicon ();
extern void XBitmapIcon (); 
extern void XTextIcon ();
extern void XExitGracefully ( Display *, XErrorEvent * );
extern int x_io_error ( Display * );
extern void xfixscreen ();
extern void x_term_init ();
extern void x_init_1 ( int );
static void dumpqueue ();
extern void XSetFlash ();
extern void XSetFeep ();
extern void XNewFont ( char * );
extern void XFlipColor ();
extern void XSetOffset ( int, int );
extern void XSetWindowSize ( int, int );
extern void XPopUpWindow ();
extern void spacecheck ( int, int, int, int );    
extern void loadxrepbuffer ( XEvent *, XREPBUFFER * );
extern void unloadxrepbuffer ( XEvent *, XREPBUFFER * ); 
extern void fixxrepbuffer (); 
/* end xterm.c */
#endif /* not X11 */
#endif /* HAVE_X_WINDOWS */

/* unexmips.c */
static int fatal_unexec(const char *,  ...);
void unexec (char *, char *, unsigned long, 
		unsigned long, unsigned long);
static void mark_x ( char * );

/* end unexmips.c */
/* window.c */

extern Lisp_Object Fwindowp ( Lisp_Object obj );
static Lisp_Object make_window ();
extern Lisp_Object Fselected_window ();
extern Lisp_Object Fminibuffer_window (); 
extern Lisp_Object Fpos_visible_in_window_p ( Lisp_Object, Lisp_Object );
static struct window * decode_window ( Lisp_Object );
extern Lisp_Object Fwindow_buffer ( Lisp_Object );
extern Lisp_Object Fwindow_height ( Lisp_Object );
extern Lisp_Object Fwindow_width ( Lisp_Object );
extern Lisp_Object Fwindow_hscroll ( Lisp_Object );
extern Lisp_Object Fset_window_hscroll ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fwindow_edges ( Lisp_Object );
extern Lisp_Object Fwindow_point ( Lisp_Object );
extern Lisp_Object Fwindow_start ( Lisp_Object );
extern Lisp_Object Fset_window_point ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fset_window_start ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fdelete_window ( Lisp_Object );
static void replace_window ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fnext_window ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprevious_window ( Lisp_Object );
extern Lisp_Object Fother_window ( Lisp_Object );
static Lisp_Object window_loop ( int, Lisp_Object );
extern Lisp_Object Fget_lru_window ();
extern Lisp_Object Fget_largest_window ();
extern Lisp_Object Fget_buffer_window ( Lisp_Object );
extern Lisp_Object Fdelete_other_windows ( Lisp_Object );
extern Lisp_Object Fdelete_windows_on ( Lisp_Object );
extern Lisp_Object Freplace_buffer_in_windows ( Lisp_Object );
extern void set_window_height ( Lisp_Object, int, int );
extern void set_window_width ( Lisp_Object, int, int );
extern Lisp_Object Fset_window_buffer ( Lisp_Object, Lisp_Object );
static void unshow_buffer ( struct window *);
extern Lisp_Object Fselect_window ( Lisp_Object );
extern Lisp_Object Fdisplay_buffer ( Lisp_Object, Lisp_Object );
extern void temp_output_buffer_show ( Lisp_Object );
extern void make_dummy_parent ( Lisp_Object );
extern Lisp_Object Fsplit_window ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fenlarge_window ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fshrink_window ( Lisp_Object, Lisp_Object );
extern int window_height ( Lisp_Object );
extern int window_width ( Lisp_Object ); 
extern void change_window_height ( int, int );
static Lisp_Object window_scroll ( Lisp_Object, int, int );
extern void scroll_command ( Lisp_Object, int );
extern Lisp_Object Fscroll_up ( Lisp_Object );
extern Lisp_Object Fscroll_down ( Lisp_Object );
extern Lisp_Object Fscroll_left ( Lisp_Object );
extern Lisp_Object Fscroll_right ( Lisp_Object );
extern Lisp_Object Fscroll_other_window ( Lisp_Object );
extern Lisp_Object Frecenter ( Lisp_Object );
extern Lisp_Object Fmove_to_window_line ( Lisp_Object ); 
extern Lisp_Object Fset_window_configuration ( Lisp_Object );
static int count_windows ( struct window * );
extern Lisp_Object Fcurrent_window_configuration ();
static int save_window_save ( Lisp_Object, struct Lisp_Vector *, int, int );
extern Lisp_Object Fsave_window_excursion ( Lisp_Object );
extern void init_window_once ();
extern void syms_of_window ();
extern void keys_of_window ();

/* end window.c */
/* undo.c */

extern void record_insert ( Lisp_Object, Lisp_Object );
extern void record_delete ( int, int );
extern void record_change ( int, int );
extern void record_first_change ();
extern Lisp_Object Fundo_boundary ();
extern Lisp_Object truncate_undo_list ( Lisp_Object, int, int );
extern Lisp_Object Fprimitive_undo ( Lisp_Object, Lisp_Object );
extern void syms_of_undo ();

/* indent.c */
extern Lisp_Object Fcurrent_column ();
extern void invalidate_current_column ();
extern int current_column ();
extern void ToCol ( int );
extern Lisp_Object Findent_to  ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fcurrent_indentation ();
extern int position_indentation ( int );
extern Lisp_Object Fmove_to_column ( Lisp_Object );
extern struct position *compute_motion ( Lisp_Object, int, int,
     Lisp_Object, Lisp_Object, Lisp_Object, Lisp_Object, Lisp_Object, int );
extern int pos_tab_offset ( struct window *, int );
extern struct position *vmotion ( int, int, int, int, Lisp_Object );
extern Lisp_Object Fvertical_motion  ( Lisp_Object );
extern void syms_of_indent ();
/* end of indent.c */

/* buffer.c */
extern void nsberror( Lisp_Object );
extern Lisp_Object Fbuffer_list ();
extern Lisp_Object Fget_buffer( Lisp_Object );
extern Lisp_Object Fget_file_buffer( Lisp_Object );
extern Lisp_Object Fget_buffer_create( Lisp_Object );
extern void reset_buffer( struct buffer *);
extern void reset_buffer_local_variables( struct buffer *);
extern Lisp_Object Fgenerate_new_buffer( Lisp_Object );
extern Lisp_Object Fbuffer_name( Lisp_Object );
extern Lisp_Object Fbuffer_file_name( Lisp_Object );
extern Lisp_Object Fbuffer_local_variables( Lisp_Object );
extern Lisp_Object Fbuffer_modified_p ( Lisp_Object );
extern Lisp_Object Fset_buffer_modified_p ( Lisp_Object );
extern Lisp_Object Frename_buffer ( Lisp_Object );
extern Lisp_Object Fother_buffer ( Lisp_Object );
extern Lisp_Object Fbuffer_flush_undo ( Lisp_Object );
extern Lisp_Object Fbuffer_enable_undo ( Lisp_Object );
extern Lisp_Object Fkill_buffer ( Lisp_Object );
extern void record_buffer ( Lisp_Object );
extern Lisp_Object Fswitch_to_buffer ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fpop_to_buffer ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fcurrent_buffer ();
extern Lisp_Object Fset_buffer ( Lisp_Object );
extern Lisp_Object Fbarf_if_buffer_read_only ();
extern Lisp_Object Fbury_buffer ( Lisp_Object );
extern void set_buffer_internal ( register struct buffer *b );
extern Lisp_Object Ferase_buffer ();
extern void validate_region ( Lisp_Object *, Lisp_Object * );
extern Lisp_Object list_buffers_1 ( Lisp_Object );
extern Lisp_Object Flist_buffers ( Lisp_Object );
extern Lisp_Object Fkill_all_local_variables ();
extern void init_buffer_once ();
extern void init_buffer ();
extern void syms_of_buffer ();
extern void keys_of_buffer ();

/* end of buffer.c */
/* alloc.c */
extern Lisp_Object malloc_warning_1 ( Lisp_Object );
extern void malloc_warning ( char * );
extern void display_malloc_warning ();
extern void memory_full ();
extern void *xmalloc ( size_t );
extern void *xrealloc ( void *, size_t );
extern void init_cons ();
extern void free_cons ( struct Lisp_Cons * );
extern Lisp_Object Fcons ( Lisp_Object, Lisp_Object );
extern Lisp_Object Flist ( int, Lisp_Object *);
extern Lisp_Object Fmake_list ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fmake_vector ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fvector( int, Lisp_Object *args);
extern void init_symbol ();
extern Lisp_Object Fmake_symbol ( Lisp_Object );
extern void init_marker ();
extern Lisp_Object Fmake_marker ();
extern void init_strings ();
extern Lisp_Object Fmake_string ( Lisp_Object, Lisp_Object );
extern Lisp_Object make_string ( char *, int );
extern Lisp_Object build_string ( char *str );
static Lisp_Object make_uninit_string ( int );
extern Lisp_Object make_pure_string ( char *, long );
extern Lisp_Object pure_cons ( Lisp_Object, Lisp_Object );
extern Lisp_Object make_pure_vector ( int );
extern Lisp_Object Fpurecopy ( Lisp_Object );
extern void staticpro ( Lisp_Object * );
extern Lisp_Object Fgarbage_collect ();
static void mark_object ( Lisp_Object * );
static void mark_buffer ( Lisp_Object );
static void gc_sweep ();
static void compact_strings ();
extern void truncate_all_undos ();
extern void init_alloc_once ();
extern void init_alloc ();
extern void syms_of_alloc ();
/* end of alloc.c */

/* start keyobard.c */

extern void echo_prompt ( char * );
extern void echo_char ( int );
extern void echo_dash ();
extern void echo ();
extern void cancel_echoing ();
extern void record_auto_save ();
extern Lisp_Object Frecursive_edit ();
extern Lisp_Object recursive_edit_1 ();
extern Lisp_Object recursive_edit_unwind ( Lisp_Object );
extern Lisp_Object cmd_error ( Lisp_Object );
extern Lisp_Object command_loop ();
extern Lisp_Object command_loop_2 ();
extern Lisp_Object top_level_2 ();
extern Lisp_Object top_level_1 ();
extern Lisp_Object Ftop_level ();
extern Lisp_Object Fexit_recursive_edit ();
extern Lisp_Object Fabort_recursive_edit ();
extern Lisp_Object command_loop_1 ();
extern void request_echo ();
extern void start_polling ();
extern void stop_polling ();
extern int read_command_char ( int );
extern Lisp_Object print_help ( Lisp_Object );
extern void kbd_buffer_store_char ( int );
extern int kbd_buffer_read_command_char ();
extern void force_input_read ();
static void get_input_pending ( int * );
extern void consume_available_input ();
static void read_avail_input ( int );
extern void gobble_input ();
extern void input_available_signal ( int );
extern int fast_read_one_key ( char * );
extern int read_key_sequence ( char *, int, unsigned char *, int );
extern Lisp_Object Fread_key_sequence ( Lisp_Object );
extern Lisp_Object Fcommand_execute ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fexecute_extended_command ( Lisp_Object );
extern int detect_input_pending ();
extern Lisp_Object Finput_pending_p ();
extern Lisp_Object Frecent_keys ();
extern Lisp_Object Fthis_command_keys ();
extern Lisp_Object Frecursion_depth ();
extern Lisp_Object Fopen_dribble_file ( Lisp_Object );
extern Lisp_Object Fdiscard_input ();
extern Lisp_Object Fsuspend_emacs ( Lisp_Object ); 
extern void stuff_buffered_input ( Lisp_Object );
extern void set_waiting_for_input ( long *word_to_clear );
extern void clear_waiting_for_input ();
extern void interrupt_signal ();
extern void quit_throw_to_read_command_char ();
extern Lisp_Object Fset_input_mode ( Lisp_Object, Lisp_Object, Lisp_Object );
extern void init_keyboard ();
extern void syms_of_keyboard ();
extern void keys_of_keyboard ();

/* end of keyboard.c */
/* start of fns.c */

extern Lisp_Object Fidentity ( Lisp_Object );
extern Lisp_Object Frandom ( Lisp_Object );
extern Lisp_Object Flength ( Lisp_Object );
extern Lisp_Object Fstring_equal ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fstring_lessp ( Lisp_Object, Lisp_Object );
extern Lisp_Object concat2 ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fappend ( int, Lisp_Object * );
extern Lisp_Object Fconcat ( int, Lisp_Object * );
extern Lisp_Object Fvconcat ( int,  Lisp_Object * );
extern Lisp_Object Fcopy_sequence ( Lisp_Object ); 
static Lisp_Object concat ( int, Lisp_Object *, enum Lisp_Type, int );
extern Lisp_Object Fcopy_alist ( Lisp_Object );
extern Lisp_Object Fsubstring ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fnthcdr ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fnth ( Lisp_Object, Lisp_Object );
extern Lisp_Object Felt ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fmemq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fassq ( Lisp_Object, Lisp_Object );
extern Lisp_Object assq_no_quit ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fassoc ( Lisp_Object, Lisp_Object );
extern Lisp_Object Frassq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fdelq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fnreverse ( Lisp_Object ); 
extern Lisp_Object Freverse ( Lisp_Object );
extern Lisp_Object Fsort ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object merge ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fget ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fput ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fequal ( Lisp_Object, Lisp_Object );
extern Lisp_Object Ffillarray ( Lisp_Object, Lisp_Object );
extern Lisp_Object nconc2 ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fnconc (int nargs, Lisp_Object *args);
static void mapcar1 ( int, Lisp_Object *, Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fmapconcat ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fmapcar ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fy_or_n_p ( Lisp_Object );
extern Lisp_Object Fyes_or_no_p ( Lisp_Object );
extern Lisp_Object Fload_average ();
extern Lisp_Object Ffeaturep ( Lisp_Object );
extern Lisp_Object Fprovide ( Lisp_Object );
extern Lisp_Object Frequire ( Lisp_Object,  Lisp_Object );
extern void syms_of_fns(void);
/* end fns.c */

/* start of cmds.c */

extern Lisp_Object Fforward_char ( Lisp_Object );
extern Lisp_Object Fbackward_char ( Lisp_Object );
extern Lisp_Object Fforward_line ( Lisp_Object );
extern Lisp_Object Fbeginning_of_line ( Lisp_Object );
extern Lisp_Object Fend_of_line ( Lisp_Object );
extern Lisp_Object Fdelete_char ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fdelete_backward_char ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fself_insert_command ( Lisp_Object );
extern Lisp_Object Fnewline ( Lisp_Object );
extern int self_insert_internal ( char, int );
extern void syms_of_cmds ();
extern void keys_of_cmds ();

/* end of cmds.c */
/* start of keymap.c */

extern Lisp_Object Fmake_keymap ();
extern Lisp_Object Fmake_sparse_keymap ();
extern void ndefkey ( Lisp_Object, int, char * );
extern Lisp_Object Fkeymapp ( Lisp_Object ); 
extern Lisp_Object get_keymap_1 ( Lisp_Object, int );
extern Lisp_Object get_keymap ( Lisp_Object );
extern Lisp_Object get_keyelt ( Lisp_Object );
extern Lisp_Object access_keymap ( Lisp_Object, int ); 
extern Lisp_Object store_in_keymap ( Lisp_Object, int, Lisp_Object ); 
extern Lisp_Object Fcopy_keymap ( Lisp_Object );
extern Lisp_Object Fdefine_key ( Lisp_Object, Lisp_Object, Lisp_Object ); 
extern Lisp_Object Flookup_key ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fkey_binding ( Lisp_Object );
extern Lisp_Object Flocal_key_binding ( Lisp_Object );
extern Lisp_Object Fglobal_key_binding ( Lisp_Object );
extern Lisp_Object Fglobal_set_key ( Lisp_Object, Lisp_Object );
extern Lisp_Object Flocal_set_key ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fglobal_unset_key ( Lisp_Object );
extern Lisp_Object Flocal_unset_key ( Lisp_Object );
extern Lisp_Object Fdefine_prefix_command ( Lisp_Object );
extern Lisp_Object Fuse_global_map ( Lisp_Object );
extern Lisp_Object Fuse_local_map ( Lisp_Object );
extern Lisp_Object Fcurrent_local_map ();
extern Lisp_Object Fcurrent_global_map ();
extern Lisp_Object Faccessible_keymaps ( Lisp_Object );
extern Lisp_Object Fkey_description ( Lisp_Object );
extern char *push_key_description ( unsigned int,  char *);
extern Lisp_Object Fsingle_key_description ( Lisp_Object );
extern char *push_text_char_description( unsigned int,  char *);
extern Lisp_Object Ftext_char_description ( Lisp_Object );
extern Lisp_Object Fwhere_is_internal ( Lisp_Object, Lisp_Object, Lisp_Object);
extern Lisp_Object Fwhere_is ( Lisp_Object );
extern Lisp_Object Fdescribe_bindings ();
extern Lisp_Object describe_buffer_bindings ( Lisp_Object );
extern void describe_map_tree (Lisp_Object, int, Lisp_Object );
extern void describe_command ( Lisp_Object );
extern void describe_map ( Lisp_Object, Lisp_Object, int, Lisp_Object );
extern void describe_alist ( Lisp_Object, Lisp_Object, void (*)(), int, 
			    Lisp_Object );
extern void describe_vector(Lisp_Object, Lisp_Object, void (*)(), int, 
			    Lisp_Object );
static void apropos_accum ( Lisp_Object, Lisp_Object );
static Lisp_Object apropos1 ( Lisp_Object );
static void insert_first_line ( char *, Lisp_Object );
extern Lisp_Object Fapropos ( Lisp_Object, Lisp_Object, Lisp_Object );
extern void syms_of_keymap ();
extern void keys_of_keymap ();
/* end keymap.c */

/* print.c */
static void printchar ( unsigned char, Lisp_Object );
static void strout ( char *, int, Lisp_Object );
extern Lisp_Object Fwrite_char ( Lisp_Object, Lisp_Object );
extern void write_string ( char *, int );
extern void write_string_1 ( char *, int, Lisp_Object );
extern void temp_output_buffer_setup ( char * );
extern Lisp_Object internal_with_output_to_temp_buffer (char *, 
			Lisp_Object (*) (), Lisp_Object);
extern Lisp_Object Fwith_output_to_temp_buffer ( Lisp_Object args );
extern Lisp_Object Fterpri ( Lisp_Object  printcharfun );
extern Lisp_Object Fprin1 ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprin1_to_string ( Lisp_Object obj);
extern Lisp_Object Fprinc ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprint ( Lisp_Object, Lisp_Object );
static void print ( Lisp_Object, Lisp_Object, int );
extern void syms_of_print ();

/* end of print.c */
/* casefiddle.c */
extern enum case_action flag;
extern Lisp_Object casify_object ( enum case_action, Lisp_Object );
extern void casify_region(enum case_action, Lisp_Object, Lisp_Object );
extern Lisp_Object Fupcase ( Lisp_Object );
extern Lisp_Object Fdowncase ( Lisp_Object );
extern Lisp_Object Fcapitalize ( Lisp_Object );
extern Lisp_Object Fupcase_region ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fdowncase_region ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fcapitalize_region ( Lisp_Object, Lisp_Object );
extern Lisp_Object upcase_initials_region ( Lisp_Object, Lisp_Object );
extern Lisp_Object operate_on_word ( Lisp_Object );
extern Lisp_Object Fupcase_word ( Lisp_Object );
extern Lisp_Object Fdowncase_word ( Lisp_Object );
extern Lisp_Object  Fcapitalize_word ( Lisp_Object );
extern void syms_of_casefiddle ();
extern void keys_of_casefiddle ();

/* end casefiddle */
/* abbrev.c */

extern Lisp_Object Fmake_abbrev_table ();
extern Lisp_Object Fclear_abbrev_table ( Lisp_Object );
extern Lisp_Object Fdefine_abbrev ( Lisp_Object, Lisp_Object, Lisp_Object,
				   Lisp_Object, Lisp_Object );
extern Lisp_Object Fdefine_global_abbrev ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fdefine_mode_abbrev ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fabbrev_symbol ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fabbrev_expansion ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fexpand_abbrev ();
extern Lisp_Object Funexpand_abbrev ();
static void write_abbrev ( Lisp_Object, Lisp_Object );
static void describe_abbrev ( Lisp_Object, Lisp_Object );
extern Lisp_Object Finsert_abbrev_table_description ( Lisp_Object, 
						     Lisp_Object );
extern Lisp_Object Fdefine_abbrev_table ( Lisp_Object tabname, 
					 Lisp_Object defns);
extern void syms_of_abbrev ();

/* end of abbrev.c */
/* bytecode */

extern Lisp_Object Fbyte_code ( Lisp_Object, Lisp_Object, Lisp_Object );
extern void syms_of_bytecode ();

/* end bytecode */
/* dired.c */
extern Lisp_Object Fdirectory_files ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Ffile_name_completion ( Lisp_Object, Lisp_Object );
extern Lisp_Object Ffile_name_all_completions ( Lisp_Object, Lisp_Object );
extern Lisp_Object file_name_completion ( Lisp_Object, Lisp_Object, int, int);
extern int file_name_completion_stat ( Lisp_Object, struct direct *, 
				      struct stat * );
extern Lisp_Object make_time ( Lisp_Object );
extern Lisp_Object Ffile_attributes ( Lisp_Object );
extern void syms_of_dired ();

/* end dired.c */

/* start emacs.c */
extern void fatal_error_signal(int sig);
static init_cmdargs ( int, char **, int );
extern int main ( int, char **, char ** );
extern Lisp_Object Fkill_emacs ( Lisp_Object );
#ifdef HAVE_SHM
extern Lisp_Object Fdump_emacs_data ( Lisp_Object );
#else
extern Lisp_Object Fdump_emacs ( Lisp_Object, Lisp_Object );
#endif /* HAVE_SHM */
extern Lisp_Object decode_env_path ( char *, char * );
extern void syms_of_emacs ();

/* end emacs.c */

/* start of doc.c */
extern Lisp_Object get_doc_string ( off_t );
extern Lisp_Object Fdocumentation ( Lisp_Object );
extern Lisp_Object Fdocumentation_property ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fsnarf_documentation ( Lisp_Object );
extern Lisp_Object Fsubstitute_command_keys ( Lisp_Object );
extern void syms_of_doc ();
/* end of doc.c */
/* doprnt.c */
extern int doprnt(char *, int, char *, int, char **);
/* end doprnt.c */
/* syntax.c */
extern Lisp_Object Fsyntax_table_p ( Lisp_Object );
extern Lisp_Object check_syntax_table ( Lisp_Object );
extern Lisp_Object Fsyntax_table ();
extern Lisp_Object Fstandard_syntax_table ();
extern Lisp_Object Fcopy_syntax_table ( Lisp_Object );
extern Lisp_Object Fset_syntax_table ( Lisp_Object );
extern Lisp_Object Fchar_syntax ( Lisp_Object );
extern Lisp_Object Fmodify_syntax_entry(Lisp_Object, Lisp_Object, Lisp_Object);
extern void describe_syntax ( Lisp_Object );
extern Lisp_Object describe_syntax_1 ( Lisp_Object );
extern Lisp_Object Fdescribe_syntax ();
extern int scan_words (int, int );
extern Lisp_Object Fforward_word ( Lisp_Object );
extern Lisp_Object scan_lists(int, int, int, int );
extern int char_quoted ( int );
extern Lisp_Object Fscan_lists ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fscan_sexps ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fbackward_prefix_chars ();
extern struct lisp_parse_state *scan_sexps_forward(int, int, int, 
						   int, Lisp_Object );
extern Lisp_Object Fparse_partial_sexp ( Lisp_Object, Lisp_Object, Lisp_Object,
					Lisp_Object, Lisp_Object );
extern void init_syntax_once(void);
extern void syms_of_syntax(void);

/* end of syntax.c */
/* editfns.c */
extern void init_editfns ();
extern Lisp_Object Fchar_to_string ( Lisp_Object );
extern Lisp_Object Fstring_to_char ( Lisp_Object );
static Lisp_Object buildmark ( int );
extern Lisp_Object Fpoint ();
extern Lisp_Object Fpoint_marker ();
extern int clip_to_bounds(int lower, int num, int upper);
extern Lisp_Object Fgoto_char ( Lisp_Object );
static Lisp_Object region_limit ( int );
extern Lisp_Object Fregion_beginning ();
extern Lisp_Object Fregion_end ();
extern Lisp_Object Fmark_marker ();
extern Lisp_Object save_excursion_save ();
extern Lisp_Object save_excursion_restore ( Lisp_Object );
extern Lisp_Object Fsave_excursion ( Lisp_Object );
extern Lisp_Object Fbufsize ();
extern Lisp_Object Fpoint_min ();
extern Lisp_Object Fpoint_min_marker ();
extern Lisp_Object Fpoint_max ();
extern Lisp_Object Fpoint_max_marker ();
extern Lisp_Object Ffollchar ();
extern Lisp_Object Fprevchar ();
extern Lisp_Object Fbobp ();
extern Lisp_Object Feobp ();
extern Lisp_Object Fbolp ();
extern Lisp_Object Feolp ();
extern Lisp_Object Fchar_after ( Lisp_Object );
extern Lisp_Object Fuser_login_name ();
extern Lisp_Object Fuser_real_login_name ();
extern Lisp_Object Fuser_uid ();
extern Lisp_Object Fuser_real_uid ();
extern Lisp_Object Fuser_full_name ();
extern Lisp_Object Fsystem_name ();
extern Lisp_Object Fcurrent_time_string ();
extern void insert1 ( Lisp_Object );
extern Lisp_Object Finsert( int, Lisp_Object * );
extern Lisp_Object Finsert_before_markers ( int, Lisp_Object *);
extern Lisp_Object Finsert_char ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fbuffer_substring ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fbuffer_string ();
extern Lisp_Object Finsert_buffer_substring ( Lisp_Object, Lisp_Object, 
					     Lisp_Object );
extern Lisp_Object Fsubst_char_in_region ( Lisp_Object, Lisp_Object, 
				      Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fdelete_region ( Lisp_Object, Lisp_Object);
extern Lisp_Object Fwiden ();
extern Lisp_Object Fnarrow_to_region ( Lisp_Object, Lisp_Object);
extern Lisp_Object save_restriction_save ();
extern Lisp_Object save_restriction_restore ( Lisp_Object );
extern Lisp_Object Fsave_restriction ( Lisp_Object );
extern Lisp_Object Fmessage ( int, Lisp_Object * );
extern Lisp_Object Fformat ( int, Lisp_Object * );
/* not sure yet 
extern int format1(char *string1, int arg0, int arg1, int arg2, int arg3, int arg4); */
extern Lisp_Object Fchar_equal ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fgetenv ( Lisp_Object );
extern void syms_of_editfns ();

/* end of editfns.c */
/* lread.c */
static int readchar (Lisp_Object readcharfun);
extern Lisp_Object Fread_char ();
extern Lisp_Object Fget_file_char ();
extern Lisp_Object Fload ( Lisp_Object, Lisp_Object, Lisp_Object, Lisp_Object);
static Lisp_Object load_unwind ( Lisp_Object );
static int absolute_filename_p ( Lisp_Object );
extern int openp(Lisp_Object, Lisp_Object, char *, Lisp_Object  *, int );
extern Lisp_Object unreadpure ();
extern Lisp_Object Feval_current_buffer ( Lisp_Object );
extern Lisp_Object Feval_region ( Lisp_Object, Lisp_Object, Lisp_Object);
extern Lisp_Object Fread (Lisp_Object);
extern Lisp_Object Fread_from_string ( Lisp_Object, Lisp_Object, Lisp_Object);
static Lisp_Object read0 ( Lisp_Object );
static Lisp_Object read1 ( Lisp_Object );
static Lisp_Object read_vector ( Lisp_Object );
static Lisp_Object read_list ( int,  Lisp_Object );
static int read_escape ( Lisp_Object );
extern Lisp_Object check_obarray ( Lisp_Object );
extern Lisp_Object intern ( char * );
extern Lisp_Object Fintern ( Lisp_Object str, Lisp_Object obarray);
extern Lisp_Object Fintern_soft ( Lisp_Object, Lisp_Object );
extern Lisp_Object oblookup( Lisp_Object, char *, int );
static int hash_string ( unsigned char *, int );
extern void map_obarray ( Lisp_Object, void (*) (), Lisp_Object );
extern void mapatoms_1 ( Lisp_Object, Lisp_Object);
extern Lisp_Object Fmapatoms(Lisp_Object, Lisp_Object);
extern void init_obarray ();
extern void defsubr ( struct Lisp_Subr * );
extern void defvar_int ( char *, int *, char * );
extern void defvar_bool ( char *, int *, char * );
extern void defvar_lisp ( char *, Lisp_Object *, char *);
extern void defvar_lisp_nopro ( char *, Lisp_Object *, char * );
extern void defvar_per_buffer ( char *, Lisp_Object *, char * );
extern void init_read ();
extern void syms_of_read ();
/* lread.c */
/* data.c */
extern enum arithop;
extern Lisp_Object wrong_type_argument ( Lisp_Object, Lisp_Object );
extern void  pure_write_error ();
extern void args_out_of_range ( Lisp_Object, Lisp_Object ); 
extern void args_out_of_range_3 ( Lisp_Object, Lisp_Object, Lisp_Object ); 
extern Lisp_Object make_number ( int num );
extern int sign_extend_lisp_int ( int num );
extern Lisp_Object Feq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fnull ( Lisp_Object );
extern Lisp_Object Fconsp ( Lisp_Object );
extern Lisp_Object Fatom( Lisp_Object );
extern Lisp_Object Flistp ( Lisp_Object );
extern Lisp_Object Fnlistp ( Lisp_Object );
extern Lisp_Object Fintegerp( Lisp_Object );
extern Lisp_Object Fnatnump ( Lisp_Object ); 
extern Lisp_Object Fsymbolp ( Lisp_Object ); 
extern Lisp_Object Fvectorp ( Lisp_Object );
extern Lisp_Object Fstringp ( Lisp_Object );
extern Lisp_Object Farrayp ( Lisp_Object );
extern Lisp_Object Fsequencep ( Lisp_Object );
extern Lisp_Object Fbufferp ( Lisp_Object );
extern Lisp_Object Fmarkerp ( Lisp_Object );
extern Lisp_Object Finteger_or_marker_p ( Lisp_Object ); 
extern Lisp_Object Fsubrp ( Lisp_Object );
extern Lisp_Object Fchar_or_string_p ( Lisp_Object ); 
extern Lisp_Object Fcar ( Lisp_Object );
extern Lisp_Object Fcar_safe ( Lisp_Object );
extern Lisp_Object Fcdr ( Lisp_Object ); 
extern Lisp_Object Fcdr_safe ( Lisp_Object );
extern Lisp_Object Fsetcar ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fsetcdr ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fboundp ( Lisp_Object );
extern Lisp_Object Ffboundp ( Lisp_Object );
extern Lisp_Object Fmakunbound ( Lisp_Object );
extern Lisp_Object Ffmakunbound ( Lisp_Object );   
extern Lisp_Object Fsymbol_function ( Lisp_Object );
extern Lisp_Object Fsymbol_plist ( Lisp_Object );
extern Lisp_Object Fsymbol_name ( Lisp_Object );  
extern Lisp_Object Ffset ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fsetplist ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object do_symval_forwarding ( Lisp_Object );
extern void store_symval_forwarding (  Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fsymbol_value ( Lisp_Object );
extern Lisp_Object Fdefault_value ( Lisp_Object ); 
extern Lisp_Object Fset ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fset_default ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fsetq_default ( Lisp_Object );
extern Lisp_Object Fmake_variable_buffer_local ( Lisp_Object ); 
extern Lisp_Object Fmake_local_variable ( Lisp_Object );
extern Lisp_Object Fkill_local_variable ( Lisp_Object );
extern Lisp_Object Faref ( Lisp_Object, Lisp_Object );
extern Lisp_Object Faset ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Farray_length ( Lisp_Object ); 
extern Lisp_Object Feqlsign ( Lisp_Object, Lisp_Object );
extern Lisp_Object Flss ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fgtr ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fleq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fgeq ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fneq ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fzerop ( Lisp_Object ); 
extern Lisp_Object Fint_to_string ( Lisp_Object );  
extern Lisp_Object Fstring_to_int ( Lisp_Object );
extern Lisp_Object arith_driver ( enum arithop, int, Lisp_Object * ); 
extern Lisp_Object Fplus ( int, Lisp_Object *);
extern Lisp_Object Fminus ( int, Lisp_Object *);
extern Lisp_Object Ftimes ( int, Lisp_Object * );
extern Lisp_Object Fquo ( int, Lisp_Object * ); 
extern Lisp_Object Frem ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fmax( int, Lisp_Object * );
extern Lisp_Object Fmin(int, Lisp_Object * ); 
extern Lisp_Object Flogand(int, Lisp_Object * );
extern Lisp_Object Flogior(int, Lisp_Object * );
extern Lisp_Object Flogxor(int, Lisp_Object * );
extern Lisp_Object Fash ( Lisp_Object, Lisp_Object );
extern Lisp_Object Flsh ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fadd1 ( Lisp_Object );
extern Lisp_Object Fsub1 ( Lisp_Object );
extern Lisp_Object Flognot ( Lisp_Object );
extern void syms_of_data ();
extern void arith_error ( int );
extern void init_data(void);
/* term.c */
extern void ring_bell ();
extern void set_terminal_modes ();
extern void reset_terminal_modes ();
extern void update_begin ();
extern void update_end(void);
extern void set_terminal_window ( int );
extern void set_scroll_region(int, int );
extern void turn_on_insert ();
extern void turn_off_insert ();
extern void turn_off_highlight ();
extern void turn_on_highlight ();
extern void background_highlight ();
static void highlight_if_desired ();
extern void write_standout_marker ( int, int );
extern void reassert_line_highlight ( int, int );
extern void change_line_highlight(int, int, int );
extern void move_cursor(int, int );
extern void raw_move_cursor(int, int );
extern void clear_to_end ();
extern void clear_screen ();
extern void clear_end_of_line ( int );
extern void clear_end_of_line_raw ( int );
extern void output_chars ( char *, int );
extern void insert_chars ( char *, int );
extern void delete_chars ( int );
extern void ins_del_lines ( int, int );
extern int string_cost ( char * );
extern int string_cost_one_line ( char * );
extern int per_line_cost ( char * );
extern void calculate_ins_del_char_costs ();
extern void calculate_costs(void);
extern void term_init ( char * );
/* extern int fatal(char *, long,...);  */
/* macros.c */
extern Lisp_Object Fstart_kbd_macro ( Lisp_Object );
extern Lisp_Object Fend_kbd_macro ( Lisp_Object );
extern void store_kbd_macro_char ( unsigned char );
extern void finalize_kbd_macro_chars ();
extern Lisp_Object Fcall_last_kbd_macro ( Lisp_Object );
static Lisp_Object pop_kbd_macro ( Lisp_Object );
extern Lisp_Object Fexecute_kbd_macro ( Lisp_Object, Lisp_Object );
extern void init_macros ();
extern void syms_of_macros ();
extern void keys_of_macros ();
/* sysdep.c */
/* sys_creat (va_alist)   va_dcl VARARGS */
extern struct save_signal;
extern void discard_tty_input(void);
extern void stuff_char ( char );
extern void init_baud_rate ();
extern void set_exclusive_use ( int );
extern void wait_without_blocking ();
extern void wait_for_termination ( int );
extern void flush_pending_output ( int );
extern void child_setup_tty ( int );
extern void setpgrp_of_tty ( int );
extern void sys_suspend(void);
extern void save_signal_handlers ( struct save_signal * );
extern void restore_signal_handlers ( struct save_signal * );
extern void init_sigio ();
extern void reset_sigio ();
extern void request_sigio ();
extern void unrequest_sigio ();
extern Lisp_Object init_sys_modes ();
extern int tabs_safe_p ();
extern void get_screen_size ( int *, int * );
extern void reset_sys_modes ();
extern void setup_pty ( int );
extern char *start_of_text ();
extern char *start_of_data ();
extern char *end_of_text ();
extern char *end_of_data ();
extern char *get_system_name(void);
extern void select_alarm ();
extern int select ( int, fd_set *, int *, int *, struct timeval * );
extern void read_input_waiting ();
/* callint.c */
extern Lisp_Object Finteractive ( Lisp_Object );
extern Lisp_Object quotify_arg ( Lisp_Object );
extern Lisp_Object quotify_args( Lisp_Object );
static void check_mark ();
extern Lisp_Object Fcall_interactively( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprefix_numeric_value ( Lisp_Object );
extern void syms_of_callint(void);
/* callproc.c */
extern Lisp_Object call_process_cleanup ( Lisp_Object );
extern Lisp_Object Fcall_process (int, Lisp_Object *);
extern Lisp_Object Fcall_process_region ( int, Lisp_Object * );
extern int child_setup(int, int, int, char **, char ** );
extern void init_callproc ();
extern void syms_of_callproc ();
/* minibuf.c */
extern Lisp_Object read_minibuf( Lisp_Object, Lisp_Object, Lisp_Object, int );
extern Lisp_Object get_minibuffer ( int );
extern Lisp_Object read_minibuf_unwind(void);
extern Lisp_Object Fread_from_minibuffer ( Lisp_Object, Lisp_Object, 
						Lisp_Object, Lisp_Object );
extern Lisp_Object Fread_minibuffer ( Lisp_Object, Lisp_Object );
extern Lisp_Object Feval_minibuffer ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fread_string ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fread_no_blanks_input ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fread_variable ( Lisp_Object ); 
extern Lisp_Object Fread_buffer ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Ftry_completion ( Lisp_Object, Lisp_Object, Lisp_Object );
extern int scmp ( char *, char *, int );
extern Lisp_Object Fall_completions ( Lisp_Object, Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fcompleting_read ( Lisp_Object, Lisp_Object, Lisp_Object,
					Lisp_Object, Lisp_Object );
extern void temp_echo_area_contents(char *m);
extern int do_completion ();
extern Lisp_Object assoc_for_completion ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fminibuffer_complete ();
extern Lisp_Object Fminibuffer_complete_and_exit ();
extern Lisp_Object Fminibuffer_complete_word ();
extern Lisp_Object Fdisplay_completion_list ( Lisp_Object );
extern Lisp_Object Fminibuffer_completion_help ();
extern Lisp_Object Fself_insert_and_exit ();
extern Lisp_Object Fexit_minibuffer ();
extern Lisp_Object Fminibuffer_depth ();
extern void init_minibuf_once ();
extern void syms_of_minibuf ();
extern void keys_of_minibuf ();
/* termcap.c */
#ifndef emacs
static memory_out ();
static int xmalloc (int);
static int xrealloc ( ptr,  size);
#endif
static char *find_capability ( char *, char * ); 
extern int tgetnum ( char * );
extern int tgetflag ( char * );
extern char *tgetstr ( char *, char ** );
static char *tgetst1 ( char *, char ** );
extern void tputs(char *, int,  void (*)(char));
extern long tgetent(char *, char *);
static int scan_file (char *, int, struct buffer *);
static int name_match ( char *, char * );
static int compare_contin ( char *, char * );
static char *gobble_line(int, struct buffer *, char * );

/* cm.c */
extern void evalcost (char);
extern void cmputc ( char );
extern void cmcostinit ();
static int calccost ( int, int, int, int, int );
extern void losecursor ();
extern void cmgoto(int row, int col);
extern void Wcm_clear ();
extern int Wcm_init ();
/* filemode.c */
extern void filemodestring(struct stat *, char *);
static char ftypelet(struct stat *);
static void rwx (unsigned short, char []);
static void setst (unsigned short, char []);
/* insdel.c */
extern void move_gap ( int );
extern void gap_left ( int, int );
extern void gap_right ( int );
extern void adjust_markers ( int, int, int );
extern void make_gap ( int );
extern void insert_char ( unsigned char );
extern void InsStr ( char * );
extern void insert( unsigned char *, int );
extern void insert_from_string ( Lisp_Object, int, int );
extern void insert_before_markers(unsigned char *, int );
extern void insert_from_string_before_markers( Lisp_Object, int, int );
extern void del_range ( int, int );
extern void modify_region ( int, int );
extern void prepare_to_modify_buffer ();
/* scroll.c */
extern struct matrix_elt;
extern void scrolling_1 ( int, int, int, int *, int *, int *, int );
extern void calculate_scrolling( struct matrix_elt *, int, int, int *, 
	int *, int *, int );
extern void do_scrolling(struct matrix_elt *, int, int);
extern int scrolling_max_lines_saved ( int, int, int *, int *, int * );
extern int scroll_cost ( int, int, int );
extern void CalcIDCosts ( char *, char *, char *, char *, char  *, char * );
extern void CalcIDCosts1( char *, char *, char *, char *, int *, int *, int );
extern void CalcLID ( int, int, int, int, int *, int * );

/* tparam.c */
extern char *tparam ( char *, char *, int, ...);
extern char *tgoto(char *, int, int );
/* tparam.c */

/* fileio.c */
extern void report_file_error ( char *, Lisp_Object );
extern Lisp_Object Ffile_name_directory ( Lisp_Object );
extern Lisp_Object Ffile_name_nondirectory ( Lisp_Object );
extern char *file_name_as_directory ( char *, char * );
extern Lisp_Object Ffile_name_as_directory ( Lisp_Object );
extern int directory_file_name ( char *, char * );
extern Lisp_Object Fdirectory_file_name ( Lisp_Object );
extern Lisp_Object Fmake_temp_name ( Lisp_Object );
extern Lisp_Object Fexpand_file_name ( Lisp_Object,Lisp_Object );
extern Lisp_Object Fsubstitute_in_file_name ( Lisp_Object );
extern Lisp_Object expand_and_dir_to_file ( Lisp_Object, Lisp_Object );
extern void barf_or_query_if_file_exists ( Lisp_Object, char *, int );
extern Lisp_Object Fcopy_file ( Lisp_Object, Lisp_Object, Lisp_Object,
			       Lisp_Object );
extern Lisp_Object Fdelete_file ( Lisp_Object );
extern Lisp_Object Frename_file ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fadd_name_to_file ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fmake_symbolic_link (Lisp_Object, Lisp_Object, 
					   Lisp_Object ); 
extern Lisp_Object Ffile_name_absolute_p ( Lisp_Object );
extern Lisp_Object Ffile_exists_p ( Lisp_Object );
extern Lisp_Object Ffile_readable_p ( Lisp_Object );
extern Lisp_Object Ffile_symlink_p ( Lisp_Object );
extern Lisp_Object Ffile_writable_p ( Lisp_Object );
extern Lisp_Object Ffile_directory_p ( Lisp_Object );
extern Lisp_Object Ffile_modes ( Lisp_Object );
extern Lisp_Object Fset_file_modes ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Ffile_newer_than_file_p ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object close_file_unwind ( Lisp_Object );
extern Lisp_Object Finsert_file_contents ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fwrite_region ( Lisp_Object, Lisp_Object, Lisp_Object,
				  Lisp_Object, Lisp_Object );
extern int e_write(int, char *, int );
extern Lisp_Object Fverify_visited_file_modtime ( Lisp_Object );
extern Lisp_Object Fclear_visited_file_modtime ();
extern Lisp_Object auto_save_error ();
extern Lisp_Object auto_save_1();
extern Lisp_Object Fdo_auto_save ( Lisp_Object );
extern Lisp_Object Fset_buffer_auto_saved();
extern Lisp_Object Frecent_auto_save_p();
extern Lisp_Object Fread_file_name_internal (Lisp_Object, Lisp_Object, 
					     Lisp_Object );
extern Lisp_Object Fread_file_name ( Lisp_Object, Lisp_Object, Lisp_Object,
				    Lisp_Object );
extern void syms_of_fileio();
/* end of fileio.c */
/* eval.c */
extern void init_eval_once ();
extern void init_eval ();
extern Lisp_Object call_debugger ( Lisp_Object );
extern void do_debug_on_call ( Lisp_Object );
extern Lisp_Object For ( Lisp_Object );
extern Lisp_Object Fand ( Lisp_Object );
extern Lisp_Object Fif ( Lisp_Object );
extern Lisp_Object Fcond ( Lisp_Object );
extern Lisp_Object Fprogn ( Lisp_Object );
extern Lisp_Object Fprog1 ( Lisp_Object );
extern Lisp_Object Fprog2 ( Lisp_Object );
extern Lisp_Object Fsetq ( Lisp_Object );
extern Lisp_Object Fquote ( Lisp_Object );
extern Lisp_Object Ffunction ( Lisp_Object ); 
extern Lisp_Object Finteractive_p ();
extern Lisp_Object Fdefun ( Lisp_Object ); 
extern Lisp_Object Fdefmacro ( Lisp_Object ); 
extern Lisp_Object Fdefvar ( Lisp_Object ); 
extern Lisp_Object Fdefconst ( Lisp_Object ); 
extern Lisp_Object Fuser_variable_p ( Lisp_Object );
extern Lisp_Object FletX ( Lisp_Object );
extern Lisp_Object Flet ( Lisp_Object );
extern Lisp_Object Fwhile ( Lisp_Object );
extern Lisp_Object Fmacroexpand ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fcatch ( Lisp_Object );
extern Lisp_Object internal_catch( Lisp_Object,Lisp_Object (*)(),Lisp_Object );
static void unbind_catch ( struct catchtag * );
extern Lisp_Object Fthrow ( Lisp_Object, Lisp_Object );
extern Lisp_Object Funwind_protect ( Lisp_Object );
extern Lisp_Object Fcondition_case ( Lisp_Object );
extern Lisp_Object internal_condition_case ( Lisp_Object (*)(), Lisp_Object,
				    Lisp_Object (*)());
extern Lisp_Object Fsignal ( Lisp_Object, Lisp_Object );
static Lisp_Object find_handler_clause ( Lisp_Object, Lisp_Object, Lisp_Object,
					Lisp_Object, Lisp_Object * );
/* extern void error ( char *, ...);  */
extern Lisp_Object Fcommandp ( Lisp_Object );
extern Lisp_Object Fautoload ( Lisp_Object, Lisp_Object, Lisp_Object, 
			      Lisp_Object, Lisp_Object );
extern Lisp_Object un_autoload ( Lisp_Object );
extern void do_autoload ( Lisp_Object, Lisp_Object );
extern Lisp_Object Feval ( Lisp_Object );
extern Lisp_Object Fapply ( int, Lisp_Object * );
extern Lisp_Object apply1 ( Lisp_Object, Lisp_Object );
extern Lisp_Object call0 ( Lisp_Object );
extern Lisp_Object call1 ( Lisp_Object, Lisp_Object );
extern Lisp_Object call2 ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object call3 ( Lisp_Object, Lisp_Object, Lisp_Object, Lisp_Object);
extern Lisp_Object Ffuncall ( int, Lisp_Object * );
extern Lisp_Object apply_lambda ( Lisp_Object, Lisp_Object, int );
extern Lisp_Object funcall_lambda ( Lisp_Object, int, Lisp_Object * );
extern void grow_specpdl();
extern void specbind ( Lisp_Object, Lisp_Object );
extern void record_unwind_protect (Lisp_Object (*)(), Lisp_Object );
extern void unbind_to ( int );
extern Lisp_Object Fbacktrace_debug ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fbacktrace();
extern void syms_of_eval();
/* end of eval.c */
/* marker.c */
extern Lisp_Object Fmarker_buffer ( Lisp_Object );
extern Lisp_Object Fmarker_position ( Lisp_Object );
extern Lisp_Object Fset_marker ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object set_marker_restricted ( Lisp_Object, Lisp_Object,
					  Lisp_Object );
extern void unchain_marker ( Lisp_Object );
extern int marker_position ( Lisp_Object );
extern Lisp_Object Fcopy_marker ( Lisp_Object );
extern void syms_of_marker ();
/* search.c */
extern struct re_pattern_buffer;
extern void compile_pattern ( Lisp_Object, struct re_pattern_buffer *, char *);
extern Lisp_Object signal_failure ( Lisp_Object );
extern Lisp_Object Flooking_at ( Lisp_Object );
extern Lisp_Object Fstring_match ( Lisp_Object, Lisp_Object, Lisp_Object );
extern int scan_buffer ( int, int, int, int * );
extern int find_next_newline ( int, int );
extern Lisp_Object Fskip_chars_forward ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fskip_chars_backward ( Lisp_Object, Lisp_Object );
extern void skip_chars ( int, Lisp_Object, Lisp_Object );
static Lisp_Object search_command ( Lisp_Object, Lisp_Object, Lisp_Object,
		Lisp_Object, int, int );
extern int search_buffer ( Lisp_Object, int, int, int, int, unsigned char * );
static Lisp_Object wordify ( Lisp_Object );
extern Lisp_Object Fsearch_backward ( Lisp_Object, Lisp_Object, Lisp_Object, 
				     Lisp_Object );
extern Lisp_Object Fsearch_forward ( Lisp_Object, Lisp_Object, Lisp_Object, 
				     Lisp_Object );
extern Lisp_Object Fword_search_backward ( Lisp_Object, Lisp_Object, 
					  Lisp_Object, Lisp_Object );
extern Lisp_Object Fword_search_forward ( Lisp_Object, Lisp_Object, 
					  Lisp_Object, Lisp_Object );
extern Lisp_Object Fre_search_backward ( Lisp_Object, Lisp_Object, 
					  Lisp_Object, Lisp_Object );
extern Lisp_Object Fre_search_forward ( Lisp_Object, Lisp_Object, 
					  Lisp_Object, Lisp_Object );
extern Lisp_Object Freplace_match ( Lisp_Object, Lisp_Object, Lisp_Object );
extern Lisp_Object Fmatch_beginning ( Lisp_Object );
static Lisp_Object match_limit ( Lisp_Object, int );
extern Lisp_Object Fmatch_end ( Lisp_Object );
extern Lisp_Object Fmatch_data ();
extern Lisp_Object Fstore_match_data ( Lisp_Object );
extern Lisp_Object Fregexp_quote ( Lisp_Object );
extern void syms_of_search ();

/* process.c */
extern struct Lisp_Process;
extern union wait;
extern void update_status(struct Lisp_Process *);
extern Lisp_Object status_convert(union wait );
extern void decode_status(Lisp_Object, Lisp_Object *, int *, int *);
extern Lisp_Object status_message ( Lisp_Object );
extern int allocate_pty();
extern Lisp_Object make_process ( Lisp_Object );
extern void remove_process ( Lisp_Object );
extern Lisp_Object Fprocessp ( Lisp_Object );
extern Lisp_Object Fget_process ( Lisp_Object );
extern Lisp_Object Fget_buffer_process ( Lisp_Object );
extern Lisp_Object get_process ( Lisp_Object );
extern Lisp_Object Fdelete_process ( Lisp_Object );
extern Lisp_Object Fprocess_status ( Lisp_Object );
extern Lisp_Object Fprocess_exit_status ( Lisp_Object );
extern Lisp_Object Fprocess_id ( Lisp_Object );
extern Lisp_Object Fprocess_name ( Lisp_Object );
extern Lisp_Object Fprocess_command ( Lisp_Object );
extern Lisp_Object Fset_process_buffer ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprocess_buffer ( Lisp_Object );
extern Lisp_Object Fprocess_mark ( Lisp_Object );
extern Lisp_Object Fset_process_filter (Lisp_Object, Lisp_Object );
extern Lisp_Object Fprocess_filter ( Lisp_Object );
extern Lisp_Object Fset_process_sentinel ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object Fprocess_sentinel ( Lisp_Object );
extern Lisp_Object Fprocess_kill_without_query ( Lisp_Object, Lisp_Object ); 
extern Lisp_Object list_processes_1 ();
extern Lisp_Object Flist_processes ();
extern Lisp_Object Fprocess_list ();
extern Lisp_Object Fstart_process ( int, Lisp_Object * );
extern void create_process_1 ( int );
extern void create_process ( Lisp_Object, char ** );
extern Lisp_Object Fopen_network_stream ( Lisp_Object, Lisp_Object,
					 Lisp_Object, Lisp_Object );
extern void deactivate_process ( Lisp_Object );
extern void close_process_descs ();
extern Lisp_Object Faccept_process_output ( Lisp_Object );
extern void wait_reading_process_input ( long, long, int );
extern Lisp_Object run_filter ();
extern int read_process_output ( Lisp_Object, int );
extern Lisp_Object Fwaiting_for_user_input_p ();
extern void send_process_trap ();
extern void send_process ( Lisp_Object, char *, int );
extern Lisp_Object Fprocess_send_region (Lisp_Object,Lisp_Object,Lisp_Object);
extern Lisp_Object Fprocess_send_string ( Lisp_Object, Lisp_Object );
extern Lisp_Object process_send_signal(Lisp_Object, int, Lisp_Object, int );
extern Lisp_Object Finterrupt_process ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fkill_process ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fquit_process ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fstop_process ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fcontinue_process ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fprocess_send_eof ( Lisp_Object );
extern void kill_buffer_processes ( Lisp_Object );
extern void sigchld_handler(int signo);
extern void status_notify ();
extern int exec_sentinel ( Lisp_Object, Lisp_Object );
extern void init_process ();
extern void syms_of_process ();

/* filelock.c */
extern struct buffer;
static Lisp_Object lock_file_owner_name (char * );
extern void lock_file ( Lisp_Object );
extern void fill_in_lock_file_name (char *, Lisp_Object );
extern int lock_file_1 ( char *, int );
extern int lock_if_free ( char * );
extern int current_lock_owner ( char * );
extern int current_lock_owner_1 ( char * );
extern void unlock_file ( Lisp_Object );
extern void lock_superlock ( char * );
extern void unlock_all_files ();
extern Lisp_Object Flock_buffer ( Lisp_Object );
extern Lisp_Object Funlock_buffer(void);
extern void unlock_buffer ( struct buffer * );
extern Lisp_Object Ffile_locked_p ( Lisp_Object );
extern void syms_of_filelock ();

/* mocklisp.c */
extern Lisp_Object Fml_if ( Lisp_Object );
extern Lisp_Object ml_apply ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fml_nargs ();
extern Lisp_Object Fml_arg ( Lisp_Object, Lisp_Object );
extern Lisp_Object Fml_interactive ();
extern Lisp_Object Fml_provide_prefix_argument ( Lisp_Object );
extern Lisp_Object Fml_prefix_argument_loop ( Lisp_Object );
extern Lisp_Object Finsert_string ( int, Lisp_Object * );
extern void syms_of_mocklisp ();
