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
 *	@(#)$RCSfile: lvmdefmsg.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/05 16:02:35 $
 */ 
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
 *   lvmdefmsg.h
 *   
 *   Contents:
 *	This include file contains the default messages (i.e., the messages
 *	which will be printed if the message cannot be read from the message-
 *	catalogue) for all the LVM commands and library functions.
 */

/*
 *  Modification History:  lvmdefmsg.h
 *
 *  24-Apr-91     Terry Carruthers
 *	Added MSG_DISKTYPE_NOTUSED.  This message now used in
 *      in pvcreate.c function get_device_size function.
 *
 */

#ifdef	MSG
#include "lvm_msg.h"
#endif	/* MSG */

/*
 *   Many of the messages printed by lvm_perror() are using different
 *   defines, but they are the same message in the current implementation.
 *   This is done to prepare for future changes.
 *   Since many of the messages are the same, they are mapped into the
 *   same message here to save space in the message catalogue.
 */

/* These are the messages defined in terms of other messages */
#define MSG_CREATELV_ENOMEM		MSG_ACTIVATEVG_ENOMEM
#define MSG_CREATEVG_ENOMEM		MSG_ACTIVATEVG_ENOMEM
#define MSG_INSTALLPV_ENOMEM		MSG_ACTIVATEVG_ENOMEM

#define MSG_CHANGEPV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_CREATELV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_CREATEVG_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_DELETELV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_DELETEPV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_INSTALLPV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_OPTIONSET_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_OPTIONGET_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_QUERYPV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_QUERYPVMAP_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_QUERYPVPATH_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_QUERYPVS_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_RESYNCPV_ENOTTY		MSG_ATTACHPV_ENOTTY
#define MSG_SETVGID_ENOTTY		MSG_ATTACHPV_ENOTTY

#define MSG_CREATELV_EROFS		MSG_CHANGELV_EROFS
#define MSG_DELETELV_EROFS		MSG_CHANGELV_EROFS
#define MSG_INSTALLPV_EROFS		MSG_CHANGELV_EROFS

#define MSG_INSTALLPV_EIO		MSG_CREATEVG_EIO

#define MSG_INSTALLPV_ENODEV		MSG_CREATEVG_ENODEV

/* These are the actually defined messages */
#define	MSG_NOT_A_PATH_NAME			MSGSTR(NOT_A_PATH_NAME,\
"Must be a simple file name, not a path name.")

#define MSG_BETWEEN_1_AND_MAXPXS 	MSGSTR(BETWEEN_1_AND_MAXPXS, \
"Must be a value between 1 and 65535.")

#define MSG_BETWEEN_0_AND_MAXLXS 	MSGSTR(BETWEEN_0_AND_MAXLXS, \
"Must be a value between 0 and 65535.")

#define MSG_BETWEEN_1_AND_MAXLXS 	MSGSTR(BETWEEN_1_AND_MAXLXS, \
"Must be a value between 1 and 65535.")

#define MSG_ONE_OF_THEM_REQUIRED 	MSGSTR(ONE_OF_THEM_REQUIRED,\
"Exactly one of them has to be specified.")

#define MSG_ONE_OF_THEM 		MSGSTR(ONE_OF_THEM,\
"At least one of them has to be specified.")

#define MSG_BETWEEN_1_AND_2   		MSGSTR(BETWEEN_1_AND_2,\
"Must be either 1 or 2.")

#define MSG_GREATER_THAN_0    		MSGSTR(GRATER_THAN_0,\
"Must be greater than 0.")

#define MSG_MINUS_L_REQUIRED  		MSGSTR(MINUS_L_REQUIRED,\
"Can be used only if \"-l\" has been specified.")

#define MSG_EMPTY_LV_WITH_MIRRORS  	MSGSTR(EMPTY_LV_WITH_MIRRORS,\
"Can't create a mirrored empty logical volume. Specify \"-l\".")

#define MSG_BETWEEN_0_AND_1   		MSGSTR(ONE_BETWEEN_0_AND_1,\
"Must be either 0 or 1.")

#define MSG_SPECIFY_DTYPE 		MSGSTR(SPECIFY_DTYPE,\
"\"%s\": can't read disk label; disk type must be specified:\n")

#define MSG_DISKTYPE_NOTUSED            MSGSTR(DISKTYPE_NOTUSED,\
"Supplied disk type not used.  Disk label or driver provided device sizing.\n")

#define MSG_UNKNOWN_DPART		MSGSTR(UNKNOWN_DPART,\
"\"%s\": can't figure out disk partition.\n")

#define MSG_UNAVAIL_DPART		MSGSTR(UNAVAIL_DPART,\
"\"%s\": `%c' partition is unavailable.\n")

#define MSG_NOT_CHARDEV			MSGSTR(NOT_CHARDEV,\
"\"%s\": not a character device.\n")

#define MSG_NOT_LV			MSGSTR(NOT_LV,\
"\"%s\": not a logical volume.\n")

#define MSG_WRITE_LVMREC		MSGSTR(WRITE_LVMREC,\
"writing LVM record")

#define MSG_CLEAR_BBDIR 		MSGSTR(CLEAR_BBDIR,\
"clearing the bad block directory")

#define MSG_TOOMANY_BBLOCKS 		MSGSTR(TOOMANY_BBLOCKS,\
"too many bad blocks.\n")

#define MSG_WRITE_BBDIR 		MSGSTR(WRITE_BBDIR,\
"writing the bad block directory")

#define MSG_WRITE_DEF_ENTRIES		MSGSTR(WRITE_DEF_ENTRIES,\
"writing the DEFECT01 entries")

#define MSG_SUPPLY_MINUS_A  		MSGSTR(SUPPLY_MINUS_A,\
"must be supplied.")

#define MSG_SUPPLY_MINUS_X  		MSGSTR(SUPPLY_MINUS_X,\
"must be supplied.")

#define MSG_SUPPLY_BLOCK_DEV	  	MSGSTR(SUPPLY_BLOCK_DEV,\
"must be a block special file.")

#define MSG_STATUS_ON 			MSGSTR(STATUS_ON,\
"\"-p\" and \"-s\" can be used only with \"-a y\".\n")

#define	MSG_VG_ID_READ_ERROR		MSGSTR(VG_ID_READ_ERROR,\
"Couldn't read the internal id of volume group \"%s\" from \"%s\".\n")

#define MSG_POWER_OF_2			MSGSTR(POWER_OF_2,\
"\"PhysicalExtentSize\" must be a power of 2 between 1 and 256.")

#define MSG_BETWEEN_1_AND_255   	MSGSTR(BETWEEN_1_AND_255,\
"Must be a value between 1 and 255.")

#define MSG_VG_IN_FS			MSGSTR(VG_IN_FS,\
"Volume group \"%s\" does not exist in the \"%s\" file\n\
but it exists in the file-system. Cannot proceed.\n")

#define MSG_VG_IN_LVMTAB 		MSGSTR(VG_IN_LVMTAB,\
"Volume group \"%s\" already exists in the \"%s\" file.\n")

#define	MSG_LVM_CREATEVG_FAILED 	MSGSTR(LVM_CREATEVG_FAILED,\
"Volume group \"%s\" could not be created:\n")

#define	MSG_LVMTAB_ERROR 		MSGSTR(LVMTAB_ERROR,\
"Error when writing \"%s\" and \"%s\" to \"%s\".\n")

#define MSG_LVMTAB_READ_ERROR		MSGSTR(LVMTAB_READ_ERROR,\
"\"%s\" could not be read into memory.\n")

#define	MSG_PV_NOT_DEL_BY_LVDD 		MSGSTR(PV_NOT_DEL_BY_LVDD,\
"Physical volume \"%s\" could not be deleted by the LVM device driver.\n\
There is now an inconsistency between the information in the file\n\
system and the \"%s\" file for the volume group \"%s\":\n")

#define	MSG_NO_QUERYVG 			MSGSTR(NO_QUERYVG,\
"Couldn't query the state of the volume group \"%s\":\n")

#define MSG_CANNOT_OPEN_GRP_FILE 	MSGSTR(CANNOT_OPEN_GRP_FILE,\
"Cannot open the control file \"%s\":\n")

#define MSG_VG_NOT_ACTIVE 		MSGSTR(VG_NOT_ACTIVE,\
"The volume group \"%s\" is not active. Only an active\n\
volume group can be extended.\n")

#define	MSG_PV_MISSING 			MSGSTR(PV_MISSING,\
"Physical volume \"%s\" could not be removed since it is\n\
either missing or not attached.\n")

#define	MSG_PX_ALLOCATED		MSGSTR(PX_ALLOCATED,\
"Physical volume \"%s\" could not be removed since some of its\n\
physical extents are still in use.\n")

#define	MSG_PV_NOT_DELETED 		MSGSTR(PV_NOT_DELETD,\
"Physical volume \"%s\" could not be deleted from the \"%s\" file.\n\
There is now an inconsistency between the LVM device driver\n\
and the \"%s\" file.\n")

#define MSG_CREATE_LVMTAB		MSGSTR(CREATE_LVMTAB,\
"Creating \"%s\".\n")

#define MSG_REMOVE_FILE			MSGSTR(REMOVE_FILE,\
"Removing \"%s\"")

#define MSG_RESTORE_FILE		MSGSTR(RESTORE_FILE,\
"Restoring \"%s\"")

#define MSG_SAVE_FILE			MSGSTR(SAVE_FILE,\
"Saving \"%s\" to \"%s\"")

#define MSG_MUST_BE_NUMBER     		MSGSTR(MUST_BE_NUMBER,\
"Must be a number.")

#define MSG_BAD_VALUE_SUPPLIED 		MSGSTR(BAD_VALUE_SUPPLIED,\
"Bad value supplied. Legal values are \"%s\".")

#define MSG_PROG_ERROR  		MSGSTR(PROG_ERROR,\
"Program error. Bad usage of options handling routines.")

#define MSG_OPT_USED_TWICE     		MSGSTR(OPT_USED_TWICE,\
"Option used more than once.")

#define MSG_ILLEGAL_OPT        		MSGSTR(ILLEGAL_OPT,\
"Illegal option.")

#define MSG_NOT_ALONE 			MSGSTR(NOT_ALONE,\
"Options with value must not be mixed with other options.")

#define MSG_VALUE_REQ 			MSGSTR(VALUE_REQ,\
"A value after option must be supplied.")

#define MSG_MORE_ARGS          		MSGSTR(MORE_ARGS,\
"More arguments required.")

#define MSG_NO_CHAR            		MSGSTR(NO_CHAR,\
"No character after '-'.")

#define MSG_PV_NOT_ADDED		MSGSTR(PV_NOT_ADDED,\
"Physical volume \"%s\" could not be added to \"%s\" and\n\
therefore was not added to the volume group.\n")

#define	MSG_TO_MANY_PVS			MSGSTR(TO_MANY_PVS,\
"Too many physical volumes in PhysicalVolumePath. At least\
\none physical volume must  stay in the volume group.\n")

#define MSG_PV_EXISTS 			MSGSTR(PV_EXISTS,\
"The physical volume \"%s\" is already recorded in the \"%s\" file.\n")

#define MSG_NOPV			MSGSTR(NOPV,\
"Physical volume \"%s\" is not a block special file.\n")

#define MSG_LVM_INSTALLPV_FAILED	MSGSTR(LVM_INSTALLPV_FAILED,\
"Couldn't install the physical volume \"%s\".\n")

#define MSG_PV_INCONSISTENT		MSGSTR(PV_INCONSISTENT,\
"Physical volume \"%s\" was made known to the LVM device driver,\n\
but it could not be added to \"%s\".\n\
The LVM is not in a consistent state.\n")

#define MSG_VG_NOT_IN_LVMTAB		MSGSTR(VG_NOT_IN_LVMTAB,\
"Volume group \"%s\" does not exist in the \"%s\" file.\n")

#define MSG_NO_CLEAN_PATH		MSGSTR(NO_CLEAN_PATH,\
"Illegal path \"%s\".\n")

#define MSG_OPENDIR			MSGSTR(OPENDIR,\
"Couldn't open directory \"%s\":")

#define MSG_STATPV			MSGSTR(STATPV,\
"Couldn't stat physical volume \"%s\":\n")

#define MSG_OPENPV			MSGSTR(OPENPV,\
"Couldn't open physical volume \"%s\":\n")

#define MSG_QUERYPVPATH_FAILED		MSGSTR(QUERYPVPATH_FAILED,\
"Couldn't query physical volume \"%s\":\n")

#define MSG_WARN_QUERYPVPATH_FAILED	MSGSTR(WARN_QUERYPVPATH_FAILED,\
"Warning: couldn't query physical volume \"%s\":\n")

#define MSG_DELETEPV_FAILED		MSGSTR(DELETEPV_FAILED,\
"Couldn't delete physical volume:\n")

#define MSG_DELETELV_FAILED		MSGSTR(DELETELV_FAILED,\
"Couldn't delete logical volume \"%s\":\n")

#define MSG_QUERYPV_FAILED		MSGSTR(QUERYPV_FAILED,\
"Couldn't query physical volume:\n")

#define MSG_QUERYPVMAP_FAILED		MSGSTR(QUERYPVMAP_FAILED,\
"Couldn't query the allocation map of the physical volume:\n")

#define MSG_QUERYLVMAP_FAILED		MSGSTR(QUERYLVMAP_FAILED,\
"Couldn't query the allocation map of the logical volume:\n")

#define MSG_SETVGID_FAILED		MSGSTR(SETVGID_FAILED,\
"Couldn't set the unique id for volume group \"%s\":\n")

#define MSG_DEACTIVATEVG_FAILED		MSGSTR(DEACTIVATEVG_FAILED,\
"Couldn't deactivate volume group \"%s\":\n")

#define MSG_ACTIVATEVG_FAILED		MSGSTR(ACTIVATEVG_FAILED,\
"Couldn't activate volume group \"%s\":\n")

#define MSG_QUERYVG_FAILED		MSGSTR(QUERYVG_FAILED,\
"Couldn't query volume group \"%s\":\n")

#define MSG_QUERYLV_FAILED_NONAME	MSGSTR(QUERYLV_FAILED_NONAME,\
"Couldn't query the logical volume:\n")

#define MSG_QUERYLV_FAILED		MSGSTR(QUERYLV_FAILED,\
"Couldn't query logical volume \"%s\":\n")

#define MSG_REDUCELV_FAILED		MSGSTR(REDUCELV_FAILED,\
"Couldn't reduce the logical volume:\n")

#define MSG_ATTACHPV_FAILED		MSGSTR(ATTACHPV_FAILED,\
"Warning: Couldn't attach to the volume group physical volume \"%s\":\n")

#define MSG_EXTENDLV_FAILED		MSGSTR(EXTENDLV_FAILED,\
"Couldn't extend the logical volume:\n")

#define MSG_RESYNCLX_FAILED		MSGSTR(RESYNCLX_FAILED,\
"Couldn't update the contents of logical extent:\n")

#define MSG_RESYNCLV_FAILED		MSGSTR(RESYNCLV_FAILED,\
"Couldn't re-synchronize stale partitions of the logical volume:\n")

#define MSG_NO_PVNAMES			MSGSTR(NO_PVNAMES,\
"Couldn't access the list of physical volumes for volume group \"%s\".\n")

#define MSG_NO_LVNAMES			MSGSTR(NO_LVNAMES,\
"Couldn't access the list of logical volumes for volume group \"%s\".\n")

#define MSG_QUERYPVS_FAILED		MSGSTR(QUERYPVS_FAILED,\
"Warning: couldn't query all of the physical volumes.\n")

#define MSG_QUERYLVS_FAILED		MSGSTR(QUERYLVS_FAILED,\
"Couldn't query the list of logical volumes.\n")

#define MSG_DUMPPV_FAILED		MSGSTR(DUMPPV_FAILED,\
"Couldn't print the information about physical volume \"%s\".\n")

#define MSG_DUMPLV_FAILED		MSGSTR(DUMPLV_FAILED,\
"Couldn't print the information about logical volume \"%s\".\n")

#define MSG_PV_AVAILABLE		MSGSTR(PV_AVAILABLE,\
"available")

#define MSG_PV_UNAVAILABLE		MSGSTR(PV_UNAVAILABLE,\
"unavailable")

#define MSG_NO_VGFORPV			MSGSTR(NO_VGFORPV,\
"Couldn't find the volume group to which\n\
physical volume \"%s\" belongs.\n")

#define MSG_NO_VGFORLV			MSGSTR(NO_VGFORLV,\
"Couldn't find the volume group to which\n\
logical volume \"%s\" belongs.\n")

#define MSG_NO				MSGSTR(NO,\
"no")

#define MSG_YES				MSGSTR(YES,\
"yes")

#define MSG_NO_VGNAMES			MSGSTR(NO_VGNAMES,\
"No volume group name could be read from \"%s\".\n")

#define MSG_CANT_DUMP_VG		MSGSTR(CANT_DUMP_VG,\
"Cannot display volume group \"%s\".\n")

#define MSG_CANT_DUMP_LV		MSGSTR(CANT_DUMP_LV,\
"Cannot display logical volume \"%s\".\n")

#define MSG_CANT_DUMP_PV		MSGSTR(CANT_DUMP_PV,\
"Cannot display physical volume \"%s\".\n")

#define MSG_GETCWD			MSGSTR(GETCWD,\
"Couldn't get current working directory")

#define MSG_CANNOT_FIGURE_VG_FOR_LV	MSGSTR(CANNOT_FIGURE_VG_FOR_LV,\
"Cannot figure out the name of volume group to which\n\
logical volume \"%s\" belongs.\n")

#define	MSG_LV_NAME_NOT_GENERATED 	MSGSTR(LV_NAME_NOT_GENERATED,\
"Unable to generate a name for the logical volume.\n")

#define	MSG_GENERATED_LV_NAME 		MSGSTR(GENERATED_LV_NAME,\
"A logical volume with name \"%s\" will be created.\n")

#define	MSG_VG_CREATED			MSGSTR(VG_CREATED,\
"Volume group \"%s\" has been successfully created\n")

#define	MSG_LV_CREATED			MSGSTR(LV_CREATED,\
"Logical volume \"%s\" has been successfully created\n\
with minor number %d.\n")

#define	MSG_PV_CREATED			MSGSTR(PV_CREATED,\
"Physical volume \"%s\" has been successfully created.\n")

#define MSG_MOVING_LV			MSGSTR(MOVING_LV,\
"Transferring logical extents of logical volume \"%s\"...\n")

#define	MSG_PV_MOVED			MSGSTR(PV_MOVED,\
"Physical volume \"%s\" has been successfully moved.\n")

#define	MSG_LV_ALREADY_EXISTS 		MSGSTR(LV_ALREADY_EXISTS,\
"Logical volume \"%s\" already exists.\n")

#define	MSG_VG_NOT_READ 		MSGSTR(VG_NOT_READ,\
"The status of the volume group \"%s\" could not be read.\n\
The major number, which is needed when creating the\n\
logical volume, could therefore not be retrieved.\n")

#define	MSG_LV_NOT_CREATED 		MSGSTR(LV_NOT_CREATED,\
"The logical volume \"%s\" could not be created:\n")

#define	MSG_LV_NOT_CR_IN_FS 		MSGSTR(LV_NOT_CR_IN_FS,\
"The logical volume \"%s\" could not be created\n\
as a special file in the file-system:\n")

#define	MSG_LV_NOT_DELETED 		MSGSTR(LV_NOT_DELETED,\
"The logical volume \"%s\" could not be deleted from the\n\
LVM device driver.\n\
There is now an inconsistency between the device driver and the file-system:\n")

#define MSG_LV_RDONLY			MSGSTR(LV_RDONLY,\
"read-only")

#define MSG_LV_RDWR			MSGSTR(LV_RDWR,\
"read/write")

#define MSG_LV_OPENSTALE		MSGSTR(LV_OPENSTALE,\
"available/stale")

#define MSG_LV_OPENSYNCD		MSGSTR(LV_OPENSYNCD,\
"available/syncd")

#define MSG_LV_CLOSED			MSGSTR(LV_CLOSED,\
"unavailable")

#define MSG_LV_VERIFY			MSGSTR(LV_VERIFY,\
"on")

#define MSG_LV_NOVERIFY			MSGSTR(LV_NOVERIFY,\
"off")

#define MSG_LV_RELOC			MSGSTR(LV_RELOC,\
"on")

#define MSG_LV_NORELOC			MSGSTR(LV_NORELOC,\
"off")

#define MSG_LV_STRICT			MSGSTR(LV_STRICT,\
"strict")

#define MSG_LV_NONSTRICT		MSGSTR(LV_NONSTRICT,\
"non-strict")

#define MSG_LV_SEQUENTIAL		MSGSTR(LV_SEQUENTIAL,\
"sequential")

#define MSG_LV_PARALLEL			MSGSTR(LV_PARALLEL,\
"parallel")

#define MSG_VG_ON			MSGSTR(VG_ON,\
"available")

#define MSG_VG_OFF			MSGSTR(VG_OFF,\
"unavailable")

#define MSG_LX_STALE			MSGSTR(LX_STALE,\
"stale")

#define MSG_LX_CURRENT			MSGSTR(LX_CURRENT,\
"current")

#define MSG_LX_MISSING			MSGSTR(LX_MISSING,\
"missing")

#define MSG_PX_STALE			MSGSTR(PX_STALE,\
"stale")

#define MSG_PX_CURRENT			MSGSTR(PX_CURRENT,\
"current")

#define MSG_PX_FREE			MSGSTR(PX_FREE,\
"free")

#define	MSG_LV_PATH_WRONG		MSGSTR(LV_PATH_WRONG,\
"\"%s\" is not a logical volume.\n")

#define	MSG_CANT_GET_LV_MINOR		MSGSTR(CANT_GET_LV_MINOR,\
"Can't get minor number of logical volume \"%s\".\n")

#define	MSG_LV_NOT_QUERIED		MSGSTR(LV_NOT_QUERIED,\
"Couldn't query from the LVM device driver\n\
the current setting of the logical volume \"%s\":\n")

#define	MSG_VG_CHANGED			MSGSTR(VG_CHANGED,\
"Volume group \"%s\" has been successfully changed.\n")

#define	MSG_LV_CHANGED			MSGSTR(LV_CHANGED,\
"Logical volume \"%s\" has been successfully changed.\n")

#define	MSG_LV_NOT_CHANGED		MSGSTR(LV_NOT_CHANGED,\
"Logical volume \"%s\" could not be changed:\n")

#define	MSG_LV_NOT_CHANGED_BACK		MSGSTR(LV_NOT_CHANGED_BACK,\
"Logical volume \"%s\" could not be changed. \n\
The driver has wrong number of mirrors information:\n")

#define	MSG_PV_CHANGED			MSGSTR(PV_CHANGED,\
"Physical volume \"%s\" has been successfully changed.\n")

#define	MSG_PV_NOT_CHANGED		MSGSTR(PV_NOT_CHANGED,\
"Physical volume \"%s\" could not be changed:\n")

#define	MSG_VG_PATH_NOT_GEN		MSGSTR(VG_PATH_NOT_GEN,\
"The volume group to which the logical volume \"%s\" belongs\n\
could not be found. The logical volume path is probably wrong.\n")

#define	MSG_MIRRORS_NOT_ADDED		MSGSTR(MIRRORS_NOT_ADDED,\
"\"MirrorCopies\" is not bigger than current setting.\n")

#define	MSG_LE_NOT_ADDED		MSGSTR(LE_NOT_ADDED,\
"\"LogicalExtentsNumber\" is not bigger than current setting.\n")

#define	MSG_LVM_CANNOT_EXTEND		MSGSTR(LVM_CANNOT_EXTEND,\
"\"LogicalExtentsNumber\" is bigger than the maximum value allowed.\n")

#define	MSG_VG_EXTENDED			MSGSTR(VG_EXTENDED,\
"Volume group \"%s\" has been successfully extended.\n")

#define	MSG_LV_EXTENDED			MSGSTR(LV_EXTENDED,\
"Logical volume \"%s\" has been successfully extended.\n")

#define	MSG_VG_REDUCED			MSGSTR(VG_REDUCED,\
"Volume group \"%s\" has been successfully reduced.\n")

#define	MSG_LV_REDUCED			MSGSTR(LV_REDUCED,\
"Logical volume \"%s\" has been successfully reduced.\n")

#define MSG_LV_NOT_REDUCED		MSGSTR(LV_NOT_REDUCED,\
"Logical volume \"%s\" is not reduced.\n")

#define	MSG_VG_REMOVED			MSGSTR(VG_REMOVED,\
"Volume group \"%s\" has been successfully removed.\n")

#define	MSG_LV_REMOVED			MSGSTR(LV_REMOVED,\
"Logical volume \"%s\" has been successfully removed.\n")

#define	MSG_LV_MAP_NOT_READ		MSGSTR(LV_MAP_NOT_READ,\
"Couldn't retrieve the allocation map of\n\
logical volume \"%s\".\n")

#define MSG_PV_MAP_NOT_READ		MSGSTR(PV_MAP_NOT_READ,\
"Couldn't retrieve the allocation map of\n\
physical volume \"%s\".\n")

#define	MSG_PV_NAMES_NOT_READ		MSGSTR(PV_NAMES_NOT_READ,\
"Couldn't retrieve the list of the physical volumes\n\
belonging to volume group \"%s\".\n")

#define MSG_PV_NOT_IN_VG		MSGSTR(PV_NOT_IN_VG,\
"Physical volume \"%s\" does not belong\n\
to volume group \"%s\".\n")

#define MSG_NOT_ENOUGH_FREE_PX		MSGSTR(NOT_ENOUGH_FREE_PX,\
"Not enough free physical extents available.\n\
Logical volume \"%s\" could not be extended.\n")

#define MSG_FAILURE_CLUE		MSGSTR(FAILURE_CLUE,\
"Failure possibly caused by strict allocation policy\n")

#define MSG_LVDD_COULD_NOT_EXTEND	MSGSTR(LVDD_COULD_NOT_EXTEND,\
"The LVM device driver could not extend the\n\
logical volume \"%s\".\n")

#define MSG_MIRR_NOT_ADDED		MSGSTR(MIRR_NOT_ADDED,\
"It is not possible to add two new mirrors to one physical\n\
volume when the allocation policy is STRICT.\n")

#define MSG_MIRRORS_NOT_REMOVED		MSGSTR(MIRRORS_NOT_REMOVED,\
"\"MirrorCopies\" is not smaller than current setting;\n\
therefore no mirrors are removed.\n")

#define MSG_LVM_CANNOT_REDUCE		MSGSTR(LVM_CANNOT_REDUCE,\
"\"LogicalExtentNumber\" is not smaller than current setting;\n\
therefore no logical extents are removed.\n")

#define MSG_USER_CONFIRMATION		MSGSTR(USER_CONFIRMATION,\
"When a logical volume is reduced useful data might get lost;\n\
do you really want the command to proceed (y/n) : ")

#define MSG_USER2_CONFIRMATION		MSGSTR(USER2_CONFIRMATION,\
"The logical volume \"%s\" is not empty;\n\
do you really want to delete the logical volume (y/n) : ")

#define MSG_USER3_CONFIRMATION 		MSGSTR(USER3_CONFIRMATION,\
"The physical volume has a file system on it.\nDo you like to proceed (y/n) : ")

#define MSG_VG_ID_ON_PV			MSGSTR(VG_ID_ON_PV,\
"The physical volume already belongs to a volume group\n")

#define MSG_REDUCE_LX_FAILED		MSGSTR(REDUCE_LX_FAILED,\
"The LVM device driver failed to reduce\n\
the logical volume \"%s\".\n")

#define MSG_REDUCE_MIRRORS_FAILED	MSGSTR(REDUCE_MIRRORS_FAILED,\
"The LVM device driver failed to reduce mirrors on\n\
the logical volume \"%s\".\n")

#define MSG_LV_NOT_DISABLED		MSGSTR(LV_NOT_DISABLED,\
"Logical volume \"%s\" can not be removed since it is active.\n")

#define MSG_LV_NOT_RM_FROM_FS		MSGSTR(LV_NOT_RM_FROM_FS,\
"Couldn't remove the special file corresponding to\n\
Logical volume \"%s\" from the file-system.\n\
Since the logical volume is not known to the LVM device driver\n\
any more, it should be removed using the usual Unix commands\n\
(chmod(1), rm(1)).\n")

#define MSG_CANT_GET_PV_NAMES		MSGSTR(CANT_GET_PV_NAMES,\
"Couldn't retrieve the names of the physical volumes\n\
belonging to volume group \"%s\".\n")

#define MSG_CANT_GET_LV_NAMES		MSGSTR(CANT_GET_LV_NAMES,\
"Couldn't retrieve the names of the logical volumes\n\
belonging to volume group \"%s\".\n")

#define MSG_CANT_ALLOC_ON_PV		MSGSTR(CANT_ALLOC_ON_PV,\
"Allocation is not allowed on physical volume \"%s\"\n.")

#define MSG_SRC_IN_DEST_SET		MSGSTR(SRC_IN_DEST_SET,\
"The source physical volume is also in the set of\n\
destination physical volume(s).\n")

#define MSG_NOLV_IN_VG			MSGSTR(NOLV_IN_VG,\
"Volume group \"%s\" does not contain any logical volume.\n")

#define MSG_LV_NOT_IN_VG		MSGSTR(LV_NOT_IN_VG,\
"Logical volume \"%s\" does not belong to\n\
volume group \"%s\".\n")

#define MSG_CANT_OPEN_VG		MSGSTR(CANT_OPEN_VG,\
"Cannot open volume group \"%s\".\n")

#define MSG_NO_PE_MOVE			MSGSTR(NO_PE_MOVE,\
"Cannot find a free physical extent for logical extent %d\n\
of logical volume \"%s\".\n")

#define MSG_WARN_PV_MISSING		MSGSTR(WARN_PV_MISSING,\
"Warning: physical volume \"%s\" is missing.\n")

#define MSG_CANT_REMOVE_VG	MSGSTR(CANT_REMOVE_VG,\
"Couldn't remove volume group \"%s\".\n")

#define MSG_VG_STILL_ON	MSGSTR(VG_STILL_ON,\
"Volume group \"%s\" is still active.\n")

#define MSG_VG_STILL_HAS_LV	MSGSTR(VG_STILL_HAS_LV,\
"Volume group \"%s\" still contains some logical volume.\n")

#define MSG_VG_STILL_HAS_PV	MSGSTR(VG_STILL_HAS_PV,\
"Volume group \"%s\" still contains more than one physical volume.\n")

#define MSG_RM_SUBTREE_FAILED	MSGSTR(RM_SUBTREE_FAILED,\
"Couldn't remove file-system subtree \"%s\".\n")

#define MSG_DEL_VG_FROM_LVMTAB	MSGSTR(DEL_VG_FROM_LVMTAB,\
"Couldn't remove the entry \"%s\" from \"%s\".\n")

#define MSG_RESYNCED_VG		MSGSTR(RESYNCED_VG,\
"Resynchronized volume group \"%s\".\n")

#define MSG_RESYNCED_LV		MSGSTR(RESYNCED_LV,\
"Resynchronized logical volume \"%s\".\n")

#define MSG_CANT_SYNC_VG	MSGSTR(CANT_SYNC_VG,\
"Couldn't resynchronize volume group \"%s\".\n")

#define MSG_CANT_SYNC_LV	MSGSTR(CANT_SYNC_LV,\
"Couldn't resynchronize logical volume \"%s\".\n")

#define MSG_ACTIVATEVG_ENODEV	MSGSTR(ACTIVATEVG_ENODEV,\
"Either no physical volumes are attached or no valid VGDAs were found\n\
on the physical volumes.")

#define MSG_ACTIVATEVG_ENOMEM	MSGSTR(ACTIVATEVG_ENOMEM,\
"Insufficient kernel memory to complete request.")

#define MSG_ACTIVATEVG_EIO	MSGSTR(ACTIVATEVG_EIO,\
"I/O error while reading the VGDA.")

#define MSG_ACTIVATEVG_ENOENT	MSGSTR(ACTIVATEVG_ENOENT,\
"Quorum not present, or some physical volume(s) are missing.")

#define MSG_ATTACHPV_ENODEV	MSGSTR(ATTACHPV_ENODEV,\
"A component of the path of the physical volume does not exist.")

#define MSG_ATTACHPV_ENXIO	MSGSTR(ATTACHPV_ENXIO,\
"The path of the physical volume refers to a device that does not\n\
exist, or is not configured into the kernel.")

#define MSG_ATTACHPV_ENOTTY	MSGSTR(ATTACHPV_ENOTTY,\
"Inappropriate ioctl for device - the command was attempted on a\n\
logical volume device rather than the control device.")

#define MSG_CHANGELV_ENODEV	MSGSTR(CHANGELV_ENODEV,\
"The supplied minor number refers to a non-existent logical volume.")

#define MSG_CHANGELV_EROFS	MSGSTR(CHANGELV_EROFS,\
"Volume group not activated.")

#define MSG_CHANGELV_EBUSY	MSGSTR(CHANGELV_EBUSY,\
"The reduction request has not been preceded by a proper deallocation.")

#define MSG_CREATELV_EEXIST	MSGSTR(CREATELV_EEXIST,\
"The supplied minor number refers to an already-existent logical volume.")

#define MSG_CREATEVG_ENODEV	MSGSTR(CREATEVG_ENODEV,\
"The path does not specify a valid physical volume.")

#define MSG_CREATEVG_EIO	MSGSTR(CREATEVG_EIO,\
"Unable to read the physical volume.")

#define MSG_CREATEVG_ENXIO	MSGSTR(CREATEVG_ENXIO,\
"The physical volume has no driver configured.")

#define MSG_CREATEVG_ENOSPC	MSGSTR(CREATEVG_ENOSPC,\
"Insufficient space on the volume for the VGRA.")

#define MSG_CREATEVG_EPERM	MSGSTR(CREATEVG_EPERM,\
"Permission denied on open of path of physical volume.")

#define MSG_CREATEVG_ENOTBLK	MSGSTR(CREATEVG_ENOTBLK,\
"The path of the physical volume does not designate a block device.")

#define MSG_DELETELV_ENODEV	MSGSTR(DELETELV_ENODEV,\
"The supplied minor number refers to a non-existent logical volume.")

#define MSG_DELETELV_EBUSY	MSGSTR(DELETELV_EBUSY,\
"The specified logical volume is open.")

#define MSG_EXTENDLV_ENODEV	MSGSTR(EXTENDLV_ENODEV,\
"The specified logical volume does not exist.")

#define MSG_EXTENDLV_EBUSY	MSGSTR(EXTENDLV_EBUSY,\
"A physical extent described by the extent array is already in use.")

#define MSG_INSTALLPV_ENXIO	MSGSTR(INSTALLPV_ENXIO,\
"The physical volume has no driver configured.")

#define MSG_INSTALLPV_EPERM	MSGSTR(INSTALLPV_EPERM,\
"Write permission denied on the device.")

#define MSG_INSTALLPV_ENOTBLK	MSGSTR(INSTALLPV_ENOTBLK,\
"The path designates a file that is not a block device.")

#define MSG_INSTALLPV_EACCES	MSGSTR(INSTALLPV_EACCES,\
"A component of the path was not accessible.")

#define MSG_QUERYLV_ENXIO	MSGSTR(QUERYLV_ENXIO,\
"Volume group not activated.")

#define MSG_NOT_ALL_PV_AVAIL	MSGSTR(NOT_ALL_PV_AVAIL,\
"Could not attach to volume group \"%s\" all of\nits physical volumes.\n")

#define MSG_DEATTACHPV_FAILED	MSGSTR(DEATTACHPV_FAILED,\
"Warning: Couldn't detach the physical volume \"%s\" \nwith the key \"%d\" \
from the volume group.\n")

#define MSG_FILE_NOT_DELETED	MSGSTR(FILE_NOT_DELETED,\
"Couldn't remove file \"%s\":")

#define MSG_DIR_NOT_DELETED	MSGSTR(DIR_NOT_DELETED,\
"Couldn't remove directory \"%s\":")

#define MSG_PP_VG_NAME		MSGSTR(PP_VG_NAME,\
"VG Name")

#define MSG_PP_VG_STATUS	MSGSTR(PP_VG_STATUS,\
"VG Status")

#define MSG_PP_VG_MAX_LV	MSGSTR(PP_VG_MAX_LV,\
"Max LV")

#define MSG_PP_VG_CUR_LV	MSGSTR(PP_VG_CUR_LV,\
"Cur LV")

#define MSG_PP_VG_OPEN_LV	MSGSTR(PP_VG_OPEN_LV,\
"Open LV")

#define MSG_PP_VG_MAX_PV	MSGSTR(PP_VG_MAX_PV,\
"Max PV")

#define MSG_PP_VG_CUR_PV	MSGSTR(PP_VG_CUR_PV,\
"Cur PV")

#define MSG_PP_VG_ACT_PV	MSGSTR(PP_VG_ACT_PV,\
"Act PV")

#define MSG_PP_VG_PX_SIZE	MSGSTR(PP_VG_PX_SIZE,\
"PE Size")

#define MSG_PP_VG_MAX_PX_PER_PV	MSGSTR(PP_VG_MAX_PX_PER_PV,\
"Max PE per PV")

#define MSG_PP_VG_PX_CNT	MSGSTR(PP_VG_PX_CNT,\
"Total PE")

#define MSG_PP_VG_USED_PX	MSGSTR(PP_VG_USED_PX,\
"Alloc PE")

#define MSG_PP_VG_FREE_PX	MSGSTR(PP_VG_FREE_PX,\
"Free PE")

#define MSG_PP_VG_VGDA_CNT	MSGSTR(PP_VG_VGDA_CNT,\
"VGDA")

#define MSG_PP_LV_NAME		MSGSTR(PP_LV_NAME,\
"LV Name")

#define MSG_PP_LV_VGNAME	MSGSTR(PP_LV_VGNAME,\
"VG Name")

#define MSG_PP_LV_PERM		MSGSTR(PP_LV_PERM,\
"LV Permission")

#define MSG_PP_LV_STATUS	MSGSTR(PP_LV_STATUS,\
"LV Status")

#define MSG_PP_LV_WRITE_VER	MSGSTR(PP_LV_WRITE_VER,\
"Write verify")

#define MSG_PP_LV_MIRRORS	MSGSTR(PP_LV_MIRRORS,\
"Mirror copies")

#define MSG_PP_LV_SCHED		MSGSTR(PP_LV_SCHED,\
"Schedule")

#define MSG_PP_LV_LX_CNT	MSGSTR(PP_LV_LX_CNT,\
"Current LE")

#define MSG_PP_LV_USED_PX	MSGSTR(PP_LV_USED_PX,\
"Allocated PE")

#define MSG_PP_LV_BBLOCK_POL	MSGSTR(PP_LV_BBLOCK_POL,\
"Bad block")

#define MSG_PP_LV_ALLOC		MSGSTR(PP_LV_ALLOC,\
"Allocation")

#define MSG_PP_LV_USED_PV	MSGSTR(PP_LV_USED_PV,\
"Used PV")

#define MSG_PP_PV_NAME		MSGSTR(PP_PV_NAME,\
"PV Name")

#define MSG_PP_PV_VGNAME	MSGSTR(PP_PV_VGNAME,\
"VG Name")

#define MSG_PP_PV_STATUS	MSGSTR(PP_PV_STATUS,\
"PV Status")

#define MSG_PP_PV_ALLOC		MSGSTR(PP_PV_ALLOC,\
"Allocatable")

#define MSG_PP_PV_VGDA_CNT	MSGSTR(PP_PV_VGDA_CNT,\
"VGDA")

#define MSG_PP_PV_CUR_LV	MSGSTR(PP_PV_CUR_LV,\
"Cur LV")

#define MSG_PP_PV_PX_SIZE	MSGSTR(PP_PV_PX_SIZE,\
"PE Size")

#define MSG_PP_PV_PX_CNT	MSGSTR(PP_PV_PX_CNT,\
"Total PE")

#define MSG_PP_PV_FREE_PX	MSGSTR(PP_PV_FREE_PX,\
"Free PE")

#define MSG_PP_PV_USED_PX	MSGSTR(PP_PV_USED_PX,\
"Allocated PE")

#define MSG_PP_PV_STALE_PX	MSGSTR(PP_PV_STALE_PX,\
"Stale PE")

#define MSG_PP_LX_ID		MSGSTR(PP_LX_ID,\
"LE")

#define MSG_PP_LX_PV1		MSGSTR(PP_LX_PV1,\
"PV1")

#define MSG_PP_LX_PX1		MSGSTR(PP_LX_PX1,\
"PE1")

#define MSG_PP_LX_STAT1		MSGSTR(PP_LX_STAT1,\
"Status 1")

#define MSG_PP_LX_PV2		MSGSTR(PP_LX_PV2,\
"PV2")

#define MSG_PP_LX_PX2		MSGSTR(PP_LX_PX2,\
"PE2")

#define MSG_PP_LX_STAT2		MSGSTR(PP_LX_STAT2,\
"Status 2")

#define MSG_PP_LX_PV3		MSGSTR(PP_LX_PV3,\
"PV3")

#define MSG_PP_LX_PX3		MSGSTR(PP_LX_PX3,\
"PE3")

#define MSG_PP_LX_STAT3		MSGSTR(PP_LX_STAT3,\
"Status 3")

#define MSG_PP_PX_ID		MSGSTR(PP_PX_ID,\
"PE")

#define MSG_PP_PX_STAT		MSGSTR(PP_PX_STAT,\
"Status")

#define MSG_PP_PX_LV		MSGSTR(PP_PX_LV,\
"LV")

#define MSG_PP_PX_LX		MSGSTR(PP_PX_LX,\
"LE")

#define MSG_PP_LVDISTR_PV	MSGSTR(PP_LVDISTR_PV,\
"PV Name")

#define MSG_PP_LVDISTR_LX	MSGSTR(PP_LVDISTR_LX,\
"LE on PV")

#define MSG_PP_LVDISTR_PX	MSGSTR(PP_LVDISTR_PX,\
"PE on PV")

#define MSG_PP_PVDISTR_LV	MSGSTR(PP_PVDISTR_LV,\
"LV Name")

#define MSG_PP_PVDISTR_LX	MSGSTR(PP_PVDISTR_LX,\
"LE of LV")

#define MSG_PP_PVDISTR_PX	MSGSTR(PP_PVDISTR_PX,\
"PE for LV")

#define MSG_PVG_GROUP		MSGSTR(PVG_GROUP,\
"Volume groups")

#define MSG_PLV_GROUP		MSGSTR(PLV_GROUP,\
"Logical volumes")

#define MSG_PPV_GROUP		MSGSTR(PPV_GROUP,\
"Physical volumes")

#define MSG_PLX_GROUP		MSGSTR(PLX_GROUP,\
"Logical extents")

#define MSG_PPX_GROUP		MSGSTR(PPX_GROUP,\
"Physical extents")

#define MSG_PLVDISTR_GROUP	MSGSTR(PLVDISTR_GROUP,\
"Distribution of logical volume")

#define MSG_PPVDISTR_GROUP	MSGSTR(PPVDISTR_GROUP,\
"Distribution of physical volume")

#define MSG_QUERYPVPATH_ENODEV	MSGSTR(QUERYPVPATH_ENODEV,\
"The specified path does not correspond to physical volume attached to \n\
this volume group")

#define MSG_QUERYPVPATH_ENOXIO	MSGSTR(QUERYPVPATH_ENOXIO,\
"The volume group is not activated")

#define MSG_BAD_INPUT_PARAMETER	MSGSTR(BAD_INPUT_PARAMETER,\
"Bad input parameter\n")

#define MSG_NO_PE_ALLOCATED	MSGSTR(NO_PE_ALLOCATED,\
"The number of mirrors can not be extended since \
the logical \nvolume do not have any logical extents allocated to it.\n")

#define MSG_NOALLOCPV_WARNING	MSGSTR(NOALLOCPV_WARNING,\
"Warning: The physical volume \"%s\" is not \navailable for allocation.\n")

#define MSG_SYNC_TAKES_TIME	MSGSTR(SYNC_TAKES_TIME,\
"The newly allocated mirrors is now being synchronized. This operation will \n\
take some time. Please wait ....\n")
