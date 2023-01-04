/*
*****************************************************************************
**                                                                          *
**               COPYRIGHT (c) 1988, 1989, 1991, 1992 BY		    *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**	PrintScreen
**
**  ABSTRACT:
**
**	Data structures for print screen customization
**
**  ENVIRONMENT:
**
**
**  MODIFICATION HISTORY:
**
**	29-Jan-1992	Edward P Luwish
**		Soft-code length of prtattr_di.capture_file
**
**	11-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
**--
**/

#include "smconstants.h"

#define	mapped	0
#define iconified 1

#define ismanaged 1
#define notmanaged 0

#define mratio		0x00000001
#define msaver		0x00000002
#define mcolor		0x00000004
#define mformat		0x00000008
#define mfile		0x00000010
#define mprtprompt	0x00000020
#define mrotateprompt	0x00000040
#define mdelay		0x00000080
#define mfit		0x00000100
#define mpagesize	0x00000200
#define msixelprinter	0x00000400
#define morientation	0x00000800
#define mcapture	0x00001000
#define msend		0x00002000

#define mscreen_applprompt	0x00000001
#define mscreen_prtprompt	0x00000002
#define mscreen_applscreennum	0x00000004
#define mscreen_prtscreennum	0x00000008
#define mscreen_mask		0x0000000f

extern  struct smattr_di
        {
        unsigned int	changed;
        Widget		sm_attr_id;
        Widget		state_id;
        unsigned int	startup_state;
        Widget		mapped_id;
        Widget		icon_id;
        Widget		header_id;
        char		*header_text;
        Widget		pause_id;
        char		*pause_text;
        Widget		history_id;
        unsigned int    history;
        unsigned int    cols;
        unsigned int    rows;
        unsigned int    x;
        unsigned int    y;
        unsigned int    end_confirm;
        unsigned int	end_confirm_id;
        char		managed;
        };

extern	struct prtattr_di
{
    unsigned int	changed;
    Widget		prt_attr_id;
    Widget		ratio_id;		/* obsolete */
    unsigned int	ratio;			/* obsolete */
    Widget		r1to1_id;		/* obsolete */
    Widget		r2to1_id;		/* obsolete */
    Widget		prtcolor_id;
    unsigned int	color;
    Widget		bw_id;
    Widget		grey_id;
    Widget		color_id;
    Widget		saver_id;
    unsigned int	saver;
    Widget		positive_id;
    Widget		negative_id;
    Widget		format_id;
    unsigned int	format;
    Widget		sixel_id;
    Widget		postscript_id;
    Widget		bitmap_id;
    Widget		capture_file_id;
    char		capture_file[MAXFILSTR];
    Widget		file_prompt_id;
    unsigned int	file_prompt;
    char		managed;
    Widget		rotate_prompt_id;	/* obsolete */
    unsigned int	rotate_prompt;		/* obsolete */
    unsigned int	fit;
    Widget		fit_id;
    Widget		rotate_id;
    Widget		grow_id;
    Widget		shrink_id;
    Widget		reduce_id;
    Widget		scale_id;
    Widget		crop_id;
    unsigned int	delay;
    Widget		delay_id;
    unsigned int	sixel_printer;
    Widget		sixel_box_id;
    Widget		sixelyorn_id;
    Widget		sixel_OK_id;
    Widget		sixel_Cancel_id;
    Widget		sixel_Help_id;
    unsigned int	page_size;
    Widget		page_size_box_id;
    Widget		pageyorn_id;
    Widget		page_size_OK_id;
    Widget		page_size_Cancel_id;
    Widget		page_size_Help_id;
    unsigned int	orientation;
    Widget		orient_box_id;
    Widget		best_id;
    Widget		portrait_id;
    Widget		landscape_id;
    Widget		letter_id;
    Widget		Bsize_id;
    Widget		Asize_id;
    Widget		A5size_id;
    Widget		A4size_id;
    Widget		A3size_id;
    Widget		B5size_id;
    Widget		B4size_id;
    Widget		ledger_id;
    Widget		legal_id;
    Widget		exec_id;
    Widget		vt_id;
    Widget		la50_id;
    Widget		la75_id;
    Widget		la100_id;
    Widget		la210_id;
    Widget		ln03_id;
    Widget		lj250_id;
    Widget		lj250lr_id;
    Widget		lcg01_id;
    Widget		menubar_id;
    Widget		blinker_id;
    unsigned int	capture_method;
    Widget		entire_id;
    Widget		partial_id;
    unsigned int	send_to_printer;
    unsigned int	print_widget;
    Widget		print_id;
    Widget		file_id;
    Widget		both_id;
};

extern struct screenattr_di
	{
	unsigned int	changed;
	Widget		scr_attr_id;
	Widget		appl_prompt_id;
	Widget		prt_prompt_id;
	Widget		*appl_scale_id;
	Widget		*prt_scale_id;
	int		appl_screennum;
	int		appl_prompt;
	int		prt_screennum;
	int		prt_prompt;
	char		managed;
	};

