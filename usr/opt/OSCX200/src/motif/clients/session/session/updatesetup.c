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
#include "smdata.h"
#include "smresource.h"


int	updatesetup()
{
/* update the structure for session manager attributes */
smattr_get_values();

/* if the dialog is created, update the widgets to the correct values */
if (smsetup.sm_attr_id != 0)
    {
    smattr_set_values();
    }

#ifdef DOPRINT
/* update the structure for printer attributes */
prtattr_get_values();
if (prtsetup.prt_attr_id != 0)
    {
    prtattr_set_values();
    }
#endif

/* update the structure for keyboard attributes */
keyattr_get_values();
if (keysetup.key_attr_id != 0)
	{
	keyattr_set_values();
	}

/* update the structure for pointer attributes */
/* if this is visible, we had better free some color cells */
if (pointsetup.point_attr_id != 0)
    {
    if (XtIsManaged(pointsetup.point_attr_id))
	{
	/* if this is visible, we had better free some color cells */
	free_allocated_color(&pointsetup.foreground);
	free_allocated_color(&pointsetup.background);
	}
    }
pointattr_get_values();
if (pointsetup.point_attr_id != 0)
    	{
	/* set the right button values to on and off for the dialog */
	pointattr_set_values();
	}

/* update the structure for window attributes*/
/* if this is visible, we had better free some color cells */
if (windowsetup.window_attr_id != 0)
    {
    if (XtIsManaged(windowsetup.window_attr_id))
	{
	free_allocated_color(&windowsetup.screen_foreground);
	free_allocated_color(&windowsetup.screen_background);
	free_allocated_color(&windowsetup.foreground);
	free_allocated_color(&windowsetup.background);
	free_allocated_color(&windowsetup.activetitle);
	free_allocated_color(&windowsetup.inactivetitle);
	}
    }

windowattr_get_values();
if (windowsetup.window_attr_id != 0)
	{
	/* set the right button values to on and off for the dialog */
	windowattr_set_values();
	}

get_customized_menu(smdata.create_menu);
if (appmenusetup.menu_attr_id != 0)
    /* set the list box correctly for the dialog */
    if (XtIsManaged(appmenusetup.menu_attr_id))
	{
	ResetListbox();
	}
if (appdefsetup.menu_attr_id != 0)
    /* set the list box correctly for the dialog */
    if (XtIsManaged(appdefsetup.menu_attr_id))
	{
	ResetAppDefListbox();
	}

if (autostartsetup.menu_attr_id != 0)
    /* set the list box correctly for the dialog */
    if (XtIsManaged(autostartsetup.menu_attr_id))
	{
	ResetASListbox();
	}

/* update the structure for multi head attributes */
screenattr_get_values();
/* if the dialog is created, update the widgets to the correct values */
if (screensetup.scr_attr_id != 0)
    {
    screenattr_set_values();
    }

/* update the structure for international attributes */
interattr_get_values();

/* if the dialog is created, update the widgets to the correct values */
if (intersetup.inter_attr_id != 0)
    {
    interattr_set_values();
    }
}
