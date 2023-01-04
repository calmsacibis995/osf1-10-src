
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
static char *rcsid = "@(#)$RCSfile: latsetup.c,v $ $Revision: 1.1.7.5 $ (DEC) $Date: 1993/11/09 21:08:48 $";
#endif

#include "latsetup.h"

/* forward declarations */
void signalexit();

static char *main_menu[] = {
			"Exit",
			"Initial LAT Setup",
			"Create Additional Devices",
			"Start/Stop LAT",
			"Enable/Disable LAT Automatic Startup/Shutdown",
			"Undo LAT Setup"
};

/* these global variables are used by the INITIAL_LAT_SETUP, START/STOP and */
/* ENABLE/DISABLE sections of code */

struct latioctl_node latioctl_node;

/* these global variables are used throughout */

struct latttyinfo lat_ttys[MAX_DEVS];		/* everything we need to know
						   about the ttys */

int lat_minors[MAX_DEVS];			/* state of LAT minor #'s
						   1=allocated, 0=free */

int lat_entries_total = 0;			/* total # of LAT entries in
						   inittab at any given time */

int lat_dev_total = 0;				/* total # of LAT ttys in /dev
						   at any given time */

int non_lat_dev_total = 0;			/* total # of non-LAT ttys in 
						   /dev at any given time */

int latfd;			/* global file descriptor for /dev/lat */

int y;				/* value which holds current y location */

int curses = FALSE;		/* Determine if curses are being used */


main(argc, argv)
int argc;
char **argv;
{

	initialize();

	/* Determine if they want to force non curses mode */

	if ( argc > 1 ) {
		if ( strcmp(argv[1], "-nocurses") == 0 )
			curses = FALSE;
		else
			if ( strcmp(argv[1], "-zimbabwe") == 0 )
				curses = FALSE;
			else {
				fprintf(stdout, "usage: latsetup [-nocurses]\n\n");
				exit(2);
			}
	}
	else
		check_term();	/* else check TERM for curses capability */

	if ( curses ) {
		init_curses();
		enter_main_menu();
	} else
		non_curses_enter_main_menu();

	clean_n_exit(0);

}   /* end of main() */

check_term()
{
	int err_ret;
	register char *cp = getenv("TERM");

	curses = TRUE;
	setupterm(cp,fileno(stdout),&err_ret);	/* initialize cur_term */
	if (err_ret == -1) {	/* setupterm can't initialize - serious error */
		curses = FALSE;
		return;		/* don't bother with checking capabilities */
	}

	/* IF the termcap entry says that:
 	*      this is a hard-copy terminal, or
 	*      this terminal doesn't support screen clearing, or
 	*      this terminal requires over-striking, or
 	*      this terminal doesn't support cursor movement
 	* THEN use non curses mode.  Note: this covers "dumb" and "unknown".
	*/

	if ( (hard_copy || clear_screen == 0 || over_strike) ||
	   !((cursor_up || cursor_home) && cursor_down &&
	     (cursor_left || carriage_return)) ) {

		curses = FALSE;
	}

	if ( !curses ) {
		fprintf(stdout, "\tYour terminal is not capable of using curses mode.\n");
		fprintf(stdout, "\tCheck your TERM environment variable.\n\n",cp);
	}
} /* end of check_term */


/*
 * This routine is called before latsetup allows the user to do anything.
 * It sets up certain signals to be caught, and before allowing the user
 * to continue, it makes sure the user is super user, checks to make sure
 * LAT is in running kernel, and makes sure we are at run-level 3.  If LAT
 * is not in the running kernel and there are LAT entries in /etc/inittab,
 * then latsetup will allow the user to remove these entries from inittab
 * before exitting.  This routine will also create /dev/streams/kinfo,
 * create /dev/lat if it doesn't exist or is improperly setup, and will
 * initialize the lat_ttys[] array.
 */
initialize()
{
#define BOOTEDFILELEN 80
     char bootedfile[BOOTEDFILELEN];

	struct nlist nl[] = {
        	{ "lat_open" },
        	"",
	};
	struct utmp *utmp;
	char runlevel_found = 0;
	struct stat statbuf;
	int ret, status;
 	char buf[512], not_done, remove_entries;
	pid_t child;

	signal(SIGINT,signalexit);
	signal(SIGHUP,signalexit);
	signal(SIGQUIT,signalexit);
	signal(SIGKILL,signalexit);

	/* Check for super-user. */
	if ( getuid() != 0 ) {
		fprintf(stderr,
			"You must su to root before executing latsetup\n\n");
		exit(2);
	}

	/* 
	 * Call getutent() to see what run-level we're at.  We need this
	 * info to decide whether or not to call perform_initq() after removing
	 * the LAT configuration via remove_inittab_entries() AND also to make 
	 * sure we're at run-level 3 during normal operation.
	 */
	 
	while ( (utmp = getutent()) != NULL )
		if ( utmp->ut_type == RUN_LVL ) {
			runlevel_found++;
			break;
		}

	/* Get the name of the running kernel. */
	bootedfile[0] = '/';
	getsysinfo(GSI_BOOTEDFILE, &bootedfile[1], BOOTEDFILELEN, 0, 0);

	/* Determine if LAT is in the running kernel. */
	if ( nlist(bootedfile, nl) < 0) {
		fprintf(stderr, "Could not find kernel %s.\n", bootedfile);
		exit(2);
	}
	if ( nl[0].n_type == 0 ) {  /* if LAT is not configured into kernel */
		fprintf(stdout,
                     "LAT has not been built into the running kernel, %s.\n",
		     bootedfile);

		if (init_tty_info(COUNT_INITTAB_TOTAL,OUT_STDERR) == FAILURE) {
			fprintf(stdout,"Fatal Error: cannot initialize - insufficient memory\n");
			exit(2);
		}

 		if (lat_entries_total == 0)
			/* There are no LAT entries in /etc/inittab */
 			fprintf(stdout, "latsetup cannot be executed.\n");
 
 		else {
			/* There are LAT entries in /etc/inittab */
 			not_done = remove_entries = TRUE;
 			while (not_done) { 
 				fprintf(stdout,
 			   	  "\n\nThere are %d LAT entries in /etc/inittab",lat_entries_total);
 				fprintf(stdout," that will be removed.\n");
 				fprintf(stdout,
 				  "Type <c> to continue or <q> to quit: ");
                		fflush(stdout);
                		buf[0] = '\0';
        			if (gets(buf) == NULL)
					continue;
				if (buf[0] == 'q' || buf[0] == 'Q') {
 					not_done = FALSE;
 					remove_entries = FALSE;
 				}
				if (buf[0] == 'c' || buf[0] == 'C') {
 					not_done = FALSE;
 					remove_entries = TRUE;
 				} 
         		}  /* while not_done_2 */
 
 			if (remove_entries) { /* remove entries from inittab */
 				remove_inittab_entries(lat_entries_total, OUT_STDERR);
				if (sync_inittab(OUT_STDERR) != SUCCESS) {
 					fprintf(stdout,"Error: could not rewrite /etc/inittab\n");
					exit(2);
				}
				if ( runlevel_found )
					if (perform_initq(OUT_STDERR) != SUCCESS) 
 						fprintf(stdout,"Error: could not\n");
 			} else
 				fprintf(stdout,
 				   "Exiting...  /etc/inittab unchanged.\n");
 		} /* else there are LAT inittab entries... */
		exit(2);
	}


	/* Make sure we're at run-level 3. */
	if ( runlevel_found ) {
		if ( strcmp(utmp->ut_line, RUN_LEVEL_3) != 0 ) {
			fprintf(stderr,"Please bring the system to run-level");
			fprintf(stderr," 3 before running latsetup.\n\n");
			exit(2);
		}
	} else {
		fprintf(stderr, "Could not check current run level\n\n");
		exit(2);
	}

	/* Create necessary streams device /dev/streams/kinfo */
	if ((child = fork()) == 0) {
		close(1);	/* Close stdout */
		execl("/usr/sbin/strsetup", "strsetup", "-i", 
			 (char *)0);    
		exit(1);
	}
	waitpid(child, &status, 0);
	if (status) {
		fprintf(stderr, "UNABLE to configure STREAMS devices\n");
		exit(2);
	}

	/* check to see if the LATCPDEV exists by using the stat system */
	/* call.  If the device doesn't exist, we will create it in the */
	/* make_latcpdev() routine.  If the device exists, but not as a */
	/* character device, we will remove the file and create a new one, */
	/* the way we want it to be by the make_latcpdev() routine. */
        /* If the device exists but with an incorrect major or minor number, 
	/* then we will remove the file and create a new one. */

	if (stat(LATCPDEV, &statbuf) == 0) {		/* device exists */

                /* if device is not char or device has wrong minor number */
                if (!(S_ISCHR(statbuf.st_mode)) ||
                     (major(statbuf.st_rdev) != LAT_MAJOR) ||
                     (minor(statbuf.st_rdev) != CLONE_MINOR)) {
			/* remove the old file - send output to stderr */
			ret = rm_file(LATCPDEV, OUT_STDERR);
			if (ret != SUCCESS) {
				exit(2);
			}
			make_latcpdev();		/* go create new one */
		}
	} else {				/* device doesn't exist. */
		make_latcpdev();		/* go create new one */
	}

	/* 
	 * Open the clonable device for global use.
 	 */
	if ( (latfd = open(LATCPDEV, O_RDWR)) < 0 ) {
		fprintf(stderr,"Fatal error: could not open %s\n", LATCPDEV);
		perror(LATCPDEV);
		exit(2);
	}

	if (init_tty_info(INIT_ALL,OUT_STDERR) == FAILURE) {
		fprintf(stdout,"Fatal Error: cannot initialize - insufficient memory\n");
		exit(2);
	}

	return(0);

}  /* end of function initialize() */

/*
 * This routine is called to create the LATCPDEV.
 */
make_latcpdev()
{
	int ret;
	mode_t lmode;

	/*
	 * Create necessary LAT device, LATCPDEV.  The permissions
	 * of the file should be (octal) 020666.
	 */

	lmode = S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH; 

	if ((ret = mknod(LATCPDEV,lmode,makedev(LAT_MAJOR,CLONE_MINOR))) < 0) {
		fprintf(stderr, "UNABLE to create %s device\n", LATCPDEV);
		exit(2);
	}

	/* set LATCPDEV with the appropriate permissions */

	if ((ret = chmod(LATCPDEV,lmode)) != 0) {
		fprintf(stderr, "UNABLE to change permissions of %s\n", LATCPDEV);
		exit(2);
        }

}


/*
 * This routine will set everything up so that curses can be used.
 */
init_curses()
{
	/* Initialize terminal settings and allocate space for windows */ 
	initscr();

	/* Set up character-at-a-time input with no echo */
	nonl();
	cbreak();
	noecho();

	keypad(stdscr,TRUE);	/* enable KEY_* translations */

}  /* end of function init_curses() */


/*
 * This routine prepares for exitting curses mode, closes LATCPDEV and exits 
 * with a status of exit_status.
 */
clean_n_exit(int exit_status)
{
	struct stat statb;

	if ( curses == TRUE ) {
		/* Prepare for exiting curses */
		move(LINES-2, COLS-1);
		addstr(" "); refresh();
		endwin(); 	/* no more curses */
	}

	close(latfd);		/* close global /dev/lat fd */

	/* remove temp files */
	if (stat(INITTAB_TEMPLATE,&statb) == 0)
		rm_file(INITTAB_TEMPLATE,OUT_STDERR);	
	if (stat(INITTAB_TMP_FILE,&statb) == 0)
		rm_file(INITTAB_TMP_FILE,OUT_STDERR);	

	exit(exit_status);

}  /* end of function clean_n_exit() */



enter_main_menu()
{
	int option, not_done = 1;

	while ( not_done ) {

	        option = get_main_option();
	        switch ( option ) {

	        case EXIT:
	             not_done = 0;
	             break;
	        case INITIAL_SETUP:
	             initial_lat_setup(); get_CR();
	             break;
	        case CREATE_DEV:
	             create_devices(MAIN_MENU_CALLER);	get_CR();
	             break;
	        case START_STOP:
	             start_stop();	get_CR();
	             break;
	        case E_D_AUTO_S_S:
	             en_dis_auto();	get_CR();
	             break;
                case UNDO_SETUP:
                     undo_lat_setup();  get_CR();
                     break;

	        }  /* end of switch(option) */

	}  /* end of while(not_done) */
		
}   /* end of function enter_main_menu() */


void
print_main_menu()
{
	clear(); 
	move(MIN_Y-7, 31); attrset(A_BOLD);
	addstr("LAT Setup Utility");
	attrset(0);
	move(MIN_Y-5, 7);
	addstr("The latsetup program provides assistance in setting up LAT on");
	move(MIN_Y-4, 7);
	addstr("your system.  If this is the first time running latsetup on this");
	move(MIN_Y-3, 7);
	addstr("system, it is recommended to select the ");
	attrset(A_UNDERLINE); addstr("Initial LAT Setup");
	attrset(0); addstr(" option."); 

	move(MIN_Y+7, 7);
	addstr("To select an option, use the <UP> or <DOWN> arrow keys and hit");
	move(MIN_Y+8, 7);
	addstr("<RETURN> to enter the chosen submenu.  To select options on a");
	move(MIN_Y+9, 7);
	addstr("horizontal plane use the <LEFT> or <RIGHT> arrow keys and hit");
	move(MIN_Y+10, 7);
	addstr("<RETURN>.  To enter input, type it in and then hit <RETURN>.");

	print_main_options(EXIT);  /* With EXIT option selected */
	refresh();

}  /* end of funtion print_main_menu() */


print_main_options(option_highlighted)
  int option_highlighted;
{
	int i;

	/* Print main menu options and highlight option specified by */
	/* option_highlighted to show that this is the current selected. */

	for ( i=0; i< NUM_ENTRIES; i++ ) {
		move(MIN_Y+i, MAIN_X);
		if ( option_highlighted == i ) attrset(A_REVERSE);
		addstr(main_menu[i]);
		if ( option_highlighted == i ) attrset(0);
	}
}   /* end of function print_main_options() */

int
get_main_option()
{
	int c;
	int y, x, option, not_done = 1;

	print_main_menu();

	move(MIN_Y+EXIT, MAIN_X); getyx(stdscr, y, x); x--;
	option = EXIT;
	while ( not_done ) {
		move(y, x); refresh(); c = getch();

		if ( c == KEY_UP ) {
		     y--; option--;
		     /*
		      * When adding items to the main menu, make sure that
		      * in the following instruction, "option" is set to
		      * the last entry in the list.
		      */
	     	     if ( y < MIN_Y ) { y = MAX_Y; option = UNDO_SETUP; }
		} 
		else
		if ( c == KEY_DOWN ) {
		     y++; option++;
	     	     if ( y > MAX_Y ) { y = MIN_Y; option = EXIT; }
		} 
		else
		if (IS_RETURN(c))
		     not_done = 0;

		print_main_options(option);
		refresh();

	}  /* end of while ( not_done ) */

	clear(); refresh();
	return(option);

}   /* end of function get_main_option() */


/*
 * This routine prints out a message and waits until the user
 * hits the carriage return.
 */
get_CR()
{
	int c;

	attrset(A_BLINK);   move(CR_Y, CR_X);
	addstr("Enter <RETURN> to return to main menu");
	refresh();   attrset(0);
	while ((c = getch()) && !IS_RETURN(c)) ;

}  /* end of function get_CR() */


introduction()
{
	int y_loc=17;
	int intro_ret = -1;

	clear(); 
	attrset(A_BOLD);
	mvaddstr(1, 29, "Initial LAT Setup");
	attrset(0);
	mvaddstr(4, 4,
	  "This menu will allow you to setup LAT on your system by creating LAT");
	mvaddstr(5, 4,
	  "special device files, selecting how many getty entries should be placed");
	mvaddstr(6, 4,
	  "into /etc/inittab, choosing whether or not to execute init q, and starting");
	mvaddstr(7, 4,
	  "LAT to allow interactive connections to be made to this system.  A");
	mvaddstr(8, 4,
	  "variable will also be set in /etc/rc.config so that LAT automatic startup");
	mvaddstr(9, 4,
	  "and shutdown is enabled which will cause LAT to be started each time ");
	mvaddstr(10, 4, "this system reaches run-level 3.");
	mvaddstr(15, 15, "Would you like to continue with this option?");

	switch (intro_ret = get_yes_no(y_loc) ) {

	case YES:  /*  can't see display create devices overwrites  */
		mvaddstr(15, 15, "Would you like to continue with this option?  YES");
		clrtobot();
		refresh();
		break;
	case NO:
		mvaddstr(15, 15, "Would you like to continue with this option?  NO");
		clrtobot();
		refresh();
		attrset(A_BOLD);
		move(17, 19);
		addstr("You have chosen not to continue.");
		attrset(0);
	}
	move(16,16);
	clrtobot();
	return(intro_ret);
}  /* end of function introduction */


initial_lat_setup()
{
	int y_start = 15;
	FILE *fp;
	char lat_start[S_S_BUF_SIZE];

	if (introduction() == NO)
		return;

	if (create_devices(INITIAL_SETUP) == FAILURE)
		return;

        if (set_LAT_SETUP() < 0)  /*  set LAT_SETUP to 1 in rc.config */
		return;

/* Start LAT if user says to start it */
	if (enter_start_lat() == YES) 
		start_lat();

}  /* end of function initial_lat_setup() */


enter_start_lat()
{
	int do_start = NO;    /* no start if active */
	int y_loc = 16;

/* get state of node if active or inactive */
        bzero(&latioctl_node, sizeof(latioctl_node));
        latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
        if (ioctl(latfd, LIOCGNODE, &latioctl_node) < 0) {
                fprintf(stderr, "LIOCGNODE ioctl failed. errno=%d.\n", errno);
                clean_n_exit(2);
        }

        if (!latioctl_node.ln_state) {
		if (curses) {
                	move(11,23);
                	addstr("LAT Protocol is not active");
                	move(13,16);
                	addstr("Would you like to start the LAT Protocol?");
                	do_start =  get_yes_no(y_loc);
                	move(13,16);
                	addstr("Would you like to start the LAT Protocol?");
        		if (do_start == YES) {
                		addstr("  YES");  /* user wants to start LAT */
        		} else {
                		addstr("  NO");  /* user doesn't want to start LAT */
        		}
			clrtobot();
			refresh();
                } else {
                        printf("\nLAT Protocol is not active\n");
                        if (yesno(NO_CUR_YES,"Would you like to start the LAT Protocol?"))
                                do_start = YES;
                }

	}
	return(do_start);

} /* end of function enter_start_lat() */



en_dis_auto()
{
	int sys_ret;
	int y_loc=12;
	FILE *fd;
	char lat_setup[4];    /* lat_setup buffer area */
	clear(); refresh();
	attrset(A_BOLD);
	move(4, 5);
	addstr("This menu is used to enable or disable LAT automatic startup/shutdown.");
	attrset(0);
	lat_setup[0] = '\0';
	if ((fd = popen("/usr/sbin/rcmgr get LAT_SETUP", "r")) == NULL) {
		fprintf(stderr,"Lat auto: Could not get LAT_SETUP");
		return;
	}
	fgets(lat_setup,4,fd);
	pclose(fd);
	if (lat_setup[0] != '1') {
		move(8,19);
		addstr("LAT automatic startup/shutdown is disabled.");
		move(10,13);
		addstr("Would you like to enable LAT automatic startup/shutdown?");
                switch ( get_yes_no(y_loc) ) {

                case YES:
			move(10,13);
			addstr("Would you like to enable LAT automatic startup/shutdown?  YES");
			clrtobot();
			refresh();
/*  Set LAT_SETUP in rc.config for automatic startup/shutdown */
                        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 1");
                        if (system_err_check(sys_ret) < 0 )
                                return;

			attrset(A_BOLD);
			move(15,19);
			addstr("LAT automatic startup/shutdown is enabled.");
			attrset(0);
                        break;
                case NO:
			move(10,13);
			addstr("Would you like to enable LAT automatic startup/shutdown?  NO");
			clrtobot();
			refresh();
			attrset(A_BOLD);
                        move(15, 11);
                        addstr("You have chosen not to enable LAT automatic startup/shutdown.");
			attrset(0);
                        break;

                }  /* end of switch(get_yes_no) */
	} else {
		move(8,19);
		addstr("LAT automatic startup/shutdown is enabled.");
		move(10,13);
		addstr("Would you like to disable LAT automatic startup/shutdown?   ");
                switch ( get_yes_no(y_loc) ) {

                case YES:
			move(10,13);
			addstr("Would you like to disable LAT automatic startup/shutdown?  YES");
			clrtobot();
			refresh();
/*  Reset LAT_SETUP in rc.config for automatic startup/shutdown */
                        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 0");
                        if (system_err_check(sys_ret) < 0 )
                                return;

			attrset(A_BOLD);
			move(15,19);
			addstr("LAT automatic startup/shutdown is disabled.");
			attrset(0);
                        break;
                case NO:
			move(10,13);
			addstr("Would you like to disable LAT automatic startup/shutdown?  NO");
			clrtobot();
			refresh();
			attrset(A_BOLD);
                        move(15, 11);
                        addstr("You have chosen not to disable LAT automatic startup/shutdown.");
			attrset(0);
                        break;

                }  /* end of switch(get_yes_no) */
	}
	refresh();

}  /* end of function en_dis_auto() */


start_stop()
{
	int sys_ret;
        int y_loc = 12;
	int y_start = 14;
	char lat_setup[4];    /* lat_setup buffer area */
	char lat_start[S_S_BUF_SIZE];    /* lat_start buffer area */
	FILE *fp;

	clear(); 
	refresh();
	attrset(A_BOLD);
	move(2, 16);
	addstr("This menu is used to start or stop LAT.");
	attrset(0);

	bzero(&latioctl_node, sizeof(latioctl_node));
	latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
	if (ioctl(latfd, LIOCGNODE, &latioctl_node) < 0) {
                move(y_loc,16);
		printw("LIOCGNODE ioctl failed. errno=%d.\n", errno);
		refresh();
		return;
	}

	if (!latioctl_node.ln_state) {
		move(7,23);
		addstr("LAT Protocol is not active");
		move(10,16);
                addstr("Would you like to start the LAT Protocol?");
                switch ( get_yes_no(y_loc) ) {

                case YES:
			move(10,16);
                	addstr("Would you like to start the LAT Protocol?  YES");
			clrtobot();
			mvaddstr(y_start,16,"Attempting to start the driver.  Please wait...");
			refresh();

			bzero(&latioctl_node, sizeof(latioctl_node));
			latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
			latioctl_node.ln_state = LAT_STATE_ON;
			if (ioctl(latfd, LIOCSNODE, &latioctl_node) < 0) {
                		move(y_loc,16);
				clrtobot();
				printw("LAT NOT started. (LIOCSNODE failed, errno=%d)\n",errno);
				refresh();
			} else {
				move(y_start,16);
				clrtobot();
				mvaddstr(y_start,16,"LAT started.");
				refresh();
			}

                        break;
                case NO:
			move(10,16);
                	addstr("Would you like to start the LAT Protocol?  NO");
			clrtobot();
			refresh();
			attrset(A_BOLD);
                        move(15, 19);
                        addstr("You have chosen not to start LAT.");
			attrset(0);
                        break;

                }  /* end of switch(get_yes_no) */
	}
	else {
		move(7,23);
		addstr("LAT Protocol is active");
		move(10,16);
                addstr("Would you like to stop the LAT Protocol?");
                switch ( get_yes_no(y_loc) ) {

                case YES:
			move(10,16);
                	addstr("Would you like to stop the LAT Protocol?  YES");
			clrtobot();
			mvaddstr(y_start,16,"Attempting to stop the driver.  Please wait...");
			refresh();

			bzero(&latioctl_node, sizeof(latioctl_node));
			latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
			latioctl_node.ln_state = LAT_STATE_OFF;
			if (ioctl(latfd, LIOCSNODE, &latioctl_node) < 0) {
                		move(y_loc,16);
				clrtobot();
				printw("LIOCSNODE ioctl failed. errno=%d.\n", errno);
				refresh();
			} else {
				move(y_start,16);
				clrtobot();
				mvaddstr(y_start,16,"LAT stopped.");
				refresh();
			}
                        break;

                case NO:
			move(10,16);
                	addstr("Would you like to stop the LAT Protocol?  NO");
			clrtobot();
			refresh();
			attrset(A_BOLD);
                        move(15, 19);
                        addstr("You have chosen not to stop LAT.");
			attrset(0);
                        break;

                }  /* end of switch(get_yes_no) */
	}
	refresh();

}  /* end of function start_stop() */


start_lat()
{
	int y_start = 11;
	int y;
	char lat_start[S_S_BUF_SIZE];    /* lat_start buffer area */
	FILE *fp;

	y = y_start;
	move(y,10);
	clrtobot();
	refresh();
	lat_start[0] = '\0';
	if ((fp = popen("/sbin/init.d/lat start", "r")) == NULL) {
		printw("Lat start: Could not start LAT");
		return;
	}
	attrset(A_BOLD);
	mvaddstr(y,16,"Executing ""/sbin/init.d/lat start"".  This may");
	y++;
	mvaddstr(y,16,"cause output to be printed to this screen.");
	y++;
	mvaddstr(y,16,"Attempting to start the driver.  Please wait...");
	y++;
	refresh();
	sleep(5);
	move(7,X_LOC);
	clrtobot();
	showfile(fp,15,8,0);
	attrset(0);
	pclose(fp);

} /* end of function start_lat */


stop_lat()
{
	int y_start = 14;
	char lat_start[S_S_BUF_SIZE];    /* lat_start buffer area */
	FILE *fp;

	move(15,10);
	refresh();
	lat_start[0] = '\0';
	if ((fp = popen("/sbin/init.d/lat stop", "r")) == NULL) {
		printw("Lat stop: Could not stop LAT");
		return;
	}	
	attrset(A_BOLD);
	mvaddstr(y_start,16,"Executing ""/sbin/init.d/lat stop"".  This may");
	y_start++;
	mvaddstr(y_start,16,"cause output to be printed to this screen.");
	y_start++;
	mvaddstr(y_start,16,"Attempting to halt the driver.  Please wait...");
	y_start++;
	refresh();
	while ( fgets(lat_start,S_S_BUF_SIZE,fp) != NULL ) {
		if ((strncmp("Attempt",lat_start,7) != 0) &&
		    (strncmp("\r",lat_start,1) != 0)) { 
			mvaddstr(y_start,16,lat_start);
		}
	}  /* end of fgets while  */
	attrset(0);
	pclose(fp);
} /* end of function stop_lat() */


get_yes_no(y_loc)
int y_loc;
{
        /* Determine if user responds yes or no to question */
        int c;
        int option, not_done = 1, YES_LOC = YES, NO_LOC = NO, x, y;

        move (y_loc, YES_LOC);
        addstr("YES");
        move(y_loc, NO_LOC);
        addstr("NO");

        move (y_loc, YES_LOC);
        getyx(stdscr, y, x);
        option = YES;

        while ( not_done ) {
                move(y,x);
                refresh();
                c = getch();
                if ( (c == KEY_RIGHT) || (c == KEY_LEFT) )
                        x = option = (option == YES) ? NO : YES;
                else
                     if (IS_RETURN(c))  /* CARRIAGE RETURN */
                         not_done = 0;

        }  /* end of while(not_done) */

        refresh();
        return(option);
}   /* end of function get_yes_no()  */


/* this routine is called to create additional devices for either the main */
/* menu or for the INITIAL_LAT_SETUP code */
int
create_devices(caller)
char caller;
{
int i, ret;
int left_to_make, inittab_total;
int create_ttys, gettys_added, do_initq;
int lockfd, proceed_y;
int lockerr = 0;

	/* 
	 * Open the /etc/inittab file and lock it for exclusive use.  This
	 * file descriptor is used only for locking and unlocking the
	 * file.  If we can't open it or lock it, warn the user and make
	 * them acknowledge this condition.
	 */

	if ((lockfd = open(INITTAB_FILE,O_RDWR)) < 0)
		lockerr++;
	if (!lockerr)
		if (flock(lockfd,LOCK_EX|LOCK_NB) < 0)
			lockerr++;
	if (lockerr) {
		attrset(A_BOLD|A_BLINK);
		y = OUTPUT_Y;
		move(y, X_LOC);
		clrtobot();
		mvaddstr(y++, X_LOC+2, "Warning!  The /etc/inittab file could not be locked for");
		mvaddstr(y++, X_LOC+2, "exclusive use.  Another process may be modifying it.");  
		proceed_y = y+=2;
		mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?");
		y+=3;
		switch (get_yes_no(y)) {
		case YES:
			mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?  YES");
			attrset(0);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return(FAILURE);
		case NO:
			mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?  NO");
			refresh();
			attrset(0);
			break;
		}
	}
		
	/*
	 * Call init_tty_info() now to make sure we have the latest
	 * and greatest information.  If the call returns FAILURE status,
	 * we have no choice but to exit, as the lat_ttys[] array and the
	 * global counts are no longer valid.
	 */
	if (init_tty_info(INIT_ALL,OUT_STDSCR) == FAILURE) {
		attrset(A_BOLD|A_BLINK);
		y = OUTPUT_Y;
		move(y, X_LOC);
		clrtobot();
		mvaddstr(y++, X_LOC+2, " Fatal Error: cannot initialize - insufficient memory\n");
		refresh();
		attrset(0);
		clean_n_exit(8);
	}

	if (caller == MAIN_MENU_CALLER) {  /* we were called by MAIN_MENU */
		clear(); 
		attrset(A_BOLD);
		y = HDR_Y;
		mvaddstr(y++, X_LOC, 
		   "This menu is used to create additional LAT devices. ");
		attrset(0);
		y++;	    /* let's skip a line before we start with data */
	} else {   			/* we were called by someone else */
		y = OUTPUT_Y;
		move(y, X_LOC); /* only incr y if we ouput data or skip line */
		clrtobot();
	}
	refresh();

	left_to_make = (MAX_DEVS - lat_dev_total - non_lat_dev_total);  

	/* tell the user what we found out about their devices and gettys */
	move(y++, X_LOC);
	switch(lat_dev_total) {
		case 0:
			addstr("No LAT tty devices have been created.");
			break;
		case 1:
			printw("There is %d LAT tty device created.", 
				lat_dev_total);
			break;
		default:
                        printw("There are %d LAT tty devices created.",
                                lat_dev_total);
                        if ((lat_dev_total + non_lat_dev_total) >= MAX_DEVS) {
                                mvaddstr(y++,X_LOC,
                           "You cannot create any additional LAT tty devices.");
                                return;
                        }
                        break;

	}
	move(y++, X_LOC);
	printw("You can create %d more LAT tty devices.", left_to_make);
	move(y++, X_LOC);
	switch(lat_entries_total) {
		case 0:
			printw("No LAT entries have been defined in %s.", 
			 	INITTAB_FILE);
			break;
		case 1:
			printw("There is %d LAT entry already defined in %s.", 
				lat_entries_total, INITTAB_FILE);
			break;
		default:
			printw("There are %d LAT entries already defined in %s.", 
				lat_entries_total, INITTAB_FILE);
			break;
	}

	/* this routine will continue to prompt the user for */
	/* input until they are happy with what they have entered.  The */
	/* values input by the user are passed back to this routine. */

	y++;		/* let's skip a line before we ask for input */

	get_user_input(left_to_make, &create_ttys, &gettys_added, 
			&do_initq);
	if (create_ttys == 0) {  /* if they want none, then exit.. */
		attrset(A_BOLD);
		y++;	/* let's skip a line before we issue this message. */
		mvaddstr(y++,X_LOC,
			"You have chosen not to create any LAT tty devices.");
		attrset(0);
		refresh();
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return(SUCCESS);
	}

	y++;	/* let's skip a line before we tell them we are working on it */
	mvaddstr(y++,X_LOC,"Working on your request");
	refresh();

	ret = make_devs_and_gettys(create_ttys, gettys_added, do_initq, 
		OUT_STDSCR);
	if (ret != SUCCESS) {
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return(FAILURE);
	}

	y = OUTPUT_Y;	/* let's go back to OUTPUT_Y to tell them we are done */
	move(y++, X_LOC);
	clrtobot();
	attrset(A_BOLD);
	addstr("LAT tty devices have been created");
	if (gettys_added) {
		if (do_initq == YES) {
			addstr(", getty entries defined,");
			mvaddstr(y++, X_LOC, "	and init q performed.");
		} else {
			addstr(" and getty entries defined.");
		}
	} else {
		addch('.');
	}
	attrset(0);
	switch(lat_dev_total) {
		case 1:
			mvaddstr(y++, X_LOC, "There is ");
			break;
		default:
			mvaddstr(y++, X_LOC, "There are ");
			break;
	}
	attrset(A_BOLD);
	addstr("now ");
	attrset(0);
	printw("%d LAT tty devices created.", lat_dev_total);
	switch(lat_entries_total) {
		case 1:
			mvaddstr(y++, X_LOC, "There is ");
			break;
		default:
			mvaddstr(y++, X_LOC, "There are ");
			break;
	}
	attrset(A_BOLD);
	addstr("now ");
	attrset(0);
	printw("%d LAT entries defined in %s.", 
		(lat_entries_total), INITTAB_FILE);
	refresh();

	flock(lockfd,LOCK_UN);
	close(lockfd);
	return(SUCCESS);

}  /* end of routine create_devices() */


/*
 * This routine is called to read in an integer value from stdscr.
 */
int
get_int_value(def_value)
int def_value;
{
int  value;

	cbreak();
	echo();
	keypad(stdscr,FALSE); 

	value = def_value;
	scanw("%d", &value);

	noecho();
	keypad(stdscr,TRUE);
	return(value);

}  /* end of routine get_int_value() */


/*
 * This routine is used to remove a file that has been created.
 */
int
rm_file(filename, output_flag)
char *filename;
int  output_flag;
{
int   status;

	/* Remove the file that was passed to this routine. */

	if ((status = unlink(filename)) < 0) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr, 
			"Problem removing %s'\n",filename);
		} else {
			move(y++,X_LOC);
			printw("Problem removing %s'",filename);
		}
		return(FAILURE);
	}

	return(SUCCESS);

} /* end of routine rm_file() */


/*
 * This routine is used to solicit the number of LAT tty devices to be created
 * from the user.
 */
int
get_tty_count(left_to_make)
int left_to_make;
{
int not_done, create_ttys;
char curr_y;
int c;

	not_done = 1;
	curr_y = y;
	while (not_done) {
		move(y++,X_LOC); 
		clrtobot();
		printw("How many LAT ttys do you want to create? (0 - %d) [0]  ",
			left_to_make);
		refresh();
		create_ttys = get_int_value(0); /* pass in default value */
		if (create_ttys < 0 || create_ttys > left_to_make) {
			move(y++,X_LOC);
			printw("You have provided an incorrect value, %d.", 
				create_ttys);
			y++;			        /* skip a line */
			move(y++,X_LOC);
			printw("Enter <RETURN> to reenter the number of ttys to be created."); 
			refresh();
			while ((c = getch()) && !IS_RETURN(c)) ;
			y = curr_y;
		} else {
			not_done = 0;
		}
	}
	
	return(create_ttys);

} /* end of routine get_tty_count() */

/*
 * This routine is used to solicit from the user the number of getty entries
 * to be added to the INITTAB_FILE.
 */
int
get_getty_count(create_ttys)
int create_ttys;
{
int not_done, gettys_added;
char curr_y, get_x, get_y;
int c;

	not_done = 1;
	curr_y = y;
	while (not_done) {
		move(y++,X_LOC);
		clrtobot();
		addstr("How many gettys would you like added to the ");
		move(y++,X_LOC);
		printw("	%s file? (0 - %d) [0]  ", 
			INITTAB_FILE, create_ttys);

		/* save where to get input so we can go back after printing */
		/* out this warning message. */
		getyx(stdscr, get_y, get_x);
		mvaddstr(y++,X_LOC,
			"NOTE: Please keep your system's memory resources");
		mvaddstr(y++,X_LOC,
			"      in mind when specifying gettys to be added.");
		move(get_y, get_x);   /* go back so we can get input */
		refresh();

		gettys_added = get_int_value(0); /* pass in default value */
		if (gettys_added < 0 || gettys_added > create_ttys) {
			move(y++,X_LOC);
			printw("You have provided an incorrect value, %d.", 
				gettys_added);
			y++;				/* skip a line */
			move(y++,X_LOC);
			printw("Enter <RETURN> to reenter the number of entries to be defined."); 
			refresh();
			while ((c = getch()) && !IS_RETURN(c)) ;
			y = curr_y;
		} else {
			not_done = 0;
		}
	}
	
	return(gettys_added);

} /* end of routine get_getty_count() */

/*
 * This routine is used to solicit whether the user wishes to execute 
 * init q to spawn the new entries that have been added to the INITTAB_FILE.
 */
int
get_initq_req()
{
int do_initq;
char save_y;

	mvaddstr(y++,X_LOC, "Would you like ");
	attrset(A_BOLD);
	addstr("init q ");
	attrset(0);
	addstr("to be executed to spawn the ");
	save_y = y;	/* save this y so that we can go back to add answer */
	move(y++,X_LOC);
	printw("	new getty entries in the %s file?  ", INITTAB_FILE);
	refresh();

	/* find out what the user wants */
	do_initq = get_yes_no(y);

	/* now put their answer next to the question and clear the rest of */
	/* the screen so that we can ask our next question. */
	move(save_y,X_LOC);
	printw("	new getty entries in the %s file?  ", INITTAB_FILE);
	if (do_initq == YES) {
		addstr("YES");
	} else {
		addstr("NO");
	}
	clrtobot();
	refresh();
	
	return(do_initq);

} /* end of routine get_initq_req() */

/*
 * This routine is used to solicit user input to the questions of how many
 * tty devices to make, how many getty entries to add, and whether init q 
 * should be executed.  This code will continue to prompt the user until the
 * user is happy with their input.
 */
int
get_user_input(left_to_make, create_ttys, gettys_added, do_initq)
int left_to_make;
int *create_ttys, *gettys_added, *do_initq;
{
int input_no_good, satisfied;
char curr_y, save_y;
int c;

	/* perform necessary initialization */
	*create_ttys = *gettys_added = 0;
	*do_initq = NO;

	input_no_good = 1;
	curr_y = y;
	while (input_no_good) {

		/* clear screen in case this is not the first time we are */
		/* entering our input. */
		move(y, X_LOC);
		clrtobot();

		/* find out how many LAT tty devices the user wants to make. */
		*create_ttys = get_tty_count(left_to_make);
		if (*create_ttys == 0) {  /* if they want none, then exit.. */
			return;
		} else {
			/* find out how many getty entries the user wants */
			/* added to the INITTAB_FILE. */
			*gettys_added = get_getty_count(*create_ttys);
			if (*gettys_added > 0) {
				*do_initq = get_initq_req();
			} else {
				*do_initq = NO;
			}
		}

		y++;		/* let's skip a line */
		move(y++, X_LOC);
		printw("You requested: %d tty", *create_ttys);
		if (*create_ttys > 1)  {
			addch('s');   /* make tty plural if more than one tty */
		}
		if (*gettys_added) {
			if (*do_initq == YES) {
				printw(", %d getty",
					*gettys_added);
				if (*gettys_added == 1) {
					addstr(", ");
				} else {
					addstr("s, ");  /* make it plural */
				}
				addstr("and that ");
				attrset(A_BOLD);
				addstr("init q ");
				attrset(0);
				addstr("be performed.");
			} else {
				printw(" and %d getty", 
					*gettys_added);
				if (*gettys_added == 1) {
					addch('.');
				} else {
					addstr("s.");  /* make it plural */
				}
			}
		} else {
			addch('.');
		}
		refresh();

		/* Ask the user if they are happy with their input?? */
		y++;		/* skip a line before we ask this question */
		save_y = y;
		mvaddstr(y++,X_LOC, "Are you satisfied with this input?  ");
		satisfied = get_yes_no(y);

		/* now put their answer next to the question and clear the */
		/* rest of the screen so that we can go back again. */
		move(save_y,X_LOC);
		clrtobot();
		addstr("Are you satisfied with this input?  ");
		if (satisfied == YES) {
			addstr("YES");
			input_no_good = 0;
		} else {
			addstr("NO");
			y++;			/* skip a line */
			move(y++,X_LOC);
			printw("Enter <RETURN> to reenter your input."); 
			refresh();
			while ((c = getch()) && !IS_RETURN(c)) ;
			y = curr_y;
		}

		refresh();
	} /* while (input_no_good) */

} /* end of routine get_user_input() */

/*
 * This routine is used to create the tty devices, add getty entries to
 * to the INITTAB_FILE, and perform the init q, if it was requested.
 */
int
make_devs_and_gettys(create_ttys, gettys_added, do_initq, output_flag)
int create_ttys, gettys_added, do_initq, output_flag;
{
int ret, i, j, dots;
int ttys_to_be_created, gettys_to_be_added;
char curr_dir[30];
char new_tty[TTYNAMELEN];
struct latioctl_port liop;
int valid_tty;
	
	/* initialize our variables so we can count the number of ttys left */
	/* to create and the number of gettys left to define. */

	ttys_to_be_created = create_ttys;
	gettys_to_be_added = gettys_added;

	/* let's save what our current directory is, so we can go back to */
	/* it later, when we are done. */
	ret = (int) getwd(curr_dir);
	if (ret == 0) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr,
			   "Problem getting current directory; %s \n", curr_dir);
		} else {
			move(y++,X_LOC);
			printw("Problem getting current directory; %s ", 
				curr_dir);
			refresh();
		}
		return(FAILURE);
	} 
	
	/* let's 'cd' to the DEV_DIRECTORY */
	ret = chdir(DEV_DIRECTORY);
	if (ret == -1) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr,
			   "Problem setting current directory to %s \n", 
				DEV_DIRECTORY);
		} else {
			move(y++,X_LOC);
			printw("Problem setting current directory to %s ", 
				DEV_DIRECTORY);
			refresh();
		}
		return(FAILURE);
	} 

	/*
	 * Save a copy of the INITTAB_FILE if we are going to modify it
	 */
	if (gettys_to_be_added) 
		ret = copy_file(INITTAB_FILE,INITTAB_SAVE,output_flag);

	dots = ttys_to_be_created / 20;
	if (dots == 0)
		dots = 1;

	for (j=0; j<ttys_to_be_created; j++) {

		if ((j % dots) == 0) {
			/* so user sees # of '.' increasing */
			if (output_flag == OUT_STDERR) {  /* write to stderr */
				fprintf(stderr,".");
			} else {
				addch('.');	
				refresh();
			}
		}

		/*
		 * Create a tty.  After successfully creating it, check to 
		 * see if it has an application port mapping associated with
		 * it.  This check guards against the case where a tty (which
		 * was being used as an application port) was deleted and
		 * latsetup recreates it and uses it for a getty.
	 	 */

		new_tty[0] = '\0';	/* no valid name yet */
		valid_tty = FALSE;

		while (!valid_tty) {
			if (make_lat_tty(new_tty, output_flag) != SUCCESS)
				return(FAILURE);
			valid_tty = TRUE;
			bzero(&liop,sizeof(liop));
			liop.lp_devno = lat_ttys[tty_to_int(new_tty)].tdev;
			if ((ioctl(latfd, LIOCGPORT, &liop) >= 0) &&
		    	    (liop.lp_flags & LATIOCTL_PORT_OUTBOUND)) {
				valid_tty = FALSE;
				lat_ttys[tty_to_int(new_tty)].flags |= ISMAPPED;
			}
		}

		if (gettys_to_be_added) {
			if (add_getty(new_tty, output_flag) != SUCCESS)
				return(FAILURE);
			gettys_to_be_added--;
		}
	} /* for j */

	/* Sync the inittab file */ 
	if (gettys_added)
		if (sync_inittab(output_flag) != SUCCESS)
			return(FAILURE);

	if (do_initq == YES || do_initq == NO_CUR_YES) {
		ret = perform_initq(output_flag);
		if (ret != SUCCESS) {
			return(ret);
		}
	}	

	/* let's 'cd' back to our original directory */
	if ((ret = chdir(curr_dir)) == -1) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr,
			   "Problem setting current directory to %s \n", 
				curr_dir);
		} else {
			move(y++,X_LOC);
			printw("Problem setting current directory to %s ", 
				curr_dir);
			refresh();
		}
		return(FAILURE);
	} 

	return(SUCCESS);
	
} /* end of routine make_devs_and_gettys() */


/*
 * This routine creates a new /etc/inittab file by using the temporary
 * inittab file and adding all of the entries that are described 
 * in lat_ttys[] at the time of its invocation.  This routine assumes that
 * the information in lat_ttys[].iline is correct and sufficient to create an 
 * entry in /etc/inittab.
 */
int
sync_inittab(output_flag)
int output_flag;
{
	FILE *itbtmp;
	int i=0;

	/*
	 * Strategy for sync'ing the /etc/inittab file is to copy the 
	 * template file that was created during initialization (in
	 * get_inittab_info()) to a working copy.  Entries are added as
	 * per lat_ttys[] to the working copy and then the working copy
	 * is renamed as /etc/inittab.
	 */
	if (copy_file(INITTAB_TEMPLATE,INITTAB_TMP_FILE,output_flag) == FAILURE)
		return(FAILURE);

	if ((itbtmp = fopen(INITTAB_TMP_FILE, "a")) == NULL) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr, 
			   "Fatal Error: can't open %s\n", INITTAB_TMP_FILE);
		} else {
                       	attrset(A_BOLD);
			move(y++, X_LOC);
			clrtobot();
			printw("Fatal Error: can't open %s", INITTAB_TMP_FILE);
                        attrset(0);
			refresh();
		}
		return(FAILURE);
	}

	for (i=0; i<MAX_DEVS; i++) {
		if (lat_ttys[i].flags & ININITTAB) {
			fprintf(itbtmp,"%s:%s:%s:%s\n",lat_ttys[i].iline.c_id,lat_ttys[i].iline.c_levels,lat_ttys[i].iline.c_action,lat_ttys[i].iline.c_command);
		}
	}
	
	fclose(itbtmp);			/* write out temp file */

        if (rename(INITTAB_TMP_FILE,INITTAB_FILE) < 0) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr, 
			"Fatal Error: can't rename %s to %s error %d\n", INITTAB_TMP_FILE, INITTAB_FILE, errno);
		} else {
                       	attrset(A_BOLD);
			move(y++, X_LOC);
			clrtobot();
			printw("Fatal Error: can't rename %s to %s error %d\n", INITTAB_TMP_FILE, INITTAB_FILE, errno);
			refresh();
                       	attrset(0);
		}
		return(FAILURE);
        }
	
	return(SUCCESS);
}


/*
 * This routine is used to create a new LAT tty device.
 */
int
make_lat_tty(new_tty, output_flag)
char *new_tty;
int output_flag;
{
	static int try_new_index=0;
	static minor_t try_new_minor=0;
	int i, j, ret, usable_index, usable_minor;
	mode_t lmode = S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH; 

	/* find the next available name */
	for (i=try_new_index; i<MAX_DEVS; i++) {
		if (lat_ttys[i].name[0] == '\0') {
			usable_index = i;
			break;
		}
	}
	try_new_index = usable_index + 1;	/* save it for next time */

	/* find the next available minor number */
	for (j=try_new_minor; j<MAX_DEVS; j++) {
		if (!lat_minors[j]) {
			usable_minor = j;
			break;
		}
	}
	try_new_minor = usable_minor + 1;	/* save it for next time */

	/* create new LAT tty */

	/* Check to see if tty name is in range */
	if ( usable_index > 3843 ) { /* max # of tty[0-9a-zA-Z][0-9a-zA-Z] */
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"LAT tty name out of range\n");
		} else {
			move(y++,X_LOC);
			printw("LAT tty name out of range\n");
			refresh();
		}
		new_tty[0] = '\0';      /* signal that the create failed */
		return(FAILURE);
	}
	else
		int_to_tty(usable_index,new_tty);
	
	if ((ret = mknod(new_tty,lmode,makedev(LAT_MAJOR,usable_minor))) < 0) {
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"Error creating new LAT tty, error = %x\n",errno);
		} else {
			move(y++,X_LOC);
			printw("Error creating new LAT tty, error = %x\n",errno);
			refresh();
		}
		new_tty[0] = '\0';	/* signal that the create failed */
		return(FAILURE);
        }

        /* set appropriate permissions on new tty */

        if ((ret = chmod(new_tty,lmode)) != 0) {
		if (output_flag == OUT_STDERR) { 
                	fprintf(stderr, "unable to change permissions of %s\n", new_tty);
		} else {
			move(y++,X_LOC);
			printw("unable to change permissions of %s\n",new_tty);
			refresh();
		}
		rm_file(new_tty,output_flag);
		new_tty[0] = '\0';	/* signal that the create failed */
		return(FAILURE);
        }

	change_tty_info(new_tty,CREATE_TTY_DEVICE);
	return(SUCCESS);

} /* end of make_lat_tty() */


/*
 * This routine is called to initialize LAT getty entry information in
 * the lat_ttys[] structure.  This information is later used to create a
 * new /etc/inittab file.
 */
int
add_getty(new_tty, output_flag)
char *new_tty;
int  output_flag;
{
	char cmdbuf[80];
	int  ttyidx;

	ttyidx = tty_to_int(new_tty);

	strcpy(lat_ttys[ttyidx].iline.c_id, INITTAB_ID);
	strncat(lat_ttys[ttyidx].iline.c_id, &new_tty[8], 3);
	strcpy(lat_ttys[ttyidx].iline.c_levels, "3");
	strcpy(lat_ttys[ttyidx].iline.c_action, "respawn");
	sprintf(cmdbuf,COMMAND,new_tty);

	if ((lat_ttys[ttyidx].iline.c_command = (char *)malloc(strlen(cmdbuf) +1)) == NULL)
		return(FAILURE);
	strcpy(lat_ttys[ttyidx].iline.c_command,cmdbuf);

	change_tty_info(new_tty,ADD_TO_INITTAB);

	return(SUCCESS);

} /* end of routine add_getty() */

/*
 * This routine is called to perform the init q command.
 */
int
perform_initq(output_flag)
int output_flag;
{
pid_t child;
int   status;

	if ((child = fork()) == 0) {

		/* perform the init q */
		execl(INIT_PROG, "init", "q", (char *)0);   
		exit(1);
	}
	waitpid(child, &status, 0);
	if (status) {
		if (output_flag == OUT_STDERR) {  /* write to stderr */
			fprintf(stderr, 
			"Problem executing init q; status = %o\n", status);
		} else {
			move(y++,X_LOC);
			printw("Problem executing init q; status = %o", status);
			refresh();
		}
		return(FAILURE);
	}

	return(SUCCESS);

} /* end of routine perform_initq() */



/* this routine is called to create additional devices for either the main */
/* menu or for the INITIAL_LAT_SETUP code */
int
non_curses_create_devs()
{
int i, ret, lockfd;
int dev_total, left_to_make, inittab_total;
int create_ttys, gettys_added, do_initq;
int lockerr = 0;


	/* 
	 * Open the /etc/inittab file and lock it for exclusive use.  This
	 * file descriptor is used only for locking and unlocking the
	 * file.  If we can't open it or lock it, warn the user and make
	 * them acknowledge this condition.
	 */

	if ((lockfd = open(INITTAB_FILE,O_RDWR)) < 0)
		lockerr++;
	if (!lockerr)
		if (flock(lockfd,LOCK_EX|LOCK_NB) < 0)
			lockerr++;
	if (lockerr) {
		fprintf(stdout,"\nWarning!  The /etc/inittab file could not be locked for\n");
		fprintf(stdout,"exclusive use.  Another process may be modifying it.\n");  
		switch (yesno(NO_CUR_YES,"\nDo you wish to cancel this operation?")) {

		case NO_CUR_YES:
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return(FAILURE);
		case NO_CUR_NO:
			break;
		}
	}

	/*
	 * Call init_tty_info() now to make sure we have the latest
	 * and greatest information.  If the call returns FAILURE status,
	 * we have no choice but to exit, as the lat_ttys[] array and the
	 * global counts are no longer valid.
	 */
	if (init_tty_info(INIT_ALL,OUT_STDERR) == FAILURE) {
		fprintf(stderr," Fatal Error: cannot initialize - insufficient memory\n");
		clean_n_exit(8);
	}

	left_to_make = (MAX_DEVS - lat_dev_total - non_lat_dev_total);

	/* tell the user what we found out about their devices and gettys */
	switch(lat_dev_total) {
		case 0:
			fprintf(stdout,
				"\nNo LAT tty devices have been created.\n");
			break;
		case 1:
			fprintf(stdout,
			  "\nThere is %d LAT tty device created.\n", lat_dev_total);
			break;
		default:
                        fprintf(stdout,
                         "\nThere are %d LAT tty devices created.\n",lat_dev_total);
                        if(lat_dev_total >= MAX_DEVS) {
                                fprintf(stdout,
                         "You cannot create any additional LAT tty devices.\n");
                                return;
                        }
                        break;
	}
	fprintf(stdout,"You can create %d more LAT tty devices.\n", 
		left_to_make);
	switch(lat_entries_total) {
		case 0:
			fprintf(stdout,
			   "No LAT entries have been defined in %s.\n", 
			 	INITTAB_FILE);
			break;
		case 1:
			fprintf(stdout,
			   "There is %d LAT entry already defined in %s.\n", 
				lat_entries_total, INITTAB_FILE);
			break;
		default:
			fprintf(stdout,
			 "There are %d LAT entries already defined in %s.\n", 
				lat_entries_total, INITTAB_FILE);
			break;
	}

	/* this routine will continue to prompt the user for */
	/* input until they are happy with what they have entered.  The */
	/* values input by the user are passed back to this routine. */
	non_curses_get_user_input(left_to_make, &create_ttys, &gettys_added, 
			&do_initq);
	if (create_ttys == 0) {  /* if they want none, then exit.. */
		fprintf(stdout,
		   "\nYou have chosen not to create any LAT tty devices.\n");
		return;
	}

	fprintf(stdout,"\nWorking on your request");

	ret = make_devs_and_gettys(create_ttys, gettys_added, do_initq, 
		OUT_STDERR);
	fprintf(stdout,"\n");
	if (ret != SUCCESS) {
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return(FAILURE);
	}

	fprintf(stdout,"\nLAT tty devices have been created");
	if (gettys_added) {
		if (do_initq == NO_CUR_YES) {
			fprintf(stdout,", getty entries defined,");
			fprintf(stdout," and init q performed.\n");
		} else {
			fprintf(stdout,
				" and getty entries have been defined.\n");
		}
	} else {
		fprintf(stdout,".\n");
	}
	switch(lat_dev_total) {
		case 1:
			fprintf(stdout, "There is ");
			break;
		default:
			fprintf(stdout, "There are ");
			break;
	}
	fprintf(stdout,
		"now %d LAT tty devices created.\n", lat_dev_total);
	switch(lat_entries_total) {
		case 1:
			fprintf(stdout, "There is ");
			break;
		default:
			fprintf(stdout, "There are ");
			break;
	}
	fprintf(stdout,"now %d LAT entries defined in %s.\n", 
		lat_entries_total, INITTAB_FILE);
			
	flock(lockfd,LOCK_UN);
	close(lockfd);
	return(SUCCESS);

}  /* end of routine non_curses_create_devs() */

/*
 * This routine is called to read in an integer value from stdin.
 */
int
non_curses_get_int_value(def_value)
int def_value;
{
char buf[512];

        buf[0] = '\0';
        if (gets(buf) == NULL) {
                return(def_value);
        }
        if (buf[0] == '\0')
                return(def_value);

        return(atoi(buf));

}  /* end of routine non_curses_get_int_value() */

/*
/*
 * This routine is used to solicit the number of LAT tty devices to be created
 * from the user.
 */
int
non_curses_get_tty_count(left_to_make)
int left_to_make;
{
int not_done, create_ttys, not_done_2;
char buf[512];

	not_done = 1;
	while (not_done) {
		fprintf(stdout,
		   "\nHow many LAT ttys do you want to create? (0 - %d) [0]  ",
			left_to_make);
		create_ttys = non_curses_get_int_value(0);
		if (create_ttys < 0 || create_ttys > left_to_make) {
			fprintf(stdout,
			  "You have provided an incorrect value, %d.\n", 
				create_ttys);
			fprintf(stdout,
"\nEnter <c> to continue and reenter the number of ttys to be created\n");
			fprintf(stdout, "or <q> to quit.  "); 
			not_done_2 = 1;
			while (not_done_2) { 
                		fflush(stdout);
                		buf[0] = '\0';
        			if (gets(buf) == NULL) {
					continue;
        			}
				if (buf[0] == 'q' || buf[0] == 'Q') {
					not_done_2 = 0;
					create_ttys = 0;
					return(create_ttys);
				}
                		if (buf[0] == 'c' || buf[0] == 'C') {
					not_done_2 = 0;
				} 
        		}  /* while not_done_2 */
		} else {
			not_done = 0;
		} /* if create_ttys is invalid */
	} /* while not done */
	
	return(create_ttys);

} /* end of routine non_curses_get_tty_count() */

/*
 * This routine is used to solicit from the user the number of getty entries
 * to be added to the INITTAB_FILE.
 */
int
non_curses_get_getty_count(create_ttys)
int create_ttys;
{
int not_done, gettys_added, not_done_2;
char buf[512];

	not_done = 1;
	while (not_done) {
		fprintf(stdout,
			"\nNOTE: Please keep your system's memory resources");
		fprintf(stdout,
			" in mind when\nspecifying gettys to be added.\n");
		fprintf(stdout,"How many gettys would you like added to ");
		fprintf(stdout,"%s ? (0 - %d) [0] ", INITTAB_FILE, create_ttys);

		gettys_added = non_curses_get_int_value(0);
		if (gettys_added < 0 || gettys_added > create_ttys) {
			fprintf(stdout,
			  "You have provided an incorrect value, %d.\n", 
				gettys_added);
			fprintf(stdout,
"\nEnter <c> to continue and reenter the number of gettys to be defined.  "); 
			not_done_2 = 1;
			while (not_done_2) { 
                		fflush(stdout);
                		buf[0] = '\0';
        			if (gets(buf) == NULL) {
					continue;
        			}
                		if (buf[0] == 'c' || buf[0] == 'C') {
					not_done_2 = 0;
				} 
        		}  /* while not_done_2 */
		} else {
			not_done = 0;
		} /* if gettys_added is good */
	} /* while not_done */
	
	return(gettys_added);

} /* end of routine non_curses_get_getty_count() */

/*
 * This routine is used to solicit whether the user wishes to execute 
 * init q to spawn the new entries that have been added to the INITTAB_FILE.
 */
int
non_curses_get_initq_req()
{
int do_initq;
char save_y;


	fprintf(stdout,"\nWould you like init q to be executed to spawn the "); 
	fprintf(stdout,"new getty entries\nin the %s file?  ", INITTAB_FILE);

	/* find out what the user wants */
	do_initq = yesno(NO_CUR_YES,"");

	return(do_initq);

} /* end of routine non_curses_get_initq_req() */

/*
 * This routine is used to solicit user input to the questions of how many
 * tty devices to make, how many getty entries to add, and whether init q 
 * should be executed.  This code will continue to prompt the user until the
 * user is happy with their input.
 */
int
non_curses_get_user_input(left_to_make, create_ttys, gettys_added, do_initq)
int left_to_make;
int *create_ttys, *gettys_added, *do_initq;
{
int input_no_good, satisfied, not_done;
char c, curr_y, save_y;
char buf[512];

	/* perform necessary initialization */
	*create_ttys = *gettys_added = 0;
	*do_initq = NO_CUR_NO;

	input_no_good = 1;
	curr_y = y;
	while (input_no_good) {

		/* find out how many LAT tty devices the user wants to make. */
		*create_ttys = non_curses_get_tty_count(left_to_make);
		if (*create_ttys == 0) {  /* if they want none, then exit.. */
			return;
		} else {
			/* find out how many getty entries the user wants */
			/* added to the INITTAB_FILE. */
			*gettys_added=non_curses_get_getty_count(*create_ttys);
			if (*gettys_added > 0) {
				*do_initq = non_curses_get_initq_req();
			} else {
				*do_initq = NO_CUR_NO;
			}
		}


		fprintf(stdout,
		   "\nYou requested the creation of %d tty", *create_ttys);
		if (*create_ttys > 1)  {
			/* make tty plural if more than one tty */
			fprintf(stdout,"s");  
		}
		if (*gettys_added) {
			if (*do_initq == NO_CUR_YES) {
				fprintf(stdout,", the definition of %d getty",
					*gettys_added);
				if (*gettys_added == 1) {
					fprintf(stdout,", \n");
				} else {
					/* make it plural */
					fprintf(stdout,"s, \n");  
				}
				fprintf(stdout,
				"and that init q be performed.\n");
			} else {
				fprintf(stdout,
				  " and the definition of %d getty", 
					*gettys_added);
				if (*gettys_added == 1) {
					fprintf(stdout, ".\n");
				} else {
					/* make it plural */
					fprintf(stdout,"s.\n");  
				}
			}
		} else {
			fprintf(stdout,".\n");
		}

		/* Ask the user if they are happy with their input?? */
		satisfied = yesno(NO_CUR_YES, 
			"\nAre you satisfied with this input?  ");

		if (satisfied == NO_CUR_YES) {
			input_no_good = 0;
		} /* if satisfied */

	} /* while (input_no_good) */

} /* end of routine non_curses_get_user_input() */

/* Non curses yesno routine */
yesno(t, s)
char *s;
int t;
{
        char buf[512];

        for (;;) {
                printf(s);
                switch(t) {
                case 0:
                        printf(" [no] ");
                        break;
                case 1:
                        printf(" [yes] ");
                        break;
                default:
                        printf(" (yes or no) ");
                        break;
                }
                fflush(stdout);
                buf[0] = '\0';
                if (gets(buf) == NULL) {
                        eof();
                        return(t);
                }
                if (buf[0] == 'y' || buf[0] == 'Y')
                        return(1);
                if (buf[0] == 'n' || buf[0] == 'N')
                        return(0);
                if ((t != 2) && (buf[0] == '\0' || buf[0] == '\n'))
                        return(t);
        }
        /* To supress the silly warning about statement not reached, */
        /* we comment out the following:  */

        /* return(0); */
}

/*
 *              e o f
 *
 * Unexpected EOF, try to recover.
 */
eof()
{
        if (isatty(0) && (freopen((const char *)ttyname(0),"r",stdin) != NULL))
{
                printf("\n");
                return(0);
        }
        fprintf(stderr,"\nUnexpected EOF.  Exiting.\n");
        clean_n_exit(1);
}


/* This routine begins the non curses mode of latsetup */
non_curses_enter_main_menu()
{
	int option;

	non_curses_intro();
	option = non_curses_get_main_option();
        switch ( option ) {

	case INITIAL_SETUP:
	     non_curses_setup();
	     break;
        case UNDO_SETUP:
	     non_curses_undo_lat_setup();
	     break;
	case EXIT:
	     break;

	}  /* end of switch(option) */
	printf("\n");

}  /* end of function non_curses_enter_main_menu() */


/* This routine is the introduction of the non curses screen */
non_curses_intro()
{
	printf("\n\t\tLAT Setup Utility (non-curses mode)\n\n");
	printf("The latsetup program provides assistance in administering LAT on\n");
	printf("your system.  You can choose to setup or undo LAT on your system\n");
	printf("\n");
}  /* end of function non_curses_intro()   */


/*
 * This routine prints out the options allowed for the non curses mode
 * of latsetup and allows the user to select one of the choices.  The
 * choice selected by the user is returned to the calling routine.
 */
int
non_curses_get_main_option()
{
	char buf[512];

	printf("\t1) Setup LAT\n");
	printf("\t2) Undo LAT\n");
	printf("\t3) Exit\n");

 	while (1) { 
		printf("\n\tSelect option (1, 2, or 3): ");
                fflush(stdout);
                buf[0] = '\0';
        	if (gets(buf) == NULL)
			continue;
		if (buf[0] == '1')
			return(INITIAL_SETUP);
		if (buf[0] == '2')
			return(UNDO_SETUP);
		if (buf[0] == '3')
 			return(EXIT);

         }  /* while not_done_2 */
	printf("\n\n");

}  /* end of function non_curses_get_main_option() */


/*
 * This routine will perform setup functions in non curses mode.
 */
non_curses_setup()
{
        int sys_ret;
        if (non_curses_setup_intro() == NO_CUR_NO)
                return;

        if (non_curses_create_devs() == FAILURE)
		return;

        if (non_curses_set_LAT_SETUP() < 0)  /*  set LAT_SETUP to 1 in rc.config */
		return;

	if (enter_start_lat() == YES)  {
        	sys_ret = system ( "/sbin/init.d/lat start");
        	if (non_curses_system_err_check(sys_ret) < 0 )
                	return;
	}
	return;

} /* end of function non_curses_setup() */


non_curses_setup_intro()
{
	printf(
	  "\nThis option will allow you to setup LAT on your system by\ncreating LAT ");
	printf(
	  "special device files, selecting how many getty entries\nshould be placed ");
	printf(
	  "into /etc/inittab, choosing whether or not to execute\ninit q, and starting ");
	printf(
	  "LAT to allow interactive connections to be\nmade to this system.  A ");
	printf(
	  "variable will also be set in /etc/rc.config\nso that LAT automatic startup ");
	printf(
	  "and shutdown is enabled which will\ncause LAT to be started each time ");
	printf("this system reaches run-level 3.\n");
	if(yesno(NO_CUR_YES,"\nWould you like to continue with this option?"))
		return(NO_CUR_YES);
	else
		return(NO_CUR_NO);

} /* end of function non_curses_setup_intro() */



/*
 * This routine will perform undo functions in non curses mode.
 */
non_curses_undo_lat_setup()
{
        int ret, sys_ret, entries_to_remove = 0;
	int lockfd, lockerr = 0;

        if (non_curses_undo_intro() == NO_CUR_NO)
                return;

	/* 
	 * Open the /etc/inittab file and lock it for exclusive use.  This
	 * file descriptor is used only for locking and unlocking the
	 * file.  If we can't open it or lock it, warn the user and make
	 * them acknowledge this condition.
	 */

	if ((lockfd = open(INITTAB_FILE,O_RDWR)) < 0)
		lockerr++;
	if (!lockerr)
		if (flock(lockfd,LOCK_EX|LOCK_NB) < 0)
			lockerr++;
	if (lockerr) {
		fprintf(stdout,"\nWarning!  The /etc/inittab file could not be locked for\n");
		fprintf(stdout,"exclusive use.  Another process may be modifying it.\n");  
		switch (yesno(NO_CUR_YES,"\nDo you wish to cancel this operation?")) {

		case NO_CUR_YES:
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return(FAILURE);
		case NO_CUR_NO:
			break;
		}
	}

	/*
	 * Call init_tty_info() now to make sure we have the latest
	 * and greatest information.  If the call returns FAILURE status,
	 * we have no choice but to exit, as the lat_ttys[] array and the
	 * global counts are no longer valid.
	 */
	if (init_tty_info(INIT_ALL,OUT_STDERR) == FAILURE) {
		fprintf(stderr," Fatal Error: cannot initialize - insufficient memory\n");
		clean_n_exit(8);
	}

	if ( lat_entries_total > 0 )
		/* Get the number of LAT entries to delete from /etc/inittab */
		entries_to_remove = non_curses_get_remove_input();

	printf("\nRemoving %d", entries_to_remove);
	if (entries_to_remove == 1)
		printf(" LAT entry ");
	else
		printf(" LAT entries ");
	printf("from %s, deleting %d", INITTAB_FILE, entries_to_remove);

	if (entries_to_remove == 1)
		printf(" LAT tty,\n");
	else
		printf(" LAT ttys,\n");
	printf("stopping LAT, and disabling LAT automatic startup and shutdown.\n");
	if (entries_to_remove > 0) {
		printf("Saving %s to %s.\n", INITTAB_FILE, INITTAB_SAVE);
		ret = copy_file(INITTAB_FILE,INITTAB_SAVE,OUT_STDERR);
	}
	printf("\nWorking.......\n");

	/*
	 * Stop LAT,
	 * disable automatic LAT startup/shutdown, remove entries
	 * from /etc/inittab and delete ttys.
	 */

	printf("Attempting to stop the driver.  Please wait...");

	bzero(&latioctl_node, sizeof(latioctl_node));
	latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
	latioctl_node.ln_state = LAT_STATE_OFF;
	if (ioctl(latfd, LIOCSNODE, &latioctl_node) < 0) {
		fprintf(stderr, "\nLIOCSNODE ioctl failed. errno=%d.\n", errno);
		printf("LAT NOT stopped.\n");
	} else {
		printf("LAT stopped.\n");
	}
        if (non_curses_reset_LAT_SETUP() < 0) {
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return(FAILURE); 
	}
	if ( entries_to_remove > 0 ) {
        	remove_inittab_entries(entries_to_remove, OUT_STDERR, TRUE);
		if (sync_inittab(OUT_STDERR) != SUCCESS)
			fprintf(stdout,"Error: could not rewrite /etc/inittab!\n");
		if (perform_initq(OUT_STDERR) != SUCCESS)
			fprintf(stdout,"Error: could not issue ""init q"" command\n");
	}

	flock(lockfd,LOCK_UN);
	close(lockfd);

}  /* end of function non_curses_undo_lat_setup() */


int
non_curses_undo_intro()
{
	printf("\nThis option will allow you to undo the LAT setup on your system\n");
	printf("by removing a specified number of LAT entries from /etc/inittab,\n");
	printf("deleting these LAT ttys, and stopping LAT.  A variable will also \n");
	printf("be reset in /etc/rc.config to disable LAT automatic startup and\n");
	printf("shutdown so that LAT will not start when entering run-level 3.\n\n");
	printf("NOTE: Active LAT sessions will be disconnected when LAT is stopped.\n\n");

	if(yesno(NO_CUR_YES,"Would you like to continue with this option?"))
		return(NO_CUR_YES);
	else
		return(NO_CUR_NO);
	printf("\n");

} /* end of function non_curses_undo_intro() */


/*
 * This function will prompt the user, in non-curses mode, for the number
 * of LAT entries to be removed from the /etc/inittab file and return this
 * number to the calling routine.
 */
int
non_curses_get_remove_input()
{
int not_done, remove_count;
char buf[512];

	not_done = 1;
	fprintf(stdout, "\n");
	while (not_done) {
		fprintf(stdout, "\n");
		fprintf(stdout,
		   "How many LAT entries do you want to remove? (0 - %d) [%d] ",
		    lat_entries_total, lat_entries_total);
		remove_count = non_curses_get_int_value(lat_entries_total);
		if (remove_count < 0 || remove_count > lat_entries_total) {
			fprintf(stdout,
			  "You have provided an incorrect value, %d.\n", 
				remove_count);
		} else {
			not_done = 0;
		}
	} /* while not done */
	
	return(remove_count);

}  /* end of function non_curses_get_remove_input() */


int
set_LAT_SETUP()
{
        int sys_ret;
	/* Set LAT_SETUP in rc.config for automatic startup/shutdown */
        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 1");
        if (system_err_check(sys_ret) < 0 )
                return(-1);
	return(0);

}   /*end of function set_LAT_SETUP() */

int
non_curses_set_LAT_SETUP()
{
        int sys_ret;
	/* Set LAT_SETUP in rc.config for automatic startup/shutdown */
        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 1");
        if (non_curses_system_err_check(sys_ret) < 0 )
                return(-1);
	return(0);

}   /*end of function non_curses_set_LAT_SETUP() */

int
reset_LAT_SETUP()
{
        int sys_ret;
	/* Reset LAT_SETUP in rc.config to disable automatic startup/shutdown */
        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 0");
        if (system_err_check(sys_ret) < 0 )
                return(-1);
	return(0);

}   /*end of function reset_LAT_SETUP() */

int
non_curses_reset_LAT_SETUP()
{
        int sys_ret;
	/* Reset LAT_SETUP in rc.config to disable automatic startup/shutdown */
        sys_ret = system("/usr/sbin/rcmgr set LAT_SETUP 0");
        if (non_curses_system_err_check(sys_ret) < 0 )
                return(-1);
	return(0);

}   /*end of function non_curses_reset_LAT_SETUP() */

/*
 * This routine checks the return status from a 'system' system call.
 */
int
system_err_check(sys_ret)
int sys_ret;
{
        if (sys_ret < 0)  {
                if (errno == EAGAIN)
                        mvaddstr(CR_Y-2,CR_X,"Lat: Number of processes exceeded.");
                else if (errno == EINTR)
                        mvaddstr(CR_Y-2,CR_X,"Lat: Interrupt signal.");
                else if (errno == ENOMEM)
                        mvaddstr(CR_Y-2,CR_X,"Lat: No space left.");
		refresh();
                return(-1);
        }
        if (sys_ret ==  SHELL_NOT_EXEC)  {
                mvaddstr(CR_Y-2,CR_X,"Lat: Could not execute start shell.");
		refresh();
                return(-1);
        }
	return(0);

}  /*end of function system_err_check() */

int
non_curses_system_err_check(sys_ret)
int sys_ret;
{
        if (sys_ret < 0)  {
                if (errno == EAGAIN)
                        fprintf(stderr,"Lat: Number of processes exceeded.");
                else if (errno == EINTR)
                        fprintf(stderr,"Lat: Interrupt signal.");
                else if (errno == ENOMEM)
                        fprintf(stderr,"Lat: No space left.");
                return(-1);
        }
        if (sys_ret ==  SHELL_NOT_EXEC)  {
                fprintf(stderr,"Lat: Could not execute start shell.");
                return(-1);
        }
	return(0);

}  /*end of function non_curse_system_err_check() */

/* this is the routine that is executed when signals are caught by latsetup. */
void
signalexit()
{
	clean_n_exit(3);
}


int
init_tty_info(initlevel,output_flag)
int initlevel;
int output_flag;
{
	register int i;
	char ttytemp[TTYNAMELEN];
	struct stat statb;
	struct latioctl_port liop;


	if (initlevel & (INIT_ALL)) {

		lat_dev_total = 0;
		non_lat_dev_total = 0;

		for (i=0; i<MAX_DEVS; i++) {

			int_to_tty(i,ttytemp);
			bzero(&lat_ttys[i],(sizeof(struct latttyinfo) - sizeof(i_line)));
			lat_minors[i] = FALSE;

			if ((stat(ttytemp,&statb) < 0))
				continue;	/* node doesn't exist */

			if ((!(S_ISCHR(statb.st_mode))) ||
			      (major(statb.st_rdev) != LAT_MAJOR)) {
				non_lat_dev_total++;
				strcpy(lat_ttys[i].name,ttytemp);
				lat_ttys[i].tdev = statb.st_rdev;
				continue;	/* not a character device or */
			}			/* major # != 5 */

			/* o/w it's a LAT tty... */
			lat_dev_total++;
			strcpy(lat_ttys[i].name,ttytemp);
			lat_ttys[i].tdev = statb.st_rdev;
			lat_minors[minor(statb.st_rdev)] = TRUE;

			bzero(&liop,sizeof(liop));
			liop.lp_devno = statb.st_rdev;	/* set up for ioctl */
			if ((ioctl(latfd, LIOCGPORT, &liop) >= 0) && 
			    (liop.lp_flags & LATIOCTL_PORT_OUTBOUND)) {
				lat_ttys[i].flags |= ISMAPPED; 	
							/* it's mapped to an
							   appl. port */
			}
		} /* for (i=...) */
	} /* INIT_ALL */

	if (initlevel & (INIT_ALL|COUNT_INITTAB_TOTAL))
		if (get_inittab_info(OUT_STDERR) == FAILURE)
			return(FAILURE);	/* Not enough memory!*/

	return(SUCCESS);

}  /* end of function init_tty_info() */


int_to_tty(integer, tty)
int integer;
char tty[];
{
	int i, tens, ones;

	strcpy(tty,"/dev/tty");
	tens = integer/62;
	ones = integer % 62;

	if ((ones >= 0) && (ones <= 9))		/* ttyAB where B is '0'-'9' */
		ones += '0';
	else if ((ones >= 10) && (ones <= 35))	/* ttyAB where B is 'a'-'z' */
		ones += 'W';
	else if ((ones >= 36) && (ones <= 61))	/* ttyAB where B is 'A'-'Z' */
		ones += 29;

	if ( tens < 10 )
		tens += '0';
	else if ( tens > 9 && tens < 36 ) {
		tens -= 10;
		tens += 'a';
	} else {
		tens -= 36;
		tens += 'A';
	}

	tty[8] = tens;
	tty[9] = ones;
	tty[10] = '\0';
}  /* end of function int_to_tty() */


int
tty_to_int(tty)
char *tty;
{
	int i;
	char tens,ones;

	if (tty[0] == '/') {		/* format is "/dev/ttyXX" */
		tens = tty[8];
		ones = tty[9];
	}
	else if (tty[0] == 't') {	/* format is "ttyXX" */
		tens = tty[3];
		ones = tty[4];
	}
	else
		return(-1);

	if ( tens >= 48 && tens <= 57 )
		tens -= '0';
	else if ( tens >= 97 && tens <= 122 ) {
		tens += 10;
		tens -= 'a';
	} else if ( tens >= 65 && tens <= 90 ) {
		tens += 36;
		tens -= 'A';
	}
	else return(-1);

	if ((ones >= '0') && (ones <= '9'))
		return(tens*62 + (ones - '0'));
	
	else if ((ones >= 'a') && (ones <= 'z'))
		return(tens*62 + (10 + (ones - 'a')));

	else if ((ones >= 'A') && (ones <= 'Z'))
		return(tens*62 + (36 + (ones - 'A')));	

	else return(-1);
}  /* end of function tty_to_int() */


add_new_tty(newtty)
char *newtty;
{
	register int newt;
	struct stat statb;
	char tempname[TTYNAMELEN];

	newt = tty_to_int(newtty);
	bzero(&statb,sizeof(statb));
	if (newtty[0] == '/') {
		strcpy(lat_ttys[newt].name, newtty);
	} else {
		strcpy(tempname,"/dev/");
		strcat(tempname,newtty);
		strcpy(lat_ttys[newt].name, tempname);
	}
	if ((stat(newtty,&statb) >= 0)) {
		lat_ttys[newt].tdev = statb.st_rdev;
		lat_minors[minor(statb.st_rdev)] = TRUE;
	}
	/*
	 * else return an error ??? - shouldn't have to: stat() should never
	 * fail because we just created this device...
	 */
}  /* end of function add_new_tty() */


remove_tty(byetty)
char *byetty;
{
	register int byet;

	byet = tty_to_int(byetty);
	lat_minors[minor(lat_ttys[byet].tdev)] = FALSE;
	bzero(&lat_ttys[byet],sizeof(lat_ttys[byet]));
}


change_tty_info(tty,command)
char *tty;
int command;
{
	register int ttyi;

	ttyi = tty_to_int(tty);

	switch(command) {

	case CREATE_TTY_DEVICE:
		add_new_tty(tty);
		lat_dev_total++;        /* increment total */
		break;

	case REMOVE_TTY_DEVICE:
		remove_tty(tty);
		lat_dev_total--;        /* decrement total */
		break;

	case ADD_TO_INITTAB:
		lat_ttys[ttyi].flags |= ININITTAB;
		lat_entries_total++;            /* new inittab entry */
		break;

	case REMOVE_FROM_INITTAB:
		lat_ttys[ttyi].flags &= ~ININITTAB;
		lat_entries_total--;            /* no more inittab entry */
		break;
	}
}  /* end of function change_tty_info() */


int
get_inittab_info(output_flag)
int output_flag;
{
	int c = 0;
	int lastc;
	FILE *inittab,*itmpl;
	char linebuffer[LINESIZE];
	char *lbuf,tbuf;
	char tty_expbuf[EXPBUFSIZE];
	char getty_expbuf[EXPBUFSIZE];
	char *tty_arg = "tty";
	char *getty_arg = "getty";
	char ttyname[TTYNAMELEN];
	int ttyidx;
	int no_lat_tty = 0;
	struct stat statbuf;
	int i;
	static int first_time = TRUE;

	/*
	 * If get_inittab_info() has been called before, then we have to
	 * check for the existence of the inittab template and temporary
	 * files and remove them if they exist. 
	 */
	if (!first_time) {
		/* remove temp files */
		if (stat(INITTAB_TEMPLATE,&statbuf) == 0)
			rm_file(INITTAB_TEMPLATE,output_flag);	
		if (stat(INITTAB_TMP_FILE,&statbuf) == 0)
			rm_file(INITTAB_TMP_FILE,output_flag);	
	}

	/* clear out inittab info in lat_ttys[] entries and reset count */
	for (i = 0; i < MAX_DEVS; i++) {
		if (lat_ttys[i].flags & ININITTAB) {
			if (lat_ttys[i].iline.c_command)
				free(lat_ttys[i].iline.c_command);
			bzero(&lat_ttys[i].iline,sizeof(lat_ttys[i].iline));
			lat_ttys[i].flags &= ~ININITTAB;
		}
	}
	lat_entries_total = 0;

	if ((inittab = fopen(INITTAB_FILE, "r")) == NULL)  {
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"Fatal Error: couldn't open %s\n",INITTAB_FILE);
			perror(INITTAB_FILE);
		}
		else {
                       	attrset(A_BOLD);
			move(y++, X_LOC);
			clrtobot();
			printw("Fatal Error: couldn't open %s\n",INITTAB_FILE);
			refresh();
                       	attrset(0);
		}
		clean_n_exit(5);
	}

	/* Create a template inittab file to be used as necessary later
	 * to create a new /etc/inittab. The template file contains everything
	 * that was in the original inittab with the exception of LAT gettys
	 * that are found by this routine. 
	 */

	if ((itmpl = fopen(INITTAB_TEMPLATE, "w")) == NULL)  {
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"Fatal Error: couldn't open %s\n",INITTAB_TEMPLATE);
			perror(INITTAB_TEMPLATE);
		}
		else {
                       	attrset(A_BOLD);
			move(y++, X_LOC);
			clrtobot();
			printw("Fatal Error: couldn't open %s\n",INITTAB_FILE);
			refresh();
                       	attrset(0);
		}
		clean_n_exit(5);
	}

	/* 
 	 * Set up expression buffers for regular expression parsing 
 	 */
	compile(tty_arg,tty_expbuf,(tty_expbuf+EXPBUFSIZE),(int) '\0');
	compile(getty_arg,getty_expbuf,(getty_expbuf+EXPBUFSIZE),(int) '\0');

	/*
	 * Read and process all lines from the /etc/inittab file.
	 */

	while (c != EOF) {		/* read until end-of-file */
		lbuf = linebuffer;
		lastc = '\0';
		no_lat_tty = 0;
		while (((c = getc(inittab)) != '\0') && (c != EOF)) {
			/* check for continuation */
			if ((c == '\n') && (lastc != '\\'))
				break;		/* it's end of line */
			*lbuf++ = (char)c;
			lastc = c;
		}

		*lbuf++ = NULL;		/* got a line, NULL terminate it */


		/* if it's a comment or no match, skip it */
		if ((linebuffer[0] == '#') || !step(linebuffer,getty_expbuf)) {	
			if (c != EOF)
				*(lbuf - 1) = (char)c;	/* add back the CR */
			*lbuf++ = NULL;
                        fputs(linebuffer,itmpl);
			continue;
		}
		else {	
			/* 
			 * It's a match - we found a getty.  now 
			 * start looking for "tty" in the line.  Start
			 * after after where we found "getty" to make the 
			 * search easier and more efficient.
			 */
			if (step(loc2,tty_expbuf)) {
				strncpy(ttyname,loc1,LASTCOMPLEN);
				if (its_a_lat_tty(ttyname) && 
			   	  ((ttyidx = tty_to_int(ttyname)) != -1) &&
			  	  !(lat_ttys[ttyidx].flags & ISMAPPED)) {

					if (save_inittab_line(linebuffer,ttyidx)								     == FAILURE)
						return(FAILURE);
					lat_ttys[ttyidx].flags |= ININITTAB;
					lat_entries_total++;
				}
				else
					no_lat_tty++;
			}
			else
				no_lat_tty++;

			if (no_lat_tty) {
				/* tty wasn't a LAT tty - pass the line thru */
				if (c != EOF)
					*(lbuf - 1) = (char)c;
				*lbuf++ = NULL;
                        	fputs(linebuffer,itmpl);
				continue;
			}
		}  /* it's a match */
	}  /* while ! EOF */

	fclose(inittab);
	fclose(itmpl);
	first_time = FALSE;
	return(SUCCESS);
}  /* end of function get_inittab_info() */


int
save_inittab_line(lbuf,idx)
char *lbuf;
int idx;
{
	char *tp = lbuf;
	char *tempptr;

	/* get ID field */
	tempptr = lat_ttys[idx].iline.c_id;
	while (*tp != ':')
		*tempptr++ = *tp++;
	*tempptr++ = NULL;		/* NULL terminate it */
	tp++;				/* get past the ':' */

	/* get levels field */
	tempptr = lat_ttys[idx].iline.c_levels;
	while (*tp != ':')
		*tempptr++ = *tp++;
	*tempptr++ = NULL;		/* NULL terminate it */
	tp++;				/* get past the ':' */

	/* get action field */
	tempptr = lat_ttys[idx].iline.c_action;
	while (*tp != ':')
		*tempptr++ = *tp++;
	*tempptr++ = NULL;		/* NULL terminate it */
	tp++;				/* get past the ':' */

	/* the rest (up to the NULL in lbuf) is the command field */

	if ((lat_ttys[idx].iline.c_command = (char *)malloc(strlen(tp)+1)) == NULL)
		return(FAILURE);
	strcpy(lat_ttys[idx].iline.c_command,tp);
	return(SUCCESS);
}


int
its_a_lat_tty(tty)
char *tty;
{
	int idx;

	/*
	 * Major premise of this function is that the information
	 * in lat_ttys[] has already been initialized / is up-to-date !!
	 *
	 * The test is:
	 * IF tty is in range AND tty exists AND tty has LAT major number
	 */

	if (((idx = tty_to_int(tty)) != -1) &&
	    (lat_ttys[idx].name[0] != '\0') &&
	    (major(lat_ttys[idx].tdev) == LAT_MAJOR))
		
		return(TRUE);
	else
		return(FALSE);
}


int
regerr(x)
int x;
{
	fprintf(stderr,"\nFATAL RE error detected: ");
	switch(x) {
	case 11:
		fprintf(stderr,"Range endpoint too large\n");
	case 16:
		fprintf(stderr,"Bad number\n");
	case 25:
		fprintf(stderr,"\\digit out of range\n");
	case 36:
		fprintf(stderr,"Illegal or missing delimiter\n");
	case 41:
		fprintf(stderr,"No remembered search string\n");
	case 42:
		fprintf(stderr,"There is a \\(\\) pair imbalance\n");
	case 43:
		fprintf(stderr,"Too many \\(\\) pairs (max is 9)\n");
	case 44:
		fprintf(stderr,"More than 2 numbers given in the \\{\\} pair\n");
	case 45:
		fprintf(stderr,"A } expected after \\\n");
	case 46:
		fprintf(stderr,"1st number exceeds 2nd in the \\{\\} pair\n");
	case 49:
		fprintf(stderr,"There is a [] pair imbalance\n");
	case 50:
		fprintf(stderr,"Regular expression overflow\n");
	case 70:
		fprintf(stderr,"Invalid endpoint in range expression\n");
	}
	exit(5);
}  /* end of function regerr() */


#ifdef	DEBUG
dump_lat_ttys()		/* FOR DEBUG ONLY!!!! */
{
        register int i;
        FILE *fp;

        if ((fp = fopen("/tmp/lat_tty.dump","w")) == NULL) {
                printf("Can't dump lat_ttys !! \n");
                exit(99);
        }

        for (i=0; i<620; i++) {

                if (lat_ttys[i].name[0] != NULL) {
                        fprintf(fp,"lat_ttys[%d].name = \t\t%s\n",i,lat_ttys[i].
name);
                        fprintf(fp,"major(lat_ttys[%d].tdev) = \t%d\n",i,major(lat_ttys[i].tdev));
                        fprintf(fp,"minor(lat_ttys[%d].tdev) = \t%d\n",i,minor(lat_ttys[i].tdev));
                        fprintf(fp,"lat_ttys[%d].flags = \t\t0x%x\n",i,lat_ttys[i].flags);
                        fprintf(fp,"\n");
                }
		if (lat_minors[i])
			fprintf(fp,"lat_minors[%d] is ALLOCATED\n",i);
        }
	fprintf(fp,"\nlat_dev_total = \t\t%d\n",lat_dev_total);
	fprintf(fp,"non_lat_dev_total = \t\t%d\n",non_lat_dev_total);
        fprintf(fp,"\n\n");

        fclose(fp);

}
#endif

/*
 * This routine is called from main menu to undo the LAT setup such as
 * delete LAT entries from the /etc/inittab file, remove the corresonding
 * /dev/tty files, reset the LAT automatic startup/shutdown variable, and
 * stop LAT.
 */
undo_lat_setup()
{
        int y;
        int ret, entries_to_remove = 0;
	int ioctl_failed = 0;
	int lockfd, proceed_y;
	int lockerr = 0;

        if (undo_intro() == NO)	/* put out undo introduction & see if we */
                return;		/* should continue */

	/* 
	 * Open the /etc/inittab file and lock it for exclusive use.  This
	 * file descriptor is used only for locking and unlocking the
	 * file.  If we can't open it or lock it, warn the user and make
	 * them acknowledge this condition.
	 */

	if ((lockfd = open(INITTAB_FILE,O_RDWR)) < 0)
		lockerr++;
	if (!lockerr)
		if (flock(lockfd,LOCK_EX|LOCK_NB) < 0)
			lockerr++;
	if (lockerr) {
		attrset(A_BOLD|A_BLINK);
		y = OUTPUT_Y;
		move(y, X_LOC);
		clrtobot();
		mvaddstr(y++, X_LOC+2, "Warning!  The /etc/inittab file could not be locked for");
		mvaddstr(y++, X_LOC+2, "exclusive use.  Another process may be modifying it.");  
		proceed_y = y+=2;
		mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?");
		y+=3;
		switch (get_yes_no(y)) {
		case YES:
			mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?  YES");
			attrset(0);
			flock(lockfd,LOCK_UN);
			close(lockfd);
			return;
		case NO:
			mvaddstr(proceed_y, X_LOC+5, "Do you wish to cancel this operation?  NO");
			refresh();
			attrset(0);
			break;
		}
	}

	/*
	 * Call init_tty_info() now to make sure we have the latest
	 * and greatest information.  If the call returns FAILURE status,
	 * we have no choice but to exit, as the lat_ttys[] array and the
	 * global counts are no longer valid.
	 */
	if (init_tty_info(INIT_ALL,OUT_STDSCR) == FAILURE) {
		attrset(A_BOLD|A_BLINK);
		y = OUTPUT_Y;
		move(y, X_LOC);
		clrtobot();
		mvaddstr(y++, X_LOC+2, " Fatal Error: cannot initialize - insufficient memory\n");
		refresh();
		attrset(0);
		clean_n_exit(8);
	}

	/* Proceed with interrogation and undo operation */

	y = 3;
        move(y++, X_LOC); clrtobot(); refresh();
	if ( lat_entries_total > 0 ) {
		/* Get the number of LAT entries to delete from /etc/inittab */
		entries_to_remove = get_remove_input(y);
        	y+=5;
	}

        move(y++,X_LOC);
	printw("Removing %d", entries_to_remove);

	if (entries_to_remove == 1)
		printw(" LAT entry ");
	else
		printw(" LAT entries ");

	printw("from %s, deleting %d", INITTAB_FILE, entries_to_remove);

	if (entries_to_remove == 1)
		printw(" LAT tty,");
	else
		printw(" LAT ttys,");

        move(y++,X_LOC);
	printw("stopping LAT, and disabling LAT automatic startup and shutdown.");
	if (entries_to_remove > 0) {
        	move(y++,X_LOC);
		printw("Saving %s to %s.", INITTAB_FILE, INITTAB_SAVE);
		ret = copy_file(INITTAB_FILE,INITTAB_SAVE,OUT_STDERR);
	}
	y++;

	/*
	 * Stop LAT,
	 * disable automatic LAT startup/shutdown, remove entries
	 * from /etc/inittab and delete ttys.
	 */
	clrtobot();
	mvaddstr(y++,X_LOC,"Attempting to stop the driver.  Please wait...");
	refresh();
	y++;

	bzero(&latioctl_node, sizeof(latioctl_node));
	latioctl_node.ln_flags = LATIOCTL_NODE_STATE;
	latioctl_node.ln_state = LAT_STATE_OFF;

	if (ioctl(latfd, LIOCSNODE, &latioctl_node) < 0) {
		move(y++,X_LOC);
		clrtobot();
		printw("LAT NOT stopped. (LIOCSNODE failed, errno=%d)",errno);
		refresh();
	} else {
		move(y,X_LOC);
		clrtobot();
		mvaddstr(y,X_LOC,"LAT stopped.");
		refresh();
	}

	/* disable automatic LAT startup/shutdown */
        if (reset_LAT_SETUP() < 0) {
		flock(lockfd,LOCK_UN);
		close(lockfd);
		return;
	}
	if (entries_to_remove > 0) {

        	remove_inittab_entries(entries_to_remove, OUT_STDSCR, TRUE);
		if (sync_inittab(OUT_STDSCR) != SUCCESS) {
			move(y++,X_LOC);
			clrtobot();
			printw("Error: could not rewrite /etc/inittab!\n");
			refresh();
		}
		if (perform_initq(OUT_STDSCR) != SUCCESS) {
			move(y++,X_LOC);
			clrtobot();
			printw("Error: could not issue ""init q"" command!\n");
			refresh();
		}
	}
	flock(lockfd,LOCK_UN);
	close(lockfd);
}  /* end of function undo_lat_setup() */

/*
 * This routine outputs the initial undo screen and asks the user if they
 * want to continue.
 */
undo_intro()
{
        int y_loc=14;
        int intro_ret = -1;

        clear();
        attrset(A_BOLD);
        mvaddstr(1, 31, "Undo LAT Setup");
        attrset(0);
        mvaddstr(4, 4,
         "This option will allow you to undo the LAT setup on your system");
        mvaddstr(5, 4,
         "by removing a specified number of LAT entries from /etc/inittab,");
        mvaddstr(6, 4,
          "deleting these LAT ttys, and stopping LAT.  A variable will also");
        mvaddstr(7, 4,
          "be reset in /etc/rc.config to disable LAT automatic startup and");
        mvaddstr(8, 4,
          "shutdown so that LAT will not start when entering run-level 3.");
        mvaddstr(10, 2,
          "NOTE: Active LAT sessions will be disconnected when LAT is stopped.");
        mvaddstr(12, 15, "Would you like to continue with this option?");

        switch (intro_ret = get_yes_no(y_loc) ) {

        case YES:
                mvaddstr(12, 15, "Would you like to continue with this option?  YES");
                clrtobot();
                refresh();
                break;
        case NO:
                mvaddstr(12, 15, "Would you like to continue with this option?  NO");
                clrtobot();
                refresh();
                attrset(A_BOLD);
                move(17, 19);
                addstr("You have chosen not to continue.");
                attrset(0);
        }
        move(16,16);
        clrtobot();
        return(intro_ret);
}  /* end of function undo_intro */


/*
 * This routine is used to solicit from the user the number of LAT entries
 * to be removed from the INITTAB_FILE.
 */
int
get_remove_input(y)
{
int not_done, entries_to_remove, c, curr_y;

        not_done = 1;
        curr_y = y;
        while (not_done) {
                move(y++,X_LOC);
                clrtobot();
                addstr("How many LAT entries would you like removed from the ");
                move(y++,X_LOC);
                printw("        %s file? (0 - %d) [%d]  ",
                        INITTAB_FILE, lat_entries_total, lat_entries_total);
                refresh();

		/* pass in default value to get_int_value() */
                entries_to_remove = get_int_value(lat_entries_total);
                if (entries_to_remove < 0 || entries_to_remove > lat_entries_total) {
                        move(y++,X_LOC);
                        printw("You have provided an incorrect value, %d.",
                                entries_to_remove);
                        y++;                            /* skip a line */
                        move(y++,X_LOC);
                        printw("Enter <RETURN> to reenter the number of entries to be removed.");
                        refresh();
			while ((c = getch()) && !IS_RETURN(c)) ;
                        y = curr_y;
                } else {
                        not_done = 0;
                }
        }

        return(entries_to_remove);

} /* end of routine get_remove_input() */


/*
 * This routine removes the inittab information and the corresponding 
 * device special file for as many ttys as requested.
 */
remove_inittab_entries(entries_to_remove, output_flag)
int entries_to_remove, output_flag;
{
        int i;

       	for (i=0; i < MAX_DEVS; i++) {
               	if (lat_ttys[i].flags & ININITTAB && 
		    lat_ttys[i].name[0] != NULL) {

			if (lat_ttys[i].iline.c_command)
				free(lat_ttys[i].iline.c_command);
			bzero(&lat_ttys[i].iline,sizeof(lat_ttys[i].iline));
                       	change_tty_info(lat_ttys[i].name,REMOVE_FROM_INITTAB);
                       	rm_file(lat_ttys[i].name, output_flag);
                       	change_tty_info(lat_ttys[i].name,REMOVE_TTY_DEVICE);
			if (--entries_to_remove == 0)
				break;
               	} 
       	} /* for i... */

}       /* end of function remove_inittab_entries() */

/*
 * This routine removes the inittab information associated with a paricular
 * entry in the lat_ttys[] array.
 */
int
remove_inittab_info(ttyidx,output_flag)
int ttyidx; 
int output_flag;
{
	return(SUCCESS);

}       /* end of function remove_inittab_info() */


int
copy_file(from,to,output_flag)
char *from,*to;
int output_flag;
{

	int fd1, fd2, n;
	char buf[BUFSIZE];

	if ((fd1 = open(from,O_RDONLY)) == NULL) {
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"copy_file: ERROR, can't open %s\n",from); 
		} else {
			move(y++,X_LOC);
			printw("copy_file: ERROR, can't open %s\n",from);
			refresh();
		}
		return(FAILURE);
	}

	if ((fd2 = open(to,O_CREAT|O_TRUNC|O_WRONLY,PMODE)) == NULL) {
		if (output_flag == OUT_STDERR) {
			fprintf(stderr,"copy_file: ERROR, can't open %s\n",to); 
		} else {
			move(y++,X_LOC);
			printw("copy_file: ERROR, can't open %s\n",to);
			refresh();
		}
		return(FAILURE);
	}

	while ((n = read(fd1, buf, BUFSIZE)) > 0) {
		if (write(fd2, buf, n) != n) {
			if (output_flag == OUT_STDERR) {
				fprintf(stderr,"copy_file: write ERROR on %s\n",to);
			} else {
				move(y++,X_LOC);
				printw("copy_file: write ERROR on %s\n",to);
				refresh();
			}
			return(FAILURE);
		}
	}
	close(fd1);
	close(fd2);
	return(SUCCESS);
}

/* display a file one "page" at a time */
showfile(fd,minx,miny,ScreenLength)
FILE *fd;
int minx,miny,ScreenLength;
{
	int c,d = ' ';
	int x,y;
	int xcentermore,xcenterdone;
	char morestr[50] = "-More (q to quit, any key for next page)-";
	char donestr[40] = "-Done (hit any key to continue)-";

	if (ScreenLength == 0)
		ScreenLength = LINES-3;
	xcentermore = (COLS - strlen(morestr))/2;		
	xcenterdone = ((COLS - strlen(donestr))/2) - 4;		
	move(miny,minx);
	clrtobot();
	refresh();
	c = fgetc(fd);
	while ((c != EOF) && ((char) d != 'q') && ((char) d!=ESCAPE)) {
		getyx(stdscr,y,x);
		if (y >= ScreenLength) {
			printw("\n");
			y++;
			move(y,xcentermore);
			printw("%s",morestr);
			refresh();
			d = wgetch(stdscr);
			move(miny,minx);
			getyx(stdscr,y,x);
			clrtobot();
			refresh();
		}
		if ((c != '\n') && ((char) d != 'q') && ((char) d!=ESCAPE))
			printw("%c",(char) c);
		else {	/* simulate a carriage return */
			y++;
			move(y,minx);
		}
		c = fgetc(fd);
	}
	if (((char) d != 'q') && ((char) d!=ESCAPE)) {
		printw("\n");
		getyx(stdscr,y,x);
		move(y,xcenterdone);
		printw("%s",donestr);
		refresh();
		getch();
		move(miny,minx);
		clrtobot();
		refresh();
	}
}
