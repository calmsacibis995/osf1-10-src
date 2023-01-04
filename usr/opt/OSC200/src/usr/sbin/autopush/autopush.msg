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


$quote "
$set MS_AUTOPUSH


APUSH_AUTH	"autopush: need sysadmin authorization\n"
APUSH_PRIV	"autopush: insufficient privileges\n"
APUSH_USAGE	"-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major-m minor"
APUSH_NOPENF	"Can't open file: %s\n"
APUSH_USE1	"usage: %s %s"
APUSH_ENUMB	"error number %d"
APUSH_USPERR	"unspecified error"
APUSH_SADOP	"SAD open failed for %s\n"
APUSH_PRBUG	"Program bug - bad arg/infop (%p)\n"
APUSH_INMAJ	"Invalid major (%d)\n"
APUSH_MAJNST	"Major (%d) is not a STREAMS device\n"
APUSH_DNOTAT	"Device (%d,%d) not configured for autopush\n"
APUSH_UNXIOCT	"Unexpected IOCTL(SAD_GAP) error: %d\n"
APUSH_HDR1	"Major Minor Last Minor  %2d module(s)\n"
APUSH_LINE1	"----- ----- ----------  ------------\n"
APUSH_ALL	" ALL   "
APUSH_LITLIN	"  ---    "
APUSH_INFOP	"infop->sap_cmd = %d\n"
APUSH_MORMOD	"More than %d modules in line %d.  Entry ignored\n"
APUSH_LINERR	"\tLine in error: %.40s ...\n"
APUSH_INMAJMIN	"Inconsistent Major (%d), Minor (%d), Last_minor (%d) at line %d\n"
APUSH_CNPSH	"Can't push requested modules on STREAM for entry %d\n"
APUSH_ONETO	"1 to %d module names required at line %d\n"
APUSH_INMN	"Invalid major (%d)\n"
APUSH_MODAVAL	"\nModule          Available\n"
APUSH_LN1	"------          ---------\n"
APUSH_SYES	"   Yes\n"
APUSH_SNO	"   No\n"
APUSH_UXIOC	"Unexpected IOCTL(SAD_VML) error (%d) checking module list.\n"
APUSH_DEVCON	"Device (%d,%d) already configured\n"
APUSH_PRGMB	"Program bug - bad arg/infop (%p)\n"
APUSH_MAJNSTR	"Major (%d) is not a STREAMS device\n"
APUSH_DNOAUTO	"Device (%d,%d) not configured for autopush\n"
APUSH_MINNOT	"Minor (%d) not first number in configured range\n"
APUSH_NOMAUTO	"No memory for autopush data structure\n"
APUSH_ONE	"ONE"
APUSH_RANGE	"RANGE"
APUSH_SPALL	"ALL"
APUSH_QUES	"???"
APUSH_UNXSAD	"Unexpected IOCTL(SAD_SAP/%s) error: %d\n"
APUSH_CONTI	"Continuing...\n"
APUSH_PBGARG	"Program bug - bad arg/infop (%p)\n"
APUSH_CONALL	"Major (%d) configured for ALL minors. Use \"-m0\" for minor.\n"
APUSH_NSTRDEV	"Major (%d) is not a STREAMS device\n"
APUSH_DNOCNF	"Device (%d,%d) not configured for autopush\n"
APUSH_CONFRAN	"Minor (%d) not first number in configured range\n"
APUSH_NOSTR	"No memory for autopush data structure\n"
APUSH_SADN	"Unexpected IOCTL(SAD_SAP/CLEAR) error: %d\n"
APUSH_STRCON	"str_to_major: couldn't open '%s', %s\n"
APUSH_STRCPSH	"str_to_major: couldn't PUSH 'sc', %s\n"
APUSH_STRGMAJ	"str_to_major: ioctl to get major number for '%s' failed, %s\n"
