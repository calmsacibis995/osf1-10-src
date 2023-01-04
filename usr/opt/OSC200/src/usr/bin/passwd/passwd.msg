 
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
 
$ @(#)$RCSfile: passwd.msg,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/03/03 17:25:34 $ 8/15/90 SecureWare
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0
$quote "
$set MS_PASSWD

CHFNU		"usage: chfn [username]\n"
CHSHU		"usage: chsh [username]\n"
USAGE		"usage: passwd [-fs] [username]\n"
UNKUID		"%s: %u: unknown user uid.\n"
OUTOFSPACE	"%s: out of space.\n"
UNKUSER		"%s: %s: unknown user.\n"
PERMDEN1	"Permission denied.\n"
CHANG		"Changing %s for %s.\n"
FINGINF		"finger information"
LOGINSHELL	"login shell"
PASSWORD	"password"
PWDFILEBUSY	"%s: password file busy - try again.\n"
FDOPNFAIL	"%s: fdopen failed.\n"
WARN1		"Warning: dbm_open failed: %s: "
WARN2		"Warning: lock failed"
PERMDEN2	"%s: permission denied.\n"
WARN3		"Warning: dbm_store failed.\n"
WARN4		"Warning: %s write error, %s not updated.\n"
OLDPWD		"Old password:"
SORRY		"Sorry."
NEWPWD		"New password:"
PWDUNCH		"Password unchanged."
LONGER		"Please enter a longer password."
SUGGEST		"Please don't use an all-lower case password.\nUnusual \
		 capitalization, control characters or digits are suggested."
RETYPR		"Retype new password:"
MISMATCH	"Mismatch - password unchanged."
RESTRICT	"Cannot change from restricted shell %s\n"
OLDSH		"Old shell: %s\nNew shell: "
LOGINSH		"Login shell unchanged."
UNACCSH		"%s is unacceptable as a new shell.\n"
UNAVAILB	"%s is unavailable.\n"
DEF_VALUE	"Default values are printed inside of '[]'."
ACCEPT		"To accept the default, type <return>."
ENTRY1		"To have a blank entry, type the word 'none'."
NAME		"\nName [%s]: "
ROOM		"Room number (Exs: 597E or 197C) [%s]: "
PHONE1		"Office Phone (Ex: 6426000) [%s]: "
PHONE2		"Home Phone (Ex: 9875432) [%s]: "
FINGERINFO	"Finger information unchanged."
BADCHARS	"':' and ',' are not allowed."
MAX_CHAR_NUM	"Maximum number of characters allowed is %d\n"
CTRCHARS	"Control characters are not allowed."
PHONEMSG	"Phone numbers may only contain digits."
ALLOCATE	"\nUnable to allocate storage in get_defaults!\n"
NONE		"none\n"
BADTTY		"No controlling tty."
NODB		"Hashed database not in use, only %s text file updated.\n"
TOOLONG		"Warning - only the first 8 characters of the password will be used."
$set MS_PASSWD_SEC
S_QUIT		"quit"
S_ENT1		"Password: %-*s  Hyphenation: %s\nEnter password:"
S_ENT2		"Password: %-*s   Enter password:"
S_ID_NOT_SET	"Login user ID not set yet"
S_LBL_NOT_SET	"Sensitivity label not set"
S_NO_ENTRY_PWD	"No /etc/passwd entry for user ID %d"
S_MEM_ALL_ERR	"Memory allocation error"
S_PWD_FAIL	"password change failure"
S_PROT_PWD	"cannot access protected password entry"
S_MISMATCH	"ID mismatch"
S_NOT_AUTH1	"not authorized to change another user's password"
S_CANT_RETR	"cannot access system default database"
S_FAIL_RETR	"cannot access your protected password entry"
S_NOOPT		"you do not have any password changing options"
S_LAST_SUCC	"Last successful password change for %s: %s"
S_NEVER		"NEVER\n"
S_LAST_UNSUCC	"Last unsuccessful password change for %s: %s\n"
S_OLD_PWD	"Old password:"
S_UNK_OLD_PWD	"old password entered incorrectly"
S_OPN_CNTL_TERM	"opening control terminal"
S_ABORT_PWD	"abort password change"
S_CANT_OPN	"Cannot open your control terminal\n"
S_WANT		"Do you want (choose one letter only):\n"
S_PWDGEN	"\tpronounceable passwords generated for you (g)\n"
S_STRING1	"\ta string of characters generated (c) ?\n"
S_STRING2	"\ta string of letters generated (l) ?\n"
S_PICK1		"\tto pick your passwords (p) ?\n\n"
S_ENTER		"Enter choice here (q to quit): "
S_ABORT		"user aborted program"
S_INVCH		"Invalid choice\n"
S_NOT_AUTH2	"You are not authorized for that method\n"
S_MINIMUM	"minimum time between changes has not elapsed"
S_LIFETIME	"password lifetime has passed"
S_NEW_PWD	"New password:"
S_RE_NEW_PWD	"Re-enter new password:"
S_DONT_MATCH1	"They don't match"
S_ILLEGAL	"Illegal password"
S_TRY_AGAIN	", try again\n"
S_TOOMANY	"too many unsuccessful attempts"
S_GENERATING	"\nGenerating random "
S_PWD_PRON	"pronounceable password"
S_CHAR_STR	"string of characters"
S_LETT_STR	"string of letters"
S_FOR		" for %s.\n"
S_SHOWN		"The password, along with a hyphenated version, is shown.\n"
S_SLOW		"\t(Password generation will be a bit slow.)\n"
S_CHOICE	"Hit <RETURN> or <ENTER> until you like the choice.\n"
S_TYPE		"When you have chosen the password you want, type it in.\n"
S_NOTE		"Note: type your interrupt character or `%s' to abort at any time.\n\n"
S_UNCOMP_PWD	"cannot compute password size"
S_BAD_SEED	"bad seed"
S_PROG_STPD	"user stopped program"
S_DONT_MATCH2	"They don't match; try again.\n"
S_PICK2		"pick of a random password was rejected or interrupted"
S_USER_FORCED	"user forced to pick again"
S_FAILED	"failed to write protected password entry"
S_NOT_ALLOWED	"You are not allowed to have a null password.\n"
S_NULL_PWD	"You already have a null password.\n"
S_NOT_RE_USE	"You may not re-use the same password.\n"
S_DENIED_REQ	"\nPassword request denied.\nReason: %s.\n"
S_PWD_CANT_CHG	"Password not changed: %s.\n"
S_ERR_AUTH	"Authentication error; see Account Administrator\n"
S_INSUFF_PRIV	"%s: insufficient privileges\n"
