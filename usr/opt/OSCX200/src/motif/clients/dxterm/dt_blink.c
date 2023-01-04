/* #module DT_BLINK "X0.0" */
/*
 *  Title:	DECterm Blink Control
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
 *	Procedures related to blinking items on the DECterm screen,
 *	such as cursor and characters with blink attribute.
 *
 *  Procedures contained in this module:
 *
 *	static void blink_timeout ();
 *	static void start_blink_timer ();
 *  
 *
 *  Author:	Eric Osman 3/24/88
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     07-Oct-1992     Ag/BL10
 *	- Added type cast to satisfy Alpha compiler.
 *
 * Aston Chan		15-Oct-1991	V3.1
 *	- New pixmap parameter added to o_update_segment().  A dummy NULL
 *	  is needed here.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  BMATTHEWS 		June 1990
 *  - Make cursor_*_time a global symbol instead of a psect so the 
 *    name is local to the shareable image
 *
 * Bob Messenger	 8-Apr-1989	X2.0-6
 *	- Make cursor_on_time and cursor_off_time be readonly on VMS
 *	  so we can put this module in the shareable library.
 *
 * Bob Messenger	16-Jan-1989	X1.1-1
 *	- moved many ld fields to common area
 *
 * Tom Porcher		 2-Aug-1988	X0.4-42
 *	- changed w->input.has_focus to i_check_cursor_blink(w).
 *	  This allows DT_input to make blink decision.
 *
 * Mike Leibow		20-Jun-1988	X0.4-something
 *	- added scan_char_blinks_on_line() so that blinking chars
 *	  in the status line could be easily detected.
 * Tom Porcher		21-Jun-1988	X0.4-32
 *	- Changed XtAddTimeout() to XtAppAddTimeout() for new intrinsics.
 *
 * Eric Osman		27-May-1988	X0.4-27
 *	- Use XtAddTimeOut and not old vms version
 * Tom Porcher		25-Apr-1988	X0.4-12
 *	- Added declaration of s_read_data().
 * Eric Osman		17-May-1988	X0.4-27
 *	- Don't set blink timer unless needed.
 *
 */

#include "wv_hdr.h"

extern s_line_structure* s_read_data();

extern Boolean i_check_cursor_blink();





/*
 * Spec says entire cursor blink cycle should be 1 second, with 67%
 * of the time on.  The number of milliseconds for on and off are
 * defined here.
 *
 * The spec says that character blinking should be a full cycle
 * every 2 seconds.  Instead of using a separate timer for character
 * blinking, we'll use the same interrupt for both, and merely only
 * tend to the character blinking on alternate interrupts.
 */

#ifdef VMS_DECTERM
globaldef readonly
#endif
#ifdef VXT_DECTERM
globaldef
#endif VXT_DECTERM

	int

		cursor_on_time = 670,
		cursor_off_time = 330;

/*
 * Respond to blink timer.
 */

		void blink_timeout (w) DECtermData *w;
		/************************************/

{

	Boolean blinking_chars_flag = 0;

	extern void start_blink_timer ();

	wvtp ld = w_to_ld( w );

	Boolean cursor_blink_mode =
	    _cld wvt$l_vt200_flags & vt1_m_cursor_blink_mode ? 1 : 0;
/*
 * Remember that timer is no longer set.
 */
	w->output.blink_set_flag = 0;
/*
 * Alternate the blinking phase and cause  character blinking transition every
 * other interrupt.  This gives character blinking half the rate of cursor
 * blinking.
 *
 * However, we still check for existence of blinking characters every
 * interrupt in order to know whether we need to interrupt again.
 *
 * We only incur the burden of scanning all the characters to see if any
 * are blinking, if the "some_blink_flag" indicates that they may be.
 * If the scan reveals no blinking characters, we clear some_blink, such
 * that no more scanning will occur until draw_segment once again
 * tells us that some are blinking.
 */
	if (w->output.some_blink_flag)
	scan_char_blinks (w, w->output.char_blink_phase,
	    &blinking_chars_flag);
	w->output.some_blink_flag = blinking_chars_flag;

	w->output.char_blink_phase++;
/*
 * If we're supposed to blink cursor, and
 * we've got focus, then reverse it.
 */
	if (w->common.textCursorEnable
	  && cursor_blink_mode && i_check_cursor_blink(w))
	    w->output.char_blink_phase & 1 ? o_display_cursor(w) :
		o_erase_cursor (w);
/*
 * Restart cursor timer, but only if we have focus and cursor blink mode is on
 * or there's at least one blinking character.
 *
 * We depend on other code starting the timer if we don't.  For example,
 * if we don't start timer here because we don't have focus, then we depend
 * on other code calling us when focus is received.
 *
 * If we don't restart timer, make sure cursor is on if it's supposed to be.
 */
	if (blinking_chars_flag ||
	    i_check_cursor_blink(w) && cursor_blink_mode
	      && w->common.textCursorEnable)
	start_blink_timer (w, w->output.char_blink_phase & 1);
	else if (! (w->output.status & CURSOR_IS_VISIBLE)) o_display_cursor (w);
}









/*
 * Procedure to start cursor timer.
 *
 * This procedure knows to not start the timer if it's already
 * set.
 *
 */

		void start_blink_timer (w, phase)
		/*******************************/

			DECtermData	*w;
			Boolean		phase;

{

	extern void blink_timeout ();
/*
 * Don't set timer if it's already set.
 */
	if (w->output.blink_set_flag)
	return;
/*
 * Interrupt a different amount of time from
 * now according to whether we just turned cursor on or off.
 */
	w->output.blink_id = XtAppAddTimeOut (
				      XtWidgetToApplicationContext((Widget)w),
				      phase ? cursor_on_time : cursor_off_time,
				      blink_timeout, w);

	w->output.blink_set_flag = 1;
}








/*
 * Procedure to scan for characters that are supposed to blink, and
 * alternate their states every other time through.
 *
 * blinking_chars_flag is a predicate that indicates 0 if nothing needed to be
 * blinked, and 1 if at least one character needed to be blinked.
 */

		scan_char_blinks (w, phase, blinking_chars_flag)
		/***************************************/

			DECtermData *w;
			Boolean phase;
			Boolean *blinking_chars_flag;

{
	int row;

/*
 * Assume nothing needs to be blinked, until we determine it's true.
 */
	*blinking_chars_flag = 0;
/*
 * Scan visible portion of display for contiguous segments with the
 * blink attribute, and call o_update_segment with that segment.
 */
    for ( row = w->output.top_visible_line;
      row < w->output.top_visible_line + w->output.visible_rows; row++ )

    {
	scan_char_blinks_on_line (w, phase, blinking_chars_flag, row);
    }
    if (w->common.statusDisplayEnable)
	scan_char_blinks_on_line( w, phase, blinking_chars_flag, STATUS_LINE);
}

		scan_char_blinks_on_line (w, phase, blinking_chars_flag, row)
		/***************************************/

			DECtermData *w;
			Boolean phase;
			Boolean *blinking_chars_flag;
			int row;
{
	int column, start_column, scale;

	s_line_structure *ls = s_read_data( w, row);

	start_column = w->output.left_visible_column;

	scale = ls->b_rendits&(csa_M_DOUBLE_WIDTH|csa_M_DOUBLE_HIGH) ?
		2 : 1;

	while (start_column < w->output.left_visible_column +
	    w->output.visible_columns)
	{
/*
 * Find next blinking character as start of new chunk.
 */
    while (start_column < w->output.left_visible_column
	    + w->output.visible_columns
	    && (ls->a_rend_base[start_column] & csa_M_BLINK) == 0)
	  start_column++;

	column = start_column;
/*
 * Find end of this chunk of blinking characters.
 */
    while (
	column < w->output.left_visible_column + w->output.visible_columns
	    && (ls->a_rend_base[column] & csa_M_BLINK) != 0)
	column++;
/*
 * Update this segment if any characters were found and phase is right, and
 * remember that we encountered at least one blinking character.  This
 * information is used to determine whether the blink timer needs to be set
 * again.
 */
	if (column - start_column) {
	    if (phase & 1)
		o_update_segment( w, row,
	  			  scale*start_column,
	  			  scale*(column - start_column),
	  			  0,
	  			  NULL);

	    *blinking_chars_flag = 1;
	}

	start_column = column;
    }
}
