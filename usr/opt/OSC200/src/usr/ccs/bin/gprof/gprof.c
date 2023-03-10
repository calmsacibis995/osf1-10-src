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
static char	*sccsid = "@(#)$RCSfile: gprof.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 93/02/18 14:45:09 $";
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include "gprof.h"

char	*whoami = "gprof";

    /*
     *	things which get -E excluded by default.
     */
char	*defaultEs[] = { "_mcount" , "__mcleanup" , 0 };

main(argc, argv)
    int argc;
    char **argv;
{
    char	**sp;
    nltype	**timesortnlp;

    --argc;
    argv++;
    debug = 0;
    bflag = TRUE;
    while ( *argv != 0 && **argv == '-' ) {
	(*argv)++;
	switch ( **argv ) {
	case 'a':
	    aflag = TRUE;
	    break;
	case 'b':
	    bflag = FALSE;
	    break;
	case 'c':
	    cflag = TRUE;
	    break;
	case 'd':
	    dflag = TRUE;
	    (*argv)++;
	    debug |= atoi( *argv );
	    debug |= ANYDEBUG;
#	    ifdef DEBUG
		printf("[main] debug = %d\n", debug);
#	    else not DEBUG
		printf("%s: -d ignored\n", whoami);
#	    endif DEBUG
	    break;
	case 'E':
	    ++argv;
	    addlist( Elist , *argv );
	    Eflag = TRUE;
	    addlist( elist , *argv );
	    eflag = TRUE;
	    break;
	case 'e':
	    addlist( elist , *++argv );
	    eflag = TRUE;
	    break;
	case 'F':
	    ++argv;
	    addlist( Flist , *argv );
	    Fflag = TRUE;
	    addlist( flist , *argv );
	    fflag = TRUE;
	    break;
	case 'f':
	    addlist( flist , *++argv );
	    fflag = TRUE;
	    break;
	case 'k':
	    addlist( kfromlist , *++argv );
	    addlist( ktolist , *++argv );
	    kflag = TRUE;
	    break;
	case 's':
	    sflag = TRUE;
	    break;
	case 'z':
	    zflag = TRUE;
	    break;
	}
	argv++;
    }
    if ( *argv != 0 ) {
	a_outname  = *argv;
	argv++;
    } else {
	a_outname  = A_OUTNAME;
    }
    if ( *argv != 0 ) {
	gmonname = *argv;
	argv++;
    } else {
	gmonname = GMONNAME;
    }
	/*
	 *	turn off default functions
	 */
    for ( sp = &defaultEs[0] ; *sp ; sp++ ) {
	Eflag = TRUE;
	addlist( Elist , *sp );
	eflag = TRUE;
	addlist( elist , *sp );
    }
	/*
	 *	how many ticks per second?
	 *	if we can't tell, report time in ticks.
	 */
    hz = hertz();
    if (hz == 0) {
	hz = 1;
	fprintf(stderr, "time is in ticks, not seconds\n");
    }
	/*
	 *	get information about a.out file.
	 */
    getnfile();
	/*
	 *	get information about mon.out file(s).
	 */
    do	{
	getpfile( gmonname );
	if ( *argv != 0 ) {
	    gmonname = *argv;
	}
    } while ( *argv++ != 0 );
	/*
	 *	dump out a gmon.sum file if requested
	 */
    if ( sflag ) {
	dumpsum( GMONSUM );
    }
	/*
	 *	assign samples to procedures
	 */
    asgnsamples();
	/*
	 *	assemble the dynamic profile
	 */
    timesortnlp = doarcs();
	/*
	 *	print the dynamic profile
	 */
    printgprof( timesortnlp );	
	/*
	 *	print the flat profile
	 */
    printprof();	
	/*
	 *	print the index
	 */
    printindex();	
    done();
}

    /*
     * Set up string and symbol tables from a.out.
     *	and optionally the text space.
     * On return symbol table is sorted by value.
     */
getnfile()
{
    FILE	*nfile;
    int		valcmp();

    nfile = fopen( a_outname ,"r");
    if (nfile == NULL) {
	perror( a_outname );
	done();
    }
    /* Get the file header */
    fread(&xbuf, 1, sizeof(xbuf), nfile);
    if (N_BADMAG(xbuf.ah)) {
	fprintf(stderr, "%s: %s: bad format\n", whoami , a_outname );
	done();
    }
#if __mips__ || __alpha__
    /* Get the symbol table header XCOFF */
    fseek(nfile,xbuf.fh.f_symptr,0);
    if (fread(&shdr,sizeof(shdr),1,nfile) != 1) {
	fprintf(stderr, "%s: %s: can't read symbolic header\n",
	    whoami , a_outname );
	done();
    }
#endif
    getstrtab(nfile);
    getsymtab(nfile);
    gettextspace( nfile );
    qsort(nl, nname, sizeof(nltype), valcmp);
    fclose(nfile);
#   ifdef DEBUG
	if ( debug & AOUTDEBUG ) {
	    register int j;

	    for (j = 0; j < nname; j++){
		printf("[getnfile] 0x%0lx\t%s\n", nl[j].value, nl[j].name);
	    }
	}
#   endif DEBUG
}

getstrtab(nfile)
    FILE	*nfile;
{
#if __mips__ || __alpha__
    ssiz = shdr.issExtMax;
#else
    fseek(nfile, (long)(N_SYMOFF(xbuf) + xbuf.a_syms), 0);
    if (fread(&ssiz, sizeof (ssiz), 1, nfile) == 0) {
	fprintf(stderr, "%s: %s: no string table (old format?)\n" ,
		whoami , a_outname );
	done();
    }
#endif
    strtab = (char *)calloc(ssiz, 1);
    if (strtab == NULL) {
	fprintf(stderr, "%s: %s: no room for %d bytes of string table",
		whoami , a_outname , ssiz);
	done();
    }
#if __mips__ || __alpha__
    fseek(nfile, shdr.cbSsExtOffset, 0);
    if (fread(strtab, ssiz, 1, nfile) != 1)
#else
    if (fread(strtab+sizeof(ssiz), ssiz-sizeof(ssiz), 1, nfile) != 1)
#endif
    {
	fprintf(stderr, "%s: %s: error reading string table\n",
		whoami , a_outname );
	done();
    }
}

    /*
     * Read in symbol table
     */
getsymtab(nfile)
    FILE	*nfile;
{
    register long	i;
    int			askfor;

    /* pass1 - count symbols */
#if __mips__ || __alpha__
    EXTR sym;
    fseek(nfile, shdr.cbExtOffset, 0);
    for(nname=0, i=0 ; i<shdr.iextMax; i++) {
	fread(&sym, sizeof(sym), 1, nfile);
	if (sym.asym.st == stProc)
	    nname++;
    }
#else
    struct nlist nbuf;
    fseek(nfile, (long)N_SYMOFF(xbuf), 0);
    for (nname=0, i = xbuf.ah.a_syms; i > 0; i -= sizeof(struct nlist)) {
	fread(&nbuf, sizeof(nbuf), 1, nfile);
	if ( ! funcsymbol( &nbuf ) ) {
	    continue;
	}
	nname++;
    }
#endif
    if (nname == 0) {
	fprintf(stderr, "%s: %s: no symbols\n", whoami , a_outname );
	done();
    }
    askfor = nname + 1;
    nl = (nltype *) calloc( askfor , sizeof(nltype) );
    if (nl == 0) {
	fprintf(stderr, "%s: No room for %d bytes of symbol table\n",
		whoami, askfor * sizeof(nltype) );
	done();
    }

    /* pass2 - read symbols */
    npe = nl;
    nname = 0;
#if __mips__ || __alpha__
    fseek(nfile, shdr.cbExtOffset, 0);
    for( i=0 ; i<shdr.iextMax; i++ ) {
	fread(&sym, sizeof(sym), 1, nfile);
	if( sym.asym.st != stProc ) {
#	    ifdef DEBUG
		if ( debug & AOUTDEBUG ) {
		    printf("[getsymtab] rejecting: 0x%lx %s\n",
			sym.asym.st , strtab + sym.asym.iss );
		}
#	    endif DEBUG
	    continue;
	}
        npe->value = sym.asym.value;
	npe->name = strtab+sym.asym.iss;
#else /* __mips__ || __alpha__ */
    fseek(nfile, (long)N_SYMOFF(xbuf), 0);
    for (i = xbuf.ah.a_syms; i > 0; i -= sizeof(struct nlist)) {
	fread(&nbuf, sizeof(nbuf), 1, nfile);
	if ( ! funcsymbol( &nbuf ) ) {
#	    ifdef DEBUG
		if ( debug & AOUTDEBUG ) {
		    printf( "[getsymtab] rejecting: 0x%lx %s\n" ,
			    nbuf.n_type , strtab + nbuf.n_un.n_strx );
		}
#	    endif DEBUG
	    continue;
	}
	npe->value = nbuf.n_value;
	npe->name = strtab+nbuf.n_un.n_strx;
#endif /* __mips__ || __alpha__ */
#	ifdef DEBUG
	    if ( debug & AOUTDEBUG ) {
		printf( "[getsymtab] %d %s 0x%0lx\n" ,
			nname , npe -> name , npe -> value );
	    }
#	endif DEBUG
	npe++;
	nname++;
    }
    npe->value = -1;
}

    /*
     *	read in the text space of an a.out file
     */
gettextspace( nfile )
    FILE	*nfile;
{
    char	*malloc();
#if __mips__ || __alpha__
    int         size = xbuf.ah.tsize;
#else
    int         size = xbuf.ah._a_text;
#endif
    
    if ( cflag == 0 ) {
	return;
    }
    textspace = (u_char *) malloc( size );
    if ( textspace == 0 ) {
	fprintf( stderr , "%s: ran out room for %d bytes of text space:  " ,
			whoami , size );
	fprintf( stderr , "can't do -c\n" );
	return;
    }
    (void) fseek( nfile , N_TXTOFF( xbuf.fh, xbuf.ah ) , 0 );
    if ( fread( textspace , 1 , size , nfile ) != size ) {
	fprintf( stderr , "%s: couldn't read text space:  " , whoami );
	fprintf( stderr , "can't do -c\n" );
	free( textspace );
	textspace = 0;
	return;
    }
}
    /*
     *	information from a gmon.out file is in two parts:
     *	an array of sampling hits within pc ranges,
     *	and the arcs.
     */
getpfile(filename)
    char *filename;
{
    FILE		*pfile;
    FILE		*openpfile();
    struct rawarc	arc;

    pfile = openpfile(filename);
    readsamples(pfile);
	/*
	 *	the rest of the file consists of
	 *	a bunch of <from,self,count> tuples.
	 */
    while ( fread( &arc , sizeof arc , 1 , pfile ) == 1 ) {
#	ifdef DEBUG
	    if ( debug & SAMPLEDEBUG ) {
		printf( "[getpfile] frompc 0x%lx selfpc 0x%lx count %d\n" ,
			arc.raw_frompc , arc.raw_selfpc , arc.raw_count );
	    }
#	endif DEBUG
	    /*
	     *	add this arc
	     */
	tally( &arc );
    }
    fclose(pfile);
}

FILE *
openpfile(filename)
    char *filename;
{
    struct hdr	tmp;
    FILE	*pfile;

    if((pfile = fopen(filename, "r")) == NULL) {
	perror(filename);
	done();
    }
    fread(&tmp, sizeof(struct hdr), 1, pfile);
    if ( s_highpc != 0 && ( tmp.lowpc != h.lowpc ||
	 tmp.highpc != h.highpc || tmp.ncnt != h.ncnt ) ) {
	fprintf(stderr, "%s: incompatible with first gmon file\n", filename);
	done();
    }
    h = tmp;
    s_lowpc = (unsigned long) h.lowpc;
    s_highpc = (unsigned long) h.highpc;
    lowpc = (unsigned long)h.lowpc / sizeof(UNIT);
    highpc = (unsigned long)h.highpc / sizeof(UNIT);
    sampbytes = h.ncnt - sizeof(int);
    nsamples = sampbytes / sizeof (UNIT);
#   ifdef DEBUG
	if ( debug & SAMPLEDEBUG ) {
	    printf( "[openpfile] hdr.lowpc 0x%lx hdr.highpc 0x%lx hdr.ncnt %d\n",
		h.lowpc , h.highpc , h.ncnt );
	    printf( "[openpfile]   s_lowpc 0x%lx   s_highpc 0x%lx\n" ,
		s_lowpc , s_highpc );
	    printf( "[openpfile]     lowpc 0x%lx     highpc 0x%lx\n" ,
		lowpc , highpc );
	    printf( "[openpfile] sampbytes %d nsamples %d\n" ,
		sampbytes , nsamples );
	}
#   endif DEBUG
    return(pfile);
}

tally( rawp )
    struct rawarc	*rawp;
{
    nltype		*parentp;
    nltype		*childp;

    parentp = nllookup( rawp -> raw_frompc );
    childp = nllookup( rawp -> raw_selfpc );
    if (!parentp || !childp) { return; }
    if ( kflag
	 && onlist( kfromlist , parentp -> name )
	 && onlist( ktolist , childp -> name ) ) {
	return;
    }
    childp -> ncall += rawp -> raw_count;
#   ifdef DEBUG
	if ( debug & TALLYDEBUG ) {
	    printf( "[tally] arc from %s to %s traversed %d times\n" ,
		    parentp -> name , childp -> name , rawp -> raw_count );
	}
#   endif DEBUG
    addarc( parentp , childp , rawp -> raw_count );
}

/*
 * dump out the gmon.sum file
 */
dumpsum( sumfile )
    char *sumfile;
{
    register nltype *nlp;
    register arctype *arcp;
    struct rawarc arc;
    FILE *sfile;

    if ( ( sfile = fopen ( sumfile , "w" ) ) == NULL ) {
	perror( sumfile );
	done();
    }
    /*
     * dump the header; use the last header read in
     */
    if ( fwrite( &h , sizeof h , 1 , sfile ) != 1 ) {
	perror( sumfile );
	done();
    }
    /*
     * dump the samples
     */
    if (fwrite(samples, sizeof (UNIT), nsamples, sfile) != nsamples) {
	perror( sumfile );
	done();
    }
    /*
     * dump the normalized raw arc information
     */
    for ( nlp = nl ; nlp < npe ; nlp++ ) {
	for ( arcp = nlp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
	    arc.raw_frompc = arcp -> arc_parentp -> value;
	    arc.raw_selfpc = arcp -> arc_childp -> value;
	    arc.raw_count = arcp -> arc_count;
	    if ( fwrite ( &arc , sizeof arc , 1 , sfile ) != 1 ) {
		perror( sumfile );
		done();
	    }
#	    ifdef DEBUG
		if ( debug & SAMPLEDEBUG ) {
		    printf( "[dumpsum] frompc 0x%lx selfpc 0x%lx count %d\n" ,
			    arc.raw_frompc , arc.raw_selfpc , arc.raw_count );
		}
#	    endif DEBUG
	}
    }
    fclose( sfile );
}

valcmp(p1, p2)
    nltype *p1, *p2;
{
    if ( p1 -> value < p2 -> value ) {
	return LESSTHAN;
    }
    if ( p1 -> value > p2 -> value ) {
	return GREATERTHAN;
    }
    return EQUALTO;
}

readsamples(pfile)
    FILE	*pfile;
{
    register i;
    UNIT	sample;
    
    if (samples == 0) {
	samples = (UNIT *) calloc(sampbytes?sampbytes:1, sizeof (UNIT));
	if (samples == 0) {
	    fprintf( stderr , "%s: No room for %d sample pc's\n", 
		whoami , sampbytes / sizeof (UNIT));
	    done();
	}
    }
    for (i = 0; i < nsamples; i++) {
	fread(&sample, sizeof (UNIT), 1, pfile);
	if (feof(pfile))
		break;
	samples[i] += sample;
    }
    if (i != nsamples) {
	fprintf(stderr,
	    "%s: unexpected EOF after reading %d/%d samples\n",
		whoami , --i , nsamples );
	done();
    }
}

/*
 *	Assign samples to the procedures to which they belong.
 *
 *	There are three cases as to where pcl and pch can be
 *	with respect to the routine entry addresses svalue0 and svalue1
 *	as shown in the following diagram.  overlap computes the
 *	distance between the arrows, the fraction of the sample
 *	that is to be credited to the routine which starts at svalue0.
 *
 *	    svalue0                                         svalue1
 *	       |                                               |
 *	       v                                               v
 *
 *	       +-----------------------------------------------+
 *	       |					       |
 *	  |  ->|    |<-		->|         |<-		->|    |<-  |
 *	  |         |		  |         |		  |         |
 *	  +---------+		  +---------+		  +---------+
 *
 *	  ^         ^		  ^         ^		  ^         ^
 *	  |         |		  |         |		  |         |
 *	 pcl       pch		 pcl       pch		 pcl       pch
 *
 *	For the vax we assert that samples will never fall in the first
 *	two bytes of any routine, since that is the entry mask,
 *	thus we give call alignentries() to adjust the entry points if
 *	the entry mask falls in one bucket but the code for the routine
 *	doesn't start until the next bucket.  In conjunction with the
 *	alignment of routine addresses, this should allow us to have
 *	only one sample for every four bytes of text space and never
 *	have any overlap (the two end cases, above).
 */
asgnsamples()
{
    register int	j;
    UNIT		ccnt;
    double		time;
    unsigned long	pcl, pch;
    register int	i;
    unsigned long	overlap;
    unsigned long	svalue0, svalue1;

    /* read samples and assign to namelist symbols */
    scale = (double)(highpc - lowpc);
    scale /= (double)nsamples;
    alignentries();
    for (i = 0, j = 1; i < nsamples; i++) {
	ccnt = samples[i];
	if (ccnt == 0)
		continue;
	pcl = lowpc + (unsigned long)(scale * i);
	pch = lowpc + (unsigned long)(scale * (i + 1));
	time = (double)ccnt;
#	ifdef DEBUG
	    if ( debug & SAMPLEDEBUG ) {
		printf( "[asgnsamples] pcl 0x%lx pch 0x%lx ccnt %d\n" ,
			pcl , pch , ccnt );
	    }
#	endif DEBUG
	totime += time;
	for (j = j - 1; j < nname; j++) {
	    svalue0 = nl[j].svalue;
	    svalue1 = nl[j+1].svalue;
		/*
		 *	if high end of tick is below entry address, 
		 *	go for next tick.
		 */
	    if (pch < svalue0)
		    break;
		/*
		 *	if low end of tick into next routine,
		 *	go for next routine.
		 */
	    if (pcl >= svalue1)
		    continue;
	    overlap = min(pch, svalue1) - max(pcl, svalue0);
	    if (overlap > 0) {
#		ifdef DEBUG
		    if (debug & SAMPLEDEBUG) {
			printf("[asgnsamples] (0x%lx->0x%lx-0x%lx) %s gets %f ticks %d overlap\n",
				nl[j].value/sizeof(UNIT), svalue0, svalue1,
				nl[j].name, 
				overlap * time / scale, overlap);
		    }
#		endif DEBUG
		nl[j].time += overlap * time / scale;
	    }
	}
    }
#   ifdef DEBUG
	if (debug & SAMPLEDEBUG) {
	    printf("[asgnsamples] totime %f\n", totime);
	}
#   endif DEBUG
}


unsigned long
min(a, b)
    unsigned long a,b;
{
    if (a<b)
	return(a);
    return(b);
}

unsigned long
max(a, b)
    unsigned long a,b;
{
    if (a>b)
	return(a);
    return(b);
}

    /*
     *	calculate scaled entry point addresses (to save time in asgnsamples),
     *	and possibly push the scaled entry points over the entry mask,
     *	if it turns out that the entry point is in one bucket and the code
     *	for a routine is in the next bucket.
     */
alignentries()
{
    register struct nl	*nlp;
    unsigned long	bucket_of_entry;
    unsigned long	bucket_of_code;

    for (nlp = nl; nlp < npe; nlp++) {
	nlp -> svalue = nlp -> value / sizeof(UNIT);
	bucket_of_entry = (nlp->svalue - lowpc) / scale;
	bucket_of_code = (nlp->svalue + UNITS_TO_CODE - lowpc) / scale;
	if (bucket_of_entry < bucket_of_code) {
#	    ifdef DEBUG
		if (debug & SAMPLEDEBUG) {
		    printf("[alignentries] pushing svalue 0x%lx to 0x%lx\n",
			    nlp->svalue, nlp->svalue + UNITS_TO_CODE);
		}
#	    endif DEBUG
	    nlp->svalue += UNITS_TO_CODE;
	}
    }
}

#if ! __mips__ && !__alpha__
bool
funcsymbol( nlistp )
    struct nlist	*nlistp;
{
    extern char	*strtab;	/* string table from a.out */
    extern int	aflag;		/* if static functions aren't desired */
    char	*name;

	/*
	 *	must be a text symbol,
	 *	and static text symbols don't qualify if aflag set.
	 */
    if ( ! (  ( nlistp -> n_type == ( N_TEXT | N_EXT ) )
	   || ( ( nlistp -> n_type == N_TEXT ) && ( aflag == 0 ) ) ) ) {
	return FALSE;
    }
	/*
	 *	can't have any `funny' characters in name,
	 *	where `funny' includes	`.', .o file names
	 *			and	`$', pascal labels.
	 */
    for ( name = strtab + nlistp -> n_un.n_strx ; *name ; name += 1 ) {
#ifdef	ibmrt
	if ( *name == '.' && name != (strtab + nlistp -> n_un.n_strx) +1)
	    return FALSE;
	if ( *name == '$' && aflag == 0)
	    return FALSE;
#else
	if ( *name == '.' || *name == '$' ) {
	    return FALSE;
	}
#endif
    }
    return TRUE;
}
#endif /* __mips__ && !__alpha__  */

done()
{
    exit(0);
}
