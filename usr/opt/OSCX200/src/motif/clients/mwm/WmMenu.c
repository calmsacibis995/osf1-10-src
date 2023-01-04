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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmMenu.c,v $ $Revision: 1.1.5.4 $ $Date: 1993/08/23 19:40:57 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */
/*
 * (c) Copyright 1987, 1988 DIGITAL EQUIPMENT CORPORATION */
/*
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY */

/*
 * Included Files:
 */

#include "WmGlobal.h"
#include "WmResource.h"
#include "WmResParse.h"
#ifdef DEC_MOTIF_BUG_FIX
#include "WmCEvent.h"
#endif

#include <stdio.h>

#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MenuShell.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/RowColumnP.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#ifdef DEC_MOTIF_EXTENSION
#include <DXm/DECspecific.h>
#endif
#define SHELL_NAME "menu"
#define SEPARATOR_NAME "separator"
#define TITLE_NAME "title_name"
#define CASCADE_BTN_NAME "cascadebutton"
#define PUSH_BTN_NAME "pushbutton"

#define CHILDREN_CACHE  22
#define MENU_BUTTON_INC 5

/*
 * include extern functions
 */
#include "WmMenu.h"
#include "WmCDecor.h"
#include "WmColormap.h"
#include "WmEvent.h"
#include "WmFunction.h"
#include "WmIconBox.h"
#include "WmImage.h"
#include "WmError.h"

#ifdef AUTOMATION
#ifdef _NO_PROTO
static void SetMwmMenuInfo ();
static void SetMwmMenuWindow ();
#else
static void SetMwmMenuInfo (ClientData *pcd, MenuSpec *menuSpec);
static void SetMwmMenuWindow (ClientData *pcd, Widget menuWidget);
#endif /* _NO_PROTO */
#endif /* AUTOMATION */

#ifdef DEC_MOTIF_EXTENSION
#include "WmResNames.h"
#include "mwm_internal.h"
#include "mwm_util.h"
#endif

#ifdef DEC_MOTIF_EXTENSION
/* For getting menu resources on B&w, gray-scale monitors */
#define k_mwm_menu_fore 0
#define k_mwm_menu_back 1
#define k_mwm_menu_colors 4
#define k_mwm_menu_pixmaps 3
#define k_mwm_menu_prefix "menu."
#define k_mwm_dot_str "."
static struct
  {
    Pixel col[ k_mwm_menu_colors ];
    char pix[ k_mwm_menu_pixmaps ][ 64 ];  
    char col_name[ k_mwm_menu_colors ][ 64 ];
    char pix_name[ k_mwm_menu_pixmaps ][ 64 ];     
    Pixmap pixmap[ k_mwm_menu_pixmaps ];
    Boolean col_init[ k_mwm_menu_colors ];
    Boolean pix_init[ k_mwm_menu_pixmaps ];                  
  } mwm_menu_res = { 0, 0, 0, 0, "", "", "",
                     WmNforeground, WmNbackground,
                     WmNtopShadowColor, WmNbottomShadowColor,
                     WmNbackgroundPixmap,
                     WmNtopShadowPixmap, WmNbottomShadowPixmap,
                     (Pixmap)NULL, (Pixmap)NULL, (Pixmap)NULL,
                     FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE };
#endif /* DEC_MOTIF_EXTENSION */



/*************************************<->*************************************
 *
 *  MakeMenu (menuName, initialContext, accelContext, moreMenuItems,
 *            fSystemMenu)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a menu widget.
 *
 *
 *  Inputs:
 *  ------
 *  menuName       = name of the top-level menu pane for the menu
 *  initialContext = initial context for menuitem sensitivity
 *  accelContext   = accelerator context
 *  moreMenuItems  = additional menuitems for custom menu.
 *  fSystemMenu    = TRUE iff the menu is a client system menu.
 *
 * 
 *  Outputs:
 *  -------
 *  Return = pointer to a MenuSpec structure with updated currentContext,
 *           menuWidget, and menuButtons members.
 *
 *
 *  Comments:
 *  --------
 *  If moreMenuItems is nonNULL, a custom MenuSpec will be created, with
 *  menuItem member pointing to moreMenuItems.  The menuItems for the
 *  standard MenuSpec of the same name and the moreMenuItems list will be 
 *  used to create menubuttons, and the menu widget will be separate from 
 *  any existing standard menu widget.
 *
 *  When the client is destroyed, this custom MenuSpec, its menuItem and
 *  menuButton lists, and its menu widget should be freed.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
MenuSpec *
MakeMenu (pSD, menuName, initialContext, accelContext, 
	  moreMenuItems, fSystemMenu)

    WmScreenData *pSD;
    String    menuName;
    Context   initialContext;
    Context   accelContext;
    MenuItem *moreMenuItems;
    Boolean   fSystemMenu;
#else /* _NO_PROTO */
MenuSpec *MakeMenu (WmScreenData *pSD, String menuName, Context initialContext,
		    Context accelContext, MenuItem *moreMenuItems,
		    Boolean fSystemMenu)
#endif /* _NO_PROTO */
{
    unsigned int n;
    MenuSpec     *menuSpec;
    MenuSpec     *newMenuSpec;
    MenuItem     *menuItem;
    KeySpec      *accelKeySpec;
    
    if ((menuName == NULL) || (pSD == NULL))
    {
	return (NULL);
    }

    /*
     * Look for the menu specification:
     */

    menuSpec = pSD->menuSpecs;
    while (menuSpec)
    {
	if ((menuSpec->name != NULL) && !strcmp (menuSpec->name, menuName))
	/* Found the menu pane. */
	{
	    break;
	}
	menuSpec = menuSpec->nextMenuSpec;
    }
    
    if (menuSpec == NULL)
    /* the menuSpecs list is exhausted */
    {
	MWarning("Menu specification %s not found\n", menuName);
	return (NULL);
    }

    /*
     * The top-level menu pane specification was found.
     * Adjust the menu accelerator context?
     */

    if (fSystemMenu)
    {
	accelContext = 0;
    }
    else if (accelContext & F_CONTEXT_ROOT)
    /* root context accelerators apply everywhere */
    {
	accelContext = F_CONTEXT_ALL;
    }

    /*
     * If making a custom menu, create a custom copy of the specification with 
     *   which to build the custom menu.
     * Otherwise, if the menu widget exists, possibly modify the accelerator
     *   contexts and return the specification.
     */

    if (moreMenuItems != NULL)
    {
        if ((newMenuSpec = (MenuSpec *) XtMalloc (sizeof (MenuSpec))) == NULL)
	/* Handle insufficent memory */
        {
            MWarning("Insufficient memory for menu %s\n", menuName);
	    return (NULL);
        }
	newMenuSpec->name = NULL;  /* distinguishes this as custom */
	newMenuSpec->whichButton = SELECT_BUTTON;
	newMenuSpec->height = 0;
	newMenuSpec->menuItems = menuSpec->menuItems; /* temporary */
	newMenuSpec->accelContext = menuSpec->accelContext;
	newMenuSpec->accelKeySpecs = NULL;
	newMenuSpec->nextMenuSpec = NULL;
	menuSpec = newMenuSpec;
    }
    else if (menuSpec->menuWidget)
    {
	/* 
	 * OR the accelContext into the accelerators, if necessary.
	 */
        if (accelContext != (menuSpec->accelContext & accelContext))
        {
            menuSpec->accelContext |= accelContext;
	    accelKeySpec = menuSpec->accelKeySpecs;
	    while (accelKeySpec)
	    {
		accelKeySpec->context |= accelContext;
	        accelKeySpec = accelKeySpec->nextKeySpec;
	    }
	}
        return (menuSpec);
    }

    /*
     * We have a menu specification with which to build the menu.
     * Set the initial and accelerator contexts -- they are needed within 
     *   CreateMenuWidget.
     */

    menuSpec->currentContext = initialContext;
    menuSpec->accelContext = accelContext;

    /*
     * Scan the toplevel MenuSpec and create its initial menuButtons array
     * if any of its items will need to be included.  This array will be
     * created or enlarged within CreateMenuWidget below if necessary.
     */

    n = 0;
    menuItem = menuSpec->menuItems;
    while (menuItem)
    {
	if ((menuItem->greyedContext) || (menuItem->mgtMask))
	{
	    n++;
	}
	menuItem = menuItem->nextMenuItem;
    }
    menuItem = moreMenuItems;
    while (menuItem)
    {
	if ((menuItem->greyedContext) || (menuItem->mgtMask))
	{
	    n++;
	}
	menuItem = menuItem->nextMenuItem;
    }
    if (n)
    {
        if ((menuSpec->menuButtons =
	       (MenuButton *) XtMalloc (n * sizeof(MenuButton))) == NULL)
        /* insufficent memory */
	{                  
            MWarning("Insufficient memory for menu %s\n", menuName);
	    return (NULL);
	}
        menuSpec->menuButtonSize = n;
    }
    else
    {
        menuSpec->menuButtons = NULL;
        menuSpec->menuButtonSize = 0;
    }
    menuSpec->menuButtonCount = 0;

    /*
     * Create a PopupShell widget as a child of the workspace manager widget
     *   and a PopupMenu as a child of the shell.
     * Fill the PopupMenu with the menu items.
     */

    menuSpec->menuWidget = CreateMenuWidget (pSD, menuName, 
					     pSD->screenTopLevelW,
					     TRUE, menuSpec, moreMenuItems);
    if (menuSpec->menuWidget == NULL)
    {
        /*
	 *  Could not make the top-level menu pane.
	 */
	return (NULL);
    }
/*
    _XmSetPopupMenuClick(menuSpec->menuWidget, False); 
*/
    /* Return the top MenuSpec */

    return (menuSpec);

} /* END OF FUNCTION MakeMenu */



/*************************************<->***********************************/
void CheckTerminalSeparator(menuSpec, buttonWidget, manage)
     MenuSpec *menuSpec;
     Widget buttonWidget;
     Boolean manage;
{
    CompositeWidget cw;
    WidgetList      children;
    Cardinal        wPos;


    if ((menuSpec == NULL) || (menuSpec->menuWidget == NULL))
      {
	 return;
      }

    cw = (CompositeWidget)menuSpec->menuWidget;
    children = cw->composite.children;

    for (wPos = 0;  wPos < cw->composite.num_children; wPos++)
    {
	if((Widget)children[wPos] == buttonWidget)
	{
	    break;
	}
    }
    
    
    if(wPos > 0 &&
       XtClass((Widget) children[wPos -1]) == xmSeparatorGadgetClass)
    {
	if(manage)
	{
	    if (!(XtIsManaged((Widget)children[wPos -1])))
	    {
		XtManageChild((Widget)children[wPos -1]);
	    }
	}
	else
	{
	    if (XtIsManaged((Widget)children[wPos -1]))
	    {
		XtUnmanageChild((Widget)children[wPos -1]);
	    }
	}
    }

} /* END OF FUNCTION CheckTerminalSeparator */



/*************************************<->*************************************
 *
 *  static Boolean
 *  AdjustPBs (menuSpec, pCD, newContext)
 *
 *
 *  Description:
 *  -----------
 *  This procedure adjusts menu PushButton sensitivities and manage/unmanaged
 *  status for a toplevel menu.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =    nonNULL toplevel menu specification with gadget
 *  pCD =         client data
 *  newContext =  context that the menu is to be posted under.
 *
 * 
 *  Outputs:
 *  -------
 *  menuSpec =      menu specification with modifications
 *  Return =        TRUE iff at least one menu item changed manage status.
 *
 *
 *  Comments:
 *  --------
 *  Adjusts PushButton sensitivity according to context and function type.
 *  Manages/Unmanages PushButtons according to clientFunction resource.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
static Boolean AdjustPBs (menuSpec, pCD, newContext)
    MenuSpec    *menuSpec;	/* RETURNED */
    ClientData  *pCD;
    Context      newContext;

#else /* NO_PROTO */
static Boolean AdjustPBs (MenuSpec *menuSpec, ClientData  *pCD,
			  Context newContext)
#endif /* _NO_PROTO */
{
    MenuButton    *menuButton;
    MenuItem      *menuItem;
    int            msgc;
    unsigned int   n;
    long          *pMsg;
    Boolean        fSupported;
    Boolean        fChangeManaged = FALSE;

    /*
     *  Set PushButton sensitivity.
     *  Set f.send_msg button sensitivity according to context and client 
     *  message list.  Adjust other button sensitivities only for context.
     */

    /* check for bad input value - shouldn't happen. */
    if (menuSpec == NULL) return (FALSE);

    for (n = 0, menuButton = menuSpec->menuButtons;
         n < menuSpec->menuButtonCount;
	 n++, menuButton++)
    {
        menuItem = menuButton->menuItem;
        if (menuItem->wmFunction == F_Send_Msg)
	/* f.send_msg button:  set according to context and message. */
        {
            if ((newContext & menuItem->greyedContext) ||
		!(pCD && pCD->mwmMessagesCount && pCD->mwmMessages))
	    /* insensitive context or empty client message list */
	    {
	        XtSetSensitive (menuButton->buttonWidget, FALSE);
	    }
	    else
            /* 
             * Have a context sensitive f.send_msg item and a client with a 
	     * nonempty message list.  Set sensitive only if the message is 
	     * supported by this client.  Otherwise set insensitive.
             */
            {
                msgc = pCD->mwmMessagesCount;
		pMsg = pCD->mwmMessages;
	        fSupported = FALSE;
	        while (msgc--)
		/* scan nonempty message list */
	        {
	            if (*pMsg == (long) menuItem->wmFuncArgs)
	            /* found match */
	            {
		        fSupported = TRUE;
		        break;
	            }
	            pMsg++;  /* next message in list */
	        }
	        XtSetSensitive (menuButton->buttonWidget, fSupported);
            }
	}
	else
	/*
	 * Non f.send_msg button:
	 *  Adjust sensitivity according to context.
	 *  Manage/Unmanage according to clientFunction.
	 */
	{
	    if (menuSpec->currentContext & menuItem->greyedContext)
	    /* button is currently insensitive */
	    {
	        if (!(newContext & menuItem->greyedContext))
	        /* insensitive -> sensitive */
	        {
	            XtSetSensitive (menuButton->buttonWidget, TRUE);
	        }
	    }
	    else
	    /* button is currently sensitive */
	    {
	        if (newContext & menuItem->greyedContext)
	        /* sensitive -> insensitive */
	        {
	            XtSetSensitive (menuButton->buttonWidget, FALSE);
	        }
	    }

	    if ((menuItem->mgtMask) && pCD)
	    /* PushButton might not apply */
	    {
	        if (pCD->clientFunctions & menuItem->mgtMask)
	        /* function applies -- manage it */
	        {
	            if (!menuButton->managed)
	            /* unmanaged -> managed */
	            {
	                XtManageChild (menuButton->buttonWidget);
	                menuButton->managed = TRUE;
                        fChangeManaged = TRUE;
			if (n == menuSpec->menuButtonCount - 1)
			{
			    /* 
			     * last item, if it has a separator before
			     * it, manage the separator
			     */
			    
			    CheckTerminalSeparator(menuSpec, 
						   menuButton->buttonWidget, 
						   True);
			}
	            }
	        }
	        else
	        /* function does not apply -- unmanage it */
	        {
	            if (menuButton->managed)
	            /* managed -> unmanaged */
	            {
	                XtUnmanageChild (menuButton->buttonWidget);
	                menuButton->managed = FALSE;
                        fChangeManaged = TRUE;

			if (n == menuSpec->menuButtonCount - 1)
			{
			    /* 
			     * last item, if it has a separator before
			     * it, unmanage the separator
			     */
			    CheckTerminalSeparator(menuSpec, 
						   menuButton->buttonWidget, 
						   False);
			}

	            }
	        }
	    }
	    else if (!menuButton->managed)
	    /* unmanaged PushButton applies */
	    {
	        XtManageChild (menuButton->buttonWidget);
	        menuButton->managed = TRUE;
                fChangeManaged = TRUE;
	    }
	}
    }
#ifdef AUTOMATION
	SetMwmMenuInfo(pCD, menuSpec);
#endif

    return (fChangeManaged);

} /* END OF FUNCTION AdjustPBs */



/*************************************<->*************************************
 *
 *  static Boolean
 *  SavePBInfo (topMenuSpec, menuItem, itemW)
 *
 *
 *  Description:
 *  -----------
 *  Fills a MenuButton structure for a PushButton.
 *  If necessary, mallocs or reallocs the menuButtons array in the toplevel
 *  MenuSpec.
 *
 *
 *  Inputs:
 *  ------
 *  topMenuSpec = pointer to toplevel MenuSpec structure
 *  menuItem    = pointer to PushButton MenuItem structure
 *  itemW       = PushButton gadget
 *  topMenuSpec->menuButtons[]
 *  topMenuSpec->menuButtonSize
 *  topMenuSpec->menuButtonCount
 *
 * 
 *  Outputs:
 *  -------
 *  Return = FALSE iff insufficient memory for malloc or realloc
 *           or bad input value forces exit.
 *  topMenuSpec->menuButtons[]
 *  topMenuSpec->menuButtonSize
 *  topMenuSpec->menuButtonCount
 *
 *
 *  Comments:
 *  --------
 *  The initial managed status of PushButtons is TRUE.
 * 
 *************************************<->***********************************/
#ifdef _NO_PROTO
static Boolean SavePBInfo (topMenuSpec, menuItem, itemW)

    MenuSpec *topMenuSpec;	/* RETURNED */
    MenuItem *menuItem;
    Widget    itemW;
#else /* _NO_PROTO */
static Boolean SavePBInfo (MenuSpec *topMenuSpec, MenuItem *menuItem,
			     Widget itemW)
#endif /* _NO_PROTO */
{
    MenuButton *menuButton;


    /* check for bad input value - shouldn't happen. */
    if (topMenuSpec == NULL) return (FALSE);

    if (topMenuSpec->menuButtonSize == 0)
    /* need to create array */
    {
        topMenuSpec->menuButtonSize = MENU_BUTTON_INC;
        topMenuSpec->menuButtons =
	    (MenuButton *) XtMalloc (MENU_BUTTON_INC * sizeof(MenuButton));
    }
    else if (topMenuSpec->menuButtonCount == topMenuSpec->menuButtonSize)
    /* need larger array */
    {
        topMenuSpec->menuButtonSize += MENU_BUTTON_INC;
        topMenuSpec->menuButtons = (MenuButton *) 
	    XtRealloc ((char*)topMenuSpec->menuButtons,
		     topMenuSpec->menuButtonSize * sizeof(MenuButton));
    }

    if (topMenuSpec->menuButtons == NULL)
    /* insufficent memory */
    {
	topMenuSpec->menuButtonSize = 0;
	topMenuSpec->menuButtonCount = 0;
	return (FALSE);
    }

    menuButton = &(topMenuSpec->menuButtons[topMenuSpec->menuButtonCount]);
    topMenuSpec->menuButtonCount++;

    menuButton->menuItem = menuItem;
    menuButton->buttonWidget = itemW;
    menuButton->managed = TRUE;
    return (TRUE);

}



#ifdef DEC_MOTIF_EXTENSION
/*************************************<->*************************************
 *
 *  WmMenuResInit( pSD, menuName )
 *
 *  Description:
 *  -----------
 *  Initialize the resources for B&w and gray-scale systems for menus.
 *
 *  Inputs:
 *  ------
 *  pSD ---------- pointer to screen data
 *  menuName ----- the name of the menu specification to be used to create
 *                 the menu widget.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmMenuResInit( pSD, menuName )

WmScreenData *pSD;
String    menuName;
#else /* _NO_PROTO */
void WmMenuResInit( WmScreenData *pSD, String menuName )
#endif /* _NO_PROTO */

{
/* local variables */
char menu_name[ 128 ];
int index;

/********************************/

    /* Is this a one screen system or is a multiscreen system
       with all the same color ? */
    if ( !wmGD.multicolor )
        /* Yes, don't reload the menu resources; they have been merged. */
        return;
    /* If this is other than a color system, reload the menu resources. */
    if ( pSD->reset_resources )
      {
        for ( index = 0; index < k_mwm_menu_colors; index++ )
          {
            strcpy( menu_name, k_mwm_menu_prefix );
            strcat( menu_name, menuName );
            strcat( menu_name, k_mwm_dot_str );
            strcat( menu_name, mwm_menu_res.col_name[ index ] );
            if ( !mwm_res_get( pSD, pSD->user_database, menu_name,
                               k_mwm_res_pixel, &mwm_menu_res.col[ index ], 0 ))
              {                              
                mwm_menu_res.col_init[ index ] =
                    mwm_res_get( pSD, pSD->sys_database, menu_name,
                                 k_mwm_res_pixel, &mwm_menu_res.col[ index ], 0 );
              }
            else mwm_menu_res.col_init[ index ] = TRUE;
          }    
        for ( index = 0; index < k_mwm_menu_pixmaps; index++ )
          {
            strcpy( menu_name, k_mwm_menu_prefix );
            strcat( menu_name, menuName );
            strcat( menu_name, k_mwm_dot_str );
            strcat( menu_name, mwm_menu_res.pix_name[ index ] );
            if ( !mwm_res_get( pSD, pSD->user_database, menu_name,
                               k_mwm_res_string, (void *)mwm_menu_res.pix[ index ], 0 ))
              {                              
                mwm_menu_res.pix_init[ index ] =
                  mwm_res_get( pSD, pSD->sys_database, menu_name,
                           k_mwm_res_string, (void *)mwm_menu_res.pix[ index ], 0 );
              }
            else mwm_menu_res.pix_init[ index ] = TRUE;
          }    
      }

}

/*************************************<->*************************************
 *
 *  WmMenuResGet( pSD )
 *
 *  Description:
 *  -----------
 *  Get the resources for B&w and gray-scale systems for menus.
 *
 *  Inputs:
 *  ------
 *  pSD ---------- pointer to screen data
 *
 *  Input/Output:
 *  -------------
 *  args,
 *  number of arguments
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO                     
void WmMenuResSet( pSD, args, arg_num )

WmScreenData *pSD;
Arg args[20];
int *arg_num;
#else /* _NO_PROTO */
void WmMenuResSet( WmScreenData *pSD, Arg args[ 20 ], int *arg_num )
#endif /* _NO_PROTO */

{
/* local variables */
int index;

/********************************/

    /* Is this a one screen system or is a multiscreen system
       with all the same color ? */
    if ( !wmGD.multicolor )
        /* Yes, don't reload the menu resources; they have been merged. */
        return;
    /* If this is other than a color system, reload the menu resources. */
    if ( pSD->reset_resources )
      {
        /* Set the resources */
        for ( index = 0; index < k_mwm_menu_colors; index++ )
          {
            /* Was it initialized ? */
            if ( mwm_menu_res.col_init[ index ])
             {
                XtSetArg( args[ *arg_num ], mwm_menu_res.col_name[ index ],
                          (XtArgVal)mwm_menu_res.col[ index ]);
                *arg_num = *arg_num + 1;
             }
          }
        for ( index = 0; index < k_mwm_menu_pixmaps; index++ )
          {
            /* Was it initialized ? */
            if ( mwm_menu_res.pix_init[ index ])
              {
                /* It is unspecified ? */
                if ( !mwm_str_eq( mwm_menu_res.pix[ index ], "unspecified_pixmap" ))
                  {
                    /* No, get the pixmap. */
                    mwm_menu_res.pixmap[ index ] = 
                      XmGetPixmap( ScreenOfDisplay (DISPLAY, pSD->screen),
                                   mwm_menu_res.pix[ index ],
                                   mwm_menu_res.col[ k_mwm_menu_fore ],
                                   mwm_menu_res.col[ k_mwm_menu_back ] );
                    if ( mwm_menu_res.pixmap[ index ] == XmUNSPECIFIED_PIXMAP )
                        continue;
                   }
                else continue;
                XtSetArg( args[ *arg_num ], mwm_menu_res.pix_name[ index ],
                          (XtArgVal)mwm_menu_res.pixmap[ index ] );
                *arg_num = *arg_num + 1;
              }
          }    
      }

}

#endif /* DEC_MOTIF_EXTENSION */

/*************************************<->*************************************
 *
 *  CreateMenuWidget (pSD, menuName, parent, fTopLevelPane, topMenuSpec, 
 *                    moreMenuItems)
 *
 *
 *  Description:
 *  -----------
 *  Creates a MenuShell as a child of the specified parent widget, and a 
 *  PopupMenu or PulldownMenu as a child of the shell.  Fill the menu with
 *  the named menupane items.
 *
 *
 *  Inputs:
 *  ------
 *  pSD ---------- pointer to screen data
 *  menuName ----- the name of the menu specification to be used to create
 *                 the menu widget.
 *  parent -------- parent of popup shell
 *  fTopLevelPane - TRUE iff the menupane is a top level one
 *  topMenuSpec --- pointer to the top menu specification.
 *  moreMenuItems - pointer to additional menu items for custom menu.
 * 
 * 
 *  Outputs:
 *  -------
 *  Return = created PopupMenu or PulldownMenu widget, or NULL.
 *
 *
 *  Comments:
 *  --------
 *  We attach a popdowncallback to the menu to set wmGD.menuActive to NULL,
 *  allowing us to not dispatch key events separately from the toolkit 
 *  dispatcher.
 * 
 *************************************<->***********************************/

typedef struct _StrList
{
   XmString         string;
   struct _StrList *next;
} StrList;

#ifdef _NO_PROTO
Widget CreateMenuWidget (pSD, menuName, parent, fTopLevelPane,
			 topMenuSpec, moreMenuItems)
    WmScreenData *pSD;
    String    menuName;
    Widget    parent;
    Boolean   fTopLevelPane;
    MenuSpec *topMenuSpec;
    MenuItem *moreMenuItems;
#else /* _NO_PROTO */
Widget CreateMenuWidget (WmScreenData *pSD, String menuName, Widget parent,
			 Boolean fTopLevelPane, MenuSpec *topMenuSpec,
			 MenuItem *moreMenuItems)
#endif /* _NO_PROTO */
{
    int         i, n;
    Arg         sepArgs[1];
#ifdef DEC_MOTIF_EXTENSION
    Arg         args[20];
#else
    Arg         args[10];
#endif
    MenuSpec   *menuSpec;
    MenuItem   *menuItem;
    Widget      menuShellW;
    Widget      menuW;
    Widget      subMenuW;
    Widget      children[CHILDREN_CACHE];
    Pixmap      labelPixmap;
    KeySpec    *accelKeySpec;
    Dimension   menuHeight;
    Boolean     fUseTitleSep = FALSE;
    StrList    *stringsToFree = NULL, *sPtr;
    XmString    tmpstr;
#ifdef DEC_MOTIF_EXTENSION
    long bytes, status;
#endif

    /* check for bad input values. */
    if ((menuName == NULL) || (pSD == NULL))
    
    {
	return (NULL);
    }

    /*
     *  Find the menu pane specifications for menuName.
     *  The top-level menu specification is passed as an argument (it may
     *  be custom).  A submenu specification must be found and might not exist.
     *  Return NULL if a submenu specification is not found.
     */
    if (fTopLevelPane)
    {
        menuSpec = topMenuSpec;
    }
    else
    {
        menuSpec = pSD->menuSpecs;
        while (menuSpec)
        {
	    if ((menuSpec->name != NULL) && !strcmp (menuSpec->name, menuName))
	    {
	        break;  /* found menuName's specification */
	    }
	    menuSpec = menuSpec->nextMenuSpec;  /* keep looking */
        }
    }

    if (menuSpec == NULL)                                
    /* (submenu) specification not found */
    {
	MWarning("Menu specification %s not found\n", menuName);
	return (NULL);
    }

    /*
     * If menuSpec is marked, we have menu recursion => fail.
     *  Otherwise, mark it.
     */

    if (menuSpec->currentContext & CR_MENU_MARK)   /* marked? */
    /* menu recursion */
    {
	MWarning("Menu recursion detected for %s\n", menuName);
	return (NULL);
    }
#ifdef DEC_MOTIF_EXTENSION
    /* Check the menu resources for b&w and gray-scale */
    WmMenuResInit( pSD, menuName );
#endif /* DEC_MOTIF_EXTENSION */
    menuSpec->currentContext |= CR_MENU_MARK;   /* no, mark it */

    /*
     * Create a PopupShell widget.
     * If the parent of the specified parent ("grandparent") is a MenuShell 
     * widget, then use the grandparent as the parent of the PopupShell.
     * Otherwise, use the specified parent.
     */       
    i = 0;
    XtSetArg (args[i], XmNwidth, (XtArgVal) 5); i++;
    XtSetArg (args[i], XmNheight, (XtArgVal) 5); i++;
    XtSetArg (args[i], XmNallowShellResize, (XtArgVal) TRUE); i++;
    XtSetArg (args[i], XtNoverrideRedirect, (XtArgVal) TRUE); i++;
    XtSetArg (args[i], XtNdepth, 
		(XtArgVal) DefaultDepth(DISPLAY, pSD->screen)); i++;
    XtSetArg (args[i], XtNscreen, 
		(XtArgVal) ScreenOfDisplay(DISPLAY, pSD->screen)); i++;
                                                         
    if ((XtParent (parent) != NULL) && XmIsMenuShell (XtParent (parent)))
    {
	parent = XtParent (parent);                 
    }

    menuShellW = XtCreatePopupShell (SHELL_NAME, xmMenuShellWidgetClass,
                                     parent, (ArgList) args, i);

    /*
     * Create a RowColumn widget as a child of the shell for the menu pane.
     * If the menu pane is top-level, create a popup menu for it and attach 
     *   the unmap callback to it.
     * Otherwise, create a pulldown menu for it.
     */

    i = 0;
    XtSetArg (args[i], XmNborderWidth, (XtArgVal) 0); i++;
    XtSetArg (args[i], XmNwhichButton, (XtArgVal) SELECT_BUTTON); i++;
    XtSetArg (args[i], XmNadjustMargin, (XtArgVal) TRUE); i++;
                                
#ifdef DEC_MOTIF_EXTENSION
    /* Set the resources for menus for B&w and Gray-scale systems */
    WmMenuResSet( pSD, args, &i );
#endif /* DEC_MOTIF_EXTENSION */
    if (fTopLevelPane)
    {
        XtSetArg (args[i], XmNrowColumnType, (XtArgVal) XmMENU_POPUP); i++;
        XtSetArg (args[i], XmNpopupEnabled, (XtArgVal) TRUE); i++;
        menuW = XtCreateWidget (menuName, xmRowColumnWidgetClass, menuShellW,
				(ArgList) args, i);
	XtAddCallback (menuW, XmNunmapCallback, (XtCallbackProc)UnmapCallback, 
				(XtPointer) menuSpec);
    }
    else
    {
        XtSetArg (args[i], XmNrowColumnType, (XtArgVal) XmMENU_PULLDOWN); i++;
        menuW = XtCreateWidget (menuName, xmRowColumnWidgetClass, menuShellW,
				(ArgList) args, i);
    }

    /*
     * Create the specified menu entries as children of the menupane.
     * Menus may contain the following widgets:
     *
     *   Label
     *   Separator 
     *   CascadeButton
     *   PushButton
     *
     * Add separator gadgets around menu titles.
     */

    XtSetArg (sepArgs[0], XmNseparatorType, (XtArgVal) XmDOUBLE_LINE);

    n = 0;
    menuItem = menuSpec->menuItems;
    if ((menuItem == NULL) && (moreMenuItems != NULL))
    /* handle custom menu with empty standard specification */
    {
        menuSpec->menuItems = menuItem = moreMenuItems;
	moreMenuItems = NULL;
    }
    while (menuItem)
    {
        i = 0;

	if (menuItem->wmFunction == F_Separator)
   	/* 
	 * Add a Separator gadget for a menu separator.
	 * An immediately following title will not have a top separator.
	 */
	{
	    children[n] =
		XmCreateSeparatorGadget (menuW, SEPARATOR_NAME, 
				(ArgList)NULL, 0);

            fUseTitleSep = FALSE;
	} /* F_Separator */
                                           
	else
	/*
         * We will use one of:
	 *
         *   Label
         *   CascadeButton
         *   PushButton
	 */
        {
	    /*
	     * Construct the label
	     */                                     
            if ((menuItem->labelType == XmPIXMAP) &&
	         (labelPixmap =
		      MakeCachedLabelPixmap (pSD, menuW,
                                             menuItem->labelBitmapIndex)))
            {
                XtSetArg (args[i], XmNlabelType, (XtArgVal) XmPIXMAP); i++;
                XtSetArg (args[i], XmNlabelPixmap, (XtArgVal) labelPixmap); i++;
                XtSetArg (args[i], XmNlabelInsensitivePixmap,
			  (XtArgVal) labelPixmap); i++;
            }
            else
            {
                XtSetArg (args[i], XmNlabelType, (XtArgVal) XmSTRING); i++;
#ifdef DEC_MOTIF_EXTENSION
/* For internationalization */
                XtSetArg (args[i], XmNlabelString, (XtArgVal)
                  (tmpstr = 
                  DXmCvtFCtoCS( menuItem->label, &bytes, &status ))); i++;
#else
                XtSetArg (args[i], XmNlabelString, (XtArgVal)
			  (tmpstr = XmStringLtoRCreate(menuItem->label,
					    XmFONTLIST_DEFAULT_TAG))); i++;
#endif /* DEC_MOTIF_EXTENSION */
		sPtr = (StrList *) XtMalloc(sizeof(StrList));
		if (sPtr == NULL)
		  {
		     MWarning ("Insufficient memory for menu %s\n",
			       menuName);
		     return (NULL);
		  }
		else
		  {
		     sPtr->string  = tmpstr;
		     sPtr->next    = stringsToFree;
		     stringsToFree = sPtr;
		  }
		  i++;
            }
	    if (menuItem->wmFunction == F_Title)
	    /* 
	     * Add a centered Label gadget for a menu title.
	     * Include separators above and below the title.
	     * Don't include the top one if the title is the first pane item
	     *   or immediately follows a user-supplied separator.
	     */
	    {
                if (fUseTitleSep)
		{
                    children[n] =
			XmCreateSeparatorGadget (menuW, SEPARATOR_NAME,
					         sepArgs, 1); n++;
		}

                XtSetArg (args[i], XmNalignment, XmALIGNMENT_CENTER); i++; 
	        children[n] = XmCreateLabelGadget (menuW, TITLE_NAME,
					           (ArgList) args, i); n++;
                children[n] = XmCreateSeparatorGadget (menuW, SEPARATOR_NAME,
					  sepArgs, 1); 

                /*
		 * A following title will have both separators.
		 */

                fUseTitleSep = TRUE;
	    }

	    else
	    /*
             * We will use one of:
	     *
             *   CascadeButton
             *   PushButton
             *
	     * Both support mnemonics; only PushButtons support accelerators.
	     */
	    {
	        /*
		 * Align text on the left.
	         * Set any mnemonic text.
	         */
                XtSetArg (args[i], XmNalignment, XmALIGNMENT_BEGINNING); i++;

                if (menuItem->mnemonic)
                {
                    XtSetArg (args[i], XmNmnemonic, 
			       (XtArgVal) menuItem->mnemonic); i++;
                }

	        if (menuItem->wmFunction == F_Menu)
	        /* 
	         * Create a PopupShell and PulldownMenu for a submenu (the 
		 *   menushells are linked together).
	         * Create a CascadeButton Widget 
	         * The submenu widget is attached to the CascadeButton gadget
		 *   using the subMenuId resource.
	         * Make the CascadeButton insensitive if the submenu cannot be 
	         *   created.
	         */
	        {
		    subMenuW = CreateMenuWidget (pSD, 
						 menuItem->wmFuncArgs, menuW,
						 FALSE, topMenuSpec, 
						 (MenuItem *)NULL);
                    if (subMenuW)
		    /*
		     * Attach submenu to cascade button. 
		     */
		    {
                        XtSetArg (args[i], XmNsubMenuId, (XtArgVal) subMenuW);
			    i++;
	                children[n] = XmCreateCascadeButtonGadget (menuW,
					  CASCADE_BTN_NAME, (ArgList) args, i);
		    }
		    else
		    /*
		     * Unable to create submenupane: make the entry insensitive.
		     */
	            {
	                children[n] = XmCreateCascadeButtonGadget (menuW,
					  CASCADE_BTN_NAME, (ArgList) args, i);
	                XtSetSensitive (children[n], FALSE);
	            }

                    /*
		     * A following title will have both separators.
		     */                                         

                    fUseTitleSep = TRUE;
	        }

	        else 
	        /* 
	         * Create a PushButton gadget.
	         */
	        {
	            /*
	             * If an accelerator is specified, set acceleratorText,
		     * then create an accelerator KeySpec and insert it at the
		     * head of the toplevel MenuSpec's list.
	             */
                    if (menuItem->accelText)
                    {
#ifdef DEC_MOTIF_EXTENSION
/* For internationalization */
                        XtSetArg (args[i], XmNacceleratorText, (XtArgVal)
                           (tmpstr = DXmCvtFCtoCS( menuItem->accelText, &bytes, 
                                         &status ))); i++;
#else
		        XtSetArg (args[i], XmNacceleratorText, (XtArgVal)
				  (tmpstr = XmStringLtoRCreate(menuItem->accelText,
						  	       XmFONTLIST_DEFAULT_TAG))); i++;
#endif /* DEC_MOTIF_EXTENSION */
		        sPtr = (StrList *) XtMalloc(sizeof(StrList));
			if (sPtr == NULL)
			  {
			     MWarning ("Insufficient memory for menu %s\n",
				       menuName);
			     return (NULL);
			  }
			else
			  {
			     sPtr->string = tmpstr;
			     sPtr->next   = stringsToFree;
			     stringsToFree = sPtr;
			  }
                        if ((accelKeySpec = (KeySpec *)
                                 XtMalloc (sizeof (KeySpec ))) == NULL)
	                /* Handle insufficent memory */
                        {
                            MWarning ("Insufficient memory for menu %s\n",
				      menuName);
                            menuSpec->currentContext &= ~CR_MENU_MARK;
	                    return (NULL);
                        }

		        accelKeySpec->state = menuItem->accelState;
		        accelKeySpec->keycode = menuItem->accelKeyCode;
		        accelKeySpec->context = topMenuSpec->accelContext;
		        accelKeySpec->subContext = 0;
		        accelKeySpec->wmFunction = menuItem->wmFunction;
		        accelKeySpec->wmFuncArgs = menuItem->wmFuncArgs;
		        accelKeySpec->nextKeySpec = topMenuSpec->accelKeySpecs;
                        topMenuSpec->accelKeySpecs = accelKeySpec;
                    }

	            children[n] = XmCreatePushButtonGadget (menuW, 
				      PUSH_BTN_NAME, (ArgList) args, i);

	            /* 
		     * Set sensitivity.  Initially we only consider the context
		     * of the top level menupane.
		     */

	            if (menuItem->greyedContext & topMenuSpec->currentContext)
	            /* insensitive button in this context*/
	            {
	                XtSetSensitive (children[n], FALSE);
	            }
	            else
	            /* sensitive button in this context*/
	            {
	                XtSetSensitive (children[n], TRUE);
	            }
#ifdef DEC_MOTIF_EXTENSION
                    /* Is this customize colors on a multicolor system
                       that is not the same as the main monitor ? */
          	    if (( menuItem->wmFunction == F_DEC_Customize) &&
                          wmGD.multicolor &&
                          ( ! menuItem->greyedContext ) &&
                          ( pSD->monitor != wmGD.main_monitor ))
                      /* Is this a customize color dialog box ? */
                      {
                        if ( mwm_str_eq( menuItem->wmFuncArgs,
                                         k_mwm_cust_border_col_arg ) ||
                             mwm_str_eq( menuItem->wmFuncArgs,
                                         k_mwm_cust_icon_col_arg ) ||
                             mwm_str_eq( menuItem->wmFuncArgs,
                                         k_mwm_cust_matte_arg ))
                          /* Yes, disable customize colors */
                          {
                            menuItem->greyedContext = F_CONTEXT_ALL;
	                    XtSetSensitive (children[n], FALSE);
                          }
                       }
#endif /* DEC_MOTIF_EXTENSION */

		    /*
		     * If necessary, fill a menuButtons element for this 
		     * PushButton.  Malloc or Realloc the array if necessary.
		     */
                    if ((menuItem->greyedContext) || (menuItem->mgtMask))
                    {
			if (!SavePBInfo (topMenuSpec, menuItem, children[n]))
			{
                            MWarning("Insufficient memory for menu %s\n",
				       menuName);
                            menuSpec->currentContext &= ~CR_MENU_MARK;
	                    return (NULL);
                        }
		    }

	            /*
	             * Set up the function callback.
		     * A following title will have both separators.
	             */

	            XtAddCallback (children[n], XmNactivateCallback,
			    (XtCallbackProc)ActivateCallback, 
				   (XtPointer) menuItem);

                    fUseTitleSep = TRUE;
	        }
	    }
	}

	/*
	 * Next menu item:  handle custom items and full children[]. 
	 */
	n++;
	menuItem = menuItem->nextMenuItem;
        if ((menuItem == NULL) && (moreMenuItems != NULL))
        {
            menuSpec->menuItems = menuItem = moreMenuItems;
	    moreMenuItems = NULL;
        }
	if (n >= CHILDREN_CACHE - 2)  /* leave room for title separators */
	{
            XtManageChildren (children, n);
	    n = 0;
        }
    }

    if (n > 0)
    {
        XtManageChildren (children, n);
    }

    /*
     * Get the initial height of the top level menu pane shell.
     * The actual height will change according to clientFunctions.
     */
    if (fTopLevelPane)
    {
	i = 0;
	XtSetArg (args[i], XtNheight, &menuHeight);  i++;
	XtGetValues (menuW, (ArgList)args, i);
	topMenuSpec->height = (unsigned int) menuHeight;
    }


    /* free the string that may have been created earlier. */
    for (sPtr = stringsToFree; sPtr != NULL; )
      {
	 stringsToFree = stringsToFree->next;
	 XtFree((char *)sPtr->string);
	 sPtr = stringsToFree;
      }


    /* Unmark the menu specification and return. */
    menuSpec->currentContext &= ~CR_MENU_MARK;
    return (menuW);

} /* END OF FUNCTION CreateMenuWidget */



/*************************************<->*************************************
 *
 *  PostMenu (menuSpec, pCD, x, y, button, newContext, flags, passedInEvent)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to post a menu at a particular location.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =      menu specification
 *  pCD =           client data
 *  x,y =           position to post the menu if (flags & POST_AT_XY) set
 *  button =        button number posting the menu or NoButton (WmGlobal.h) if
 *                  posted by a key
 *  newContext =    context that the menu is to be posted under.
 *  flags =         POST_AT_XY bit set iff x,y are valid, else compute from pCD
 *                  POST_TRAVERSAL_ON bit set if set traversal on
 * 
 *  Outputs:
 *  -------
 *  menuSpec =        menu specification with modifications
 *  wmGD.menuClient = pCD
 *  wmGD.menuActive = menuSpec
 *
 *
 *  Comments:
 *  --------
 *  Accepts x,y only if POST_AT_XY flag bit set.  Otherwise, computes from pCD.
 *  Adjusts PushButton sensitivity according to context and function type.
 *  Manages/Unmanages PushButtons according to clientFunction resource.
 *  Sets traversal on if button==NoButton or POST_TRAVERSAL_ON flag bit set.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void PostMenu (menuSpec, pCD, x, y, button, newContext, flags, passedInEvent)

    MenuSpec    *menuSpec;
    ClientData  *pCD;
    int          x, y;
    unsigned int button;
    Context      newContext;
    long         flags;
    XEvent       *passedInEvent;
    
#else /* _NO_PROTO */
void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, unsigned int button, Context newContext, long flags, XEvent *passedInEvent)
#endif /* _NO_PROTO */
{
    int              i;
    Arg              args[3];
    unsigned int     whichButton;
    Dimension        menuHeight;
    XButtonPressedEvent event;
    Window           saveWindow;
    int              saveType;
    
    if ((menuSpec == NULL) || (menuSpec->menuWidget == NULL))
    {
	return;
    }


    /* 
     * Don't post a menu from an icon in the iconbox if the
     * icon is not visible
     */
    if((newContext == F_SUBCONTEXT_IB_WICON ||
       newContext == F_SUBCONTEXT_IB_IICON) &&
       (!(IconVisible(pCD))))
    {
	return;
    }

    /*
     * Set grabContext to be used in GrabWin when no event is passed
     * to GrabWin. 
     */

    wmGD.grabContext = newContext;

    /*
     *  Adjust PushButton sensitivity and manage/unmanage status.
     *  If the manage status of the system menu has changed, 
     *  then get the height of the top level menu pane shell and
     *  cache it in its MenuSpec.
     */
    

    if (AdjustPBs (menuSpec, pCD, newContext))
    {
        i = 0;
        XtSetArg (args[i], XtNheight, &menuHeight);  i++;
        XtGetValues (menuSpec->menuWidget, (ArgList)args, i);
        menuSpec->height = (unsigned int) menuHeight;
    }
    menuSpec->currentContext = newContext;

    /*
     *  Adjust the whichButton resource if necessary.
     *  Use SELECT_BUTTON for NoButton.
     */

    whichButton = (button == NoButton) ? SELECT_BUTTON : button;
    if (whichButton != menuSpec->whichButton)
    {
        i = 0;
        XtSetArg (args[i], XmNwhichButton, (XtArgVal) whichButton); i++;
        XtSetValues (menuSpec->menuWidget, args, i);
        menuSpec->whichButton = whichButton;
    }

    /*
     *  Determine the position of the popup menu.
     *  Compute position if necessary (system menu).
     */

    if (!(flags & POST_AT_XY))
    /* compute the position */
    {
	GetSystemMenuPosition (pCD, &x, &y, menuSpec->height, newContext);
    }

    event.x_root = x;
    event.y_root = y;
    XmMenuPosition (menuSpec->menuWidget, &event);

    wmGD.menuClient = pCD;
    wmGD.menuActive = menuSpec;   /* set to NULL within UnmapCallback() */

    /* 
     * Post the menu by managing its top-level RowColumn.
     *
     * First dispatch the event to set the time stamp in the toolkit
     */

    if(passedInEvent)
    {
	saveWindow = passedInEvent->xany.window;
	passedInEvent->xany.window = 0;
        saveType = passedInEvent->type;
        passedInEvent->type = 0;
	
	XtDispatchEvent(passedInEvent);
	passedInEvent->xany.window = saveWindow;
	passedInEvent->type = saveType;
    }
    
#ifndef ALTERNATE_POSTMENU

    XtManageChild (menuSpec->menuWidget);

#else
    if (flags & POST_STICKY)
    {
	_XmSetPopupMenuClick(menuSpec->menuWidget, True);
    }
    else
    {
	_XmSetPopupMenuClick(menuSpec->menuWidget, False);
    }

    /* 
     * Post the menu by calling the convenience routine that verifies
     * the button event, updates the Xt timestamp, and finally manages
     * the pane.
     */

    _XmPostPopupMenu( menuSpec->menuWidget, passedInEvent);
#endif

#ifdef AUTOMATION
	SetMwmMenuWindow(pCD, menuSpec->menuWidget);
#endif

    /*
     *  set the traversal state.
     */

    if ((button == NoButton) || (flags & POST_TRAVERSAL_ON))
    /* turn traversal on */
    {
	TraversalOn (menuSpec);
    }
    else
    /* turn traversal off */
    {
	TraversalOff (menuSpec);
    }

} /* END OF FUNCTION PostMenu */



/*************************************<->*************************************
 *
 *  UnpostMenu (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function is used to unpost a menu.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec =      menu specification
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  wmGD.menuActive and wmGD.menuUnpostKey are set to NULL within 
 *  UnmapCallback.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void UnpostMenu (menuSpec)

    MenuSpec    *menuSpec;
#else /* _NO_PROTO */
void UnpostMenu (MenuSpec *menuSpec)
#endif /* _NO_PROTO */
{
    if (menuSpec && (menuSpec->menuWidget))
    /* 
     * Unpost the menu by unmanaging its top-level RowColumn.
     */
    {
        XtUnmanageChild (menuSpec->menuWidget);
#ifndef OLD_COLORMAP
        ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    }

} /* END OF FUNCTION UnpostMenu */



/*************************************<->*************************************
 *
 *  ActivateCallback (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  This function is called whenever a menu item is selected.
 *
 *
 *  Inputs:
 *  ------
 *  w =               menubuttonWidget
 *  client_data =     pointer to menu button's MenuItem structure
 *  call_data =       not used
 *  wmGD.menuClient = pointer to client's ClientData structure
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ActivateCallback (w, client_data, call_data)

    Widget  w;
    XtPointer client_data;
    XtPointer call_data;
#else /* _NO_PROTO */
void ActivateCallback (Widget w, XtPointer client_data, XtPointer call_data)
#endif /* _NO_PROTO */
{
#ifdef DEC_MOTIF_BUG_FIX
    /* Reset the global active screen variable.*/
    if ( wmGD.numScreens > 1 )
        SetActiveScreen( &wmGD.Screens[ XScreenNumberOfScreen( XtScreen( w )) ] );
#endif /* DEC_MOTIF_BUG_FIX */
#ifdef DEC_MOTIF_EXTENSION
    /* pass widget for customize and help */
    if (( ((MenuItem *)client_data)->wmFunction == F_DEC_Customize) ||
        ( ((MenuItem *)client_data)->wmFunction == F_DEC_Help ) ||
        ( ((MenuItem *)client_data)->wmFunction == F_Restart )) 
       ((MenuItem *)client_data)->wmFunction (
       		    ((MenuItem *)client_data)->wmFuncArgs, wmGD.menuClient, 
                     (caddr_t)w);
    else ((MenuItem *)client_data)->wmFunction (
     	       	    ((MenuItem *)client_data)->wmFuncArgs, wmGD.menuClient, NULL);
#else
    ((MenuItem *)client_data)->wmFunction (
		((MenuItem *)client_data)->wmFuncArgs, wmGD.menuClient, NULL);
#endif /* DEC_MOTIF_EXTENSION */

} /* END OF FUNCTION ActivateCallback */



/*************************************<->*************************************
 *
 *  UnmapCallback (w, client_data, call_data)
 *
 *
 *  Description:
 *  -----------
 *  This function is called whenever a toplevel RowColumn is unmapped.
 *
 *
 *  Inputs:
 *  ------
 *  w =
 *  client_data =       not used
 *  call_data =         not used
 *  wmGD.gadgetClient = last client with depressed client
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD.menuActive = NULL
 *  wmGD.menuUnpostKeySpec = NULL
 *  wmGD.checkHotspot = FALSE
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void UnmapCallback (w, client_data, call_data)

    Widget  w;
    XtPointer client_data;
    XtPointer call_data;
#else /* _NO_PROTO */
void UnmapCallback (Widget w, XtPointer client_data, XtPointer call_data)
#endif /* _NO_PROTO */
{
    wmGD.menuActive = NULL;
    wmGD.menuUnpostKeySpec = NULL;
    wmGD.checkHotspot = FALSE;

    if (wmGD.gadgetClient) 
    {
	PopGadgetOut(wmGD.gadgetClient, FRAME_SYSTEM);
    }

#ifndef OLD_COLORMAP
    ForceColormapFocus (ACTIVE_PSD, ACTIVE_PSD->colormapFocus);
#endif
    PullExposureEvents();

} /* END OF FUNCTION UnmapCallback */


/*************************************<->*************************************
 *
 *  MWarning (message)
 *
 *
 *  Description:
 *  -----------
 *  This function lists a message to stderr.
 *
 *
 *  Inputs:
 *  ------
 *  format  = pointer to a format string
 *  message = pointer to a message string
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MWarning (format, message)

    char *format;
    char *message;
#else /* _NO_PROTO */
void MWarning (char *format, char *message)
#endif /* _NO_PROTO */
{
#define MAXPATH 1023

    if (strlen(format) + strlen(message)  <  MAXPATH)
      {
	 char pch[MAXPATH+1];

	 sprintf (pch, format, message);
	 Warning (pch);
      }

} /* END OF FUNCTION MWarning */



/*************************************<->*************************************
 *
 *  TraversalOff (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function turns menu traversal off.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = menu specification
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void TraversalOff (menuSpec)

    MenuSpec  *menuSpec;
#else /* _NO_PROTO */
void TraversalOff (MenuSpec *menuSpec)
#endif /* _NO_PROTO */
{
    if (menuSpec && (menuSpec->menuWidget))
    {
	/* function pointer */
	(*((XmRowColumnWidgetClass) XtClass (menuSpec->menuWidget))
				       ->row_column_class.menuProcedures) 
		/* argument list */
	       (XmMENU_TRAVERSAL, menuSpec->menuWidget, False, NULL, NULL);
    }

} /* END OF FUNCTION TraversalOff */



/*************************************<->*************************************
 *
 *  TraversalOn (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This function turns menu traversal on.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = menu specification
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  None.
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void TraversalOn (menuSpec)

    MenuSpec  *menuSpec;
#else /* _NO_PROTO */
void TraversalOn (MenuSpec *menuSpec)
#endif /* _NO_PROTO */
{

    if (menuSpec && (menuSpec->menuWidget))
    {
	/* function pointer */
	(*((XmRowColumnWidgetClass) XtClass (menuSpec->menuWidget))
				   ->row_column_class.menuProcedures) 
		/*argument list */
	       (XmMENU_TRAVERSAL, menuSpec->menuWidget, True, NULL, NULL);
    }

} /* END OF FUNCTION TraversalOn */



/*************************************<->*************************************
 *
 *  FreeCustomMenuSpec (menuSpec)
 *
 *
 *  Description:
 *  -----------
 *  This procedure destroys a custom MenuSpec structure and its associated 
 *  menu widget, menuItems list, menuButtons array, and menu accelerator list.
 *
 *
 *  Inputs:
 *  ------
 *  menuSpec = MenuSpec structure
 *
 * 
 *  Outputs:
 *  -------
 *  None.
 *
 *
 *  Comments:
 *  --------
 *  Assumes that a MenuSpec is custom iff its name is NULL.
 *
 *  Assumes that ParseWmFuncStr() has parsed a menu item's function
 *  argument only for F_Exec and F_Menu.  If it is used for other functions,
 *  be sure to include them here!
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void FreeCustomMenuSpec (menuSpec)

    MenuSpec  *menuSpec;
#else /* _NO_PROTO */
void FreeCustomMenuSpec (MenuSpec *menuSpec)
#endif /* _NO_PROTO */
{
    MenuItem    *menuItem;
    MenuItem    *nextMenuItem;
    KeySpec     *accelKeySpec;
    KeySpec     *nextAccelKeySpec;

    if ((menuSpec == NULL) || (menuSpec->name != NULL))
    /* we only destroy custom menus! */
    {
	return;
    }

    menuItem = menuSpec->menuItems;
    while (menuItem)
    {
	nextMenuItem = menuItem->nextMenuItem;
        FreeMenuItem (menuItem);
	menuItem = nextMenuItem;
    }

    if (menuSpec->menuButtons)
    {
        XtFree ((char *)menuSpec->menuButtons);
    }

    accelKeySpec = menuSpec->accelKeySpecs;
    while (accelKeySpec)
    {
	nextAccelKeySpec = accelKeySpec->nextKeySpec;
	XtFree ((char *)accelKeySpec);
	accelKeySpec = nextAccelKeySpec;
    }

    if (menuSpec->menuWidget)
    /* destroy all children of the menu's MenuShell parent */
    {
        XtDestroyWidget (XtParent(menuSpec->menuWidget));
    }

    XtFree ((char *)menuSpec);

} /* END OF FUNCTION FreeCustomMenuSpec */


#ifdef AUTOMATION
#ifdef _NO_PROTO
static void SetMwmMenuInfo (pCD, menuSpec)
ClientData   *pCD;
MenuSpec     *menuSpec;

#else
static void SetMwmMenuInfo (ClientData *pCD, MenuSpec *menuSpec)
#endif /* _NO_PROTO */
{

    PropMwmFrameIconInfo    FrameIconProp;
    char                    *RetData;
    Atom                    FrameIconAtom, NewType;
    int                     NewFormat;
    unsigned long           NewNitems, NewBytesAfter;
    int                     SensitiveCount;
    MenuInfo                *NewMenuInfo;
    MenuButton              *NewMenuButton;
    MenuItem                *NewMenuItem;
    unsigned int            n;
    int                     status;


	RetData = NULL;

    if (pCD == NULL || menuSpec == NULL)
        return;

    FrameIconAtom = XmInternAtom(DISPLAY, _XA_MOTIF_WM_FRAME_ICON_INFO, False);
    status = XGetWindowProperty(DISPLAY, pCD->client, FrameIconAtom, 0L, 
                                PROP_MWM_FRAME_ICON_INFO_ELEMENTS, False, 
                                AnyPropertyType, &NewType, &NewFormat, 
                                &NewNitems, &NewBytesAfter, 
                                (unsigned char **)&RetData);
	if (status != Success || RetData == NULL)
	{
		/* Need a warning here; waiting for general warning handling code */
		return;
	}

    bcopy((char *)RetData, (char *)&FrameIconProp, 
          PROP_MWM_FRAME_ICON_INFO_ELEMENTS);
    /* Swap bytes if necessary */
    AutoSwapBytes(&FrameIconProp);

    FrameIconProp.menuItemCount = 0;
    FrameIconProp.sensitiveItemCount = 0;

    FrameIconProp.menuItemCount = menuSpec->menuButtonCount;
    NewMenuInfo = &FrameIconProp.windowMenu[0];
    for (n = 0; n < FrameIconProp.menuItemCount; n++)
	{

        NewMenuInfo->item_name[0] = '\0';
        NewMenuInfo->sensitive = FALSE;
        NewMenuInfo++;

    }

    NewMenuInfo = &FrameIconProp.windowMenu[0];
    SensitiveCount = 0;

    for (n = 0, NewMenuButton = menuSpec->menuButtons;
         n < menuSpec->menuButtonCount;
         n++, NewMenuButton++)
    {

        if (NewMenuButton->managed == FALSE)
            continue;

        NewMenuItem = NewMenuButton->menuItem;
        strcpy(NewMenuInfo->item_name, NewMenuItem->label);
        NewMenuInfo->sensitive = NewMenuButton->buttonWidget->core.sensitive;
        NewMenuInfo->item_y = NewMenuButton->buttonWidget->core.y +
                            (NewMenuButton->buttonWidget->core.height / 2);
        if (NewMenuInfo->sensitive == TRUE)
            SensitiveCount++;
        NewMenuInfo++;

    }

    FrameIconProp.byte_order = AutoByteOrderChar;
    FrameIconProp.sensitiveItemCount = SensitiveCount;
    XChangeProperty(DISPLAY, pCD->client, wmGD.xa_MWM_FRAME_ICON_INFO, 
                    wmGD.xa_MWM_FRAME_ICON_INFO, 8, PropModeReplace,
                    (unsigned char *)&FrameIconProp, 
                    PROP_MWM_FRAME_ICON_INFO_ELEMENTS);
    XSync(DISPLAY, False);

    if (RetData != NULL)
        XFree((char *)RetData);

} /* SetMwmMenuInfo */


#ifdef _NO_PROTO
static void SetMwmMenuWindow (pCD, menuWidget)
ClientData *pCD;
Widget     menuWidget;

#else
static void SetMwmMenuWindow (ClientData *pCD, Widget menuWidget)
#endif /* _NO_PROTO */
{

    PropMwmFrameIconInfo    FrameIconProp;
    char                    *RetData;
    Atom                    FrameIconAtom, NewType;
    int                     NewFormat;
    unsigned long           NewNitems, NewBytesAfter;
    Window                  MenuWin;
    int                     status;

    if (pCD == NULL)
        return;

    MenuWin = XtWindow(menuWidget);

    FrameIconAtom = XmInternAtom(DISPLAY, _XA_MOTIF_WM_FRAME_ICON_INFO, False);
    status = XGetWindowProperty(DISPLAY, pCD->client, FrameIconAtom, 0L, 
                                PROP_MWM_FRAME_ICON_INFO_ELEMENTS, False, 
                                AnyPropertyType, &NewType, &NewFormat, 
                                &NewNitems, &NewBytesAfter, 
                                (unsigned char **)&RetData);
	if (status != Success || RetData == NULL)
	{
		/* Need a warning here; waiting for general warning handling code */
		return;
	}

    bcopy((char *)RetData, (char *)&FrameIconProp, 
          PROP_MWM_FRAME_ICON_INFO_ELEMENTS);
    /* Swap bytes if necessary */
    AutoSwapBytes(&FrameIconProp);

    FrameIconProp.byte_order = AutoByteOrderChar;
    FrameIconProp.menuWin = MenuWin;
    XChangeProperty(DISPLAY, pCD->client, wmGD.xa_MWM_FRAME_ICON_INFO, 
                    wmGD.xa_MWM_FRAME_ICON_INFO, 8, PropModeReplace,
                    (unsigned char *)&FrameIconProp, 
                    PROP_MWM_FRAME_ICON_INFO_ELEMENTS);
    XSync(DISPLAY, False);

    if (RetData != NULL)
        XFree((char *)RetData);

    return;

} /* SetMwmMenuWindow */

#endif /* AUTOMATION */
