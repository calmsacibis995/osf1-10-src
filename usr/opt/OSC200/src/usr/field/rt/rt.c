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
static char *rcsid = "@(#)$RCSfile: rt.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/10/13 14:04:10 $";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#define SECTOR_SIZE 512

unsigned char sector_buf[SECTOR_SIZE];
unsigned char* sbe = sector_buf+SECTOR_SIZE;

#define PAGE_SIZE 4096

#define BIG_BUF_NSEC 80 /* 50 */       /* At least enough to span a cylinder.*/
#define BIG_BUF_SIZE (SECTOR_SIZE*BIG_BUF_NSEC)
unsigned char bb[BIG_BUF_SIZE+4+PAGE_SIZE]; /*Leave 2 bytes slop at each end.*/
unsigned char* p1s = bb+2;
unsigned char* p1e = bb+2+PAGE_SIZE;

#define GUARD 0x23

#define CODEQ(O,IDC,Q) (((O)&0x7F)^(((O)>>7)&0x7F)^(((O)>>14)&0xFF)^ \
	 ((IDC)&0x7F)^((Q)?0x80:0))
#define CODE(O,IDC) CODEQ((O),(IDC),0)
#define CHK_CODE(V,O,IDC) (((V)^CODE((O),(IDC)))&0x7F)
#define CHK_CODEQ(V,O,IDC,Q) (((V)^CODEQ((O),(IDC),(Q))))


int get_bit( unsigned char* array, long bit_no )
    {
    return array[bit_no>>3]&(1<<(bit_no&7)) ? 1 : 0;
    }

int set_bit( unsigned char* array, long bit_no, int val )
    {
    if ( val )
       {
       array[bit_no>>3] |=  (1<<(bit_no&7));
       return 1;
       }
    else
       {
       array[bit_no>>3] &= ~(1<<(bit_no&7));
       return 0;
       }
    }

unsigned char* get_array( long n_bits )
    {
    return (unsigned char*)calloc(sizeof(unsigned char),(n_bits+7)/8);
    }


void usage( char* my_name, char* problem )
    {
    if ( problem )
	fprintf( stderr, "Bad arg, \"%s\".\n", problem );
    fprintf( stderr,
	    "Usage: %s [s=<s>] [id=<id>] [out=<out>] [-main] [size=<size>]\n",
	    my_name );
    }

FILE* of = stderr;
char iline[200] = "";

FILE* e()
    {
    FILE* foo;
    time_t now;

    time(&now);

    fprintf( of, "\n---------------------------------------------------\n" );
    if ( foo = fopen( "/dev/rfd_debug_on", "r" ) )
	{
	int i;
	while ( (i=fgetc(foo)) != EOF )
            fprintf(of,"%c",i);
	fclose(foo);
	if ( fopen("/dev/rfd_debug_off","r") )
	    fclose(foo);
	}
    fprintf( of, "\n---------------------------------------------------\n" );

    fprintf( of, "%s    %s ", iline, ctime(&now) );
    return of;
    }

int main( int argc, char* argv[] )
    {
    unsigned char* big_buf = p1s;
    unsigned char* big_buf_end = big_buf+BIG_BUF_SIZE;
    long s = 0;
    long old_s = 0;
    long len = 0;
    unsigned char* bits;

    char* special = "";
    char* id = "";
    long is_main = 0;
    long xlimit = -1;
    int i;
    int fd;
    int be_standard = 0;
    char* out = "";
    int trace = 0;
    FILE* to = stdout;
    char* tout = "";
    int test = 0;
    long size = -1;
    int is_block = 0;                  /* 0 == Any alignment.                */
                                       /* 1 == Align start of transfer with  */
                                       /*       start of a sector.           */
                                       /* 2 == Align both start and end of   */
                                       /*       each transfer with sector    */
                                       /*       boundary.                    */
    char* my_name = "xxxxx";
    unsigned char idc = 0;
    char* cp;

    if ( *argv )
	strcat(iline,my_name = *argv++);

    for ( ; *argv ; argv++ )
	{
	strcat(iline," ");
	strcat(iline,*argv);

	if ( !strncmp(*argv,"s=",2) && !*special )
	    special = *argv+2;
	else if ( !strncmp(*argv,"id=",3) && !*id )
	    id = *argv+3;
	else if ( !strncmp(*argv,"tout=",5) && !*tout )
	    trace = 1, tout = *argv+5;
	else if ( !strncmp(*argv,"out=",4) && !*out )
	    out = *argv+4;
	else if ( !strncmp(*argv,"size=",5) && size<0 )
	    size = atol(*argv+5);
	else if ( !strncmp(*argv,"limit=",6) && xlimit<0 )
	    xlimit = atol(*argv+6);
	else if ( !strcmp(*argv,"-test") && !test )
	    test = 1;
	else if ( !strcmp(*argv,"-trace") && !trace )
	    trace = 1;
	else if ( !strcmp(*argv,"block=1") && !is_block )
	    is_block = 1;
	else if ( !strcmp(*argv,"-block1") && !is_block )
	    is_block = 1;
	else if ( !strcmp(*argv,"block=2") && !is_block )
	    is_block = 2;
	else if ( !strcmp(*argv,"-block2") && !is_block )
	    is_block = 2;
	else if ( !strcmp(*argv,"-standard") && !be_standard )
	    be_standard = 1;
	else if ( !strcmp(*argv,"-main") && !is_main )
	    is_main = 1;
	else
	    return usage(my_name,*argv), 2;
	}

    srandom(be_standard?0:time(0));


    if ( !*special )
	return usage(my_name,"must specify s="), 2;
    
    if ( *out )
	if ( ! (of = fopen( out, "a" )) )
	    return usage(my_name,"can't open OUT file"), 2;

    if ( *tout )
	if ( ! (to = fopen( tout, "a" )) )
	    return usage(my_name,"can't open TOUT file"), 2;

    for ( cp = id ; *cp ; cp++ )
	idc ^= *cp;

    printf("Opening \"%s\" ...\n",special);
    fd = open( special, O_RDWR, 0 );
    if ( fd<0 )
	return printf("Failed.\n"), 2;
    else
	printf("ok.\n");
    printf("Reading sector 0 ...\n");
    if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	return printf("Failed.\n"), 2;
    printf("ok.\n");
    if ( size < 0 )
	{
	long min_sectors = 0;
	long max_sectors = 10000;
	printf("Sizing...\n");
	while ( min_sectors + 1 < max_sectors )
	    {
	    long test_sector = (min_sectors+max_sectors)/2;
	    size = min_sectors;
	    if ( lseek(fd,test_sector*SECTOR_SIZE,SEEK_SET) < 0 )
		max_sectors = test_sector;
	    else
		{
		if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
		    max_sectors = test_sector;
		else
		    min_sectors = test_sector;
		}
	    }
	size = max_sectors*SECTOR_SIZE;
	printf( "Size is %ld.\n", size );
	}
    
    if ( is_main )
	{
	long s;
	if ( !(bits = get_array(size)) )
	    return fprintf(e(),"Get_bits failed.\n"), 2;
	printf("Initializing data on disk...\n");
	if ( lseek(fd,0,SEEK_SET) < 0 )
	    return fprintf(e(),"Failed lseek (A).\n"), 2;
	for ( s = 0 ; s < size ; )
	    {
	    unsigned char* cp;
	    long tmp;
	    long len = size-s;

	    if ( len > BIG_BUF_SIZE )
		len = BIG_BUF_SIZE;

	    for ( cp = big_buf ; cp < big_buf_end ; cp++ )
		*cp = CODEQ( cp-big_buf+s, idc, get_bit(bits,cp-big_buf+s) );

if ( s==0 && trace )
fprintf(to,"id:%s, BIG_BUF_SIZE=%ld,@0x%08lx,len=%ld,s=%ld\n",id,
BIG_BUF_SIZE,(long)big_buf,len,s);

	    if ( (tmp=write(fd,big_buf,len)) != len )
		return fprintf(e(),
			"Unable to write(fd,big_buf,%ld), val=%ld, e=%d\n",
			len, tmp, errno ), 2;
	    s += len;
	    }
	printf("Disk initialized OK.\n");
	}

    printf("Reading last sector, (%ld)  ...\n", (size-1)/SECTOR_SIZE );
    if ( lseek(fd,size-SECTOR_SIZE,SEEK_SET) < 0 )
	return fprintf(e(),"Failed lseek.\n"), 2;
    if ( read(fd,sector_buf,SECTOR_SIZE) != SECTOR_SIZE )
	return fprintf(e(),"Failed read.\n"), 2;
    if ( lseek(fd,0,SEEK_SET) < 0 )
	return fprintf(e(),"Failed lseek (2).\n"), 2;

    fprintf(to,"ok. begin main loop, id=%s ...\n",id);

    for( ; xlimit ; xlimit -= xlimit>0 )
	{
	unsigned long r1 = random();
	unsigned long r2 = random();
	unsigned char* cp;
	unsigned char* cpe;
	int do_seek = 1;
	int do_write = 0;

	if ( ++big_buf >= p1e )
	    big_buf = p1s;
	big_buf_end = big_buf+BIG_BUF_SIZE;

	if ( (r1 & 0x06) == 0x06 )
	    s = ((r1>>3)&0x003FFFFF) % size;
	else if ( (r1 & 0x07) == 0x04 )
	    s = old_s;
	else if ( (r1 & 0x07) == 0x05 )
	    s = ((((r1>>3)&0x003FFFFF) % size) & ~(SECTOR_SIZE-1))
		    + ((r1>>4)&0x01) - ((r1>>5)&0x01);
	else if ( (r1 & 0x06) == 0x02 )
	    s = old_s + len,  do_seek = 0;
	else if ( (r1 & 0x07) == 0x00 )
	    s = old_s + len;
	else /* ( (r1 & 0x07) == 0x01 ) */
	    s = size/2 + (r1>>3)%(SECTOR_SIZE*2) - SECTOR_SIZE ;

	if ( test )
	    s &= SECTOR_SIZE-1;

	if ( is_block )
	    s &= ~(SECTOR_SIZE-1);

	if ( s < 0 || s >= size )
	    s = 0;

	old_s = s;

	if ( s != old_s+len )
	    do_seek = 1;

	if ( (r1 & 0x06000000) == 0x06000000 )
	    len = r2 % BIG_BUF_SIZE;
	else if ( (r1 & 0x06000000) == 0x04000000 )
	    len = (SECTOR_SIZE * (r2&0x07)) + ((r2>>3)&0x01)
		    - ((r2>>4)&0x01);
	else
	    len = r2 % (2*SECTOR_SIZE);

	if ( is_block > 1 )
	    len &= ~(SECTOR_SIZE-1);

	if ( len < 0 )
	    len = 0;
	else if ( len > size-s )
	    len = size-s;

	if ( (r1 & 0x78000000) == 0x70000000 )
	    {
	    if ( trace )
		fprintf(to,"id:%s, REOPEN%s\n", id,
			(r1&0x04000000)?"-PAUSED":"" );
	    if ( close(fd) )
		return fprintf(e(),"close error.\n"), 2;
	    if ( r1 & 0x04000000 )
		sleep(4);
	    fd = open( special, O_RDWR, 0 );
	    if ( fd<0 )
		return fprintf(e(),"open error.\n"), 2;
	    do_seek = s != 0;
	    }
	else if ( (r1 & 0x78000000) == 0x78000000 )
	    {
	    if ( trace )
		fprintf(to,"id:%s,SLEEP\n",id);
	    sleep(2);
	    }
	
	if ( is_main && (r2 & 0x30000000) == 0x30000000 )
	    do_write = 1;


if ( trace )
{
fprintf(to,"id:%s,s=%8ld(%4ld+0x%03lx),len=0x%08lx %s\n",id,s,s/SECTOR_SIZE,
s%SECTOR_SIZE,len,do_write?"WRITE":"");
}

	if ( do_seek )
	    if ( lseek(fd,s,SEEK_SET) < 0 )
		return fprintf(e(),"Failed lseek (d,%ld).\n",s), 2;

	cpe = big_buf+len;
	big_buf[-1] = GUARD;
	*cpe = GUARD;

	if ( do_write )
	    {
	    for ( cp = big_buf ; cp < cpe ; cp++ )
		*cp = CODEQ(cp-big_buf+s,idc,set_bit(bits,cp-big_buf+s,
			random()&0x01));
	    if ( write(fd,big_buf,len) != len )
		return fprintf(e(),"Failed write (d).\n"), 2;
	    }
	else
	    {
	    for ( cp = big_buf ; cp < cpe ; cp++ )
		*cp = 0x20;

	    if ( read(fd,big_buf,len) != len )
		return fprintf(e(),"Failed read (d).\n"), 2;
	    }

	if ( *cpe != GUARD || big_buf[-1] != GUARD )
	    return fprintf(e(),"Guard overwrite.\n"), 2;

	for ( cp = big_buf ; cp < cpe ; cp++ )
	    {
	    long ss = cp-big_buf+s;
	    if ( is_main ?  CHK_CODEQ(*cp, ss, idc, get_bit(bits,ss))
		    :  CHK_CODE(*cp, ss, idc) )
		{
		unsigned char* p;
		fprintf(e(),
"\nData bad%s, o=%ld,s=%ld(%ld+0x%02lx)@0x%08lx,d=%02x,e=%02x,len=%ld.\n",
			do_write?" on WRITE":"",
			cp-big_buf, ss, ss/SECTOR_SIZE, ss%SECTOR_SIZE,
			cp, *cp,
			is_main?CODE(ss,idc):CODEQ(ss,idc,get_bit(bits,ss)),
			len );
		p = cp-10;
		if ( p < big_buf )
		    p = big_buf;
		for ( ; p < cp+10 && p < cpe ; p++ )
		    if ( p == cp )
			fprintf(of,"{%02x}",*p);
		    else
			fprintf(of," %02x ",*p);
		fprintf(of,"\n");
		return 2;
		}
	    }
	}
    }
