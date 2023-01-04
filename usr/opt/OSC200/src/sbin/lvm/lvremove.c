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
static char	*sccsid = "@(#)$RCSfile: lvremove.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/*
 * OSF/1 Release 1.0
 */

/*
 *   lvremove:
 *   Removes one or more logical volumes from a volume group.
 */

/* Each file containing a main() has some privilege: see "lvmcmds.h" */
#define LVM_CMD_MAIN_FILE
#include "lvmcmds.h"

/*
 *   Here are all the declarations that are specific to this command,
 *   that is, file inclusions, definitions, variables, types, etc.
 */

/* Local functions */
static int check_usage_semantics();
static int set_defaults();

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: lvremove  [-f] \
 LogicalVolumePath... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	0
#define OPTIONS_WITH_VALUE	""

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	1
#define OPTIONS_WITHOUT_VALUE	"f"
char fflag;

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		1
char **LogicalVolumePath;	/* maybe a list => array of pointers */
int LogicalVolumePath_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{

 	struct  lv_querylv      querylv;
        char    *vgpath, *yes, answer[10];
	char	raw_lv_path[PATH_MAX + 1];
	register char *basename;
        int     vg_fd, i;
	dev_t	lv_dev_num;
	int	minor_num;


   /* Initialize the i18n (internationalization) support */
   msg_init();

   /* Set defaults specific to this command */
   set_defaults();

   /* See if the user typed a proper request */
   if (check_usage(argc, argv) != OK) {
      print_usage(USAGE);
      print_arg_error();
      exit(1);
   }

   init_debug();

	/*
         * Open the volume group if possible and query the logical
         * volume LogicalVolumePath to get the current setting.
         * Error messages are printed by openvg_and_querylv() in case
         * of failure.
         */
	if (openvg_and_querylv(LogicalVolumePath[0], &vgpath, &vg_fd,
                                &querylv) != OK)
                exit(FATAL_ERROR);

	/* 
	 * Because of consistency reasons, the interupts are disabled 
	 * from here on.
	 */
	disable_intr();

	for (i = 0; i < LogicalVolumePath_cnt; i++) {
		/* Is LogicalVolumePath[i] a legal path ? */
		if (!isalv(LogicalVolumePath[i])) {
			print_prgname();
			fprintf(stderr, MSG_LV_PATH_WRONG,
					LogicalVolumePath[i]);
			continue;
		}

		/* If LV not empty and fflag not set, ask for confirmation */
		if (!fflag && querylv.numlxs != 0) {
			printf(MSG_USER2_CONFIRMATION, LogicalVolumePath[i]);
			if (read_line(answer, sizeof(answer)) == EOF) {
				print_prgname();
				fprintf(stderr, MSG_BAD_INPUT_PARAMETER);
				exit(FATAL_ERROR);
			}

			/* Get the internationalized yes response */
			yes = nl_langinfo(YESSTR);

			debug_msg("user said \"%s\"\n", answer);
			debug_msg("nl_langinfo says \"%s\"\n", yes);

			if (*yes != *answer) 
				exit(0);
		}


		/*
		 *   Store the path name of the char raw device of LV;
		 *   we have to insert an 'r' before "lvol"
		 */
		strcpy(raw_lv_path, LogicalVolumePath[i]);
		for (basename = &raw_lv_path[strlen(raw_lv_path) - 1];
				basename >= raw_lv_path && *basename != '/';
				basename--)
			continue;
		basename++;
		*basename++ = 'r';
		strcpy(basename,
			&LogicalVolumePath[i][(basename - raw_lv_path) - 1]);
		debug_msg("raw_lv_path: \"%s\"\n", raw_lv_path);

		/* Get the minor number of the logical volume */
		if (special_f_tst(LogicalVolumePath[i], S_IFBLK, &lv_dev_num) 
				== NOT_OK ||
		    special_f_tst(raw_lv_path, S_IFCHR, NULL) 
				== NOT_OK) {
			print_prgname();
			fprintf(stderr, MSG_CANT_GET_LV_MINOR, 
					LogicalVolumePath[i]);
			continue;
		}

		/*
		 * Then remove the logical volume from the
		 * knowledge of the driver
		 */
		minor_num = (int)minor(lv_dev_num);
		debug(dbg_pvID_dump(&minor_num));
		if (ioctl(vg_fd, LVM_DELETELV, &minor_num) == -1) {
			print_prgname();
			fprintf(stderr, MSG_DELETELV_FAILED,
					LogicalVolumePath[i]);
			lvm_perror(LVM_DELETELV);
			debug_msg("ioctl(DELETELV)\n", NULL);
			continue;
		}

		/*
		 * Then finally remove the logical volume from the
		 * file-system
		 */
		close(vg_fd);
		if (rm_dir(LogicalVolumePath[i]) != OK) {
			print_prgname();
			fprintf(stderr, MSG_LV_NOT_RM_FROM_FS,
					LogicalVolumePath[i]);
			continue;
		}
		if (rm_dir(raw_lv_path) != OK) {
			print_prgname();
			fprintf(stderr, MSG_LV_NOT_RM_FROM_FS,
					raw_lv_path);
			continue;
		}
		printf(MSG_LV_REMOVED, LogicalVolumePath[i]);
	}
		
		


   /* Clean exit */
   return(0);
}



int
check_usage(int argc, char **argv)
{
   /* Call the general-purpose routine to check usage syntax */
   if (parse_args(&argc, &argv, OPTIONS_WITHOUT_VALUE, 
         OPTIONS_WITH_VALUE, REQ_ARGS_NUM) != OK)
      return(NOT_OK);

   /* Check whether usage syntax is correct */
   if (check_usage_syntax() != OK)
      return(NOT_OK);

   /* If we get to this point, usage syntax is correct */
   if (check_usage_semantics() != OK)
      return(NOT_OK);

   return(OK);
}



int
check_usage_syntax()
{
   register int i;

   /* See which options without value have been used */
   fflag = used_opt('f');

   /* Set references to mandatory arguments */
   LogicalVolumePath = left_arg(&LogicalVolumePath_cnt);

   return(OK);
}



/*
 *   Here are all the subroutines that are specific to this command,
 *   that is, routines that don't fit into the LVM cmd's library
 */

static int
check_usage_semantics()
{

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
}
