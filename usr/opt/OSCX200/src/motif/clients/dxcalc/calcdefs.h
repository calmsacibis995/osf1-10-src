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
#ifdef OSF1
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calcdefs.h,v 1.1.5.2 1993/04/13 22:53:54 Ronald_Hegli Exp $";
#endif
#endif
/* Header from VMS source pool
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calcdefs.h,v 1.1.5.2 1993/04/13 22:53:54 Ronald_Hegli Exp $";
*/
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
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

**--
**/
/*
**++
**  MODULE NAME:
**	calcdefs.h
**
**  FACILITY:
**      OOTB Calculator
**
**  ABSTRACT:
**	Global Defines and typedefs for the calculator
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/

#define NULL 0

#ifdef VMS
#define DWTVMS
#else
#define DWTUNIX       
#endif

#define APPL_NAME		"Calc"
#ifdef VMS
#define CLASS_NAME		"DECW$CALC"
#else
#define CLASS_NAME		"DXcalc"
#endif

#ifndef NO_HYPERHELP
#ifdef VMS
#define CALC_HELP		"DECW$CALC"
#else
#define CALC_HELP		"DXcalc"
#endif
#endif

#define xrm_geometry		"geometry"
#define xrc_geometry		"Geometry"

#define DwtNkeyFontFamily	"keyFontFamily"
#define XtCKeyFontFamily	"KeyFontFamily"
#define DwtNsqrtFontFamily	"sqrtFontFamily"
#define XtCSqrtFontFamily	"SqrtFontFamily"

#define DwtNlanguage		"language"
#define XtCLanguage		"Language"
#define DwtNnumberFormat	"numberFormat"
#define XtCNumberFormat		"NumberFormat"

#define GeoMode(r) ((r)->request_mode)

#define	CalcOffsetPtr(w) \
(((CalcWidgetClass) ((CalcWidget) w)->core.widget_class)->calc_class.calcoffsets)

/* These defines allow easy access to the pieces of the widget structure */
#define CalcHierarchy(w) ((w)->calc.calc_hierarchy)
#define ContextPtr(w) ((w)->calc.context_ptr)
#define ArrayContextPtr(w) ((w)->calc.array_context_ptr)
#define HexArrayContextPtr(w) ((w)->calc.hex_array_context_ptr)
#define MessageWid(w) ((w)->calc.message_wid)

#define TopMenuBar(w) ((w)->calc.top_menu_bar)
#define Menu(w) ((w)->calc.menu)
#define HelpWidget(w) ((w)->calc.help_widget)

#define LanguageId(w) ((w)->calc.language_id)
#define Language(w) ((w)->calc.language)
#define NumberFormat(w) ((w)->calc.number_format)
#define CSNumberFormat(w) ((w)->calc.CSnumber_format)

#define UserDatabase(w) ((w)->calc.user_database)
#define MergedDatabase(w) ((w)->calc.merged_database)
#define SystemDatabase(w) ((w)->calc.system_database)
#define SystemDefaultsName(w) ((w)->calc.system_defaults_name)
#define DefaultsName(w) ((w)->calc.defaults_name)

#define MemBoundBox(w) ((w)->calc.mem_bound_box)
#define KeyBoundBox(w) ((w)->calc.key_bound_box)
#define ClearBoundBox(w) ((w)->calc.clear_bound_box)
#define OpsBoundBox(w) ((w)->calc.ops_bound_box)
#define FuncBoundBox(w) ((w)->calc.func_bound_box)
#define InvFuncBoundBox(w) ((w)->calc.inv_func_bound_box)

#define Disp(w) ((w)->calc.disp)
#define Memory(w) ((w)->calc.memory)
#define LogoWindow(w) ((w)->calc.logo_window)

#define Keys(w) ((w)->calc.keys)
#define Clears(w) ((w)->calc.clears)
#define Ops(w) ((w)->calc.ops)
#define Func(w) ((w)->calc.func)
#define InvFunc(w) ((w)->calc.inv_func)
#define Mems(w) ((w)->calc.mems)  

/* Changed "Font" to "CFont" because it overrides the Xlib typedef of Font. */
#define CFont(w) ((w)->calc.font)
#define CFonts(w) ((w)->calc.fonts)

#define SqrtFont(w) ((w)->calc.sqrt_font)
#define SqrtFonts(w) ((w)->calc.sqrt_fonts)
#define DisplayFontHeight(w) ((w)->calc.display_font_height)
#define FontHeights(w) ((w)->calc.font_heights)
#define FontWidths(w) ((w)->calc.font_widths)
#define SqrtFontHeights(w) ((w)->calc.sqrt_font_heights)
#define SqrtFontWidths(w) ((w)->calc.sqrt_font_widths)
#define NumFonts(w) ((w)->calc.num_fonts)
#define NumSqrtFonts(w) ((w)->calc.num_sqrt_fonts)
#define MemoryFontHeight(w) ((w)->calc.memory_font_height)
#define KeyFontHeight(w) ((w)->calc.key_font_height)
#define SqrtFontHeight(w) ((w)->calc.sqrt_font_height)

#define ErrorString(w) ((w)->calc.error_string)
#define ZeroPoint(w) ((w)->calc.zero_point)
#define AppName(w) ((w)->calc.app_name)
#define ChrPoint(w) ((w)->calc.chr_point)
#define ChrZero(w) ((w)->calc.chr_zero)
#define SqrtKey(w) ((w)->calc.sqrt_key)
#define PiKey(w) ((w)->calc.pi_key)

#define MemFgpixel(w) ((w)->calc.mem_fgpixel)
#define MemBgpixel(w) ((w)->calc.mem_bgpixel)
#define DispFgpixel(w) ((w)->calc.disp_fgpixel)
#define DispBgpixel(w) ((w)->calc.disp_bgpixel)
#define CalcGC(w) ((w)->calc.calc_gc)

#define CalcTopShadowGC(cw,o) \
    XmField(cw,o,XmManager,top_shadow_GC,GC)
#define CalcBottomShadowGC(cw,o) \
    XmField(cw,o,XmManager,bottom_shadow_GC,GC)
#define CalcShadowThickness(cw,o) \
    XmField(cw,o,XmManager,shadow_thickness,Dimension)

#define CalcClearGC(w) ((w)->calc.calc_clear_gc)
#define LogoGC(w) ((w)->calc.logo_gc)
#define SqrtGC(w) ((w)->calc.sqrt_gc)
#define SqrtClearGC(w) ((w)->calc.sqrt_clear_gc)
#define LogoPixmap(w) ((w)->calc.logo_pixmap)
#define PDUndoButtonWid(w) ((w)->calc.p_d_undo_button_wid)
#define PUUndoButtonWid(w) ((w)->calc.p_u_undo_button_wid)
#define Dpy(w) ((w)->calc.dpy)

#define UndoEnabled(w) ((w)->calc.undo_enabled)
#define ButtonPressed(w) ((w)->calc.button_pressed)
#define KeyPressed(w) ((w)->calc.key_pressed)
#define MemoryFlag(w) ((w)->calc.memory_flag)
#define Strmax(w) ((w)->calc.strmax)
#define DisplaySelected(w) ((w)->calc.display_selected)
#define NumStackCount(w) ((w)->calc.num_stack_count)
#define BufferCount(w) ((w)->calc.buffer_count)
#define CurrentOp(w) ((w)->calc.current_op)
#define Buffer(w) ((w)->calc.buffer)
#define BufferCopy(w) ((w)->calc.buffer_copy)
#define NumStack(w) ((w)->calc.num_stack)
#define ErrorFlag(w) ((w)->calc.error_flag)
#define MemErrorFlag(w) ((w)->calc.mem_error_flag)
#define MemValue(w) ((w)->calc.mem_value)
#define Numstr(w) ((w)->calc.numstr)
#define Mode(w) ((w)->calc.mode)
#define InputFormatStr(w) ((w)->calc.input_format_str)
#define OutputFormatStr(w) ((w)->calc.output_format_str)

#define TrigType(w) ((w)->calc.trig_type)
#define InvsEnabled(w) ((w)->calc.invs_enabled)

#define SaveMemoryFlag(w) ((w)->calc.save_memory_flag)
#define SaveStrmax(w) ((w)->calc.save_strmax)
#define SaveNumStackCount(w) ((w)->calc.save_num_stack_count)
#define SaveBufferCount(w) ((w)->calc.save_buffer_count)
#define SaveCurrentOp(w) ((w)->calc.save_current_op)
#define SaveBuffer(w) ((w)->calc.save_buffer)
#define SaveNumStack(w) ((w)->calc.save_num_stack)
#define SaveErrorFlag(w) ((w)->calc.save_error_flag)
#define SaveMemErrorFlag(w) ((w)->calc.save_mem_error_flag)
#define SaveMemValue(w) ((w)->calc.save_mem_value)
#define SaveNumstr(w) ((w)->calc.save_numstr)

#define HighNo(w) ((w)->calc.high_no)
#define LowNo(w) ((w)->calc.low_no)

#define Win(w) (XtWindow(w))

#define Parent(w) ((w)->core.parent)
#define Width(w) ((w)->core.width)
#define Height(w) ((w)->core.height)
#define Foreground(w) ((w)->manager.foreground)
#define Background(w) ((w)->core.background_pixel)

#define Double(x)	((x) << 1)
#define Half(x)		((x) >> 1)

#define TotalWidth(w)   (XtWidth  (w) + Double (XtBorderWidth (w)))
#define TotalHeight(w)  (XtHeight (w) + Double (XtBorderWidth (w)))

#define MAX_V1_FONT_CHARS	126
#define V1_PI_KEY		112
#define V2_PI_KEY		123

#define V1_SQRT_KEY		86
#define V2_SQRT_KEY		214

#define NUMCLEARS		2
#define NUMKEYS			12
#define NUMOPS			7
#define NUMFUNC			4
#define NUMINVFUNC		8
#define NUMMEMS			4

#define HORSCALE		9.0
#define VERTSCALE		13.0
#define HORPADDINGRATIO		0.25
#define VERTPADDINGRATIO	0.25
#define DISPLAYRATIO		1/HORSCALE
#define CLEARBOXRATIO		1.5/VERTSCALE
#define KEYBOXRATIO		3.0/VERTSCALE
#define OPSBOXRATIO		2.0/VERTSCALE
#define BOXHEIGHTRATIO		4.0/HORSCALE
#define PI			3.14159265358979

#define DEGREES			1
#define RADIANS			2
#define GRADIANTS		3

/* constant definitions */
#define BadLanguage     "Can't get language from toolkit, using US English \n"
#define BadLocale	"Can't open locale \n"
#define NoPopup		"Can't fetch popup menu \n"
#define NoHierarchy 	"Can't open hierarchy \n"
#define NoMain		"Can't fetch calculator widget \n"
#define NoKeyFont	"Can't open keys font, using default font \n"
#define NoSqrtFont	"Can't open square root font, using default font \n"
#define NoBellPixmap	"Can't create logo pixmap \n"
#define NoHelpWidget	"Can't fetch help widget \n"
#define NoBeginClip	"Can not begin clipboard operation \n"
#define NoClipCopy	"Can not copy to clipboard \n"
#define NoEndClip	"Nothing of type text found to Paste \n"
#define NoPasteCopy	"Can not end clipboard operation \n"
#define MAX_BUFF    	14
#define MAX_HEX_BUFF	8
#define MAX_OCT_BUFF	11
#define HIGH_HEX_NUM	FFFFFFFF
#define HIGH_OCT_NUM	377777777777
#define INT_MAX_STR 	32		/* internally let the display string
					 * get this long */
#define NUM_KEYS    	37
#define MAX_FILE_LEN	256

/* calculator key literals */
#define k_display	1
#define k_memory	2
#define k_key_bound_box	3
#define k_clear_bound_box	4
#define k_ops_bound_box	5

#define MEM_BASE	6
#define k_mem_clear	6
#define k_mem_recall	7
#define k_mem_plus	8
#define k_mem_minus	9

#define DIGIT_BASE	10
#define k_key_0		10
#define k_key_period	11
#define k_key_pi	12
#define k_key_1		13
#define k_key_2		14
#define k_key_3		15
#define k_key_4		16
#define k_key_5		17
#define k_key_6		18
#define k_key_7		19
#define k_key_8		20
#define k_key_9		21

#define OPS_BASE	22
#define k_key_divide	22
#define k_key_times	23
#define k_key_minus	24
#define k_key_plus	25
#define k_key_plus_minus	26
#define k_key_percent	27
#define k_key_equals	28

#define CLEAR_BASE	29
#define k_key_clear	29
#define k_key_clear_entry	30

#define FUNC_BASE	31
#define k_key_degrees	31
#define k_key_xfact	32
#define k_key_1overx	33
#define k_key_random	34

#define INV_FUNC_BASE	35
#define k_key_inverse	35
#define k_key_sine	36
#define k_key_cosine	37
#define k_key_tangent	38
#define k_key_logrithm	39
#define k_key_natlog	40
#define k_key_square_root	41
#define k_key_ytothex	42

#define INVERSE_FUNCS	50
#define k_key_asine	50
#define k_key_acosine	51
#define k_key_atangent	52
#define k_key_10tox	53
#define k_key_exp	54
#define k_key_square	55
#define k_key_ytominusx	56

#define HEX_BASE	60
#define k_key_a		60
#define k_key_b		61
#define k_key_c		62
#define k_key_d		63
#define k_key_e		64
#define k_key_f		65
#define k_key_not	66
#define k_key_neg	67

#define LOGICAL_BASE	70
#define k_key_and	70
#define k_key_or	71
#define k_key_xor	72
#define k_key_nor	73

/* Widget definitions	*/
#define k_help_wid	31
#define k_message_wid	32
#define k_pop_up_undo_button	33
#define k_pull_down_undo_button	34
#define k_menu_bar	35
#define k_logo		36

#define k_help_main	40

/* Operation modes	*/
#define k_decimal    	1
#define k_hexadecimal	2
#define k_octal		3
#define k_binary	4

#define SHFT_KEY  174
#define CTRL_KEY  175
#define LOCK_KEY  176
#define COMP_KEY  177

#define EMPTY     -1			/* literal for empty stack */
#define NONE      0
#define MENU_OPS  6

/* Several OS specific defines
OK_STATUS - status to return for success
ERROR_STATUS - status to return for failure
*/

#ifdef VMS
#define OK_STATUS	1
#define ERROR_STATUS	0
#else
#define OK_STATUS	0
#define ERROR_STATUS	1
#endif

#define INFORMATION		0
#define WARNING			1
#define ERROR			2
#define FATAL			3

#ifdef VMS

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *                      VMSDSC (VMS descriptors)
 *                      ========================
 *
 *  These macros require the following header file:
 *
 *              descrip.h
 *
 * VMSDSC (typedef)
 * ------
 *
 * $INIT_VMSDSC (intialize a VMS static descriptor)
 * ------------
 *      dsc             : VMSDSC        : write : value
 *      len             : int           : read  : value
 *      addr            : char          : read  : ref
 *
 *    RETURNS: address of the descriptor (dsc)
 */

#define VMSDSC          struct dsc$descriptor

#define $INIT_VMSDSC(dsc, len, addr)            \
        ((dsc .dsc$b_class) = DSC$K_CLASS_S,    \
         (dsc .dsc$b_dtype) = DSC$K_DTYPE_T,    \
         (dsc .dsc$w_length) = (len),           \
         (dsc .dsc$a_pointer) = (addr),         \
         (& (dsc) )                             \
        )
#endif
