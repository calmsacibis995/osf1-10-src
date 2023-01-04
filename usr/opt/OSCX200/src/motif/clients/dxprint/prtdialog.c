/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module handles the creation of the Customize print screen
**	dialog box.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	 4-Feb-1992	Edward P Luwish
**		Performance and I18N modifications
**
**	 3-Jun-1991	Edward P Luwish
**		Change default delay time to zero.
**
**	13-Apr-1991	Edward P Luwish
**		Port to Motif UI.  Added numerous new options, made the
**		resource strings I18N compatible by extracting them from
**		the compound string labels of the associated toggle buttons.
**
**	30-NOV-90		KMR
**		Really allow negative DDIF (took two tries to get it right)
**
**	31-JUL-90		KMR
**		Allow negative DDIF
**
*/

/*
** Include files
*/
#include "iprdw.h"
#include "smdata.h"
#include "smresource.h"

#ifdef VMS
#include <ssdef.h>
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <img/IdsImage.h>
#include <DXm/DXmPrint.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw.h"
#include "prdw_entry.h"

static char *get_label_string PROTOTYPE((Widget	awidget));

static void sixel_setup PROTOTYPE((void));

void initialize_defaults ()

/* Fills up the resource database with its default values, getting them
*  from the label strings of the default push buttons.  This way, when the
*  user saves the current settings, they will be in the user's language.
*/
{
    strcpy( def_table[0].def_value, "1,1" );		/* obsolete */
    strcpy( def_table[1].def_value, get_label_string(prtsetup.bw_id));

#ifdef VMS
    strcpy( def_table[2].def_value, "sys$login:decw$capture.tmp" );
#else
    strcpy( def_table[2].def_value, "printscreen.ps");
#endif

    strcpy( def_table[3].def_value, get_label_string(prtsetup.positive_id));
    strcpy( def_table[4].def_value, get_label_string(prtsetup.postscript_id));
    strcpy( def_table[5].def_value, "200" );		/* obsolete */
    strcpy( def_table[6].def_value, "660" );		/* obsolete */
    strcpy( def_table[7].def_value, "1" );		/* Ultrix only */
    strcpy( def_table[8].def_value, "0" );		/* multiscreen only */
    strcpy( def_table[9].def_value, "-1" );		/* multiscreen only */
    strcpy( def_table[10].def_value, "10" );		/* x value */
    strcpy( def_table[11].def_value, "95" );		/* y value */
    strcpy( def_table[12].def_value, "1" );		/* obsolete */
    strcpy( def_table[13].def_value, "Control Panel" );	/* obsolete */
    strcpy( def_table[14].def_value, get_label_string(prtsetup.crop_id));
    strcpy( def_table[15].def_value, get_label_string(prtsetup.portrait_id));
    strcpy( def_table[16].def_value, get_label_string(prtsetup.letter_id));
    strcpy( def_table[17].def_value, get_label_string(prtsetup.vt_id));
    strcpy( def_table[18].def_value, "0" );		/* 0 second delay */
    return;
}

int prt_dialog_get_values ()
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
    char	*value;

    /*
    ** Storage format.
    */
    if (XmToggleButtonGetState(prtsetup.sixel_id))
    {
	if (prtsetup.format != dxPrscSixel)
	{
	    prtsetup.format = dxPrscSixel;
	    prtsetup.changed = prtsetup.changed | mformat;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.bitmap_id))
    {
	if (prtsetup.format != dxPrscDDIF)
	{
	    prtsetup.format = dxPrscDDIF;
	    prtsetup.changed = prtsetup.changed | mformat;
	}
    }
    else
    {
	if (prtsetup.format != dxPrscPostscript)
	{
	    prtsetup.format = dxPrscPostscript;
	    prtsetup.changed = prtsetup.changed | mformat;
	}
    }
    /*
    ** Positive or negative.
    */
    if (XmToggleButtonGetState(prtsetup.positive_id))
    {
	if (prtsetup.saver != dxPrscPositive)
	{
	    prtsetup.saver = dxPrscPositive;
	    prtsetup.changed = prtsetup.changed | msaver;
	}
    }
    else
    {
	if (prtsetup.saver != dxPrscNegative)
	{
	    prtsetup.saver = dxPrscNegative;
	    prtsetup.changed = prtsetup.changed | msaver;
	}
    }

    /*
    ** orientation
    */
    if (XmToggleButtonGetState(prtsetup.portrait_id))
    {
	if (prtsetup.orientation != dxPrscPortrait)
	{
	    prtsetup.orientation = dxPrscPortrait;
	    prtsetup.changed = prtsetup.changed | morientation;
	}
    }
    else if (XmToggleButtonGetState (prtsetup.landscape_id))
    {
	if (prtsetup.orientation != dxPrscLandscape)
	{
	    prtsetup.orientation = dxPrscLandscape;
	    prtsetup.changed = prtsetup.changed | morientation;
	}
    }
    else
    {
	if (prtsetup.orientation != dxPrscBestFit)
	{
	    prtsetup.orientation = dxPrscBestFit;
	    prtsetup.changed = prtsetup.changed | morientation;
	}
    }

    /*
    ** Color type of output.
    */
    if (XmToggleButtonGetState(prtsetup.color_id))
    {
	if (prtsetup.color != dxPrscPrinterColor)
	{
	    prtsetup.color = dxPrscPrinterColor;
	    prtsetup.changed = prtsetup.changed | mcolor;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.grey_id))
    {
	if (prtsetup.color != dxPrscPrinterGrey)
	{
	    prtsetup.color = dxPrscPrinterGrey;
	    prtsetup.changed = prtsetup.changed | mcolor;
	}
    }
    else
    {
	if (prtsetup.color != dxPrscPrinterBW)
	{
	    prtsetup.color = dxPrscPrinterBW;
	    prtsetup.changed = prtsetup.changed | mcolor;
	}
    }

    /*
    ** Fit to screen method.
    */
    if (XmToggleButtonGetState(prtsetup.reduce_id))
    {
	if (prtsetup.fit != dxPrscReduce)
	{
	    prtsetup.fit = dxPrscReduce;
	    prtsetup.changed |= mfit;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.scale_id))
    {
	if (prtsetup.fit != dxPrscScale)
	{
	    prtsetup.fit = dxPrscScale;
	    prtsetup.changed |= mfit;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.grow_id))
    {
	if (prtsetup.fit != dxPrscGrow)
	{
	    prtsetup.fit = dxPrscGrow;
	    prtsetup.changed |= mfit;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.shrink_id))
    {
	if (prtsetup.fit != dxPrscShrink)
	{
	    prtsetup.fit = dxPrscShrink;
	    prtsetup.changed |= mfit;
	}
    }
    else
    {
	if (prtsetup.fit != dxPrscCrop)
	{
	    prtsetup.fit = dxPrscCrop;
	    prtsetup.changed |= mfit;
	}
    }

    /*
    ** Capture method.
    */
    if (XmToggleButtonGetState(prtsetup.entire_id))
    {
	if (prtsetup.capture_method != dxPrscEntire)
	{
	    prtsetup.capture_method = dxPrscEntire;
	    prtsetup.changed |= mcapture;
	}
    }
    else
    {
	if (prtsetup.capture_method != dxPrscPartial)
	{
	    prtsetup.capture_method = dxPrscPartial;
	    prtsetup.changed |= mcapture;
	}
    }

    /*
    ** Delay
    */
    XmScaleGetValue (prtsetup.delay_id, &i);
    if (i != prtsetup.delay)
    {
	prtsetup.delay = i;
	prtsetup.changed = prtsetup.changed | mdelay;
    }

    /*
    ** Capture method.
    */
    if (XmToggleButtonGetState(prtsetup.print_id))
    {
	if (prtsetup.send_to_printer != dxPrscPrint)
	{
	    prtsetup.send_to_printer = dxPrscPrint;
	    prtsetup.changed = prtsetup.changed | msend;
	}
    }
    else if (XmToggleButtonGetState(prtsetup.file_id))
    {
	if (prtsetup.send_to_printer != dxPrscFile)
	{
	    prtsetup.send_to_printer = dxPrscFile;
	    prtsetup.changed = prtsetup.changed | msend;
	}
    }
    else
    {
	if (prtsetup.send_to_printer != dxPrscBoth)
	{
	    prtsetup.send_to_printer = dxPrscBoth;
	    prtsetup.changed = prtsetup.changed | msend;
	}
    }

    /*
    ** Send to printer.
    */

    /*
    ** File name.
    */
    value = (char *)XmTextGetString(prtsetup.capture_file_id);
    if ((strcmp(value, prtsetup.capture_file) != 0))
	prtsetup.changed = prtsetup.changed | mfile;
    strcpy(prtsetup.capture_file, value);
    XtFree(value);

    /* page size */

    i = (XmToggleButtonGetState(prtsetup.letter_id) * DXmSIZE_LETTER +
	XmToggleButtonGetState(prtsetup.ledger_id) * DXmSIZE_LEDGER +
	XmToggleButtonGetState(prtsetup.legal_id) * DXmSIZE_LEGAL +
	XmToggleButtonGetState(prtsetup.exec_id) * DXmSIZE_EXECUTIVE +
	XmToggleButtonGetState(prtsetup.A5size_id) * DXmSIZE_A5 +
	XmToggleButtonGetState(prtsetup.A4size_id) * DXmSIZE_A4 +
	XmToggleButtonGetState(prtsetup.A3size_id) * DXmSIZE_A3 +
	XmToggleButtonGetState(prtsetup.B5size_id) * DXmSIZE_B5 +
	XmToggleButtonGetState(prtsetup.B4size_id) * DXmSIZE_B4);

    if ( i != prtsetup.page_size)
    {
	prtsetup.page_size = i;
	prtsetup.changed = prtsetup.changed | mpagesize;
    }

    /* sixel device */

    i = (XmToggleButtonGetState(prtsetup.vt_id) * Ids_TmpltVt240 +
	XmToggleButtonGetState(prtsetup.la50_id) * Ids_TmpltLa50 +
	XmToggleButtonGetState(prtsetup.la75_id) * Ids_TmpltLa75 +
	XmToggleButtonGetState(prtsetup.la100_id) * Ids_TmpltLa100 +
	XmToggleButtonGetState(prtsetup.la210_id) * Ids_TmpltLa210 +
	XmToggleButtonGetState(prtsetup.ln03_id) * Ids_TmpltLn03s +
	XmToggleButtonGetState(prtsetup.lj250_id) * Ids_TmpltLj250 +
	XmToggleButtonGetState(prtsetup.lj250lr_id) * Ids_TmpltLj250Lr +
	XmToggleButtonGetState(prtsetup.lcg01_id) * Ids_TmpltLcg01);

    if ( i != prtsetup.sixel_printer)
    {
	prtsetup.sixel_printer = i;
	prtsetup.changed = prtsetup.changed | msixelprinter;
    }

    return;
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
*/
{
    char	svalue[256];
    int		value[4];
    int		size;

    /* printer color */
    size = sm_get_any_resource(pcolor, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.grey_id)) == 0)
	prtsetup.color = dxPrscPrinterGrey;
    else if (strcmp(svalue, get_label_string(prtsetup.color_id)) == 0)
	prtsetup.color = dxPrscPrinterColor;
    else
	prtsetup.color = dxPrscPrinterBW;

    /* screen saver */
    size = sm_get_any_resource(psaver, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.negative_id)) == 0)
	prtsetup.saver = dxPrscNegative;
    else
	prtsetup.saver = dxPrscPositive;

    /* printer format */
    size = sm_get_any_resource(pformat, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.sixel_id)) == 0)
	prtsetup.format = dxPrscSixel;
    else if (strcmp(svalue, get_label_string(prtsetup.bitmap_id)) == 0)
	prtsetup.format = dxPrscDDIF;
    else
	prtsetup.format = dxPrscPostscript;

    /* fit-to-page options */
    size = sm_get_any_resource(pfit, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.reduce_id)) == 0)
	prtsetup.fit = dxPrscReduce;
    else if (strcmp(svalue, get_label_string(prtsetup.scale_id)) == 0)
	prtsetup.fit = dxPrscScale;
    else if (strcmp(svalue, get_label_string(prtsetup.shrink_id)) == 0)
	prtsetup.fit = dxPrscShrink;
    else if (strcmp(svalue, get_label_string(prtsetup.grow_id)) == 0)
	prtsetup.fit = dxPrscGrow;
    else
	prtsetup.fit = dxPrscCrop;

    /* orientation */
    size = sm_get_any_resource(porientation, svalue, value);
    if (strcmp(svalue, get_label_string(prtsetup.portrait_id)) == 0)
	prtsetup.orientation = dxPrscPortrait;
    else if (strcmp(svalue, get_label_string(prtsetup.landscape_id)) == 0)
	prtsetup.orientation = dxPrscLandscape;
    else
	prtsetup.orientation = dxPrscBestFit;


    /* capture method */
    size = sm_get_any_resource(pcapture, svalue, value);
    if (strcmp(svalue, get_label_string(prtsetup.entire_id)) == 0)
	prtsetup.capture_method = dxPrscEntire;
    else
	prtsetup.capture_method = dxPrscPartial;

    /* send to printer */
    size = sm_get_any_resource(psend, svalue, value);
    if (strcmp(svalue, get_label_string(prtsetup.both_id)) == 0)
	prtsetup.send_to_printer = dxPrscBoth;
    else if (strcmp(svalue, get_label_string(prtsetup.print_id)) == 0)
	prtsetup.send_to_printer = dxPrscPrint;
    else
	prtsetup.send_to_printer = dxPrscFile;


    /* page size */
    size = sm_get_any_resource(psize, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.letter_id)) == 0)
	prtsetup.page_size = DXmSIZE_LETTER;
    else if (strcmp(svalue, get_label_string(prtsetup.ledger_id)) == 0)
	prtsetup.page_size = DXmSIZE_LEDGER;
    else if (strcmp(svalue, get_label_string(prtsetup.legal_id)) == 0)
	prtsetup.page_size = DXmSIZE_LEGAL;
    else if (strcmp(svalue, get_label_string(prtsetup.exec_id)) == 0)
	prtsetup.page_size = DXmSIZE_EXECUTIVE;
    else if (strcmp(svalue, get_label_string(prtsetup.A5size_id)) == 0)
	prtsetup.page_size = DXmSIZE_A5;
    else if (strcmp(svalue, get_label_string(prtsetup.A4size_id)) == 0)
	prtsetup.page_size = DXmSIZE_A4;
    else if (strcmp(svalue, get_label_string(prtsetup.A3size_id)) == 0)
	prtsetup.page_size = DXmSIZE_A3;
    else if (strcmp(svalue, get_label_string(prtsetup.B5size_id)) == 0)
	prtsetup.page_size = DXmSIZE_B5;
    else
	prtsetup.page_size = DXmSIZE_B4;

    /* sixel device */
    size = sm_get_any_resource(psixel, svalue, value);

    if (strcmp(svalue, get_label_string(prtsetup.vt_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltVt240;
    else if (strcmp(svalue, get_label_string(prtsetup.la50_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLa50;
    else if (strcmp(svalue, get_label_string(prtsetup.la75_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLa75;
    else if (strcmp(svalue, get_label_string(prtsetup.la100_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLa100;
    else if (strcmp(svalue, get_label_string(prtsetup.la210_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLa210;
    else if (strcmp(svalue, get_label_string(prtsetup.ln03_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLn03s;
    else if (strcmp(svalue, get_label_string(prtsetup.lj250_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLj250;
    else if (strcmp(svalue, get_label_string(prtsetup.lj250lr_id)) == 0)
	prtsetup.sixel_printer = Ids_TmpltLj250Lr;
    else
	prtsetup.sixel_printer = Ids_TmpltLcg01;

    /* delay */
    size = sm_get_any_resource(pdelay, svalue, value);
    prtsetup.delay = value[0];

    /* capture screen file name */
    size = sm_get_any_resource(pfile, svalue, value);
    strcpy(prtsetup.capture_file, svalue);

    if ((prtsetup.color * prtsetup.saver * prtsetup.format *
	prtsetup.fit * prtsetup.orientation * prtsetup.page_size *
	prtsetup.sixel_printer) == 0)
    {
	/* error exit with "Bad Resource" message or some such */
    }
    return;
}

int prtattr_set_values()
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
*/
{
    char		temp[256];
    unsigned int	status;


    prtsetup.changed = 0;

    /* ribbon or toner saver */
    if (prtsetup.saver == dxPrscPositive)
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


    /* The setting of output color can affect Toner Saver setting, so
       always set this after toner saver has been set */
    if (prtsetup.color == dxPrscPrinterColor)
    /* Toggle Color output option on */
    {
	XmToggleButtonSetState(prtsetup.color_id, 1, 1);
	XmToggleButtonSetState(prtsetup.grey_id,0, 1);
	XmToggleButtonSetState(prtsetup.bw_id,0, 1);
    }
    else if (prtsetup.color == dxPrscPrinterGrey)
    /* Toggle Greyscale output option on */
    {
	XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	XmToggleButtonSetState(prtsetup.grey_id,1, 1);
	XmToggleButtonSetState(prtsetup.bw_id,0, 1);
    }
    else
    /* Toggle Black & White output option on */
    {
	XmToggleButtonSetState(prtsetup.color_id, 0, 1);
	XmToggleButtonSetState(prtsetup.grey_id,0, 1);
	XmToggleButtonSetState(prtsetup.bw_id,1, 1);
    }


    /* information format */
    /* The setting of output format can affect other settings, so
       always set this last */
    if (prtsetup.format == dxPrscPostscript)
    {
	XmToggleButtonSetState(prtsetup.postscript_id, 1, 1);
	XmToggleButtonSetState(prtsetup.bitmap_id,0, 1);
	XmToggleButtonSetState(prtsetup.sixel_id,0, 1);
    }
    else if (prtsetup.format == dxPrscSixel)
    {
	XmToggleButtonSetState(prtsetup.postscript_id, 0, 1);
	XmToggleButtonSetState(prtsetup.bitmap_id,0, 1);
	XmToggleButtonSetState(prtsetup.sixel_id,1, 1);
    }
    else
    {
	XmToggleButtonSetState(prtsetup.postscript_id, 0, 1);
	XmToggleButtonSetState(prtsetup.bitmap_id,1, 1);
	XmToggleButtonSetState(prtsetup.sixel_id,0, 1);
    }

    /* fit-to-page */

    switch (prtsetup.fit)
    {
    case dxPrscGrow:
	XmToggleButtonSetState(prtsetup.shrink_id,1,1);
	break;
    case dxPrscShrink:
	XmToggleButtonSetState(prtsetup.grow_id,1,1);
	break;
    case dxPrscScale:
	XmToggleButtonSetState(prtsetup.scale_id,1,1);
	break;
    case dxPrscReduce:
	XmToggleButtonSetState(prtsetup.reduce_id,1,1);
	break;
    case dxPrscCrop:
	XmToggleButtonSetState(prtsetup.crop_id,1,1);
	break;
    }

    sixel_setup();	/* Disable fit options if sixel format selected */

    /* orientation */

    switch (prtsetup.orientation)
    {
    case dxPrscPortrait:
	XmToggleButtonSetState(prtsetup.portrait_id,1,1);
	break;
    case dxPrscLandscape:
	XmToggleButtonSetState(prtsetup.landscape_id,1,1);
	break;
    case dxPrscBestFit:
	XmToggleButtonSetState(prtsetup.best_id,1,1);
	break;
    }

    switch (prtsetup.capture_method)
    {
    case dxPrscEntire:
	XmToggleButtonSetState(prtsetup.entire_id,1,1);
	break;
    case dxPrscPartial:
    default:
	XmToggleButtonSetState(prtsetup.partial_id,1,1);
	break;
    }

    switch (prtsetup.send_to_printer)
    {
    case dxPrscFile:
	XmToggleButtonSetState(prtsetup.file_id,1,1);
	break;
    case dxPrscBoth:
	XmToggleButtonSetState(prtsetup.both_id,1,1);
	break;
    case dxPrscPrint:
	XmToggleButtonSetState(prtsetup.print_id,1,1);
	break;
    }

    /* page size */

    reset_page_size();

    /* sixel device */

    reset_sixel_printer();

    /* delay */

    XmScaleSetValue( prtsetup.delay_id, prtsetup.delay );

    return;
}

int prt_put_attrs ()
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
*/
{
    char		*value;
    unsigned int	ivalue,status;
    char		astring[10];	

    /* ribbon or toner saver */
    if ((prtsetup.changed & msaver) != 0)
    {
	if (prtsetup.saver == dxPrscPositive)
	    sm_put_resource(psaver,get_label_string(prtsetup.positive_id));
	else
	    sm_put_resource(psaver,get_label_string(prtsetup.negative_id));
    }

    /* storage format */
    if ((prtsetup.changed & mformat) != 0)
    {
	switch (prtsetup.format)
	{
	case dxPrscPostscript:
	    sm_put_resource(pformat,get_label_string(prtsetup.postscript_id));
	    break;
	case dxPrscSixel:
	    sm_put_resource(pformat,get_label_string(prtsetup.sixel_id));
	    break;
	case dxPrscDDIF:
	    sm_put_resource(pformat,get_label_string(prtsetup.bitmap_id));
	    break;
	}
    }

    /* color */
    if ((prtsetup.changed & mcolor) != 0)
    {
	switch (prtsetup.color)
	{
	case dxPrscPrinterBW:
	    sm_put_resource(pcolor,get_label_string(prtsetup.bw_id));
	    break;
	case dxPrscPrinterGrey:
	    sm_put_resource(pcolor,get_label_string(prtsetup.grey_id));
	    break;
	case dxPrscPrinterColor:
	    sm_put_resource(pcolor,get_label_string(prtsetup.color_id));
	    break;
	}
    }

    /* capture screen filename */
    if ((prtsetup.changed & mfile) != 0)
    {
	sm_put_resource(pfile, prtsetup.capture_file);
    }


    /* delay */
    if ((prtsetup.changed & mdelay) != 0)
	sm_put_int_resource(pdelay, prtsetup.delay);

    /* fit-to-paper */
    if ((prtsetup.changed & mfit) != 0)
    {
	switch (prtsetup.fit)
	{
	case dxPrscGrow:
	    sm_put_resource(pfit,get_label_string(prtsetup.grow_id));
	    break;
	case dxPrscShrink:
	    sm_put_resource(pfit,get_label_string(prtsetup.shrink_id));
	    break;
	case dxPrscScale:
	    sm_put_resource(pfit,get_label_string(prtsetup.scale_id));
	    break;
	case dxPrscReduce:
	    sm_put_resource(pfit,get_label_string(prtsetup.reduce_id));
	    break;
	case dxPrscCrop:
	    sm_put_resource(pfit,get_label_string(prtsetup.crop_id));
	    break;
	}
    }

    /* orientation */
    if ((prtsetup.changed & morientation) != 0)
    {
	switch (prtsetup.orientation)
	{
	case dxPrscPortrait:
	    sm_put_resource
		(porientation, get_label_string (prtsetup.portrait_id));
	    break;
	case dxPrscLandscape:
	    sm_put_resource
		(porientation, get_label_string (prtsetup.landscape_id));
	    break;
	case dxPrscBestFit:
	    sm_put_resource
		(porientation, get_label_string (prtsetup.best_id));
	    break;
	}
    }

    /* capture type */
    if ((prtsetup.changed & mcapture) != 0)
    {
	switch (prtsetup.capture_method)
	{
	case dxPrscPartial:
	    sm_put_resource (pcapture, get_label_string (prtsetup.partial_id));
	    break;
	case dxPrscEntire:
	    sm_put_resource (pcapture, get_label_string (prtsetup.entire_id));
	    break;
	}
    }

    /* send to printer */
    if ((prtsetup.changed & msend) != 0)
    {
	switch (prtsetup.send_to_printer)
	{
	case dxPrscFile:
	    sm_put_resource (psend, get_label_string (prtsetup.file_id));
	    break;
	case dxPrscPrint:
	    sm_put_resource (psend, get_label_string (prtsetup.print_id));
	    break;
	case dxPrscBoth:
	    sm_put_resource (psend, get_label_string (prtsetup.both_id));
	    break;
	}
    }

    /* page size */
    if ((prtsetup.changed & mpagesize) != 0)
    {
	switch (prtsetup.page_size)
	{
	case DXmSIZE_LETTER:
	    sm_put_resource(psize,get_label_string(prtsetup.letter_id));
	    break;
	case DXmSIZE_LEDGER:
	    sm_put_resource(psize,get_label_string(prtsetup.ledger_id));
	    break;
	case DXmSIZE_LEGAL:
	    sm_put_resource(psize,get_label_string(prtsetup.legal_id));
	    break;
	case DXmSIZE_EXECUTIVE:
	    sm_put_resource(psize,get_label_string(prtsetup.exec_id));
	    break;
	case DXmSIZE_A5:
	    sm_put_resource(psize,get_label_string(prtsetup.A5size_id));
	    break;
	case DXmSIZE_A4:
	    sm_put_resource(psize,get_label_string(prtsetup.A4size_id));
	    break;
	case DXmSIZE_A3:
	    sm_put_resource(psize,get_label_string(prtsetup.A3size_id));
	    break;
	case DXmSIZE_B5:
	    sm_put_resource(psize,get_label_string(prtsetup.B5size_id));
	    break;
	case DXmSIZE_B4:
	    sm_put_resource(psize,get_label_string(prtsetup.B4size_id));
	    break;
	}
    }

    /* sixel device */
    if ((prtsetup.changed & msixelprinter) != 0)
    {
	switch (prtsetup.sixel_printer)
	{
	case Ids_TmpltVt240:
	    sm_put_resource(psixel,get_label_string(prtsetup.vt_id));
	    break;
	case Ids_TmpltLa50:
	    sm_put_resource(psixel,get_label_string(prtsetup.la50_id));
	    break;
	case Ids_TmpltLa75:
	    sm_put_resource(psixel,get_label_string(prtsetup.la75_id));
	    break;
	case Ids_TmpltLa100:
	    sm_put_resource(psixel,get_label_string(prtsetup.la100_id));
	    break;
	case Ids_TmpltLa210:
	    sm_put_resource(psixel,get_label_string(prtsetup.la210_id));
	    break;
	case Ids_TmpltLn03s:
	    sm_put_resource(psixel,get_label_string(prtsetup.ln03_id));
	    break;
	case Ids_TmpltLj250:
	    sm_put_resource(psixel,get_label_string(prtsetup.lj250_id));
	    break;
	case Ids_TmpltLj250Lr:
	    sm_put_resource(psixel,get_label_string(prtsetup.lj250lr_id));
	    break;
	case Ids_TmpltLcg01:
	    sm_put_resource(psixel,get_label_string(prtsetup.lcg01_id));
	    break;
	}
    }

    if (prtsetup.changed != 0)
    {
	smdata.resource_changed = 1;
	prtsetup.changed = 0;
    }
    return;
}

static char *get_label_string
#if _PRDW_PROTO_
(
    Widget	awidget
)
#else
(awidget)
    Widget	awidget;
#endif
{
    XmString	*comp_str;
    Arg		arg[2];
    char	**string = (char **)&arg[1];
    long	byte_count, cvt_status;

    /*
    ** Get a pointer to the label string of the toggle button
    */
    XtVaGetValues (awidget, XmNlabelString, &comp_str, NULL);

    /*
    ** Convert the compound label string to a null-terminated ASCII string
    ** return the comp string to the heap and return a pointer to the string
    */
    *string = (char *)DXmCvtCStoFC(comp_str, &byte_count, &cvt_status);

    strcpy (temp, *string);
    XmStringFree (comp_str);
    XtFree (*string);
    return (temp);
}

void PSDDIFCallback
#if _PRDW_PROTO_
(
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
)
#else
(widget, reason, data)
    Widget				*widget;
    unsigned int			reason;
    XmToggleButtonCallbackStruct	*data;
#endif
{
    XtSetSensitive(prtsetup.shrink_id, !data->set);
    XtSetSensitive(prtsetup.grow_id, !data->set);
    XtSetSensitive(prtsetup.reduce_id, !data->set);
    XtSetSensitive(prtsetup.scale_id, !data->set);
    XtSetSensitive(prtsetup.crop_id, !data->set);

    XtSetSensitive(prtsetup.portrait_id, !data->set);
    XtSetSensitive(prtsetup.landscape_id, !data->set);
    XtSetSensitive(prtsetup.best_id, !data->set);
}

void	PSColorCallback
#if _PRDW_PROTO_
(
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
)
#else
(widget, reason, data)
    Widget				*widget;
    unsigned int			reason;
    XmToggleButtonCallbackStruct	*data;
#endif
{
    /* If Color option has been selected, turn off negative option	*/
    if (data->set == 1)
    {
	if (XmToggleButtonGetState (prtsetup.negative_id) == 1)
	{
	    XmToggleButtonSetState(prtsetup.negative_id, 0, 1);
	    XmToggleButtonSetState(prtsetup.positive_id, 1, 1);
	}
	XtSetSensitive(prtsetup.negative_id, False);    
    }
    else
    /* Color option has been deselected, so enable negative option	   */
    {
	XtSetSensitive(prtsetup.negative_id, True);    
    }
}

void	PSPostCallback
#if _PRDW_PROTO_
(
    Widget				*widget,
    unsigned int			reason,
    XmToggleButtonCallbackStruct	*data
)
#else
(widget, reason, data)
    Widget				*widget;
    unsigned int			reason;
    XmToggleButtonCallbackStruct	*data;
#endif
/* This function is called whenever the PostScript output option is */
/* toggled on, or toggled off.					    */
{
    int	screen_type;

    /* If the PostScript button was just selected, limit some of the options */

    sixel_setup();

    if (!data->set)
    /* PostScript output was toggled off.  Enable color output & non 1:1    */
    /* aspect ratio.							    */
    {
	/*  Enable color or greyscale output options. */
	XtSetSensitive(prtsetup.grey_id, True);    
	XtSetSensitive(prtsetup.color_id, True);    
    }
}

static void sixel_setup ()
{
    XtSetSensitive(prtsetup.shrink_id, True);
    XtSetSensitive(prtsetup.grow_id, True);
    XtSetSensitive(prtsetup.reduce_id, True);
    XtSetSensitive(prtsetup.scale_id, True);
    XtSetSensitive(prtsetup.crop_id, True);
}

int reset_page_size ()
{
    switch (prtsetup.page_size)
    {
    case DXmSIZE_LETTER:
	XmToggleButtonSetState(prtsetup.letter_id,1,1);
	break;
    case DXmSIZE_LEDGER:
	XmToggleButtonSetState(prtsetup.ledger_id,1,1);
	break;
    case DXmSIZE_LEGAL:
	XmToggleButtonSetState(prtsetup.legal_id,1,1);
	break;
    case DXmSIZE_EXECUTIVE:
	XmToggleButtonSetState(prtsetup.exec_id,1,1);
	break;
    case DXmSIZE_A5:
	XmToggleButtonSetState(prtsetup.A5size_id,1,1);
	break;
    case DXmSIZE_A4:
	XmToggleButtonSetState(prtsetup.A4size_id,1,1);
	break;
    case DXmSIZE_A3:
	XmToggleButtonSetState(prtsetup.A3size_id,1,1);
	break;
    case DXmSIZE_B5:
	XmToggleButtonSetState(prtsetup.B5size_id,1,1);
	break;
    case DXmSIZE_B4:
	XmToggleButtonSetState(prtsetup.B4size_id,1,1);
	break;
    }
}

int reset_sixel_printer ()
{
    switch (prtsetup.sixel_printer)
    {
    case Ids_TmpltVt240:
	XmToggleButtonSetState(prtsetup.vt_id,1,1);
	break;
    case Ids_TmpltLa50:
	XmToggleButtonSetState(prtsetup.la50_id,1,1);
	break;
    case Ids_TmpltLa75:
	XmToggleButtonSetState(prtsetup.la75_id,1,1);
	break;
    case Ids_TmpltLa100:
	XmToggleButtonSetState(prtsetup.la100_id,1,1);
	break;
    case Ids_TmpltLa210:
	XmToggleButtonSetState(prtsetup.la210_id,1,1);
	break;
    case Ids_TmpltLn03s:
	XmToggleButtonSetState(prtsetup.ln03_id,1,1);
	break;
    case Ids_TmpltLj250:
	XmToggleButtonSetState(prtsetup.lj250_id,1,1);
	break;
    case Ids_TmpltLj250Lr:
	XmToggleButtonSetState(prtsetup.lj250lr_id,1,1);
	break;
    case Ids_TmpltLcg01:
	XmToggleButtonSetState(prtsetup.lcg01_id,1,1);
	break;
    }
}
