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
#ifdef DOPRINT
#include "smdata.h"
#include "smresource.h"
#include "prdw.h"

void	print_action();
void	print_cancel();
void	print_apply();
void widget_create_proc();
void PSDDIFCallback();
void PSColorCallback();
void PSPostCallback();

int	create_print_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects PringScreen from the
**	Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to set certain defaults which will
**	be used when they select the PrintScreen function
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**	
**--
**/
{
static MrmRegisterArg reglist[] = {
        {"PSOkButtonCallback", (caddr_t) print_action},
        {"PSApplyButtonCallback", (caddr_t) print_apply},
        {"PSCancelButtonCallback", (caddr_t) print_cancel},
	{"PSDDIFCallback", (caddr_t) PSDDIFCallback},
        {"PSColorCallback", (caddr_t) PSColorCallback},
        {"PSPostCallback", (caddr_t) PSPostCallback},
        {"FormatBoxId", &prtsetup.format_id},
        {"PostScriptId", &prtsetup.postscript_id},
        {"SixelId", &prtsetup.sixel_id},
        {"BitmapId", &prtsetup.bitmap_id},
        {"RatioBoxId", &prtsetup.ratio_id},
        {"OnetoOneId", &prtsetup.r1to1_id},
        {"TwotoOneId", &prtsetup.r2to1_id},
        {"SaverBoxId", &prtsetup.saver_id},
        {"PositiveId", &prtsetup.positive_id},
        {"NegativeId", &prtsetup.negative_id},
        {"FilenameId", &prtsetup.capture_file_id},
        {"FilePromptId", &prtsetup.file_prompt_id},
        {"RotatePromptId", &prtsetup.rotate_prompt_id},
        {"ColorBoxId", &prtsetup.prtcolor_id},
        {"PSColorId", &prtsetup.color_id},
        {"PSGreyId", &prtsetup.grey_id},
        {"PSBWId", &prtsetup.bw_id},
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

MrmRegisterNames (reglist, reglist_num);

/* build the dialog using UIL */
MrmFetchWidget(s_DRMHierarchy, "CustomizePrinter", smdata.toplevel,
                        &prtsetup.prt_attr_id,
                        &drm_dummy_class);

return(1);
}

void	print_action(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The OK button was hit from the PrintScreen Customization dialog box.
**	We need to look at the widgets, determine what changed, 
**	and unmanage the dialog box.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**	
**--
**/
{
/* this call back can be called multiple times if the user hits the
    button quickly several times with either mouse, or CR.  Check
    to see if we have already unmanaged the widget before we do
    anything */
if (prtsetup.managed == ismanaged)
    {
    prtsetup.managed = notmanaged;
    XtUnmanageChild(XtParent(*widget));
    /* Get the settings of the widgets */
    prt_dialog_get_values();
    /* Save new settings */
    prt_put_attrs();
    }
}

void	print_apply(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The APPLY button was hit from the PrintScreen Customization dialog box.
**      We need to look at the widgets, determine what changed.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
/* Get the current settings */
prt_dialog_get_values();
/* save them in the resource data base */
prt_put_attrs();
}

void	print_cancel(widget, tag, reason)
Widget	*widget;
caddr_t	tag;
unsigned int reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The CANCEL button was hit from the PrintScreen Customization dialog box.
**      We need to unmanage the dialog box.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
/* The user can hit the buttons several times before the dialog box is
   actually unmanaged.   Mark the first time through the callback so that
   we only free stuff once */
if (prtsetup.managed == ismanaged)
    {
    prtsetup.managed = notmanaged;
    XtUnmanageChild(XtParent(*widget));
    prtsetup.changed = 0;
    }
}


int	prt_dialog_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      The OK or APPLY button was hit.  Look at each widget and store
**      its value in the data structures IF it is different from the
**      current value.  Also mark the bit in the change mask if it
**      changes.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned    int	i,j;

/* get value of printer ratio */
i = XmToggleButtonGetState(prtsetup.r1to1_id);

if (i == 1)
    {
    if (prtsetup.ratio != DECWC_PRSC_ASPECT_1)
	{
	prtsetup.changed = prtsetup.changed | mratio;
	prtsetup.ratio = DECWC_PRSC_ASPECT_1;
	}
    }
else
    {
    if (prtsetup.ratio != DECWC_PRSC_ASPECT_2)
	{
	prtsetup.changed = prtsetup.changed | mratio;
	prtsetup.ratio = DECWC_PRSC_ASPECT_2;
	}
    }

/* get value of toner ribbon saver*/
i = XmToggleButtonGetState(prtsetup.positive_id);

if (i ==1)
    {
    if (prtsetup.saver != DECWC_PRSC_POSITIVE)
	{
	prtsetup.saver = DECWC_PRSC_POSITIVE;
	prtsetup.changed = prtsetup.changed | msaver;
	}
    }
else
    {
    if (prtsetup.saver != DECWC_PRSC_NEGATIVE)
	{
	prtsetup.saver = DECWC_PRSC_NEGATIVE;
	prtsetup.changed = prtsetup.changed | msaver;
	}
    }

/* information storage format */
i = XmToggleButtonGetState(prtsetup.sixel_id);
j = XmToggleButtonGetState(prtsetup.bitmap_id);
if (i==1)
    {
    if (prtsetup.format != DECWC_PRSC_SIXEL)
	{
	prtsetup.format = DECWC_PRSC_SIXEL;
	prtsetup.changed = prtsetup.changed | mformat;
	}
    }
else
    {
    if (j == 1)
	{
	if (prtsetup.format != DECWC_PRSC_DDIF)
	    {
	    prtsetup.format = DECWC_PRSC_DDIF;
	    prtsetup.changed = prtsetup.changed | mformat;
	    }
	}
    else
	{
	if (prtsetup.format != DECWC_PRSC_POSTSCRIPT)
	    {
	    prtsetup.format = DECWC_PRSC_POSTSCRIPT;
	    prtsetup.changed = prtsetup.changed | mformat;
	    }
	}
    }

/* color output*/
i = XmToggleButtonGetState(prtsetup.color_id);
j = XmToggleButtonGetState(prtsetup.grey_id);
if (i==1)
    {
    if (prtsetup.color != DECWC_PRSC_PRINTER_COLOR)
	{
	prtsetup.color = DECWC_PRSC_PRINTER_COLOR;
	prtsetup.changed = prtsetup.changed | mcolor;
	}
    }
else
    {
    if (j == 1)
	{
	if (prtsetup.color != DECWC_PRSC_PRINTER_GREY)
	    {
	    prtsetup.color = DECWC_PRSC_PRINTER_GREY;
	    prtsetup.changed = prtsetup.changed | mcolor;
	    }
	}
    else
	{
	if (prtsetup.color != DECWC_PRSC_PRINTER_BW)
	    {
	    prtsetup.color = DECWC_PRSC_PRINTER_BW;
	    prtsetup.changed = prtsetup.changed | mcolor;
	    }
	}
    }

/* prompt for filename option */
j = XmToggleButtonGetState(prtsetup.file_prompt_id);
if (j != prtsetup.file_prompt)
	    {
	    prtsetup.file_prompt = j;
            prtsetup.changed = prtsetup.changed | mprtprompt;
	    }

/* prompt for filename option */
j = XmToggleButtonGetState(prtsetup.rotate_prompt_id);
if (j == 1)
    {
    if (prtsetup.rotate_prompt != DECWC_PRSC_FORM)
	{
	prtsetup.rotate_prompt = DECWC_PRSC_FORM;
	prtsetup.changed = prtsetup.changed | mrotateprompt;
	}
    }
else
    {
    if (prtsetup.rotate_prompt != DECWC_PRSC_NOFORM)
	{
	prtsetup.rotate_prompt = DECWC_PRSC_NOFORM;
	prtsetup.changed = prtsetup.changed | mrotateprompt;
	}
    }
}

int	prtattr_get_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine will get the resources from the resource database
**      for each item in the PrintScreen Customization.  We need to
**      set the values of the widgets when the dialog box is managed.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
char	svalue[256];
int	value[4];
int	size;

/* aspect ratio */
size = sm_get_any_resource(paspect, svalue, value);
if ((value[0] == 2) && (value[1] == 1))
    {
    prtsetup.ratio = DECWC_PRSC_ASPECT_2;
    }
else
    {
    prtsetup.ratio = DECWC_PRSC_ASPECT_1;
    }



/* printer color */
size = sm_get_any_resource(pcolor, svalue, value);
if (strcmp(svalue, "grey") == 0)
    {
    prtsetup.color = DECWC_PRSC_PRINTER_GREY;
    }
else
    {
    if (strcmp(svalue, "color") == 0)
	{
	prtsetup.color = DECWC_PRSC_PRINTER_COLOR;
	}
    else
	prtsetup.color = DECWC_PRSC_PRINTER_BW;
    }

/* screen saver */
size = sm_get_any_resource(psaver, svalue, value);
if (strcmp(svalue, "negative") == 0)
    {
    prtsetup.saver = DECWC_PRSC_NEGATIVE;
    }
else
    prtsetup.saver = DECWC_PRSC_POSITIVE;

/* printer format */
size = sm_get_any_resource(pformat, svalue, value);
if (strcmp(svalue, "sixel") == 0)
    {
    prtsetup.format = DECWC_PRSC_SIXEL;
    }
else
    {
    if (strcmp(svalue, "DDIF") == 0)
	{
	prtsetup.format = DECWC_PRSC_DDIF;
	}
    else
	prtsetup.format = DECWC_PRSC_POSTSCRIPT;
    }



/* capture screen file name */
size = sm_get_any_resource(pfile, svalue, value);
strcpy(prtsetup.capture_file, svalue);

/* file prompt */
size = sm_get_any_resource(prtprompt, svalue, value);

if (size == 0)
    prtsetup.file_prompt = 1;
else
    if (value[0] == 1)
	prtsetup.file_prompt = 1;
    else
	prtsetup.file_prompt = 0;

/* ratio prompt */
size = sm_get_any_resource(prtrotateprompt, svalue, value);

if (size == 0)
    prtsetup.rotate_prompt = DECWC_PRSC_FORM;
else
    if (value[0] == 1)
	prtsetup.rotate_prompt = DECWC_PRSC_FORM;
    else
	prtsetup.rotate_prompt = DECWC_PRSC_NOFORM;
}

int	prtattr_set_values()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When the dialog box is managed, we need to set all of the widgets
**      to the correct values based on the current settings of the
**      resources.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
char	temp[256];
unsigned	int	status;


prtsetup.changed = 0;

/* aspect ratio */
if (prtsetup.ratio == DECWC_PRSC_ASPECT_1)
	{
	XmToggleButtonSetState(prtsetup.r1to1_id, 1, 1);
	XmToggleButtonSetState(prtsetup.r2to1_id,0, 1);
	}
else
	{
	XmToggleButtonSetState(prtsetup.r1to1_id, 0, 1);
	XmToggleButtonSetState(prtsetup.r2to1_id, 1, 1);
	}

/* ribbon or toner saver */
if (prtsetup.saver == DECWC_PRSC_POSITIVE)
	{
	XmToggleButtonSetState(prtsetup.positive_id, 1, 1);
	XmToggleButtonSetState(prtsetup.negative_id,0, 1);
	}
else
	{
	XmToggleButtonSetState(prtsetup.positive_id, 0, 1);
	XmToggleButtonSetState(prtsetup.negative_id, 1, 1);
	}


/* capture screen filename */
XmTextSetString(prtsetup.capture_file_id, prtsetup.capture_file);

if (prtsetup.file_prompt == 0)
	{
	XmToggleButtonSetState(prtsetup.file_prompt_id, 0, 0);
	}
else
	{
	XmToggleButtonSetState(prtsetup.file_prompt_id, 1, 0);
	}

if (prtsetup.rotate_prompt == DECWC_PRSC_NOFORM)
	{
	XmToggleButtonSetState(prtsetup.rotate_prompt_id, 0, 0);
	}
else
	{
	XmToggleButtonSetState(prtsetup.rotate_prompt_id, 1, 0);
	}


/* The setting of output color can affect Toner Saver setting, so
   always set this after toner saver has been set */
if (prtsetup.color == DECWC_PRSC_PRINTER_COLOR)
	{
	XmToggleButtonSetState(prtsetup.color_id, 1, 1);
	XmToggleButtonSetState(prtsetup.grey_id,0, 1);
	XmToggleButtonSetState(prtsetup.bw_id,0, 1);
	}
else
    {
    if (prtsetup.color == DECWC_PRSC_PRINTER_GREY)
	{
	XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	XmToggleButtonSetState(prtsetup.grey_id,1, 1);
	XmToggleButtonSetState(prtsetup.bw_id,0, 1);
	}
   else
	{
	XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	XmToggleButtonSetState(prtsetup.grey_id,0, 1);
	XmToggleButtonSetState(prtsetup.bw_id,1, 1);
	}
    }


/* information format */
/* The setting of output format can affect other settings, so
   always set this last */
if (prtsetup.format == DECWC_PRSC_POSTSCRIPT)
	{
	XmToggleButtonSetState(prtsetup.postscript_id, 1, 1);
	XmToggleButtonSetState(prtsetup.bitmap_id,0, 1);
	XmToggleButtonSetState(prtsetup.sixel_id,0, 1);
	if (smdata.print_es) XtSetSensitive(smdata.print_es, TRUE);    
	if (smdata.print_pos) XtSetSensitive(smdata.print_pos, TRUE);    
	}
else
    {
    if (prtsetup.format == DECWC_PRSC_SIXEL)
	{
	XmToggleButtonSetState(prtsetup.postscript_id, 0, 1);
	XmToggleButtonSetState(prtsetup.bitmap_id,0, 1);
	XmToggleButtonSetState(prtsetup.sixel_id,1, 1);
	if (smdata.print_es) XtSetSensitive(smdata.print_es, FALSE);    
	if (smdata.print_pos) XtSetSensitive(smdata.print_pos, FALSE);    
	}
   else
	{
	XtSetSensitive(smdata.print_es, TRUE);    
	XtSetSensitive(smdata.print_pos, TRUE);    
	XmToggleButtonSetState(prtsetup.postscript_id, 0, 1);
	if (smdata.print_es) XmToggleButtonSetState(prtsetup.bitmap_id,1, 1);
	if (smdata.print_pos) XmToggleButtonSetState(prtsetup.sixel_id,0, 1);
	}
    }
}


int	prt_put_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      When Customization is changed, we need to write the
**      new values of the resources to the resource database.  This
**      routine will write each resource to the database.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
char	*value;
unsigned	int	ivalue,status;
char	astring[10];	

/* aspect ratio */
if ((prtsetup.changed & mratio) != 0)
	{
	if (prtsetup.ratio == DECWC_PRSC_ASPECT_1)
		{
		sm_put_resource(paspect, "1,1");
		}
	else
		{
		sm_put_resource(paspect, "2,1");
		}
	}

/* ribbon or toner saver */
if ((prtsetup.changed & msaver) != 0)
	{
	if (prtsetup.saver == DECWC_PRSC_POSITIVE)
		{
		sm_put_resource(psaver, "positive");
		}
	else
		{
		sm_put_resource(psaver, "negative");
		}

	}

/* storage format */
if ((prtsetup.changed & mformat) != 0)
	{
	if (prtsetup.format == DECWC_PRSC_POSTSCRIPT)
		{
		sm_put_resource(pformat, "postscript");
		}
	else
	    {
	    if (prtsetup.format == DECWC_PRSC_SIXEL)
		{
		sm_put_resource(pformat, "sixel");
		}
	    else
		{
		sm_put_resource(pformat, "DDIF");
		}
	    }
	}

/* storage format */
if ((prtsetup.changed & mcolor) != 0)
	{
	if (prtsetup.color == DECWC_PRSC_PRINTER_COLOR)
		{
		sm_put_resource(pcolor, "color");
		}
	else
	    {
	    if (prtsetup.color == DECWC_PRSC_PRINTER_GREY)
		{
		sm_put_resource(pcolor, "grey");
		}
	    else
		{
		sm_put_resource(pcolor, "bw");
		}
	    }
	}

/* capture screen filename */
value = XmTextGetString(prtsetup.capture_file_id);
if ((strcmp(value, prtsetup.capture_file) != 0))
	{
	strcpy(prtsetup.capture_file, value);
	sm_put_resource(pfile, value);
	prtsetup.changed = prtsetup.changed | mfile;
	}
XtFree(value);

if ((prtsetup.changed & mprtprompt) != 0)
	{
	sm_put_int_resource(prtprompt, prtsetup.file_prompt);
	}

if ((prtsetup.changed & mrotateprompt) != 0)
	{
	if (prtsetup.rotate_prompt == DECWC_PRSC_NOFORM)
	    sm_put_int_resource(prtrotateprompt, 0);
	else
	    sm_put_int_resource(prtrotateprompt, 1);
	}

if (prtsetup.changed != 0)
	{
	sm_change_property(XtDisplay(smdata.toplevel));
	smdata.resource_changed = 1;
	prtsetup.changed = 0;
	}

    if (XmToggleButtonGetState(prtsetup.bitmap_id) == 0) {
      XtSetSensitive(smdata.print_es, TRUE);    
      XtSetSensitive(smdata.print_pos, TRUE);    
    } else {
      XtSetSensitive(smdata.print_es, FALSE);    
      XtSetSensitive(smdata.print_pos, FALSE);    
    }
}

void	PSDDIFCallback(widget, reason, data)
Widget	*widget;
unsigned int reason;
XmToggleButtonCallbackStruct	*data;
{

if (data->set == True)
    {
    if (XmToggleButtonGetState (prtsetup.r2to1_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.r2to1_id, 0, 1);
	XmToggleButtonSetState(prtsetup.r1to1_id, 1, 1);
	}
    XtSetSensitive(prtsetup.r2to1_id, FALSE); 
    if (XmToggleButtonGetState (prtsetup.rotate_prompt_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.rotate_prompt_id, 0, 1);
	}
    XtSetSensitive(prtsetup.rotate_prompt_id, FALSE);    
    if (XmToggleButtonGetState (prtsetup.negative_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.negative_id, 0, 1);
	XmToggleButtonSetState(prtsetup.positive_id, 1, 1);
	}
    XtSetSensitive(prtsetup.negative_id, FALSE);    
    }
else
    {
    if (XmToggleButtonGetState(prtsetup.postscript_id) == 0)
	XtSetSensitive(prtsetup.r2to1_id, TRUE); 
    XmToggleButtonSetState(prtsetup.rotate_prompt_id, 1, 1);
    XtSetSensitive(prtsetup.rotate_prompt_id, TRUE);    
    
    if (XmToggleButtonGetState(prtsetup.color_id) == 0)
	XtSetSensitive(prtsetup.negative_id, TRUE);    
    }
}

void	PSColorCallback(widget, reason, data)
Widget	*widget;
unsigned int reason;
XmToggleButtonCallbackStruct	*data;
{
if (data->set == True)
    {
    if (XmToggleButtonGetState (prtsetup.negative_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.negative_id, 0, 1);
	XmToggleButtonSetState(prtsetup.positive_id, 1, 1);
	}
    XtSetSensitive(prtsetup.negative_id, FALSE);    
    }
else
    {
    if (XmToggleButtonGetState(prtsetup.bitmap_id) == 0)
	XtSetSensitive(prtsetup.negative_id, TRUE);    
    }
}

void	PSPostCallback(widget, reason, data)
Widget	*widget;
unsigned int reason;
XmToggleButtonCallbackStruct	*data;
{
if (data->set == True)
    {
    if (XmToggleButtonGetState (prtsetup.r2to1_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.r2to1_id, 0, 1);
	XmToggleButtonSetState(prtsetup.r1to1_id, 1, 1);
	}
    XtSetSensitive(prtsetup.r2to1_id, FALSE); 
    if (XmToggleButtonGetState (prtsetup.color_id) == 1)
	{
	XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	XmToggleButtonSetState(prtsetup.bw_id, 1, 1);
	}
    XtSetSensitive(prtsetup.color_id, FALSE);    
    }
else
    {
    if (XmToggleButtonGetState(prtsetup.bitmap_id) == 0)
	XtSetSensitive(prtsetup.r2to1_id, TRUE); 
#ifdef NO_COLOR
    XtSetSensitive(prtsetup.color_id, FALSE);    
#else
    XtSetSensitive(prtsetup.color_id, TRUE);    
#endif
    }
}
#endif /* DOPRINT */
