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
static char *rcsid = "@(#)$RCSfile: slabel.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:08 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	slabel.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:52  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:47  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:45  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:12  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*  Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	@(#)slabel.c	1.5 16:31:33 5/17/91 SecureWare
 */

/* #ident "@(#)slabel.c	1.1 11:18:56 11/8/91 SecureWare" */

#include <sys/secdefines.h>
#if SEC_MAC /*{*/

/*
 * Sensitivity label "widget"
 * input is a character string title and a buffer for a sensitivity label.
 * output is a filled buffer and return value specifying whether screen
 * successfully executed the action or not (R_EXECUTE, R_ABORTED, or R_QUIT).
 *
 * The screen is modeled after the sensitivity/information label widget
 * on the X screen.  The screen displays a scrolling region for typing
 * an explicit sensitivity label and two (three) columns of scrolled
 * toggle regions for classifications and compartments (and markings).
 * If the user starts typing in the typing region, the display at the
 * bottom of the screen changes from "Enter=Execute" to "Enter=Check".
 * When the user attempts to leave the field or presses Enter after
 * typing, the validation routine tries to parse the senstivity label
 * and displays the expanded parsed label in the field and updates the
 * classification and category scrolling region to reflect those
 * associated with the changed typed label.  The bottom of the screen
 * is changed back from "Enter=Check" to "Enter=Execute" after a successful
 * label parse.  Otherwise, the bell rings and the cursor is put back
 * to the beginning of the typed label field.
 *
 * When the user selects fields in the classification and compartment
 * scrolling toggle regions, the validation routine is called and the
 * typed region is updated to reflect the selection.  The classification
 * field needs special treatment; when a new one is selected, the previously
 * selected one is de-selected.  These are done with the validation
 * fields in the scrn_desc entry for the fields.
 *
 * NOTE that the current version of this screen assumes the
 * classes/categories files for sensitivity label definitions.
 * The Encodings version of this routine is to be implemented later.
 */

#include "If.h"
#include "AIf.h"

/*
 * Static routine declarations
 */

static int setup_structures();
static int reset_structures();
static int solicit_label();
static int build_tables();
static int typed_label_validate();
static void set_label();
static int initialize_fillin();
static void initialize_this_future();
static void initialize_default_sl();
static void initialize_just_sl();
static int use_default_sl();
static void setup_label_table();
static int aif_setup();

/*
 * Static global variables
 */

static int IsTyping = 0;
static int Checked = 0;		/* Checked this time through the execution */
static int ErrorLabel = 0;
static Scrn_struct *SL_struct;

#define PARMTEMPLATE	SL_scrn
#define STRUCTTEMPLATE	SL_struct
#define DESCTEMPLATE	SL_desc
#define FILLINSTRUCT	label_fillin
#define FILLIN		sl_fill

/* boolean flag telling how aif_label was originally called */

static enum {ThisFuture, UseDefault, SLonly} HowCalled;

/* offsets for scrn_struct */

#define LABEL_TEXT	0
#define CLASSIFICATIONS	1
#define COMPARTMENTS	2

/* if called for this/future */
#define THIS_SESSION	3
#define FUTURE_SESSIONS 4

/* if called for use default */
#define USE_DEFAULT_TOG	3
#define DEFAULT_TEXT	4

/* if called for neither */
#define NEITHER_1	3
#define NEITHER_2	4

#define NSCRNSTRUCT	5

/* Screen layout parameters */

#define OUTSIDE_MARGIN		6
#define SL_TEXT_LINES		2
#define DEF_SL_TEXT_LINES	2	/* if default SL requested */
#define TOG_LINES		5
#define SL_TEXT_WIDTH		70

#define SL_TITLE_DESC		0
#define SL_TEXT_DESC		1
#define CLASS_TITLE_DESC	2
#define COMPS_TITLE_DESC	3
#define CLASS_UNDER_DESC	4
#define COMPS_UNDER_DESC	5
#define CLASS_SCRTOG_DESC	6
#define COMPS_SCRTOG_DESC	7

/* if this/future requested */
#define SL_THIS_DESC		8
#define SL_FUTURE_DESC		9

/* if default requested */
#define SL_DEF_SL_TOGGLE_DESC	8
#define SL_DEF_SL_DESC		9

/* if neither requested */
#define SL_NEITHER_FIRST_DESC	8

#define FIRSTDESC	SL_TEXT_DESC

static Scrn_desc SL_desc[] = {
/* row, col, type, len, inout, required, prompt, help,
 * lines, spaces, itemsperline, valid_on_leave
 */
{ 0, 6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, NULL },  /* prompt is dynamic */
{ 2, OUTSIDE_MARGIN, FLD_TEXT,  SL_TEXT_WIDTH, FLD_BOTH,    NO, NULL, NULL,
	SL_TEXT_LINES, 0, 1, 1 },
{ 4, OUTSIDE_MARGIN, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Classifications:" },
{ 4,39, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Compartments:" },
{ 5, OUTSIDE_MARGIN, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------" },
{ 5,39, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "-------------" },
{ 6, OUTSIDE_MARGIN, FLD_SCRTOG, 30, FLD_BOTH,    NO, NULL, NULL,
	TOG_LINES, 0, 1, 1 },
{ 6,39, FLD_SCRTOG, 30, FLD_BOTH,    NO, NULL, NULL,
	TOG_LINES, 0, 1, 1 },
/*
 * The remainder of the descs will be filled depending whether the
 * calling routine specifies this/future, default, or neither
 */

{0},
{0},
};

/* define the number of optional descs at the end of the structure */

#define NUM_OPTIONAL_DESCS 2

/*
 * The this/future option specifies two toggle buttons
 */

static Scrn_desc SL_this_future_desc[] = {
{12,OUTSIDE_MARGIN, FLD_TOGGLE, 17, FLD_BOTH, NO, "Current Session" },
{12,39,             FLD_TOGGLE, 17, FLD_BOTH, NO, "Future Sessions" },
};

/*
 * The default SL option specifies a toggle button and a sensitivity label
 */

Scrn_desc SL_default_desc[] = {
{11, OUTSIDE_MARGIN, FLD_TOGGLE, 11, FLD_BOTH, NO, "Use Default" },
{12, OUTSIDE_MARGIN, FLD_TEXT, SL_TEXT_WIDTH, FLD_OUTPUT, NO, NULL, NULL,
	DEF_SL_TEXT_LINES, 0, 1, 0 },
};

static Scrn_desc SL_neither_desc[] = {
{13, 0, FLD_PROMPT, 0, FLD_OUTPUT, NO, " "},
{13, 2, FLD_PROMPT, 0, FLD_OUTPUT, NO, " "},
};

int SL_setup(), SL_init(), SL_free(), SL_exit();

static Menu_scrn SL_menu[] = {
	ROUT_DESC(1, SL_exit, 0, NULL),
	ROUT_DESC(2, SL_exit, 0, NULL),
	ROUT_DESC(3, SL_exit, 0, NULL),
	ROUT_DESC(4, SL_exit, 0, NULL),
	ROUT_DESC(5, SL_exit, 0, NULL),
};

Scrn_hdrs SL_hdrs = {
	NULL,				/* title */
	cur_date, runner, cur_time,	/* lh, ch, rh */
	NULL, NULL, NULL,		/* lf, cf, rf */
	NULL, "SPACE=Select",
	"RET=Execute  ESC=Leave menu  ^Y=Item help  ^B=Quit Program",
	NULL				/* help (filled in dynamically) */
};

static
SKIP_PARMF(SL_scrn, SCR_FILLIN, SL_desc, SL_menu, NULL, &SL_hdrs,
	SL_setup, SL_init, SL_free);
	
/*
 * Error message declarations
 */

static char
	**msg_error_cant_setup_labels,
	*msg_error_cant_setup_labels_text;

/*
 * Fillin structure for labels.
 */

static struct label_fillin {
	char this;		/* change this session */
	char future;		/* change future session */
	mand_ir_t *old_label;	/* label before latest selection */
	char *text_label;	/* textual representation */
	int ntext;		/* size of allocated string */
	char **label_tab;	/* table of character strings for multi text */
	int nlabel_tab;		/* number of entries in label_tab */
	char **class_table;	/* words in the classification list */
	char *class_select;	/* which words selected */
	int nclasses;		/* number in the list */
	char **comp_table;	/* words in the compartment list */
	char *comp_select;	/* which words selected */
	int ncomps;		/* number in the list */
	int nstructs;		/* number of scrn_structs */
	char use_default;	/* use default sl */
	mand_ir_t *def_sl_ir;	/* default sl IR */
	char **def_label_tab;	/* default sl label text table */
} SL_save_fill, SL_fill, *sl_fill = &SL_fill;

/*
 * Main routine that sets up data structures and calls screen solicitation
 * routine to retrieve label.
 */

int
aif_label(title, subtitle,
	  ir_buffer, this_ptr, future_ptr,
	  use_default, default_sl, dflt_sl_string,
	  help)
	char *title;		/* screen title */
	char *subtitle;		/* title of SL prompt */
	mand_ir_t *ir_buffer;	/* internal representation */
	char *this_ptr;		/* pointer if "Current Session" toggle */
	char *future_ptr;	/* pointer if "Future Session" toggle */
	char *use_default;	/* pointer if default SL specified */
	mand_ir_t *default_sl;	/* pointer to IR if default SL */
	char *dflt_sl_string;	/* string to appear as "Use default" toggle */
	char *help;		/* help screen identifier */
{
	int ret;

	ENTERFUNC("aif_label");

	/* call the setup routine to initialize the screen data structures */

	if (aif_setup(title, subtitle, ir_buffer, this_ptr, future_ptr,
			use_default, default_sl, dflt_sl_string, help))
		return 1;

	/*
	 * Call the routine that displays the screen and solicits input
	 */

	ret = traverse(&SL_scrn, SL_TEXT_DESC);
	
	switch (ret) {
	case INTERRUPT:		/* successful execution */

		mand_copy_ir(sl_fill->old_label, ir_buffer);
		if (this_ptr)
			*this_ptr = sl_fill->this;
		if (future_ptr)
			*future_ptr = sl_fill->future;

		if (use_default)
			*use_default = sl_fill->use_default;

		ret = 0;

		break;

	case CONTINUE:		/* user typed escape */
	case QUIT:		/* user gave up */
		ret = 1;
		break;
	}

	EXITFUNC("aif_label");
	return ret;
}

/*
 * setup routine that is called to initialize the data structures for the
 * screen.  This routine is called by aif_label() and aif_print().
 */

static int
aif_setup(title, subtitle,
	  ir_buffer, this_ptr, future_ptr,
	  use_default, default_sl, dflt_sl_string, help)
	char *title;		/* screen title */
	char *subtitle;		/* title of prompt field */
	mand_ir_t *ir_buffer;	/* internal representation */
	char *this_ptr;		/* pointer if "Current Session" toggle */
	char *future_ptr;	/* pointer if "Future Session" toggle */
	char *use_default;	/* pointer if default SL specified */
	mand_ir_t *default_sl;	/* pointer to IR if default SL */
	char *dflt_sl_string;	/* string to appear as "Use default" toggle */
	char *help;		/* help screen identifier */
{
	int i;
	int ret;
	int widest_class = 0;
	int widest_comp = 0;
	int middle_spaces;
	int right_column;

	/*
	 * set up the data structures for the screen.
	 */

	ret = initialize_fillin(ir_buffer, &SL_save_fill);
	if (ret) {
		ERRFUNC("aif_label", "label setup");
		return 1;
	}

	/* temporarily, if use_default set and default_sl not, SCR_NOCHANGE */

	if (use_default && default_sl == (mand_ir_t *) 0) {
		SL_scrn.scrntype = SCR_NOCHANGE;
		use_default = NULL;
	} else
		SL_scrn.scrntype = SCR_FILLIN;

	/*
	 * Set up the screen structures to accommodate the sizes of
	 * the various fields.
	 *
	 * Place the screen title and SL/Clearance title in the right place
	 */

	SL_hdrs.title = title;
	SL_hdrs.help = help;
	SL_desc[SL_TITLE_DESC].prompt = subtitle;
	if (use_default != NULL)
		if (dflt_sl_string != NULL)
			SL_desc[SL_DEF_SL_TOGGLE_DESC].prompt = dflt_sl_string;
		else
			SL_desc[SL_DEF_SL_TOGGLE_DESC].prompt =
				MSGSTR(USE_DEF, "Use Default");

	/*
	 * see if the screen is wide enough to accommodate the widest
	 * classification and the widest compartment.
	 */

	for (i = 0; i < sl_fill->nclasses; i++) {
		int class_len = strlen(sl_fill->class_table[i]);

		if (class_len > widest_class)
			widest_class = class_len;
	}

	for (i = 0; i < sl_fill->ncomps; i++) {
		int comp_len = strlen(sl_fill->comp_table[i]);

		if (comp_len > widest_comp)
			widest_comp = comp_len;
	}

	/*
	 * There is a 6 space margin on both sides, and two spaces in
	 * between the columns.  If they are too wide, make the columns
	 * equal width.
	 */

	if (COLS - widest_comp - widest_class < (OUTSIDE_MARGIN * 2) + 2)
		widest_comp =
		  widest_class = (COLS - ((OUTSIDE_MARGIN * 2) + 2)) / 2;

	middle_spaces = COLS - widest_comp - widest_class -
					(OUTSIDE_MARGIN * 2); 

	right_column = OUTSIDE_MARGIN + widest_class + middle_spaces / 2;

	SL_desc[COMPS_TITLE_DESC].col =
	SL_desc[COMPS_UNDER_DESC].col =
	SL_desc[COMPS_SCRTOG_DESC].col =
		right_column;
	
	/*
	 * do specific initialization for this/future or default SL
	 */

	if (this_ptr)
		initialize_this_future(this_ptr, future_ptr);
	else if (use_default)
		initialize_default_sl(use_default, default_sl, &SL_save_fill);
	else
		initialize_just_sl();

	return 0;
}

/*
 * Routine called when the user types a character.  If not in typing
 * mode, set typing mode.  If the first time user is caught typing,
 * note that the user is now typing and change the screen label
 * to Enter=Check.
 */

static int
typed_label_validate()
{
	register struct label_fillin *lf = sl_fill;

	ENTERFUNC("typed_label_validate");
	if (!IsTyping) {

		/* turn off "Use Default" if currently using default SL */

		if (HowCalled == UseDefault && sl_fill->use_default) {
			sl_fill->use_default = 0;
			PARMTEMPLATE.ss[USE_DEFAULT_TOG].changed = 1;
		}

		IsTyping = 1;
		SL_hdrs.c1 = MSGSTR(AIF_335,
		"RET=Check  ESC=Leave menu  ^Y=Item help  ^B=Quit Program");
		headers(&SL_scrn);
		PARMTEMPLATE.ss[LABEL_TEXT].changed = 1;
		return 1;
	}
	EXITFUNC("typed_label_validate");
	return 0;
}

/*
 * Routine called when the user presses Enter after typing in the label
 * field.  If the user was typing,
 * parse the typed label and update the classification and compartment
 * scrolling regions to match it.
 * If not, the user typed execute in this field and the screen action
 * is done.
 * Returns 0 on success and 1 on failure
 */

static int
type_check_valid(lf)
	struct label_fillin *lf;
{
	mand_ir_t *ir;
	int ret;
	static char **msg_error, *msg_error_text;

	ENTERFUNC("type_check_valid");
	if (IsTyping) {
		int i;

		lf->text_label[0] = '\0';
		for (i = 0; i < lf->nlabel_tab; i++)
			strcat(lf->text_label, lf->label_tab[i]);
		ir = mand_er_to_ir(lf->text_label);
		if (ir == NULL) {
			ErrorLabel = 1;
			if (!msg_error)
				LoadMessage(MSGSTR(SLABEL_1,
				"msg_sensitivity_label_parse_error"),
				&msg_error, &msg_error_text);
			ErrorMessageOpen(-1, msg_error, 0, NULL);
		} else {
			ErrorLabel = 0;

			set_label(ir, lf);
			mand_free_ir(ir);
			IsTyping = 0;
			SL_hdrs.c1 = MSGSTR(AIF_329,
                 "RET=Execute  ESC=Leave menu  ^Y=Item help  ^B=Quit Program");
			headers(&SL_scrn);
			Checked = 1;
		}
		ret = 1;
	} else
		ret = 0;

	EXITFUNC("type_check_valid");

	return ret;
}

/*
 * Action routine for the screen.
 * Returns QUIT on success, INTERRUPT on failure
 */

SL_action(lf)
	struct label_fillin *lf;
{
	int ret;

	ENTERFUNC("SL_action");
	ret = INTERRUPT;
	EXITFUNC("SL_action");
	return ret;
}

/*
 * Routine called when the user toggles a classification.
 * Turn off any classifications that were already selected and turn on
 * the current one.  Update the typed section that has the text
 * representation of the label.
 */

static int
class_toggle_validate()
{
	register struct label_fillin *fill = sl_fill;
	int i;
	char *er;

	ENTERFUNC("class_toggle_validate");

	if (IsTyping) {

		beep();

		/* reset back to the original classification */

		for (i = 0; i < fill->nclasses; i++) {
			int class_number;

			class_number = mand_nametocl(fill->class_table[i]);
			fill->class_select[i] =
			  (class_number == fill->old_label->class);
		}
		PARMTEMPLATE.ss[CLASSIFICATIONS].changed = 1;

		EXITFUNC("class_toggle_validate");

		return 1; /* Refresh screen */
	}

	/* turn off "Use Default" if currently using default SL */

	if (HowCalled == UseDefault && sl_fill->use_default) {
		sl_fill->use_default = 0;
		PARMTEMPLATE.ss[USE_DEFAULT_TOG].changed = 1;
	}

	/*
	 * If the user is turning off a classification and there are
	 * none selected, do not allow the user to do it.  Only allow
	 * if the user is turning on another toggle.
	 */

	for (i = 0; i < fill->nclasses; i++) {
		int class_number;

		class_number = mand_nametocl(fill->class_table[i]);
		if (fill->class_select[i] &&
		    class_number != fill->old_label->class) {

			/*
			 * A new classification was turned on.
			 */

			char *cl_name = mand_cltoname(fill->old_label->class);
			int j;

			/*
			 * find the one that was previously on, and turn it off
			 */

			for (j = 0; j < fill->nclasses; j++)
				if (!strcmp(cl_name, fill->class_table[j])) {
					fill->class_select[j] = 0;
					break;
				}
			fill->old_label->class = class_number;
			break;
		}

		if (!fill->class_select[i] &&
		    class_number == fill->old_label->class) {

			/*
			 * A classification was turned off
			 * Just turn it back on.
			 */

			fill->class_select[i] = 1;
			break;
		}
	}

	/*
	 * Update the text string with the new classification
	 */

	set_label(fill->old_label, fill);
	PARMTEMPLATE.ss[LABEL_TEXT].changed = 1;
	PARMTEMPLATE.ss[CLASSIFICATIONS].changed = 1;
	EXITFUNC("class_toggle_validate");
	return 1;
}

/*
 * Routine called when the user toggles a compartment.
 * Re-display the text field in the scrolling region and update the
 * internal copy of the sensitivity label.
 */

static int
comp_toggle_validate()
{
	register struct label_fillin *fill = sl_fill;
	int i;
	char *er;

	ENTERFUNC("comp_toggle_validate");
	if (IsTyping) {
		beep();
		/*
		 * reset back to the original set of compartments */
		for (i = 0; i < fill->ncomps; i++) {
			int comp_number;

			comp_number = mand_nametocat(fill->comp_table[i]);
			fill->comp_select[i] =
			  ISBITSET(fill->old_label->cat, comp_number);
		}
		PARMTEMPLATE.ss[COMPARTMENTS].changed = 1;
		EXITFUNC("comp_toggle_validate");

		return 1; /* refresh screen */
	}

	/* turn off "Use Default" if currently using default SL */

	if (HowCalled == UseDefault && sl_fill->use_default) {
		sl_fill->use_default = 0;
		PARMTEMPLATE.ss[USE_DEFAULT_TOG].changed = 1;
	}

	for (i = 0; i < fill->ncomps; i++) {
		int comp_number;

		comp_number = mand_nametocat(fill->comp_table[i]);
		if (fill->comp_select[i] &&
		    !ISBITSET(fill->old_label->cat, comp_number)) {
			ADDBIT(fill->old_label->cat, comp_number);
		} else if (!fill->comp_select[i] &&
			   ISBITSET(fill->old_label->cat, comp_number)) {
			RMBIT(fill->old_label->cat, comp_number);
		}
	}

	/*
	 * Update the new text string with the new compartment
	 */

	set_label(fill->old_label, fill);
	PARMTEMPLATE.ss[LABEL_TEXT].changed = 1;
	PARMTEMPLATE.ss[COMPARTMENTS].changed = 1;
	EXITFUNC("comp_toggle_validate");
	return 1;
}

/*
 * helper sort routine for qsort
 */

static int
tstrcmp(ptr1, ptr2)
	char **ptr1;
	char **ptr2;
{
	return strcmp(*ptr1, *ptr2);
}

/*
 * Initialize the fillin structure based on the incoming label
 */

static int
initialize_fillin(initial_label, fill)
	mand_ir_t *initial_label;
	register struct label_fillin *fill;
{
	int i;
	char *cp;
	static int initialized = 0;

	ENTERFUNC("initialize_fillin");
	if (!initialized) {
		struct a {
			char *string;
			struct a *next;
		} *ap, *head = NULL;
		int label_width, widest_class = 0;

		/*
		 * copy the initial label into a newly allocated buffer
		 */

		fill->old_label = mand_alloc_ir();
		if (fill->old_label == NULL)
			MemoryError();

		/*
		 * Create a table of classifications
		 */

		fill->nclasses = 0;
		for (i = 0; i <= mand_max_class; i++) {
			if (cp = mand_cltoname(i)) {

				/* keep track of widest classification */

				int class_width = strlen(cp);

				if (class_width > widest_class)
					widest_class = class_width;

				fill->nclasses++;
				ap = (struct a *) Malloc(sizeof(*ap));
				if (ap == NULL)
					MemoryError();
				ap->string = cp;
				if (head)
					ap->next = head;
				else
					ap->next = NULL;
				head = ap;
			}
		}
		fill->class_table = (char **) Calloc(fill->nclasses,
							sizeof(char *));
		if (fill->class_table == NULL)
			MemoryError();

		/*
		 * put the classifications in the array in the reverse
		 * order they were found in.
		 */

		for (ap = head, i = fill->nclasses - 1; ap; i--) {
			struct a *oap;
			fill->class_table[i] = ap->string;
			oap = ap;
			ap = ap->next;
			free(oap);
		}
		head = NULL;

		/*
		 * allocate a table to tell which classifications are selected
		 */

		fill->class_select = Calloc(fill->nclasses, 1);
		if (fill->class_select == NULL)
			MemoryError();

		/*
		 * Create a table of compartments
		 */

		fill->ncomps = 0;
		label_width = 0;
		for (i = 0; i <= mand_max_cat; i++) {
			if (cp = mand_cattoname(i)) {

				/* keep track of width of all compartments */
				label_width += strlen(cp);

				fill->ncomps++;
				ap = (struct a *) Malloc(sizeof(*ap));
				if (ap == NULL)
					MemoryError();
				ap->string = cp;
				if (head)
					ap->next = head;
				else
					ap->next = NULL;
				head = ap;
			}
		}
		fill->comp_table = (char **) Calloc(fill->ncomps,
							sizeof(char *));
		if (fill->comp_table == NULL)
			MemoryError();

		/*
		 * put the compartments in the array and sort them
		 */

		for (ap = head, i = fill->ncomps - 1; ap; i--) {
			struct a *oap;
			fill->comp_table[i] = ap->string;
			oap = ap;
			ap = ap->next;
			free(oap);
		}
		head = NULL;

		qsort(fill->comp_table, fill->ncomps,
			sizeof(fill->comp_table[0]), tstrcmp);

		/*
		 * allocate a table to tell which compartments are selected
		 */

		fill->comp_select = Calloc(fill->ncomps, 1);
		if (fill->comp_select == NULL)
			MemoryError();

		/*
		 * allocate a string wide enough to hold the widest label
		 */

		fill->ntext = 
			widest_class + label_width +
			(fill->ncomps - 1) * 2 +	/* ", " separator */
			4;				/* "//", " ", \0 */
			
		fill->text_label = Malloc(fill->ntext);
		if (fill->text_label == NULL)
			MemoryError();

		/*
		 * allocate the label table for the character strings
		 */

		fill->nlabel_tab = (fill->ntext+SL_TEXT_WIDTH-1)/SL_TEXT_WIDTH;
		fill->label_tab = alloc_cw_table(fill->nlabel_tab,
							SL_TEXT_WIDTH + 1);
		if (fill->label_tab == (char **) 0)
			MemoryError();

		initialized = 1;
	}

	/* update the scrolling region to reflect the new label */

	set_label(initial_label, fill);
	EXITFUNC("initialize_fillin");
	return 0;
}

/*
 * initialize for the this/future option
 * set up the last two descs for the this/future toggles
 */

static void
initialize_this_future(this_ptr, future_ptr)
	char *this_ptr;
	char *future_ptr;
{
	int i;
	int first_desc;

	HowCalled = ThisFuture;

	first_desc = SL_THIS_DESC;
	for (i = 0; i < NUMDESCS(SL_this_future_desc); i++)
		SL_desc[first_desc++] = SL_this_future_desc[i];
}

/*
 * initialize for the default SL option
 * set up the last three descs for the use default toggle and the
 * default SL text region
 */

static void
initialize_default_sl(use_default, default_sl, fill)
	char *use_default;
	mand_ir_t *default_sl;
	register struct label_fillin *fill;
{
	int i;
	int first_desc;
	char *default_text;

	HowCalled = UseDefault;

	/* copy optional descs and set the number of descs for the screen */

	first_desc = SL_DEF_SL_TOGGLE_DESC;
	for (i = 0; i < NUMDESCS(SL_default_desc); i++)
		SL_desc[first_desc++] = SL_default_desc[i];

	fill->def_sl_ir = default_sl;
	fill->use_default = *use_default;

	/* map the specified default sl and create the text table */

	default_text = mand_ir_to_er(default_sl);
	fill->def_label_tab = alloc_cw_table(fill->nlabel_tab,
							SL_TEXT_WIDTH + 1);
	if (fill->def_label_tab == (char **) 0)
		MemoryError();
	
	setup_label_table(default_text,
				fill->def_label_tab, fill->nlabel_tab);
}

/*
 * initialize for the case where neither this/future nor default SL
 * are required.  The last three descs are unused.
 */

static void
initialize_just_sl()
{
	int i;
	int first_desc = SL_NEITHER_FIRST_DESC;

	HowCalled = SLonly;

	for (i = 0; i < NUMDESCS(SL_neither_desc); i++)
		SL_desc[first_desc++] = SL_neither_desc[i];
}

/*
 * Copy the screen representation into the text representation.
 */

static void
setup_label_table(text_label, label_tab, nlabel_tab)
	char *text_label;
	char **label_tab;
	int nlabel_tab;
{
	int i, len;

	len = strlen(text_label);
	for (i = 0; i < nlabel_tab; i++) {
		if (len > i * SL_TEXT_WIDTH) {
			strncpy(label_tab[i],
			       &text_label[i * SL_TEXT_WIDTH],
			       SL_TEXT_WIDTH);
			label_tab[i][SL_TEXT_WIDTH] = '\0';
		} else
			label_tab[i][0] = '\0';
	}
}

/*
 * update the scrolling region to reflect a label
 */

static void
set_label(label, fill)
	mand_ir_t *label;
	struct label_fillin *fill;
{
	int i;
	char *cp;

	ENTERFUNC("set_label");
	/*
	 * Initialize the filled fields based on the passed-in label
	 */

	memset(fill->class_select, '\0', fill->nclasses);
	memset(fill->comp_select,  '\0', fill->ncomps);

	/*
	 * Mark the proper classification as selected
	 */

	cp = mand_cltoname(label->class);
	for (i = 0; i < fill->nclasses; i++)
		if (!strcmp(fill->class_table[i], cp))
			fill->class_select[i] = 1;

	/*
	 * Mark the proper compartments as selected
	 */

	for (i = 0; i <= mand_max_cat; i++)
		if (ISBITSET(label->cat, i)) {
			int j;

			cp = mand_cattoname(i);
			for (j = 0; j < fill->ncomps; j++)
				if (!strcmp(fill->comp_table[j], cp))
					fill->comp_select[j] = 1;
		}

	/*
	 * Set text representation to expanded label
	 */

	cp = mand_ir_to_er(label);
	memset(fill->text_label, '\0', fill->ntext);
	strcpy(fill->text_label, cp);

	/*
	 * Copy the string to the on-screen scrolled region
	 */

	setup_label_table(fill->text_label, fill->label_tab, fill->nlabel_tab);

	/*
	 * Copy the label into the saved IR buffer.
	 */

	if (label != fill->old_label)
		mand_copy_ir(label, fill->old_label);
	EXITFUNC("set_label");
}

/*
 * Fill in the scrn_struct
 */

static int
SL_bstruct(lf, sptemplate)
	struct label_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("SL_bstruct");
	lf->nstructs = NSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* Zero the scrn_struct table (some may not be filled in) */

	sp[LABEL_TEXT].pointer = (char *) lf->label_tab;
	sp[LABEL_TEXT].filled  = lf->nlabel_tab;

	/*
	 * smash the expanded screen representation into the
	 * scrolling text region.
	 */

	setup_label_table(lf->text_label, lf->label_tab,
				lf->nlabel_tab);

	sp[LABEL_TEXT].val_act = typed_label_validate;
	sp[CLASSIFICATIONS].pointer = (char *) lf->class_table;
	sp[CLASSIFICATIONS].filled = lf->nclasses;
	sp[CLASSIFICATIONS].state = lf->class_select;
	sp[CLASSIFICATIONS].val_act = class_toggle_validate;
	sp[CLASSIFICATIONS].validate = NULL;
	sp[COMPARTMENTS].pointer = (char *) lf->comp_table;
	sp[COMPARTMENTS].filled = lf->ncomps;
	sp[COMPARTMENTS].state = lf->comp_select;
	sp[COMPARTMENTS].val_act = comp_toggle_validate;
	sp[COMPARTMENTS].validate = NULL;

	/* Set up for this/future if specified to aif_label */

	switch (HowCalled) {
	case ThisFuture:
		sp[THIS_SESSION].validate = NULL;
		sp[THIS_SESSION].val_act = NULL;
		sp[THIS_SESSION].pointer = &lf->this;
		sp[THIS_SESSION].filled = 1;
		sp[FUTURE_SESSIONS].validate = NULL;
		sp[FUTURE_SESSIONS].val_act = NULL;
		sp[FUTURE_SESSIONS].pointer = &lf->future;
		sp[FUTURE_SESSIONS].filled = 1;
		switch (CheckAuditEnabled()) {
		case 1:		/* audit enabled, daemon running */
			SL_desc[SL_THIS_DESC].inout = FLD_BOTH;
			break;
		case -1:        /* audit enabled, no daemon */
		case 0:         /* audit is not enabled */
			SL_desc[SL_THIS_DESC].inout = FLD_OUTPUT;
			break;
		}
		break;

	case UseDefault:

		/* Set up for Use Default if specified to aif_label */

		sp[USE_DEFAULT_TOG].pointer = &lf->use_default;
		sp[USE_DEFAULT_TOG].filled = 1;
		sp[USE_DEFAULT_TOG].val_act = use_default_sl;
		sp[USE_DEFAULT_TOG].validate = NULL;

		sp[DEFAULT_TEXT].pointer = (char *) lf->def_label_tab;
		sp[DEFAULT_TEXT].filled = lf->nlabel_tab;
		sp[DEFAULT_TEXT].validate = NULL;
		sp[DEFAULT_TEXT].val_act = NULL;
		break;
	default:

		/* remove residual values from previous use */

		sp[NEITHER_1].pointer = NULL;
		sp[NEITHER_1].changed = 0;
		sp[NEITHER_1].filled = 0;
		sp[NEITHER_2].pointer = NULL;
		sp[NEITHER_2].changed = 0;
		sp[NEITHER_2].filled = 0;
		break;
	}

	EXITFUNC("SL_bstruct");
	return 0;
}

/*
 * set up the screen to use the default sensitivity label
 */

static int
use_default_sl()
{
	int ret;
	int i;

	ENTERFUNC("use_default_sl");
	/* user turned use_default off */

	if (sl_fill->use_default == 0) {
		ret = 0;
	}
	else {
		struct scrn_struct *sp = PARMTEMPLATE.ss;
		set_label(sl_fill->def_sl_ir, sl_fill);
		PARMTEMPLATE.ss[LABEL_TEXT].changed = 1;
		PARMTEMPLATE.ss[CLASSIFICATIONS].changed = 1;
		PARMTEMPLATE.ss[COMPARTMENTS].changed = 1;
		ret = 1;
	}

	EXITFUNC("use_default_sl");
	return ret;
}

/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static int
SL_bfill(lf)
	struct label_fillin *lf;
{
	ENTERFUNC("SL_bfill");
	memcpy(lf, &SL_save_fill, sizeof(*lf));
	lf->this = (char) 0;
	lf->future = (char) 0;
	SL_hdrs.c1 = MSGSTR(AIF_329,
        "RET=Execute  ESC=Leave menu  ^Y=Item help  ^B=Quit Program");
	EXITFUNC("SL_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static int
SL_auth(argv, lf)
	char **argv;
	struct label_fillin *lf;
{
	ENTERFUNC("SL_auth");
	IsTyping = 0;
	Checked = 0;
	ErrorLabel = 0;
	EXITFUNC("SL_auth");
	return 0;
}

/*
 * validate function for the whole screen.
 * If this/future is supported, make sure that at least one is selected.
 */

static int
SL_valid(argv, lf)
	char **argv;
	struct label_fillin *lf;
{
	int ret = 0;
	static char **msg_error, *msg_error_text;

	ENTERFUNC("SL_valid");
	/* if typing, call type_check_valid and respond appropriately */
	if (IsTyping) {
		ret = type_check_valid(lf);
		if (ErrorLabel)	/* Error in typing, stay on screen */
			ret = 1;
		else if (Checked) /* Parsed but not executed, stay on screen */
			ret = 1;
	}
	if (ret == 0 && HowCalled == ThisFuture) {
		if (lf->this == 0 && lf->future == 0) {
			if (!msg_error)
				LoadMessage(MSGSTR(SLABEL_2,
					"msg_isso_audit_events"),
				&msg_error, &msg_error_text);
			ErrorMessageOpen(-1, msg_error, 32, NULL);
			ret = 1;
		}
	}
	EXITFUNC("SL_valid");
	return ret;
}

/*
 * routine to free memory.
 * since sensitivity labels may be re-used, keep memory allocated.
 */

static int
do_free()
{
	ENTERFUNC("do_free");
	EXITFUNC("do_free");
	return 1;
}

#ifdef PRINTSCR

/* function to print out a screen */

void
aif_print(title, subtitle,
	  ir_buffer, this_ptr, future_ptr,
	  use_default, default_sl, dflt_sl_string,
	  help)
	char *title;		/* screen title */
	char *subtitle;		/* title of first text field */
	mand_ir_t *ir_buffer;	/* internal representation */
	char *this_ptr;		/* pointer if "Current Session" toggle */
	char *future_ptr;	/* pointer if "Future Session" toggle */
	char *use_default;	/* pointer if default SL specified */
	mand_ir_t *default_sl;	/* pointer to IR if default SL */
	char *dflt_sl_string;	/* string to appear as "Use default" toggle */
	char *help;		/* help screen identifier */
{
	if (aif_setup(title, subtitle, ir_buffer, this_ptr, future_ptr,
			use_default, default_sl, dflt_sl_string, help))
		return;

	printscreen(&SL_scrn);
}
#endif /* PRINTSCR */

#define SETUPFUNC	SL_setup	/* defined by stemplate.c */
#define AUTHFUNC	SL_auth
#define BUILDFILLIN	SL_bfill

#define INITFUNC	SL_init		/* defined by stemplate.c */
#define BUILDSTRUCT	SL_bstruct

#define ROUTFUNC	SL_exit		/* defined by stemplate.c */
#define VALIDATE	SL_valid
#define SCREENACTION	SL_action

#define FREEFUNC	SL_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"

#endif /*} SEC_MAC */
