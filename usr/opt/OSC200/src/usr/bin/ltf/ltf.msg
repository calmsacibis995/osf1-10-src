$ 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
$ 
$ @(#)$RCSfile: ltf.msg,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/01/08 15:03:53 $
$ 

$quote "
$set MS_LTF

MSG_ALTERN	 "Alternative pathname (return to skip extract)?"
MSG_ANSIV	 "  Volume is:  ANSI Version"

MSG_BADCENT	 "Warning > Invalid century in creation date, 20th century default used"
MSG_BADCNT1	 "Warning > For file"
MSG_BADCNT2	 "File character count ="
MSG_BADCNT3	 "!= bytes read from volume ="
MSG_BADST	 "FATAL > Bad stat call on path name ->"
MSG_BFRCNE	 "FATAL > Begining & final FUF record counts not equal ->"
MSG_BYTE	 " byte"
MSG_BYTES	 " bytes"

MSG_CANTBUF	 "FATAL > Cannot read buffer offset field of size ->"
MSG_CANTCGET	 "Warning > Cannot get status of device ->"
MSG_CANTCLS	 "FATAL > Cannot close device ->"
MSG_CANTCHD	 "FATAL > Cannot change directory to ->"
MSG_CANTCHW	 "Warning > Cannot change directory to ->"
MSG_CANTCRE	 "FATAL > Cannot create file ->"
MSG_CANTDEVIO	 "Warning > Cannot perform DEVIOCGET ioctl call on device ->"
MSG_CANTFPW	 "Warning > Cannot find user name in password file, UID = "
MSG_CANTFSF	 "FATAL > Cannot forwardspace (file) ->"
MSG_CANTL1	 "Warning > Cannot find link file for ->"
MSG_CANTLF	 "Warning > Cannot link"
MSG_CANTMKD	 "FATAL > Cannot make directory ->"
MSG_CANTOD	 "FATAL > Cannot open directory ->"
MSG_CANTOPEN	 "FATAL > Cannot open ->"
MSG_CANTOPW	 "Warning > Cannot open ->"
MSG_CANTPER	 "Warning > Can only use p key with x option"
MSG_CANTRL	 "FATAL > Cannot read label ->"
MSG_CANTRD	 "FATAL > Cannot read from ->"
MSG_CANTREW	 "FATAL > Cannot rewind. Busy or not online ->"
MSG_CANTRSL	 "Warning > Cannot read symbolic link ->"
MSG_CANTSTS	 "Warning > Cannot execute file stat call for symbolic link file of ->"
MSG_CANTSTW	 "Warning > Cannot execute file stat call for file ->"
MSG_CANTWEOF     "FATAL > Cannot write EOF on ->"
MSG_CANTWVL	 "FATAL > Cannot write VOL1 on ->"
MSG_CONFF	 "FATAL > Options c, t, and x are mutually exclusive"

MSG_DIRCRE	 " (directory created)"

MSG_ENFNAM	 "Enter FILE name, or return to quit or end input > "
MSG_EINVLD	 "FATAL > -- END INVALID LABEL DATA --"
MSG_EOFINM	 "FATAL > EOF encountered in middle of file ->"
MSG_ERRDEV	 "FATAL > Error on device"
MSG_ERREOT	 "FATAL > End of Tape (EOT) encountered on ->"
MSG_ERRUNIT	 "device unit #"
MSG_ERRWRF	 "FATAL > Error writing file ->"
MSG_EXISTS	 "Warning > File already exists ->"

MSG_FILENNG	 "Warning > Original file name cannot be reproduced on other systems"
MSG_FNTL	 "Warning > File name too long for ANSI label set ->"
MSG_FSTCB	 "Warning > First control byte in FUF is ->"
MSG_FUFTL	 "FATAL > Fortran Unformatted File record too long"

MSG_GETWDF	 "FATAL > Get working directory call (getwd) failure ->"

MSG_HELP1	 "One of the following options enclosed in {} is required\n"
MSG_HELP2	 "c = create a new volume, previous content is overwritten"
MSG_HELP3	 "H = help mode, print this summary"
MSG_HELP4	 "t = table the contents of the volume"
MSG_HELP5	 "x = extract files from the volume\n"
MSG_HELP6	 "Items enclosed in second [] are optional keys\n"
MSG_HELP7	 "a = output ANSI Version 3 format to volume"
MSG_HELP8	 "h = output file pointed to by a symbolic link instead of symbolic link file"
MSG_HELP9	 "o = omit outputting directory blocks to volume"
MSG_HELP10	 "O = omit the usage of headers 3 to 9"
MSG_HELP11	 "p = change permissions and owner of extracted files to original values,"
MSG_HELP12	 "    must be super user, used only in extraction"
MSG_HELP13	 "v = verbose mode, provide additional information about files/operation"
MSG_HELP14	 "V = big verbose mode, include directory information in table of contents"
MSG_HELP15	 "w = warn if a file name is truncated on creation or may be overwritten"
MSG_HELP16	 "    on extract"
MSG_HELP17	 "0..31 = select the unit number for the named tape device\n"
MSG_HELP18	 "Press RETURN to continue ..."
MSG_HELP19	 "Items enclosed in third [] are optional keys & require respective"
MSG_HELP20	 "arguments\n"
MSG_HELP21	 "B = set blocksize, max = 20480 bytes, default = 2048 bytes,"
MSG_HELP22	 "    min = 18 bytes, used only in creation"
MSG_HELP23	 "f = set device name, default = /dev/rmt0h"
MSG_HELP24	 "I = set input method, either by stdin or by providing a file name"
MSG_HELP25	 "L = set volume label, maximum six characters"
MSG_HELP26	 "P = set position number in form of #,# with # > 0, not used in creation"

MSG_HLINKTO	 "Info > Hard link to ->"
MSG_HOSTF	 "FATAL > Call to gethostname (HOSTNM) failed"

MSG_IMPIDC       "Warning > Implementation ID changed to ->"
MSG_IMPIDM	 " Implementation ID is: "
MSG_INTERCH	 "Interchange Name ="
MSG_INVBS	 "FATAL > Invalid block size. Min = 18 bytes, Max ="
MSG_INVOWN	 "FATAL > Non  'a'  characters in  OWNER ID  ->"
MSG_INVLD	 "FATAL > -- INVALID LABEL DATA FOLLOWS --"
MSG_INVLF	 "FATAL > Invalid label format in ->"
MSG_INVLFS	 "Warning > Skipping Unsupported Tape Label -> "
MSG_INVLFW	 "Warning > Reading Tape Volume Label -> VOL"
MSG_INVLNO	 " (invalid label number)"
MSG_INVNF	 "FATAL > Invalid numeric format in ->"
MSG_INVPN	 "FATAL > Invalid position sequence number -> "
MSG_INVPNUSE	 "Warning > P key only used with t or x option "
MSG_INVPS	 "FATAL > Invalid position section number -> "
MSG_INVUNIT	 "FATAL > Invalid unit number.  Min = 0, Max = 31"
MSG_INVRS	 "FATAL > Invalid record length. Min = 1 byte, Max ="
MSG_INVVID1	 "FATAL > Invalid characters in L key (see LTF(5))"
MSG_INVVID2	 "Warning > 'Z' Indicates invalid character(s) ->"

MSG_MHL	         "Head link file not extracted ?\n"
MSG_MS1	         "The  -u  key has precedence over the  -d  key"
MSG_MS2	         "The  -u  key does not apply when extracting binary files"
MSG_MS3	         "The  -d  key does not apply when extracting binary files"
MSG_MULTIV1	 "FATAL > EOV label encountered. Last input file not complete"

MSG_NOARGS	 "FATAL > No file names specified for c option"
MSG_NOBLK	 "FATAL > No blocksize specified with B key"
MSG_NOFIL	 "FATAL > No device specified with f key"
MSG_NOFUNC	 "FATAL > No option specified"
MSG_NOINP	 "FATAL > No input specified with I key"
MSG_NOMEM	 "FATAL > No free memory, exiting ..."
MSG_NOMDIR	 "Warning > No free memory for directory list"
MSG_NONAFN	 "Warning > Non 'A' characters in file name ->"
MSG_NOPOS	 "FATAL > No posnmbr specified with P key"
MSG_NOREC	 "FATAL > No reclen specified with R key"
MSG_NOTEX	 "FATAL > File not extracted"
MSG_NOTONP	 "Warning > File not found on volume after position number ->"
MSG_NOTONV	 "Warning > File not found on volume ->"
MSG_NOTSU	 "Warning > Not super user, cannot use p key with uid ->"
MSG_NOVALFI	 "FATAL > No valid files in argument list"
MSG_NOVOL	 "FATAL > No volumeid specified with L key"

MSG_OFFL1	 ", Place"
MSG_OFFL2	 "ONLINE"
MSG_OWNRID	 " Owner  ID is: "
MSG_OVRWRT	 "Overwrite (y/n return=no) ?"

MSG_RECLTS	 "FATAL > Record length too short"

MSG_SCNDCB	 "Warning > Second control byte in FUF is ->"
MSG_SLINKTO	 "Info > Symbolic link to ->"
MSG_SPCLDF	 "Warning > Cannot dump special device file ->"
MSG_STOPCRIN	 "Press y to quit OR return to skip unknown file when volume is created >"

MSG_TAPEB	 " Tape block"
MSG_TAPEBS	 " Tape blocks"
MSG_TMA	         "FATAL > Too many arguments, out of memory"
MSG_TRYHELP	 "Info > Type ltf H for explanation of the usage of the switches "
MSG_TRYHELPU11	 "Use 'help ltf' for an explanation of the switches."
MSG_TRYNH3	 "Warning > Try reading the volume with the O key (Noheader3)"

MSG_UNQ	         "FATAL > Unknown key ->"
MSG_USEDF	 "Warning > Ltf file type determination being overwritten for file ->"
MSG_USE1	 "usage: ltf [-]{cHtx}[ahoOpvVw0..31][BfILP] [blocksize] [devicefilename]"
MSG_USE2	 "       [inputfile] [volumelabel] [positionnumber] [files...]\n"
MSG_USE1U11	 "usage: ltf [-]{ctx} [aghknoOpvVw0..9] [BfILPR] [blocksize] [devicefilename]"
MSG_USE2U11	 "       [inputfile] [volumelabel] [positionnumber] [recordlength] [files...]\n"
MSG_UNSAV	 " (unsupported ANSI version)"

MSG_VOLCRE	 " Volume  created   on: "
MSG_VOLIDTL	 "FATAL > Maximum volume id length is 6 a-characters"
MSG_VOLIS	 " Volume ID is: "

MSG_WRLINM	 "FATAL > Wrong record length in middle of file ->"
MSG_WRTLCK	 ", WRITE ENABLE ->"
