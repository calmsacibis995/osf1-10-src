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
static char	*sccsid = "@(#)$RCSfile: vgcreate.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/11/06 14:04:35 $";
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
 *   vgcreate:
 *   Creates a volume group.
 */

/*
 *  Modification History:  vgcreate.c
 *
 *  29-Aug-91     Terry Carruthers
 *	Modified code to generate the clean vg path name before
 *      calling the /etc/lvmtab operations to determine if the vg
 *      already exists.
 *
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
static void clean_up_vg(int vg_fd, char *vgpath, char *pvpath);
static char * local_check_and_openvg(char *vgpath, int *vg_fd);

/* Local defines */
#define LVM_DEFPEXS	1016
#define LVM_DEFPEXSIZE	1
#define LVM_DEFPVS	32

/*
 *   Usage message: badly formatted because automatically
 *   generated. A local library routine will print it
 *   in a better fashion.
 */

#define USAGE	"Usage: vgcreate  [-x Extensibility] [-e MaxPhysicalExtents] [\
-l MaxLogicalVolumes] [-p MaxPhysicalVolumes] [-s PhysicalExtentSize] [\
-v VGDA] \
 VolumeGroupName  PhysicalVolumePath... \n"

/* Options which require an argument for their value */
#define OPT_WITH_VAL_NUM	6
#define OPTIONS_WITH_VALUE	"xelpsv"
char xflag; char Extensibility;       /* legal values: "yn" */
char eflag; int MaxPhysicalExtents;
char lflag; int MaxLogicalVolumes;
char pflag; int MaxPhysicalVolumes;
char sflag; int PhysicalExtentSize;
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

	struct	lv_createvg	createvg;
	struct	lv_installpv	installpv;
	char 	*clean_vgpath;
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

	/* read the lvmtab-file into memory */
	if (lvmtab_read() == NOT_OK) {
		print_prgname();
		fprintf(stderr, MSG_LVMTAB_READ_ERROR, LVMTABPATH);
		exit(FATAL_ERROR);
	}

	/*
         * check VolumeGroupName and open the VG control file.
         * If successful, a clean path which can be used in
         * operations on /etc/lvmtab and vg_fd are returned, else
         * NULL is returned.
         * check_and_openvg() prints the needed error messages.
         */
        if ((clean_vgpath = local_check_and_openvg(VolumeGroupName, &vg_fd)) 
			== NULL)
                exit(FATAL_ERROR);

	/* it's an error if the VG already exists */
	if (lvmtab_isvgdefined(clean_vgpath)) {
		print_prgname();
		fprintf(stderr, MSG_VG_IN_LVMTAB, clean_vgpath, LVMTABPATH);
		exit(FATAL_ERROR);
	}

	/* 
	 * initialize parts of struct vgcreate.
	 * The variables on the right side of '=' are either initialized
	 * by the default value or the value read from the command line
	 */
	createvg.vg_id.id1 = gethostid();
	createvg.vg_id.id2 = time(NULL);
	createvg.pv_flags = 0;
	createvg.maxlvs = MaxLogicalVolumes;
	createvg.maxpvs = MaxPhysicalVolumes;
	createvg.maxpxs = MaxPhysicalExtents;
	createvg.pxsize = PhysicalExtentSize * 1024 * 1024;
	createvg.pxspace = createvg.pxsize;
	createvg.maxdefects = 0;
 	 
	if (Extensibility == 'n')
		createvg.pv_flags |= LVM_PVNOALLOC;

        /*
         * Test if PhysicalVolumePath is a block special file and
         * returnes a clean path starting from root in the second
         * argument. If successful OK is returned, else NOT_OK.
         * isblock_and_clean() displays the needed error messages.
         * See if it's already in the lvmtab file
         */
        if (isblock_and_clean(PhysicalVolumePath[0], &createvg.path,
                        CHECKLVMTAB) == NOT_OK)
                exit(FATAL_ERROR);

   	/* 
       	 * Because of consistency reasons, the interupts are disabled 
    	 * from here on.
    	 */
   	disable_intr();

	/* Dump struct createvg if in debug mode */
	debug(dbg_createvg_dump(&createvg));

	/*
	 * let the LVDD create the volume group and install the first
	 * physical volume. Bouth steps are done in one system call.
	 * It is presumed that LVM_CREATEVG leaves the VG in an
 	 * active state.
	 */
	if (ioctl(vg_fd, LVM_CREATEVG, &createvg) == -1) {
		print_prgname();
		fprintf(stderr, MSG_LVM_CREATEVG_FAILED, clean_vgpath);
      		lvm_perror(LVM_CREATEVG);
		debug_msg("ioctl(CREATEVG)\n", NULL);
		close(vg_fd);
		exit(FATAL_ERROR);
	}

	/*
	 * Add the clean_vgpath, vg_id and the first physical volume to the 
	 * lvmtab file.
	 * In case of error when writing the information to lvmtab,
	 * the first PV is removed from the VG. A removal of the
	 * last PV in the VG will have the reverse effect as LVM_ACTIVATEVG.
	 */
	if ((lvmtab_addvg(clean_vgpath, &createvg.vg_id, NOWRITE) == NOT_OK) ||
	    (lvmtab_addpvtovg(clean_vgpath, createvg.path, DOWRITE) == NOT_OK)) {
		print_prgname();
		fprintf(stderr, MSG_LVMTAB_ERROR, clean_vgpath, createvg.path,
			 LVMTABPATH);
		clean_up_vg(vg_fd, clean_vgpath, createvg.path);
		exit(FATAL_ERROR);
	}
	

	/*
	 * Install the rest of the physical volumes in PhysicalVolumePath
	 * into the VG
	 */
	installpv.pv_flags = 0;
	installpv.pxspace = createvg.pxspace;
	installpv.maxdefects = 0;

	if (Extensibility == 'n')
		installpv.pv_flags |= LVM_PVNOALLOC;

	if (VGDA == 'n')
		installpv.pv_flags |= LVM_NOVGDA;

	/*
	 * install_pv will display error messages, but none of the
	 * error cases in install_pv() are fatal
	 */
	if (install_pv(vg_fd, clean_vgpath, &installpv, &PhysicalVolumePath[1],
		   PhysicalVolumePath_cnt - 1) == OK)

		printf(MSG_VG_CREATED, clean_vgpath);
	
	   

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

   if (bad_int_arg_value(&eflag, &MaxPhysicalExtents, 'e', "MaxPhysicalExtents"))
      return(NOT_OK);

   if (bad_int_arg_value(&lflag, &MaxLogicalVolumes, 'l', "MaxLogicalVolumes"))
      return(NOT_OK);

   if (bad_int_arg_value(&pflag, &MaxPhysicalVolumes, 'p', "MaxPhysicalVolumes"))
      return(NOT_OK);

   if (bad_int_arg_value(&sflag, &PhysicalExtentSize, 's', "PhysicalExtentSize"))
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

   /* Check for ranges */
   if (!in_range(MaxPhysicalExtents, 1, LVM_MAXPXS)) {
      usage_error("MaxPhysicalExtents", MSG_BETWEEN_1_AND_MAXPXS);
      return(NOT_OK);
   }

   if (!in_range(MaxLogicalVolumes, 1, LVM_MAXLVS)) {
      usage_error("MaxLogicalVolumes", MSG_BETWEEN_1_AND_255);
      return(NOT_OK);
   }

   if (!in_range(MaxPhysicalVolumes, 1, LVM_MAXPVS)) {
      usage_error("MaxPhysicalVolumes", MSG_BETWEEN_1_AND_255);
      return(NOT_OK);
   }

   /* PhysicalExtentSize (in Mbytes) must be a power of 2 in the range 1..256 */
   switch (PhysicalExtentSize) {
      case 1:
      case 2:
      case 4:
      case 8:
      case 16:
      case 32:
      case 64:
      case 128:
      case 256:
	    /* they all are correct */
	 break;
      default:
            usage_error("-s", MSG_POWER_OF_2);
	    return(NOT_OK);
   }

   /* Nothing to complain about */
   return(OK);
}



static int
set_defaults()
{
   /* Defaults as defined in the manual pages */
   Extensibility = 'y';
   MaxPhysicalExtents = LVM_DEFPEXS;
   MaxLogicalVolumes = LVM_MAXLVS;
   MaxPhysicalVolumes = LVM_DEFPVS;
   PhysicalExtentSize = LVM_DEFPEXSIZE; 	/* megabytes */
   VGDA = 'y';
}

static void
clean_up_vg(int vg_fd, char *vgpath, char *pvpath)
{
struct 	lv_querypvpath	querypvpath;
int	pv_key;

	debug(dbg_entry("clean_up_vg"));

	/* Get the pv_key to be used by LVM_DELETEPV */
	querypvpath.path = pvpath;

	if (query_driver(vg_fd, LVM_QUERYPVPATH, &querypvpath) == -1) {
		print_prgname();
		fprintf(stderr, MSG_PV_NOT_DEL_BY_LVDD, pvpath, LVMTABPATH, 
			vgpath);
      		lvm_perror(LVM_QUERYPVPATH);
	}
	else {
		/* 
		 * Deleting the last PV in the VG has the inverse effect
		 * of LVM_CREATEVG
		 */
		pv_key = querypvpath.pv_key;
		debug(dbg_pvID_dump(&pv_key));
		if (ioctl(vg_fd, LVM_DELETEPV, &pv_key) == -1) {
			print_prgname();
			fprintf(stderr, MSG_PV_NOT_DEL_BY_LVDD, pvpath, 
				LVMTABPATH, vgpath);
      			lvm_perror(LVM_DELETEPV);
			debug_msg("ioctl(DELETEPV)\n", NULL);
		}
	}
	debug(dbg_exit());
}

static char *
local_check_and_openvg(char *vgpath, int *vg_fd)
{
char *clean_path;
char grp_path[PATH_MAX];

        debug(dbg_entry("local_check_and_openvg"));

	/* test if vgpath is good and can be used by lvmtab_isvgdefined */
        if ((clean_path = mk_clean_path(vgpath)) == NULL) {
                print_prgname();
                fprintf(stderr, MSG_NO_CLEAN_PATH, vgpath);
                debug(dbg_exit());
                return(NULL);
        }

        strncpy(grp_path, clean_path, PATH_MAX - sizeof(GROUP) - 1);
        strcat(grp_path, "/");
        strcat(grp_path, GROUP);

        /* open the VG control file */
        if ((*vg_fd = open(grp_path, O_RDWR)) == -1) {
                print_prgname();
                fprintf(stderr, MSG_CANNOT_OPEN_GRP_FILE, grp_path);
                perror("");
                debug_msg("(open)\n", NULL);
                debug(dbg_exit());
                return(NULL);
	}
	debug(dbg_exit());

	return(clean_path);
}
