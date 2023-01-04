/*****  REGIS (IDENT = 'V1.65') *****/					/*2*/
/* #module <module name> "X0.0" */
/*
 *  Title:	regis.c
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
* FACILITY:	Common ReGIS interpreter for VT200 and CT100
*
* ABSTRACT:	Main interpreter code
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	David Larrick	CREATION DATE:	6-Jul-81
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
* Dave Doucette		 7-Apr-1993	V1.2/BL2
*	- Added support for ReGIS input cursor.
*
* Aston Chan		12-Jan-1993	Post V3.1
*	- Fix Bart.six and DECW_SSB QAR 717 where DECterm crashes because it
*	  mis-interpreted the S(C(H*)) command as Hard Copy command.  In fact,
*	  it is the Output Cursor command which we are not supporting (yet).
*
* Aston Chan		17-Dec-1991	V3.1
*	- I18n code merge
*
* Bob Messenger		27-Aug-1990	X3.0-7
*	- Initialize alphabet sizes when VARIABLE_CELL_STORAGE_SIZE is set
*	  (fixes problem introduced in X3.0-5).
*
* Bob Messenger         17-Jul-1990     X3.0-5
*      Merge in Toshi Tanimoto's changes to support Asian terminals -
*	- change cell height/width in Kanji ReGIS mode
*
* Bob Messenger		31-May-1989	X2.0-13
*	- initialize tmp_write_flag in new_regis() instead of regis() so
*	  temporary writing modes are restored properly for <ESC>P1p.
*
* Bob Messenger		12-May-1989	X2.0-10
*	- new_regis() and especially regis() can't contain any local
*	  variables, because they can be called with another routine's
*	  frame pointer (on VAX/VMS).  It's not enough to declare rs as a
*	  register, because VAXC ignores the register allocation if we're
*	  compiled /NOOPTIMIZE.
*	  
* Tom Porcher		14-Sep-1988	X0.5-2
*	- temporary fix to WVT$RESET_COLORMAP() so that it does
*	  not accidentally set rstruct->initialized.
*
* Tom Porcher		12-Aug-1988	DECterm X0.4-44
*	- changed to consistent use of fg/bg resources:
*	    - common.background replaced by core.background_pixel.
*	    - set_default_fore/background() now use color 0 as bg, 7 as fg.
*	    - set_default_fore/background() now do not attempt to alter
*	      color map unless ReGIS has been initialized.
*
*	Edit 065	 8-May-84	*RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 064	??-???-83	*RFD
* Translated the origional Bliss code into C.
*
*	Edit 063	14-Oct-83	*AFV				   2
* Have scrn_action_addressing call UPDATE_CELL_STANDARD --		   2
* standard sizes in Regis depend only on the screen addressing		   2
* parameters -- alphabet and direction no longer matter.		   2
*									   2
*	Edit 062	14-Oct-83	*AFV				   2
* Have gid_instruction mark that it is sending information		   2
* to prevent inch_q_unq (in SCAN.C) from zapping it with		   2
* FLUSH_BUFFER's							   2
*									   2
*	Edit 061	25-Aug-83	*AFV
* Change linkage for SCAN.
*
*	Edit 060	23-Aug-83	*AFV
* Change linkage for inch_q_unq and puts_regis and getl_regis
*
*	Edit 059	11-Aug-83	*AFV
* Change "send_gidis" to "G_SEND_GIDIS"
*
*	Edit 058	 9-Aug-83	*AFV
* Add SC_CURRENT_OPCODE global to allow for DRAW_LINES and 
* DRAW_CHARACTERS which use END_LIST terminated lists to 
* flush buffers when waiting for input.  if SC_CURRENT_OPCODE
* is not zero,  before doing FLUSH_BUFFERS send an 
* END_LIST code.  After the FLUSH_BUFFERS, send the contents
* of SC_CURRENT_OPCODE.  Currently used only for 
* DRAW_CHARACTERS.
*
*	Edit 057	 2-Aug-83	*AFV
* Make text work in EXPLICIT_LOCAL mode -- used to request 
* EXPLICIT_GLOBAL mode.
*
*	Edit 056	12-Jun-83	*DCL & AFV
* Change all 'unimplemented_feature' errors to 
* 'UNEXPECTED_CHARACTER' errors.
*
*	Edit 055	14-Jun-83	*DCL & AFV
* Report 'unimplemented_feature' if variable size fonts
* are not implemented.  This is Regis v1 edit 055.
*
*	Edit 054	14-Jun-83	*DCL & AFV
* Support variable length alphabet names.  This is Regis v1 edit 054.
*
* (Regis v1 edit 053 skipped because not applicable.
*
*	Edit 053	17-May-83	*DCL & AFV
* Fix a bug in character set loading when REVERSE_PATTERN is FALSE.
* This is a copy of Regis v1 edit 052 *DCL
*
*	Edit 052	 2-May-83	*AFV
* Prepare for GIDIS v2
*
*	Edit 051	23-Mar-83	*DCL
* GIDIS expects the "extent" argument for loadable alphabets to be
* non-case-folded (i.e. highest loadable ASCII code, although it won't load
* controls).  The ReGIS syntax calls for number of loadable characters,
* i.e. case folded.  Patch routine load_option to unfold.
*
*	Edit 050	22-Mar-83	*DCL
* Routine c_args (color argument processor) now resides in routine REGIS3.
*
*	Edit 049	21-Mar-83	*DCL
* Migrate more routines to module REGIS3:  x1_floating_bracketed_pair, number,
* temp_write_options, and all of their subordinates.  Migrate routine
* new_regis from REGIS3 back to REGIS.  Migrate routine scalar to REGIS3,
* but comment it out there since it is unused.
*
*	Edit 048	16-Mar-83	*DCL
* Implement OVERLAY_STYLE = 2, i.e. VT240 overlay support.
*
*	Edit 047	4-Mar-83	*DCL
* Modify routine p1_pixel_vector to use the same argument block used by
* routine COORDINATES.
*
*	Edit 046	4-Mar-83	*DCL
* Move standalone global routines to module REGIS3.
*
*	Edit 045	21-Feb-83	*DCL
* Install the argument block parameter passing method for routine
* COORDINATES.
*
*	Edit 044	18-Feb-83	*DCL
* Make all of the variable cell storage size code conditional, to save
* space if it is not necessary.
*
*	Edit 043	15-Feb-83	*DCL
* Change save_state and restore_state masks to exclude text state; it is
* saved separately by the text code in module REGIS2.  As part of parser
* init in new-regis, do a save_state( all_states ) in case we ever restore
* without saving first.
*
*	Edit 042	12-Feb-83	*DCL
* Implement the $P1p entry point:  Make routine new_regis global, and make
* it call routine regis; new_regis is now the powerup entry point.  Don't
* call routine new_regis from routine regis; regis is now the reinit ($P1p)
* entry point (in addition to continuing to be the main parser loop).
*
*	Edit 041	28-Jan-83	*DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The
* version number is now in the IDENT modifier of the MODULE heading.
*
* 	Edit 040	11-Jan-83	*DCL
* Fix a conditional compilation switch so we don't get undefineds
* when DIRECT_GID_ENABLED is FALSE.
*
*	Edit 039	14-Dec-82	*DCL
* Implement REVERSE_PATTERN_REGISTER = 0, i.e. put in conditionals for
* whether the pattern register shifts out high bit or low bit first.
* Affects both the pattern register load and character set load routines.
*
*	Edit 038	4-Nov-82	*DCL
* Implement OVERLAY_STYLE = 1, i.e. call REGIS1 and REGIS2 overlays
* through a separate mapping module.
*
*	Edit 037	27-Oct-82	*DCL
* Remove impure data to module REGISRAM.
*
*	Edit 036	15-Oct-82	*DCL
* Strip off color parsing and report routines into overlay module
* REGIS1.  Take text routines from RGTEXT.REQ to make overlay module
* REGIS2.  Make various service routines global so the overlay
* modules can see them.  Also count number of times skip_paren
* recurses so we don't blow the stack away.
*
*	Edit 035	6-Oct-82	*BKC
* Change GID_PROCESS call for the direct path to GIDIS to be a
* GIDI_DIRECT and 9240 to be GID_PASSWORD macros.
*
*	Edit 034	28-Sep-82	*DCL
* Don't resend pattern register on new V instruction when the pattern
* is all ones.
*
*	Edit 033	7-Sep-82	*DCL
* Add an initial dummy argument to G_LINE_TEXTURE.  This argument
* is reserved for future use as a pattern register length.
* Change some error code names:  extra_hardcopy_coordinates -->
* EXTRA_OPTION_COORDINATES (since this error occurs for both S(H) and
* S(A)) and extra_coordinates --> EXTRA_COORDINATE_ELEMENTS.
* Rip out some old commented code related to the possibility of not
* reversing pattern register bits for some products.
*
*	Edit 032	10-Aug-82	*DCL
* Implement correct HLS defaults.
*
*	Edit 031	20-Jul-82	*DCL
* Handle correctly the construct V(W(...))[...][...], i.e. teach
* temporary write options to stay in effect during subsequent
* arguments to the same instruction.
*
*	Edit 030	16-Jun-82	*DCL
* Don't map S(H(P[])) (hardcopy position) specifier.  When S(H) has
* only one position argument, default the other to [+0,+0] correctly.
* Init hardcopy position specifier correctly.  Fix a bug in temp
* write options.  Send <CR>s for unimplemented reports.  Make the G
* instruction harder to get at.
*
*	Edit 029	15-Jun-82	*DCL
* Implement init string and remove temporary initialization code from
* new_regis.
*
*	Edit 028	9-Jun-82	*DCL
* Re-send the pattern register for each V instruction; reset various
* states on S(E); make the arguments to S(H(P[,])) sticky; move
* translation of the "RGB" color specifiers from this module (REGIS)
* to the device-dependent color model module (COL???).
*
*	Edit 027	1-Jun-82	*DCL
* Pass the new pattern register multiplier to GIDIS even when the
* pattern register value itself does not change.
*
*	Edit 026	16-Apr-82	*DCL
* Fix a bug that causes W(S1,) to generate incorrect GIDIS.  Also
* make a preliminary fix to a one-off error in screen addressing.
*
*	Edit 025	25-Mar-82	*DCL
* Fix a 16th-scan-line bug in character set loading.  Implement best
* match algorithm for HLS or RGB specifiers for screen and write
* intensity.
*
*	Edit 024	22-Mar-82	*DCL
* Fix a few one-off errors in screen addressing and coordinates
* calculations.  Reverse bits in loadable fonts.  Finish job of
* reversing bits in pattern register loading, especially predefined
* patterns.
*
*	Edit 023	17-Feb-82	*DCL
* Install hooks to call HLS to RGB color mapping module, thus
* partially implementing color map handling.  Reverse order of bits
* in pattern register.  In screen addressing calculations, use number
* of pixels vs. highest pixel number correctly.  Fix ASCIZ problems
* of reports by modifying binary_to_decimal_convert to return the
* number of characters in the output string.
*
*	Edit 022	9-Feb-1982	J.A.Lomicka
* Change name of main routine from main to regis.  Remove main
* declaration.
*
*	Edit 021	27-Jan-82	*DCL
* Implement negated writing modes.  Start all canned patterns with 1.
* Implement G instruction for GIDIS debugging.  Implement W(F).
* Implement S(D) (data shift toggle).  Delimit error report with
* double quotes, not parens.
*
*	Edit 020	11-Jan-82	*DCL
* Implement external report paths.  Enhance resynch (;) handling.
*
*
*	Edit 019	8-Jan-82	*DCL
* Handle redundant comma in character set loading.  Fix lower case
* problem following quotation (use INCH, not INCH_Q).  Fix precedence
* of "E" and "." in numbers (2E1.9 no longer yields 29).
*
*	Edit 018	7-Jan-82	*DCL
* Implement P(S) and V(S).  Make reentrant data structure global.
*
*	Edit 017	6-Jan-82	*DCL
* Fix another annoying bug in character set loading.
*
*	Edit 016	4-Jan-82	*DCL
* Fix an annoying bug in character set loading.  gid_PROC is no
* longer an external routine; its equivalent is declared in GIDDEF.
*
*	Edit 015	22-Dec-81	*DCL
* Implement internal parser initialization.
*
*	Edit 014	19-Dec-81	*DCL
* Convert all OWN variables to block references, hidden by macro
* calls.  This technique provides hooks for reentrancy.
*
*	EDIT 013	17-Dec-81	*DCL
* Implement temporary write options.
*
*	Edit 012	15-Dec-81	*DCL
* Reporting functions internal to the parser are implemented.
* Integration and testing of report functions await implementation of
* the appropriate hooks in GIDIS (for position report) and SCAN (for
* macrograph reports).  Alphabet name report and error report are
* fully functional.
*
*	Edit 011	25-Nov-81	*DCL
* Significantly change coordinate handling once again, to handle
* always-square logical pixels.  Implement character set loading.
*
*	Edit 010	28-Oct-81	*DCL
* Finish implementing shading.  Implement pattern register processing.
*
*	Edit 009	12-Oct-81	*DCL
* Modified SKIP_QUOTE routine to take its terminating character as an
* argument rather than in .CHAR.  Partially implemented shading.
* Revised screen addressing and coordinate handling to use powers of
* 2 in addition to powers of 10, thereby increasing both efficiency
* and resolution.  The upper limit of coordinates passed to GIDIS for
* S(A instructions will be between 2 ** 13 and 2 ** 14 - 1, thus
* allowing one bit for positive offscreen addressing and one bit for
* negative offscreen addressing.
*
*	Edit 008	23-Sep-81	*DCL
* Implemented screen addressing and appropriate transformations of
* coordinates and pixel vectors.
*
*	Edit 007	14-Sep-81	*DCL
* Implemented write instructions.
*
*	Edit 006	29-Aug-81	*DCL
* Implemented screen instructions, except for some of the color
* processing - colors are collected in HLS, not yet converted to
* RGB.  Pixel vectors are now translated into vectors, etc, by
* this program, although pixel vector multipliers and logical
* pixel size have yet to be implemented.
*
*	Edit 005	25-Aug-81	*DCL
* Partially implemented screen instructions.  Rewrote COORDINATES
* routine to work around a compiler problem.
*
*	Edit 004	20-Aug-81	*DCL
* Installed fraction handling and exponent adjustment for coordinates.
* Generalized the number parsing routine to handle non-coordinate
* numbers and to perform range checking.  
*
*	Edit 003	10-Aug-81	*DCL
* Significantly improve coordinate handling, but still use the old
* word-encoded (4095 maximum) coordinate passing scheme for GIDIS.
*
*	Edit 002
* Coordinates and pixel vectors work for C instructions.
* Install an #include command so Jeff can proceed independently on
* implementing text instructions.
*
*	Edit 001	14-Jul-81	*DCL
* Fill in skeleton.  Coordinates and pixel vectors work for P and
* V instructions.  Only integers (no fractions or exponents) are
* implemented for coordinates.  Default for coordinates is absolute
* zero, not relative zero.
**/

/**
*		F I L E    D I R E C T O R Y
**/

/**
* FILE		DESCRIPTION
* ----		-----------
*
* COLrgb.C  NOTE:  r, g, and b are respectively the number of bits of red,
* 	     green, and blue resolution in the hardware.
*		Color model: device dependent color code
*    COL332.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			hlsrgb(v,v,v,a)					no
*			rgbhls(v,a,a,a)					no
*
* FIELDS.C	Simulates bit field operations.
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			get_field(v,v,v)				yes
*			put_field(v,v,v,a)				no
*			get_mask(v)					yes
*
* GIDISQ.C	QIO version.
* GIDISR.C
* GIDIST.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			gid_flush()					no
*			gid_kill()					no
*			gid_new()					no
*			rgid_process(a,v)				no
*			gid_report(a,v)					yes
*			gr_flush()					no
*
* MGMM.C	Macrograph memory manager
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			mac_append(v)					no
*			mac_clear_all()					no
*			mac_redefine(v)					no
*			mg_delete(v)					no
*			mgm_report(a,a)					no
*
* MULTILINE.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			draw_multi_line(v,a,a)				no
*			sign_mult_6(v,v,v,v,v,v)			yes
*
* OPSYS.C	Helpful IO routines used for testing and
*		debugging -- not to be part of the product.
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			getl_regis()					yes
*			open_regis()					yes
*			PutDecimal(v,a)					no
*			PutOctal(v,a)					no
*			PutString(a,a)					no
*			puts_regis(a,v)					no
*
* RECURV.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			cdraw(v,v,v)					no
*			crv_begin(v)					no
*			crv_continue(v,v)				no
*			crv_end()					no
*			draw_arc(v,v,v,v,v)				no
*
* REDRAW.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			draw_line(v,v)					no
*
* REGIO.C	Helpful IO routines used for testing and
*		debugging only -- not to be part of the
*		product.
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			Get_Whole_Line(a,v)				yes
*			GetChar(a)					yes
*			GetLine(a,v)					yes
*			PeekChar(a)					yes
*			PutBreak()					no
*			PutChar(v)					no
*
* REGIS.C	Main parser code.
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			alphabet()					no
*			crv_instruction()				no
*			crv_option(a,a)					no
*			cursor_select()					no
*			data_shift()					no
*			gid_instruction()				no
*			hardcopy()					no
*			h2_hardcopy_suboption()				no
*			load1_action(v)					no
*			load_instruction()				no
*			load_option()					no
*			new_regis()					no
*			p1_pixel_vector()				no
*			pos_instruction_vector(v)			no
*			pos_option_vector(v)				no
*			regis()						no
*			scrn1_addressing()				no
*			scrn_action_addressing(v,v,v,v,v,v,v,v)		no
*			scrn_instruction()				no
*			scrn_option()					no
*			scrn_scale()					no
*			sync()						no
*			timer()						no
*
* REGIS1.C	Parser overlay 1:  reports and device - independent color code
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			c_map()						no
*			color_map_suboption(v)				no
*			binary_to_decimal_convert(v,v,a,a)		no
*			rprt_alphabet()					no
*			rprt_error()					no
*			rprt_instruction()				no
*			rprt_macrographs(a)				no
*			r2_rprt_macrographs_suboption(a)		no
*			rprt_option()					no
*			rprt_position()					no
*			rprt1_action_position(v)			no
*			rprt_suboption_position()			no
*			set_color_map(v,v,v)				no
*
* REGIS2.C	Parser overlay 2:  text command parsing
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			a_option()					no
*			d_option()					no
*			h_option()					no
*			i_option()					no
*			m_option()					no
*			p2_pixel_offset(v,v,a,a)			no
*			r1_restore_test_state()				no
*			save_text_state()				no
*			send_quoted_string(v)				no
*			standard_size(v)				no
*			s_option()					no
*			text_instruction()				no
*			text_option()					no
*			u_option()					no
*			update_cell_standard()				no
*
* REGIS3.C	Stand-alone global routines
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			bracketed_pair(a,a,a)				yes
*			c_args(a,a)					yes
*			convert_floating_to_integer(v,v)		yes
*			coordinates()					yes
*			divide_by_10_rounded(v,v)			yes
*			fetch_one_character(v)				yes
*			x1_floating_bracketed_pair(a,a,a,a,a,v,v)	yes
*			ignore_ch()					no
*			intensity_suboption(a)				no
*			number(a,a,v)					yes
*			p0_pattern_register()				no
*			p1_p0_pattern_register_suboption(a)		yes
*			rel_coordinates()				yes
*			scan(a)						yes
*			scrn_write_intensity(v)				no
*			set_line_texture()				no
*			s0_shading()					no
*			s1_shading_action(v,v)				no
*			s2_shading_suboption()				no
*			skip_coord()					no
*			skip_option()					no
*			skip_paren()					no
*			skip_quote()					no
*			subtract_floating(v,v,v,v,a,a)			no
*			temp_write_options()				no
*			write_instruction()				no
*			w1_write_option()				no
*
* REGIS4.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			load_alphabet(v,a,v,v)				no
*			restore_state()					no
*			save_state()					no
*			scroll_screen(v,v)				no
*			reg_wait(v)					no
*
* REGLOB.C	Contains all the global variable storage declarations.  ( RAM )
*
* RENUMBER.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			a1_accumulate_digit()				yes
*			a2_accumulate_exponent()			no
*			a3_accumulate_exponent_digits()			no
*			a4_accumulate_fraction()			no
*			float_number(a,a,a,v)				yes
*			f1_floating_scaler()				yes
*			scaler(a,a,v)					yes
*
* REPORT.C
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			get_gidi_report(v,v,a,v)			yes
*			request_gidi_pos()				no
*
* SCAN.C	Scanner:  operating system-dependent character input, case
*		folding, macrograph dispatch
*
*			Functions(parameter passing)		Value Returned
*			----------------------------		--------------
*			clear_all_macrographs()				no
*			define_a_macrograph()				yes
*			expand_a_macrograph(v)				no
*			inch()						yes
*			inch_q_unq(v)					yes
*			new_scanner()					no
*			p_inch(v)					no
*			process_atsign()				yes
*			rst_scanner()					no
**/

/*****		I N C L U D E    F I L E    D I R E C T O R Y		 *****/
/*									     */
/**
* FILE		CONTENTS				    USED BY
* ----		--------				    -------
*    
* GIDCALLS.H	Translates the routine call syntax that	REGIS, REGIS1, REGIS2
*		the ReGIS parser uses to call GIDIS 
*		into whatever transport mechanism is
*		actually used to invoke GIDIS functions.
*
* REGDEVDEP.H  Theoretically contains device-dependent
*		constants.  Actually points to another
*		file which contains device-dependent
*		constants, e.g. REGXT100.H or
*		REGVT240.H.
*
* REGINIT.H	Initial state for the parser, coded
*		in the ReGIS protocol.
*
* REGSTRUCT.H	RS struct to make regis reentrant. Included by all c modules.
*		Note that all global/static variables are now in RS.
*
* REGUSEFUL.H   Useful literals and macros,		REGIS, REGIS1, REGIS2
*		including some debugging macros.
*
* REGVT240.INC	Device-dependent constants for VT240	REGDEVDEP.H
*
* REGXT350.H	Device-dependent constants for XT350	REGDEVDEP.H
**/

#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L   F U N C T I O N S	         *****/
/*									     */
/*    FUNCTION		          DESCRIPTION			      FILE   */
/*    --------		          -----------			      ----   */
extern
    bracketed_pair(),	       /* Process [ (unscaled, normalized)    REGIS3 */
    convert_floating_to_integer(), /* 				      REGIS3 */
    coordinates(),	       /* Process [ (scaled, normalized, abs) REGIS3 */
    crv_begin(),	       /*  				      RECURV */
    crv_continue(),	       /*  				      RECURV */
    crv_end(),		       /*  				      RECURV */
    crv_abort(),	       /* 				      RECURV */
    draw_line(),	       /*				      REDRAW */
    draw_arc(),		       /*				      RECURV */
    fetch_one_character(),     /* 				      REGIS3 */
    x1_floating_bracketed_pair(), /* Process [ (unscaled,not normalized) REGIS3 */
    gid_new(),		       /* Inititialize entry point for	      GIDIS  */
			       /*   GIDIS processor			     */
    rgid_process(),	       /* Send gidis commands.		      GIDIS  */
    ignore_ch(),	       /* Skip over a character		      REGIS3 */
    inch(),		       /* Fetch char, normal mode	      SCAN   */
    inch_q_unq(),	       /* Fetch char, quoted & unquoted modes SCAN   */
    load_alphabet(),	       /*				      REGIS4 */
    new_scanner(),	       /* Init entry point for scanner	      SCAN   */
    number(),		       /* Collect signed, normalized number   REGIS3 */
    p_inch(),		       /* push character back into scanner    SCAN   */
#if VARIANT == 1 
    PutBreak(),		       /* Flush all pending terminal output   REGIO  */
    PutChar(),		       /* Queue character for terminal output REGIO  */
    PutDecimal(),	       /* Output signed decimal number	      OPSYS  */
    Put_String(),	       /* Output an asciz string	      OPSYS  */
#endif
    rel_coordinates(),         /* Process [ (scaled, normalized)      REGIS3 */
#if SEPARATE_CURRENT_POSITION 
    rpg_request_gidi_pos(),	       /* Request and read current	      REPORT */
#endif			       /*   GIDIS position			     */
    restore_state(),	       /*				      REGIS4 */
    rst_scanner(),	       /* Reset point for scanner	      SCAN   */
    save_state(),	       /*				      REGIS4 */
    save_text_state(),	       /*				      REGIS2 */
    scan(),		       /* Lookup char in list	 	      REGIS3 */
    scrn_write_intensity(),    /* Process S(I and W(I		      REGIS3 */
    scroll_screen(),	       /* Do a scroll_clipping_region	      REGIS4 */
    set_line_texture(),	       /*				      REGIS3 */
    skp_option(),	       /* Option skip state		      REGIS3 */
    skp_paren(),	       /* () skip state			      REGIS3 */
    skp_quote(),	       /* Quotation skip state		      REGIS3 */
    skp_coord(),	       /* Skip past [] pair		      REGIS3 */
    standard_size(),	       /*				      REGIS2 */
    subtract_floating(),       /* 				      REGIS3 */
    temp_write_options(),      /* Process <instruction>(W	      REGIS3 */
    ucs_update_cell_standard(),    /*				      REGIS2 */
    wi_write_instruction(),       /* Process W			      REGIS3 */
    reg_wait();		       /*				      REGIS4 */

/*    XT100-style overlays          VT240-style overlay			     */
#if ( (OVERLAY_STYLE == 1)    ||    (OVERLAY_STYLE == 2) )
extern
    map_cmap(),		       /* Calls c_map (REGIS1)      	     REGISOVR*/
			       /*   to process S(M, 		             */
    map_rep_inst(),	       /* Calls rprt_instruction (REGIS1)    REGISOVR*/
			       /*   to process R,		             */
    map_text_inst();	       /* Calls text_instruction (REGIS2)    REGISOVR*/
			       /*  to process T,		             */
#else
extern
    c_map(),			/* Process S(M			      REGIS1 */
    ri_rprt_instruction(),		/* Process R			      REGIS1 */
    text_instruction();		/* Process T			      REGIS2 */
#endif

/*****		F U N C T I O N S   I N   T H I S   F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		       VALUE */
/*	--------			-----------		       ----- */
/**
*	alphabet()			Process L(A			NO
*	crv_instruction() 		Process C			N0
*	crv_option(a,a)			Process C(			N0
*	cursor_select() 		Process S(C			N0
*	data_shift()			Process W(D			N0
*	gid_instruction() 		Process G			N0
*	hardcopy()			Process S(H			N0
*	h2_hardcopy_suboption()		Process S(H(			N0
*	load1_action()			Generate GIDIS for L		N0
*	load_instruction()		Process L			N0
*	load_option()			Process L(			N0
*	new_regis()			Powerup entry point		N0
*	p1_pixel_vector()		Process pixel vectors		N0
*	pos_instruction_vector(v)	Process P and V			N0
*	pos_option_vector(v)		Process P( and V(		N0
*	regis()				Powerup entry point		N0
*	scrn1_addressing()		Process S(A			N0
*	scrn_action_addressing(v...)	Generate GIDIS for S(A		N0
*	scrn_instruction()		Process S			N0
*	scrn_option()			Process S(			N0
*	scrn_scale()			Process S(N			N0
*	sync()				Process ;			N0
*	timer()				Process S(T			N0
**/

alphabet()
{
    /**
    * FUNCTIONAL DESCRIPTION: 
    *   Handle the A(lphabet) option of the L(oad) instruction 
    * 
    * FORMAL PARAMETERS:	none
    * 
    * IMPLICIT INPUTS:		ld_alphabet
    *
    * IMPLICIT OUTPUTS:
    *   al_name [ld_alphabet * TOTAL1_CHARS_PER_ALPH_NAME + character]
    *   ld_alphabet
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {
	switch (scan( ")" ))
	{
	    case -3 :		/*					     */
	    case -1 : 		/* Sync, unexpected alpha, right paren 	     */
	    case  1 :		/*					     */
		rs->first_process_me = TRUE;
		return;
	    case -2 :		/* Quotation - load alphabet name 	     */
	    {
		int    done;
		int    name_character;
		int    quote_character;

		done = FALSE;
		quote_character = rs->character;
		name_character = 0;

		while (name_character <= (TOTAL1_CHARS_PER_ALPH_NAME - 1))
		{
		    rs->character = INCH_Q(0);	/* Fetch one character, literal mode */

		    if (rs->character == quote_character) /* Was character a quote?   */
		    {		/* Yes - check next char for duplication     */

			if ((rs->character = INCH_Q(0)) == quote_character)
			{ 	/* Duplicates - fall through 		     */
			}
			else
			{	/* Really closing quote - 		     */
			    p_inch( rs->character ); /* push unwanted char back into  */
				/* scanner so that quote xxx quote @x works. */
			    done = TRUE;
			    break;	/* Exit while loop.		     */
			}

		    }		/* Non-quote, or quoted quote - 	     */

		    rs->al_name[ (rs->ld_alphabet * TOTAL1_CHARS_PER_ALPH_NAME)
				        + name_character ] = rs->character;
		    name_character++;	/* ...and count it 		     */
		}

		if ( done )		/* Did we see a closing quote?       */
		{ 			/* Yes - save actual length 	     */
		    rs->al_length[ rs->ld_alphabet ] = name_character;
		}
		else	
		{		/* no - handle any extra characters 	     */
		    skp_quote( quote_character );
		    rs->al_length[ rs->ld_alphabet ] = TOTAL1_CHARS_PER_ALPH_NAME;
		}
		break;		/* Exit case				     */
	    }
	    case  0 :		/* Unexpected non-alpha - number? 	     */
	    {
		int    rel;
		int    temp;

		rs->first_process_me = TRUE;

		if (number( &temp, &rel, 0 ))
		{
		    if ((temp < TOTAL3_NUMBER_OF_ALPHABETS) && (temp >= 0))
		    {		/* Yes - in valid alphabet range? 	     */
			rs->ld_alphabet = temp;  /* make it the current alphabet */
				/*  but don't tell GIDIS about it -- 	     */
				/*  load_alphabet() will handle that.	     */
		    }
		    else
		    {
			ERROR( ALPHABET_OUT_OF_RANGE, 0 );
		    }
		}
		else
		{
		    ignore_ch();
		}
	    }
	}
    }
}

crv_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the C(urve) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issue G_CENTER_ARC, G_crv_CONTINUE, and/or
    *   G_CIRCUM_ARC.
    *
    **/

	int	crv_mode;	/*  0 for circles 		      */
    					/*  1 for circles, circumference mode */
    					/*  2 for open and closed curves      */
	int	degrees;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    crv_mode = 0;			/* Initialize to circles 	     */
    degrees = 360;			/* Initialize to circles 	     */

    if ( ( rs->pt_register & 0xffff ) != 0xffff )
			 		/* if pattern is not solid 	     */
    {
	G49_SET_LINE_TEXTURE(16, rs->pt_register, rs->pt_multiplier);
    }					/* ...reinit pattern register 	     */
    while ( TRUE )
    {
	switch (scan( "(01234567[)" ))
	{
	    case -3 :
	    case -1 :		/*  ; or unexpected alpha (new cmd) 	     */
		if (crv_mode == 2)
		{
		    crv_abort();
		}
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();	/* Nothing interesting 			     */
		break;
	    case  1 :		/* Left paren 				     */
		crv_option( &crv_mode, &degrees );
		break;
	    case  2 :		/* Digits 				     */
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
		p1_pixel_vector();	/* Convert to x and y offsets 		     */
		switch ( crv_mode )	/* Use the right mode 		     */
		{
		    case  0 :		/*  C(Axx) [peripheral point] 	     */
		    {
			int	x_center;
			int	y_center;
#if SEPARATE_CURRENT_POSITION 
			if (IS_FALSE(rs->gid_xy_valid))
			{
			    rpg_request_gidi_pos();
			}
#endif
			x_center = rs->gid_x;
			y_center = rs->gid_y;
			draw_arc(x_center, y_center, rs->x_crd, rs->y_crd, degrees);
			G56_SET_POSITION( x_center, y_center );
			rs->gid_x = x_center;
			rs->gid_y = y_center;
#if SEPARATE_CURRENT_POSITION 
			rs->gid_xy_valid = TRUE;
#endif
			break;		/* Exit switch (crv_mode)	*/
		    }
		    case  1 :		/*  C(C Dxx) [center] 		*/
#if SEPARATE_CURRENT_POSITION 
			if (IS_FALSE( rs->gid_xy_valid ))
			{
			    rpg_request_gidi_pos();
			}
#endif
			draw_arc( rs->x_crd, rs->y_crd, rs->gid_x, rs->gid_y, degrees );
			break;		/* Exit switch (crv_mode)	*/
		    case  2 :		/*  C(B) or C(S) 		*/
			crv_continue( rs->x_crd, rs->y_crd );
		}			/* End switch (crv_mode)	*/
		break;			/* Exit switch scan		*/
	    case 10 :
		if (IS_TRUE( coordinates() ))
		{
		    switch (crv_mode)	/* Use the right mode 		*/
		    {
			case  0 :	/*  C(Axx) [peripheral point] 	*/
			{
			    int x_center;
			    int y_center;
#if SEPARATE_CURRENT_POSITION 
			    if (IS_FALSE(rs->gid_xy_valid))
			    {
				rpg_request_gidi_pos();
			    }
#endif
			    x_center = rs->gid_x;
			    y_center = rs->gid_y;
			    draw_arc(x_center, y_center, rs->x_crd, rs->y_crd, degrees);
			    G56_SET_POSITION( x_center, y_center );
			    rs->gid_x = x_center;
			    rs->gid_y = y_center;
#if SEPARATE_CURRENT_POSITION 
			    rs->gid_xy_valid = TRUE;
#endif
			    break;	/* Exit switch (crv_mode)	*/
			}		/* End case 0			*/
			case  1 :	/*  C(C Axx) [center] 		*/
#if SEPARATE_CURRENT_POSITION 
			    if (IS_FALSE( rs->gid_xy_valid ))
			    {
				rpg_request_gidi_pos();
			    }
#endif
			    draw_arc(rs->x_crd, rs->y_crd, rs->gid_x, rs->gid_y, degrees);
			    break;	/* Exit switch (crv_mode)	*/
			case  2 :	/*  C(B) or C(S) 		*/
			    crv_continue( rs->x_crd, rs->y_crd );
		    }
		}
		break;

	    case 11:	/* right paren */
		if ( rs->sh_mode >= WANT_POLYGON_FILL )
		    {
		    rs->first_process_me = TRUE;
		    return;
		    }
		else
		    ignore_ch();
		break;
	}
    }
}

crv_option( crv_mode, degrees )
int	*crv_mode;
int	*degrees;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    * 	 Handle the options of the C(urve) instruction
    *
    * FORMAL PARAMETERS:
    * 	 crv_mode	= address of a byte containing
    *				0 for circles
    *				1 for circles, circumference mode
    *				2 for open or closed curves
    *	 degrees	= address in which to store C(A argument
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issue crv_begin and crv_end
    **/

    int		rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {
	switch (scan( "(ABCESW)" ))
	{
	    case -3 :		/* Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :
		skp_option();		/* Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/* Left paren */
		break;
	    case  2 : 			/* A */
		number( degrees, &rel, 0 );
		break;
	    case  3 :			/* B */
		if (*crv_mode == 2)
		{
		    crv_abort();
		}
		*crv_mode = 2;
		crv_begin( TRUE );
		break;
	    case  4 :
		if (*crv_mode == 2)
		{
		    crv_abort();
		}
		*crv_mode = 1;	/* C */
		break;
	    case  5 :
		if (*crv_mode == 2)   /* E - only end a curve if we began one */
		{
		    crv_end();
		    *crv_mode = 0;
		}
		break;			/*  if no curve, do nothing */
	    case  6 :			/* S */
		if (*crv_mode == 2)
		{
		    crv_abort();
		}
		*crv_mode = 2;
		crv_begin( FALSE );
		break;
	    case  7 : 			/* W */
		temp_write_options();
		break;
	    case  8 :
		return;			/* Right paren */
	}
    }
}

void regis_motion_rband_rectangle()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle drawing the rubberband square option of ReGIS.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		Screen Image has rectangle drawn on it.
    **/

#define MAP_X_ABSOLUTE( x ) ( (int) ( (x) * rs->x_scale + rs->x_offset ) )
#define MAP_Y_ABSOLUTE( y ) ( (int) ( (y) * rs->y_scale + rs->y_offset ) )
int		 start_x, start_y, 
		 rx,      ry, 
		 hard_x,  hard_y,
		 dummy;
unsigned int	 udummy;
Window 		 wdummy;

struct regis_cntx 
		*rs = RSTRUCT;
DECtermWidget    w  = rs->widget;

if (!XQueryPointer( rs->display, rs->window, &wdummy, &wdummy,
            &dummy, &dummy, &rx, &ry, &udummy )
         || rx < X_MARGIN || (rx >= X_MARGIN + w->common.display_width )
         || ry < Y_MARGIN || (ry >= Y_MARGIN + w->common.display_height) )
    {     /* locator is off screen or outside display area */
    rx = ( -1 );    /* special error indicator */
    ry = ( -1 );
    }
else
    {       /* in display area, so convert to coordinates */
    if ( rx < 0 || rx >= rs->output_ids_width
      || ry < 0 || ry >= rs->output_ids_height )
        {   /* locator is outside addressable area */
        rx = ( -1 );
        ry = ( -1 );
        }
    }

if ( rx == -1 || ry == -1 ||
     ( rx == rs->cs_x_rband && ry == rs->cs_y_rband ) ) return;
	/* Cursor out of the window, or no different 
	   coordinates than the last one. Skip draw.  */

/* Only Draw a rectangle to erase the current one if one has already 
 * been drawn.  If no rectangle 
 * */
if (  rs->cs_height_rband != 0 && rs->cs_width_rband != 0 )
    {
    XDrawRectangle (rs->display, rs->window, rs->cs_gc_rband, 
		    rs->cs_x_rband,      rs->cs_y_rband,
		    rs->cs_width_rband,  rs->cs_height_rband );
    }
else
    {
    XCopyGC( rs->display, rs->gc, GCPlaneMask | GCForeground | GCBackground,
	     rs->cs_gc_rband );
    }


/* Then figure out where the next one will be drawn */
hard_x  = MAP_X_ABSOLUTE( rs->x_soft_pos ); 
hard_y  = MAP_Y_ABSOLUTE( rs->y_soft_pos ); 
start_x = ( hard_x > rx) ? rx : hard_x;
start_y = ( hard_y > ry) ? ry : hard_y;

rs->cs_x_rband       = start_x;		/* Keep copies for future reference */
rs->cs_y_rband       = start_y;
rs->cs_height_rband  = abs( hard_y - ry );
rs->cs_width_rband   = abs( hard_x - rx );

XDrawRectangle (rs->display, rs->window, rs->cs_gc_rband,
		start_x,	    start_y,
		rs->cs_width_rband, rs->cs_height_rband );
}

void regis_erase_rband_rectangle()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Draw the last rubberband rectangle input cursor and cleanup.
    *
    **/
struct regis_cntx 
		*rs = RSTRUCT;

    XDrawRectangle (rs->display, rs->window, rs->cs_gc_rband,
                    rs->cs_x_rband,      rs->cs_y_rband,
                    rs->cs_width_rband,  rs->cs_height_rband );

rs->cs_height_rband = rs->cs_width_rband = 0;

}


regis_erase_rband_expose()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Erase any rubberband input cursor on expose event.
    *
    * FORMAL PARAMETERS:	none
    *
    **/
struct regis_cntx 
		*rs = RSTRUCT;

if (  rs->cs_x_rband != -1 && rs->cs_y_rband != -1 &&
      rs->cursor_cleanup_handler != NULL )
    {
    (*rs->cursor_cleanup_handler)();
    }

}
/* These four routines are NULLed out until written. -DPD */

#define regis_motion_crosshairs		NULL;
#define regis_motion_rband_line 	NULL;
#define regis_erase_crosshairs		NULL;
#define regis_erase_rband_line	 	NULL;



cs_cursor_select()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the C(ursor) option of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    **/

    int		changed, input_cursor_select;

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    changed = FALSE;			/*  assume no change */
    input_cursor_select = FALSE;	/* Default.  */

    while ( TRUE )
    {
	switch (scan( "([)I0123456" ))
	{
	    case -2 : 			/* Quotation */
		changed = TRUE;
		rs->cs_alphabet = rs->tx_alphabet;
		rs->cs_index = fetch_one_character( rs->character );
		if (rs->tx_alphabet != 0)
		{
		    rs->cs_index = CH_FOLD( rs->cs_index );
		}
		rs->cs_x_size = rs->tx_x_unit_size;
		rs->cs_y_size = rs->tx_y_unit_size;
		break;
	    case -3 : 		/* Unexpected alpha, right paren, synch */
	    case -1 :
		skp_option();   /* We should skip the output cursor command */
				/* or else the "H" is treated as Hard-copy  */
				/* command.				    */
		break;
	    case  3 :
		if (IS_TRUE(changed))
		{
		GSET_OUTPUT_CURSOR(rs->cs_alphabet,rs->cs_index,rs->cs_x_size,
		    rs->cs_y_size,(rs->cs_x_size>>1),(rs->cs_y_size>>1));
					 /* shift rights */
		}
		rs->first_process_me = TRUE;
		return;
	    case  0 :			/* Unexpected non-alpha */
	    {
		int    valu;
		int    rel;

		rs->first_process_me = TRUE;

		if (IS_TRUE(number (&valu, &rel, 0) ))
		{
		    changed = TRUE;
		    if (valu == 0)
		    {
			rs->cs_index = G18_NO_CURSOR;
		    }
		    else
		    {
			rs->cs_index = G19_DEFAULT_CURSOR;
		    }
		    rs->cs_alphabet = G17_SPECIAL_ALPHABET;
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 : 			/* Left paren */
		skp_option();
		break;
	    case  2 : 			/* [ */
		skp_coord();
		break;
	    case  4 :			/* I */
		input_cursor_select = TRUE;
		/* The default is to use crosshairs.
		** Below is the code used to define input cursor types based
		** tested responses of a VT340.
		** ( Fall through to next case )
		*/
	    case  5 :			/* 0 */
	    case  7 :			/* 2 */
	    case 10 :			/* 5 */
		if ( input_cursor_select == TRUE )
			{
			rs->cursor_motion_handler  = regis_motion_crosshairs;
			rs->cursor_cleanup_handler = regis_erase_crosshairs;
			}
		break;
	    case  6 :			/* 1 */
	    case 11 :			/* 6 */
		if ( input_cursor_select == TRUE )
		    {	/* Set ReGIS cursor */
		    rs->cursor_motion_handler  = NULL;
		    rs->cursor_cleanup_handler = NULL;
		    }
		break;

	    case  8 :			/* 3 */
		if ( input_cursor_select == TRUE )
		    {
		    rs->cursor_motion_handler  = regis_motion_rband_line;
		    rs->cursor_cleanup_handler = regis_erase_rband_line;
		    }
		break;

	    case  9 :			/* 4 */
		if ( input_cursor_select == TRUE )
		    {
		    rs->cursor_motion_handler  = regis_motion_rband_rectangle;
		    rs->cursor_cleanup_handler = regis_erase_rband_rectangle;
		    }
		break;

	}
    }		    /*  of while TRUE */
}		    /*  of routine CURSOR_SELECT */


data_shift()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the D(ata shift toggle) option of the S(creen)
    *   instruction.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    *
    * NOTE:  Since S(D1) is the only mode implemented so far, we ignore
    *   S(D1) and report S(D0) as an error.
    **/

	int	valu;
	int	rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE( number( &valu, &rel, 0 ) ))
    {
	if (valu != 1)
	{
	    ERROR( UNEXPECTED_CHARACTER, rs->character );
	}
    }
    else
    {
	rs->first_process_me = TRUE;
    }
}

fill_flood_instruction()

    /**
    * FUNCTIONAL DESCRIPTION:
    *	Handle the F (fill/flood) instruction.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:	none
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:	none
    *
    * SIDE EFFECTS:		none
    *
    **/

{
    struct regis_cntx *rs = RSTRUCT;

    for ( ;; )
	{
	switch( scan( "([" ) )
	    {

	    case -3:		/* synch */
	    case -1:		/* unexpected alpha */
		rs->first_process_me = TRUE;
		return;

	    case -2:		/* quote or double quote */
		skp_quote( rs->character );
		break;

	    case 0:		/* non-alpha */
		ignore_ch();
		break;

	    case 1:		/* open paren '(' */
		fill_flood_option();
		break;

	    case 2:		/* open bracket '[' */
		skp_coord();
		break;

	    }
	}
}

fill_flood_option()

    /**
    * FUNCTIONAL DESCRIPTION:
    *	Handle the options of the F (fill/flood) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:	none
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:	none
    *
    * SIDE EFFECTS:
    *	Sets sh_mode to WANT_POLYGON_FILL
    *	draw_line will set sh_mode to POLYGON_FILL after the first
    *	point - done this way because GIDIS includes the current
    *	position in the polygon, but ReGIS does not.
    *
    **/

{
    int
	saved_sh_mode,
	saved_sh_x,
	saved_sh_y;
    struct regis_cntx *rs = RSTRUCT;

/* hack: always save state so sh_mode will be restored correctly */

    rs->tmp_write_flag = TRUE;	/*set flag... */
    save_state();		/*...and save state */

    saved_sh_mode = rs->sh_mode;
    saved_sh_x = rs->sh_x;
    saved_sh_y = rs->sh_y;

    rs->sh_mode = WANT_POLYGON_FILL;
    rs->sh_x = rs->gid_x;
    rs->sh_y = rs->gid_y;

    for ( ;; )
	{
	switch( scan( "([CVW)P" ) )
	    {

	    case -3:		/* sync */
		if ( rs->sh_mode == POLYGON_FILL )
		    G127_SET_FILL_OFF();
		rs->gid_x = rs->sh_x;
		rs->gid_y = rs->sh_y;
#if SEPARATE_CURRENT_POSITION
		rs->gid_xy_valid = TRUE;
#endif
		G56_SET_POSITION( rs->gid_x, rs->gid_y );
		rs->sh_mode = saved_sh_mode;
		rs->sh_x = saved_sh_x;
		rs->sh_y = saved_sh_y;
		rs->first_process_me = TRUE;
		return;

	    case -2:		/* quote or double quote */
		skp_quote( rs->character );
		break;

	    case -1:		/* unexpected alpha */
		skp_option();
		break;

	    case 0:		/* unexpected non-alpha */
		ignore_ch();
		break;

	    case 1:		/* left paren */
		skp_paren();
		break;

	    case 2:		/* left bracket */
		skp_coord();
		break;

	    case 3:		/* C */
		crv_instruction();
		break;

	    case 4:		/* V */
		pos_instruction_vector( TRUE );
		break;

	    case 5:		/* W */
		temp_write_options();
		break;

	    case 6:		/* right paren */
		if ( rs->sh_mode == POLYGON_FILL )
		    G65_END_FILLED_FIGURE();
		rs->gid_x = rs->sh_x;
		rs->gid_y = rs->sh_y;
#if SEPARATE_CURRENT_POSITION
		rs->gid_xy_valid = TRUE;
#endif
		G56_SET_POSITION( rs->gid_x, rs->gid_y );
		rs->sh_mode = saved_sh_mode;
		rs->sh_x = saved_sh_x;
		rs->sh_y = saved_sh_y;
		return;

	    case 7:		/* P */
		pos_instruction_vector( FALSE );
		break;
	    }
	}
}
		    
#if DIRECT_GID_ENABLED
gid_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the G(idis) instruction (direct entry of GIDIS
    *   opcodes and parameters for GIDIS debugging and testing).
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		password;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    password = 0;

    while ( TRUE )
    {
	switch (scan( "([" ))
	{
	    case -3 : 			/* Synch, unexpected alpha */
	    case -1 :
		rs->sc_current_opcode = 0;				/* 2 */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :			/* Unexpected non-alpha - number? */
	    {
		int    valu;
		int    rel;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number( &valu, &rel, 0 ) ))
		{
		    if (password == GID_PASSWORD)
		    {		    
			rgid_process( &valu, 1 );
		    }
		    else
		    {
			password = valu;			      /* 2 */
				/*  prevent unwanted flush_buffers */ /* 2 */
			rs->sc_current_opcode = (-1);
		    }
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		skp_paren();
		break;
	    case  2 :
		skp_coord();
	}
    }
}
#endif


hardcopy()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the H(ardcopy) option of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:
    *   hd_x_offset and hd_y_offset contain the sticky default
    *   hardcopy offsets.
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Calls G140_PRINT_SCREEN.
    **/

    int		crds_seen;
    int		x1;
    int		x2;
    int		y1;
    int		y2;

    int	left;
    int	htop;
    int	width;
    int	height;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    crds_seen = 0;		/* Haven't seen any coordinates yet */
    x1 = 0;				/* ...and... */
    x2 = 32767;				/* ...the... */
    y1 = 0;				/* ...full... */
    y2 = 32767;				/* ...screen */

    while ( TRUE )
    {
	switch (scan( "[()" ))
	{
	    case -3 : 			/* Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -1 : 			/* Unexpected alpha, ) */
	    case 3:
	    {

		width = x2 - x1;
		if (width < 0)
		{
		    left = x2;
		    width = (-width);
		}
		else
		{
		    left = x1;
		}
		height = y2 - y1;
		if (height < 0)
		{
		    htop = y2;
		    height = (-height);
		}
		else
		{
		    htop = y1;
		}
		G140_PRINT_SCREEN(left, htop, width, height, rs->hd_x_offset, rs->hd_y_offset);
		rs->first_process_me = TRUE;
		return;
	    }
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 : 			/* [ */
		if (IS_TRUE( coordinates() ))
		{
		    crds_seen++;

		    switch ( crds_seen )
		    {
			case  1 :
			    x1 = rs->x_crd;	/* Save returned values */
			    y1 = rs->y_crd;
#if SEPARATE_CURRENT_POSITION 
			    if (IS_FALSE(rs->gid_xy_valid))
			    {
				rpg_request_gidi_pos();
			    }
#endif
			    x2 = rs->gid_x;	/* Second pair defaults to rel 0 */
			    y2 = rs->gid_y;
			    break;		/* Exit switch (coord... */
			case  2 :
			    x2 = rs->x_crd;	/* Save returned values  */
			    y2 = rs->y_crd;
			    break;		/* Exit switch (coord... */
			default :
			    ERROR( EXTRA_OPTION_COORDINATES, 0 );
		    }
		}
		break;			/* Exit switch scan		*/
	    case  2 :
		h2_hardcopy_suboption();	/* Go look for S(H(P 		*/
	}
    }
}

h2_hardcopy_suboption()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the H(ardcopy) option of the
    *   S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		p_seen;
    int		rel;
    int		x;
    int		y;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    p_seen = FALSE;			/* Haven't seen "P" yet */

    while ( TRUE )
    {
	switch (scan( "(P[)" ))
	{
	    case -3 : 			/* Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :
		p_seen = FALSE;
		skp_option();		/* Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/* Left paren */
		break;
	    case  2 :
		p_seen = TRUE;		/* P */
		break;
	    case  3 :
		if (IS_TRUE( p_seen )) 	/*  [ */
		{
		    if (IS_TRUE( bracketed_pair( &x, &y, &rel ) ))
		    {
			p_seen = FALSE;

			if (x >= 0)
			{
			    rs->hd_x_offset = x;
			}
			if (y >= 0)
			{
			    rs->hd_y_offset = y;
			}
		    }
		}
		else
		{
		    ignore_ch();
		}
		break;
	    case  4 :
		return;			/* Right paren */
	}
    }
}

load1_action( load_character )
int	load_character;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Parse hex characters and generate GIDIS code to load a
    *   character
    *
    * FORMAL PARAMETERS:
    *   load_character	= value of the ASCII code of the character
    *				to be loaded
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Calls load_alphabet
    **/

#if REVERSE_PATTERN_REGISTER == TRUE

/* This declaration and definition should be bound (it can be stored in ROM) */
/* and it should be an array of bytes when possible.			     */

    static int	backward_hex_table[] =
{0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF};

/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
/* backward_hex_table[] is defined as the backwards binary representation of */
/* the 16 hex decimals and are positioned in the lowest order nibble.	     */

#endif

    int		done;
    int		enough_nibbles;
    int		nibble;
    int         pixel_table[MAX1_ALPH_WIDE_CELL_HEIGHT];
    int		scan_index;
    int		scan_line;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /**
    * The ENOUGH_NIBBLES facility handles the case in which a comma is
    * encountered even though there were enough hex characters to complete
    * the scan line.  ENOUGH_NIBBLES = TRUE means that the last scan line
    * had its full complement of hex characters, and that an immediately
    * following comma should therefore be ignored. ENOUGH_NIBBLES = FALSE
    * indicates any other condition - no scan lines seen yet, scan line in
    * progress, or last scan line terminated with a comma (including the
    * case in which the last scan line terminated with an ignored comma) -
    * and that a comma is therefore significant.
    **/

    enough_nibbles = FALSE;		/* A comma now is significant - */
    					/*  no scan lines seen yet */
    done = FALSE;

				/* Erase the character table */
    for (scan_line = 0; scan_line < rs->max1_alph_cell_height; scan_line++)
    {
	pixel_table[scan_line] = 0;
    }
    scan_line = 0;

    while ( TRUE )		/* Outer loop on scan_line */
    {				/* Note that this loop will continue for */
				/* more than 16 scan lines, but will not */
				/* store any hex characters. */
	nibble = 0;

	while ( TRUE ) 			/* Loop on nibbles */
	{
	    int	exit;

	    exit = FALSE;
	    switch (scan_index = scan( "([0123456789ABCDEF," ))
	    {
		case -3 :
		case -2 :
		case -1 :
		case  1 :
		case  2 : 	/* Sync, quotation, unexpected alpha,..left paren, [ */
		    exit = TRUE;
		    done = TRUE;
		    break;
		case  0 :
		    ignore_ch();		/* Unexpected non-alpha */
		    break;
		case  3 :
		case  4 :
		case  5 :
		case  6 :
		case  7 :
		case  8 :
		case  9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
		case 17 :
		case 18 : 		/* Hex digit */
		    			/* Change the "- 3" below if the case */
		    			/*  label is other than "[3 to 18]" */
		    if (scan_line < rs->max1_alph_cell_height)
		    {			/* Make sure we're still interested */
				int    temp;

			temp = pixel_table[scan_line];
#if REVERSE_PATTERN_REGISTER == TRUE

			temp = (((temp >> 4) & 0xFFF) |
				(backward_hex_table[scan_index - 3] << 12));
				/* Slide old nibbles right 4 bits, and mask  */
				/*  off high bits ( bit 12 -> ... ), and     */
				/*  move in reversed bits of this hex        */
				/*  character.  NOTE:  This line should be   */
				/*  changed if the case label above is other */
				/*  than "[3 to 18]". 			     */
#else
			temp = ((temp << 4) | (scan_index - 3));
				/* Slide old nibbles left 4 bits and move in */
				/*  this hex character.  NOTE:  This line    */
				/*  should be changed if the case label above*/
				/*  is other than "[3 to 18]" 		     */
#endif

			pixel_table[scan_line] = temp;
			nibble++;
#if VARIABLE_CELL_STORAGE_SIZE == TRUE
			if (nibble > (rs->al_width[rs->ld_alphabet] - 1)/4)
#else
			if (nibble > (rs->max2_alph_cell_width - 1)/4)
#endif					/* Enough to fill this scan line? */
		        {		/* Yes - set flag to ignore next comma */
			    enough_nibbles = TRUE;
		    	    exit = TRUE; /* And go to next scan line */
			}
		    }
		    if (exit == FALSE)
		    {
			enough_nibbles = FALSE;	/* A comma now is significant - */
		    }    			/*  scan line in progress */
		    break;
		case 19 : 		/* Comma - shall we skip it? */
		    if ( IS_TRUE( enough_nibbles ) )
		    {
			enough_nibbles = FALSE;	/* Yes - don't skip the next one */
		    }
		    else
		    {
			exit = TRUE;	/* No - go increment scan line */
		    }
	    }
	    if ( exit )
	    {
		break;
	    }
	}
#if REVERSE_PATTERN_REGISTER == TRUE
	if ( scan_line < rs->max1_alph_cell_height )
	{ 			/* Allow for cell widths of less than 16 */
	    pixel_table[scan_line] = pixel_table[scan_line]
#if VARIABLE_CELL_STORAGE_SIZE == TRUE
	    >> (16 - rs->al_width[rs->ld_alphabet]);	/* shift right */
#else
	    >> (16 - rs->max2_alph_cell_width);	/* shift right */
#endif
	}
#endif

	if ( done )
	{
	    break;
	}
	else 
	{
	    scan_line++;	/* New scan line */
	}
    }
    if ( rs->ld_alphabet != 0 )
    {
	load_alphabet( CH_FOLD( load_character ), pixel_table,

#if VARIABLE_CELL_STORAGE_SIZE == TRUE
	    rs->al_width[rs->ld_alphabet], rs->al_height[rs->ld_alphabet]);
#else
	    rs->max2_alph_cell_width, rs->max1_alph_cell_height);
#endif
    }
    rs->first_process_me = TRUE;
}

load_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the L(oad) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issues G67_CREATE_ALPHABET.
    **/

    int	load_character;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {
	switch (scan( "([" ))
	{
	    case -3 : 			/* Sync, unexpected alpha */
	    case -1 :
		rs->first_process_me = TRUE;
		return;
	    case -2 :			/* Quotation */
		load_character = fetch_one_character( rs->character );
		load1_action( load_character );
		break;
	    case  0 :
		ignore_ch();		/* Unexpected non-alpha */
		break;
	    case  1 :
		load_option();		/* Left paren */
		break;
	    case  2 :			/* [ */
#if VARIABLE_CELL_STORAGE_SIZE == TRUE
	    {
		int	    cell_height;
		int	    cell_width;
			int	    relflags;

		bracketed_pair( &cell_width, &cell_height, &relflags );
		if ((rs->ld_alphabet == 0) || (cell_height > rs->max1_alph_cell_height)
		    || (cell_width > rs->max2_alph_cell_width) || (cell_width < 0)
		    || (cell_height < 0))
		{
		    ERROR( ATTRIBUTE_ALPHABET_ERROR, 0 );
		}
		else
		{
		    if ( cell_width < W_ALPH_CELL_WIDTH_DEFAULT )
		        cell_width = W_ALPH_CELL_WIDTH_DEFAULT;
		    if ( cell_height < H_ALPH_CELL_HEIGHT_DEFAULT )
		        cell_height = H_ALPH_CELL_HEIGHT_DEFAULT;
		    rs->al_width[rs->ld_alphabet] = cell_width;
		    rs->al_height[rs->ld_alphabet] = cell_height;
		    if (rs->ld_alphabet != rs->gid_alphabet)
		    {
		      rs->gid_alphabet = rs->ld_alphabet;
		      G66_SET_ALPHABET( rs->ld_alphabet );
		    }
G67_CREATE_ALPHABET( cell_width, cell_height, rs->al_extent[rs->ld_alphabet], 0 );
		}
	    }
#else
		ERROR( UNEXPECTED_CHARACTER, rs->character );
		skp_coord();
#endif

	}
    }
}

load_option()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the options of the L(oad) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issues G67_CREATE_ALPHABET.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {
	switch (scan( "([AE)" ))
	{
	    case -3 : 			/* Sync */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :
		skp_option();		/* Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/* Unexpected non-alpha */
		break;
	    case  1 :
		skp_paren();		/* Left paren */
		break;
	    case  2 :
		skp_coord();		/* [ */
		break;
	    case  3 :
		alphabet();		/* A */
		break;
	    case  4 : 			/* E */
#if VARIABLE_CELL_STORAGE_SIZE == TRUE
	    {
		int	    extent;
		int	    rel;

		number (&extent, &rel, 0);

		if ((rs->ld_alphabet == 0) || (extent > MAX3_ALPH_EXTENT))
		{		
		    ERROR( ATTRIBUTE_ALPHABET_ERROR, 0 );
		}
		else
		{
		    rs->al_extent[rs->ld_alphabet] = extent;
		    if (rs->ld_alphabet != rs->gid_alphabet)
		    {
		      rs->gid_alphabet = rs->ld_alphabet;
		      G66_SET_ALPHABET(rs->ld_alphabet);
		    }
G67_CREATE_ALPHABET( rs->al_width[rs->ld_alphabet], rs->al_height[rs->ld_alphabet], extent, 0 );
		}
	    }
#else
		ERROR( UNEXPECTED_CHARACTER, rs->character );
		skp_option();
#endif
		break;
	    case  5 :
		return;			/* Right paren */
	}
    }
}

new_regis()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Perform initialization for the parser and for the graphics
    *   machine as a whole.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

#if VARIABLE_CELL_STORAGE_SIZE == TRUE
    {
    register int i;

    for ( i = 0; i < TOTAL3_NUMBER_OF_ALPHABETS; i++ )
	{
	rstruct->al_height[i] = H_ALPH_CELL_HEIGHT_DEFAULT;
	rstruct->al_width[i]  = W_ALPH_CELL_WIDTH_DEFAULT;
	rstruct->al_extent[i] = ALPH_EXTENT_DEFAULT;
	}
    }
#endif

    rstruct->err_code = 0;
    rstruct->err_char = 0;
    rstruct->hd_x_offset = 0;
    rstruct->hd_y_offset = 0;
    rstruct->gid_alphabet = (-1);

    new_scanner();			/* Init the scanner */
/* don't call gid_new since it inits the whole screen */
#if 0
    gid_new();				/* Init the GIDIS processor */
#endif

    G3_INITIALIZE();
    G11_NEW_PICTURE();
    G93_SET_CELL_ALIGNMENT( 0, 0 );	/*  characters appear in upper left */
    G77_SET_CELL_MOVEMENT_MODE( G78_EXPLICIT_LOCAL );
    G116_SET_INPUT_TRIGGER( G119_TRIGGER_BUTTON, 0 );
    G127_SET_FILL_OFF();

    save_state();			/* In case we restore before saving, */
    					/*   restore something meaningful.   */
    save_text_state();

    rstruct->tmp_write_flag = FALSE;

    regis();				/* Now go into the main parser loop */
}

p1_pixel_vector()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle pixel vectors for Position, Screen, Vector instructions
    *   by converting to relative coordinates.  Take the pixel vector
    *   multiplier and logical pixel size into account.
    *
    * FORMAL PARAMETERS:
    *
    * IMPLICIT INPUTS:	a digit, 0 to 7, in character
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    *
    *	Note:	Screen orientation handling is not yet implemented.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (rs->character == '0' || rs->character == '1' || rs->character == '7')
    {
	rs->rel_x_coord = rs->pix_multiplier_vector;
    }
    if (rs->character == '2' || rs->character == '6')
    {
	rs->rel_x_coord = 0;
    }
    if (rs->character == '3' || rs->character == '4' || rs->character == '5')
    {
	rs->rel_x_coord = (-rs->pix_multiplier_vector);
    }
    if (rs->character == '5' || rs->character == '6' || rs->character == '7')
    {
	rs->rel_y_coord = rs->pix_multiplier_vector;
    }
    if (rs->character == '0' || rs->character == '4')
    {
	rs->rel_y_coord = 0;
    }
    if (rs->character == '1' || rs->character == '2' || rs->character == '3')
    {
	rs->rel_y_coord = (-rs->pix_multiplier_vector);
    }

    rs->coord_relflags = XRELATIVE | YRELATIVE;	/* Set both relative flags */

#if SEPARATE_CURRENT_POSITION 
    if (IS_FALSE( rs->gid_xy_valid ))
    {
	rpg_request_gidi_pos();
    }
#endif
    rs->x_crd = rs->rel_x_coord + rs->gid_x;
    rs->y_crd = rs->rel_y_coord + rs->gid_y;
}

pos_instruction_vector( vector_flag )
int	vector_flag;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle P(osition) and V(ector) instructions
    *
    * FORMAL PARAMETERS:
    *   vector_flag =	value	0 = P(osition) instruction
    *				1 = V(ector) instruction
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issues the GIDIS opcodes G49_SET_LINE_TEXTURE, draw_line,
    *   and\or G56_SET_POSITION.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    if (IS_TRUE( vector_flag )		/* On new V instruction... */
      && ( (rs->pt_register & 0xffff) != 0xffff))
					/* ...with non-default pattern... */
    {					/* ...reinit pattern register */
	G49_SET_LINE_TEXTURE( 16, rs->pt_register, rs->pt_multiplier );
    }
    while ( TRUE )
    {
	switch (scan( "(01234567[)" ))
	{
	    case -3 :
	    case -1 : 			/* Sync, unexpected alpha */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :
		pos_option_vector( vector_flag );	/* Left paren */
		break;
	    case  2 :
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
		p1_pixel_vector();

		if (IS_TRUE( vector_flag ))
		{
		    draw_line( rs->x_crd, rs->y_crd );
		}
		else
		{
		    G56_SET_POSITION( rs->x_crd, rs->y_crd );
		}
		rs->gid_x = rs->x_crd;
		rs->gid_y = rs->y_crd;
#if SEPARATE_CURRENT_POSITION 
		rs->gid_xy_valid = TRUE;
#endif
		break;
	    case 10 :			/* [ */
		if (IS_TRUE(coordinates() ))
		{
		    if (IS_TRUE( vector_flag ))
		    {
			draw_line( rs->x_crd, rs->y_crd );
		    }
		    else
		    {
			G56_SET_POSITION( rs->x_crd, rs->y_crd );
		    }
		    rs->gid_x = rs->x_crd;
		    rs->gid_y = rs->y_crd;
#if SEPARATE_CURRENT_POSITION 
		    rs->gid_xy_valid = TRUE;
#endif
		}
		break;
	    case 11:		/* right paren */
		if ( rs->sh_mode >= WANT_POLYGON_FILL )
		    {
		    rs->first_process_me = TRUE;
		    return;
		    }
		else
		    ignore_ch();
		break;
	}
    }

}

pos_option_vector( vector_flag )
int	vector_flag;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the options of the P(osition) and V(ector) instructions
    *
    * FORMAL PARAMETERS:
    *   vector_flag =	value	0 = P(osition) instruction
    *				1 = V(ector) instruction
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   pushes arguments onto the position stack
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
	switch (scan( "(BESW)[" ))
	{
	    case -3 :			/* Sync */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :
		skp_option();		/* Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/* Left paren */
		break;
	    case  2 : 			/* B */

#if VARIANT == 1 
		Put_String( (" [P(B) count "), PutChar);
		PutDecimal( rs->ps_count, PutChar );
		Put_String( ("]\n"), PutChar );
		PutBreak();
#endif
		if (rs->ps_count < POSITION_STACK_SIZE)
		{
		    rs->ps_bits |= (1 << rs->ps_count);
#if SEPARATE_CURRENT_POSITION 
		    if ( IS_FALSE( rs->gid_xy_valid ) )
		    {
			rpg_request_gidi_pos();
		    }
#endif
		    rs->ps_x[rs->ps_count] = rs->gid_x;
		    rs->ps_y[rs->ps_count] = rs->gid_y;
		    rs->ps_count++;
		}
		else
		{
		    ERROR( BEGIN1_START_OVERFLOW, rs->character );
		}
		break;
	    case  3 : 			/* E */

#if VARIANT == 1 
		Put_String( (" [P(E) count " ), PutChar );
		PutDecimal( rs->ps_count, PutChar );
		Put_String( ("] "), PutChar );
		PutBreak();
#endif
		if (rs->ps_count > 0)
		{
		    rs->ps_count--;

		    if (IS_TRUE( rs->ps_bits & (1 << rs->ps_count) ))   /*  (E)nding a (B) */
		    {			/*   move or draw to point (B) */

			if (IS_TRUE( vector_flag ))
			{
			    draw_line( rs->ps_x[rs->ps_count], rs->ps_y[rs->ps_count] );
			}
			else
			{
			    G56_SET_POSITION( rs->ps_x[rs->ps_count], rs->ps_y[rs->ps_count] );
			}
			rs->gid_x = rs->ps_x[rs->ps_count];
			rs->gid_y = rs->ps_y[rs->ps_count];
#if SEPARATE_CURRENT_POSITION 
			rs->gid_xy_valid = TRUE;
#endif
		    }		/*  if (E)nding a (S) */
				/*  do nothing */
		}
		else
		{
		    ERROR( BEGIN2_START_UNDERFLOW, rs->character );
		}
		break;
	    case  4 : 			/*  S */

		if (rs->ps_count < POSITION_STACK_SIZE)
		{
		    rs->ps_bits &= ~(1 << rs->ps_count++);
		}
		else
		{
		    ERROR( BEGIN1_START_OVERFLOW, rs->character );
		}
		break;
	    case  5 : 			/* W */
		temp_write_options();
		break;
	    case  6 :
		return;			/* Right paren */
	    case  7 :
		skp_coord();		/* [ */
	}
    }
}

regis()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Instruction state - look for an instruction and dispatch to
    *   the appropriate routine to handle it.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none - main routine
    *
    * SIDE EFFECTS:
    *   Issues restore_state.
    **/

    rstruct->ps_count = 0;
    rstruct->first_process_me = FALSE;

    rst_scanner();

    if ( rstruct->sh_mode >= WANT_POLYGON_FILL )
	{
	if ( rstruct->sh_mode == POLYGON_FILL )
	    G127_SET_FILL_OFF();
	rstruct->sh_mode = SHADE_OFF;
	rstruct->gid_x = rstruct->sh_x;
	rstruct->gid_y = rstruct->sh_y;
#if SEPARATE_CURRENT_POSITION
	rstruct->gid_xy_valid = TRUE;
#endif
	G56_SET_POSITION( rstruct->gid_x, rstruct->gid_y );
	}

    while ( TRUE )
    {
	if (IS_TRUE( rstruct->tmp_write_flag ))
					/* Are we ending temp write options? */
	{				/* Yes - do so */

	    restore_state();
	    rstruct->tmp_write_flag = FALSE;
	}
	switch (scan( "([CGLPRSTVWF" ))
	{
	    case -3 :

		sync();
		break;
	    case -2 :

		skp_quote( rstruct->character );
		break;
	    case -1 :
	    case  0 :

		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :

		skp_paren();
		break;
	    case  2 :

		skp_coord();		/* [ */
		break;
	    case  3 :

		crv_instruction();
		break;
	    case  4 :
#if DIRECT_GID_ENABLED		/* G - enabled? */

		gid_instruction();
#else

		ignore_ch();
#endif
		break;
	    case  5 : 

		load_instruction();
		break;
	    case  6 : 			/* Position */

		pos_instruction_vector( FALSE );
		break;
	    case  7 : 			/* R */
#if OVERLAY_STYLE == 1 || OVERLAY_STYLE == 2	/* VT240-style overlays */

	    	map_rep_inst();
#else

	    	ri_rprt_instruction();
#endif
		break;
	    case  8 : 

		scrn_instruction();
		break;
	    case  9 : 			/* T */
#if OVERLAY_STYLE == 1 || OVERLAY_STYLE == 2	/* VT240-style overlays */

	    	map_text_inst();
#else

	    	text_instruction();
#endif
		break;
	    case 10 : 			/* Vector */

		pos_instruction_vector( TRUE );
		break;
	    case 11 : 

		wi_write_instruction();
		break;

	    case 12:			/* Fill polygon */
		fill_flood_instruction ();
		break;
	}

   }

}

scrn1_addressing()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the A(ddressing) option of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		crds_seen;
    int		t1_temp_ulx_exp;
    int		t2_temp_ulx_mant;
    int		t3_temp_uly_exp;
    int		t4_temp_uly_mant;
    int		x_exponent;
    int		x_mantissa;
    int		y_exponent;
    int		y_mantissa;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    crds_seen = 0;		/* 0 = nothing yet */
					/* 1 = first coordinate pair seen */
					/* 2 = first coordinate pair OK */
					/* 3 = second coordinate pair seen */
					/* 4 = second coordinate pair OK */
					/* Note that this value must equal 4 */
					/* at exit time or the entire option */
					/* is treated as a no-op. */
    while ( TRUE )
    {
	switch (scan( "[()" ))
	{
	    case -3 :		/* Resynch */
	    case -1 : 		/* Unexpected alpha, ) */
	    case  3 :
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();	/* Nothing interesting */
		break;
	    case  1 :		/* [ */
	    {
		int	relflags;

if (IS_TRUE( x1_floating_bracketed_pair( &x_mantissa, &x_exponent, &y_mantissa, &y_exponent, &relflags, 0, 0 ) ))
		{
		    ++crds_seen;

		    switch ( crds_seen )
		    {
			case  1 :
			    t2_temp_ulx_mant = x_mantissa;
			    t1_temp_ulx_exp = x_exponent;
			    t4_temp_uly_mant = y_mantissa;
			    t3_temp_uly_exp = y_exponent;
			    crds_seen = 2;
			    break;
			case  3 :
			    scrn_action_addressing( t2_temp_ulx_mant,
				t1_temp_ulx_exp, t4_temp_uly_mant,
				t3_temp_uly_exp, x_mantissa, x_exponent,
				y_mantissa, y_exponent);
			    crds_seen = 4;
			    break;
			default  :
			    ERROR( EXTRA_OPTION_COORDINATES, 0 );
		    }
		}
		break;
	    }
	    case  2 :
		skp_paren();	/* Left paren */
	}
    }
}

scrn_action_addressing( ulx_mant, ulx_exp, uly_mant, uly_exp,	lrx_mant,
			  lrx_exp, lry_mant, lry_exp)
int	ulx_mant;
int	ulx_exp;
int	uly_mant;
int	uly_exp;
int	lrx_mant;
int	lrx_exp;
int	lry_mant;
int	lry_exp;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Perform GIDIS code generation for the A(ddressing) option of
    *   the S(creen) instruction
    *
    * FORMAL PARAMETERS:
    *   ulx_mant	= value of the mantissa of the x coordinate of
    *			  the upper left corner of the addressed area
    *   ulx_exp	= value of the exponent of the x coordinate of
    *			  the upper left corner of the addressed area
    *   uly_mant	= value of the mantissa of the y coordinate of
    *			  the upper left corner of the addressed area
    *   uly_exp	= value of the exponent of the y coordinate of
    *			  the upper left corner of the addressed area
    *   lrx_mant	= value of the mantissa of the x coordinate of
    *			  the lower right corner of the addressed area
    *   lrx_exp	= value of the exponent of the x coordinate of
    *			  the lower right corner of the addressed area
    *   lry_mant	= value of the mantissa of the y coordinate of
    *			  the lower right corner of the addressed area
    *   lry_exp	= value of the exponent of the y coordinate of
    *			  the lower right corner of the addressed area
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:	updates screen addressing-related module-wide
    *			variables and issues GIDIS screen addressing
    *			instruction
    **/

    int		dx_mantissa;
    int		dx_exponent;
    int		dy_mantissa;
    int		dy_exponent;
    int		tmp_x_extent;
    int		tmp_y_extent;
    int		tmp_10_power;
    int		tmp_2_power;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */


    /* *RFD* */
/**
#if VARIANT == 1
    PutString("BEGIN: scrn_action_addresssing,regis.c\n",PutChar);
#endif
**/
    tmp_10_power = 0;		/*  initialize  */
    tmp_2_power = 0;		

    /* Calculate x difference... */

    subtract_floating( lrx_mant, lrx_exp, ulx_mant, ulx_exp,
        &dx_mantissa, &dx_exponent );

    if (dx_mantissa < 0)
    {
        dx_mantissa = (-dx_mantissa);
        tmp_x_extent = (-1);
    }
    else
    {
	tmp_x_extent = 1;
    }
#if VARIANT == 1 
    Put_String( (" [tmp_x_extent = "), PutChar );
    PutDecimal( tmp_x_extent, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    /* Calculate y difference... */

    subtract_floating( lry_mant, lry_exp, uly_mant, uly_exp,
        &dy_mantissa, &dy_exponent );

    if (dy_mantissa < 0)
    {
        dy_mantissa = (-dy_mantissa);
        tmp_y_extent = (-1);
    }
    else
    {
	tmp_y_extent = 1;
    }
			/* Test for fast path */
    if ((dx_exponent == 0) && (dy_exponent == 0) 
        && (((dx_mantissa == X_HARDWARE_RESOLUTION - 1) && (dy_mantissa
        <= Y_HARDWARE_RESOLUTION - 1)) || ((dx_mantissa <=
        X_HARDWARE_RESOLUTION - 1) && (dy_mantissa ==
        Y_HARDWARE_RESOLUTION - 1))))
    {			/* Fast path - set these and done */

        dx_mantissa = X_HARDWARE_RESOLUTION;
        dy_mantissa = Y_HARDWARE_RESOLUTION;
    }
    else		/* Not fast path - we must scale */
    {

	int	ce_compare_exp;
	int	cm_compare_mant;	/* Subtract to see if x or y is larger */
        subtract_floating( dx_mantissa, dx_exponent, dy_mantissa,
    	 dy_exponent, &cm_compare_mant, &ce_compare_exp );

        if (cm_compare_mant >= 0)
	{		/* x is larger - save it */
	    cm_compare_mant = dx_mantissa;
	    ce_compare_exp = dx_exponent;
	}
        else
	{		/* y is larger - save it */
	    cm_compare_mant = dy_mantissa;
	    ce_compare_exp = dy_exponent;
	}
	if (cm_compare_mant == 0)
	{

	    return;	/*  no change if error */
	}
	while (cm_compare_mant < X_HARDWARE_RESOLUTION)
	{				/* Now to scale by powers of 10 */
	    cm_compare_mant *= 10;		/* compare_mant = compare_mant * 10; */
	    tmp_10_power++;
	}
	while (cm_compare_mant > 16384)
	{
	    cm_compare_mant /= 10;
	    tmp_10_power--;
	}
	tmp_10_power -= ce_compare_exp;
	dx_exponent += tmp_10_power;
	dy_exponent += tmp_10_power;	/* Normalize x and y */

	if (dx_mantissa > 10)
	{

	    dx_mantissa = convert_floating_to_integer( dx_mantissa + 1,
			dx_exponent );

	}
	else
	{

 	    dx_mantissa = convert_floating_to_integer( dx_mantissa, 
			dx_exponent ) + 1;

	}
	if (dy_mantissa > 10)
	{

	    dy_mantissa = convert_floating_to_integer( dy_mantissa + 1,
			dy_exponent );

	}
	else
	{

	    dy_mantissa = convert_floating_to_integer( dy_mantissa, 
			dy_exponent ) + 1;

	}
			/* end of slow (scaling) path, rejoin with fast path */
    }
    rs->sa_10_power = tmp_10_power;
    rs->sa_2_power = tmp_2_power;


    G28_SET_OUTPUT_IDS( dx_mantissa, dy_mantissa );
			/* Issue GIDIS screen addr instruction */

    rs->sa_x_extent = MULTIPLY_BY_SIGN(dx_mantissa, tmp_x_extent);
    rs->sa_y_extent = MULTIPLY_BY_SIGN(dy_mantissa, tmp_y_extent);

#if VARIANT == 1 
    Put_String( (" [sa_x_extent = "), PutChar );
    PutDecimal( rs->sa_x_extent, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    rs->sa_ulx = convert_floating_to_integer( ulx_mant, 
    					ulx_exp + rs->sa_10_power );
    rs->sa_ulx = MULTIPLY_BY_SIGN( rs->sa_ulx, rs->sa_x_extent );

    rs->sa_uly = convert_floating_to_integer( uly_mant, 
					uly_exp + rs->sa_10_power );
    rs->sa_uly = MULTIPLY_BY_SIGN( rs->sa_uly, rs->sa_y_extent );

    ucs_update_cell_standard();	/*  ask Gidis for def. of cell standard !2 */

}

scrn_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* *RFD* */
/**
#if VARIANT == 1
    PutString("BEGIN: scrn_instruction, regis.c\n",PutChar);
#endif
**/
    while ( TRUE )
    {
	switch (scan( "(01234567[" ))
	{
	    case -3 :
	    case -1 : 			/* Resynch, unexpected alpha */
		rs->first_process_me = TRUE;

		return;
	    case -2 :

		skp_quote( rs->character );
		break;
	    case  0 :

		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :

		scrn_option();	/* Left paren */
		break;
	    case  2 : 			/* Digits */
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :

		p1_pixel_vector();

		scroll_screen( rs->rel_x_coord, rs->rel_y_coord );
		break;
	    case 10 :			/* [ */
		if (IS_TRUE( rel_coordinates() ))
		{
		    scroll_screen( rs->rel_x_coord, rs->rel_y_coord );
		}
	}

    }
}

scrn_option()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the options of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
        switch (scan( "(ADEHIMSTW)C" ))
	{
	    case -3 :		/* Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :
		skp_option();	/* Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();	/* Nothing interesting */
		break;
	    case  1 :
		skp_paren();	/* Left paren */
		break;
	    case  2 :
		scrn1_addressing();	/* A */
		break;
	    case  3 :
		data_shift();	/* D */
		break;
	    case  4 :		/* E */
		if ( rs->sh_mode == POLYGON_FILL )
		    G127_SET_FILL_OFF();
		rs->sh_mode = SHADE_OFF;	/* Reset various drawing states */
		rs->ps_count = 0;	/* Reinit position stack */
		if ( (rs->pt_register & 0xffff) != 0xffff)
						/*  must reset pattern */
		{
		    G49_SET_LINE_TEXTURE( 16, rs->pt_register, rs->pt_multiplier );
		}
		G12_END_PICTURE();
		G11_NEW_PICTURE();
		break;
	    case  5 :
		hardcopy();	/* H */
		break;
	    case  6 :
		scrn_write_intensity( FALSE );	/* I */
		break;
	    case  7 : 			/* M */
#if OVERLAY_STYLE == 1 || OVERLAY_STYLE == 2	/* VT240-style overlays */

		map_cmap();
#else

		c_map();
#endif
		break;
	    case  8 : 			/* S */

		scrn_scale();
		break;
	    case  9 : 			/* T */

		timer();
		break;
	    case 10 : 			/* W */

		temp_write_options();
		break;
	    case 11 : 			/* Right paren */

		return;
	    case 12 :			/*  C */

		cs_cursor_select();
	}

    }

}

scrn_scale()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the S(scale) option of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    *
    *	Note:  To be implemented in VT200 by windowing
    *		Not implemented in CT100.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    ERROR( UNEXPECTED_CHARACTER, rs->character );
}

sync()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Implement the ; (Resynchronize) instruction.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    rs->err_code = 0;
    rs->err_char = 0;
}

timer()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the T(ime) option of the S(creen) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		valu;
    int		rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE( number( &valu, &rel, 0 ) ))
    {
	reg_wait( valu );
    }
    else
    {
	rs->first_process_me = TRUE;
    }
}

		/***  END OF FILE REGIS.C  ***/
/**
*	CMS REPLACEMENT HISTORY
**/
/**
*	2 A_VESPER 21-OCT-1983 16:29:00 "fixed G command bug"
*	1 A_VESPER 14-SEP-1983 14:01:22 ""
**/
