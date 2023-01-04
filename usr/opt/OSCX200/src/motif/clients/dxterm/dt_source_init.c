/* #module DT_source_init.c "X0.0" */
/*
 *  Title:	DT_source_init.c 
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
 *	Routines for initializing source portion of DECterms.
 *
 *  Author:	Mike Leibow
 *
 *  Modification history:
 *
 * Eric Osman		 4-Oct-1993	BL-E
 *	- Add DECTERM_SHOW_PARSING to optionally show characters as they
 *	  are parsed.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     03-Jun-1993     V1.2
 *	- Add initializations requested by Hebrew group
 *
 * Alfred von Campe     21-May-1993     V1.2
 *	- Remove unnecessary variable declarations
 *
 * Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Eric Osman		18-Dec-1990
 *	- Tanimoto-san, sumimasen.  WVT$INIT no XrmGetStringDatabase ga arimasu
 *	ga, XrmDestroyDatabase wa arimasen.  Simasita.  (I added missing
 *	XrmDestroyDatabase call in WVT$INIT).
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	Merge in Toshi Tanimoto's changse to support Asian terminals -
 *	- language switching mechanism
 *
 * Bob Messenger	31-May-1989	X2.0-13
 *	- Use memset instead of bzero.
 *
 * Bob Messenger	13-May-1989	X2.0-10
 *	- Disable saveErasedLines during startup, so we don't start out
 *	  with a screenful of blank lines in the transcript.
 *
 * Bob Messenger	1-Apr-1989	X2.0-5
 *	- Support transcriptSize
 *
 * Eric Osman		2-Sep-1988	BL9.2
 *	- Separate codes and renditions into independently allocated buffers.
 *
 * Mike Leibow		01-Aug-1988		X0.4-41
 *	- fixed bug in status line initialization code.
 * 	  rend pointer pointed to middle of code block.
 * Mike Leibow		some time between 18 and 31
 *	- added status line
 *
 * Tom Porcher		18-Jul-1988		X0.37
 *	- cast XtMalloc()s fro Ultrix.
 *
 * Tom Porcher		 8-Jul-1988		X0.35
 *	- replaced malloc with XtMalloc.  This is freed with XtFree.
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 */

#include "wv_hdr.h"

WVT$INIT(ld)
wvtp ld;
{
	long bytes, ic, len;
	int columns, rows, y;
	long *cp;
	DECtermWidget w = ld_to_w( ld );
	Boolean saveErasedLines;
	long int wvt$l_ext_flags = _cld wvt$l_ext_flags,
		 wvt$l_ext_specific_flags = _cld wvt$l_ext_specific_flags;

	saveErasedLines = w->common.saveErasedLines;
	w->common.saveErasedLines = False;

	/* clear ld->source structure. */
	memset(&ld->source, 0, sizeof(ld->source));

	_cld wvt$l_ext_flags = wvt$l_ext_flags;
	_cld wvt$l_ext_specific_flags = wvt$l_ext_specific_flags;

	/* allocate memory for display list */

	WVT$MAIN_DISPLAY(ld);
	xinit( ld, ld->common.columns, ld->common.rows,
		ld->common.transcriptSize );

	/* tell output the size of the display list */
	_ld wvt$l_transcript_top = 1;
	o_set_top_line(ld, 1);
	o_set_bottom_line(ld, ld->common.rows);
	_ld display_height = ld->common.rows;
	o_set_display_width( ld, ld->common.columns );
	_cld display_width = ld->common.columns;

	/* tell output where top line is */
	o_set_initial_line(ld, 1);

	/* allocate memory for status line display list */
	WVT$STATUS_DISPLAY(ld);

	xinit(ld, ld->common.columns, 1, 0 );

	xris(ld);

	w->common.saveErasedLines = saveErasedLines;

        _cld wvt$l_ext_specific_flags &= ~vte2_m_rtl;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_copy_dir;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map_pend_rqst;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_map_pend_toheb;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_not_first_reset;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_soft_switch;
        _cld wvt$l_ext_specific_flags &= ~vte2_m_kb_was_heb;

/*
 * See if user wants a log of parsed characters as we parse them.
 */
	{
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
	noshare
#endif
	extern char show_parsing_flag;
	if (getenv ("DECTERM_SHOW_PARSING"))
	    {
	    show_parsing_flag = 1;
	    printf (
"Saw symbol DECTERM_SHOW_PARSING, so parsed characters will be shown\n");
	    }
	}
}
