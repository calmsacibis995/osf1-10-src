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
static char rcsid[] = "@(#)$RCSfile: cdsuf.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/07/16 13:07:40 $";
#endif

/*
 * cdsuf: Read the System Use Fields from a System Use Area of a RRIP-format
 * ISO-9660 CD-ROM.
 */

#include <sys/cdrom.h>
#include <stdio.h>
#include <errno.h>

#define CDFS_COPYINT(src,dest) {dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = src[3];}

extern int errno;

void binaryout(int, char *);
void textout(int, char *);
time_t rrip_convert_tf_ts(int, signed char *);

void
usage()
{
    fprintf(stderr,
	    "cdsuf: usage: cdsuf [ -s number ] [-b] file\n");
    exit(1);
}

void
main(int argc, char *argv[])
{
    int opt;
    extern char *optarg;
    extern int optind;
    extern int opterr;
    int secnumber = -1;
    int binary = 0;

    while ((opt = getopt(argc, argv, "s:b")) != EOF) {
	switch (opt) {
	case 's':
	    secnumber = strtol(optarg, 0, 0);
	    break;
	case 'b':
	    binary++;
	    break;
	default:
	    usage();
	}
    }
    if (optind != argc - 1)
	usage();
    if (binary)
	binaryout(secnumber, argv[optind]);
    else
	textout(secnumber, argv[optind]);
    exit(0);
}

void
binaryout(int secnum, char *filename)
{
    int i = 0;
    int retval;
    unsigned char optbuf[256];		/* SUSP fields are limited to <= 255
					   bytes since length is encoded
					   as 8-bit unsigned number */

    for (i = 1; ; i++) {
	retval = cd_suf(filename, secnum, NULL, i, (char *)optbuf,
			sizeof(optbuf));
	if (retval == 0)
	    break;			/* no more */
	if (retval == -1) {
	    if (errno == EINVAL)
		break;			/* probably bad index */
	    fprintf(stderr, "cd_suf: file ");
	    perror(filename);
	    exit(1);
	}
	fwrite(optbuf, 1, retval, stdout);
	fflush(stdout);
    }
    return;
}
union susp_union {
    struct cd_suf_ce ce;
    struct cd_suf_sp sp;
    struct cd_suf_st st;
    struct cd_suf_er er;
    struct rrip_px px;
    struct rrip_pn pn;
    struct rrip_cl cl;
    struct rrip_pl pl;
    struct rrip_re re;
    struct rrip_sl sl;
    struct rrip_nm nm;
    struct rrip_tf tf;
    struct rrip_rr rr;
};

void
textout(int secnum, char *filename)
{
    int i = 0;
    int retval;
    unsigned char optbuf[256];		/* SUSP fields are limited to <= 255
					   bytes since length is encoded
					   as 8-bit unsigned number */
    
    struct cd_suf_header *hdr = (struct cd_suf_header *)&optbuf[0];
    union susp_union *un = (union susp_union *)hdr;
    union {
	unsigned char incoming[4];
	int outgoing;
    } convert;
    int tsnum;
    time_t tval;
    char *cp;
    int offset;

    for (i = 1; ; i++) {
	retval = cd_suf(filename, secnum, NULL, i, (char *)optbuf,
			sizeof(optbuf));
	if (retval == 0)
	    break;			/* no more */
	if (retval == -1) {
	    if (errno == EINVAL)
		break;			/* probably bad index */
	    fprintf(stderr, "cd_suf: file ");
	    perror(filename);
	    exit(1);
	}
	if (i == 1)
	    printf("Signature\tLength\tVersion\tData\n");

	printf("%c%c\t\t", hdr->suf_sig_word[0], hdr->suf_sig_word[1]);
	printf("%d\t",	hdr->suf_length);
	printf("%d\t", hdr->suf_version);

	if (hdr->suf_version == 1) switch(SUSP_SHORTIFY(hdr->suf_sig_word)) {
#define SUSP_SHORTIFY_SIG(a,b) ((a << 8) | b)
	case SUSP_SHORTIFY_SIG('C','E'):
	    CDFS_COPYINT(un->ce.cont_lbn_lsb, convert.incoming);
	    printf("Continued at block %#x ", convert.outgoing);
	    CDFS_COPYINT(un->ce.cont_offset_lsb, convert.incoming);
	    printf("offset %#x ", convert.outgoing);
	    CDFS_COPYINT(un->ce.cont_len_lsb, convert.incoming);
	    printf("length %#x\n", convert.outgoing);
	    break;
	case SUSP_SHORTIFY_SIG('S', 'P'):
	    if (un->sp.sp_checkbytes[0] != SUF_SP_CHECK1 ||
		un->sp.sp_checkbytes[1] != SUF_SP_CHECK2)
		printf("unknown SP version %02x%02x\n",
		       un->sp.sp_checkbytes[0],
		       un->sp.sp_checkbytes[1]);
	    else
		printf("SP version 0xBEEF\n");
	    break;
	case SUSP_SHORTIFY_SIG('S', 'T'):
	    printf("SUSP terminator\n");
	    break;
	case SUSP_SHORTIFY_SIG('E', 'R'):
	    printf("extent id '%.*s'\n", un->er.er_len_id, &un->er.er_data[0]);
	    printf("\t\tdescriptor id '%.*s'\n", un->er.er_len_des,
		   &un->er.er_data[un->er.er_len_id]);
	    printf("\t\tsource id '%.*s'\n", un->er.er_len_src,
		   &un->er.er_data[un->er.er_len_id + un->er.er_len_des]);
	    printf("\t\tversion id %02x\n", un->er.er_ext_ver);
	    break;
	case RRIP_SHORTIFY_SIG(PX):
	    CDFS_COPYINT(un->px.px_mode_lsb, convert.incoming);
	    printf("mode %0o ", convert.outgoing);
	    CDFS_COPYINT(un->px.px_nlink_lsb, convert.incoming);
	    printf("nlink %d ", convert.outgoing);
	    CDFS_COPYINT(un->px.px_uid_lsb, convert.incoming);
	    printf("uid %d ", convert.outgoing);
	    CDFS_COPYINT(un->px.px_gid_lsb, convert.incoming);
	    printf("gid %d\n", convert.outgoing);
	    break;
	case RRIP_SHORTIFY_SIG(PN):
	    CDFS_COPYINT(un->pn.pn_dev_t_high_lsb, convert.incoming);
	    printf("high %#x ", convert.outgoing);
	    CDFS_COPYINT(un->pn.pn_dev_t_low_lsb, convert.incoming);
	    printf("low %#x\n", convert.outgoing);
	    break;
	case RRIP_SHORTIFY_SIG(CL):
	    CDFS_COPYINT(un->cl.cl_child_lbn_lsb, convert.incoming);
	    printf("child pointer %#x\n", convert.outgoing);
	    break;
	case RRIP_SHORTIFY_SIG(PL):
	    CDFS_COPYINT(un->pl.pl_parent_lbn_lsb, convert.incoming);
	    printf("parent pointer %#x\n", convert.outgoing);
	    break;
	case RRIP_SHORTIFY_SIG(TF):
	    tsnum = 1;
	    if (un->tf.tf_flags & RRIP_TF_CREATION) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("creation: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_MODIFY) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("modify: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_ACCESS) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("access: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_ATTRIBUTES) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("attributes: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_BACKUP) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("backup: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_EXPIRATION) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("expiration: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    if (un->tf.tf_flags & RRIP_TF_EFFECTIVE) {
		tval = rrip_convert_tf_ts(un->tf.tf_flags&RRIP_TF_LONG_FORM,
					  RRIP_TF_TSPTR((&un->tf),tsnum));
		printf("effective: %s\t\t\t\t", ctime(&tval));
		tsnum++;
	    }
	    putchar('\n');
	    break;
	case RRIP_SHORTIFY_SIG(RE):
	    printf("relocated directory\n");
	    break;
	    
	case RRIP_SHORTIFY_SIG(NM):
	    switch(un->nm.nm_flags & ~RRIP_NM_CONTINUE) {
	    case 0:
		printf("%.*s ", un->nm.hdr.suf_length - 5,
		       un->nm.nm_component);
		break;
	    case RRIP_NM_CURRENT:
		printf("[current]");
		break;
	    case RRIP_NM_PARENT:
		printf("[parent]");
		break;
	    case RRIP_NM_HOST:
		printf("[hostname]");
		break;
	    default:
		printf("[unknown flags %02X]", un->nm.nm_flags);
		break;
	    }
	    if (un->nm.nm_flags & RRIP_NM_CONTINUE)
		printf("<<continued in next>>\n");
	    else
		putchar('\n');
	    break;

	case RRIP_SHORTIFY_SIG(SL):
	    if (hdr->suf_length < 5) {
		printf("malformed length on SL field\n");
		break;
	    }
	    for (offset = 0; offset < hdr->suf_length - 5; ) {
		struct rrip_sl_component *comp =
		    (struct rrip_sl_component *)&un->sl.sl_components[offset];
		offset += comp->slc_len + 2; 

		switch(comp->slc_flags & ~RRIP_SLC_CONTINUE) {
		case 0:
		    printf("%.*s ", comp->slc_len, comp->slc_component);
		    break;
		case RRIP_SLC_CURRENT:
		    printf("[current]");
		    break;
		case RRIP_SLC_PARENT:
		    printf("[parent]");
		    break;
		case RRIP_SLC_ROOT:
		    printf("[root]");
		    break;
		case RRIP_SLC_VOLROOT:
		    printf("[volroot]");
		    break;
		case RRIP_SLC_HOST:
		    printf("[HOST]");
		    break;
		default:
		    printf("[unknown flags %02X]", un->nm.nm_flags);
		    break;
		}
		if (comp->slc_flags & RRIP_SLC_CONTINUE)
		    printf("<<continued in next>>\n\t\t\t\t");
		else
		    fputs("\n\t\t\t\t", stdout);
	    }
	    putchar('\n');
	    break;
	case RRIP_SHORTIFY_SIG(RR):
	    fputs("fields present: ", stdout);
	    if (un->rr.rr_present & RRIP_RR_PX_PRESENT)
		fputs(RRIP_SIG_PX " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_PN_PRESENT)
		fputs(RRIP_SIG_PN " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_SL_PRESENT)
		fputs(RRIP_SIG_SL " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_NM_PRESENT)
		fputs(RRIP_SIG_NM " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_CL_PRESENT)
		fputs(RRIP_SIG_CL " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_PL_PRESENT)
		fputs(RRIP_SIG_PL " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_RE_PRESENT)
		fputs(RRIP_SIG_RE " ", stdout);
	    if (un->rr.rr_present & RRIP_RR_TF_PRESENT)
		fputs(RRIP_SIG_TF " ", stdout);
	    putchar('\n');
	    break;

	default:
	    /* interpret the field */
	    /* retval is total length, so subtract out these 4 bytes */
	    fwrite(&optbuf[sizeof(*hdr)], 1, retval - sizeof(*hdr), stdout);
	    putchar('\n');
	    break;
	} else {
	    /* not version 1 */
	    printf("unknown version on SUSP field\n");
	}
    }
    return;
}

/*
 * code lifted from sys/cdfs/cdfs_subr.c
 */

static	int	dmsize[12] =
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
/* leap year is something that == 0 mod 4 and != 0 mod 100,
   or == 0 mod 400 */
#define	dysize(A) (((A)%4)? 365 : (!((A) % 100) && ((A) % 400)) ? 365 : 366)

/*
 * cdfs_tounixdate():	convert 7-byte ISO date format to UNIX timestamp.
 *
 * ARGS:
 *	ret_time:	(out)	fill in the result time here
 *	y,m,d,h,mi,s:	(in)	one-byte quantities representing year, month,
 *				day, hour, minutes, seconds
 *	o:		(in)	signed quantity of 15-minute intervals
 *				from GMT.
 */
static void
cdfs_tounixdate(ret_time,yrs,mon,dys,hrs,mins,secs,gmtoff)
time_t *ret_time;
char yrs;
char mon;
char dys;
char hrs;
char mins;
char secs;
char gmtoff;
{
	register int i;
#ifdef __alpha
	register unsigned long val = 0;
#else
#error  You need to figure out how to detect timeval overflow
#endif
	int iy;
	int im;


	*ret_time = (time_t)0;
	if (yrs < 0 || yrs > 100 || mon < 1 || mon > 12 ||
	    dys < 1 || dys > 31 || hrs < 0 || hrs > 23 ||
	    mins < 0 || mins > 59 || secs < 0 || secs > 59)
		
		return;
	/* XCDR says dates before the oldest representable time
	   are to be returned as (time_t) 0.
	   1970 is the witching hour.
	   XXX gmt offset? */
	if (yrs >= 69) {
	    iy = yrs + 1900;
	    for (i = 1970; i < iy; i++)
		val += dysize(i);
	    im = mon;
	    /* 
	     * Leap year 
	     */
	    if (dysize(iy) == 366 && im >= 3)
		val++;
	    /*
	     * Do the current year
	     */
	    while(--im)
		val += dmsize[im-1];
	    val += dys-1;
	    val = 24*val + hrs;
	    val = 60*val + mins;
	    val = 60*val + secs;
	    /* o is signed value, in 15-minute increments.
	       negative is west (behind GMT)
	       positive is east (ahead of GMT).
	       subtract so that, e.g. noon US EST (which is GMT-5h, or -20)
	       becomes 5pm GMT */
	    val -= gmtoff * 15 * 60;	
	    /* now, if it was 1969, take off the year and see if we
	       still fit */
	    if (yrs == 69) {
		/* 1969 was not a leap year */
		if (365*24*60*60 < val)
		    val -= 365*24*60*60;
		else
		    val = 0;		/* too early */
	    }
	}
#ifdef __alpha
	/* depends on time_t being typedef'd as an int */
	if (val > INT_MAX)
	    *ret_time = INT_MAX;
	else
#endif
	*ret_time = (time_t)val;
	return;
}
/*
 * rrip_convert_tf_ts():	convert RRIP timestamp to UNIX time.
 *				either a 7-byte or a 17-byte quantity.
 *
 * ARGS:
 *
 *	longform:	(in)	is this a long-form timestamp?
 *	tstring: 	(in)	bytes of the timestamp
 *
 * RETURNS:
 *	the time_t representation of that time.
 */
time_t
rrip_convert_tf_ts(int longform,
		   signed char *tstring)
{
    time_t retval;
    if (longform) {
	register int i;
#ifdef __alpha
	register unsigned long val = 0;
#else
#error  You need to figure out how to detect timeval overflow
#endif
	int iy;
	int im;
	/* The long form is YYYYMMDDHHMMSSo, with YMDHMS as ASCII digits,
	   and o as an unsigned GMT offset */

	/*
	 * Added up the seconds since the epoch
	 *  val = o - tz.tz_minuteswest;  should divide into 24 hour periods
	 */
#define digitval(c) (c - '0')
#define twodigits(cp) (digitval((cp)[0])*10+digitval((cp)[1]))
	iy = twodigits(tstring);
	tstring += 2;
	iy = iy*100 + twodigits(tstring);
	tstring += 2;
	/* per XCDR, old dates are time_t 0 */
	if (iy < 1970)
	    return (time_t) 0;
	for (i = 1970; i < iy; i++)
		val += dysize(i);
	im = twodigits(tstring);
	tstring += 2;
	/* 
	 * Leap year 
	 */
	if (dysize(iy) == 366 && im >= 3)
		val++;
	/*
	 * Do the current year
	 */
	while(--im)
		val += dmsize[im-1];
	val += twodigits(tstring)-1;
	tstring += 2;
	val = 24*val + twodigits(tstring); /* hours */
	tstring += 2;
	val = 60*val + twodigits(tstring); /* minutes */
	tstring += 2;
	val = 60*val + twodigits(tstring); /* secs */
	tstring += 2;
	tstring += 2;			/* can't resolve hundredths */
	/* o is signed value, in 15-minute increments.
	   negative is west (behind GMT)
	   positive is east (ahead of GMT).
	   subtract so that, e.g. noon US EST (which is GMT-5h)
	   becomes 5pm GMT */
	val -= *tstring * 15 * 60;	
#ifdef __alpha
	/* depends on time_t being typedef'd as an int */
	if (val > INT_MAX)
	    retval = INT_MAX;
	else
#endif
	    retval = (time_t)val;
    } else
	cdfs_tounixdate(&retval,
			tstring[0],
			tstring[1],
			tstring[2],
			tstring[3],
			tstring[4],
			tstring[5],
			tstring[6]);
    return retval;
}
