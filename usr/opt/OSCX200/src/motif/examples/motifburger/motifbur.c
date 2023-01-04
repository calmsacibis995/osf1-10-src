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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: motifbur.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 17:40:33 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: motifbur.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 17:40:33 $"
#endif
#endif
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
 * A sample program which uses UIL and MRM to create the interface.
 */

#include <stdio.h>                              /* For printf and so on. */

#include <Xm/Xm.h>                /* Motif Toolkit and MRM */
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Mrm/MrmPublic.h>                /* Motif Toolkit and MRM */
/*
 * These numbers are matched with corresponding numbers in the DECburger
 * UIL module.
 */

#define k_create_order           1
#define k_order_pdme             2
#define k_file_pdme              3
#define k_edit_pdme              4
#define k_nyi                    5
#define k_apply                  6
#define k_dismiss                7
#define k_noapply                8
#define k_cancel_order           9
#define k_submit_order           10
#define k_order_box              11
#define k_burger_min           12
#define k_burger_rare            12
#define k_burger_medium          13
#define k_burger_well            14
#define k_burger_ketchup         15
#define k_burger_mustard         16
#define k_burger_onion           17
#define k_burger_mayo            18
#define k_burger_pickle          19
#define k_burger_max           19
#define k_burger_quantity        20
#define k_fries_tiny             21
#define k_fries_small            22
#define k_fries_medium           23
#define k_fries_large            24
#define k_fries_huge             25
#define k_fries_quantity         26
#define k_drink_list             27
#define k_drink_add              28
#define k_drink_sub              29
#define k_drink_quantity         30
#define k_total_order            31
#define k_burger_label           32
#define k_fries_label            33
#define k_drink_label            34
#define k_menu_bar               35
#define k_file_menu              36
#define k_edit_menu              37
#define k_order_menu             38

#define k_max_widget             38

#define MAX_WIDGETS (k_max_widget + 1)

#define NUM_BOOLEAN (k_burger_max - k_burger_min + 1)

#define k_burger_index   0
#define k_fries_index    1
#define k_drinks_index   2
#define k_index_count    3

/*
 * Global data
 */

Display         *display;		/* Display variable */
XtAppContext    app_context;		/* application context */

static Widget toplevel_widget,          /* Root widget ID of our */
                                        /* application. */
  main_window_widget,                   /* Root widget ID of main */
                                        /* MRM fetch */
  widget_array[MAX_WIDGETS];            /* Place to keep all other */
                                        /* widget IDs */

static char toggle_array[NUM_BOOLEAN];  /* Our TRUTH about the state */
                                        /* of user interface toggles. */

static XmString current_drink,     /* Last selected drink name. */
  current_fries,                        /* Last selected fries size. */
  name_vector[k_index_count];           /* Miscellaneous names gotten from */
                                        /* various widgets. */

static int quantity_vector[k_index_count];      /* Current quantities of */
                                                /* burger, fries, drinks. */

static XmString latin_create;      /* Variables for */
static XmString latin_dismiss;     /* compound strings. */
static XmString latin_space;
static XmString latin_zero;
static MrmHierarchy s_MrmHierarchy;     /* MRM database hierarchy ID */
static MrmType dummy_class;            /* and class variable. */
static char *db_filename_vec[] =        /* Mrm.heirachy file list. */
  {"motifbur.uid"                    /* There is only one UID file for */
  };                                    /* this application. */                                               
static int db_filename_num =
                (sizeof db_filename_vec / sizeof db_filename_vec [0]);

/*
 * Forward declarations
 */

static void s_error();
static void get_something();
static void set_something();

static void activate_proc();
static void create_proc();
static void list_proc();
static void quit_proc();
static void pull_proc();
static void scale_proc();
static void show_hide_proc();
static void show_label_proc();
static void toggle_proc();

static init_application();

/* The names and addresses of things that Mrm.has to bind.  The names do
 * not have to be in alphabetical order.  */

static MrmRegisterArg reglist[] = {
#ifdef DEC_MOTIF_BUG_FIX
    {"activate_proc", (XtPointer) activate_proc}, 
    {"create_proc", (XtPointer) create_proc}, 
    {"list_proc", (XtPointer) list_proc}, 
    {"pull_proc", (XtPointer) pull_proc}, 
    {"quit_proc", (XtPointer) quit_proc}, 
    {"scale_proc", (XtPointer) scale_proc}, 
    {"show_hide_proc", (XtPointer) show_hide_proc}, 
    {"show_label_proc", (XtPointer) show_label_proc}, 
    {"toggle_proc", (XtPointer) toggle_proc}
#else
    {"activate_proc", (caddr_t) activate_proc}, 
    {"create_proc", (caddr_t) create_proc}, 
    {"list_proc", (caddr_t) list_proc}, 
    {"pull_proc", (caddr_t) pull_proc}, 
    {"quit_proc", (caddr_t) quit_proc}, 
    {"scale_proc", (caddr_t) scale_proc}, 
    {"show_hide_proc", (caddr_t) show_hide_proc}, 
    {"show_label_proc", (caddr_t) show_label_proc}, 
    {"toggle_proc", (caddr_t) toggle_proc}
#endif
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);
static font_unit = 400;

/*
 * OS transfer point.  The main routine does all the one-time setup and
 * then calls XtAppMainLoop.
 */

unsigned int main(argc, argv)
    int    argc;                  /* Command line argument count. */
    String argv[];                /* Pointers to command line args. */
{
    Arg arglist[2];
    int n;


    MrmInitialize();                 /* Initialize MRM before initializing */
                                        /* the X Toolkit. */

    /* If we had user-defined widgets, we would register them with Mrm.here. */

    /* Initialize the X Toolkit. We get back a top level shell widget. */

    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();
    display = XtOpenDisplay(app_context, NULL, argv[0], "example",
                            NULL, 0, &argc, argv);
    if (display == NULL) {
        fprintf(stderr, "%s:  Can't open display\n", argv[0]);
        exit(1);
    }

    n = 0;
    XtSetArg(arglist[n], XmNallowShellResize, True);  n++;
    toplevel_widget = XtAppCreateShell(argv[0], NULL, applicationShellWidgetClass,
                              display, arglist, n);

    /* Open the UID files (the output of the UIL compiler) in the hierarchy*/

    if (MrmOpenHierarchy(db_filename_num, /* Number of files. */
      db_filename_vec,                    /* Array of file names.  */
      NULL,                               /* Default OS extenstion. */
      &s_MrmHierarchy)                    /* Pointer to returned MRM ID */
      !=MrmSUCCESS)
        s_error("can't open hierarchy");

    init_application();

    /* Register the items MRM needs to bind for us. */

    MrmRegisterNames(reglist, reglist_num);


/*    XmSetFontUnit(XtDisplay(toplevel_widget), font_unit);
*/
    /* Go get the main part of the application. */
    if (MrmFetchWidget(s_MrmHierarchy, "S_MAIN_WINDOW", toplevel_widget,
      &main_window_widget, &dummy_class) != MrmSUCCESS)
        s_error("can't fetch main window");

    /* Manage the main part and realize everything.  The interface comes up
     * on the display now. */

    XtManageChild(main_window_widget);
    XtRealizeWidget(toplevel_widget);

    /* Sit around forever waiting to process X-events.  We never leave
     * XtAppMainLoop. From here on, we only execute our callback routines. */
    XtAppMainLoop(app_context);
}




/*
 * One-time initialization of application data structures.
 */

static int init_application()
{
    int k;

    /* Initialize the application data structures. */
    for (k = 0; k < MAX_WIDGETS; k++)
        widget_array[k] = NULL;
    for (k = 0; k < NUM_BOOLEAN; k++)
        toggle_array[k] = FALSE;

    /*  Set the medium 'hamburger doneness' toggle button so that the 
     *  radio box has one toggle button ON at startup. */

    toggle_array[k_burger_medium - k_burger_min] = TRUE;

    /* Initialize current values of various items to match their initial values
     * in the DECburger UIL module. */
    current_drink = XmStringLtoRCreate("Apple Juice","");
    current_fries = XmStringLtoRCreate("Medium","");

    /* Set up the compound strings that we need. */
    latin_create = XmStringLtoRCreate("Create order box...","");
    latin_dismiss = XmStringLtoRCreate("Dismiss order box...","");
    latin_space = XmStringLtoRCreate(" ","");
    latin_zero = XmStringLtoRCreate(" 0 ","");
}

/***************************************************************************
 *
 * These are some little utilities used by the callback routines.
 */


/*
 * All errors are fatal.
 */

static void s_error(problem_string)
    char *problem_string;
{
    printf("%s\n", problem_string);
    exit(0);
}

/*
 * Simplified SET VALUE routine to use only when changing a single attribute.
 * If we need to change more than one, all new values should be put 
 * into one arglist and we should make one XtSetValues call (which is MUCH 
 * more efficient).
 */

static void set_something(w, resource, value)
    Widget w;
    char *resource, *value;
{
    Arg al[1];

    XtSetArg(al[0], resource, value);
    XtSetValues(w, al, 1);
}


/*
 * Simplified GET VALUE routine to use only when retrieving a single attribute.
 * If we need to retrieve more than one, all values should be put 
 * into one arglist and we should make one XtGetValues call (which is MUCH 
 * more efficient).
 */

static void get_something(w, resource, value)
    Widget w;
    char *resource, *value;
{
    Arg al[1];

    XtSetArg(al[0], resource, value);
    XtGetValues(w, al, 1);
}


/*
 * Keep our boolean array current with the user interface toggle buttons.
 */

static void set_boolean(i, state)
    int i;                              /* Widget ID index. */
    int state;
{
    toggle_array[i - k_burger_min] = state;

    XmToggleButtonSetState(widget_array[i], 
                                        /* Which widget */
      state,                            /* state it should have. */
      FALSE);                           /* Do not call me back now. */
}



/*
 * Format and update the drink quantity widget.
 */

static void update_drink_display()
{
    char drink_txt[50];

    sprintf(drink_txt, " %d ", quantity_vector[k_drinks_index]);
    set_something(widget_array[k_drink_quantity], XmNlabelString,
      XmStringLtoRCreate(drink_txt,""));

}








/***************************************************************************
 *
 * This section contains callback routines.
 */



/*
 * Reset the user interface and the application to a known state.
 */

static void reset_values()
{
    int i;

    /*  Reset the toggle buttons and our boolean array. */
    for (i = k_burger_min; i <= k_burger_max; i++) {

        /* The radio box requires that one button be set; we choose medium. */

        set_boolean(i, (i == k_burger_medium));
    }

    /* Reset the burger quantity scale widget and global value. */
    set_something(widget_array[k_burger_quantity], XmNvalue, 0);
    quantity_vector[k_burger_index] = 0;

    /* Reset the fries quantity text widget.  We do not have a global for this.
     * We read the widget whenever we need to know the quantity. */
    XmTextSetString(widget_array[k_fries_quantity], " 0 ");

    /* Reset the drinks quantity text widget and global value. */
    set_something(widget_array[k_drink_quantity], XmNlabelString, latin_zero);
    quantity_vector[k_drinks_index] = 0;
}




/*
 * Clear the order display area in the main window.
 */

static void clear_order()
{
    Arg arglist[5];
    int ac = 0;

    XtSetArg(arglist[ac], XmNitemCount, 0);
    ac++;
/*
!    XtSetArg(arglist[ac], XmNitems, NULL);
!    ac++;
*/
    XtSetArg(arglist[ac], XmNselectedItemCount, 0);
    ac++;
    XtSetValues(widget_array[k_total_order], arglist, ac);
}





/*
 * All push buttons in this application call back to this routine.  We
 * use the tag to tell us what widget it is, then react accordingly.
 */

static void activate_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    int widget_num = *tag;              /* Convert tag to widget number. */
    int i, value, fries_num;
    XmString txt, list_txt;
    char *fries_text, list_buffer[20];

    switch (widget_num) {
        case k_nyi: 

            /* The user activated a 'not yet implemented' push button.  Send
             * the user a message. */
            if (widget_array[k_nyi] == NULL)
                                        /* The first time, fetch from */
              {                         /* the data base. */
                if (MrmFetchWidget(s_MrmHierarchy, "nyi", toplevel_widget,
                  &widget_array[k_nyi], &dummy_class) != MrmSUCCESS) {
                    s_error("can't fetch nyi widget");
                }
            }

            /*  Put up the message box saying 'not yet implemented'. */
            XtManageChild(widget_array[k_nyi]);
            break;

        case k_submit_order: 

            /*  This would send the order off to the kitchen. In this case,
             *  we just pretend the order was submitted. */
            clear_order();
            break;

        case k_cancel_order: 

            /*  Clear out the order display. */
            clear_order();
            break;

        case k_dismiss: 

            /*  Bring down the control box and reset the values to the default.
             */
            XtUnmanageChild(widget_array[k_order_box]);

        /* no break */

        case k_noapply: 
            reset_values();
            break;

       case 333:
            printf ("callback 333");
            break;
 
       case 334:
            printf ("callback 334");
            break;

       case 335:
            printf ("callback 335");
            break;

       case 336:
            printf ("callback 336");
            break;

        case k_apply: 

            /*  Take the current settings and write them into the list box.  */
            if (quantity_vector[k_burger_index] > 0) {

                /* Put burger quantity in the display string. */
                sprintf(list_buffer, "%d ", quantity_vector[k_burger_index]);
                list_txt = XmStringLtoRCreate(list_buffer,"");

                /* Collect hambuger attributes that are ON. */
                for (i = k_burger_min; i <= k_burger_max; i++)
                    if (toggle_array[i - k_burger_min]) {

                        /*  Get the name of the qualifier from the widget and
                         *  add to the display string. */
                        get_something(widget_array[i], XmNlabelString, &txt);
                        list_txt = XmStringConcat(list_txt, txt);
                        list_txt = XmStringConcat(list_txt, latin_space);                    
                    }

                /* Add hamburger name and add to the display string. */
                list_txt = XmStringConcat(list_txt, name_vector[k_burger_index]);

                XmListAddItem(widget_array[k_total_order], list_txt, 0);
            }

            /*  Text widget does not have a callback.  So we query the widget
             *  now to determine what its value is.  Convert to an integer. */
	    fries_num = 0;
            fries_text = XmTextGetString(widget_array[k_fries_quantity]);
            sscanf(fries_text, "%d", &fries_num);

            if (fries_num != 0) {

                /* Put the fries quantity in the display string. */
                sprintf(list_buffer, "%d ", fries_num);
                list_txt = XmStringLtoRCreate(list_buffer,"");

                /*  Get all the qualifiers for the fries. */
                list_txt = XmStringConcat(list_txt, current_fries);
                list_txt = XmStringConcat(list_txt, latin_space);

                /* Add fries name and display. */
                list_txt = XmStringConcat(list_txt, name_vector[k_fries_index]);

                XmListAddItem(widget_array[k_total_order], list_txt, 0);
            }

            if (quantity_vector[k_drinks_index] > 0) {

                /* Put drinks quantity into the display string. */
                sprintf(list_buffer, "%d ", quantity_vector[k_drinks_index]);
                list_txt = XmStringLtoRCreate(list_buffer,"");                                                                   

                /*  Now get the qualifiers for the drinks. */
                list_txt = XmStringConcat(list_txt, current_drink);
                list_txt = XmStringConcat(list_txt, latin_space);

                /* Add the drink name to the display string. */

                list_txt = XmStringConcat(list_txt, name_vector[k_drinks_index]);

                XmListAddItem(widget_array[k_total_order], list_txt, 0);
            }
            break;

        case k_fries_tiny:                                                                                        
        case k_fries_small: 
        case k_fries_medium: 
        case k_fries_large: 
        case k_fries_huge: 

            /*  Some fries size push button was activated, so get the string 
             *  from the interface.  Helps with internationalization. */
            get_something(w, XmNlabelString, &current_fries);
            break;

        case k_drink_add: 

            /*  Increment the drink quantity and update the display. */
            quantity_vector[k_drinks_index]++;
            update_drink_display();
            break;

        case k_drink_sub: 

            /*  Decrement drink quantity, but do not let it go below zero. */
            if (quantity_vector[k_drinks_index] > 0)
                quantity_vector[k_drinks_index]--;
            update_drink_display();
            break;

        default: 
            break;
    }
}




/*
 * The toggle buttons which control the 'hamburger doneness' and toppings
 * call back to this procedure when they change state.  Use the
 * tag to index into the boolean array.  Just keep the booleans current
 * with the user interface.
 */

static void toggle_proc(w, tag, toggle)
    Widget w;
    int *tag;
    XmToggleButtonCallbackStruct *toggle;
{
    toggle_array[*tag - k_burger_min] = toggle->set;
}




/*
 * The drink selection list box calls back to this procedure whenever the
 * user selects a drink.  Just keep the global current drink up to date.
 */

static void list_proc(w, tag, list)
    Widget w;
    int *tag;
    XmListCallbackStruct *list;
{
    XtFree((char *)current_drink);
    current_drink = XmStringCopy(list->item);
}



/*
 * The hamburger quantity scale widget will call back to this procedure whenever
 * the user changes it.  Just keep the global hamburger quantity up to date.
 */

static void scale_proc(w, tag, scale)
    Widget w;
    int *tag;
    XmScaleCallbackStruct *scale;
{
    quantity_vector[k_burger_index] = scale->value;
}





/*
 * The next two procedures put up and take down the order box and change
 * the label in the pulldown menu from Create to Dismiss.
 */


/*
 * The user selected the Order push button in the control pulldown menu.
 * We just change the state of the order box.  If the order box is 
 * currently displayed (managed), then remove (unmanage) it.  Otherwise,
 * we manage the order box.
 */

static void show_hide_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    if (XtIsManaged(widget_array[k_order_box]))

        XtUnmanageChild(widget_array[k_order_box]);
    else
        XtManageChild(widget_array[k_order_box]);
}


/*
 * This callback runs as the control pulldown menu is about to be pulled down.
 * We use this opportunity to fetch the order box (if not done already)
 * and to make sure the push button displays the correct label.
 */

static void show_label_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    if (widget_array[k_order_box] == NULL)
                                        /* The first time, fetch order box. */
      {
        if (MrmFetchWidget(s_MrmHierarchy, "control_box", toplevel_widget,
          &widget_array[k_order_box], &dummy_class) != MrmSUCCESS) {
            s_error("can't fetch order box widget");
        }
    }

    /* Now figure out what the label on the push button in the pulldown menu 
     * should be. */

    if (XtIsManaged(widget_array[k_order_box]))
        set_something(widget_array[k_create_order], XmNlabelString, latin_dismiss);
    else
        set_something(widget_array[k_create_order], XmNlabelString, latin_create);
}







/*
 * All widgets that are created call back to this procedure. We just log the 
 * ID in the global array.
 */

static void create_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    int widget_num = *tag;

    widget_array[widget_num] = w;

    /*  For internationalization ease, we capture a few strings from the
     *  widgets themselves.  We could go out and fetch them as needed but
     *  since we use these all the time, this method if more efficient.
     */
    switch (widget_num) {
        case k_burger_label: 
            get_something(w, XmNlabelString, &name_vector[k_burger_index]);
            break;

        case k_fries_label: 
            get_something(w, XmNlabelString, &name_vector[k_fries_index]);
            break;

        case k_drink_label: 
            get_something(w, XmNlabelString, &name_vector[k_drinks_index]);
            break;

        default: 
            break;
    }
}




/*
 * The user pushed the quit button, so the application exits.
 */
static void quit_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    if (tag != NULL)
        printf("Quitting - %s\n", tag);
    exit(1);
}


/*
 * This callback runs just as a pulldown menu is about to be pulled down.
 * It fetches the menu if it is currently empty, and does other
 * special processing as required.
 * We use this opportunity to fetch the order box (if not done already)
 * and to make sure the push button displays the correct label.
 */

static void pull_proc(w, tag, reason)
    Widget w;
    int *tag;
    unsigned long *reason;
{
    int widget_num = *tag;

    switch (widget_num) {
        case k_file_pdme: 
            if (widget_array[k_file_menu] == NULL) {
                if (MrmFetchWidget(s_MrmHierarchy, "file_menu", widget_array[
                  k_menu_bar], &widget_array[k_file_menu], &dummy_class) !=
                  MrmSUCCESS)
                    s_error("can't fetch file pulldown menu widget");
                set_something(widget_array[k_file_pdme], XmNsubMenuId,
                  widget_array[k_file_menu]);
            }
            break;

        case k_edit_pdme: 
            if (widget_array[k_edit_menu] == NULL) {
                if (MrmFetchWidget(s_MrmHierarchy, "edit_menu", widget_array[
                  k_menu_bar], &widget_array[k_edit_menu], &dummy_class) !=
                  MrmSUCCESS)
                    s_error("can't fetch edit pulldown menu widget");
                set_something(widget_array[k_edit_pdme], XmNsubMenuId,
                  widget_array[k_edit_menu]);
            }
            break;

        case k_order_pdme: 
            if (widget_array[k_order_menu] == NULL) {
                if (MrmFetchWidget(s_MrmHierarchy, "order_menu", widget_array[
                  k_menu_bar], &widget_array[k_order_menu], &dummy_class) !=
                  MrmSUCCESS)
                    s_error("can't fetch order pulldown menu widget");
                set_something(widget_array[k_order_pdme], XmNsubMenuId,
                  widget_array[k_order_menu]);
                if (MrmFetchWidget(s_MrmHierarchy, "control_box",
                  toplevel_widget, &widget_array[k_order_box], &dummy_class) !=
                  MrmSUCCESS)
                    s_error("can't fetch order box widget");
            }

            /* Figure out what the label of the push button in the pulldown
             * menu should be. */

            if ( widget_array[k_order_box] == NULL )
                if (MrmFetchWidget (
                     s_MrmHierarchy,
                     "control_box",
                     toplevel_widget,
                     &widget_array [k_order_box],
                     &dummy_class) != MrmSUCCESS) 
                       s_error ("can't fetch order box widget");
            if (XtIsManaged(widget_array[k_order_box]))
                set_something(widget_array[k_create_order], XmNlabelString,
                  latin_dismiss);
            else
                set_something(widget_array[k_create_order], XmNlabelString,
                  latin_create);
            break;

    }

}
