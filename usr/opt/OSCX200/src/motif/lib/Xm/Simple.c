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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Simple.c,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:50:11 $"
#endif
#endif
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <stdio.h>
#include <X11/StringDefs.h>
#include <Xm/RowColumnP.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

static void EvaluateConvenienceStructure() ;

#else

static void EvaluateConvenienceStructure( 
                        Widget wid,
                        XmSimpleMenu sm) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/

static XtResource SimpleMenuResources[] =
{
	{ XmNbuttonCount, XmCButtonCount, XmRInt, sizeof(int),
	  XtOffsetOf( struct _XmSimpleMenuRec, count),
	  XmRImmediate, (XtPointer) 0
	},
	{ XmNpostFromButton, XmCPostFromButton, XmRInt, sizeof(int),
	  XtOffsetOf( struct _XmSimpleMenuRec, post_from_button),
	  XmRImmediate, (XtPointer) -1
	},
	{ XmNsimpleCallback, XmCCallback, XmRCallbackProc, 
	  sizeof(XtCallbackProc), XtOffsetOf( struct _XmSimpleMenuRec, callback),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttons, XmCButtons, XmRXmStringTable,
	  sizeof(XmStringTable), XtOffsetOf( struct _XmSimpleMenuRec, label_string),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonAccelerators, XmCButtonAccelerators, XmRStringTable, 
	  sizeof(String *), XtOffsetOf( struct _XmSimpleMenuRec,  accelerator),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonAcceleratorText, XmCButtonAcceleratorText, 
	  XmRXmStringTable, sizeof(XmStringTable),
	  XtOffsetOf( struct _XmSimpleMenuRec, accelerator_text),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonMnemonics, XmCButtonMnemonics, XmRKeySymTable,
	  sizeof(XmKeySymTable), XtOffsetOf( struct _XmSimpleMenuRec, mnemonic),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonMnemonicCharSets, XmCButtonMnemonicCharSets, 
	  XmRCharSetTable, sizeof(XmStringCharSetTable),
	  XtOffsetOf( struct _XmSimpleMenuRec, mnemonic_charset),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonType, XmCButtonType, XmRButtonType,
	  sizeof(XmButtonTypeTable), XtOffsetOf( struct _XmSimpleMenuRec, button_type),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNbuttonSet, XmCButtonSet, XmRInt,
	  sizeof(int), XtOffsetOf( struct _XmSimpleMenuRec, button_set),
	  XmRImmediate, (XtPointer) -1
	},
	{ XmNoptionLabel, XmCOptionLabel, XmRXmString,
	  sizeof(XmString), XtOffsetOf( struct _XmSimpleMenuRec, option_label),
	  XmRImmediate, (XtPointer) NULL
	},
	{ XmNoptionMnemonic, XmCOptionMnemonic, XmRKeySym,
	  sizeof (KeySym), XtOffsetOf( struct _XmSimpleMenuRec, option_mnemonic),
          XmRImmediate, (XtPointer) NULL
	},
};

static void 
#ifdef _NO_PROTO
EvaluateConvenienceStructure( wid, sm )
        Widget wid ;
        XmSimpleMenu sm ;
#else
EvaluateConvenienceStructure(
        Widget wid,
        XmSimpleMenu sm )
#endif /* _NO_PROTO */
{
        XmRowColumnWidget rc = (XmRowColumnWidget) wid ;
	int i, n;
	char name_buf[20];
	int button_count = 0;
	int separator_count = 0;
	int label_count = 0;
	Arg args[6];
	Widget child;
	XmButtonType btype;

	/* stop when we hit the NULL terminators in case the lists are shorter
	** than XmNbuttonCount claims
	*/
	Boolean	moreXmNbuttons = True;
	Boolean moreXmNbuttonAcceleratorText = True;
	Boolean moreXmNbuttonAccelerators =  True;
	Boolean moreXmNbuttonMnemonics = True;
	Boolean moreXmNbuttonMnemonicCharSets = True;
	Boolean moreXmNbuttonType = True;

	for (i = 0; i < sm->count; i++)
	{
		n = 0;
		if (moreXmNbuttons && sm->label_string && sm->label_string[i])
		{
			XtSetArg(args[n], XmNlabelString, sm->label_string[i]);
			n++;
		}
		else moreXmNbuttons =  False;
		if (moreXmNbuttonAccelerators && sm->accelerator && sm->accelerator[i])
		{
			XtSetArg(args[n], XmNaccelerator, sm->accelerator[i]);
			n++;
		}
		else moreXmNbuttonAccelerators = False;
		if (moreXmNbuttonAcceleratorText && sm->accelerator_text && sm->accelerator_text[i])
		{
			XtSetArg(args[n], XmNacceleratorText, 
				sm->accelerator_text[i]); 
			n++;
		}
		else moreXmNbuttonAcceleratorText = False;
		if (moreXmNbuttonMnemonics && sm->mnemonic && sm->mnemonic[i])
		{
			XtSetArg(args[n], XmNmnemonic, sm->mnemonic[i]);
			n++;
		}
		else moreXmNbuttonMnemonics = False;
		if (moreXmNbuttonMnemonicCharSets && sm->mnemonic_charset && sm->mnemonic_charset[i])
		{
			XtSetArg(args[n], XmNmnemonicCharSet, 
				sm->mnemonic_charset[i]); 
			n++;
		}
		else moreXmNbuttonMnemonicCharSets = False;
		
		/* Dynamic Defaulting of button type */

		if (moreXmNbuttonType && sm->button_type && sm->button_type[i])
			btype = sm->button_type[i];
		else
			{
			moreXmNbuttonType = False;
			btype = XmNONE;
			}

		if (btype == XmNONE)
		{
			if (rc->row_column.type == XmMENU_BAR)
				btype = XmCASCADEBUTTON;
			else
				btype = XmPUSHBUTTON;
		}
		
		switch (btype)
		{
			case XmTITLE:
				sprintf(name_buf,"label_%d", label_count++);
				child = XtCreateManagedWidget( name_buf,
                                     xmLabelGadgetClass, (Widget) rc, args, n);
			break;
			case XmDOUBLE_SEPARATOR:
				XtSetArg(args[n], XmNseparatorType, XmDOUBLE_LINE); n++;
			case XmSEPARATOR:
				sprintf(name_buf,"separator_%d", separator_count++);
				child = XtCreateManagedWidget(name_buf, 
                                 xmSeparatorGadgetClass, (Widget) rc, args, n);
			break;
			case XmPUSHBUTTON:
				sprintf(name_buf,"button_%d", button_count++);
				child = XtCreateManagedWidget(name_buf, 
                                                       xmPushButtonGadgetClass,
                                                         (Widget) rc, args, n);
				if (sm->callback)
					XtAddCallback(child,
                                             XmNactivateCallback, sm->callback,
                                               (XtPointer) (button_count - 1));
			break;
			case XmRADIOBUTTON:
				XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); n++;
			case XmCHECKBUTTON:
				sprintf(name_buf,"button_%d", button_count++);
				XtSetArg(args[n], XmNindicatorOn, TRUE); n++;
				child = XtCreateManagedWidget(name_buf,
                                                     xmToggleButtonGadgetClass,
                                                         (Widget) rc, args, n);
				if (sm->callback)
					XtAddCallback(child,
                                         XmNvalueChangedCallback, sm->callback,
                                               (XtPointer) (button_count - 1));
			break;
			case XmCASCADEBUTTON:
				sprintf(name_buf,"button_%d", button_count++);
				child = XtCreateManagedWidget(name_buf,
                                                    xmCascadeButtonGadgetClass,
                                                         (Widget) rc, args, n);
				if (sm->callback)
					XtAddCallback(child,
                                             XmNactivateCallback, sm->callback,
                                               (XtPointer) (button_count - 1));
			break;
			default:
				/* this is an error condition */
				;
			break;
		}
	}
}

Widget 
#ifdef _NO_PROTO
XmCreateSimpleMenuBar( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimpleMenuBar(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Widget rc;
	XmSimpleMenuRec mr;

	XtGetSubresources(parent, &mr, name, XmCSimpleMenuBar,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);

	rc = XmCreateMenuBar(parent, name, args, arg_count);

	EvaluateConvenienceStructure( rc, &mr);

	return(rc);
}

Widget 
#ifdef _NO_PROTO
XmCreateSimplePopupMenu( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimplePopupMenu(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Widget rc;
	XmSimpleMenuRec mr;

	XtGetSubresources(parent, &mr, name, XmCSimplePopupMenu,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);

	rc = XmCreatePopupMenu(parent, name, args, arg_count);

	EvaluateConvenienceStructure( rc, &mr);

	return(rc);
}

Widget 
#ifdef _NO_PROTO
XmCreateSimplePulldownMenu( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimplePulldownMenu(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Widget rc;
	XmSimpleMenuRec mr;
	int n, i;
	Arg local_args[3];
	WidgetList buttons;
	Cardinal num_buttons;

	XtGetSubresources(parent, &mr, name, XmCSimplePulldownMenu,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);
	
	rc = XmCreatePulldownMenu(parent, name, args, arg_count);

	EvaluateConvenienceStructure(rc, &mr);

	if (mr.post_from_button >= 0)
	{
		n = 0;
		XtSetArg(local_args[n], XtNchildren, &buttons); n++;
		XtSetArg(local_args[n], XtNnumChildren, &num_buttons); n++;
		XtGetValues(parent, local_args, n);

		if (!num_buttons)
		{
			/* error condition */
			return(rc);
		}
		else
		{
			for (i = 0; i < num_buttons; i++)
			{
				if (((XmIsCascadeButtonGadget(buttons[i])) ||
					(XmIsCascadeButton(buttons[i])))
					&&
					(i == mr.post_from_button))
					break;
			}

			if ( i < num_buttons)
			{
				n = 0;
				XtSetArg(local_args[n], XmNsubMenuId, rc); n++;
				XtSetValues(buttons[i], local_args, n);
			}
		}
	}
	return(rc);
}

Widget 
#ifdef _NO_PROTO
XmCreateSimpleOptionMenu( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimpleOptionMenu(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Widget rc, sub_rc;
	XmSimpleMenuRec mr;
	int n, i, button_count;
	Arg local_args[5];
	WidgetList buttons;
	Cardinal num_buttons;

	XtGetSubresources(parent, &mr, name, XmCSimpleOptionMenu,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);
	
	rc = XmCreateOptionMenu(parent, name, args, arg_count);

	sub_rc = XmCreatePulldownMenu(parent, name, args, arg_count);

	EvaluateConvenienceStructure(sub_rc, &mr);

	n = 0;
	if (mr.option_label)
	{
		XtSetArg(local_args[n], XmNlabelString, mr.option_label); n++;
	}
	if (mr.option_mnemonic)
	{
		XtSetArg(local_args[n], XmNmnemonic, mr.option_mnemonic); n++;
	}
	
	XtSetArg(local_args[n], XmNsubMenuId, sub_rc); n++;
	XtSetValues(rc, local_args, n);

	if (mr.button_set >= 0)
	{
		n = 0;
		XtSetArg(local_args[n], XtNchildren, &buttons); n++;
		XtSetArg(local_args[n], XtNnumChildren, &num_buttons); n++;
		XtGetValues(sub_rc, local_args, n);

		if (!num_buttons)
		{
			/* error condition */
			return(rc);
		}
		else
		{
			button_count = 0;
			for (i = 0; i < num_buttons; i++)
			{				/* count only PushB */
				if ((XmIsPushButtonGadget(buttons[i])) ||
					(XmIsPushButton(buttons[i])))
				{
					if (button_count == mr.button_set)
						break;
					button_count++;
				}
			}

			if ( i < num_buttons)
			{
				n = 0;
				XtSetArg(local_args[n], XmNmenuHistory, buttons[i]); n++;
				XtSetValues(rc, local_args, n);
			}
		}
	}

	return(rc);
}

Widget 
#ifdef _NO_PROTO
XmCreateSimpleRadioBox( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimpleRadioBox(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Arg local_args[5];
	Widget rc, child;
	int i, n;
	XmSimpleMenuRec mr;
	char name_buf[20];

	rc = XmCreateRadioBox(parent, name, args, arg_count);

	XtGetSubresources(parent, &mr, name, XmCSimpleRadioBox,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);

	for(i=0; i < mr.count; i++)
	{
		sprintf(name_buf,"button_%d", i);

		n = 0;
		if (mr.label_string && mr.label_string[i])
		{
			XtSetArg(local_args[n], 
				XmNlabelString, mr.label_string[i]); n++;
		}
		if (mr.button_set == i)
		{
			XtSetArg(local_args[n], XmNset, TRUE); n++;
		}
		child = XtCreateManagedWidget(name_buf, 
			xmToggleButtonGadgetClass, (Widget) rc, local_args, n);
		if (mr.callback)
			XtAddCallback(child, XmNvalueChangedCallback,
				mr.callback, (XtPointer) i);
	}
	
	return(rc);
}

Widget 
#ifdef _NO_PROTO
XmCreateSimpleCheckBox( parent, name, args, arg_count )
        Widget parent ;
        String name ;
        ArgList args ;
        Cardinal arg_count ;
#else
XmCreateSimpleCheckBox(
        Widget parent,
        String name,
        ArgList args,
        Cardinal arg_count )
#endif /* _NO_PROTO */
{
	Arg local_args[5];
	Widget rc, child;
	int i, n;
	XmSimpleMenuRec mr;
	char name_buf[20];


	rc = XmCreateRadioBox(parent, name, args, arg_count);

	n = 0;
        XtSetArg(local_args[n], XmNradioBehavior, FALSE); n++;

	XtSetValues(rc, local_args, n);
	

	XtGetSubresources(parent, &mr, name, XmCSimpleCheckBox,
		SimpleMenuResources, XtNumber(SimpleMenuResources), 
		args, arg_count);

	for(i=0; i < mr.count; i++)
	{
		sprintf(name_buf,"button_%d", i);

		n = 0;
		if (mr.label_string && mr.label_string[i])
		{
			XtSetArg(local_args[n], 
				XmNlabelString, mr.label_string[i]); n++;
		}
		child = XtCreateManagedWidget(name_buf,
			xmToggleButtonGadgetClass, (Widget) rc, local_args, n);
		if (mr.callback)
			XtAddCallback(child, XmNvalueChangedCallback,
				mr.callback, (XtPointer) i);
	}

	return(rc);
}
