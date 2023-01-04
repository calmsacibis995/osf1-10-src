#ifndef _prdw_entry_h_
#define _prdw_entry_h_
/* $Id$ */
/* #module prdw_entry.h */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows PrintScreen
**
**  ABSTRACT:
**
**	Function prototypes for all modules.
**	
**--
*/

/*
** caution.c
*/
int decw$caution_ack PROTOTYPE((
    Widget		widget,
    unsigned int	tag,
    XmAnyCallbackStruct	*any
));

int decw$create_caution PROTOTYPE((unsigned int message_value));

/*
** createpanel.c
*/
int create_panel PROTOTYPE((void));

XtActionProc map_event PROTOTYPE((
    Widget	w,
    XEvent	*event,
    String	*params,
    Cardinal	*num_params
));

Widget MyCreateWidget PROTOTYPE((
    String      name,
    WidgetClass widgetClass,
    Widget      parent,
    ArgList     args,
    Cardinal    num_args
));

int move_event PROTOTYPE ((void));

XtActionProc configure_event PROTOTYPE((
    Widget	widget,
    XEvent	*event,
    char	*params,
    Cardinal	*num_params
));

/*
** creatoutput.c
*/
int creatdd PROTOTYPE((
    dxPrscOptions	*options,
    int			height,
    int			width,
    XImage		*ximage,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
));

int creatsx PROTOTYPE((
    dxPrscOptions	*options,
    int			height,
    int			width,
    FILE		*file,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
));

int creatps PROTOTYPE((
    dxPrscOptions	*options,
    int			height,
    int			width,
    FILE		*file,
    unsigned long	image_id,
    unsigned int	converted,
    Display		*dpy,
    int			dfs
));

int prsc_emit_image_line PROTOTYPE((
    unsigned char	*buf_ptr,
    int			buf_len,
    FILE		*file
));

/*
** error.c
*/
int decw$create_error PROTOTYPE((void));

int decw$error_display PROTOTYPE((
    XmString	message
));

int decw$restart PROTOTYPE((
    Widget		*widget,
    caddr_t		tag,
    XmAnyCallbackStruct	*any
));

/*
** getcoor.c
*/
int getrectwin PROTOTYPE((
    Display	*dpy,
    int		dfs,
    XRectangle	*xrect,
    Window	*screen_ret
));

/*
** getfile.c
*/
FILE *getfile PROTOTYPE((
    dxPrscOptions	*options
));

/*
** getqueue.c
*/
int dxPrscInitialize PROTOTYPE((void));

int getqueue PROTOTYPE((
    dxPrscOptions	*options,
    Widget		top
));

/*
** helpcb.c
*/
void help_menu_cb PROTOTYPE((
    Widget			*widgetID,
    int				*tag,
    XmRowColumnCallbackStruct	*any
));

void help_unmap PROTOTYPE((void));

void sens_help_proc PROTOTYPE((
    Widget		*w,
    char		*tag,
    XmAnyCallbackStruct	*any
));

/*
** main.c
*/
int finish PROTOTYPE((void));

int getEquiv PROTOTYPE((
    char	*env,
    char	*equiv
));

void PrintDebugInfo PROTOTYPE((
    unsigned int	debug_flags
));

/*
** messages.c
*/
void put_error PROTOTYPE((
    unsigned int	status,
    int			text_index
));

/*
** parse.c
*/
int parse PROTOTYPE((int argc, char **argv));

/*
** prdw_c.c
*/
unsigned long dxPrscPrintScreenRect PROTOTYPE((
    Display		*dpy,
    int			dfs,
    Command		*command,
    int			x,
    int			y,
    unsigned int	width,
    unsigned int	height,
    Widget		top
));

unsigned long dxPrscPrintScreenWindow PROTOTYPE((
    Display		*dpy,
    int			dfs,
    Window		window,
    Command		*command,
    Widget		top
));

/*
** printcb.c
*/
void print_menu_cb PROTOTYPE((
    Widget			*widgetID,
    caddr_t			tag,
    XmRowColumnCallbackStruct	*any
));

int timed_capture_screen PROTOTYPE((void));

int do_capture_screen PROTOTYPE((void));

/*
** prtdialog.c
*/
void initialize_defaults PROTOTYPE((void));

int prt_dialog_get_values PROTOTYPE((void));

int prtattr_get_values PROTOTYPE((void));

int prtattr_set_values PROTOTYPE((void));

int prt_put_attrs PROTOTYPE((void));

void PSDDIFCallback PROTOTYPE((
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
));

void	PSColorCallback PROTOTYPE((
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
));

void	PSPostCallback PROTOTYPE((
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
));

int reset_page_size PROTOTYPE((void));

int reset_sixel_printer PROTOTYPE((void));

/*
** render.c
*/
unsigned long XimageToFid PROTOTYPE ((
     Display		*display,
     Screen		*screen,
     XImage		*ximage,
     Visual		*visual,
     Colormap		cmap,
     unsigned long	output_class,
     int		polarity
));

unsigned long CvtDataClass PROTOTYPE ((
    unsigned long	srcfid,
    int			inclass,
    int			outclass
));

long img_signal_handler PROTOTYPE((
    ChfSigVecPtr	sigarg,
    ChfMchArgsPtr	mcharg
));

/*
** sessioncb.c
*/
void end_session PROTOTYPE((
    Widget		*widgetID,
    caddr_t		tag,
    XmAnyCallbackStruct	*any
));

void session_menu_cb PROTOTYPE((
    Widget			*widgetID,
    caddr_t			tag,
    XmRowColumnCallbackStruct	*any
));

/*
** setupcb.c
*/
void setup_menu_cb PROTOTYPE((
    Widget			widgetID,
    int				tag,
    XmRowColumnCallbackStruct	*reason
));

void page_size_cancel PROTOTYPE((
    Widget			widgetID,
    int				tag,
    XmPushButtonCallbackStruct	*reason
));

void sixel_device_cancel PROTOTYPE((
    Widget			widgetID,
    int				tag,
    XmPushButtonCallbackStruct	*reason
));

void updatesetup PROTOTYPE((void));

/*
** smconvert.c
*/
int str_to_int PROTOTYPE((
    char	*strptr,
    int		*value
));

int int_to_str PROTOTYPE((
    unsigned int	value,
    char		*result,
    unsigned int	maxsize
));

int sm_convert_int PROTOTYPE((
    int		index,
    char	*svalue,
    int		value[4]
));

int sm_convert_str PROTOTYPE((
    unsigned int	index,
    char		*source,
    char		schar[4]
));

int int_to_hexstr PROTOTYPE((
    unsigned short	value,
    char		*result,
    unsigned int	maxsize
));

/*
** smdata.c
*/
/* no entry points, just data */

/*
** smdialog.c
*/
int smattr_get_values PROTOTYPE((void));

void set_control_size PROTOTYPE((void));

/*
** smicon.c
*/
int IconInit PROTOTYPE((Widget w));

/*
** sminit.c
*/
char *ResolveFilename PROTOTYPE((
    Display	*dpy,
    String	class_name,
    Boolean	userfilename,
    Boolean	must_exist
));

int ws_init PROTOTYPE((
    int		*argc,
    char	**argv
));

int sm_put_database PROTOTYPE((
    Display	*dpy
));

int sm_switch_database PROTOTYPE((
    Display	*display
));

int sm_save_database PROTOTYPE((
    Display	*dpy
));

int sm_use_managers PROTOTYPE((
    Display	*display
));

XmString get_drm_message  PROTOTYPE((
    int		message
));

/*
** smresource.c
*/
/* no entry points, just data */

/*
** swap.c
*/
void swapbits PROTOTYPE((
    register unsigned char	*b,
    register int		n
));

void negbits PROTOTYPE((
    register unsigned char	*b,
    register int		n
));

void swapshort PROTOTYPE((
    register char	*bp,
    register int	n
));

void swaplong PROTOTYPE((
    register char	*bp,
    register int	n
));

void swapthree PROTOTYPE((
    register char	*bp,
    register int	n
));

/*
** utilities.c
*/
void widget_create_proc PROTOTYPE((
    Widget		w,
    Widget		*tag,
    unsigned int	*reason
));

void wait_cursor PROTOTYPE((int on));

int determine_system_color PROTOTYPE((
    Display		*display_id,
    unsigned int	screen_num
));

/*
** xrmutil.c
*/
int sm_get_resource PROTOTYPE((
    char		*name,
    char		*value
));

int sm_get_int_resource PROTOTYPE((
    unsigned int	index,
    int			intptr[4]
));

int sm_get_string_resource PROTOTYPE((
    unsigned int	index,
    char		*charptr
));

int sm_get_any_resource PROTOTYPE((
    unsigned int	index,
    char		*charptr,
    int			intptr[4]
));

void sm_put_resource PROTOTYPE((
    unsigned int	index,
    char		*value
));

void sm_put_int_resource PROTOTYPE((
    unsigned int	index,
    int			value
));

int sm_get_screen_resource PROTOTYPE((
    char		*name,
    unsigned int	screen_type,
    char		*value
));

int sm_get_int_screen_resource PROTOTYPE((
    unsigned int	index,
    unsigned int	screen_type,
    int			intptr[4]
));

char *CSToLatin1 PROTOTYPE((XmString cs));

/*
** xwdio.c
*/
int xwdWrite PROTOTYPE((
    char		*xwd_filename,
    XImage		*ximage,
    int			colorct,
    Visual		*vis,
    char		*wname,
    XColor		**colors,
    Display		*display,
    int			screen
));

int getColors PROTOTYPE((
    Display		*display,
    Colormap		cmap,
    Visual		*vis,
    XColor		**colors
));

#endif /* _prdw_entry_h_ */
