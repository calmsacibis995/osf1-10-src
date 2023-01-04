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
$
$ HISTORY

$ @(#)$RCSfile: uucp.msg,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/09/07 16:09:07 $
$  
$ COMPONENT_NAME: UUCP uucp.msg 
$  
$ FUNCTIONS: CDEBUG, Logent  
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
$ All Rights Reserved 
$ Licensed Materials - Property of IBM 
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ uucp.msg	1.12  com/cmd/uucp,3.1,9013 3/7/90 14:26:13
$  
$quote "         define quote character.
$len 
$ The range of message numbers assigned to this file are 001-175
$set 1        Define initial set#

$ The following messages are used in uutry, Nutry and Uutry     
$ Do not translate %s, %d, %c, %1$s, etc.    
$ The following messages are from uutry
1                    "uutry: Specify a system name.\n"
2                    "uutry: %s is not a valid system name.\n"
$ The following messages are from uulog     
3                    "uulog: %s is not a recognized flag.\n"
4                    "uulog: Specify only one system with the -f flag.\n"
$ The following messages are from uutry
5                    "uutry: %s is not a recognized flag.\n"
6                    "Usage: uutry [-xN] [r] system\n"
$
$ Messages for uuto.sh
$
7 "Usage: uuto [-mp] files remote!user"
8 "%s: file/directory not found\n"

$set MS_UUCP        Redefine set#

$ There are no message related defines from the following soruce files:     
$       
$  \tuucpname.c\tchremdir.c \tcpmv.c     
$  \teio.c\t \tgetargs.c \tgetpwinfo.c     
$  \tgetprm.c\tgio.c \t\tgnamef.c     
$  \tgwd.c\t \timsg.c \t\tpkdefs.c     
$  \tstrpbrk.c\tuucpd.c \txio.c     
$  \txqt.c\t \tdbg.h \t\tparms.h     
$  \tpk.h\t\tshio.c     
$ Do not translate uucp.    
MSG_UUCHECK_1        "Usage: uucheck [-v] [-xNumber]\n"
MSG_UUCHECK_2        "uucheck: Check Required Files and Directories.\n"
MSG_UUCHECK_3        "uucheck: The check of the directories is complete.\n\n"
MSG_UUCHECK_4        "uucheck: Check file %s.\n"
MSG_UUCHECK_5        "uucheck: The %s check is complete.\n\n"
MSG_UUCHECK_6        "Machine phase. (To call or run uux requests.)"
$ Do not translate uux.    
MSG_UUCHECK_7        "Logname phase. (To receive calls.)"
MSG_UUCHECK_8        "uucheck: Cannot open %s.\n"
MSG_UUCHECK_9        "When a system logs in as: "
MSG_UUCHECK_10       "\tWe will call the system back.\n\n"
MSG_UUCHECK_11       "When we call the  system or systems."
MSG_UUCHECK_12       "\tWe allow the system to request files.\n"
MSG_UUCHECK_13       "\tWe do not allow the system to request files.\n"
MSG_UUCHECK_14       "\tWe will send files queued for the system on this call.\n"
MSG_UUCHECK_15       "\tWe will not send files queued for the system on this call.\n"
MSG_UUCHECK_16       "\tThe other system can send files to\n"
MSG_UUCHECK_17       "\t%s    (The default.)\n"
MSG_UUCHECK_18       "\tExcept for\n"
MSG_UUCHECK_19       "\tThe other system can request files from\n"
MSG_UUCHECK_21       "\tMyname for the conversation will be %s.\n"
$ Do not translate Myname.    
MSG_UUCHECK_22       "\tPUBDIR for the conversation will be %s.\n"
$ Do not translate PUBDIR.   
MSG_UUCHECK_23       "Machine or machines: "
MSG_UUCHECK_24       "\nCan run the following commands:\n"
MSG_UUCHECK_25       "uucheck: There is a software error in the permission.c file.\n"
$ Do not translate permission.c     
MSG_UUCHECK_26       "Command: %s. Fullname %s.\n"
$ Do not translate Fullname. 
$ These messages are from callers.c     
MSG_CALLERS_L1       "The system is attempting a generic open."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L2       "TIMEOUT"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L3       "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L4       "There is a generic call to gdial."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L5       "ACU write"
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L6       "The specified host does not exist."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L7       "There is no available socket."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L8       "The connect failed."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L9       "There is an 801 open."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_L10      "There is no available socket."
$ This is a log message. It is better not to translate this message. 
MSG_CALLERS_A1       "callers: program stopped: There is no uucp server."
$ Do not translate uucp.    
$ The following messages are from conn.c     
MSG_CONN_CD1         "conn(%s)\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD2         "getto returned %d.\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD3         "Call Failed. %s\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD4         "Device Type %s is wanted\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD5         "Requested Device Type Not Found\n"
$ This is debug message. Do not translate. 
MSG_CONN_A1          "BAD LINE"
$ This is debug message. Do not translate. 
MSG_CONN_CD6         "Wrong Time To Call %s\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD7         "expect: ("
$ This is debug message. Do not translate. 
MSG_CONN_CD9         "lost line errno - %d\n"
$ This is debug message. Do not translate. 
MSG_CONN_L1          "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_L2          "LOGIN"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_L3          "LOST LINE"
$ This is a log message. It is better not to translate this message. 
MSG_CONN_CD10        "enough already\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD8         "got it\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD12        "timed out\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD11        "BREAK\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD13        "EOT\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD14        "Sendthem ("
$ This is debug message. Do not translate. 
MSG_CONN_CD15        "DELAY\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD16        "NO CR"
$ This is debug message. Do not translate. 
MSG_CONN_CD17        "NO CR - MIDDLE IGNORED>\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD18        "PAUSE\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD19        "ECHO CHECK ON\n"
$ This is debug message. Do not translate. 
MSG_CONN_CD20        "ECHO CHECK OFF\n"
$ This is debug message. Do not translate. 
$ The following messages are from ct.c     
MSG_CT1              "ct: Cannot open %s.\n"
$ Do not translate ct.    
MSG_CT2              "Usage: %1$s %2$s\n"
MSG_CT3              "ct: The %s parameter to the -w flag must be greater than zero.\n"
MSG_CT5              "Usage: %1$s %2$s\n"
MSG_CT6              "Usage: %1$s %2$s\n"
MSG_CT7              "ct: Specify a phone number.\n"
MSG_CT20             "ct: The phone number %s is too long.\n"
MSG_CT8              "ct: The phone numbe %s is not correct.\n"
MSG_CT9              "ct: There is an allocated dialer at %s baud.\n"
$ Do not translate ct. 
MSG_CT10             "ct: There are no %s dialers on this system.\n"
$ Do not translate ct. 
MSG_CTV2             "ct: The dialer is busy %d minutes."
$ Do not translate ct. 
MSG_CTV3             "ct: The dialers are busy %d minutes."
$ Do not translate ct. 
MSG_CT11             "ct: The %1$d %2$s dialer is busy.\n"
$ Do not translate ct. 
MSG_CT12             "ct: The %1$d %2$s dialers are busy.\n"
$ Do not translate ct. 
MSG_CT13             "ct: Waiting for %d minutes.\n"
$ Do not translate ct. 
MSG_CT14             "ct: Waiting for %d minute.\n"
$ Do not translate ct. 
MSG_CT15             "ct: Do you want to wait for a dialer?\nType y for yes: "
$ Do not translate ct. 
$ Replace y for yes to the language you translate. 
MSG_CT16             "ct: Specify the number of minutes to wait: "
$ Do not translate ct. 
MSG_CT17             "ct: Waiting for a dialer.\n"
$ Do not translate ct. 
MSG_CT18             "ct: Time out.\n"
$ Do not translate ct. 
MSG_CTV5             "ct: Connected.\n"
$ Do not translate ct. 
MSG_CT22             "ct: Press the carriage return key."
$ Do not translate ct. 
MSG_CTCD1            "there is a shared or delayed getty; exit\n"
$ This is debug message. Do not translate. 
MSG_CTCD2            "start getty %s "
$ This is debug message. Do not translate. 
MSG_CT19             "ct: Cannot create another process for getty.\n"
$ Do not translate getty.    
MSG_CTV6             "ct: Cannot run getty.\n"
$ Do not translate getty.    
MSG_CT21             "\nTry to connect again?\nType y for yes: "
$ Replace y for yes to the language you translate. 
MSG_CTV7             "The connection is ended.\n"
MSG_CTCD3            "cleanup(%d)\n"
$ This is debug message. Do not translate. 
MSG_CTCD4            "Can't chown/chmod on %s "
$ This is debug message. Do not translate. 
MSG_CTCD5            "to %d\n"
$ This is debug message. Do not translate. 
MSG_CTCD6            "process still active\n"
$ This is debug message. Do not translate. 
$ The following messages are from line.c     
MSG_LINEA1           "program stopped: tty baud rate is not correct."
MSG_LINEA2           "program stopped: Error return from ioctl fixline."
$ Do not translate fixline.    
$ Do not translate ioctl.    
MSG_LINEA4           "program stopped: Error return from sytfixline."
$ Do not translate sytfixline.    
MSG_LINEA5           "program stopped: Error return from sytfix2line."
$ Do not translate sytfix2line.    
$ The following messages are from ulockf.c     
MSG_ULOCKFA1         "ulockf: program stopped: There are too many locks."
MSG_ULOCKFA2         "ulockf: program stopped: Cannot allocate for"
$ This message is followed by a filename. 
MSG_ULOCKFL1         "ULOCKC"
$ This is a log message. It is better not to translate this message. 
MSG_ULOCKFL2         "ULOCKLNK"
$ This is a log message. It is better not to translate this message. 
MSG_ULOCKFL3         "ULOCKF"
$ This is a log message. It is better not to translate this message. 
$ The following messages are from line.c     
MSG_ALTC_CD1         "altconn called\r\n"
$ This is debug message. Do not translate. 
MSG_ALTC_CD2         "getto ret %d\n"
$ This is debug message. Do not translate. 
$ The following messages are from altconn.c     
MSG_ALTCONNCD1       "altconn called\r\n"
$ This is debug message. Do not translate. 
MSG_ALTCONNCD2       "getto ret %d\n"
$ This is debug message. Do not translate. 
$ The following messages are from anlwrk.c     
MSG_ANLWRKe1         "anlwrk: Cannot use the old workfile."
$ Do not translate workfile.    
MSG_ANLWRKe2         "anlwrk: A file name in the bldflst subroutine is not valid."
$ Do not translate bldflst.    
$ The following messages are from cico.c     
MSG_CICOA1           "cico: program stopped: The user number is not correct. "
$ Leave spcae at the end.    
MSG_CICOA2           "cico: program stopped: The user name is not correct. "
$ Leave space at the end.    
MSG_CICO1            "Usage: uucico [-rROLE_NUMBER] [-xDEBUG_LEVEL] [-sSYSTEM_NA ME]\n" 
MSG_CICO6            "here=%s"
$ Do not translate this message. 
MSG_CICOL1           "FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL2           "CALLBACK"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL3           "REQUIRED"
$ This is a log message. It is better not to translate this message. 
MSG_CICO2            "CALL BACK"
$ Do not translate. 
MSG_CICOL4           "PREVIOUS"
$ This is a log message. It is better not to translate this message. 
MSG_CICO3            "BADSEQ"
$ Do not translate. 
MSG_CICO4            "OK"
$ Do not translate. 
MSG_CICOL5           "HANDSHAKE FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICO5            "BADSEQ"
$ Do not translate. 
MSG_CICOA3           "ULIMIT TOO SMALL"
$ Do not translate. 
MSG_CICOL21          "SYSTEM STATUS"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL6           "CAN NOT CALL"
$ This is a log message. It is better not to translate this message. 
MSG_CICO9            "call to %s "
$ Do not translate. 
MSG_CICOL22          "LOCKED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL7           "CONN FAILED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL8           "SUCCEEDDED"
$ This is a log message. It is better not to translate this message. 
MSG_CICOCD1          "Login Successful: System=%s\n"
$ This is debug message. Do not translate. 
MSG_CICOCD2          "HANDSHAKE FAILED: %s\n"
$ This is debug message. Do not translate. 
MSG_CICOL10          "startup"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL11          "TIMEOUT"
$ This is a log message. It is better not to translate this message. 
MSG_CICO7            "conversation complete %s %ld"
$ Do not translate. 
MSG_CICOL17          "conversation complete"
$ This is a log message. It is better not to translate this message. 
MSG_CICOCD3          "send OO %d,"
$ This is debug message. Do not translate. 
$ Do not translate OO.    
MSG_CICOCD4          "Conversation Complete: Status FAILED\n\n"
$ This is debug message. Do not translate. 
MSG_CICOCD5          "Conversation Complete: Status SUCCEEDED\n\n"
$ This is debug message. Do not translate. 
MSG_CICOL19          "CAUGHT"
$ This is a log message. It is better not to translate this message. 
MSG_CICOL20          "INTREXIT"
$ This is a log message. It is better not to translate this message. 
$ Do not translate intrexit.    
MSG_CICO21           "cico: The time out value is not valid.\n"
MSG_CICO22           "cico: %s not in Systems file\n"
$ The following messages are from dbg.c     
MSG_DBG1             "dbg: Cannot append to %1$s.\nThe errno is %2$d.\n"
$ Do not translate errno. 
MSG_DBG2             "Opened dbgfile %s."
$ Do not translate dbgfile.    
MSG_DBG3             "dbg: Cannot open /dev/tty because of %1$s %2$s."
$ Do not translate /dev/tty    
$ The following message is from expfile.c     
MSG_EXPFILE1         "expfile: File name %s is not correct.\n"
MSG_EXPFILE2         "expfile: File name %s is not correct. Use ascii characters only.\n"
$ Do not translate ascii.    
$ The following message is from culine.c     
MSG_CULINECD1        "fixline(%d, "
$ This is debug message. Do not translate. 
MSG_CULINEA1         "culine: program stopped: The speed is not correct."
MSG_CULINECD2        "fixline - direct\n"
$ This is debug message. Do not translate. 
MSG_CULINEA2         "program stopped: Error return from ioctl in fixline."
$ Do not translate fixline.    
$ Do not translate ioctl.    
$ The following message is from cu.c     
MSG_CU1              "Usage: %s [-s Speed] [-l Line] [-dhtu] [-o|-e] telno | systemname\n"
MSG_CU2              "cu: The connection failed. %s\r\n"
MSG_CU3              "cu: Cannot open %s.\r\n"
MSG_CU4              "cu: The remote line is gone.\r\n"
MSG_CU5              "cu: Cannot run a shell.\r\n"
MSG_CU6              "cu: Cannot change direction %s.\r\n"
MSG_CU7              "Use ~~ to start a line with the ~ character.\r\n"
MSG_CU8              "after %ld bytes.\r\n"
$ This line is constructed. 
MSG_CU9              "%d lines/"
$ This line is constructed. 
MSG_CU10             "%ld characters\r\n"
$ This line is constructed. 
MSG_CU11             "The file transmission was interrupted.\r\n"
MSG_CU12             "cu: Cannot create another process at this time.\r\n"
MSG_CU13             "r\ncu: Cannot transmit the special character `%#o`.\r\n"
MSG_CU14             "\nThe line is too long.\r\n"
MSG_CU15             "r\ncu: There is an input or output error.\r\n"
MSG_CU16             "Use the '~$' command.\r\n"
MSG_CU17             "Use '~+' commmand.\r\n"
MSG_CU18             "cu: Cannot get the terminal status.\r\n"
MSG_CU19             "cu: Cannot use the cu from a 3B console.\r\n"
$ Do not translate cu.    
$ Do not translate 3B.    
MSG_CU20             "cu: There is a parity option error.\r\n"
MSG_CU21             "cu: The telephone number cannot exceed 58 digits.\r\n"
MSG_CU22             "Enter the number: "
MSG_CUV1             "cu: Cannot run the setuid or setgid command.\n"
$ Do not translate setgid.    
$ Do not translate setuid.    
MSG_CUV2             "Connected\007\r\n"
$ The digits 007 cause a bell to ring.   
MSG_CUCD1            "tranmission started\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD2            "got break in transmit\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD3            "got a tintr\n\r"
$ This is debug message. Do not translate. 
MSG_CUCD4            "call tilda(%s)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV3             "(continue)"
MSG_CUCD5            "call _shell(%s)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD6            "call _dopercen(\"%s\")\r\n"
$ This is debug message. Do not translate. 
$ Do not translate dopercen.    
MSG_CUV4             "Usage: ~%%take From [To]\r\n"
MSG_CUV5             "Usage: ~%%put From [To]\r\n"
MSG_CUV6             "cu: Cannot change to the %s directory.\r\n"
MSG_CUV7             "cu: Cannot change to the %s directory.\r\n"
MSG_CUV12            "cu: ~%%%s is not known to the cu command.\r\n"
$ Do not translate %%%s.   
MSG_CUCD8            "_receive started\r\n"
$ This is debug message. Do not translate. 
MSG_CUV13            "\r\nLost carrier.\r\n"
MSG_CUCD9            "call _mode(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD10           "got read error. not EINTR\n\r"
$ This is debug message. Do not translate. 
MSG_CUV8             "\n\rcu: The output is blocked.\r\n"
MSG_CUCD11           "call _rcvdead(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD12           "call _quit(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUCD13           "call _bye(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV9             "\r\nThe connection is ended.\007\r\n"
$ The digit 007 cause a bell to ring.   
$ Do not translate \007. 
MSG_CUCD14           "call cleanup(%d)\r\n"
$ This is debug message. Do not translate. 
MSG_CUV10            "\rThe status of device fd is %d.\n"
$ Do not translate fd.    
MSG_CUV11            "\rF_GETFL=%o,"
$ Do not translate. 
$ The following messages is from cntrl.c     
MSG_CNTRL9           "REQUEST"
$ Do not translate this message. 
MSG_CNTRLCD1         "Request: %s\n"
$ This is debug message. Do not translate. 
MSG_CNTRL10          "DENIED"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL11          "ACCESS"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL12          "access denied"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL13          "cntrl: Access denied.\n"
$ This is a log message. It is better not to translate this message. 
$ Do not translate /etc/temp.    
MSG_CNTRL14          "cntrl: Cannot access"
$ This is a log message. It is better not to translate this message. 
MSG_CNTRL15          "cntrl: Cannot read %1$s %2$d."
MSG_CNTRL16          "cntrl: Failed."
MSG_CNTRL17          "cntrl: Cannot read %s.\n"
MSG_CNTRLCD2         "Failed: Access Denied - File: %s\n"
$ This is debug message. Do not translate. 
MSG_CNTRL18          "cntrl: Cannot create the temporary file."
MSG_CNTRLCD3         "Failed: No Space!\n"
$ This is debug message. Do not translate. 
MSG_CNTRL20          "Requested"
MSG_CNTRL22          "cntrl: There are not enough parameters in the slave SNDFILE."
$ Do not translate cntrl.    
MSG_CNTRL23          "Remote requested."
MSG_CNTRL24          "Remote requested: %s\n"
MSG_CNTRL26          "Permission"
MSG_CNTRL27          "cntrl: Cannot open."
MSG_CNTRL29          "cntrl: Cannot create a temporary file.\n"
MSG_CNTRL30          "cntrl: There is an input failure."
MSG_CNTRL31          "In send or slave mode."
MSG_CNTRL32          "Copy"
MSG_CNTRL33          "Converting System V multi-hop request."
MSG_CNTRL35          "cntrl: There are not enough parameters in slave RCVFILE."
$ Do not translate RCVFILE.    
MSG_CNTRL37          "The remote has requested %s.\n"
MSG_CNTRL39          "Requesting"
MSG_CNTRL40          "cntrl: Access is denied.\n"
MSG_CNTRL19          "Permission"
MSG_CNTRL21          "cntrl: Access is denied.\n"
MSG_CNTRL41          "Cannot open"
MSG_CNTRL42          "cntrl: Cannot open %s.\n"
MSG_CNTRL43          "BAD READ"
$ Do not translate this message. 
MSG_CNTRL44          "Expected %1$c: Received %2$s."
MSG_CNTRL45          "BAD READ"
$ Do not translate this message. 
MSG_CNTRL55          "The copy succeeded."
MSG_CNTRL46          "Request: %1$s\n(System: %2$s)  %3$s.\n"
MSG_CNTRL47          "Request: %1$s\n(System: %2$s)  %3$s.\n"
MSG_CNTRL48          "The uucp job is: %1$s (%2$s)."
MSG_CNTRL49          "Protocol started %c.\n"
MSG_CNTRL50          "Protocol handshaking failed '%c'.\n"
$ Do not translate handshaking. 
MSG_CNTRL51          "%1$s from %2$s!%3$s has arrived.\n"
MSG_CNTRL52          "The freespace is low."
MSG_CNTRL53          "The remote transfer is denied. "
MSG_CNTRL54          "There are too few inodes."
$ Do not translate inode.    
MSG_CNTRLN0          "cntrl: The copy failed.  The reason for the failure is not given by the remote."
MSG_CNTRLN1          "cntrl: Local access to the file is denied."
MSG_CNTRLN2          "cntrl: Remote access to a path or a file is denied."
MSG_CNTRLN3          "cntrl: There is a system error. A generated uucp command is not valid."
$ Do not translate uucp.    
MSG_CNTRLN4          "cntrl: The remote system cannot create a temporary file."
MSG_CNTRLN5          "cntrl: Cannot copy to a file or a directory.\n\tThe file remains in PUBDIR/user/file."
$ Do not translate PUBDIR/user/file.    
MSG_CNTRLN6          "cntrl: Cannot copy to a file or a directory.\n\tThe file remains in PUBDIR/user/file."
$ Do not translate PUBDIR/user/file.    
MSG_CNTRLN7          "cntrl: There is a forwarding error."
$ The following message is from gnxseq.c     
MSG_GX1              "gnxseq: Cannot lock."
$ The following message is from gtcfile.c     
MSG_GTC1             "gtcfile: There are too many saved C files."
$ The following messages are from gename.c     
MSG_GE1              "null sys -> %s\n"
$ Do not translate this message. 
MSG_GE2              "file - %s\n"
MSG_GE3              "gename: There is a SYSLST overflow."
$ Do not translate SYSLST.    
MSG_GE4              "%s seq "
MSG_GE5              "now %x\n"
$ The following messages are from mailst.c     
MSG_MAILST1          "%s mail %s"
$ Do not translate. 
MSG_MAILST2          "\n\t===== The standard error was =====\n"
MSG_MAILST3          "\n\t===== UUCP general failure =====\n"
$ Do not translate UUCP.  
MSG_MAILST4          "\n\t===== The standard input was ===== \n"
$ The following messages are from pk0.c     
MSG_PK0_1            "not control\n"
MSG_PK0_2            "pk0: The alloc change is not implemented.\n"
$ Do not translate alloc.    
MSG_PK0_4            "pk0: The srj is not implemented.\n"
$ Do not translate srj.    
MSG_PK0_7            "r short %d want %d\n"
$ Do not translate r short. 
MSG_PK0_5            "The rcount is %d.\n"
$ Do not translate rcount.    
MSG_PK0_6            "The xcount is %d.\n"
$ Do not trnaslate xcount.    
$ The following messages are from pk1.c     
MSG_PK1_1            "Dead!"
MSG_PK1_2            "INIT code a."
$ Do not translate INIT.    
MSG_PK1_3            "INIT code b."
$ Do not translate INIT.    
MSG_PK1_4            "O.K."
$ Do not translate this message. 
MSG_PK1_5            "Rcv/Xmit"
$ Do not translate this message. 
MSG_PK1_6            "RREJ?"
$ Do not translate this message. 
MSG_PK1_7            "PDEBUG?"
$ Do not translate this message. 
MSG_PK1_8            "Draino..."
$ Do not translate this message. 
MSG_PK1_9            "Waiting."
MSG_PK1_10           "The link is down."
MSG_PK1_11           "RCLOSE?"
$ Do not translate RCLOSE.    
MSG_PK1_12           "pk1: There is a frame that is not valid."
MSG_PK1_13           "End of line."
MSG_PK1_19           "State -"
$ The following message is from tio.c     
MSG_TIO2             "Sent data: %1$ld bytes in %2$ld.%3$02d seconds."
MSG_TIO5             "Received data: %1$ld bytes in %2$ld.%3$02d seconds."
MSG_TIO6             "Received data: %1$ld bytes in %2$ld.%3$02d seconds."
$ The following message is from permission.c     
MSG_PERM_A1          "program stopped: The following is not a valid option in the permission file :"
MSG_PERM_1           "permission: The following is not a valid option in the permission file:%s\n"
MSG_PERM_2           "permission: There is not enough memory available now.\n"
$ The following messages are from sys5uux.c     
MSG_SYS5_1           "%c return status in event of a failure.\n"
MSG_SYS5_2           "%c the return address for status\n"
MSG_SYS5_3           "QUEUED"
$ Do not translate. 
$ The following messages are from systat.c     
MSG_SYSTAT_A1        "program stopped: Cannot open SYSTAT."
$ Do not translate SYSTAT.   
MSG_SYSTAT_L1        "RETRY TIME NOT REACHED"
$ This is a log message. It is better not to translate this message. 
MSG_SYSTAT_L2        "NO CALL"
$ This is a log message. It is better not to translate this message. 
$ The following messages are from uucleanup.c     
MSG_UCL_1            "Usage: uucleanup [-oDays] [-mString] [-CDays] [-DDays] [-WDays]\n\t[-XDays] [-xLevel] [-sSystem]\n"
MSG_UCL_2            "Subject: Mail that cannot be delivered.\n"
MSG_UCL_3            "This mail message cannot be delivered.\n"
MSG_UCL_4            "(Probably to or from system %s)\n"
MSG_UCL_5            "It was sent to you or sent by you.\n"
MSG_UCL_6            "Sorry for the inconvenience.\n"
MSG_UCL_7            "Subject: Warning from uucp.\n"
$ Do not translate uucp.    
MSG_UCL_8            "uucleanup: Not able to contact machine %s since the job was queued.\n"
MSG_UCL_9            "The job will be deleted in several days if the problem is not corrected.\n"
MSG_UCL_10           "To stop the job, run the following command:\n"
MSG_UCL_12           "The -C, -D, -W, -X, and -o flags require a value greater than zero.\n"
MSG_UCL_13           "uucleanup: Cannot change to directory %1$s. The errno is %2$d.\n"
MSG_UCL_14           "uucleanup: Cannot open %1$s. The errno is %2$d.\n"
MSG_UCL_15           "uucleanup: dDprocess: Dummy command file %s unlinked."
$ Do not translate dDProcess.    
MSG_UCL_16           "uucleanup: cprocess: Bad command file %1$s, unlink %2$s."
MSG_UCL_17           "uucleanup: The job %s has been deleted.\n"
MSG_UCL_18           "uucleanup: cprocess: C. %1$s, mail returned %2$d, unlink %3$s."
$ Do not translate cprocess.    
MSG_UCL_19           "uucleanup: wprocess: %1$s: %2$s. Warning message sent to %3$s, returned %4$d."
$ Do not translate wprocess.    
MSG_UCL_20           "uucleanup: dMprocess: mail %1$s to %2$s!%3$s, returned %4$d, unlink %5$s."
$ Do not translate dMprocess.    
MSG_UCL_21           "uucleanup: dNprocess: Local news item unlink %s."
$ Do not translate dNprocess.    
MSG_UCL_22           "uucleanup: dNprocess: Remote run of rnews %1$s, returned %2$d, unlink %3$s."
$ Do not translate dNprocess.    
$ Do not translate rnews.    
MSG_UCL_23           "\n\tSincerely,\n\t%s!uucp\n"
$ Do not translate uucp.    
MSG_UCL_24           "##### Data File: ############################\n"
MSG_UCL_25           "Subject: uucp deleted the job.\n"
$ Do not translate uucp.    
MSG_UCL_26           "uucleanup: Cannot contact machine '%s'.\n"
MSG_UCL_27           "uucleanup: Files for machine %s not found.\n"
MSG_UUCLEAN1         "Subject: Mail that cannot be delivered.\n"
MSG_UUCLEAN2         "This mail message cannot be delivered.\n"
MSG_UUCLEAN3         "(Probably to or from system %s.\n"
MSG_UUCLEAN4         "It was sent to you or sent by you.\n"
MSG_UUCLEAN5         "Sorry for the inconvenience.\n"
MSG_UUCLEAN6         "Subject: uucp deleted the job.\n"
$ Do not translate uucp.    
MSG_UUCLEAN7         "uucleanup: Cannot contact machine %s.\n"
MSG_UUCLEAN8         "Subject: Warning from uucp.\n"
$ Do not translate uucp.    
MSG_UUCLEAN9         "uucleanup: able to contact machine %s since job was queued.\n"
MSG_UUCLEAN10        "The job will be deleted in several days if the problem is not corrected.\n"
MSG_UUCLEAN11        "To delete the job, run the following command:\n"
$ The following messages are from uucp.c     
MSG_UUCP1            "uucp: %s is not a known flag.\n"
MSG_UUCP2            "uucp: There is no work directory named %s.\n"
MSG_UUCP3            "uucp: From SourceFiles to DestinationFiles\n"
$ Do not translate uucp.    
MSG_UUCP4            "uucp: %s is not a valid system.\n"
MSG_UUCP5            "uucp: %s is not a valid syntax.\n"
MSG_UUCP6            "uucp: ULIMIT %ld is less than MINULIMIT %ld."
$ Do not translate ULIMIT.    
$ Do not translate MINULIMIT.    
MSG_UUCP7            "Low-ULIMIT"
$ Do not translate ULIMIT.    
MSG_UUCP8            "uucp: uucp failed completely. There was %d error.\n"
$ Do not translate uucp.    
MSG_UUCP9            "uucp: uucp failed partially. There were %d errors.\n"
$ Do not translate uucp.    
MSG_UUCP9P           "uucp: uucp failed partially. The error code is %d.\n"
$ Do not translate uucp.    
MSG_UUCP10           "uucp: Cannot get file status %s.\nThe copy failed.\n"
MSG_UUCP11           "uucp: %1$s and %2$s are the same file. Cannot copy.\n"
MSG_UUCP12           "uucp: Permission is denied.\n"
MSG_UUCP13           "uucp: The directory name %s is not valid.\n"
MSG_UUCP14           "uucp: uucp cannot read %s; access permission %o.\n"
$ Do not translate uucp.    
MSG_UUCP14C          "Use the cp command.\n"
$ Do not translate cp.    
MSG_UUCP15           "uucp: Cannot write file %1$s; access permission %2$o.\n"
MSG_UUCP16           "uucp: Cannot copy file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUCP17           "QUEUED"
$ Do not translate this message. 
MSG_UUCP18           "uucp: Cannot get status for file %s.\n"
MSG_UUCP19           "uucp: The directory name %s is not valid.\n"
MSG_UUCP20           "uucp: User name %s must contain ascii character only.\n"
$ Do not translate ascii.    
MSG_UUCP21           "uucp: File name %s must contain ascii character only.\n"
MSG_UUCP22           "uucp: The grade character %c is in valid use: a-z, A-Z, 0-9.\n"
MSG_UUCP23            "uucp: Cannot malloc any space %s.\n"
$ Do not translate ascii.    
$ The following messages are from uucpdefs.c     
MSG_UDEFS_1          "CAN'T OPEN"
$ Do not translate this message. 
MSG_UDEFS_2          "CAN'T WRITE"
$ Do not translate this message. 
MSG_UDEFS_3          "CAN'T READ"
$ Do not translate this message. 
MSG_UDEFS_4          "CAN'T CREATE"
$ Do not translate this message. 
MSG_UDEFS_5          "CAN'T ALLOCATE"
$ Do not translate this message. 
MSG_UDEFS_6          "CAN'T LOCK"
$ Do not translate this message. 
MSG_UDEFS_7          "CAN'T STAT"
$ Do not translate this message. 
MSG_UDEFS_8          "CAN'T CHOWN"
$ Do not translate this message. 
MSG_UDEFS_9          "CAN'T CHMOD"
$ Do not translate this message. 
MSG_UDEFS_10         "CAN'T LINK"
$ Do not translate this message. 
MSG_UDEFS_11         "CAN'T CHDIR"
$ Do not translate this message. 
MSG_UDEFS_12         "CAN'T UNLINK"
$ Do not translate this message. 
MSG_UDEFS_13         "WRONG ROLE"
$ Do not translate this message. 
MSG_UDEFS_14         "CAN'T MOVE TO CORRUPTDIR"
$ Do not translate this message. 
MSG_UDEFS_15         "CAN'T CLOSE"
$ Do not translate this message. 
MSG_UDEFS_16         "CAN'T FORK"
$ Do not translate this message. 
MSG_UDEFS_17         "FILE EXISTS"
$ Do not translate this message. 
$ The following messages come from uuname.c     
MSG_UUNAME1          "Usage: uuname [-l]\n"
$ Do not translate uuname.    
MSG_UUNAME2          "uuname: %s is a protected file.\n"
$ The following messages come from uusched.c    
MSG_UUSCHED1         "[-xNumber] [-uNumber]"
MSG_UUSCHED2         "Usage: %1$s %2$s\n"
MSG_UUSCHED3         "MACHINE TABLE FULL"
$ Do not translate this message. 
$ The following messages come from uustat.c     
MSG_UUSTAT1          "[-a] [-q|-m|-kJOB|-rJob|-p]"
MSG_UUSTAT2          "[-sSystem] [-uUser]"
MSG_UUSTAT3          "Usage: %1$s %2$s\n"
MSG_UUSTAT4          "or\nUsage: %1$s %2$s\n"
MSG_UUSTAT5          "uustat: Cannot find the job %s.  Not renewed.\n"
MSG_UUSTAT6          "uustat: Cannot get status for %1$s. The errno is%2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT7          "uustat: Cannot get status for %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT8          "uustat: %1$s is not a valid format. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT9          "uustat: %1$s is not a valid format. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT10         "uustat: Only the job owner or root can delete job %s.\n"
MSG_UUSTAT11         "uustat: Only the job owner or root can renew job %s.\n"
MSG_UUSTAT12         "uustat: Cannot delete file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT13         "uustat: Cannot renew file %1$s. The errno is %2$d.\n"
$ Do not translate errno. 
MSG_UUSTAT16         "uustat: The job %s is successfully deleted .\n"
MSG_UUSTAT17         "uustat: The job %s was successfully renewed.\n"
MSG_UUSTAT18         "uustat: Warning: Table overflow. The output is not complete.\n"
MSG_UUSTAT19         "uustat: Cannot find job %s. It was not deleted.\n"
MSG_UUSTAT_E1        "MACHINE TABLE FULL"
$ Do not translate this message. 
MSG_UUSTAT20         "uustat: User name %s must contain ascii character only.\n"
$ Do not translate ascii.    
$ The following messages come from uux.c     
MSG_UUX_1            "Usage: uux [-c | -C] [-n | -z] [- | -p] [-auser] [-bjr]\
 [-ggrade]\n\t[-sfile] [-xdebug_level] command_string\n" 
MSG_UUX_2            "File %s is not writable.\n" 
MSG_UUX_3            "uux: There is no spool directory %s.\n"
MSG_UUX_4            "uux: %s is not a valid system name.\n"
MSG_UUX_5            "%c for return status on success.\n"
MSG_UUX_6            "%c to not return status on failure.\n"
MSG_UUX_7            "%c to return status on failure.\n"
MSG_UUX_8            "%c to return input on abnormal exit.\n"
MSG_UUX_9            "%c to return address for status, or input return.\n"
MSG_UUX_11           "uux: %s is not a valid system name.\n"
MSG_UUX_12           "uux: Cannot get status for file %s.\n"
MSG_UUX_13           "uux: Permission is denied %s.\n"
MSG_UUX_14           "uux: uux failed. The errno is %d.\n"
$ Do not translate errno. 
$ Do not translate uux.    
MSG_UUX_15           "uux: Cannot copy %s.\n"
MSG_UUX_16           "uux: User name %s must contain ascii character only.\n"
$ Do not translate ascii.    
MSG_UUX_17           "uux: The command string %s must contain ascii character only.\n\n"
$ Do not translate ascii.    
MSG_UUX_18           "uux: System name, file name must contain ascii character only.\n\n"
MSG_UUX_19           "uux: The grade character %c is invalid use: a-z, A-Z, 0-9.\n"
$ Do not translate ascii.    
$ The following messages come from uuxqt.c     
MSG_UUXQT_1          "[-xDebug] [-sSystem]"
MSG_UUXQT_2          "Usage: %1$s %2$s\n"
MSG_UUXQT_3          "Usage: %1$s %2$s\n"
MSG_UUXQT_4          "%s XQT DENIED"
$ Do not translater this message.    
MSG_UUXQT_5          "uuxqt: Permission to run %s is denied."
MSG_UUXQT_6          "uuxqt: %s: XQT- stdin or stdout or file access is denied."
$ Do not translate XQT.   
MSG_UUXQT_7          "uuxqt: File access to %s is denied."
MSG_UUXQT_8          "uuxqt: Exited with status %d."
MSG_UUXQT_9          "Stopped by signal %d."
MSG_UUXQT_10         "COMMAND FAIL"
$ Do not translate this message. 
$ The following message is from versys.c     
MSG_VER_1            "versys: System name %s must contain ascii characteri only.\n"
$ The following messages are from getopt.c     
MSG_GOPT_1           "getopt: The following flag is not valid: "
MSG_GOPT_2           "getopt: The following flag requires a parameter: "
$ The following messages are from uucpd.c     
MSG_UUCPD_1          "user read\n"
MSG_UUCPD_2          "uucpd: The user is not known.\n"
MSG_UUCPD_3          "uucpd: Login is not correct."
MSG_UUCPD_4          "passwd read\n"
$ Do not translate passwd.    
MSG_UUCPD_5          "uucpd: Ther login is not correct."
MSG_UUCPD_6          "%1$s: %2$d, SRC not found, continuing without SRC support.\n"
$ Do not translate SRC. 
MSG_UUCPD_7          "%1$s: %2$d recvfrom.\n"
$ Do not translate recvfrom.    
MSG_UUCPD_8          "uucpd: The command does not support this option.\n"
MSG_UUCPD_9          "setsockopt SO_DEBUG: %m"
$ Do not translate this message. 
$ The following messages are for uuclean.c     
B_CLEAN_1            "Usage:  uuclean [-m] [-n Hours] [-d Subdirectory] [-p Prefix] . . .\n"
B_CLEAN_2            "uuclean: Specify a maximum of 10 prefixes.\n"
B_CLEAN_3            "uuclean: Prefix %s is too long.\n"
B_CLEAN_4            "uuclean: Cannot open the subdirectory %s.\n"
B_CLEAN_5            "uuclean: Cannot get status for %s.\n"
B_CLEAN_6            "uuclean: File %1$s is deleted.\nCannot contact remote. %2$d requests deleted.\n"
B_CLEAN_7            "Request: "
B_CLEAN_8            "First request : "
B_CLEAN_9            "\nLast request : "
B_CLEAN_10           "uuclean deleted file %s.\n"
$ Do not translate uuclean.    
$ The following messages are from uupoll.c     
MSG_UUPOLL_1         "\tUsage: uupoll [-gGrade] [-n] SystemName\n"
MSG_UUPOLL_2         "Current system is %s\n"
MSG_UUPOLL_3         "uupoll: %s is not a known system.\n"
MSG_UUPOLL_4         "uupoll: Cannot create directory %s.\n"
MSG_UUPOLL_5         "uupoll: Cannot write to %s.\n"
MSG_UUPOLL_6         "uupoll: Cannot run the uusched command.\n"
$ Do not translate uusched.    
$ The following messages are from uuq.c     
MSG_UUQ1 "Usage: uuq [-l] [-h] [-s system] [-u user] [-d jobno] [-r spool] [-b baudrate]\n"
MSG_UUQ2	"can't examine spooling area"
MSG_UUQ3	"out of memory\n"
MSG_UUQ4	"could not open '%s'\n"
MSG_UUQ5	"Removing data file %s\n"
MSG_UUQ6	"Removing command file %s\n"
MSG_UUQ7	"%.14s: empty command file\n"
MSG_UUQ8	"%3.1f minutes (@ effective baudrate of %d)"

MSG_UUXQT_11	"uuxqt: Cannot create %s"
