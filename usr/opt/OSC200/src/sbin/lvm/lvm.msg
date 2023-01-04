$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$set MS_LVM
$quote "

NOT_A_PATH_NAME		"Must be a simple file name, not a path name."

BETWEEN_0_AND_MAXLXS 	"Must be a value between 0 and 65535."

BETWEEN_1_AND_MAXLXS 	"Must be a value between 1 and 65535."

BETWEEN_1_AND_MAXPXS 	"Must be a value between 1 and 65535."

ONE_OF_THEM_REQUIRED	"Exactly one of them has to be specified."

ONE_OF_THEM		"At least one of them has to be specified."

BETWEEN_1_AND_2		"Must be either 1 or 2."

GRATER_THAN_0		"Must be greater than 0."

MINUS_L_REQUIRED	"Can be used only if \"-l\" has been specified."

EMPTY_LV_WITH_MIRRORS	"Can't create a mirrored empty logical volume. \
Specify \"-l\"."

ONE_BETWEEN_0_AND_1	"Must be either 0 or 1."

SPECIFY_DTYPE		"\"%s\": can't read disk label; disk type must be \
specified:\n"

DISKTYPE_NOTUSED        "Supplied disk type not used.  Disk label or \
driver provided device sizing.\n"

UNKNOWN_DPART		"\"%s\": can't figure out disk partition.\n"

UNAVAIL_DPART		"\"%s\": `%c' partition is unavailable.\n"

NOT_CHARDEV		"\"%s\": not a character device.\n"

NOT_LV			"\"%s\": not a logical volume.\n"

WRITE_LVMREC		"writing LVM record"

CLEAR_BBDIR		"clearing the bad block directory"

TOOMANY_BBLOCKS		"too many bad blocks.\n"

WRITE_BBDIR		"writing the bad block directory"

WRITE_DEF_ENTRIES	"writing the DEFECT01 entries"

SUPPLY_MINUS_A		"must be supplied."

SUPPLY_MINUS_X		"must be supplied."

SUPPLY_BLOCK_DEV	"must be a block special file."

STATUS_ON		"\"-p\" and \"-s\" can be used only with \"-a y\".\n"

VG_ID_READ_ERROR 	"Couldn't read the internal id of volume group \"%s\" \
from \"%s\".\n"

POWER_OF_2 		"\"PhysicalExtentSize\" must be a power of 2 between \
1 and 256."

BETWEEN_1_AND_255 	"Must be a value between 1 and 255."

VG_IN_FS		"Volume group \"%s\" does not exist in the \"%s\" \
file\n but it exists in the file-system. Cannot proceed.\n"

VG_IN_LVMTAB 		"Volume group \"%s\" already exists in the \"%s\" \
file.\n"

LVM_CREATEVG_FAILED 	"Volume group \"%s\" could not be created:\n"

LVMTAB_ERROR 		"Error when writing \"%s\" and \"%s\" to \"%s\".\n"

LVMTAB_READ_ERROR 	"\"%s\" could not be read into memory.\n"

PV_NOT_DEL_BY_LVDD 	"Physical volume \"%s\" could not be deleted by the \
LVM device driver.\n There is now an inconsistency between the information \
in the file\n system and the \"%s\" file for the volume group \"%s\":\n"

NO_QUERYVG 		"Couldn't query the state of the volume group \"%s\":\n"

VG_NOT_ACTIVE 		"The volume group \"%s\" is not active. Only an active\
\n volume group can be extended.\n"

PV_MISSING 		"Physical volume \"%s\" could not be removed since it \
is\n either missing or not attached.\n"

PX_ALLOCATED 		"Physical volume \"%s\" could not be removed since \
some of its\n physical extents are still in use.\n"

PV_NOT_DELETD 		"Physical volume \"%s\" could not be deleted from the \
\"%s\" file.\n There is now an inconsistency between the LVM device driver\n\
and the \"%s\" file.\n"

CREATE_LVMTAB 		"Creating \"%s\".\n"

REMOVE_FILE 		"Removing \"%s\""

RESTORE_FILE 		"Restoring \"%s\""

SAVE_FILE 		"Saving \"%s\" to \"%s\""

MUST_BE_NUMBER 		"Must be a number."

BAD_VALUE_SUPPLIED 	"Bad value supplied. Legal values are \"%s\"."

PROG_ERROR 		"Program error. Bad usage of options handling routines."

OPT_USED_TWICE 		"Option used more than once."

ILLEGAL_OPT 		"Illegal option."

NOT_ALONE 		"Options with value must not be mixed with other \
options."

VALUE_REQ 		"A value after option must be supplied."

MORE_ARGS 		"More arguments required."

NO_CHAR 		"No character after '-'."

PV_NOT_ADDED 		"Physical volume \"%s\" could not be added to \"%s\" \
and\n therefore was not added to the volume group.\n"

PV_EXISTS 		"The physical volume \"%s\" is already recorded in \
the \"%s\" file.\n"

NOPV 			"Physical volume \"%s\" is not a block special file.\n"

TO_MANY_PVS		"To many physical volumes in PhsyicalVolumePath. At least\
\none physical volume must  stay in the volume group.\n" 

LVM_INSTALLPV_FAILED 	"Couldn't install the physical volume \"%s\".\n"

PV_INCONSISTENT 	"Physical volume \"%s\" was made known to the LVM \
device driver,\n but it could not be added to \"%s\".\n The LVM is not in a \
consistent state.\n"

VG_NOT_IN_LVMTAB 	"Volume group \"%s\" does not exist in the \"%s\" \
file.\n"

CANNOT_OPEN_GRP_FILE    "Cannot open the control file \"%s\":\n"

NO_CLEAN_PATH 		"Illegal path \"%s\".\n"

OPENDIR 		"Couldn't open directory \"%s\":"

STATPV 			"Couldn't stat physical volume \"%s\":\n"

OPENPV 			"Couldn't open physical volume \"%s\":\n"

QUERYPVPATH_FAILED 	"Couldn't query physical volume \"%s\":\n"

WARN_QUERYPVPATH_FAILED	"Warning: couldn't query physical volume \"%s\":\n"

DELETEPV_FAILED 	"Couldn't delete physical volume:\n"

DELETELV_FAILED 	"Couldn't delete logical volume \"%s\":\n"

QUERYPV_FAILED 		"Couldn't query physical volume:\n"

QUERYPVMAP_FAILED 	"Couldn't query the allocation map of the physical \
volume:\n"

QUERYLVMAP_FAILED 	"Couldn't query the allocation map of the logical \
volume:\n"

SETVGID_FAILED 		"Couldn't set the unique id for volume group \"%s\":\n"

DEACTIVATEVG_FAILED 	"Couldn't deactivate volume group \"%s\":\n"

ACTIVATEVG_FAILED 	"Couldn't activate volume group \"%s\":\n"

QUERYVG_FAILED 		"Couldn't query volume group \"%s\":\n"

QUERYLV_FAILED_NONAME 	"Couldn't query the logical volume:\n"

QUERYLV_FAILED 		"Couldn't query logical volume \"%s\":\n"

REDUCELV_FAILED 	"Couldn't reduce the logical volume:\n"

ATTACHPV_FAILED 	"Warning: Couldn't attach to the volume group \
physical volume \"%s\":\n"

EXTENDLV_FAILED 	"Couldn't extend the logical volume:\n"

RESYNCLX_FAILED 	"Couldn't update the contents of logical extent:\n"

RESYNCLV_FAILED 	"Couldn't re-synchronize stale partitions of the \
logical volume:\n"

NO_PVNAMES 		"Couldn't access the list of physical volumes for \
volume group \"%s\".\n"

NO_LVNAMES 		"Couldn't access the list of logical volumes for \
volume group \"%s\".\n"

QUERYPVS_FAILED 	"Couldn't query the list of physical volumes.\n"

QUERYLVS_FAILED 	"Couldn't query the list of logical volumes.\n"

DUMPPV_FAILED 		"Couldn't print the information about physical volume \
\"%s\".\n"

DUMPLV_FAILED 		"Couldn't print the information about logical volume \
\"%s\".\n"

PV_AVAILABLE 		"available"

PV_UNAVAILABLE 		"unavailable"

NO_VGFORPV 		"Couldn't find the volume group to which\n physical \
volume \"%s\" belongs.\n"

NO_VGFORLV 		"Couldn't find the volume group to which\n logical \
volume \"%s\" belongs.\n"

NO 			"no"

YES 			"yes"

NO_VGNAMES 		"No volume group name could be read from \"%s\".\n"

CANT_DUMP_VG 		"Cannot display volume group \"%s\".\n"

CANT_DUMP_LV 		"Cannot display logical volume \"%s\".\n"

CANT_DUMP_PV 		"Cannot display physical volume \"%s\".\n"

GETCWD 			"Couldn't get current working directory"

CANNOT_FIGURE_VG_FOR_LV "Cannot figure out the name of volume group to which\n\
logical volume \"%s\" belongs.\n"

LV_NAME_NOT_GENERATED 	"Unable to generate a name for the logical volume.\n"

GENERATED_LV_NAME 	"A logical volume with name \"%s\" will be created.\n"

VG_CREATED 		"Volume group \"%s\" has been successfully created.\n"

LV_CREATED 		"Logical volume \"%s\" has been successfully created\n\
with minor number %d.\n"

PV_CREATED 		"Physical volume \"%s\" has been successfully created.\
\n"

MOVING_LV		"Transferring logical extents of logical volume \"%s\"\
...\n"

PV_MOVED 		"Physical volume \"%s\" has been successfully moved.\n"

LV_ALREADY_EXISTS 	"Logical volume \"%s\" already exists.\n"

VG_NOT_READ 		"The status of the volume group \"%s\" could not be \
read.\n The major number, which is needed when creating the\n logical volume, \
could therefore not be retrieved.\n"

LV_NOT_CREATED 		"The logical volume \"%s\" could not be created:\n"

LV_NOT_CR_IN_FS 	"The logical volume \"%s\" could not be created\n as \
a special file in the file-system:\n"

LV_NOT_DELETED 		"The logical volume \"%s\" could not be deleted from \
the\n LVM device driver.\n There is now an inconsistency between the device \
driver and the file-system:\n"

LV_RDONLY 		"read-only"

LV_RDWR 		"read/write"

LV_OPENSTALE 		"available/stale"

LV_OPENSYNCD 		"available/syncd"

LV_CLOSED 		"unavailable"

LV_VERIFY 		"on"

LV_NOVERIFY 		"off"

LV_RELOC 		"on"

LV_NORELOC 		"off"

LV_STRICT 		"strict"

LV_NONSTRICT 		"non-strict"

LV_SEQUENTIAL 		"sequential"

LV_PARALLEL 		"parallel"

VG_ON 			"available"

VG_OFF 			"unavailable"

LX_STALE 		"stale"

LX_CURRENT 		"current"

LX_MISSING 		"missing"

PX_STALE 		"stale"

PX_CURRENT 		"current"

PX_FREE 		"free"

LV_PATH_WRONG 		"\"%s\" is not a logical volume.\n"

CANT_GET_LV_MINOR 	"Can't get minor number of logical volume \"%s\".\n"

LV_NOT_QUERIED 		"Couldn't query from the LVM device driver\n the \
current setting of the logical volume \"%s\":\n"

VG_CHANGED 		"Volume group \"%s\" has been successfully changed.\n"

LV_CHANGED 		"Logical volume \"%s\" has been successfully changed.\n"

LV_NOT_CHANGED 		"Logical volume \"%s\" could not be changed:\n"

LV_NOT_CHANGED_BACK 	"Logical volume \"%s\" could not be changed. \n\
The driver has wrong number of mirrors information:\n"

PV_CHANGED 		"Physical volume \"%s\" has been successfully changed.\
\n"

PV_NOT_CHANGED 		"Physical volume \"%s\" could not be changed:\n"

VG_PATH_NOT_GEN 	"The volume group to which the logical volume \"%s\" \
belongs\n could not be found. The logical volume path is probably wrong.\n"

MIRRORS_NOT_ADDED 	"\"MirrorCopies\" is not bigger than current setting.\n"

LE_NOT_ADDED 		"\"LogicalExtentsNumber\" is not bigger than current \
setting.\n"

LVM_CANNOT_EXTEND 	"\"LogicalExtentsNumber\" is bigger than the maximum \
value allowed.\n"

VG_EXTENDED 		"Volume group \"%s\" has been successfully extended.\n"

LV_EXTENDED 		"Logical volume \"%s\" has been successfully extended.\
\n"

VG_REDUCED 		"Volume group \"%s\" has been successfully reduced.\n"

LV_REDUCED 		"Logical volume \"%s\" has been successfully reduced.\n"

LV_NOT_REDUCED		"Logical volume \"%s\" is not reduced.\n"

VG_REMOVED 		"Volume group \"%s\" has been successfully removed.\n"

LV_REMOVED 		"Logical volume \"%s\" has been successfully removed.\n"

LV_MAP_NOT_READ 	"Couldn't retrieve the allocation map of\n logical \
volume \"%s\".\n"

PV_MAP_NOT_READ 	"Couldn't retrieve the allocation map of\n physical \
volume \"%s\".\n"

PV_NAMES_NOT_READ 	"Couldn't retrieve the list of the physical volumes\n\
belonging to volume group \"%s\".\n"

PV_NOT_IN_VG 		"Physical volume \"%s\" does not belong\n to volume \
group \"%s\".\n"

NOT_ENOUGH_FREE_PX 	"Not enough free physical extents available.\n\
Logical volume \"%s\" could not be extended.\n"

FAILURE_CLUE		"Failure possibly caused by strict allocation policy\n"

LVDD_COULD_NOT_EXTEND 	"The LVM device driver could not extend the\n\
logical volume \"%s\".\n"

MIRR_NOT_ADDED 		"It is not possible to add two new mirrors to one \
physical\n volume when the allocation policy is STRICT.\n"

MIRRORS_NOT_REMOVED 	"\"MirrorCopies\" is not smaller than current setting;\
\n therefore no mirrors are removed.\n"

LVM_CANNOT_REDUCE 	"\"LogicalExtentNumber\" is not smaller than current \
setting;\n therefore no logical extents are removed.\n"

USER_CONFIRMATION 	"When a logical volume is reduced useful data might \
get lost;\n do you really want the command to proceed (y/n) : "

USER2_CONFIRMATION 	"The logical volume \"%s\" is not empty;\n\
do you really want to delete the logical volume (y/n) : "

USER3_CONFIRMATION 	"The physical volume has a file system on it.\nDo you like to proceed (y/n) : "

VG_ID_ON_PV		"The physical volume already belongs to a volume group\n"

REDUCE_LX_FAILED 	"The LVM device driver failed to reduce\n the logical \
volume \"%s\".\n"

REDUCE_MIRRORS_FAILED 	"The LVM device driver failed to reduce mirrors on\n\
the logical volume \"%s\".\n"

LV_NOT_DISABLED 	"Logical volume \"%s\" can not be removed since it is \
active.\n"

LV_NOT_RM_FROM_FS 	"Couldn't remove the special file corresponding to\n\
Logical volume \"%s\" from the file-system.\n Since the logical volume is not \
known to the LVM device driver\n any more, it should be removed using the \
usual Unix commands\n (chmod(1), rm(1)).\n"

CANT_GET_PV_NAMES 	"Couldn't retrieve the names of the physical volumes\n\
belonging to volume group \"%s\".\n"

CANT_GET_LV_NAMES 	"Couldn't retrieve the names of the logical volumes\n\
belonging to volume group \"%s\".\n"

CANT_ALLOC_ON_PV	"Allocation is not allowed on physical volume \"%s\"\n."

SRC_IN_DEST_SET 	"The source physical volume is also in the set of\n\
destination physical volume(s).\n"

NOLV_IN_VG 		"Volume group \"%s\" does not contain any logical \
volume.\n"

LV_NOT_IN_VG 		"Logical volume \"%s\" does not belong to\n\
volume group \"%s\".\n"

CANT_OPEN_VG 		"Cannot open volume group \"%s\".\n"

NO_PE_MOVE 		"Cannot find a free physical extent for logical \
extent %d\n of logical volume \"%s\".\n"

WARN_PV_MISSING 	"Warning: physical volume \"%s\" is missing.\n"

CANT_REMOVE_VG 		"Couldn't remove volume group \"%s\".\n"

VG_STILL_ON 		"Volume group \"%s\" is still active.\n"

VG_STILL_HAS_LV 	"Volume group \"%s\" still contains some logical \
volume.\n"

VG_STILL_HAS_PV 	"Volume group \"%s\" still contains more than one \
physical volume.\n"

RM_SUBTREE_FAILED 	"Couldn't remove file-system subtree \"%s\".\n"

DEL_VG_FROM_LVMTAB 	"Couldn't remove the entry \"%s\" from \"%s\".\n"

RESYNCED_VG 		"Resynchronised volume group \"%s\".\n"

RESYNCED_LV 		"Resynchronised logical volume \"%s\".\n"

CANT_SYNC_VG 		"Couldn't resynchronise volume group \"%s\".\n"

CANT_SYNC_LV 		"Couldn't resynchronise logical volume \"%s\".\n"

ACTIVATEVG_ENODEV	"Either no physical volumes are attached or no \
valid VGDAs were found\non the physical volumes."

ACTIVATEVG_ENOMEM	"Insufficient kernel memory to complete request."

ACTIVATEVG_EIO		"I/O error while reading the VGDA."

ACTIVATEVG_ENOENT	"Quorum not present, or some physical volume(s) are \
missing."

ATTACHPV_ENODEV		"A component of the path of the physical volume does \
not exist."

ATTACHPV_ENXIO		"The path of the physical volume refers to a device \
that does not\nexist, or is not configured into the kernel."

ATTACHPV_ENOTTY		"Inappropriate ioctl for device - the command was \
attempted on a\nlogical volume device rather than the control device."

CHANGELV_ENODEV		"The supplied minor number refers to a non-existent \
logical volume."

CHANGELV_EROFS		"Volume group not activated."

CHANGELV_EBUSY		"The reduction request has not been preceded by a \
proper deallocation."

CREATELV_EEXIST		"The supplied minor number refers to an already \
existent logical volume."

CREATEVG_ENODEV		"The path does not specify a valid physical volume."

CREATEVG_EIO		"Unable to read the physical volume."

CREATEVG_ENXIO		"The physical volume has no driver configured."

CREATEVG_ENOSPC		"Insufficient space on the volume for the VGRA."

CREATEVG_EPERM		"Permission denied on open of path of physical volume."

CREATEVG_ENOTBLK	"The path of the physical volume does not designate a \
block device."

DELETELV_ENODEV		"The supplied minor number refers to a non-existent \
logical volume."

DELETELV_EBUSY		"The specified logical volume is open."

EXTENDLV_ENODEV		"The specified logical volume does not exist."

EXTENDLV_EBUSY		"A physical extent described by the extent array is \
already in use."

INSTALLPV_ENXIO		"The physical volume has no driver configured."

INSTALLPV_EPERM		"Write permission denied on the device."

INSTALLPV_ENOTBLK	"The path designates a file that is not a block device."

INSTALLPV_EACCES	"A component of the path was not accessible."

QUERYLV_ENXIO		"Volume group not activated."

NOT_ALL_PV_AVAIL	"Could not attach to volume group \"%s\" all of\n\
its physical volumes.\n"

DEATTACHPV_FAILED	"Warning: Couldn't deattach the physical volume \n\
"%s\" with the key \"%d\" from the volume group.\n"

FILE_NOT_DELETED 	"Couldn't remove file \"%s\":"

DIR_NOT_DELETED 	"Couldn't remove directory \"%s\":"

PP_VG_NAME		"VG Name"
PP_VG_STATUS		"VG Status"
PP_VG_MAX_LV		"Max LV"
PP_VG_CUR_LV		"Cur LV"
PP_VG_OPEN_LV		"Open LV"
PP_VG_MAX_PV		"Max PV"
PP_VG_CUR_PV		"Cur PV"
PP_VG_ACT_PV		"Act PV"
PP_VG_PX_SIZE		"PE Size"
PP_VG_MAX_PX_PER_PV	"Max PE per PV"
PP_VG_PX_CNT		"Total PE"
PP_VG_USED_PX		"Alloc PE"
PP_VG_FREE_PX		"Free PE"
PP_VG_VGDA_CNT		"VGDA"

PP_LV_NAME		"LV Name"
PP_LV_VGNAME		"VG Name"
PP_LV_PERM		"LV Permission"
PP_LV_STATUS		"LV Status"
PP_LV_WRITE_VER		"Write verify"
PP_LV_MIRRORS		"Mirror copies"
PP_LV_SCHED		"Schedule"
PP_LV_LX_CNT		"Current LE"
PP_LV_USED_PX		"Allocated PE"
PP_LV_BBLOCK_POL	"Bad block"
PP_LV_ALLOC		"Allocation"
PP_LV_USED_PV		"Used PV"

PP_PV_NAME		"PV Name"
PP_PV_VGNAME		"VG Name"
PP_PV_STATUS		"PV Status"
PP_PV_ALLOC		"Allocatable"
PP_PV_VGDA_CNT		"VGDA"
PP_PV_CUR_LV		"Cur LV"
PP_PV_PX_SIZE		"PE Size"
PP_PV_PX_CNT		"Total PE"
PP_PV_FREE_PX		"Free PE"
PP_PV_USED_PX		"Allocated PE"
PP_PV_STALE_PX		"Stale PE"

PP_LX_ID			"LE"
PP_LX_PV1		"PV1"
PP_LX_PX1		"PE1"
PP_LX_STAT1		"Status 1"
PP_LX_PV2		"PV2"
PP_LX_PX2		"PE2"
PP_LX_STAT2		"Status 2"
PP_LX_PV3		"PV3"
PP_LX_PX3		"PE3"
PP_LX_STAT3		"Status 3"

PP_PX_ID			"PE"
PP_PX_STAT		"Status"
PP_PX_LV			"LV"
PP_PX_LX			"LE"

PP_LVDISTR_PV		"PV Name"
PP_LVDISTR_LX		"LE on PV"
PP_LVDISTR_PX		"PE on PV"

PP_PVDISTR_LV		"LV Name"
PP_PVDISTR_LX		"LE of LV"
PP_PVDISTR_PX		"PE for LV"

PVG_GROUP		"Volume groups"
PLV_GROUP		"Logical volumes"
PPV_GROUP		"Physical volumes"
PLX_GROUP		"Logical extents"
PPX_GROUP		"Physical extents"
PLVDISTR_GROUP		"Distribution of logical volume"
PPVDISTR_GROUP		"Distribution of physical volume"

QUERYPVPATH_ENODEV 	"The specified path does not correspond to physical \
volume attached to \n\ this volume group"

QUERYPVPATH_ENOXIO	"The volume group is not activated"

BAD_INPUT_PARAMETER	"Bad input parameter\n"

NO_PE_ALLOCATED		"The number of mirrors can not be extended since \
the logical \nvolume do not have any logical extents allocated to it.\n"

NOALLOCPV_WARNING	"Warning: The physical volume \"%s\" is not \n\
available for allocation.\n"

SYNC_TAKES_TIME		"The newly allocated mirrors is now being synchronized.\
This operation will \ntake some time. Please wait ....\n"

