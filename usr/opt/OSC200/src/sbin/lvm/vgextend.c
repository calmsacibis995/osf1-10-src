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
static char	*sccsid = "@(#)$RCSfile: vgextend.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 15:42:53 $";
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
 *   vgextend:
 *   Extend a volume group by adding physical volumes to it.
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

#define USAGE	"Usage: vgextend  [-x Extensibility] [-v VGDA]\
 VolumeGroupName  PhysicalVolumePath... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	2
#define OPTIONS_WITH_VALUE	"xv"
char xflag; char Extensibility;       /* legal values: "yn" */
char vflag; char VGDA;                /* legal values: "yn" */

/* Options which DO NOT require an argument; i.e., boolean flags */
#define OPT_WITHOUT_VAL_NUM	0
#define OPTIONS_WITHOUT_VALUE	""

/* Requested args (mandatory) */
#define REQ_ARGS_NUM		2
char *VolumeGroupName;
char **PhysicalVolumePath;	/* maybe a list => array of pointers */
int PhysicalVolumePath_cnt;	/* number of items in the above array */

/* There are no extra args (optional) */



main(int argc, char **argv)
{

	struct 	lv_installpv	installpv;
	struct	lv_queryvg	queryvg;
	char	*clean_vgpath;
	int	vg_fd;


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

	/* initialize parts of struct installpv */
	installpv.pv_flags = 0;
	installpv.maxdefects = 0;

	/*
	 *   Actual value for this will be derived by the driver from
	 *   the value that has been set at VG creation time.
	 */
	installpv.pxspace = 0;

	if (Extensibility == 'n')
		installpv.pv_flags |= LVM_PVNOALLOC;

	if (VGDA == 'n')
		installpv.pv_flags |= LVM_NOVGDA;

	/*
	 * check VolumeGroupName and open the VG control file.
	 * If successful, a clean path which can be used in
	 * operations on /etc/lvmtab and vg_fd are returned, else
	 * NULL is returned.
	 * check_and_openvg() prints the needed error messages.
	 */
	if ((clean_vgpath = check_and_openvg(VolumeGroupName, &vg_fd)) == NULL)
		exit(FATAL_ERROR);
	
	/*
	 * Physical volumes can only be installed in an active VG, so
	 * check the state of the VG.
	 */
	if (query_driver(vg_fd, LVM_QUERYVG, &queryvg) == -1) {
		print_prgname();
		fprintf(stderr, MSG_NO_QUERYVG, clean_vgpath);
      		lvm_perror(LVM_QUERYVG);
		exit(FATAL_ERROR);
	}
	else {
		if ((queryvg.status & LVM_VGACTIVATED) == 0) {
			print_prgname();
			fprintf(stderr, MSG_VG_NOT_ACTIVE, clean_vgpath);
			exit(FATAL_ERROR);
		}
	}

   	/* 
    	 * Because of consistency reasons, the interupts are disabled 
    	 * from here on.
    	 */
   	disable_intr();

	/*
	 * install the physical volumes in PhysicalVolumePath into
	 * the VG. install_pv will display error messages, but none of
	 * the error cases in install_pv() are fatal.
	 */
	if (install_pv(vg_fd, clean_vgpath, &installpv, PhysicalVolumePath,
		   PhysicalVolumePath_cnt) == OK)
   		printf(MSG_VG_EXTENDED, VolumeGroupName);
	else
		return(FATAL_ERROR);


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

   /*
    *   See which options with value have been used;
    *   save a flag, and the value if supplied;
    *   whenever possible, check for correct usage
    *   of values for options
    */

   if (bad_char_arg_value(&xflag, &Extensibility, 'x', "Extensibility", "yn"))
      return(NOT_OK);

   if (bad_char_arg_value(&vflag, &VGDA, 'v', "VGDA", "yn"))
      return(NOT_OK);

   /* Set references to mandatory arguments */
   VolumeGroupName = next_arg();
   PhysicalVolumePath = left_arg(&PhysicalVolumePath_cnt);

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
