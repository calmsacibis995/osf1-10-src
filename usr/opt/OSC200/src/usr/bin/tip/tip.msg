
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
$
$ @(#)$RCSfile: tip.msg,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/09/30 18:39:52 $
$
$   
$ COMPONENT_NAME: UUCP tip.msg  
$   
$ FUNCTIONS:   
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989  
$ All Rights Reserved  
$ Licensed Materials - Property of IBM  
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$
$ tip.msg  1.2  com/cmd/tip,3.1,9013 2/5/90 12:16:03  
$   
$ messages for tip  
$len 
$quote "         define quote character.
$set MS_TIP        Define initial set#

ACCESSDENY           "tip: The access is denied.\r\n"
ACUERR               "tip: There is an ACU open error."
$ Do not translate ACU.  
ALLBUSY              "tip: All the ports are busy.\n"
AWAYFOR              "away for "
BADBAUD              "tip: The baud rate %d is not valid.\n"
BADDIR               "tip: %s is not a valid directory\r\n"
BADENTRY             "tip: The remcap entry is not valid.\n"
$ Do not translate remcap. 
BADFLAG              "tip: %s is not a known flag."
BUFFOFLOW            "tip: %s was too long.\n"
CALLABORT            "\ntip: The call was aborted.\n"
CALLABORT2           "The call was aborted."
CALLCOMPLETE         "The call is completed."
CALLFAIL             "The call failed."
CALLFAILED           "The call failed.\n"
CALLFAILED2          "The call failed."
CALLTERM             "The call was stopped."
CANTALLOCATE         "tip: program stopped: Cannot allocate memory for %s."
CANTCREAT            "\r\ntip: Cannot create the file named %s.\r\n"
CANTCREAT2           "\r\ntip: Cannot create the file named %s.\r\n"
CANTCREAT3           "tip: Cannot create the file named %s.\r\n"
CANTEXECL            "tip: Cannot run the execl system call\r\n"
$ Do not translate execl. 
CANTEXECL2           "\r\ntip: Cannot run the execl system call.\r\n"
$ Do not translate execl. 
CANTFIND             "tip: Cannot find the file %s\r\n"
CANTFORK             "tip: Cannot start.\r\n"
CANTGET              "tip: There is no dial tone."
CANTHANGUP           "tip: Cannot hang up modem.\n\
\r"
CANTINIT             "tip: Cannot initialize the Bizcomp modem."
CANTOPEN             "tip: Cannot open the file %s.\r\n"
CANTOPEN2            "\ttip: Can't open log file. Contact your administrator.\r\n"
CANTOPEN3            "tip: Cannot open the host description file.\n"
CANTPIPE             "tip: Cannot open pipe.\r\n"
CANTSET              "tip: Cannot set dialing mode."
CANTSYNC             "tip: Cannot synchronize with the Courier modem.\n"
CANTOPENLOCK         "lock open"
CANTREADLOCK         "lock read"
CANTLSEEKLOCK        "lock lseek"
CANTOPENLOCK2        "tip: Cannot open lock file %s.\n"
CANTWRITLOCK         "lock write"
$ Do not translate Courier. 
CANTSYNC2            "Cannot synchronize with the modem."
CANTSYNC3            "tip: Cannot synchronize with the Hayes modem.\n\
\r"
$ Do not translate Hayes. 
CANTSYNC4            "tip: Cannot synchronize with Vadic 3451 modem.\n"
$ Do not translate Vadic 3451. 
CANTSYNC5            "tip: Cannot synchronize with the Ventel modem.\n"
$ Do not translate Ventel. 
CD                   "[cd] "
$ Do not translate this message. 
CHARSXFER            "characters transferred in "
$ Leave a space at the end. This is a constructed sentence. 
CONNECT              "CONNECT"
$ Do not translate this message. 
CONNED               "Connected.\r\n"
CONNECT12            "CONNECT 1200"
$ Do not translate this message. 
CONNECT2             "\07Connected.\r\n"
$ Do not translate \07. 
CONNECTFAIL          "The connect failed.\n"
CONNECTION           "CONNECTION\r\n\07"
$ Do not translate this message. 
DIALINGIT            "\nDialing..."
DIALTIMEDOUT         "%d second dial timeout."
DISCONNECTING        "\r\nDisconnecting..."
DISCONNECTING2       "\rDisconnecting modem...\n\r"
ECHOFAIL             "\"echo\" command failed.\n"
$ Do not translate echo. 
ENTRYTOOLONG         "The remcap entry is too long.\n"
$ Do not translate remcap. 
EOT                  "\r\n[EOT]\r\n"
$ Do not translate this message. 
EOT2                 "\07%s\n[EOT]\n"
$ Do not translate this message. 
ERROR                "tip: The input is not correct."
FAILEDDIAL           "tip: The Vadic modem failed to dial.\n"
$ Do not trabslate Vadic. 
FAILEDDIAL2          "The modem failed to dial."
FLOCK                "tip: flock"
$ Do not translate this message. 
FORK                 "fork"
$ Do not translate this message. 
INFINITE             "tip: There is an infinite tc= loop in /etc/remote.\n"
INTERACTIVE          "tip: Tip must be run in the foreground.\n"
INTERRUPT	     "interrupt"
LINEBUSY             "The telephone line is busy."
LINEFAILED           "tip: Failed to open a dialup line."
LINESXFER            " lines transferred in "
$ This is a constructed sentence. 
LINKDOWN             "tip: The link is down.\n"
LISTCOMM             "Type the command for the remote system: "
LOCALCOMM            "Local command? "
LOCALFILE            "Local file name? "
LOSTCARRIER          "tip: The carrier signal was stopped."
NOACCEPT             "tip: The Vadic modem will not accept the dial command.\n"
$ Do not translate Vadic. 
NOACCEPT2            "tip: The modem will not accept the dial."
NOACCEPTPH           "tip: The Vadic modem will not accept phone number.\n"
$ Do not translate Vadic. 
NOACCEPTPH2          "tip: The modem will not accept number."
NOCARRIER            "NO CARRIER"
$ Do not translate this message. 
NOCONNECT            " NO CONNECTION\r\n\007\r\n"
$ Do not translate this message. 
NOCORE               "tip: There is no memory available now.\r\n"
NODEVSPEC            "tip: %s: missing device specification.\n"
NODIALER             "tip: Unable to locate the dialer %s.\n"
NOHOST               "tip: No host is specified.\n"
NOMATCH              "tip: \"%s\": No match is found.\n"
NOPHONE              "tip: %s: The phone number was not specified.\n"
NOPN                 "The phone number is missing."
NOPNFILE             "tip: Cannot open /etc/phones."
NOSUPPORT            "The speed %s is not supported.\n"
NUMTOOLONG           "tip: The phone number is too long. (maximum = %d digits)\n"
OK                   "OK"
$ Do not translate this message. 
OUTOFSYNC            "The modem is out of synchronization."
OUTOFSYNC2           "tip: The Bizcomp modem is out of synchronization.\n"
$ Do not translate Bizcomp. 
PIPE                 "pipe"
$ Do not translate this message. 
PLEASE               "Use 'tip dialer' to make sure the line is disconnected.\n\
\r"
PUT                  "[put] "
$ Do not translate this message. 
READIT               "read"
$ Do not translate this message. 
RING                 "RING"
$ Do not translate this message. 
RINGING              "Ringing..."
RINGING2             "RINGING"
$ Do not translate this message. 
SET                  "[set] "
$ Do not trnaslate this message. 
SET2                 "set %s"
SH                   "[sh]\r\n"
$ Do not translate this message. 
STARTCALL            "\nStarting call..."
TAKE                 "[take] "
$ Do not translate this message. 
TIMEDOUT             "\r\ntip: The remote system is timed out at EOL.\r\n"
$ Do not translate EOL. 
TIMEDOUT2            "The remote system connection timed out.\r\n"
TIMEOUT              "\07The remote system timeout waiting for reply.\n"
TIMEOUT2             "\07The remote system timeout waiting for reply.\n\r"
TIMEOUTERR           "\r\ntip: timeout error (%s)\r\n"
TIPIOCTL             "tip: ioctl"
$ Do not translate this message. 
TOOMANYLOCKS         "TOO MANY LOCKS %d"
$ Do not translate this message. 
UNKERR               "tip: Unknown modem error: %1$c (0x%2$x)"
NOACUTYPE            "tip: The ACU type 'at' in /etc/remote is NOT setup."
UNKNOWNACU           "tip: The ACU specified is not a known type."
UNKNOWNHOST          "tip: The host %s is not known.\n"
UNKOPT               "tip: The %s is not a known option.\n"
UNKPARITY            "tip: The parity value %s is not known.\n"
UNKVAR               "tip: The variable %s is not known.\r\n"
UNRECHOST            "tip: The host name is not recognized."
USAGE                "Usage: <take> from [to]\r\n"
USAGE2               "Usage: <put> from [to]\r\n"
USAGE3               "usage: cu telno [-t] [-s Speed] [-a ACU] [-l Line] [-#]\n"
USAGE4               "Usage: tip [-v] [-Speed] SystemName | Number\n"
USING                "Using ACU %s.\n"
$ Do not translate ACU. 
WRITEIT              "write"
$ Do not translate this message. 
WRITERR              "\r\ntip: The write system call failed.\r\n"
$ Do not translate write. 
NUMSING              "%1$d %2$s "
$ Do not translate. 
NUMPLURL             "%1$d %2$ss "
$ Do not translate. 

$
$ These three message MUST follow each other
$
LITSECOND            "second"
$ This is constructed message. 
LITMINUTE            "minute"
$ This is constructed message. 
LITHOUR              "hour"
$ This is constructed message. 

$
$ Make sure to leave these mesages in order
$
HELP01		"shell"
HELP02		"Receive a file from remote host."
HELP03		"Send a file to remote host."
HELP04		"Take a file from remote host."
HELP05		"Put a file to remote host."
HELP06		"Pipe command output to a remote host."
HELP07		"Pipe local command output to a remote host."
HELP08		"Connect program to remote host."
HELP09		"Change directory."
HELP10		"Exit from tip."
HELP11		"Exit from tip."
$ Do not translate tip. 
HELP12		"Suspend tip on the local and the remote system."
$ Do not translate tip. 
HELP13		"Suspend tip on the local system only."
$ Do not translate tip. 
HELP14		"Set a variable."
HELP15		"Display this command summary."
HELP16		"Send break character to remote system."
$ Do not translate break. 

NOMEM		  "tip: out of memory\n"
